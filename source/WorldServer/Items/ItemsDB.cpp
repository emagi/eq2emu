/*  
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2005 - 2026  EQ2EMulator Development Team (http://www.eq2emu.com formerly http://www.eq2emulator.net)

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

#ifdef WIN32
	#include <WinSock2.h>
	#include <windows.h>
#endif
#include <mysql.h>
#include <assert.h>
#include "../../common/Log.h"
#include "../WorldDatabase.h"
#include "Items.h"
#include "../World.h"
#include "../Rules/Rules.h"
#include "../LuaInterface.h"

extern World world;
extern RuleManager rule_manager;
extern LuaInterface* lua_interface;

// handle new database class til all functions are converted
void WorldDatabase::LoadDataFromRow(DatabaseResult* result, Item* item) 
{
// this is too much on top of already having the top level load item debug msg
//	LogWrite(ITEM__DEBUG, 5, "Items", "\tSetting details for item ID: %i", result->GetInt32Str("id"));

	item->details.item_id						= result->GetInt32Str("id");
	int8 size									= strlen(result->GetStringStr("name"));

	if(size > 63)
		size = 63;

	item->name									= string(result->GetStringStr("name"));
	item->lowername								= ToLower(item->name);
	item->details.icon							= result->GetInt16Str("icon");
	item->details.classic_icon					= result->GetInt16Str("classic_icon");
	item->details.count							= result->GetInt16Str("count");
	item->details.tier							= result->GetInt8Str("tier");
	item->generic_info.weight					= result->GetInt32Str("weight");

	if( strlen(result->GetStringStr("description")) > 0 )
		item->description						= string(result->GetStringStr("description"));

	item->generic_info.show_name				= result->GetInt8Str("show_name");

	if( result->GetInt8Str("attuneable") == 1 )
		item->generic_info.item_flags += ATTUNEABLE;

	if( result->GetInt8Str("artifact") == 1 )
		item->generic_info.item_flags += ARTIFACT;

	if( result->GetInt8Str("lore") == 1 )
		item->generic_info.item_flags += LORE;
	
	if( result->GetInt8Str("temporary") == 1 )
		item->generic_info.item_flags += TEMPORARY;

	if( result->GetInt8Str("notrade") == 1 )
		item->generic_info.item_flags += NO_TRADE;

	if( result->GetInt8Str("novalue") == 1 )
		item->generic_info.item_flags += NO_VALUE;

	if( result->GetInt8Str("nozone") == 1 )
		item->generic_info.item_flags += NO_ZONE;

	if( result->GetInt8Str("nodestroy") == 1 )
		item->generic_info.item_flags += NO_DESTROY;

	if( result->GetInt8Str("crafted") == 1 )
		item->generic_info.item_flags += CRAFTED;

	if( result->GetInt8Str("good_only") == 1 )
		item->generic_info.item_flags += GOOD_ONLY;

	if( result->GetInt8Str("evil_only") == 1 )
		item->generic_info.item_flags += EVIL_ONLY;

	if( result->GetInt8Str("stacklore") == 1 )
		item->generic_info.item_flags += STACK_LORE;
	
	// add more Flags/Flags2 here

	if (result->GetInt8Str("lore_equip") == 1)
		item->generic_info.item_flags += LORE_EQUIP;

	if (result->GetInt8Str("no_transmute") == 1)
		item->generic_info.item_flags += NO_TRANSMUTE;

	if (result->GetInt8Str("CURSED_flags_32768") == 1)
		item->generic_info.item_flags += CURSED;

	if (result->GetInt8Str("ornate") == 1)
		item->generic_info.item_flags2 += ORNATE;

	if (result->GetInt8Str("heirloom") == 1)
		item->generic_info.item_flags2 += HEIRLOOM;

	if (result->GetInt8Str("appearance_only") == 1)
		item->generic_info.item_flags2 += APPEARANCE_ONLY;

	if (result->GetInt8Str("unlocked") == 1)
		item->generic_info.item_flags2 += UNLOCKED;

	if (result->GetInt8Str("reforged") == 1)
		item->generic_info.item_flags2 += REFORGED;

	if (result->GetInt8Str("norepair") == 1)
		item->generic_info.item_flags2 += NO_REPAIR;

	if (result->GetInt8Str("etheral") == 1)
		item->generic_info.item_flags2 += ETHERAL;

	if (result->GetInt8Str("refined") == 1)
		item->generic_info.item_flags2 += REFINED;

	if (result->GetInt8Str("no_salvage") == 1)
		item->generic_info.item_flags2 += NO_SALVAGE;

	if (result->GetInt8Str("indestructable") == 1)
		item->generic_info.item_flags2 += INDESTRUCTABLE;
		
	if (result->GetInt8Str("no_experiment") == 1)
		item->generic_info.item_flags2 += NO_EXPERIMENT;

	if (result->GetInt8Str("house_lore") == 1)
		item->generic_info.item_flags2 += HOUSE_LORE;

	if (result->GetInt8Str("building_block") == 1)
		item->generic_info.item_flags2 += BUILDING_BLOCK;

	if (result->GetInt8Str("free_reforge") == 1)
		item->generic_info.item_flags2 += FREE_REFORGE;
	

	if( result->GetInt32Str("skill_id_req") == 0 )
		item->generic_info.skill_req1			= 0xFFFFFFFF;
	else
		item->generic_info.skill_req1			= result->GetInt32Str("skill_id_req");

	if( result->GetInt32Str("skill_id_req2") == 0 )
		item->generic_info.skill_req2			= 0xFFFFFFFF;
	else
		item->generic_info.skill_req2			= result->GetInt32Str("skill_id_req2");

	item->generic_info.skill_min				= result->GetInt16Str("skill_min");

	if( result->GetInt32Str("slots") > 0)
		item->SetSlots(result->GetInt32Str("slots"));

	item->sell_price							= result->GetInt32Str("sell_price");
	item->sell_status							= result->GetInt32Str("sell_status_amount");
	item->stack_count							= result->GetInt16Str("stack_count");
	item->generic_info.collectable				= result->GetInt8Str("collectable");
	item->generic_info.offers_quest_id			= result->GetInt32Str("offers_quest_id");
	item->generic_info.part_of_quest_id			= result->GetInt32Str("part_of_quest_id");
	item->details.recommended_level				= result->GetInt16Str("recommended_level");
	item->details.item_locked					= false;
	item->details.lock_flags					= 0;
	item->generic_info.adventure_default_level	= result->GetInt16Str("adventure_default_level");
	item->generic_info.max_charges				= result->GetInt16Str("max_charges");
	item->generic_info.display_charges			= result->GetInt8Str("display_charges");
	item->generic_info.tradeskill_default_level	= result->GetInt16Str("tradeskill_default_level");

	item->generic_info.adventure_classes		= result->GetInt64Str("adventure_classes");
	item->generic_info.tradeskill_classes		= result->GetInt64Str("tradeskill_classes");

	if( !result->IsNullStr("lua_script") && strlen(result->GetStringStr("lua_script")) > 0 )
	{
		item->SetItemScript(string(result->GetStringStr("lua_script")));
		LogWrite(ITEM__DEBUG, 5, "LUA", "--Loading LUA Item Script: '%s'", item->item_script.c_str());
	}
	
	item->effect_type 							= (ItemEffectType)result->GetInt32Str("effect_type");

	if(item->generic_info.max_charges > 0)
		item->details.count						= item->generic_info.max_charges;

	if(item->details.count == 0)
		item->details.count						= 1;

	item->generic_info.usable					= result->GetInt8Str("usable");
	item->details.soe_id						= result->GetSInt32Str("soe_item_id");

	item->generic_info.harvest					= result->GetInt8Str("harvest");
	item->generic_info.body_drop				= result->GetInt8Str("body_drop");

	item->no_buy_back							= (result->GetInt8Str("no_buy_back") == 1);

	item->generic_info.pvp_description			= result->GetInt8Str("bPvpDesc");

	item->generic_info.merc_only				= (result->GetInt8Str("merc_only") == 1);
	item->generic_info.mount_only				= (result->GetInt8Str("mount_only") == 1);
	
	item->generic_info.set_id					= result->GetInt32Str("set_id");
	
	item->generic_info.collectable_unk			= result->GetInt8Str("collectable_unk");

	const char* offerQuestName = result->GetFieldValueStr("offers_quest_name");
	if(offerQuestName)
		strncpy(item->generic_info.offers_quest_name,offerQuestName,255);
	else
		strcpy(item->generic_info.offers_quest_name,"");

	const char* requiredQuestName = result->GetFieldValueStr("required_by_quest_name");
	if(requiredQuestName)
		strncpy(item->generic_info.required_by_quest_name,requiredQuestName,255);
	else
		strcpy(item->generic_info.required_by_quest_name,"");
		
	item->generic_info.transmuted_material		= result->GetInt8Str("transmuted_material");
	
	item->crafted = result->GetInt8Str("crafted");
	item->tinkered = result->GetInt8Str("tinkered");
	item->book_language = result->GetInt8Str("book_language");
	
}

int32 WorldDatabase::LoadSkillItems(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, spell_id, spell_tier FROM item_details_skill%s", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 total = 0;
	int32 id = 0;

	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			id = atoul(row[0]);
			Item* item = master_item_list.GetItem(id);

			if(!row[1] || !row[2])
				continue;

			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tLoading Skill for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\ttype: %i, spell: %i, tier: %i", ITEM_TYPE_SKILL, atoi(row[1]), atoi(row[2]));
				item->SetItemType(ITEM_TYPE_SKILL);
				item->skill_info->spell_id = atoul(row[1]);
				item->skill_info->spell_tier = atoi(row[2]);
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_skill`, ID: %i", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadShields(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, mitigation_low, mitigation_high FROM item_details_shield%s", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 total = 0;
	int32 id = 0;

	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			id = strtoul(row[0], NULL, 0);
			Item* item = master_item_list.GetItem(id);

			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tItem Shield for item_id: %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\ttype: %i, mit_low: %i, mit_high: %i", ITEM_TYPE_SHIELD, atoi(row[1]), atoi(row[2]));
				item->SetItemType(ITEM_TYPE_SHIELD);
				item->armor_info->mitigation_low = atoi(row[1]);
				item->armor_info->mitigation_high = atoi(row[2]);
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_shield`, ID: %i", id);
		}
	}
	return total;
}
int32 WorldDatabase::LoadAdornments(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, duration, item_types,slot_type FROM item_details_adornments%s", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 total = 0;
	int32 id = 0;

	if (result)
	{
		while (result && (row = mysql_fetch_row(result)))
		{
			id = strtoul(row[0], NULL, 0);
			Item* item = master_item_list.GetItem(id);

			if (item)
			{
				//LogWrite(ITEM__DEBUG, 0, "Items", "\tItem Adornment for item_id: %u", id);
				//LogWrite(ITEM__DEBUG, 0, "Items", "\ttype: %i, Duration: %i, item_types_: %i, slot_type: %i", ITEM_TYPE_ADORNMENT, atoi(row[1]), atoi(row[2]), atoi(row[3]));
				item->SetItemType(ITEM_TYPE_ADORNMENT);
				item->adornment_info->duration = atof(row[1]);
				item->adornment_info->item_types = atoi(row[2]);
				item->adornment_info->slot_type = atoi(row[3]);
				//LogWrite(ITEM__DEBUG, 0, "Items", "\ttype: %i, Duration: %i, item_types_: %i, slot_type: %i",item->generic_info.item_type, item->adornment_info->duration, item->adornment_info->item_types, item->adornment_info->slot_type);
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_shield`, ID: %i", id);
		}
	}
	return total;
}
int32 WorldDatabase::LoadClassifications()
{
	int32 total = 0;
	int32 id = 0;
	return total;
}

int32 WorldDatabase::LoadBaubles(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, cast, recovery, duration, recast, display_slot_optional, display_cast_time, display_bauble_type, effect_radius, max_aoe_targets, display_until_cancelled FROM item_details_bauble%s", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 total = 0;
	int32 id = 0;

	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			id = strtoul(row[0], NULL, 0);
			Item* item = master_item_list.GetItem(id);

			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tItem Bauble for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\ttype: %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i", ITEM_TYPE_BAUBLE, atoi(row[1]), atoi(row[2]), atoi(row[3]), atoi(row[4]), atoi(row[5]), atoi(row[6]), atof(row[7]), atoi(row[8]), atoi(row[9]), atoi(row[10]));
				item->SetItemType(ITEM_TYPE_BAUBLE);
				item->bauble_info->cast = atoi(row[1]);
				item->bauble_info->recovery = atoi(row[2]);
				item->bauble_info->duration = atoi(row[3]);
				item->bauble_info->recast = atoi(row[4]);
				item->bauble_info->display_slot_optional = atoi(row[5]);
				item->bauble_info->display_cast_time = atoi(row[6]);
				item->bauble_info->display_bauble_type = atoi(row[7]);
				item->bauble_info->effect_radius = atof(row[8]);
				item->bauble_info->max_aoe_targets = atoi(row[9]);
				item->bauble_info->display_until_cancelled = atoi(row[10]);
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_bauble`, ID: %i", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadBooks(int32 item_id)
{
	DatabaseResult result;
	int32 total = 0;
	int32 id = 0;

	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	if( database_new.Select(&result, "SELECT item_id, language, author, title FROM item_details_book%s", (item_id == 0) ? "" : select_query_addition.c_str()) )
	{
		while( result.Next() )
		{
			id = result.GetInt32Str("item_id");
			Item* item = master_item_list.GetItem(id);

			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tItem Book for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\ttype: %i, %i, %s, %s", 
					ITEM_TYPE_BOOK, 
					result.GetInt8Str("language"), 
					result.GetStringStr("author"), 
					result.GetStringStr("title"));

				item->SetItemType(ITEM_TYPE_BOOK);
				item->book_info->language = result.GetInt8Str("language");
				item->book_info->author.data = result.GetStringStr("author");
				item->book_info->author.size = item->book_info->author.data.length();
				item->book_info->title.data = result.GetStringStr("title");
				item->book_info->title.size = item->book_info->title.data.length();

				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_book`, ID: %i", id);
		}
	}

	return total;
}
int32 WorldDatabase::LoadItemsets(int32 item_id)
{
	DatabaseResult result;
	int32 total = 0;
	int32 id = 0;

	std::string select_query_addition = std::string(" and crate.item_id = ") + std::to_string(item_id);
	//if (database_new.Select(&result, "SELECT id, itemset_item_id, item_id, item_icon,item_stack_size,item_list_color,language_type FROM item_details_itemset"))
	if (database_new.Select(&result, "select crate.item_id, crateitem.reward_item_id, crateitem.icon, crateitem.stack_size, crateitem.name_color, crateitem.name, crateitem.language_type from item_details_reward_crate crate, item_details_reward_crate_item crateitem where crateitem.crate_item_id = crate.item_id%s", (item_id == 0) ? "" : select_query_addition.c_str()))
	{
		while (result.Next())
		{
			id = result.GetInt32(0);
			Item* item = master_item_list.GetItem(id);

			if (item)
			{
				item->SetItemType(ITEM_TYPE_ITEMCRATE);
				//int32 item_id = result.GetInt32Str("item_id");
				const char* setName = result.GetString(5);
				item->AddSet(result.GetInt32(1),0, result.GetInt16(2), result.GetInt16(3), result.GetInt32(4), setName ? string(setName) : string(""), result.GetInt8(6));
				

				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Item Set Crate Items", "Error loading `item_details_Items`, ID: %i", id);
		}
	}

	return total;
}
int32 WorldDatabase::LoadHouseItem(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, rent_reduction, status_rent_reduction, coin_rent_reduction, house_only, house_location FROM item_details_house%s", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 total = 0;
	int32 id = 0;

	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			id = strtoul(row[0], NULL, 0);
			Item* item = master_item_list.GetItem(id);

			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tItem HouseItem for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\ttype: %i, %i, %u, %.2f, %u", ITEM_TYPE_HOUSE, atoul(row[1]), atoi(row[2]), atof(row[3]), atoul(row[4]));
				item->SetItemType(ITEM_TYPE_HOUSE); // container will be overwritten by LoadHouseContainers which is ran after
				item->houseitem_info->status_rent_reduction = atoi(row[2]);
				item->houseitem_info->coin_rent_reduction = atof(row[3]);
				item->houseitem_info->house_only = atoi(row[4]);
				item->houseitem_info->house_location = atoul(row[5]);
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_house`, ID: %i", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadRecipeBookItems(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	//std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	//MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, name FROM item_details_recipe_items%s", (item_id == 0) ? "" : select_query_addition.c_str());
	std::string select_query_addition = std::string(" and r.item_id = ") + std::to_string(item_id);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT  r.item_id, ri.recipe_id ,ri.`name`,ri.soe_recipe_crc FROM item_details_recipe r LEFT JOIN item_details_recipe_items ri ON ri.recipe_id = r.recipe_id where ri.recipe_id is not null%s", (item_id == 0) ? "" : select_query_addition.c_str());
	
	int32 total = 0;
	int32 id = 0;
	uint32 soe_id = 0;
	if (result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			id = strtoul(row[0], NULL, 0);
			Item* item = master_item_list.GetItem(id);
			soe_id = strtoul(row[3], NULL, 0);
			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tRecipe Book for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\tType: %i, '%s'", ITEM_TYPE_RECIPE, row[2]);
				item->SetItemType(ITEM_TYPE_RECIPE);
				item->recipebook_info->recipe_id = (atoi(row[1]));
				item->recipebook_info->recipes.push_back(soe_id);
				//item->recipebook_info->recipe_id(row[1]);
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_recipe_items`, ID: %u", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadHouseContainers(int32 item_id){
	DatabaseResult result;
	int32 total = 0;
	int32 id = 0;


	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	if( database_new.Select(&result, "SELECT item_id, num_slots, allowed_types, broker_commission, fence_commission FROM item_details_house_container%s", (item_id == 0) ? "" : select_query_addition.c_str()) )
	{
		while (result.Next() )
		{
			id = result.GetInt32Str("item_id");
			Item* item = master_item_list.GetItem(id);

			if (item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tHouse Container for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\tType: %i, '%i', '%u', '%i', '%i'", ITEM_TYPE_HOUSE_CONTAINER, result.GetInt8Str("num_slots"), result.GetInt64Str("allowed_types"), result.GetInt8Str("broker_commission"), result.GetInt8Str("fence_commission"));

				item->SetItemType(ITEM_TYPE_HOUSE_CONTAINER);
				item->housecontainer_info->num_slots = result.GetInt8Str("num_slots");
				item->housecontainer_info->allowed_types = result.GetInt64Str("allowed_types");
				item->housecontainer_info->broker_commission = result.GetInt8Str("broker_commission");
				item->housecontainer_info->fence_commission = result.GetInt8Str("fence_commission");

				item->details.num_slots = item->housecontainer_info->num_slots;
				item->details.num_free_slots = item->housecontainer_info->num_slots;
				item->bag_info->num_slots = item->housecontainer_info->num_slots;
				item->bag_info->weight_reduction = 0;
				
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_house_container`, ID: %u", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadArmor(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, mitigation_low, mitigation_high FROM item_details_armor%s", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 total = 0;
	int32 id = 0;

	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			id = strtoul(row[0], NULL, 0);
			Item* item = master_item_list.GetItem(id);
			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tItem Armor for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\ttype: %i, mit_low: %i, mit_high: %i", ITEM_TYPE_ARMOR, atoi(row[1]), atoi(row[2]));
				item->SetItemType(ITEM_TYPE_ARMOR);
				item->armor_info->mitigation_low = atoi(row[1]);
				item->armor_info->mitigation_high = atoi(row[2]);
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_armor`, ID: %i", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadBags(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, num_slots, weight_reduction FROM item_details_bag%s", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 total = 0;
	int32 id = 0;
	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			id = strtoul(row[0], NULL, 0);
			Item* item = master_item_list.GetItem(id);

			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tItem Bag for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\ttype: %i, slots: %i, wt_red: %i", id, ITEM_TYPE_BAG, atoi(row[1]), atoi(row[2]));
				item->SetItemType(ITEM_TYPE_BAG);
				item->details.num_slots = atoi(row[1]);
				item->details.num_free_slots = item->details.num_slots;
				item->bag_info->num_slots = item->details.num_slots;
				item->bag_info->weight_reduction = atoi(row[2]);
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_bag`, ID: %i", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadFoods(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, type, level, duration, satiation FROM item_details_food%s", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 total = 0;
	int32 id = 0;

	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			id = strtoul(row[0], NULL, 0);
			Item* item = master_item_list.GetItem(id);

			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tItem Food for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\ttype: %i, lvl: %i, dur: %i, sat: %.2f, tier: %i", ITEM_TYPE_FOOD, atoi(row[1]), atoi(row[2]), atof(row[3]), atoi(row[4]));
				item->SetItemType(ITEM_TYPE_FOOD);
				item->food_info->type = atoi(row[1]);
				item->food_info->level = atoi(row[2]);
				item->food_info->duration = atof(row[3]);
				item->food_info->satiation = atoi(row[4]);
				item->details.tier = atoi(row[4]);
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_food`, ID: %i", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadRangeWeapons(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, dmg_low, dmg_high, dmg_mastery_low, dmg_mastery_high, dmg_base_low, dmg_base_high, delay, damage_rating, range_low, range_high, damage_type FROM item_details_range%s", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 total = 0;
	int32 id = 0;

	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			id = strtoul(row[0], NULL, 0);
			Item* item = master_item_list.GetItem(id);

			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tItem Ranged for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\ttype: %i, %i, %i, %i, %i, %i, %i, %i, %.2f, %i, %i, %i", ITEM_TYPE_RANGED, atoi(row[1]), atoi(row[2]), atoi(row[3]), atoi(row[4]), atoi(row[5]), atoi(row[6]), atoi(row[7]), atof(row[8]), atoi(row[9]), atoi(row[10]), atoi(row[11]));
				item->SetItemType(ITEM_TYPE_RANGED);
				item->ranged_info->weapon_info.damage_low1 = atoi(row[1]);
				item->ranged_info->weapon_info.damage_high1 = atoi(row[2]);
				item->ranged_info->weapon_info.damage_low2 = atoi(row[3]);
				item->ranged_info->weapon_info.damage_high2 = atoi(row[4]);
				item->ranged_info->weapon_info.damage_low3 = atoi(row[5]);
				item->ranged_info->weapon_info.damage_high3 = atoi(row[6]);
				item->ranged_info->weapon_info.delay = atoi(row[7]);
				item->ranged_info->weapon_info.rating = atof(row[8]);
				item->ranged_info->range_low = atoi(row[9]);
				item->ranged_info->range_high = atoi(row[10]);
				item->SetWeaponType(atoi(row[11]));
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_range`, ID: %i", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadThrownWeapons(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, range_bonus, damage_bonus, hit_bonus, damage_type FROM item_details_thrown%s", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 total = 0;
	int32 id = 0;

	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			id = strtoul(row[0], NULL, 0);
			Item* item = master_item_list.GetItem(id);

			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tItem Thrown for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\ttype: %i, %i, %u, %.2f, %u", ITEM_TYPE_THROWN, atoul(row[1]), atoi(row[2]), atof(row[3]), atoul(row[4]));
				item->SetItemType(ITEM_TYPE_THROWN);
				item->thrown_info->range = atoul(row[1]);
				item->thrown_info->damage_modifier = atoul(row[2]);
				item->thrown_info->hit_bonus = atof(row[3]);
				item->thrown_info->damage_type = atoul(row[4]);
				item->SetWeaponType(item->thrown_info->damage_type);
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_details_thrown`, ID: %i", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadWeapons(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, wield_style, dmg_low, dmg_high, dmg_mastery_low, dmg_mastery_high, dmg_base_low, dmg_base_high, delay, damage_rating, damage_type FROM  item_details_weapon%s", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 total = 0;
	int32 id = 0;

	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			id = strtoul(row[0], NULL, 0);
			Item* item = master_item_list.GetItem(id);

			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tItem Weapon for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\ttype: %i, %i, %i, %i, %i, %i, %i, %i, %i, %.2f, %i", ITEM_TYPE_WEAPON, atoi(row[1]), atoi(row[2]), atoi(row[3]), atoi(row[4]), atoi(row[5]), atoi(row[6]), atoi(row[7]), atoi(row[8]), atof(row[9]), atoi(row[10]));
				item->SetItemType(ITEM_TYPE_WEAPON);
				item->weapon_info->wield_type = atoi(row[1]);
				item->weapon_info->damage_low1 = atoi(row[2]);
				item->weapon_info->damage_high1 = atoi(row[3]);
				item->weapon_info->damage_low2 = atoi(row[4]);
				item->weapon_info->damage_high2 = atoi(row[5]);
				item->weapon_info->damage_low3 = atoi(row[6]);
				item->weapon_info->damage_high3 = atoi(row[7]);
				item->weapon_info->delay = atoi(row[8]);
				item->weapon_info->rating = atof(row[9]);
				item->SetWeaponType(atoi(row[10]));
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_weapons`, ID: %i", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadItemAppearances(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string("where item_id = ") + std::to_string(item_id) + " ";
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, equip_type, red, green, blue, highlight_red, highlight_green, highlight_blue FROM item_appearances %sORDER BY item_id asc", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 id = 0;
	Item* item = 0;
	int32 total = 0;

	if(result && mysql_num_rows(result) >0)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			if(id != strtoul(row[0], NULL, 0))
			{
				id = strtoul(row[0], NULL, 0);
				item = master_item_list.GetItem(id);

				if(item)
				{
					LogWrite(ITEM__DEBUG, 5, "Items", "\tItem Appearance for item_id %u", id);
					LogWrite(ITEM__DEBUG, 5, "Items", "\tequip_type: %i, R: %i, G: %i, B: %i, HR: %i, HG: %i, HB: %i", atoi(row[1]), atoi(row[2]), atoi(row[3]), atoi(row[4]), atoi(row[5]), atoi(row[6]), atoi(row[7]));
					item->SetAppearance(atoi(row[1]), atoi(row[2]), atoi(row[3]), atoi(row[4]), atoi(row[5]), atoi(row[6]), atoi(row[7]));
					total++;
				}
				else
					LogWrite(ITEM__ERROR, 0, "Items", "Error Loading item_appearances, ID: %i", id);
			}
		}
	}
	return total;
}

int32 WorldDatabase::LoadItemEffects(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string("where item_id = ") + std::to_string(item_id) + " ";
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, effect, percentage, bullet FROM item_effects %sORDER BY item_id, id", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 id = 0;
	Item* item = 0;
	int32 total = 0;

	if(result && mysql_num_rows(result) >0)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			if(id != atoul(row[0]))
			{
				id = atoul(row[0]);
				item = master_item_list.GetItem(id);
			}

			if(item && row[1])
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tItem Effects for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\tEffect: '%s', Percent: %i, Sub: %i", row[1], atoi(row[2]), atoi(row[3]));
				item->AddEffect(row[1], atoi(row[2]), atoi(row[3]));
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error Loading item_effects, ID: %i", id);
		}
	}
	return total;
}
int32 WorldDatabase::LoadBookPages(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string("where item_id = ") + std::to_string(item_id) + " ";
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, page, page_text, page_text_valign, page_text_halign FROM item_details_book_pages %sORDER BY item_id, id", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 id = 0;
	Item* item = 0;
	int32 total = 0;

	if (result && mysql_num_rows(result) > 0)
	{
		while (result && (row = mysql_fetch_row(result)))
		{
			if (id != atoul(row[0]))
			{
				id = atoul(row[0]);
				item = master_item_list.GetItem(id);
			}

			if (item && row[1])
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tBook Pages for item_id %u", id);
				//LogWrite(ITEM__DEBUG, 5, "Items", "\tPages: '%s', Percent: %i, Sub: %i", row[1], atoi(row[2]), atoi(row[3]));
				item->AddBookPage(atoi(row[1]), row[2], atoi(row[3]), atoi(row[4]));
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error Loading item_details_book_pages, ID: %i", id);
		}
	}
	return total;
}
int32 WorldDatabase::LoadItemLevelOverride(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string("where item_id = ") + std::to_string(item_id) + " ";
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, adventure_class_id, tradeskill_class_id, level FROM item_levels_override %sORDER BY item_id asc", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 id = 0;
	Item* item = 0;
	int32 total = 0;

	if(result && mysql_num_rows(result) >0)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			if(id != strtoul(row[0], NULL, 0))
			{
				id = strtoul(row[0], NULL, 0);
				item = master_item_list.GetItem(id);
			}

			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tLevel Override for item_id %u", id);
				LogWrite(ITEM__DEBUG, 5, "Items", "\tAdv: %i, TS: %i, Lvl: %i", atoi(row[1]), atoi(row[2]), atoi(row[3]));
				item->AddLevelOverride(atoi(row[1]), atoi(row[2]), atoi(row[3]));
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_levels_override`, ID: %i", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadItemStats(int32 item_id)
{
	Query query;
	MYSQL_ROW row;
	
	std::string select_query_addition = std::string("where item_id = ") + std::to_string(item_id) + " ";
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, type, subtype, iValue, fValue, sValue, level FROM item_mod_stats %sORDER BY stats_order asc", (item_id == 0) ? "" : select_query_addition.c_str());
	int32 id = 0;
	Item* item = 0;
	int32 total = 0;

	if(result && mysql_num_rows(result) >0)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			if(id != strtoul(row[0], NULL, 0))
			{
				id = strtoul(row[0], NULL, 0);
				item = master_item_list.GetItem(id);
			}

			if(item)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "\tItem Stats for item_id %u", id);

				float fValue = 0.0f;
				if(row[3])
					fValue = atof(row[3]);
				else if(row[4])
					fValue = atof(row[4]);

				//LogWrite(ITEM__DEBUG, 5, "Items", "\ttype: %i, sub: %i, val: %.2f, name: %s", atoi(row[1]), atoi(row[2]), atof(row[3]), row[4]);
				item->AddStat(atoi(row[1]), atoi(row[2]), fValue, atoul(row[6]), row[5]);
				total++;
			}
			else
				LogWrite(ITEM__ERROR, 0, "Items", "Error loading `item_stats`, ID: %i", id);
		}
	}
	return total;
}

int32 WorldDatabase::LoadItemModStrings(int32 item_id)
{
	DatabaseResult result;

	int32 id = 0;
	Item* item = 0;
	int32 total = 0;

	std::string select_query_addition = std::string(" where item_id = ") + std::to_string(item_id);
	if( !database_new.Select(&result, "SELECT * FROM item_mod_strings%s", (item_id == 0) ? "" : select_query_addition.c_str()) ) {
		LogWrite(ITEM__ERROR, 0, "Items", "Cannot load WorldDatabase::LoadItemModStrings in %s, line: %i", __FUNCTION__, __LINE__);
		return 0;
	}
	else {
		while( result.Next() )
		{
			int32 item_id = result.GetInt32Str("item_id");
			if(id != item_id)
			{
				item = master_item_list.GetItem(item_id);
				id = item_id;
			}
			
			const char* modName = result.GetFieldValueStr("mod");
			if(item && modName)
			{
				Item::ItemStatString* stat_ = new Item::ItemStatString;
				stat_->stat_string.data = string(modName);
				stat_->stat_string.size = stat_->stat_string.data.length();
				item->AddStatString(stat_);
			}
			total++;
		}
	}
	return total;
}

void WorldDatabase::LoadBrokerItemStats()
{
	DatabaseResult result;

	if( !database_new.Select(&result, "SELECT * FROM broker_item_map") ) {
		LogWrite(ITEM__ERROR, 0, "Items", "Cannot load WorldDatabase::LoadBrokerItemStats in %s, line: %i", __FUNCTION__, __LINE__);
	}
	else {
		while( result.Next() )
		{
			int32 version_range1 = result.GetInt32Str("version_range1");
			int32 version_range2 = result.GetInt32Str("version_range2");
			int64 client_bitmask = result.GetInt64Str("client_bitmask");
			int64 server_bitmask = result.GetInt64Str("server_bitmask");
			master_item_list.AddBrokerItemMapRange(version_range1, version_range2, client_bitmask, server_bitmask);
		}
	}
}
void WorldDatabase::ReloadItemList(int32 item_id) 
{
	LogWrite(ITEM__DEBUG, 0, "Items", "Unloading Item List...");
	if(!item_id) {
		master_item_list.RemoveAll();
	}
	LoadItemList(item_id);
}

void WorldDatabase::LoadItemList(int32 item_id)
{
	DatabaseResult result;

	int32 t_now = Timer::GetUnixTimeStamp();
	int32 total = 0;
	int32 normal_items = 0;
	string item_type;
	std::string select_query_addition = std::string(" where id = ") + std::to_string(item_id);
	if( !database_new.Select(&result, "SELECT * FROM items%s", (item_id == 0) ? "" : select_query_addition.c_str()) )
		LogWrite(ITEM__ERROR, 0, "Items", "Cannot load items in %s, line: %i", __FUNCTION__, __LINE__);
	else
	{
		while( result.Next() )
		{
			item_type = result.GetStringStr("item_type");
			LogWrite(ITEM__DEBUG, 5, "Items", "\tLoading: %s (ID: %i, Type: %s)...", result.GetStringStr("name"), result.GetInt32Str("id"), item_type.c_str());

			Item* item = new Item;
			LoadDataFromRow(&result, item);
			master_item_list.AddItem(item);

			if( strcmp(item_type.c_str(), "Normal") == 0 )
			{
				item->SetItemType(ITEM_TYPE_NORMAL);
				normal_items++;
			}
			total++;
		}
	}

	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Normal Items", normal_items);
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Baubles", LoadBaubles(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Bags", LoadBags(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Books", LoadBooks(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Item Sets", LoadItemsets(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u House Items", LoadHouseItem(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Food Items", LoadFoods(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Weapons", LoadWeapons(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Ranged Weapons", LoadRangeWeapons(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Thrown Weapons", LoadThrownWeapons(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Armor Pieces", LoadArmor(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Shields", LoadShields(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Skill Items", LoadSkillItems(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Adornment Items", LoadAdornments(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Recipe Book Items", LoadRecipeBookItems(item_id));
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u House Containers", LoadHouseContainers(item_id)); // must be called after LoadHouseItem

	LogWrite(ITEM__DEBUG, 0, "Items", "Loading Item Appearances...");
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Item Appearances", LoadItemAppearances(item_id));

	LogWrite(ITEM__DEBUG, 0, "Items", "Loading Item Stats...");
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Item Stats", LoadItemStats(item_id));

	LogWrite(ITEM__DEBUG, 0, "Items", "Loading Item Stats Mods (Strings)...");
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Item Stats", LoadItemModStrings(item_id));

	LogWrite(ITEM__DEBUG, 0, "Items", "Loading Item Effects...");
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Item Effects", LoadItemEffects(item_id));

	LogWrite(ITEM__DEBUG, 0, "Items", "Loading Book Pages...");
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Book Pages", LoadBookPages(item_id));

	LogWrite(ITEM__DEBUG, 0, "Items", "Loading Item Level Overrides...");
	LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded %u Item Level Overrides", LoadItemLevelOverride(item_id));
	
	if(!item_id) {
		LoadBrokerItemStats();
		LogWrite(ITEM__DEBUG, 0, "Items", "\tLoaded Broker Item Stat Map Versioning");
	}

	LogWrite(ITEM__INFO, 0, "Items", "Loaded %u Total Item%s (took %u seconds)", total, ( total == 1 ) ? "" : "s", Timer::GetUnixTimeStamp() - t_now);
}

int64 WorldDatabase::LoadNextUniqueItemID()
{
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT NEXT VALUE FOR seq_character_items AS next_id");

	if(result && (row = mysql_fetch_row(result)))
	{
		if(row[0])
		{
			int64 max_ = strtoull(row[0], NULL, 0);
			LogWrite(ITEM__DEBUG, 0, "Items", "%s: max(id): %u", __FUNCTION__, max_);
			return max_;
		}
		else 
			return 0;
	}
	else if(!result)
		LogWrite(ITEM__ERROR, 0, "Items", "%s: Unable to load next unique item ID.", __FUNCTION__);

	return 0;
}

void WorldDatabase::ResetNextUniqueItemID()
{
	Query query;
	Query query2;
	MYSQL_ROW row;
	MYSQL_ROW row2;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT next_not_cached_value FROM seq_character_items");
	MYSQL_RES* result2 = query2.RunQuery2(Q_SELECT, "SELECT COALESCE(MAX(id),0) + 1 FROM character_items");
	if(result && (row = mysql_fetch_row(result)) && result2 && (row2 = mysql_fetch_row(result2)))
	{
		if(row[0] && row2[0])
		{
			int64 max_cur = strtoull(row[0], NULL, 0);
			int64 max_expected = strtoull(row2[0], NULL, 0);
			string update_item = string("ALTER SEQUENCE seq_character_items RESTART WITH %llu");
			if(max_cur < max_expected)
				query.AddQueryAsync(0, this, Q_UPDATE, update_item.c_str(), max_expected);
			
			LogWrite(ITEM__DEBUG, 0, "Items", "%s: max(current): %u max(expected): %u", __FUNCTION__, max_cur, max_expected);
		}
	}
	else if(!result)
		LogWrite(ITEM__ERROR, 0, "Items", "%s: Unable to reset next unique item ID.", __FUNCTION__);
}

void WorldDatabase::SaveItems(Client* client) 
{
	LogWrite(ITEM__DEBUG, 3, "Items", "Save Items for Player %i", client->GetCharacterID());

	map<int32, Item*>* items = client->GetPlayer()->GetItemList();
	map<int32, Item*>::iterator item_iter;
	Item* item = 0;

	for(item_iter = items->begin(); item_iter != items->end(); item_iter++)
	{
		item = item_iter->second;

		if(item) {
			if(item->CheckFlag(TEMPORARY)) {
					item->save_needed = true; // we need to keep updating the timestamp so it doesn't expire
			}
			if(item->needs_deletion || (client->IsZoning() && item->CheckFlag(NO_ZONE))) {
				DeleteItem(client->GetCharacterID(), item, 0);
				client->GetPlayer()->item_list.DestroyItem(item->details.index);
				if(!client->IsZoning()) {
					client->QueuePacket(client->GetPlayer()->SendInventoryUpdate(client->GetVersion()));
				}
			}
			else if(item->save_needed)
			{
				LogWrite(ITEM__DEBUG, 5, "Items", "SaveItems: Acct: %u, Char: %u, Item: %u, NOT-EQUIPPED", client->GetAccountID(), client->GetCharacterID(), item);
				SaveItem(client->GetAccountID(), client->GetCharacterID(), item, "NOT-EQUIPPED");
				item->save_needed = false;
			}
		}
	}
	safe_delete(items);

	vector<Item*>* equipped_list = client->GetPlayer()->GetEquippedItemList();

	for(int32 i=0;i<equipped_list->size();i++)
	{
		item = equipped_list->at(i);

		if(item)
		{
			if(item->CheckFlag(TEMPORARY)) {
					item->save_needed = true; // we need to keep updating the timestamp so it doesn't expire
			}
			if(item->needs_deletion || (client->IsZoning() && item->CheckFlag(NO_ZONE))) {
				DeleteItem(client->GetCharacterID(), item, 0);
				client->GetPlayer()->item_list.DestroyItem(item->details.index);
				if(!client->IsZoning()) {
					client->QueuePacket(client->GetPlayer()->SendInventoryUpdate(client->GetVersion()));
				}
			}
			else if(item->save_needed) {
				if(item->details.appearance_type)
					SaveItem(client->GetAccountID(), client->GetCharacterID(), item, "APPEARANCE");
				else
					SaveItem(client->GetAccountID(), client->GetCharacterID(), item, "EQUIPPED");
			}
			item->save_needed = false;
		}
	}
	safe_delete(equipped_list);

	
	vector<Item*>* appearance_equipped_list = client->GetPlayer()->GetAppearanceEquippedItemList();

	for(int32 i=0;i<appearance_equipped_list->size();i++)
	{
		item = appearance_equipped_list->at(i);

		if(item)
		{
			if(item->CheckFlag(TEMPORARY)) {
					item->save_needed = true; // we need to keep updating the timestamp so it doesn't expire
			}
			if(item->needs_deletion || (client->IsZoning() && item->CheckFlag(NO_ZONE))) {
				DeleteItem(client->GetCharacterID(), item, 0);
				client->GetPlayer()->item_list.DestroyItem(item->details.index);
				if(!client->IsZoning()) {
					client->QueuePacket(client->GetPlayer()->SendInventoryUpdate(client->GetVersion()));
				}
			}
			else if(item->save_needed) {
				SaveItem(client->GetAccountID(), client->GetCharacterID(), item, "APPEARANCE");
				item->save_needed = false;
			}
		}
	}
	safe_delete(appearance_equipped_list);

	vector<Item*>* overflow = client->GetPlayer()->item_list.GetOverflowItemList();
	for (int32 i = 0; i < overflow->size(); i++){
		item = overflow->at(i);
		if (item) {
			if(item->CheckFlag(TEMPORARY)) {
					item->save_needed = true; // we need to keep updating the timestamp so it doesn't expire
			}
			if(item->needs_deletion || (client->IsZoning() && item->CheckFlag(NO_ZONE))) {
				DeleteItem(client->GetCharacterID(), item, 0);
				client->GetPlayer()->item_list.DestroyItem(item->details.index);
				if(!client->IsZoning()) {
					client->QueuePacket(client->GetPlayer()->SendInventoryUpdate(client->GetVersion()));
				}
			}
			else {
				sint16 slot = item->details.slot_id;
				item->details.slot_id = i;
				SaveItem(client->GetAccountID(), client->GetCharacterID(), item, "NOT-EQUIPPED");
				item->details.slot_id = slot;
			}
		}
	}
	safe_delete(overflow);
}

void WorldDatabase::SaveItem(int32 account_id, int32 char_id, Item* item, const char* type) 
{
	LogWrite(ITEM__DEBUG, 1, "Items", "Saving ItemID: %u (Type: %s) for account: %u, player: %u", item->details.item_id, type, account_id, char_id);

	Query query;
	string update_item = string("REPLACE INTO character_items (id, type, char_id, slot, equip_slot, item_id, creator,adorn0,adorn1,adorn2, condition_, attuned, bag_id, count, max_sell_value, no_sale, account_id, login_checksum) VALUES (%u, '%s', %u, %i, %i, %u, '%s', %i, %i, %i, %i, %i, %i, %i, %u, %u, %u, 0)");
	query.AddQueryAsync(char_id, this, Q_REPLACE, update_item.c_str(), item->details.unique_id, type, char_id, item->details.slot_id, item->details.equip_slot_id, item->details.item_id,
		getSafeEscapeString(item->creator.c_str()).c_str(),item->adorn0,item->adorn1,item->adorn2, item->generic_info.condition, item->CheckFlag(ATTUNED) ? 1 : 0, item->details.inv_slot_id, item->details.count, item->GetMaxSellValue(), item->no_sale, account_id);
	if(item->CheckFlag2(HEIRLOOM)) {
		std::map<int32, bool>::iterator itr;
		for(itr = item->grouped_char_ids.begin(); itr != item->grouped_char_ids.end(); itr++) {
			string addmembers_query = string("REPLACE INTO character_items_group_members (unique_id, character_id) VALUES (%u, %u)");
			query.AddQueryAsync(char_id, this, Q_REPLACE, addmembers_query.c_str(), item->details.unique_id, itr->first);
		}
	}
}

void WorldDatabase::DeleteItem(int32 char_id, Item* item, const char* type) 
{
	string delete_item;

	if(type)
	{
		LogWrite(ITEM__DEBUG, 1, "Items", "Deleting item_id %u (Type: %s) for player %u", item->details.item_id, type, char_id);

		delete_item = string("DELETE FROM character_items WHERE char_id = %u AND (id = %u OR bag_id = %u) AND type='%s'");
		Query query;
		query.RunQuery2(Q_DELETE, delete_item.c_str(), char_id, item->details.unique_id, item->details.unique_id, type);
	}
	else
	{
		LogWrite(ITEM__DEBUG, 0, "Items", "Deleting item_id %u for player %u", item->details.item_id, char_id);

		delete_item = string("DELETE FROM character_items WHERE char_id = %u AND (id = %u OR bag_id = %u)");
		Query query2;
		query2.RunQuery2(Q_DELETE, delete_item.c_str(), char_id, item->details.unique_id, item->details.unique_id);
	}
	
	if(item->CheckFlag2(HEIRLOOM)) {
		delete_item = string("DELETE FROM character_items_group_members WHERE unique_id = %u");
		Query query3;
		query3.RunQuery2(Q_DELETE, delete_item.c_str(), item->details.unique_id);
	}
}

void WorldDatabase::LoadCharacterItemList(int32 account_id, int32 char_id, Player* player, int16 version) 
{
	LogWrite(ITEM__DEBUG, 0, "Items", "Loading items for character '%s' (%u)", player->GetName(), char_id);

	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT type, id, slot, equip_slot, item_id, creator,adorn0,adorn1,adorn2, condition_, attuned, bag_id, count, max_sell_value, no_sale, UNIX_TIMESTAMP(last_saved), UNIX_TIMESTAMP(created) FROM character_items where char_id = %u or (bag_id = -4 and account_id = %u) ORDER BY bag_id, slot asc", char_id, account_id);

	if(result)
	{
		bool ret = true;

		while(result && (row = mysql_fetch_row(result)))
		{
			LogWrite(ITEM__DEBUG, 5, "Items", "Loading character item: %u, slot: %i", strtoul(row[1], NULL, 0), atoi(row[2]));
			Item* master_item = master_item_list.GetItem(strtoul(row[4], NULL, 0));
			if(master_item)
			{
				Item* item = new Item(master_item);
				int32 xxx = 0;
				if(master_item->recipebook_info)
					item->recipebook_info->recipe_id = master_item->recipebook_info->recipe_id;
				item->details.unique_id = strtoul(row[1], NULL, 0);
				item->details.slot_id = atoi(row[2]);
				if(item->IsBag()) {
					item->details.equip_slot_id = atoi(row[3]);
				}

				if(item->details.num_slots > 0)
					item->details.bag_id = item->details.unique_id;

				item->save_needed = false;

				// we need the items basics (unique id slot id bag id) to continue this temporary check
				if(item->CheckFlag(TEMPORARY)) {
					std::time_t last_saved =  static_cast<std::time_t>(atoul(row[15]));
					double timeInSeconds = std::difftime(std::time(nullptr), last_saved);
						LogWrite(ITEM__INFO, 0, "Items", "Character ID %u has a temporary item %s time in seconds %f last saved.", char_id, item->name.c_str(), timeInSeconds);
					if(timeInSeconds >= rule_manager.GetGlobalRule(R_Player, TemporaryItemLogoutTime)->GetFloat()) {
						DeleteItem(char_id, item, 0);
						LogWrite(ITEM__INFO, 0, "Items", "\tCharacter ID %u had a temporary item %s which was removed due to time limit.", char_id, item->name.c_str());
						lua_interface->SetLuaUserDataStale(item);
						safe_delete(item);
						continue;
					}
				}
				
				if(row[5])
					item->creator = string(row[5]);//creator
				item->adorn0 = atoi(row[6]); //adorn0
				item->adorn1 = atoi(row[7]); //adorn1
				item->adorn2 = atoi(row[8]); //adorn2
				item->generic_info.condition = atoi(row[9]); //condition

				if(row[10] && atoi(row[10])>0) //attuned
				{
					if(item->CheckFlag(ATTUNEABLE))
						item->generic_info.item_flags -= ATTUNEABLE;

					if(!item->CheckFlag(NO_TRADE))
						item->generic_info.item_flags += NO_TRADE;

					item->generic_info.item_flags += ATTUNED;
				}
				
				if(item->CheckFlag2(HEIRLOOM)) {
					MYSQL_ROW row2;
					MYSQL_RES* result2 = query.RunQuery2(Q_SELECT, "SELECT character_id from character_items_group_members where unique_id = %u", item->details.unique_id);

					if(result2)
					{
						bool ret = true;

						while(result2 && (row2 = mysql_fetch_row(result2)))
						{
							item->grouped_char_ids.insert(std::make_pair(atoul(row2[0]),true));
						}
					}
				}

				item->details.inv_slot_id = atol(row[11]); //bag_id
				item->details.new_item = false;
				item->details.new_index = 0;
				item->details.count = atoi(row[12]); //count
				item->SetMaxSellValue(atoul(row[13])); //max sell value
				item->no_sale = (atoul(row[14]) == 1);
				item->details.appearance_type = 0;

				// position 14 is used for the last_saved timestamp (primarily for checking temporary items on login)
				item->created = static_cast<std::time_t>(atoul(row[16]));
				
				if(strncasecmp(row[0], "EQUIPPED", 8)==0)
					ret = player->GetEquipmentList()->AddItem(item->details.slot_id, item);
				else if (strncasecmp(row[0], "APPEARANCE", 10) == 0)
				{
					item->details.appearance_type = 1;
					ret = player->GetAppearanceEquipmentList()->AddItem(item->details.slot_id, item);
				}
				else {
					if (version < 1209 && item->details.count > 255 && item->stack_count > 1) {
						int stacks = item->details.count / 255;
						int8 remainder = item->details.count % 255;
						item->details.count = remainder;
						
						if (item->details.inv_slot_id == InventorySlotType::OVERFLOW)
							player->item_list.AddOverflowItem(item);
						else {
								if(!player->item_list.AddItem(item))
									item = nullptr;
						}

						if(item) {
							for (int stack = 1; stack <= stacks; stack++) {
								item->details.count = 255;
								item->details.inv_slot_id = -2;
								player->item_list.AddOverflowItem(item);
							}
						}
					}
					else {
						if (item->details.inv_slot_id == InventorySlotType::OVERFLOW)
							player->item_list.AddOverflowItem(item);
						else
							player->item_list.AddItem(item);
					}
					
					if(item->details.equip_slot_id) {
						player->GetEquipmentList()->AddItem(item->details.equip_slot_id, item);
					}
				}
			}
			else
				ret = false;
		}

		if(!ret)
			LogWrite(ITEM__ERROR, 0, "Items", "%s: Error Loading item(s) for Char ID: %u (%s)", __FUNCTION__, char_id, player->GetName());
	}
}

