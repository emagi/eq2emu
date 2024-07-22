/*
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

    This file is part of EQ2Emulator.

    EQ2Emulator is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EQ2Emulator is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EQ2Emulator.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

#include "Tradeskills.h"
#include "../client.h"
#include "../../common/ConfigReader.h"
#include "../classes.h"
//#include "../../common/debug.h"
#include "../../common/Log.h"
//#include "../zoneserver.h"
//#include "../Skills.h"
//#include "../classes.h"
#include "../World.h"
//#include "../LuaInterface.h"
#include "../ClientPacketFunctions.h"
#include "../WorldDatabase.h"
#include "../Rules/Rules.h"

extern Classes classes;
extern ConfigReader configReader;
extern MasterSkillList master_skill_list;
extern MasterRecipeList master_recipe_list;
extern MasterTradeskillEventsList master_tradeskillevent_list;
extern WorldDatabase database;
extern RuleManager rule_manager;

TradeskillMgr::TradeskillMgr() {
	m_tradeskills.SetName("TradeskillMgr::tradeskillsList");
	// % chance for each was made up by me (Jabantiz) and may need some tweaking
	// 2% for crit fail
	m_success		= rule_manager.GetGlobalRule(R_World, TradeskillSuccessChance)->GetFloat();
	m_critSuccess	= rule_manager.GetGlobalRule(R_World, TradeskillCritSuccessChance)->GetFloat();
	m_fail			= rule_manager.GetGlobalRule(R_World, TradeskillFailChance)->GetFloat();
	m_critFail		= rule_manager.GetGlobalRule(R_World, TradeskillCritFailChance)->GetFloat();
	m_eventChance	= rule_manager.GetGlobalRule(R_World, TradeskillEventChance)->GetFloat();

	if ((m_success + m_critSuccess + m_fail + m_critFail) != 100.0f) {
		LogWrite(TRADESKILL__ERROR, 0, "Tradeskills", "Success, crit success, fail, and crit fail MUST add up to 100, reverting to defaults...");
		m_success = 87.0f;
		m_critSuccess = 2.0f;
		m_fail = 10.0f;
		m_critFail = 1.0f;
	}
}

TradeskillMgr::~TradeskillMgr() {
	m_tradeskills.writelock(__FUNCTION__, __LINE__);

	map<Client*, Tradeskill*>::iterator itr;
	for (itr = tradeskillList.begin(); itr != tradeskillList.end(); itr++)
		safe_delete(itr->second);

	tradeskillList.clear();

	m_tradeskills.releasewritelock(__FUNCTION__, __LINE__);
}

void TradeskillMgr::Process() {
	m_tradeskills.writelock(__FUNCTION__, __LINE__);
	map<Client*, Tradeskill*>::iterator itr = tradeskillList.begin();
	while (itr != tradeskillList.end()) {
		Tradeskill* tradeskill = 0;
		tradeskill = itr->second;
		if (!tradeskill)
			continue;
		if (Timer::GetCurrentTime2() >= tradeskill->nextUpdateTime) {
			Client* client = itr->first;
			if(!client->GetPlayer()) {
				continue;
			}
			SetClientIdleVisualState(client, tradeskill);
			
			sint32 progress = 0;
			sint32 durability = 0;
			/*
			Following was grabbed from
			http://eq2.stratics.com/content/guides/padasher_crafting_2.php
			old but the base fail/succes should still be the same

			-100 Durability / -50 Progress (Critical Failure) 
			-50 Durability / 0 Progress (Failure) 
			-10 Durability / +50 Progress (Standard tick) 
			+10 Durability / + 100 Progress (Critical Success) 
			*/
			float roll = MakeRandomFloat(0, 100);
			int8 effect = 0; //1 is critical success, 2 is success, 3 is failure, and 4 is critical failure.

			float success = m_success;
			float crit_success = m_critSuccess;
			float fail = m_fail;
			float crit_fail = m_critFail;

			// Modify the % chance for success based off of stats
			client->GetPlayer()->MStats.lock();
			fail -= client->GetPlayer()->stats[ITEM_STAT_SUCCESS_MOD];
			success += client->GetPlayer()->stats[ITEM_STAT_SUCCESS_MOD];
			client->GetPlayer()->MStats.unlock();

			// add values together for the if
			crit_success += crit_fail;
			fail += crit_success;
			success += fail;

			// Crit fail
			if (roll <= crit_fail) {
				progress = -50;
				durability = -100;
				effect = 4;
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Critical failure!");
			}
			// Crit success
			else if (roll > crit_fail && roll <= crit_success) {
				progress = 100;
				durability = 10;
				effect = 1;
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Critical success!");
			}
			// Fail
			else if (roll > crit_success && roll <= fail) {
				progress = 0;
				durability = -50;
				effect = 3;
			}
			// Success
			else if (roll > fail && roll <= success) {
				progress = 50;
				durability = -10;
				effect = 2;
			}
			else {
				// Just a debug, should never end up in this, if we do write out a log but treat as a success for the player
				LogWrite(TRADESKILL__ERROR, 0, "Tradeskills", "Process roll was not within valid range. roll = %f, crit fail = %f, crit success = %f, fail = %f, success = %f", roll, crit_fail, crit_success, fail, success);
				progress = 50;
				durability = -10;
				effect = 2;
			}

			// Check to see if there was an event, if there was give out the rewards/penalties for it
			if (tradeskill->CurrentEvent) {
				if (tradeskill->eventCountered) {
					progress += tradeskill->CurrentEvent->SuccessProgress;
					durability += tradeskill->CurrentEvent->SuccessDurability;
				}
				else {
					progress += tradeskill->CurrentEvent->FailProgress;
					durability += tradeskill->CurrentEvent->FailDurability;
				}
			}

			// Modify the progress/durability by the players stats
			client->GetPlayer()->MStats.lock();
			progress += client->GetPlayer()->stats[ITEM_STAT_PROGRESS_ADD];
			durability += client->GetPlayer()->stats[ITEM_STAT_DURABILITY_ADD];
			client->GetPlayer()->MStats.unlock();

			tradeskill->currentDurability += durability;
			tradeskill->currentProgress += progress;

			PacketStruct* packet = configReader.getStruct("WS_UpdateCreateItem", client->GetVersion());
			if (packet) {
				packet->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(tradeskill->table));
				packet->setDataByName("effect", effect);
				packet->setDataByName("total_durability", tradeskill->currentDurability);
				packet->setDataByName("total_progress", tradeskill->currentProgress);
				packet->setDataByName("durability_change", durability);
				packet->setDataByName("progress_change", progress);

				if (tradeskill->currentProgress >= 1000)
					packet->setDataByName("progress_level", 4);
				else if (tradeskill->currentProgress >= 800)
					packet->setDataByName("progress_level", 3);
				else if (tradeskill->currentProgress >= 600)
					packet->setDataByName("progress_level", 2);
				else if (tradeskill->currentProgress >= 400)
					packet->setDataByName("progress_level", 1);
				else
					packet->setDataByName("progress_level", 0);

				// Reset the tradeskill event
				tradeskill->CurrentEvent = 0;
				tradeskill->eventChecked = false;
				tradeskill->eventCountered = false;

				// 15% chance for an event (change this to a rule probably)

				int eventRoll = MakeRandomFloat(0, 100);
				if (eventRoll <= m_eventChance) {
					// Get a vector of all possible events for this crafting technique
					vector<TradeskillEvent*>* events = master_tradeskillevent_list.GetEventByTechnique(tradeskill->recipe->GetTechnique());
					if (events) {
						// Get the size of the vector
						int size = events->size();
						// Get a random number from 0 to size - 1 to use as an index
						int index = MakeRandomInt(0, size - 1);
						// use the index to get an event
						TradeskillEvent* TSEvent = events->at(index);
						if (TSEvent) {
							// Now that we got a random event set it in the packet
							packet->setDataByName("reaction_icon", TSEvent->Icon);
							packet->setDataByName("reaction_name", TSEvent->Name);

							// Set the current tradeskill event
							tradeskill->CurrentEvent = TSEvent;
						}
					}
				}
				EQ2Packet* pack = packet->serialize();
				//packet->PrintPacket();
				client->QueuePacket(pack);
				safe_delete(packet);
			}

			if (tradeskill->currentProgress >= 1000) {
				itr++;
				StopCrafting(client, false);
				continue;
			}
			else
				tradeskill->nextUpdateTime = Timer::GetCurrentTime2() + 4000;
		}
		itr++;
	}
	m_tradeskills.releasewritelock(__FUNCTION__, __LINE__);
}

