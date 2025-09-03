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
#include "GroundSpawn.h"
#include "World.h"
#include "Spells.h"
#include "Rules/Rules.h"
#include "../common/MiscFunctions.h"
#include "../common/Log.h"

extern ConfigReader configReader;
extern MasterSpellList master_spell_list;
extern World world;
extern RuleManager rule_manager;

GroundSpawn::GroundSpawn(){ 
	packet_num = 0;
	appearance.difficulty = 0;
	spawn_type = 2;
	appearance.pos.state = 129;
	number_harvests = 0;
	num_attempts_per_harvest = 0;
	groundspawn_id = 0;
	MHarvest.SetName("GroundSpawn::MHarvest");
	MHarvestUse.SetName("GroundSpawn::MHarvestUse");
	randomize_heading = true; // we by default randomize heading of groundspawns DB overrides
}

GroundSpawn::~GroundSpawn(){
	
}

EQ2Packet* GroundSpawn::serialize(Player* player, int16 version){
	return spawn_serialize(player, version);
}

int8 GroundSpawn::GetNumberHarvests(){
	return number_harvests;
}

void GroundSpawn::SetNumberHarvests(int8 val){
	number_harvests = val;
}

int8 GroundSpawn::GetAttemptsPerHarvest(){
	return num_attempts_per_harvest;
}

void GroundSpawn::SetAttemptsPerHarvest(int8 val){
	num_attempts_per_harvest = val;
}

int32 GroundSpawn::GetGroundSpawnEntryID(){
	return groundspawn_id;
}

void GroundSpawn::SetGroundSpawnEntryID(int32 val){
	groundspawn_id = val;
}

void GroundSpawn::SetCollectionSkill(const char* val){
	if(val)
		collection_skill = string(val);
}

const char* GroundSpawn::GetCollectionSkill(){
	return collection_skill.c_str();
}

