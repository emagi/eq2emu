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

#include <algorithm>
#include "../ClientPacketFunctions.h"
#include "../client.h"
#include "../../common/ConfigReader.h"
#include "../../common/PacketStruct.h"
#include "../Recipes/Recipe.h"
#include "../../common/Log.h"
#include "../Spells.h"
#include "../../common/MiscFunctions.h"
#include "../World.h"

extern ConfigReader configReader;
extern MasterRecipeList master_recipe_list;
extern MasterSpellList master_spell_list;
extern World world;

void ClientPacketFunctions::SendCreateFromRecipe(Client* client, int32 recipeID) {

	// if recipeID is 0 we are repeating the last recipe, if not set the players current recipe to the new one
	if (recipeID == 0)
		recipeID = client->GetPlayer()->GetCurrentRecipe();
	else
		client->GetPlayer()->SetCurrentRecipe(recipeID);

	Recipe* playerRecipe = client->GetPlayer()->GetRecipeList()->GetRecipe(recipeID);
	
	// Get the recipe
	Recipe* recipe = master_recipe_list.GetRecipe(recipeID);

	if(!playerRecipe)
	{
		LogWrite(TRADESKILL__ERROR, 0, "Tradeskills", "%s: ClientPacketFunctions::SendCreateFromRecipe Error finding player recipe %s in their recipe book for recipe id %u", client->GetPlayer()->GetName(), client->GetPlayer()->GetName(), recipe ? recipe->GetID() : 0);
		client->Message(CHANNEL_COLOR_RED, "You do not have %s (%u) in your recipe book.", recipe ? recipe->GetName() : "Unknown", recipe ? recipe->GetID() : 0);
		client->GetPlayer()->SetCurrentRecipe(0);
		return;
	}

	if (!recipe) {
		client->GetPlayer()->SetCurrentRecipe(0);
		LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error loading recipe (%u) in ClientPacketFunctions::SendCreateFromRecipe()", recipeID);
		return;
	}

	// Create the packet
	PacketStruct* packet = configReader.getStruct("WS_CreateFromRecipe", client->GetVersion());
	if (!packet) {
		client->GetPlayer()->SetCurrentRecipe(0);
		LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error loading WS_CreateFromRecipe in ClientPacketFunctions::SendCreateFromRecipe()");
		return;
	}
	vector<int32>::iterator itr;
	vector<Item*> itemss;
	int8 i = 0;
	int8 k = 0;
	int32 firstID = 0;
	int32 IDcount = 0;
	int32 primary_comp_id = 0;
	Item* first_item = 0;
	Item* item = 0;
	Item* item_player = 0;
	Spawn* target = client->GetPlayer()->GetTarget();
	int32 device_id = GetDeviceID(std::string(recipe->GetDevice()));
	if (!target || !target->IsObject() || device_id == 0 || (device_id != 0xFFFFFFFF && ((Object*)target)->GetDeviceID() != device_id)) {
		client->GetPlayer()->SetCurrentRecipe(0);
		client->Message(CHANNEL_COLOR_YELLOW, "You must be at a %s in order to craft this recipe", recipe->GetDevice());
		LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Failed to begin crafting with recipe id %u, recipe device_id is %u (%s), target object is %u.", recipe->GetID(), device_id, recipe->GetDevice(), (target && target->IsObject()) ? ((Object*)target)->GetDeviceID() : 0);
		return;
	}

	// Recipe and crafting table info
	packet->setDataByName("crafting_station", recipe->GetDevice());
	packet->setDataByName("recipe_name", recipe->GetName());
	packet->setDataByName("tier", recipe->GetTier());
	// Mass Production
	int32 mpq = 1; 
	int32 mp = 5;
	vector<int> v{ 1,2,4,6,11,21 };
	// mpq will eventually be retrieved from achievement for mass production
	mpq = v[mp];
	
	packet->setArrayLengthByName("num_mass_production_choices",mpq);
	packet->setArrayDataByName("mass_qty", 1, 0);
	for (int x = 1; x < mpq; x++) {
		packet->setArrayDataByName("mass_qty", x * 5, x);
	}
	// Product info
	item = master_item_list.GetItem(recipe->GetProductID());
	packet->setDataByName("product_name", item->name.c_str());
	packet->setDataByName("icon", item->GetIcon(client->GetVersion()));
	packet->setDataByName("product_qty", recipe->GetProductQuantity());
	packet->setDataByName("primary_title", recipe->GetPrimaryBuildComponentTitle());
	packet->setDataByName("primary_qty_needed", recipe->GetPrimaryComponentQuantity());
	packet->setDataByName("unknown6", 11);
	packet->setDataByName("unknown3", 18);
	// Reset item to 0
	item, item_player = 0;
	
	// Check to see if we have a primary component (slot = 0)
	if (recipe->components.count(0) > 0) {
		vector<int32> rc = recipe->components[0];
			vector <pair<int32, int16>> selected_items;

			int32 total_primary_items = 0;
		for (itr = rc.begin(); itr != rc.end(); itr++) {
			itemss = client->GetPlayer()->item_list.GetAllItemsFromID((*itr));
			if (itemss.size() > 0)
				total_primary_items += itemss.size();
		}
		packet->setArrayLengthByName("num_primary_choices", total_primary_items);


		int16 have_qty = 0;
		for (itr = rc.begin(); itr != rc.end(); itr++, i++) {
			if (firstID == 0)
				firstID = *itr;

			item = master_item_list.GetItem(*itr);
			if (!item)
			{
				client->GetPlayer()->SetCurrentRecipe(0);
				LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error creating packet to client missing item %u", *itr);
				client->Message(CHANNEL_COLOR_RED, "Error producing create recipe packet!  Recipe is trying to find item %u, but it is missing!", *itr);
				safe_delete(packet);
				return;
			}
			item_player = 0;
			item_player = client->GetPlayer()->item_list.GetItemFromID((*itr));

			itemss = client->GetPlayer()->item_list.GetAllItemsFromID((*itr));
			if (itemss.size() > 0) {
				
				int16 needed_qty = recipe->GetPrimaryComponentQuantity();
				if (firstID == 0)
					firstID = *itr;
				for (int8 i = 0; i < itemss.size(); i++, k++) {
					IDcount++;
					if (have_qty < needed_qty) {

						int16 stack_count = itemss[i]->details.count;
						int16 min_qty = min(stack_count, int16(needed_qty - have_qty));
						selected_items.push_back(make_pair(int32(itemss[i]->details.unique_id), min_qty));
						have_qty += min_qty;
					}
					packet->setArrayDataByName("primary_component", itemss[i]->name.c_str(), k);
					packet->setArrayDataByName("primary_item_id", itemss[i]->details.unique_id, k);
					packet->setArrayDataByName("primary_icon", itemss[i]->GetIcon(client->GetVersion()), k);
					packet->setArrayDataByName("primary_total_quantity", itemss[i]->details.count, k);
					//packet->setArrayDataByName("primary_supply_depot", itemss[i]->details.count, k);  // future need
					//packet->setArrayDataByName("primary_unknown3a",);      // future need
				}
				packet->setDataByName("primary_id", itemss[0]->details.unique_id);
				packet->setDataByName("primary_default_selected_id", itemss[0]->details.unique_id);
				for (int8 i = 0; i < selected_items.size(); i++) {





				}
				int16 qty = 0;
				if (item) {
					qty = (int8)item->details.count;
					if (qty > 0 && firstID == primary_comp_id)
						qty -= 1;
				}
			}
			else
				packet->setDataByName("primary_id",*itr);
		}
		// store the id of the primary comp
		primary_comp_id = firstID;
		
		// Set the default item id to the first component id
		packet->setArrayLengthByName("num_primary_items_selected", selected_items.size());
		for (int8 i = 0; i < selected_items.size(); i++) {
			packet->setArrayDataByName("primary_selected_item_qty", selected_items[i].second,i  );
			packet->setArrayDataByName("primary_selected_item_id", selected_items[i].first,i);

		}
				
		// Reset the variables we will reuse
		i = 0;
		firstID = 0;
		item = 0;
		k = 0;
	}
	else {
		LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Recipe has no primary component");
		return;
	}

	// Check to see if we have build components (slot = 1 - 4)
	int8 total_build_components = 0;
	if (recipe->components.count(1) > 0) 
		total_build_components++;
	if (recipe->components.count(2))
		total_build_components++;
	if (recipe->components.count(3))
		total_build_components++;
	if (recipe->components.count(4))
		total_build_components++;
	//--------------------------------------------------------------Start Build Components-------------------------------------------------------------
	if (total_build_components > 0) {
		packet->setArrayLengthByName("num_build_components", total_build_components);
		LogWrite(TRADESKILL__INFO, 0, "Recipes", "num_build_components %u", total_build_components);
		for (int8 index = 0; index < 4; index++) {
			if (recipe->components.count(index + 1) == 0)
				continue;
			packet->setArrayDataByName("build_slot", index, index);
			vector<int32> rc = recipe->components[index + 1];
			int32 total_component_items = 0;
			int8 hasComp = 0;
			vector <pair<int32, int16>> selected_items;
			for (itr = rc.begin(); itr != rc.end(); itr++) {
				itemss = client->GetPlayer()->item_list.GetAllItemsFromID((*itr));
				if (itemss.size() > 0)
					total_component_items += itemss.size();
			}
			packet->setSubArrayLengthByName("num_build_choices", total_component_items, index);// get # build choces first
			hasComp = 0;
			char msgbuf[200] = "";
			for (itr = rc.begin(); itr != rc.end(); itr++) {// iterates through each recipe component to find the stacks in inventory
				item = master_item_list.GetItem(*itr);
				itemss = client->GetPlayer()->item_list.GetAllItemsFromID((*itr));
				if (itemss.size() > 0) { 
					int16 needed_qty = 0;
					int16 have_qty = 0;
					if (index == 0) {
						needed_qty = recipe->GetBuild1ComponentQuantity();
						have_qty = 0;
					}
					else if (index == 1) {
						needed_qty = recipe->GetBuild2ComponentQuantity();
						have_qty = 0;
					}
					else if (index == 2) {
						needed_qty = recipe->GetBuild3ComponentQuantity();
						have_qty = 0;
					}
					else if (index == 3) {
						needed_qty = recipe->GetBuild4ComponentQuantity();
						have_qty = 0;
					}
					if (firstID == 0)
						firstID = *itr;
					if (hasComp == 0) {
						hasComp = 100 + k;
					}
					for (int8 j = 0; j < itemss.size(); j++, k++) { // go through each stack of a compnent
						if (have_qty < needed_qty) {

							int16 stack_count = itemss[j]->details.count;
							int16 min_qty = min(stack_count, int16(needed_qty - have_qty));
							selected_items.push_back(make_pair(int32(itemss[j]->details.unique_id), min_qty));
							have_qty += min_qty;
						}
						item = master_item_list.GetItem(itemss[j]->details.item_id);
						packet->setSubArrayDataByName("build_component", itemss[j]->name.c_str(), index, k);
						packet->setSubArrayDataByName("build_item_id", itemss[j]->details.unique_id, index, k);
						packet->setSubArrayDataByName("build_icon", itemss[j]->GetIcon(client->GetVersion()), index, k);
						packet->setSubArrayDataByName("build_total_quantity", itemss[j]->details.count, index, k);
						//packet->setSubArrayDataByName("build_supply_depot",);  // future need
						//packet->setSubArrayDataByName("build_unknown3",);      // future need

					}
				}
			}
			packet->setSubArrayLengthByName("num_build_items_selected", selected_items.size(),index   );
			for (int8 i = 0; i < selected_items.size(); i++) {
				packet->setSubArrayDataByName("build_selected_item_qty", selected_items[i].second,index, i);
				packet->setSubArrayDataByName("build_selected_item_id", selected_items[i].first,index, i);

			}
			int16 qty = 0;
			if (item) {
				qty = (int16)item->details.count;
				if (qty > 0 && firstID == primary_comp_id)
					qty -= 1;
			}
			if (index == 0) {
				packet->setArrayDataByName("build_title", recipe->GetBuild1ComponentTitle(), index);
				packet->setArrayDataByName("build_qty_needed", recipe->GetBuild1ComponentQuantity(), index);
				if (item)
					packet->setArrayDataByName("build_selected_item_qty_have", min(qty, recipe->GetBuild1ComponentQuantity()), index);
			}
			else if (index == 1) {
				packet->setArrayDataByName("build_title", recipe->GetBuild2ComponentTitle(), index);
				packet->setArrayDataByName("build_qty_needed", recipe->GetBuild2ComponentQuantity(), index);
				if (item)
					packet->setArrayDataByName("build_selected_item_qty_have", min(qty, recipe->GetBuild2ComponentQuantity()), index);
			}
			else if (index == 2) {
				packet->setArrayDataByName("build_title", recipe->GetBuild3ComponentTitle(), index);
				packet->setArrayDataByName("build_qty_needed", recipe->GetBuild3ComponentQuantity(), index);
				if (item)
					packet->setArrayDataByName("build_selected_item_qty_have", min(qty, recipe->GetBuild3ComponentQuantity()), index);
			}
			else {
				packet->setArrayDataByName("build_title", recipe->GetBuild4ComponentTitle(), index);
				packet->setArrayDataByName("build_qty_needed", recipe->GetBuild4ComponentQuantity(), index);
				if (item)
					packet->setArrayDataByName("build_selected_item_qty_have", min(qty, recipe->GetBuild4ComponentQuantity()), index);
			}

			// Reset the variables we will reuse
			i = 0;
			firstID = 0;
			item = 0;
			k = 0;
			}
		
		
		int32 xxx = 0;
	}


	// Check to see if we have a fuel component (slot = 5)
	if (recipe->components.count(5) > 0) {
		vector<int32> rc = recipe->components[5];
		vector <pair<int32, int16>> selected_items;
		for (itr = rc.begin(); itr != rc.end(); itr++, i++) {
			if (firstID == 0)
				firstID = *itr;
			item = master_item_list.GetItem(*itr);
			if (!item)
			{
				LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error creating packet to client missing item %u", *itr);
				client->Message(CHANNEL_COLOR_RED, "Error producing create recipe packet!  Recipe is trying to find item %u, but it is missing!", *itr);
				safe_delete(packet);
				return;
			}
			item_player = 0;
			item_player = client->GetPlayer()->item_list.GetItemFromID((*itr));

			if(client->GetVersion() <= 561) {
				packet->setDataByName("fuel_qty", item->details.count);
				packet->setDataByName("fuel_icon", item->GetIcon(client->GetVersion()));
			}
			
			itemss = client->GetPlayer()->item_list.GetAllItemsFromID((*itr));
			packet->setArrayLengthByName("num_fuel_choices", itemss.size());
			if (itemss.size() > 0) {
				
				int16 needed_qty = recipe->GetFuelComponentQuantity();
				int16 have_qty = 0;
				if (firstID == 0)
					firstID = *itr;
				for (int8 i = 0; i < itemss.size(); i++) {
					IDcount++;
					if (have_qty < needed_qty) {

						int16 stack_count = itemss[i]->details.count;
						int16 min_qty = min(stack_count, int16(needed_qty - have_qty));
						selected_items.push_back(make_pair(int32(itemss[i]->details.unique_id), min_qty));
						have_qty += min_qty;
					}
					packet->setArrayDataByName("fuel_component", itemss[i]->name.c_str(), i);
					packet->setArrayDataByName("fuel_item_id", itemss[i]->details.unique_id, i);
					packet->setArrayDataByName("fuel_icon", itemss[i]->GetIcon(client->GetVersion()), i);
					packet->setArrayDataByName("fuel_total_quantity", itemss[i]->details.count, i);
					//packet->setArrayDataByName("fuel_supply_depot", itemss[i]->details.count, i);  // future need
					//packet->setArrayDataByName("primary_unknown3a",);      // future need
				}
				packet->setDataByName("fuel_selected_item_id", itemss[0]->details.unique_id);
				int16 qty = 0;
				if (item) {
					qty = (int8)item->details.count;
					if (qty > 0 && firstID == primary_comp_id)
						qty -= 1;
				}
			}
			else
				packet->setDataByName("primary_vvv", *itr);
		}
		// store the id of the primary comp
		primary_comp_id = firstID;

		// Set the default item id to the first component id
		packet->setArrayLengthByName("num_fuel_items_selected", selected_items.size());
		for (int8 i = 0; i < selected_items.size(); i++) {
			packet->setArrayDataByName("fuel_selected_item_qty", selected_items[i].second, i);
			packet->setArrayDataByName("fuel_selected_item_id", selected_items[i].first, i);
		}
		packet->setDataByName("fuel_title", recipe->GetFuelComponentTitle());
		packet->setDataByName("fuel_qty_needed", recipe->GetFuelComponentQuantity());


		// Reset the variables we will reuse
		i = 0;
		firstID = 0;
		item = 0;
	}
	else {
		LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Recipe has no fuel component");
		return;
	}

	packet->setDataByName("recipe_id", recipeID);
	
	EQ2Packet* outapp = packet->serialize();
	//DumpPacket(outapp);
	// Send the packet
	client->QueuePacket(outapp);
	safe_delete(packet);
}