void TradeskillMgr::BeginCrafting(Client* client, vector<pair<int32,int16>> components) {
	Recipe* recipe = master_recipe_list.GetRecipe(client->GetPlayer()->GetCurrentRecipe());

	if (!recipe) {
		LogWrite(TRADESKILL__ERROR, 0, "Recipe", "Recipe (%u) not found in TradeskillMgr::BeginCrafting()", client->GetPlayer()->GetCurrentRecipe());
		ClientPacketFunctions::StopCrafting(client);
		return;
	}

	// TODO: use the vecotr to lock inventory slots
	vector<pair<int32, int16>>::iterator itr;
	bool missingItem = false;
	int32 itemid = 0;
	vector<Item*> tmpItems;
	for (itr = components.begin(); itr != components.end(); itr++) {
		itemid = itr->first;
		Item* item = client->GetPlayer()->item_list.GetItemFromUniqueID(itemid);
		int8 qty_req = 0;
		if(!item)
		{
			missingItem = true;
			break;
		}
		
		item->details.item_locked = true;
		tmpItems.push_back(item);
	}
	
	if(!recipe->ProvidedAllRequiredComponents(client, &tmpItems, &components)) {
		LogWrite(TRADESKILL__ERROR, 0, "Recipe", "Recipe (%u) quantity of items incorrect or component missing.", recipe->GetID());
		missingItem = true;
	}

	if (missingItem) {
		LogWrite(TRADESKILL__ERROR, 0, "Recipe", "Recipe (%u) player missing item when attempting to create recipe.", recipe->GetID());
		vector<Item*>::iterator itemitr;
		for (itemitr = tmpItems.begin(); itemitr != tmpItems.end(); itemitr++) {
			Item* tmpItem = *itemitr;
			tmpItem->details.item_locked = false;
		}
		ClientPacketFunctions::StopCrafting(client);
		return;
	}

	ClientPacketFunctions::SendItemCreationUI(client, recipe);
	Tradeskill* tradeskill = new Tradeskill;
	tradeskill->player = client->GetPlayer();
	tradeskill->table = client->GetPlayer()->GetTarget();
	tradeskill->recipe = recipe;
	tradeskill->currentDurability = 1000;
	tradeskill->currentProgress = 0;
	tradeskill->nextUpdateTime = Timer::GetCurrentTime2() + 500;
	tradeskill->usedComponents = components;
	tradeskill->CurrentEvent = 0;
	tradeskill->eventChecked = false;
	tradeskill->eventCountered = false;
	m_tradeskills.writelock(__FUNCTION__, __LINE__);
	tradeskillList.insert(make_pair(client, tradeskill));
	
	SetClientIdleVisualState(client, tradeskill);
	m_tradeskills.releasewritelock(__FUNCTION__, __LINE__);

	// Unlock TS Spells and lock all others
	client->GetPlayer()->UnlockTSSpells();
				
	client->ClearSentItemDetails();
	EQ2Packet* outapp = client->GetPlayer()->SendInventoryUpdate(client->GetVersion());
	if (outapp)
		client->QueuePacket(outapp);
}