void GroundSpawn::ProcessHarvest(Client* client) {
	LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Process harvesting for player '%s' (%u)", client->GetPlayer()->GetName(), client->GetPlayer()->GetID());

	MHarvest.lock();

	vector<GroundSpawnEntry*>* groundspawn_entries = GetZone()->GetGroundSpawnEntries(groundspawn_id);
	vector<GroundSpawnEntryItem*>* groundspawn_items = GetZone()->GetGroundSpawnEntryItems(groundspawn_id);

	Item*	master_item = 0;
	Item*	master_rare = 0;
	Item*	item = 0;
	Item*	item_rare = 0;

	int16	lowest_skill_level = 0;
	int16	table_choice = 0;
	int32	item_choice = 0;
	int32	rare_choice = 0;
	int8	harvest_type = 0;
	int32	item_harvested = 0;
	int8	reward_total = 1;
	int32	rare_harvested = 0;
	int8	rare_item = 0;
	bool	is_collection = false;

	if (!groundspawn_entries || !groundspawn_items) {
		LogWrite(GROUNDSPAWN__ERROR, 3, "GSpawn", "No groundspawn entries or items assigned to groundspawn id: %u", groundspawn_id);
		client->Message(CHANNEL_COLOR_RED, "Error: There are no groundspawn entries or items assigned to groundspawn id: %u", groundspawn_id);
		MHarvest.unlock();
		return;
	}

	if (number_harvests == 0) {
		LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Total harvests depleated for groundspawn id: %u", groundspawn_id);
		client->SimpleMessage(CHANNEL_COLOR_RED, "Error: This spawn has nothing more to harvest!");
		MHarvest.unlock();
		return;
	}

	Skill* skill = 0;
	if (collection_skill == "Collecting") {
		skill = client->GetPlayer()->GetSkillByName("Gathering");
		is_collection = true;
	}
	else
		skill = client->GetPlayer()->GetSkillByName(collection_skill.c_str()); // Fix: #576 - don't skill up yet with GetSkillByName(skill, true), we might be trying to harvest low level
	
	if (!skill) {
		LogWrite(GROUNDSPAWN__WARNING, 3, "GSpawn", "Player '%s' lacks the skill: '%s'", client->GetPlayer()->GetName(), collection_skill.c_str());
		client->Message(CHANNEL_COLOR_RED, "Error: You do not have the '%s' skill!", collection_skill.c_str());
		MHarvest.unlock();
		return;
	}

	int16 totalSkill = skill->current_val;
	int32 skillID = master_item_list.GetItemStatIDByName(collection_skill);
	int16 max_skill_req_groundspawn = rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Player, MinSkillMultiplierValue)->GetInt16();
	if(max_skill_req_groundspawn < 1) // can't be 0
		max_skill_req_groundspawn = 1;
	
	if(skillID != 0xFFFFFFFF)
	{
		((Entity*)client->GetPlayer())->MStats.lock();
		totalSkill += ((Entity*)client->GetPlayer())->stats[skillID];
		((Entity*)client->GetPlayer())->MStats.unlock();
	}

	for (int8 i = 0; i < num_attempts_per_harvest; i++) {
		vector<GroundSpawnEntry*> mod_groundspawn_entries;

		if (groundspawn_entries) {
			vector<GroundSpawnEntry*> highest_match;
			vector<GroundSpawnEntry*>::iterator itr;

			GroundSpawnEntry* entry = 0;			// current data
			GroundSpawnEntry* selected_table = 0;	// selected table data

													// first, iterate through groundspawn_entries, discard tables player cannot use
			for (itr = groundspawn_entries->begin(); itr != groundspawn_entries->end(); itr++) {
				entry = *itr;

				if(entry->min_skill_level > max_skill_req_groundspawn)
					max_skill_req_groundspawn = entry->min_skill_level;
				
				// if player lacks skill, skip table
				if (entry->min_skill_level > totalSkill)
					continue;
				// if bonus, but player lacks level, skip table
				if (entry->bonus_table && (client->GetPlayer()->GetLevel() < entry->min_adventure_level))
					continue;

				// build modified entries table
				mod_groundspawn_entries.push_back(entry);
				LogWrite(GROUNDSPAWN__DEBUG, 5, "GSpawn", "Keeping groundspawn_entry: %i", entry->min_skill_level);
			}

			// if anything remains, find lowest min_skill_level in remaining set(s)
			if (mod_groundspawn_entries.size() > 0) {
				vector<GroundSpawnEntry*>::iterator itr;
				GroundSpawnEntry* entry = 0;

				for (itr = mod_groundspawn_entries.begin(); itr != mod_groundspawn_entries.end(); itr++) {
					entry = *itr;

					// find the low range of available tables for random roll
					if (lowest_skill_level > entry->min_skill_level || lowest_skill_level == 0)
						lowest_skill_level = entry->min_skill_level;
				}
				LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Lowest Skill Level: %i", lowest_skill_level);
			}
			else {
				// if no tables chosen, you must lack the skills
				// TODO: move this check to LUA when harvest command is first selected
				client->Message(CHANNEL_COLOR_RED, "You lack the skills to harvest this node!");
				LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "All groundspawn_entry tables tossed! No Skills? Something broke?");
				MHarvest.unlock();
				return;
			}

			// now roll to see which table to use
			table_choice = MakeRandomInt(lowest_skill_level, totalSkill);
			LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Random INT for Table by skill level: %i", table_choice);

			int16 highest_score = 0;
			for (itr = mod_groundspawn_entries.begin(); itr != mod_groundspawn_entries.end(); itr++) {
				entry = *itr;

				// determines the highest min_skill_level in the current set of tables (if multiple tables)
				if (table_choice >= entry->min_skill_level && (highest_score == 0 || highest_score < table_choice)) {
					// removes old highest for the new one
					highest_match.clear();
					highest_score = entry->min_skill_level;
				}
				// if the score = level, push into highest_match set
				if (highest_score == entry->min_skill_level)
					highest_match.push_back(entry);
			}

			// if there is STILL more than 1 table player qualifies for, rand() and pick one
			if (highest_match.size() > 1) {
				int16 rand_index = rand() % highest_match.size();
				selected_table = highest_match.at(rand_index);
			}
			else if (highest_match.size() > 0)
				selected_table = highest_match.at(0);

			// by this point, we should have 1 table who's min skill matches the score (selected_table)
			if (selected_table) {
				LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Using Table: %i, %i, %i, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %i",
					selected_table->min_skill_level,
					selected_table->min_adventure_level,
					selected_table->bonus_table,
					selected_table->harvest1,
					selected_table->harvest3,
					selected_table->harvest5,
					selected_table->harvest_imbue,
					selected_table->harvest_rare,
					selected_table->harvest10,
					selected_table->harvest_coin);


				// roll 1-100 for chance-to-harvest percentage
				float chance = MakeRandomFloat(0, 100);
				LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Random FLOAT for harvest percentages: %.2f", chance);

				// starting with the lowest %, select a harvest type + reward qty
				if (chance <= selected_table->harvest10 && is_collection == false) {
					LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Harvest 10 items + Rare Item from table : %i", selected_table->min_skill_level);
					harvest_type = 6;
					reward_total = 10;
				}
				else if (chance <= selected_table->harvest_rare && is_collection == false) {
					LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Harvest Rare Item from table : %i", selected_table->min_skill_level);
					harvest_type = 5;
				}
				else if (chance <= selected_table->harvest_imbue && is_collection == false) {
					LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Harvest Imbue Item from table : %i", selected_table->min_skill_level);
					harvest_type = 4;
				}
				else if (chance <= selected_table->harvest5 && is_collection == false) {
					LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Harvest 5 Items from table : %i", selected_table->min_skill_level);
					harvest_type = 3;
					reward_total = 5;
				}
				else if (chance <= selected_table->harvest3 && is_collection == false) {
					LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Harvest 3 Items from table : %i", selected_table->min_skill_level);
					harvest_type = 2;
					reward_total = 3;
				}
				else if (chance <= selected_table->harvest1 || totalSkill >= skill->max_val || is_collection) {
					LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Harvest 1 Item from table : %i", selected_table->min_skill_level);
					harvest_type = 1;
				}
				else
					LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Harvest nothing...");
				
				float node_maxskill_multiplier = rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Player, HarvestSkillUpMultiplier)->GetFloat();
				if(node_maxskill_multiplier <= 0.0f) {
					node_maxskill_multiplier = 1.0f;
				}
				int16 skillup_max_skill_allowed =  (int16)((float)max_skill_req_groundspawn*node_maxskill_multiplier);
				if (!is_collection && skill && skill->current_val < skillup_max_skill_allowed) {
					skill = client->GetPlayer()->GetSkillByName(collection_skill.c_str(), true); // Fix: #576 - skill up after min skill and adv level checks
				}
			}

			// once you know how many and what type of item to harvest, pick an item from the list
			if (harvest_type) {
				vector<GroundSpawnEntryItem*> mod_groundspawn_items;
				vector<GroundSpawnEntryItem*> mod_groundspawn_rares;
				vector<GroundSpawnEntryItem*> mod_groundspawn_imbue;

				vector<GroundSpawnEntryItem*>::iterator itr;
				GroundSpawnEntryItem* entry = 0;

				// iterate through groundspawn_items, discard items player cannot roll for
				for (itr = groundspawn_items->begin(); itr != groundspawn_items->end(); itr++) {
					entry = *itr;

					// if this is a Rare, or an Imbue, but is_rare flag is 0, skip item
					if ((harvest_type == 5 || harvest_type == 4) && entry->is_rare == 0)
						continue;
					// if it is a 1, 3, or 5 and is_rare = 1, skip
					else if (harvest_type < 4 && entry->is_rare == 1)
						continue;

					// if the grid_id on the item matches player grid, or is 0, keep the item
					if (!entry->grid_id || (entry->grid_id == client->GetPlayer()->GetLocation())) {
						// build modified entries table
						if ((entry->is_rare == 1 && harvest_type == 5) || (entry->is_rare == 1 && harvest_type == 6)) {
							// if the matching item is rare, or harvest10 push to mod rares
							mod_groundspawn_rares.push_back(entry);
							LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Keeping groundspawn_rare_item: %u", entry->item_id);
						}
						if (entry->is_rare == 0 && harvest_type != 4 && harvest_type != 5) {
							// if the matching item is normal,or harvest 10 push to mod items
							mod_groundspawn_items.push_back(entry);
							LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Keeping groundspawn_common_item: %u", entry->item_id);
						}
						if (entry->is_rare == 2 && harvest_type == 4) {
							// if the matching item is imbue item, push to mod imbue
							mod_groundspawn_imbue.push_back(entry);
							LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Keeping groundspawn_imbue_item: %u", entry->item_id);
						}
					}
				}

				// if any items remain in the list, random to see which one gets awarded
				if (mod_groundspawn_items.size() > 0) {
					// roll to see which item index to use
					item_choice = rand() % mod_groundspawn_items.size();
					LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Random INT for which item to award: %i", item_choice);

					// set item_id to be awarded
					item_harvested = mod_groundspawn_items[item_choice]->item_id;

					// if reward is rare, set flag
					rare_item = mod_groundspawn_items[item_choice]->is_rare;

					LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Item ID to award: %u, Rare = %i", item_harvested, item_rare);

					// if 10+rare, handle additional "rare" reward
					if (harvest_type == 6) {
						// make sure there is a rare table to choose from!
						if (mod_groundspawn_rares.size() > 0) {
							// roll to see which rare index to use
							rare_choice = rand() % mod_groundspawn_rares.size();

							// set (rare) item_id to be awarded 
							rare_harvested = mod_groundspawn_rares[rare_choice]->item_id;

							// we're picking a rare here, so obviously this is true ;)
							rare_item = 1;

							LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "RARE Item ID to award: %u", rare_harvested);
						}
						else {
							// all rare entries were eliminated above, or none are assigned. Either way, shouldn't be here!
							LogWrite(GROUNDSPAWN__ERROR, 3, "GSpawn", "Groundspawn Entry for '%s' (%i) has no RARE items!", GetName(), GetID());
						}
					}
				}
				else if (mod_groundspawn_rares.size() > 0) {
					// roll to see which rare index to use
					item_choice = rand() % mod_groundspawn_rares.size();

					// set (rare) item_id to be awarded 
					item_harvested = mod_groundspawn_rares[item_choice]->item_id;

					// we're picking a rare here, so obviously this is true ;)
					rare_item = 1;

					LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "RARE Item ID to award: %u", rare_harvested);
				}
				else if (mod_groundspawn_imbue.size() > 0) {
					// roll to see which rare index to use
					item_choice = rand() % mod_groundspawn_imbue.size();

					// set (rare) item_id to be awarded 
					item_harvested = mod_groundspawn_imbue[item_choice]->item_id;

					// we're picking a rare here, so obviously this is true ;)
					rare_item = 0;

					LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "imbue Item ID to award: %u", rare_harvested);
				}




				else {
					// all item entries were eliminated above, or none are assigned. Either way, shouldn't be here!
					LogWrite(GROUNDSPAWN__ERROR, 0, "GSpawn", "Groundspawn Entry for '%s' (%i) has no items!", GetName(), GetID());
				}

				// if an item was harvested, send updates to client, add item to inventory
				if (item_harvested) {
					char tmp[200] = { 0 };

					// set Normal item harvested
					master_item = master_item_list.GetItem(item_harvested);
					if (master_item) {
						// set details of Normal item
						item = new Item(master_item);
						// set how many of this item the player receives
						item->details.count = reward_total;

						// chat box update for normal item (todo: verify output text)
						client->Message(CHANNEL_HARVESTING, "You %s %i %s from the %s.", GetHarvestMessageName(true).c_str(), item->details.count, item->CreateItemLink(client->GetVersion(), true).c_str(), GetName());
						// add Normal item to player inventory
						bool itemDeleted = false;
						client->AddItem(item, &itemDeleted);

						if(!itemDeleted) {
							//Check if the player has a harvesting quest for this
							client->GetPlayer()->CheckQuestsHarvestUpdate(item, reward_total);

							// if this is a 10+rare, handle sepErately
							if (harvest_type == 6 && rare_item == 1) {
								LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Item ID %u is Normal. Qty %i", item_harvested, item->details.count);

								// send Normal harvest message to client
								sprintf(tmp, "\\#64FFFFYou have %s:\12\\#C8FFFF%i %s", GetHarvestMessageName().c_str(), item->details.count, item->name.c_str());
								client->SendPopupMessage(10, tmp, "ui_harvested_normal", 2.25, 0xFF, 0xFF, 0xFF);
								client->GetPlayer()->UpdatePlayerStatistic(STAT_PLAYER_ITEMS_HARVESTED, item->details.count);

								// set Rare item harvested
								master_rare = master_item_list.GetItem(rare_harvested);
								if (master_rare) {
									// set details of Rare item
									item_rare = new Item(master_rare);
									// count of Rare is always 1
									item_rare->details.count = 1;

									LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Item ID %u is RARE!", rare_harvested);

									// send Rare harvest message to client
									sprintf(tmp, "\\#FFFF6ERare item found!\12%s: \\#C8FFFF%i %s", GetHarvestMessageName().c_str(), item_rare->details.count, item_rare->name.c_str());
									client->Message(CHANNEL_HARVESTING, "You have found a rare item!");
									client->SendPopupMessage(11, tmp, "ui_harvested_rare", 2.25, 0xFF, 0xFF, 0xFF);
									client->GetPlayer()->UpdatePlayerStatistic(STAT_PLAYER_RARES_HARVESTED, item_rare->details.count);

									// chat box update for rare item (todo: verify output text)
									client->Message(CHANNEL_HARVESTING, "You %s %i %s from the %s.", GetHarvestMessageName(true).c_str(), item_rare->details.count, item->CreateItemLink(client->GetVersion(), true).c_str(), GetName());
									// add Rare item to player inventory
									client->AddItem(item_rare);
									//Check if the player has a harvesting quest for this
									client->GetPlayer()->CheckQuestsHarvestUpdate(item_rare, 1);
								}
							}
							else if (rare_item == 1) {
								// if harvest signaled rare or imbue type
								LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Item ID %u is RARE! Qty: %i", item_harvested, item->details.count);

								// send Rare harvest message to client
								sprintf(tmp, "\\#FFFF6ERare item found!\12%s: \\#C8FFFF%i %s", GetHarvestMessageName().c_str(), item->details.count, item->name.c_str());
								client->Message(CHANNEL_HARVESTING, "You have found a rare item!");
								client->SendPopupMessage(11, tmp, "ui_harvested_rare", 2.25, 0xFF, 0xFF, 0xFF);
								client->GetPlayer()->UpdatePlayerStatistic(STAT_PLAYER_RARES_HARVESTED, item->details.count);
							}
							else {
								// send Normal harvest message to client
								LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "Item ID %u is Normal. Qty %i", item_harvested, item->details.count);
								sprintf(tmp, "\\#64FFFFYou have %s:\12\\#C8FFFF%i %s", GetHarvestMessageName().c_str(), item->details.count, item->name.c_str());
								client->SendPopupMessage(10, tmp, "ui_harvested_normal", 2.25, 0xFF, 0xFF, 0xFF);
								client->GetPlayer()->UpdatePlayerStatistic(STAT_PLAYER_ITEMS_HARVESTED, item->details.count);
							}

						}
					}
					else {
						// error!
						LogWrite(GROUNDSPAWN__ERROR, 0, "GSpawn", "Error: Item ID Not Found - %u", item_harvested);
						client->Message(CHANNEL_COLOR_RED, "Error: Unable to find item id %u", item_harvested);
					}
					// decrement # of pulls on this node before it despawns
					number_harvests--;
				}
				else {
					// if no item harvested
					LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "No item_harvested");
					client->Message(CHANNEL_HARVESTING, "You failed to %s anything from %s.", GetHarvestMessageName(true, true).c_str(), GetName());
				}
			}
			else {
				// if no harvest type
				LogWrite(GROUNDSPAWN__DEBUG, 3, "GSpawn", "No harvest_type");
				client->Message(CHANNEL_HARVESTING, "You failed to %s anything from %s.", GetHarvestMessageName(true, true).c_str(), GetName());
			}
		}
	} // cycle through num_attempts_per_harvest
	MHarvest.unlock();

	LogWrite(GROUNDSPAWN__DEBUG, 0, "GSpawn", "Process harvest complete for player '%s' (%u)", client->GetPlayer()->GetName(), client->GetPlayer()->GetID());
}