void ClientPacketFunctions::SendItemCreationUI(Client* client, Recipe* recipe) {
	// Check for valid recipe
	if (!recipe) {
		LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Recipe = null in ClientPacketFunctions::SendItemCreationUI()");
		return;
	}

	// Load the packet
	PacketStruct* packet = configReader.getStruct("WS_ShowItemCreation", client->GetVersion());
	if (!packet) {
		LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error loading WS_ShowItemCreation in ClientPacketFunctions::SendItemCreationUI()");
		return;
	}

	int16 item_version = GetItemPacketType(packet->GetVersion());

	Item* item = 0;
	RecipeProducts* rp = 0;

	packet->setDataByName("max_possible_durability", 1000);
	packet->setDataByName("max_possible_progress", 1000);

	// All the packets I have looked at these unknowns are always the same
	// so hardcoding them until they are identified.
	packet->setDataByName("unknown2", 1045220557, 0);
	packet->setDataByName("unknown2", 1061997773, 1);

	// Highest stage the player has been able to complete
	// TODO: store this for the player, for now use 0 (none known)
	Recipe* playerRecipe = client->GetPlayer()->GetRecipeList()->GetRecipe(recipe->GetID());
	
	if(!playerRecipe)
	{
		LogWrite(TRADESKILL__ERROR, 0, "Tradeskills", "%s: ClientPacketFunctions::SendItemCreationUI Error finding player recipe in their recipe book for recipe id %u", client->GetPlayer()->GetName(), recipe->GetID());
		client->Message(CHANNEL_COLOR_RED, "%s: SendItemCreationUI Error finding player recipe in their recipe book for recipe id %u!", client->GetPlayer()->GetName(), recipe->GetID());
		safe_delete(packet);
		return;
	}

	packet->setDataByName("progress_levels_known", playerRecipe ? playerRecipe->GetHighestStage() : 0);

	packet->setArrayLengthByName("num_process", 4);
	for (int8 i = 0; i < 4; i++) {

		// Don't like this code but need to change the IfVariableNotSet value on unknown3 element
		// to point to the currect progress_needed
		vector<DataStruct*> dataStructs = packet->GetDataStructs();
		vector<DataStruct*>::iterator itr;
		for (itr = dataStructs.begin(); itr != dataStructs.end(); itr++) {
			DataStruct* data = *itr;
			char tmp[20] = {0};
			sprintf(tmp,"_%i",i);
			string name = "unknown3";
			name.append(tmp);
			if (strcmp(data->GetName(), name.c_str()) == 0) {
				name = "progress_needed";
				name.append(tmp);
				data->SetIfNotSetVariable(name.c_str());
			}
		}
		if (i == 1)
			packet->setArrayDataByName("progress_needed", 400, i);
		else if (i == 2)
			packet->setArrayDataByName("progress_needed", 600, i);
		else if (i == 3)
			packet->setArrayDataByName("progress_needed", 800, i);

		// get the product for this stage, if none found default to fuel
		if (recipe->products.count(i) > 0)
			rp = recipe->products[i];
		if (!rp || (rp->product_id == 0)) {
			rp = new RecipeProducts;
			rp->product_id = recipe->components[5].front();
			rp->product_qty = recipe->GetFuelComponentQuantity();
			rp->byproduct_id = 0;
			rp->byproduct_qty = 0;
			recipe->products[i] = rp;
		}
		item = master_item_list.GetItem(rp->product_id);
		if (!item) {
			LogWrite(TRADESKILL__ERROR, 0, "Recipe", "Error loading item (%u) in ClientPacketFunctions::SendItemCreationUI()", rp->product_id);
			return;
		}

		packet->setArrayDataByName("item_name", item->name.c_str(), i);
		packet->setArrayDataByName("item_icon", item->GetIcon(client->GetVersion()), i);
		
		if(client->GetVersion() < 860) {
			packet->setItemArrayDataByName("item", item, client->GetPlayer(), i, 0, client->GetClientItemPacketOffset());
			//packet->setItemArrayDataByName("item", item, client->GetPlayer(), i, 0, -1);
		}
		else if (client->GetVersion() < 1193)
			packet->setItemArrayDataByName("item", item, client->GetPlayer(), i);
		else
			packet->setItemArrayDataByName("item", item, client->GetPlayer(), i, 0, 2);

		if (rp->byproduct_id > 0) {
			item = 0;
			item = master_item_list.GetItem(rp->byproduct_id);
			if (item) {
				packet->setArrayDataByName("item_byproduct_name", item->name.c_str(), i);
				packet->setArrayDataByName("item_byproduct_icon", item->GetIcon(client->GetVersion()), i);
			}
		}
		
		packet->setArrayDataByName("packettype", item_version, i);
		packet->setArrayDataByName("packetsubtype", 0xFF, i);

		item = 0;
		rp = 0;
	}
	packet->setDataByName("product_progress_needed", 1000);

	rp = recipe->products[4];
	item = master_item_list.GetItem(rp->product_id);

	packet->setDataByName("product_item_name", item->name.c_str());
	packet->setDataByName("product_item_icon", item->GetIcon(client->GetVersion()));

	if(client->GetVersion() < 860)
		packet->setItemByName("product_item", item, client->GetPlayer(), 0, client->GetClientItemPacketOffset());
	else if (client->GetVersion() < 1193)
		packet->setItemByName("product_item", item, client->GetPlayer());
	else
		packet->setItemByName("product_item", item, client->GetPlayer(), 0, 2);

	//packet->setItemByName("product_item", item, client->GetPlayer());

	if (rp->byproduct_id > 0) {
		item = 0;
		item = master_item_list.GetItem(rp->byproduct_id);
		if (item) {
			packet->setDataByName("product_byproduct_name", item->name.c_str());
			packet->setDataByName("product_byproduct_icon", item->GetIcon(client->GetVersion()));
		}
	}

	packet->setDataByName("packettype", item_version);
	packet->setDataByName("packetsubtype", 0xFF);

	// Start of basic work to get the skills to show on the tradeskill window
	// not even close to accurate but skills do get put on the ui
	int8 index = 0;
	int8 size = 0;
	vector<int32>::iterator itr;
	vector<int32> spells = client->GetPlayer()->GetSpellBookSpellIDBySkill(recipe->GetTechnique());
	for (itr = spells.begin(); itr != spells.end(); itr++) {
			size++;
			Spell* spell = master_spell_list.GetSpell(*itr,1);
			if(!spell) {
				
				return;
			}
			if (size > 6) {
				// only 6 slots for skills on the ui
				break;
			}
			packet->setDataByName("skill_id", *itr   ,spell->GetSpellData()->ts_loc_index -1);
	}


	EQ2Packet* outapp = packet->serialize();
	//DumpPacket(outapp);
	client->QueuePacket(outapp);
	safe_delete(packet);
}

void ClientPacketFunctions::StopCrafting(Client* client) {
	if(client->GetPlayer()) {
		client->GetPlayer()->SetTempVisualState(-1);
	}
	client->QueuePacket(new EQ2Packet(OP_StopItemCreationMsg, 0, 0));
}

void ClientPacketFunctions::CounterReaction(Client* client, bool countered) {
	PacketStruct* packet = configReader.getStruct("WS_TSEventReaction", client->GetVersion());
	if (packet) {
		packet->setDataByName("counter_reaction", countered ? 1 : 0);
		client->QueuePacket(packet->serialize());
	}
	safe_delete(packet);
}