void TradeskillMgr::StopCrafting(Client* client, bool lock) {
	
	if (lock)
		m_tradeskills.writelock(__FUNCTION__, __LINE__);

	if (tradeskillList.count(client) == 0) {
		if (lock)
			m_tradeskills.releasewritelock(__FUNCTION__, __LINE__);
		return;
	}

	Tradeskill* tradeskill = 0;
	tradeskill = tradeskillList[client];

	//TODO: unlock inventory slots, give the product to the player, give tradeskill xp
	ClientPacketFunctions::StopCrafting(client);

	int32 dur = tradeskill->currentDurability;
	int32 progress = tradeskill->currentProgress;
	Recipe* recipe = tradeskill->recipe;
	vector<pair<int32, int16>>::iterator itr;
	Item* item = 0;
	int32 item_id = 0;
	int8 i = 0;
	int8 qty = 0;

	Recipe* playerRecipe = client->GetPlayer()->GetRecipeList()->GetRecipe(recipe->GetID());
	
	if(!playerRecipe)
	{
		LogWrite(TRADESKILL__ERROR, 0, "Tradeskills", "%s: TradeskillMgr::StopCrafting Error finding player recipe in their recipe book for recipe id %u", client->GetPlayer()->GetName(), recipe->GetID());
		client->Message(CHANNEL_COLOR_RED, "%s: StopCrafting Error finding player recipe in their recipe book for recipe id %u!", client->GetPlayer()->GetName(), recipe->GetID());
		if (lock)
			m_tradeskills.releasewritelock(__FUNCTION__, __LINE__);
		return;
	}
	bool updateInvReq = false;
	// cycle through the list of used items and remove them
	for (itr = tradeskill->usedComponents.begin(); itr != tradeskill->usedComponents.end(); itr++, i++) {
		// Get the item in the players inventory and remove or reduce the quantity
		int32 itmid = itr->first;
		qty = itr->second > 0 ? itr->second : 1;
		item = client->GetPlayer()->item_list.GetItemFromUniqueID(itmid);
		if (item && item->details.count <= qty)
		{
			item->details.item_locked = false;
			client->GetPlayer()->item_list.RemoveItem(item);
			updateInvReq = true;
		}
		else if(item) {
			item->details.count -= qty;
			item->details.item_locked = false;
			item->save_needed = true;
			updateInvReq = true;
		}
		else
		{
			LogWrite(TRADESKILL__ERROR, 0, "Tradeskills", "%s: TradeskillMgr::StopCrafting Error finding item %u to remove quantity for recipe id %u", client->GetPlayer()->GetName(), itmid, recipe->GetID());
			client->Message(CHANNEL_COLOR_RED, "%s: StopCrafting Error finding item %u to remove quantity for recipe id %u!", client->GetPlayer()->GetName(), itmid, recipe->GetID());
		}
	}

	if(updateInvReq)
	{
		EQ2Packet* outapp = client->GetPlayer()->SendInventoryUpdate(client->GetVersion());
		if (outapp)
			client->QueuePacket(outapp);
	}

	item = 0;
	qty = recipe->GetFuelComponentQuantity();	
	item_id = recipe->components[5][0];
	int32 byproduct_itemid = 0;
	int16 byproduct_qty = 0;
	float tsx = 0;
	bool success = false;
	int8 HS = playerRecipe->GetHighestStage();
	if (progress < 400) { //stage 0
		if (recipe->products.count(0) > 0) {
			item_id = recipe->products[0]->product_id;
			qty = recipe->products[0]->product_qty;
			byproduct_itemid = recipe->products[0]->byproduct_id;
			byproduct_qty = recipe->products[0]->byproduct_qty;
		}
		tsx = 1;
	}
	else if (progress >= 400 && progress < 600) { //stage 1
		if (HS & (1 << (1 - 1))) {
		}
		else {
			playerRecipe->SetHighestStage(HS + 1 );
			database.UpdatePlayerRecipe(client->GetPlayer(), recipe->GetID(), playerRecipe->GetHighestStage());
		}
		if (recipe->products.count(1) > 0) {
			item_id = recipe->products[1]->product_id;
			qty = recipe->products[1]->product_qty;
			byproduct_itemid = recipe->products[1]->byproduct_id;
			byproduct_qty = recipe->products[1]->byproduct_qty;
		}
		tsx = .45;
	}
	//else if (progress >= 600 && progress < 800) { //stage 2
	else if ((dur < 200 && progress >= 600) || (dur >= 200 && progress >= 600 && progress < 800)) { //stage 2
		if (HS & (1 << (2 - 1))) {
		}
		else {
			playerRecipe->SetHighestStage(HS + 2);
			database.UpdatePlayerRecipe(client->GetPlayer(), recipe->GetID(), playerRecipe->GetHighestStage());
		}
		if (recipe->products.count(2) > 0) {
			item_id = recipe->products[2]->product_id;
			qty = recipe->products[2]->product_qty;
			byproduct_itemid = recipe->products[2]->byproduct_id;
			byproduct_qty = recipe->products[2]->byproduct_qty;
		}
		tsx = .30;
	}
	else if ((dur >= 200 && dur < 800 && progress >= 800) || (dur >= 800 && progress >= 800 && progress < 1000)) { // stage 3
	//else if (progress >= 800 && progress < 1000) { // stage 3
		if (HS & (1 << (3 - 1))) {
		}
		else {
			playerRecipe->SetHighestStage(HS + 4);
			database.UpdatePlayerRecipe(client->GetPlayer(), recipe->GetID(), playerRecipe->GetHighestStage());
		}
		if (recipe->products.count(3) > 0) {
			item_id = recipe->products[3]->product_id;
			qty = recipe->products[3]->product_qty;
			byproduct_itemid = recipe->products[3]->byproduct_id;
			byproduct_qty = recipe->products[3]->byproduct_qty;
		}
		tsx = .15;
	}
	else if (dur >= 800 && progress >= 1000) {  // stage 4
	//else if (progress >= 1000) {  // stage 4
		if (HS & (1 << (4 - 1))) {
		}
		else {
			playerRecipe->SetHighestStage(HS + 8);
			database.UpdatePlayerRecipe(client->GetPlayer(), recipe->GetID(), playerRecipe->GetHighestStage());
		}
		if (recipe->products.count(4) > 0) {
			success = true;
			item_id = recipe->products[4]->product_id;
			qty = recipe->products[4]->product_qty;
			byproduct_itemid = recipe->products[4]->byproduct_id;
			byproduct_qty = recipe->products[4]->byproduct_qty;
		}
	}
	
	if(progress > 1000 && item_id < 1) {
		LogWrite(TRADESKILL__INFO, 0, "Tradeskills", "%s: TradeskillMgr::StopCrafting progress over 1000, but no item id set with recipe id %u, finding override.  Highest Stage: %u, progress: %u, durability %u", client->GetPlayer()->GetName(), recipe->GetID(), HS, progress, dur);
		for(int i=4;i>=0;i--) {
			if (recipe->products.count(i) > 0) {
				item_id = recipe->products[i]->product_id;
				qty = recipe->products[i]->product_qty;
				byproduct_itemid = recipe->products[i]->byproduct_id;
				byproduct_qty = recipe->products[i]->byproduct_qty;
				break;
			}
		}
	}
	
	if(item_id) {
	LogWrite(TRADESKILL__INFO, 0, "Tradeskills", "%s: TradeskillMgr::StopCrafting crafted item %u with recipe id %u.  Highest Stage: %u, progress: %u, durability %u", client->GetPlayer()->GetName(), item_id, recipe->GetID(), HS, progress, dur);

	item = new Item(master_item_list.GetItem(item_id));
		if (!item) {
			LogWrite(TRADESKILL__ERROR, 0, "Tradeskills", "Item (%u) not found.", item_id);
		}
		else {
			item->details.count = qty;
			// use CHANNEL_COLOR_CHAT_RELATIONSHIP as that is the same value (4) as it is in a log for this message
			client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "You created %s.", item->CreateItemLink(client->GetVersion()).c_str());
			client->AddItem(item);
			if(byproduct_itemid) {
				Item* byproductItem = new Item(master_item_list.GetItem(byproduct_itemid));
				byproductItem->details.count = byproduct_qty;
				client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "You received %s as a byproduct.", byproductItem->CreateItemLink(client->GetVersion()).c_str());
				client->AddItem(byproductItem);
			}
			//Check for crafting quest updates
			int8 update_amt = 0;
			if(item->stack_count > 1)
				update_amt = 1;
			else
				update_amt = qty;
			client->GetPlayer()->CheckQuestsCraftUpdate(item, update_amt);
		}
	}
	else {
		LogWrite(TRADESKILL__WARNING, 0, "Tradeskills", "%s: TradeskillMgr::StopCrafting no item summoned for player with recipe id %u.  Highest Stage: %u, progress: %u, durability %u", client->GetPlayer()->GetName(), recipe->GetID(), HS, progress, dur);
	}

	float xp = client->GetPlayer()->CalculateTSXP(recipe->GetLevel());
	xp = xp - (xp * tsx);
	if (xp > 0) {
		int16 level = client->GetPlayer()->GetTSLevel();
		if (client->GetPlayer()->AddTSXP((int32)xp)) {
			client->Message(CHANNEL_REWARD, "You gain %u Tradeskill XP!", (int32)xp);
			LogWrite(PLAYER__DEBUG, 0, "Player", "Player: %s earned %u tradeskill experience.", client->GetPlayer()->GetName(), (int32)xp);
			if(client->GetPlayer()->GetTSLevel() != level)
				client->ChangeTSLevel(level, client->GetPlayer()->GetTSLevel());
			client->GetPlayer()->SetCharSheetChanged(true);
		}
	}
	
	if(tradeskill && tradeskill->recipe) {
		if(success) {
			int32 success_anim = GetTechniqueSuccessAnim(client->GetVersion(), tradeskill->recipe->GetTechnique());
			client->GetPlayer()->GetZone()->PlayAnimation(client->GetPlayer(), success_anim);
		}
		else {
			int32 failure_anim = GetTechniqueFailureAnim(client->GetVersion(), tradeskill->recipe->GetTechnique());
			client->GetPlayer()->GetZone()->PlayAnimation(client->GetPlayer(), failure_anim);
		}
	}

	tradeskillList.erase(client);
	safe_delete(tradeskill);

	if (lock)
		m_tradeskills.releasewritelock(__FUNCTION__, __LINE__);

	// Lock TS spells and unlock all others
	client->GetPlayer()->LockTSSpells();
}