string GroundSpawn::GetHarvestMessageName(bool present_tense, bool failure){
	string ret = "";
	if((collection_skill == "Gathering" ||collection_skill == "Collecting") && !present_tense)
		ret = "gathered";
	else if(collection_skill == "Gathering" || collection_skill == "Collecting")
		ret = "gather";
	else if(collection_skill == "Mining" && !present_tense)
		ret = "mined";
	else if(collection_skill == "Mining")
		ret = "mine";
	else if (collection_skill == "Fishing" && !present_tense)
		ret = "fished";
	else if(collection_skill == "Fishing")
		ret = "fish";
	else if(collection_skill == "Trapping" && !present_tense && !failure)
		ret = "acquired";
	else if(collection_skill == "Trapping" && failure)
		ret = "trap";
	else if(collection_skill == "Trapping")
		ret = "acquire";
	else if(collection_skill == "Foresting" && !present_tense)
		ret = "forested";
	else if(collection_skill == "Foresting")
		ret = "forest";
	else if (collection_skill == "Collecting")
		ret = "collect";
	return ret;
}

string GroundSpawn::GetHarvestSpellType(){
	string ret = "";
	if(collection_skill == "Gathering" || collection_skill == "Collecting")
		ret = "gather";
	else if(collection_skill == "Mining")
		ret = "mine";
	else if(collection_skill == "Trapping")
		ret = "trap";
	else if(collection_skill == "Foresting")
		ret = "chop";
	else if(collection_skill == "Fishing")
		ret = "fish";
	return ret;
}

string GroundSpawn::GetHarvestSpellName() {
	string ret = "";
	if (collection_skill == "Collecting")
		ret = "Gathering";
	else
		ret = collection_skill;
	return ret;
}

void GroundSpawn::HandleUse(Client* client, string type){
	if(!client || (client->GetVersion() > 561 && type.length() == 0)) // older clients do not send the type
		return;
	//The following check disables the use of the groundspawn if spawn access is not granted
	if (client) {
		bool meets_quest_reqs = MeetsSpawnAccessRequirements(client->GetPlayer());
		if (!meets_quest_reqs && (GetQuestsRequiredOverride() & 2) == 0)
			return;
		else if (meets_quest_reqs && appearance.show_command_icon != 1)
			return;
	}

	MHarvestUse.lock();
	std::string typeLwr = ToLower(type);
	if(client->GetVersion() <= 561 && (typeLwr == "" || GetHarvestMessageName(true, true) == typeLwr))
		type = GetHarvestSpellType();
	
	if (type == GetHarvestSpellType() && MeetsSpawnAccessRequirements(client->GetPlayer())) {
		Spell* spell = master_spell_list.GetSpellByName(GetHarvestSpellName().c_str());
		if (spell)
			client->GetCurrentZone()->ProcessSpell(spell, client->GetPlayer(), client->GetPlayer()->GetTarget(), true, true);
	}
	else if (appearance.show_command_icon == 1 && MeetsSpawnAccessRequirements(client->GetPlayer())) {
		EntityCommand* entity_command = FindEntityCommand(type);
		if (entity_command)
			client->GetCurrentZone()->ProcessEntityCommand(entity_command, client->GetPlayer(), client->GetPlayer()->GetTarget());
	}
	MHarvestUse.unlock();
}