bool TradeskillMgr::IsClientCrafting(Client* client) {
	bool ret = false;

	m_tradeskills.readlock(__FUNCTION__, __LINE__);
	ret = tradeskillList.count(client) > 0;
	m_tradeskills.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

void TradeskillMgr::CheckTradeskillEvent(Client* client, int16 icon) {
	// Check to see if the given client is crafting
	if (!IsClientCrafting(client))
		return;

	m_tradeskills.writelock(__FUNCTION__, __LINE__);
	// check to see if the client currently has an event and if it does if we had already tried to counter it this round
	if (tradeskillList[client]->CurrentEvent == 0 || tradeskillList[client]->eventChecked) {
		// No current event, or we already tried to counter it, return out
		m_tradeskills.releasewritelock(__FUNCTION__, __LINE__);
		return;
	}

	// set the eventChecked flag so we don't try to counter it again
	tradeskillList[client]->eventChecked = true;
	// compare the event icon with the given spell icon to see if we countered it and store the result for the update
	bool countered = (icon == tradeskillList[client]->CurrentEvent->Icon);
	tradeskillList[client]->eventCountered = countered;

	// send the success or fail message to the client
	client->Message(CHANNEL_NARRATIVE, "You %s %s.", countered ? "successfully countered" : "failed to counter", tradeskillList[client]->CurrentEvent->Name);
	
	// unlock the list and send the result packet
	m_tradeskills.releasewritelock(__FUNCTION__, __LINE__);
	ClientPacketFunctions::CounterReaction(client, countered);
}

Tradeskill* TradeskillMgr::GetTradeskill(Client* client) {
	if (tradeskillList.count(client) == 0)
		return 0;

	return tradeskillList[client];
}

int32 TradeskillMgr::GetTechniqueSuccessAnim(int16 version, int32 technique) {
	switch(technique) {
		case SKILL_ID_SCULPTING: {
			if(version <= 561) {
				return 3007; // leatherworking_success
			}
			
			return 11785; // 3005 = failure, 3006 = idle.  11783 = failure, 11784 = idle
			break;
		}
		case SKILL_ID_ARTISTRY: {
			if(version <= 561) {
				return 2319; // cooking_success
			}
			
			return 11245; // 2317 = failure, 2318 = idle.  11243 = failure, 11244 = idle
			break;
		}
		case SKILL_ID_FLETCHING: {
			if(version <= 561) {
				return 2356; // woodworking_success
			}
			
			return 13309; // 2354 = failure, 2355 = idle.  13307 = failure, 13308 = idle
			break;
		}
		case SKILL_ID_METALWORKING:
		case SKILL_ID_METALSHAPING: {
			if(version <= 561) {
				return 2442; // metalworking_success
			}
			
			return 11813; // 2441 = failure, 1810 = idle.  11811 = failure, 11812 = idle
			break;
		}
		case SKILL_ID_TAILORING: {
			if(version <= 561) {
				return 2352; // tailoring_success
			}
			
			return 13040; // 2350 = failure, 2351 = idle.  13038 = failure, 13039 = idle
			break;
		}
		case SKILL_ID_CHEMISTRY:{
			if(version <= 561) {
				return 2298; // alchemy_success
			}
			
			return 10749; // 2296 = failure, 2297 = idle.  10747 = failure, 10748 = idle
			break;
		}
		case SKILL_ID_ARTIFICING:{
			if(version <= 561) {
				return 2304; // artificing_success
			}
			
			return 10767; // 2302 = failure, 2303 = idle.  10765 = failure, 10766 = idle
			break;
		}
		case SKILL_ID_SCRIBING: {
			if(version <= 561) {
				return 0; // ???
			}
			
			return 0; // ??? = failure, 3131 = idle.  ??? = failure, 12193 = idle
			break;
		}
	}
	return 0;
}

int32 TradeskillMgr::GetTechniqueFailureAnim(int16 version, int32 technique) {
	switch(technique) {
		case SKILL_ID_SCULPTING: {
			if(version <= 561) {
				return 3005; // leatherworking_failure
			}
			
			return 11783; // 3005 = failure, 3006 = idle.  11783 = failure, 11784 = idle
			break;
		}
		case SKILL_ID_ARTISTRY: {
			if(version <= 561) {
				return 2317; // cooking_failure
			}
			
			return 11243; // 2317 = failure, 2318 = idle.  11243 = failure, 11244 = idle
			break;
		}
		case SKILL_ID_FLETCHING: {
			if(version <= 561) {
				return 2354; // woodworking_failure
			}
			
			return 13307; // 2354 = failure, 2355 = idle.  13307 = failure, 13308 = idle
			break;
		}
		case SKILL_ID_METALWORKING:
		case SKILL_ID_METALSHAPING: {
			if(version <= 561) {
				return 2441; // metalworking_failure
			}
			
			return 11811; // 2441 = failure, 1810 = idle.  11811 = failure, 11812 = idle
			break;
		}
		case SKILL_ID_TAILORING: {
			if(version <= 561) {
				return 2350; // tailoring_failure
			}
			
			return 13038; // 2350 = failure, 2351 = idle.  13038 = failure, 13039 = idle
			break;
		}
		case SKILL_ID_CHEMISTRY:{
			if(version <= 561) {
				return 2298; // alchemy_success
			}
			
			return 10749; // 2296 = failure, 2297 = idle.  10747 = failure, 10748 = idle
			break;
		}
		case SKILL_ID_ARTIFICING:{
			if(version <= 561) {
				return 2302; // artificing_failure
			}
			
			return 10765; // 2302 = failure, 2303 = idle.  10765 = failure, 10766 = idle
			break;
		}
		case SKILL_ID_SCRIBING: {
			if(version <= 561) {
				return 0; // ???
			}
			
			return 0; // ??? = failure, 3131 = idle.  ??? = failure, 12193 = idle
			break;
		}
	}
	return 0;
}

int32 TradeskillMgr::GetTechniqueIdleAnim(int16 version, int32 technique) {
	switch(technique) {
		case SKILL_ID_SCULPTING: {
			if(version <= 561) {
				return 3006; // leatherworking_idle
			}
			
			return 11784; // 3005 = failure, 3006 = idle.  11783 = failure, 11784 = idle
			break;
		}
		case SKILL_ID_ARTISTRY: {
			if(version <= 561) {
				return 2318; // cooking_idle
			}
			
			return 11244; // 2317 = failure, 2318 = idle.  11243 = failure, 11244 = idle
			break;
		}
		case SKILL_ID_FLETCHING: {
			if(version <= 561) {
				return 2355; // woodworking_idle
			}
			
			return 13308; // 2354 = failure, 2355 = idle.  13307 = failure, 13308 = idle
			break;
		}
		case SKILL_ID_METALWORKING:
		case SKILL_ID_METALSHAPING: {
			if(version <= 561) {
				return 1810; // metalworking_idle
			}
			
			return 11812; // 2441 = failure, 1810 = idle.  11811 = failure, 11812 = idle
			break;
		}
		case SKILL_ID_TAILORING: {
			if(version <= 561) {
				return 2351; // tailoring_idle
			}
			
			return 13039; // 2350 = failure, 2351 = idle.  13038 = failure, 13039 = idle
			break;
		}
		case SKILL_ID_CHEMISTRY:{
			if(version <= 561) {
				return 2297; // alchemy_idle
			}
			
			return 10748; // 2296 = failure, 2297 = idle.  10747 = failure, 10748 = idle
			break;
		}
		case SKILL_ID_ARTIFICING:{
			if(version <= 561) {
				return 2303; // artificing_idle
			}
			
			return 10766; // 2302 = failure, 2303 = idle.  10765 = failure, 10766 = idle
			break;
		}
		case SKILL_ID_SCRIBING: {
			if(version <= 561) {
				return 3131; // scribing_idle
			}
			
			return 12193; // ??? = failure, 3131 = idle.  ??? = failure, 12193 = idle
			break;
		}
	}
	return 0;
}

int32 TradeskillMgr::GetMissTargetAnim(int16 version) {
	if(version <= 561) {
		return 1144;
	}
	
	return 11814; // 11815 seems also possible?
}

int32 TradeskillMgr::GetKillMissTargetAnim(int16 version) {
	if(version <= 561) {
		return 33912;
	}
	
	return 44582; // 44583 seems also possible?
}

void TradeskillMgr::SetClientIdleVisualState(Client* client, Tradeskill* ts) {
	if(!client || !ts || !client->GetPlayer()) {
		return;
	}
	
	int32 idle_anim = 0;
	if(ts->recipe) {
		idle_anim = GetTechniqueIdleAnim(client->GetVersion(), ts->recipe->GetTechnique());
	}
	if(idle_anim) {
		client->GetPlayer()->SetTempVisualState(idle_anim);
	}
}

MasterTradeskillEventsList::MasterTradeskillEventsList() {
	m_eventList.SetName("MasterTradeskillEventsList::eventList");
}

MasterTradeskillEventsList::~MasterTradeskillEventsList() {
	m_eventList.writelock(__FUNCTION__, __LINE__);
	map<int32, vector<TradeskillEvent*> >::iterator itr;
	vector<TradeskillEvent*>::iterator ts_itr;
	for (itr = eventList.begin(); itr != eventList.end(); itr++){
		for (ts_itr = itr->second.begin(); ts_itr != itr->second.end(); ts_itr++){
			safe_delete(*ts_itr);
		}
	}
	eventList.clear();
	m_eventList.releasewritelock(__FUNCTION__, __LINE__);
}

void MasterTradeskillEventsList::AddEvent(TradeskillEvent* tradeskillEvent) {
	m_eventList.writelock(__FUNCTION__, __LINE__);
	eventList[tradeskillEvent->Technique].push_back(tradeskillEvent);
	m_eventList.releasewritelock(__FUNCTION__, __LINE__);
}

vector<TradeskillEvent*>* MasterTradeskillEventsList::GetEventByTechnique(int32 technique) {
	if (eventList.count(technique) == 0)
		return 0;

	return &eventList[technique];
}

int32 MasterTradeskillEventsList::Size() {
	int32 count = 0;
	m_eventList.readlock(__FUNCTION__, __LINE__);
	map<int32, vector<TradeskillEvent*> >::iterator itr;
	for (itr = eventList.begin(); itr != eventList.end(); itr++)
		count += itr->second.size();
	m_eventList.releasereadlock(__FUNCTION__, __LINE__);
	return count;
}
