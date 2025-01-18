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

#include <math.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ios>
#include <assert.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>


#include "WorldDatabase.h"
#include "../common/debug.h"
#include "../common/packet_dump.h"
#include "../common/GlobalHeaders.h"
#include "Items/Items.h"
#include "Factions.h"
#include "World.h"
#include "Variables.h"
#include "VisualStates.h"
#include "Appearances.h"
#include "Skills.h"
#include "Quests.h"
#include "LuaInterface.h"
#include "classes.h"
#include "../common/Log.h"
#include "Rules/Rules.h"
#include "Titles.h"
#include "Languages.h"
#include "Traits/Traits.h"
#include "ClientPacketFunctions.h"
#include "Zone/ChestTrap.h"
#include "../common/version.h"
#include "SpellProcess.h"
#include "races.h"
#include "Web/PeerManager.h"

extern Classes classes;
extern Commands commands;
extern MasterTitlesList master_titles_list;
extern MasterItemList master_item_list;
extern MasterSpellList master_spell_list;
extern MasterTraitList master_trait_list;
extern MasterFactionList master_faction_list;
extern World world;
extern Variables variables;
extern VisualStates visual_states;
extern Appearances master_appearance_list;
extern MasterSkillList master_skill_list;
extern MasterQuestList master_quest_list;
extern LuaInterface* lua_interface;
extern ZoneList zone_list;
extern GuildList guild_list;
extern MasterCollectionList master_collection_list;
extern RuleManager rule_manager;
extern MasterLanguagesList master_languages_list;
extern ChestTrapList chest_trap_list;
extern PeerManager peer_manager;

//devn00b: Fix for linux builds since we dont use stricmp we use strcasecmp
#if defined(__GNUC__)
#define stricmp strcasecmp
#define strnicmp strncasecmp
#include <sys/stat.h>
#endif

WorldDatabase::WorldDatabase(){
}

WorldDatabase::~WorldDatabase(){
}

bool WorldDatabase::ConnectNewDatabase() {
	/*
	TESTS

	database_new.Connect();
	DatabaseResult result;
	database_new.Select(&result, "select name from characters where id=1");
	if (result.Next()) {
		printf("'%s'\n", result.GetStringStr("name"));
		printf("'%s'\n", result.GetStringStr("nameBAD"));
		printf("'%s'\n", result.GetString(3));
	}
	return true;
	*/

	return database_new.Connect();
}

void WorldDatabase::PingNewDB()
{
	database_new.PingNewDB();
}

void WorldDatabase::DeleteBuyBack(int32 char_id, int32 item_id, int16 quantity, int32 price) {
	LogWrite(MERCHANT__DEBUG, 0, "Merchant", "Deleting Buyback - Player: %u, Item ID: %u, Qty: %i, Price: %u", char_id, item_id, quantity, price);

	Query query;
	query.RunQuery2(Q_DELETE, "DELETE FROM character_buyback WHERE char_id = %u AND item_id = %u AND quantity = %i AND price = %u", char_id, item_id, quantity, price);
}

void WorldDatabase::LoadBuyBacks(Client* client) {
	LogWrite(MERCHANT__DEBUG, 0, "Merchant", "Loading Buyback - Player: %u", client->GetCharacterID());

	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, item_id, quantity, price FROM character_buyback where char_id = %u ORDER BY id desc limit 10", client->GetCharacterID());
	int8 count = 0;
	int32 last_id = 0;
	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			LogWrite(MERCHANT__DEBUG, 5, "Merchant", "AddBuyBack: item: %u, qty: %i, price: %u", atoul(row[1]), atoi(row[2]), atoul(row[3]));
			last_id = atoul(row[0]);
			client->AddBuyBack(last_id, atoul(row[1]), atoi(row[2]), atoul(row[3]), false);
			count++;
		}
		if(count >= 10)
		{
			LogWrite(MERCHANT__DEBUG, 0, "Merchant", "Deleting excess Buyback from Player: %u", client->GetCharacterID());
			Query query2;
			query2.RunQuery2(Q_DELETE, "DELETE FROM character_buyback WHERE char_id = %u AND id < %u", client->GetCharacterID(), last_id);
		}
	}
}

void WorldDatabase::SaveBuyBacks(Client* client) 
{
	LogWrite(MERCHANT__DEBUG, 3, "Merchant", "Saving Buybacks - Player: %u", client->GetCharacterID());

	deque<BuyBackItem*>* buybacks = client->GetBuyBacks();

	if(buybacks && buybacks->size() > 0)
	{
		BuyBackItem* item = 0;
		deque<BuyBackItem*>::iterator itr;

		for(itr = buybacks->begin(); itr != buybacks->end(); itr++)
		{
			item = *itr;

			if(item && item->save_needed)
			{
				LogWrite(MERCHANT__DEBUG, 5, "Merchant", "SaveBuyBack: char: %u, item: %u, qty: %i, price: %u", client->GetCharacterID(), item->item_id, item->quantity, item->price);
				SaveBuyBack(client->GetCharacterID(), item->item_id, item->quantity, item->price);
				item->save_needed = false;
			}
		}
	}
}

void WorldDatabase::SaveBuyBack(int32 char_id, int32 item_id, int16 quantity, int32 price) 
{
	LogWrite(MERCHANT__DEBUG, 3, "Merchant", "Saving Buyback - Player: %u, Item ID: %u, Qty: %i, Price: %u", char_id, item_id, quantity, price);

	Query query;
	string insert = string("INSERT INTO character_buyback (char_id, item_id, quantity, price) VALUES (%u, %u, %i, %u) ");
	query.AddQueryAsync(char_id, this, Q_INSERT, insert.c_str(), char_id, item_id, quantity, price);
}

int32 WorldDatabase::LoadCharacterSpells(int32 char_id, Player* player)
{
	LogWrite(SPELL__DEBUG, 0, "Spells", "Loading Character Spells for player %s...", player->GetName());

	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT spell_id, tier, knowledge_slot, spell_book_type, linked_timer_id  FROM character_spells, spells where character_spells.spell_id = spells.id and character_spells.char_id = %u ORDER BY spell_id, tier desc", char_id);
	int32 old_spell_id = 0;
	int32 new_spell_id = 0;
	int32 count = 0;

	if(result && mysql_num_rows(result) >0)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			count++;
			new_spell_id = atoul(row[0]);

			if(new_spell_id == old_spell_id)
				continue;

			old_spell_id = new_spell_id;

			LogWrite(SPELL__DEBUG, 5, "Spells", "\tLoading SpellID: %u, tier: %i, slot: %i, type: %u linked_timer_id: %u", new_spell_id, atoi(row[1]), atoi(row[2]), atoul(row[3]), atoul(row[4]));

			int8 tier = atoi(row[1]);

			if (player->HasSpell(new_spell_id, tier, true))
				continue;

			player->AddSpellBookEntry(new_spell_id, tier, atoi(row[2]), atoul(row[3]), atoul(row[4]));
		}
	}

	return count;
}

void WorldDatabase::SavePlayerSpells(Client* client)
{
	if(!client || client->GetCharacterID() < 1)
		return;

	LogWrite(SPELL__DEBUG, 3, "Spells", "Saving Spell(s) for Player: '%s'", client->GetPlayer()->GetName());
	vector<SpellBookEntry*>* spells = client->GetPlayer()->GetSpellsSaveNeeded();

	if(spells)
	{
		vector<SpellBookEntry*>::iterator itr;
		SpellBookEntry* spell = 0;

		for(itr = spells->begin(); itr != spells->end(); itr++)
		{
			spell = *itr;
			Query query;
			LogWrite(SPELL__DEBUG, 5, "Spells", "\tSaving SpellID: %u, tier: %i, slot: %i", spell->spell_id, spell->tier, spell->slot);
			query.AddQueryAsync(client->GetCharacterID(), this, Q_INSERT, "INSERT INTO character_spells (char_id, spell_id, tier) SELECT %u, %u, %i ON DUPLICATE KEY UPDATE tier = %i",
				client->GetPlayer()->GetCharacterID(), spell->spell_id, spell->tier, spell->tier);
			spell->save_needed = false;
		}
		safe_delete(spells);
	}
}

int32 WorldDatabase::LoadCharacterSkills(int32 char_id, Player* player)
{
	Query query;
	MYSQL_ROW row;
	int32 count = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT skill_id, current_val, max_val FROM character_skills, skills where character_skills.skill_id = skills.id and character_skills.char_id = %u", char_id);

	if(result && mysql_num_rows(result) >0)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			count++;
			LogWrite(SKILL__DEBUG, 5, "Skills", "Loading SkillID: %u, cur_val: %i, max_val: %l", strtoul(row[0], NULL, 0), atoi(row[1]), atoi(row[2]));
			player->AddSkill(strtoul(row[0], NULL, 0), atoi(row[1]), atoi(row[2]));
		}
	}
	return count;
}

void WorldDatabase::DeleteCharacterSkill(int32 char_id, Skill* skill) 
{
	if (char_id > 0 && skill) 
	{
		LogWrite(SKILL__DEBUG, 0, "Skills", "Deleting Skill '%s' (%u) from char_id: %u", skill->name.data.c_str(), skill->skill_id, char_id);
		Query query;
		query.RunQuery2(Q_DELETE, "DELETE FROM `character_skills` WHERE `char_id`=%u AND `skill_id`=%u", char_id, skill->skill_id);
	}
}

int32 WorldDatabase::LoadSkills() 
{
	int32 total = 0;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, short_name, name, description, skill_type, display FROM skills");

	if(result)
	{
		if (mysql_num_rows(result) >0)
		{
			Skill* skill = 0;

			while(result && (row = mysql_fetch_row(result)))
			{
				skill = new Skill();
				skill->skill_id = strtoul(row[0], NULL, 0);
				skill->short_name.data = string(row[1]);
				skill->short_name.size = skill->short_name.data.length();
				skill->name.data = string(row[2]);
				skill->name.size = skill->name.data.length();
				skill->description.data = string(row[3]);
				skill->description.size = skill->description.data.length();
				skill->skill_type = strtoul(row[4], NULL, 0);
				//these two need to be converted to the correct numbers
				if(skill->skill_type == 13)
					skill->skill_type = SKILL_TYPE_LANGUAGE;
				else if(skill->skill_type == 12)
					skill->skill_type = SKILL_TYPE_GENERAL;

				skill->display = atoi(row[5]);
				master_skill_list.AddSkill(skill);
				total++;
				LogWrite(SKILL__DEBUG, 5, "Skill", "---Loading Skill: %s (%u)", skill->name.data.c_str(), skill->skill_id);
				LogWrite(SKILL__DEBUG, 7, "Skill", "---short_name: %s, type: %i, display: %i", skill->short_name.data.c_str(), skill->skill_type, skill->display);
			}
		}
	}
	LogWrite(SKILL__DEBUG, 3, "Skill", "--Loaded %u Skill(s)", total);
	return total;
}

map<int8, vector<MacroData*> >*	WorldDatabase::LoadCharacterMacros(int32 char_id)
{
	Query query;
	MYSQL_ROW row;
	int32 total = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT macro_number, macro_name, macro_icon, macro_text FROM character_macros where char_id = %u ORDER BY macro_number, id", char_id);

	if(result && mysql_num_rows(result) >0)
	{
		map<int8, vector<MacroData*> >* macros = new map<int8, vector<MacroData*> >;

		while(result && (row = mysql_fetch_row(result)))
		{
			MacroData* data = new MacroData;
			data->name = row[1];
			data->icon = atoi(row[2]);
			data->text = row[3];
			(*macros)[atoi(row[0])].push_back(data);
			total++;
			LogWrite(PLAYER__DEBUG, 5, "Player", "\tLoading macro: %i. %s for player: %u", atoi(row[0]), row[1], char_id);
		}
		LogWrite(PLAYER__DEBUG, 0, "Player", "\tLoaded %u macro%s", total, total == 1 ? "" : "s");
		return macros;
	}
	return 0;
}

void WorldDatabase::UpdateCharacterMacro(int32 char_id, int8 number, const char* name, int16 icon, vector<string>* updates)
{
	LogWrite(PLAYER__DEBUG, 0, "Player", "Update player id %u macro: %i", char_id, number);

	Query query;
	Query query2;
	query.RunQuery2(Q_DELETE, "delete FROM character_macros where char_id = %u and macro_number = %i", char_id, number);

	if(name && updates && updates->size() > 0)
	{
		for(int8 i=0;i<updates->size();i++)
		{
			query2.RunQuery2(Q_INSERT, "insert into character_macros (char_id, macro_number, macro_name, macro_icon, macro_text) values(%u, %i, '%s', %i, '%s')", char_id, number, getSafeEscapeString(name).c_str(), icon, getSafeEscapeString(updates->at(i).c_str()).c_str());
			LogWrite(PLAYER__DEBUG, 5, "Player", "\tAdding macro: %s, %s (Player: %u)", name, updates->at(i).c_str(), char_id);
		}
	}
}

//we use our timestamp just in case db is on another server, otherwise times might be off
void WorldDatabase::UpdateVitality(int32 timestamp, float amount){ 
	Query query, query2, query3;

	LogWrite(PLAYER__DEBUG, 3, "Player", "Reset Vitality > 100: %f", amount);
	query.RunQuery2(Q_UPDATE, "update character_details set xp_vitality=100 where (xp_vitality + %f) > 100", amount);

	LogWrite(PLAYER__DEBUG, 3, "Player", "Update Vitality <= 100: %f", amount);
	query2.RunQuery2(Q_UPDATE, "update character_details set xp_vitality=(xp_vitality+%f) where (xp_vitality + %f) <= 100", amount, amount);

	LogWrite(PLAYER__DEBUG, 3, "Player", "Update Vitality Timer: %u", timestamp);
	query3.RunQuery2(Q_UPDATE, "update variables set variable_value=%u where variable_name='vitalitytimer'", timestamp);
}

void WorldDatabase::SaveVariable(const char* name, const char* value, const char* comment){

	LogWrite(WORLD__DEBUG, 0, "Variables", "Saving Variable: %s = %s", name, value);
	Query query;
	if(comment){
		query.RunQuery2(Q_REPLACE, "replace into variables (variable_name, variable_value, comment) values('%s', '%s', '%s')", 
			getSafeEscapeString(name).c_str(), getSafeEscapeString(value).c_str(), getSafeEscapeString(comment).c_str());
	}
	else{
		query.RunQuery2(Q_REPLACE, "replace into variables (variable_name, variable_value) values('%s', '%s')", 
			getSafeEscapeString(name).c_str(), getSafeEscapeString(value).c_str());
	}
}

void WorldDatabase::LoadGlobalVariables(){
	variables.ClearVariables ( );
	int32 total = 0;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT variable_name, variable_value, comment FROM variables");

	while(result && (row = mysql_fetch_row(result)))
	{
		Variable* newVar = new Variable(row[0], row[1], row[2]);
		variables.AddVariable(newVar);
		total++;
		LogWrite(WORLD__DEBUG, 5, "World", "---Loading variable: '%s' = '%s'", row[0], row[1]);
	}
	LogWrite(WORLD__DEBUG, 3, "World", "--Loaded %u variables", total);
}


void WorldDatabase::LoadAppearanceMasterList()
{
	DatabaseResult result;
	int32 total = 0;
	int32	appearance_id;
	int16	appearance_version;

	master_appearance_list.ClearAppearances();

	database_new.Select(&result, "SELECT appearance_id, `name`, min_client_version FROM appearances ORDER BY appearance_id");

	while( result.Next() )
	{
		appearance_id				= result.GetInt32Str("appearance_id");
		const char *appearance_name	= result.GetStringStr("name");
		appearance_version			= result.GetInt16Str("min_client_version");
		Appearance* a				= new Appearance(appearance_id, appearance_name, appearance_version);

		master_appearance_list.InsertAppearance(a);

		total++;
		LogWrite(WORLD__DEBUG, 5, "World", "---Loading appearances: '%s' (%i)", appearance_name, appearance_id);
	}
	LogWrite(WORLD__DEBUG, 3, "World", "--Loaded %u appearances", total);
}


void WorldDatabase::LoadVisualStates()
{
	visual_states.Reset();
	int32 total = 0;
	Query query;
	Query query2;
	Query query3;
	MYSQL_ROW row;
	MYSQL_ROW row2;
	MYSQL_ROW row3;

	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT visual_state_id, name FROM visual_states");
	while(result && (row = mysql_fetch_row(result)))
	{
		VisualState* vs = new VisualState(atoi(row[0]), row[1]);
		visual_states.InsertVisualState(vs);
		total++;
		LogWrite(WORLD__DEBUG, 5, "World", "---Loading visual state: '%s' (%i)", row[1], atoi(row[0]));
	}
	LogWrite(WORLD__DEBUG, 3, "World", "--Loaded %u visual states", total);

	total = 0;
	MYSQL_RES* result2 = query2.RunQuery2(Q_SELECT, "SELECT name, visual_state_id, message, targeted_message, min_version_range, max_version_range FROM emotes");
	while(result2 && (row2 = mysql_fetch_row(result2)))
	{
		EmoteVersionRange* range = 0;
		if ((range = visual_states.FindEmoteRange(string(row2[0]))) == NULL)
		{
			range = new EmoteVersionRange(row2[0]);
			visual_states.InsertEmoteRange(range);
		}
		
		range->AddVersionRange(atoul(row2[4]),atoul(row2[5]), row2[0], atoul(row2[1]), row2[2], row2[3]);
		total++;
		LogWrite(WORLD__DEBUG, 5, "World", "---Loading emote state: '%s' (%i)", row2[0], atoul(row2[0]));
	}
	LogWrite(WORLD__DEBUG, 3, "World", "--Loaded %u emote state(s)", total);
	
	
	total = 0;
	MYSQL_RES* result3 = query3.RunQuery2(Q_SELECT, "SELECT name, spell_visual_id, alternate_spell_visual, min_version_range, max_version_range FROM spell_visuals");
	while(result3 && (row3 = mysql_fetch_row(result3)))
	{
		EmoteVersionRange* range = 0;
		if ((range = visual_states.FindSpellVisualRange(string(row3[0]))) == NULL)
		{
			range = new EmoteVersionRange(row3[0]);
			visual_states.InsertSpellVisualRange(range, atoul(row3[1]));
		}
		
		range->AddVersionRange(atoul(row3[3]),atoul(row3[4]), row3[0], atoul(row3[1]), row3[2]);
		total++;
		LogWrite(WORLD__DEBUG, 5, "World", "---Loading spell visual state: '%s' (%u)", row3[1], atoul(row3[0]));
	}
	LogWrite(WORLD__DEBUG, 3, "World", "--Loaded %u spell visual state(s)", total);
}

void WorldDatabase::LoadSubCommandList() 
{
	int32 total = 0;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT command, subcommand, handler, required_status FROM commands where length(subcommand) > 0 ORDER BY handler asc");
	while(result && (row = mysql_fetch_row(result)))
	{
		commands.GetRemoteCommands()->CheckAddSubCommand(string(row[0]), EQ2_RemoteCommandString(row[1], (int32)strtoul(row[2], NULL, 0), atoi(row[3])));
		total++;
		LogWrite(COMMAND__DEBUG, 5, "Command", "---Loading Command: '%s', sub '%s', handler, %u status %i", row[0], row[1], atoul(row[2]), atoi(row[3]));
	}
	LogWrite(COMMAND__DEBUG, 3, "Command", "--Loaded %i Subcommand(s)", total);
}

void WorldDatabase::LoadCommandList() 
{
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT command, handler, required_status FROM commands where length(subcommand) = 0 ORDER BY handler asc");
	int16 index = 0;

	while(result && (row = mysql_fetch_row(result)))
	{
		int32 handler = strtoul(row[1], NULL, 0);
		while(handler>index && handler != 999)
		{
			LogWrite(COMMAND__DEBUG, 5, "Command", "---Loading Remote Commands: handler %u, index %u", handler, index);
			commands.GetRemoteCommands()->addZero();
			index++;
		}
		LogWrite(COMMAND__DEBUG, 5, "Command", "---Loading Commands: handler %u, index %u", handler, index);
		commands.GetRemoteCommands()->addCommand(EQ2_RemoteCommandString(row[0], handler, atoi(row[2])));
		index++;
	}
	LogWrite(COMMAND__DEBUG, 3, "Command", "--Loaded %i Command%s", index, index > 0 ? "s" : "");
	LoadSubCommandList();
}

int32 WorldDatabase::LoadNPCSpells(){
	Query query;
	MYSQL_ROW row;
	int32 count = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT spell_list_id, spell_id, spell_tier, on_spawn_cast, on_aggro_cast, required_hp_ratio FROM spawn_npc_spells where spell_list_id > 0");
	while(result && (row = mysql_fetch_row(result))){
		if(!row[0] || !row[1] || !row[2] || !row[3] || !row[4] || !row[5]) {
			LogWrite(NPC__ERROR, 0, "NPC", "---Loading NPC Spell List: %u, found NULL values in entry, SKIP!", row[0] ? atoul(row[0]) : 0);
		}
		else {
			world.AddNPCSpell(atoul(row[0]), atoul(row[1]), atoi(row[2]), atoul(row[3]), atoul(row[4]), atoi(row[5]));
			count++;

			LogWrite(NPC__DEBUG, 5, "NPC", "---Loading NPC Spell List: %u, spell id: %u, tier: %i", atoul(row[0]), atoul(row[1]), atoi(row[2]));
		}

	}
	return count;
}

int32 WorldDatabase::LoadNPCSkills(ZoneServer* zone){
	Query query;
	MYSQL_ROW row;
	int32 count = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT skill_list_id, skill_id, starting_value FROM spawn_npc_skills");
	while(result && (row = mysql_fetch_row(result))){
		zone->AddNPCSkill(atoul(row[0]), atoul(row[1]), atoi(row[2]));
		count++;

		LogWrite(NPC__DEBUG, 5, "NPC", "---Loading NPC Skill List: %u, skill id: %u, value: %i", atoul(row[0]), atoul(row[1]), atoi(row[2]));

	}
	return count;
}

int32 WorldDatabase::LoadNPCEquipment(ZoneServer* zone){
	Query query;
	MYSQL_ROW row;
	int32 count = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT equipment_list_id, item_id FROM spawn_npc_equipment");
	while(result && (row = mysql_fetch_row(result))){
		zone->AddNPCEquipment(atoul(row[0]), atoul(row[1]));
		count++;

		LogWrite(NPC__DEBUG, 5, "NPC", "---Loading NPC Equipment List: %u, item: %u", atoul(row[0]), atoul(row[1]));

	}
	return count;
}

int8 WorldDatabase::GetAppearanceType(string type){
	int8 ret = 255;
	if (type == "soga_hair_face_highlight_color")
		ret = APPEARANCE_SOGA_HFHC;
	else if (type == "soga_hair_type_highlight_color")
		ret = APPEARANCE_SOGA_HTHC;
	else if (type == "soga_hair_face_color")
		ret = APPEARANCE_SOGA_HFC;
	else if (type == "soga_hair_type_color")
		ret = APPEARANCE_SOGA_HTC;
	else if (type == "soga_hair_highlight")
		ret = APPEARANCE_SOGA_HH;
	else if (type == "soga_hair_color1")
		ret = APPEARANCE_SOGA_HC1;
	else if (type == "soga_hair_color2")
		ret = APPEARANCE_SOGA_HC2;
	else if (type == "hair_type_color")
		ret = APPEARANCE_HTC;
	else if (type == "soga_skin_color")
		ret = APPEARANCE_SOGA_SC;
	else if (type == "soga_eye_color")
		ret = APPEARANCE_SOGA_EC;
	else if (type == "hair_type_highlight_color")
		ret = APPEARANCE_HTHC;
	else if (type == "hair_face_highlight_color")
		ret = APPEARANCE_HFHC;
	else if (type == "hair_face_color")
		ret = APPEARANCE_HFC;
	else if (type == "hair_highlight")
		ret = APPEARANCE_HH;
	else if (type == "hair_color1")
		ret = APPEARANCE_HC1;
	else if (type == "wing_color1")
		ret = APPEARANCE_WC1;
	else if (type == "hair_color2")
		ret = APPEARANCE_HC2;
	else if (type == "wing_color2")
		ret = APPEARANCE_WC2;
	else if (type == "skin_color")
		ret = APPEARANCE_SC;
	else if (type == "eye_color")
		ret = APPEARANCE_EC;
	else if (type == "soga_eye_brow_type")
		ret = APPEARANCE_SOGA_EBT;
	else if (type == "soga_cheek_type")
		ret = APPEARANCE_SOGA_CHEEKT;
	else if (type == "soga_nose_type")
		ret = APPEARANCE_SOGA_NT;
	else if (type == "soga_chin_type")
		ret = APPEARANCE_SOGA_CHINT;
	else if (type == "soga_lip_type")
		ret = APPEARANCE_SOGA_LT;
	else if (type == "eye_brow_type")
		ret = APPEARANCE_EBT;
	else if (type == "soga_ear_type")
		ret = APPEARANCE_SOGA_EART;
	else if (type == "soga_eye_type")
		ret = APPEARANCE_SOGA_EYET;
	else if (type == "cheek_type")
		ret = APPEARANCE_CHEEKT;
	else if (type == "nose_type")
		ret = APPEARANCE_NT;
	else if (type == "chin_type")
		ret = APPEARANCE_CHINT;
	else if (type == "ear_type")
		ret = APPEARANCE_EART;
	else if (type == "eye_type")
		ret = APPEARANCE_EYET;
	else if (type == "lip_type")
		ret = APPEARANCE_LT;
	else if (type == "shirt_color")
		ret = APPEARANCE_SHIRT;
	else if (type == "unknown_chest_color")
		ret = APPEARANCE_UCC;
	else if (type == "pants_color")
		ret = APPEARANCE_PANTS;
	else if (type == "unknown_legs_color")
		ret = APPEARANCE_ULC;
	else if (type == "unknown9")
		ret = APPEARANCE_U9;
	else if (type == "body_size")
		ret = APPEARANCE_BODY_SIZE;
	else if (type == "soga_wing_color1")
		ret = APPEARANCE_SOGA_WC1;
	else if (type == "soga_wing_color2")
		ret = APPEARANCE_SOGA_WC2;
	else if (type == "soga_shirt_color")
		ret = APPEARANCE_SOGA_SHIRT;
	else if (type == "soga_unknown_chest_color")
		ret = APPEARANCE_SOGA_UCC;
	else if (type == "soga_pants_color")
		ret = APPEARANCE_SOGA_PANTS;
	else if (type == "soga_unknown_legs_color")
		ret = APPEARANCE_SOGA_ULC;
	else if (type == "soga_unknown13")
		ret = APPEARANCE_SOGA_U13;
	else if (type == "body_age")
		ret = APPEARANCE_BODY_AGE;
	else if (type == "model_color")
		ret = APPEARANCE_MC;
	else if (type == "soga_model_color")
		ret = APPEARANCE_SMC;
	else if (type == "soga_body_size")
		ret = APPEARANCE_SBS;
	else if (type == "soga_body_age")
		ret = APPEARANCE_SBA;
	return ret;
}

int32 WorldDatabase::LoadAppearances(ZoneServer* zone, Client* client){
	Query query, query2;
	MYSQL_ROW row;
	int32 count = 0, spawn_id = 0, new_spawn_id = 0;
	Entity* entity = 0;
	if(client)
		entity = client->GetPlayer();
	map<string, int8> appearance_types;
	map<int32, map<int8, EQ2_Color> > appearance_colors;
	EQ2_Color color;
	color.red = 0;
	color.green = 0;
	color.blue = 0;
	string type;
	MYSQL_RES* result = 0;
	if(!client)
		result = query.RunQuery2(Q_SELECT, "SELECT distinct `type` FROM npc_appearance where length(type) > 0");
	else
		result = query.RunQuery2(Q_SELECT, "SELECT distinct `type` FROM char_colors where length(type) > 0 and char_id=%u", client->GetCharacterID());
	while(result && (row = mysql_fetch_row(result))){
		type = string(row[0]);
		appearance_types[type] = GetAppearanceType(type);
		if(appearance_types[type] == 255)
			LogWrite(WORLD__ERROR, 0, "Appearance", "Unknown appearance type '%s' in LoadAppearances.", type.c_str());
	}

	MYSQL_RES* result2 = 0;
	if(!client)
		result2 = query2.RunQuery2(Q_SELECT, "SELECT `type`, spawn_id, signed_value, red, green, blue FROM npc_appearance where length(type) > 0 ORDER BY spawn_id");
	else
		result2 = query2.RunQuery2(Q_SELECT, "SELECT `type`, char_id, signed_value, red, green, blue FROM char_colors where length(type) > 0 and char_id=%u", client->GetCharacterID());
	while(result2 && (row = mysql_fetch_row(result2))){
		if(!client){
			new_spawn_id = atoul(row[1]);
			if(spawn_id != new_spawn_id){
				entity = zone->GetNPC(new_spawn_id, true);
				if(!entity)
					continue;
				if(spawn_id > 0)
					count++;
				spawn_id = new_spawn_id;
			}
		}
		if(appearance_types[row[0]] < APPEARANCE_SOGA_EBT){ 
			color.red = atoi(row[3]);
			color.green = atoi(row[4]);
			color.blue = atoi(row[5]);
		}
		switch(appearance_types[row[0]]){
			case APPEARANCE_SOGA_HFHC:{
				entity->features.soga_hair_face_highlight_color = color;
				break;
			}
			case APPEARANCE_SOGA_HTHC:{
				entity->features.soga_hair_type_highlight_color = color;
				break;
			}
			case APPEARANCE_SOGA_HFC:{
				entity->features.soga_hair_face_color = color;
				break;
			}
			case APPEARANCE_SOGA_HTC:{
				entity->features.soga_hair_type_color = color;
				break;
			}
			case APPEARANCE_SOGA_HH:{
				entity->features.soga_hair_highlight_color = color;
				break;
			}
			case APPEARANCE_SOGA_HC1:{
				entity->features.soga_hair_color1 = color;
				break;
			}
			case APPEARANCE_SOGA_HC2:{
				entity->features.soga_hair_color2 = color;
				break;
			}
			case APPEARANCE_SOGA_SC:{
				entity->features.soga_skin_color = color;
				break;
			}
			case APPEARANCE_SOGA_EC:{
				entity->features.soga_eye_color = color;
				break;
			}
			case APPEARANCE_HTHC:{
				entity->features.hair_type_highlight_color = color;
				break;
			}
			case APPEARANCE_HFHC:{
				entity->features.hair_face_highlight_color = color;
				break;
			}
			case APPEARANCE_HTC:{
				entity->features.hair_type_color = color;
				break;
				}
			case APPEARANCE_HFC:{
				entity->features.hair_face_color = color;
				break;
				}
			case APPEARANCE_HH:{
				entity->features.hair_highlight_color = color;
				break;
			}
			case APPEARANCE_HC1:{
				entity->features.hair_color1 = color;
				break;
				}
			case APPEARANCE_HC2:{
				entity->features.hair_color2 = color;
				break;
				}
			case APPEARANCE_WC1:{
				entity->features.wing_color1 = color;
				break;
				}
			case APPEARANCE_WC2:{
				entity->features.wing_color2 = color;
				break;
				}
			case APPEARANCE_SC:{
				entity->features.skin_color = color;
				break;
			}
			case APPEARANCE_EC:{
				entity->features.eye_color = color;
				break;
			}
			case APPEARANCE_SOGA_EBT:{
				for(int i=0;i<3;i++)
					entity->features.soga_eye_brow_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_SOGA_CHEEKT:{
				for(int i=0;i<3;i++)
					entity->features.soga_cheek_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_SOGA_NT:{
				for(int i=0;i<3;i++)
					entity->features.soga_nose_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_SOGA_CHINT:{
				for(int i=0;i<3;i++)
					entity->features.soga_chin_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_SOGA_LT:{
				for(int i=0;i<3;i++)
					entity->features.soga_lip_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_SOGA_EART:{
				for(int i=0;i<3;i++)
					entity->features.soga_ear_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_SOGA_EYET:{
				for(int i=0;i<3;i++)
					entity->features.soga_eye_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_EBT:{
				for(int i=0;i<3;i++)
					entity->features.eye_brow_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_CHEEKT:{
				for(int i=0;i<3;i++)
					entity->features.cheek_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_NT:{
				for(int i=0;i<3;i++)
					entity->features.nose_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_CHINT:{
				for(int i=0;i<3;i++)
					entity->features.chin_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_EART:{
				for(int i=0;i<3;i++)
					entity->features.ear_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_EYET:{
				for(int i=0;i<3;i++)
					entity->features.eye_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_LT:{
				for(int i=0;i<3;i++)
					entity->features.lip_type[i] = atoi(row[3+i]);
				break;
			}
			case APPEARANCE_SHIRT:{
				entity->features.shirt_color = color;
				break;
			}
			case APPEARANCE_UCC:{
				break;
			}
			case APPEARANCE_PANTS:{
				entity->features.pants_color = color;
				break;
			}
			case APPEARANCE_ULC:{
				break;
			}
			case APPEARANCE_U9:{
				break;
			}
			case APPEARANCE_BODY_SIZE:{
				entity->features.body_size = color.red;
				break;
			}
			case APPEARANCE_SOGA_WC1:{
				break;
			}
			case APPEARANCE_SOGA_WC2:{
				break;
			}
			case APPEARANCE_SOGA_SHIRT:{				
				break;
			}
			case APPEARANCE_SOGA_UCC:{
				break;
			}
			case APPEARANCE_SOGA_PANTS:{
				break;
			}
			case APPEARANCE_SOGA_ULC:{
				break;
			}
			case APPEARANCE_SOGA_U13:{
				break;
			}
			case APPEARANCE_BODY_AGE: {
				entity->features.body_age = color.red;
				break;
			}
			case APPEARANCE_MC:{
				entity->features.model_color = color;
				break;
			}
			case APPEARANCE_SMC:{
				entity->features.soga_model_color = color;
				break;
			}
			case APPEARANCE_SBS: {
				entity->features.soga_body_size = color.red;
				break;
			}
			case APPEARANCE_SBA: {
				entity->features.soga_body_age = color.red;
				break;
			}
		}
		entity->info_changed = true;
	}
	return count;
}

void WorldDatabase::LoadNPCs(ZoneServer* zone){
	Query query;
	MYSQL_ROW row;
	NPC* npc = 0;
	int32 id = 0;
	int32 total = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT,"SELECT npc.spawn_id, s.name, npc.min_level, npc.max_level, npc.enc_level, s.race, s.model_type, npc.class_, npc.gender, s.command_primary, s.command_secondary, s.show_name, npc.min_group_size, npc.max_group_size, npc.hair_type_id, npc.facial_hair_type_id, npc.wing_type_id, npc.chest_type_id, npc.legs_type_id, npc.soga_hair_type_id, npc.soga_facial_hair_type_id, s.attackable, s.show_level, s.targetable, s.show_command_icon, s.display_hand_icon, s.hp, s.power, s.size, s.collision_radius, npc.action_state, s.visual_state, npc.mood_state, npc.initial_state, npc.activity_status, s.faction_id, s.sub_title, s.merchant_id, s.merchant_type, s.size_offset, npc.attack_type, npc.ai_strategy+0, npc.spell_list_id, npc.secondary_spell_list_id, npc.skill_list_id, npc.secondary_skill_list_id, npc.equipment_list_id, npc.str, npc.sta, npc.wis, npc.intel, npc.agi, npc.heat, npc.cold, npc.magic, npc.mental, npc.divine, npc.disease, npc.poison, npc.aggro_radius, npc.cast_percentage, npc.randomize, npc.soga_model_type, npc.heroic_flag, npc.alignment, npc.elemental, npc.arcane, npc.noxious, s.savagery, s.dissonance, npc.hide_hood, npc.emote_state, s.prefix, s.suffix, s.last_name, s.expansion_flag, s.holiday_flag, s.disable_sounds, s.merchant_min_level, s.merchant_max_level, s.aaxp_rewards, npc.water_type, npc.flying_type, s.loot_tier, s.loot_drop_type, npc.scared_by_strong_players, npc.action_state_str\n"
													"FROM spawn s\n"
													"INNER JOIN spawn_npcs npc\n"
													"ON s.id = npc.spawn_id\n"
													"INNER JOIN spawn_location_entry le\n"
													"ON npc.spawn_id = le.spawn_id\n"
													"INNER JOIN spawn_location_placement lp\n"
													"ON le.spawn_location_id = lp.spawn_location_id\n"
													"WHERE lp.zone_id = %u and (lp.instance_id = 0 or lp.instance_id = %u)\n"
													"GROUP BY s.id",
													zone->GetZoneID(), zone->GetInstanceID());
	while(result && (row = mysql_fetch_row(result))){
		/*npc->SetAppearanceID(atoi(row[12]));
		AppearanceData* appearance = world.GetNPCAppearance(npc->GetAppearanceID());
		if(appearance)
		memcpy(&npc->appearance, appearance, sizeof(AppearanceData));
		*/
		int32 npcXpackFlag = atoul(row[75]);
		int32 npcHolidayFlag = atoul(row[76]);

		id = atoul(row[0]);
		if(zone->GetNPC(id, true))
			continue;
		npc = new NPC();
		
		if (!CheckExpansionFlags(zone, npcXpackFlag) || !CheckHolidayFlags(zone, npcHolidayFlag))
			npc->SetOmittedByDBFlag(true);

		npc->SetDatabaseID(id);
		strcpy(npc->appearance.name, row[1]);
		vector<EntityCommand*>* primary_command_list = zone->GetEntityCommandList(atoul(row[9]));
		vector<EntityCommand*>* secondary_command_list = zone->GetEntityCommandList(atoul(row[10]));
		if(primary_command_list){
			npc->SetPrimaryCommands(primary_command_list);
			npc->primary_command_list_id = atoul(row[9]);
		}
		if(secondary_command_list){
			npc->SetSecondaryCommands(secondary_command_list);
			npc->secondary_command_list_id = atoul(row[10]);
		}
		npc->appearance.min_level = atoi(row[2]);
		npc->appearance.max_level = atoi(row[3]);
		npc->appearance.level =		atoi(row[2]);
		npc->appearance.difficulty = atoi(row[4]);
		npc->appearance.race = atoi(row[5]);
		npc->appearance.model_type = atoi(row[6]);
		npc->appearance.soga_model_type = atoi(row[62]);
		npc->appearance.adventure_class = atoi(row[7]);
		npc->appearance.gender = atoi(row[8]);
		npc->appearance.display_name = atoi(row[11]);
		npc->features.hair_type = atoi(row[14]);
		npc->features.hair_face_type = atoi(row[15]);
		npc->features.wing_type = atoi(row[16]);
		npc->features.chest_type = atoi(row[17]);
		npc->features.legs_type = atoi(row[18]);
		npc->features.soga_hair_type = atoi(row[19]);
		npc->features.soga_hair_face_type = atoi(row[20]);
		npc->appearance.attackable = atoi(row[21]);
		npc->appearance.show_level = atoi(row[22]);
		npc->appearance.targetable = atoi(row[23]);
		npc->appearance.show_command_icon = atoi(row[24]);
		npc->appearance.display_hand_icon = atoi(row[25]);
		npc->appearance.hide_hood = atoi(row[70]);
		npc->appearance.randomize = atoi(row[61]);
		npc->SetTotalHP(atoul(row[26]));
		npc->SetTotalHPBaseInstance(atoul(row[26]));
		npc->SetTotalPower(atoul(row[27]));
		npc->SetTotalPowerBaseInstance(atoul(row[27]));
		npc->SetHP(npc->GetTotalHP());
		npc->SetPower(npc->GetTotalPower());
		if(npc->GetTotalHP() == 0){
			npc->SetTotalHP(15*npc->GetLevel() + 1);
			npc->SetHP(15*npc->GetLevel() + 1);
		}
		if(npc->GetTotalPower() == 0){
			npc->SetTotalPower(15*npc->GetLevel() + 1);
			npc->SetPower(15*npc->GetLevel() + 1);
		}
		npc->size = atoi(row[28]);
		npc->appearance.pos.collision_radius = atoi(row[29]);
		npc->appearance.action_state = atoi(row[30]);
		npc->appearance.visual_state = atoi(row[31]);
		npc->appearance.mood_state = atoi(row[32]);
		npc->appearance.emote_state = atoi(row[71]);
		npc->appearance.pos.state = atoi(row[33]);
		npc->appearance.activity_status = atoi(row[34]);
		npc->faction_id = atoul(row[35]);
		if(row[36]){
			std::string sub_title = std::string(row[36]);
			if(strncmp(row[36],"<Collector>", 11) == 0) {
				npc->SetCollector(true);
			}
			if(strlen(row[36]) < sizeof(npc->appearance.sub_title))
				strcpy(npc->appearance.sub_title, row[36]);
			else
				strncpy(npc->appearance.sub_title, row[36], sizeof(npc->appearance.sub_title));
		}
		npc->SetMerchantID(atoul(row[37]));
		npc->SetMerchantType(atoi(row[38]));
		npc->SetSizeOffset(atoi(row[39]));
		npc->SetAIStrategy(atoi(row[41]));
		npc->SetPrimarySpellList(atoul(row[42]));
		npc->SetSecondarySpellList(atoul(row[43]));
		npc->SetPrimarySkillList(atoul(row[44]));
		npc->SetSecondarySkillList(atoul(row[45]));
		npc->SetEquipmentListID(atoul(row[46]));

		InfoStruct* info = npc->GetInfoStruct();
		info->set_attack_type(atoi(row[40]));
		info->set_str_base(atoi(row[47]));
		info->set_sta_base(atoi(row[48]));
		info->set_wis_base(atoi(row[49]));		
		info->set_intel_base(atoi(row[50]));
		info->set_agi_base(atoi(row[51]));
		info->set_heat_base(atoi(row[52]));
		info->set_cold_base(atoi(row[53]));
		info->set_magic_base(atoi(row[54]));
		info->set_mental_base(atoi(row[55]));
		info->set_divine_base(atoi(row[56]));
		info->set_disease_base(atoi(row[57]));
		info->set_poison_base(atoi(row[58]));
		info->set_alignment(atoi(row[64]));

		npc->SetAggroRadius(atof(row[59]));
		npc->SetCastPercentage(atoi(row[60]));
		npc->appearance.heroic_flag = atoi(row[63]);

		info->set_elemental_base(atoi(row[65]));
		info->set_arcane_base(atoi(row[66]));
		info->set_noxious_base(atoi(row[67]));
		npc->SetTotalSavagery(atoul(row[68]));
		npc->SetTotalDissonance(atoul(row[69]));
		npc->SetSavagery(npc->GetTotalSavagery());
		npc->SetDissonance(npc->GetTotalDissonance());
		if(npc->GetTotalSavagery() == 0){
			npc->SetTotalSavagery(15*npc->GetLevel() + 1);
			npc->SetSavagery(15*npc->GetLevel() + 1);
		}
		if(npc->GetTotalDissonance() == 0){
			npc->SetTotalDissonance(15*npc->GetLevel() + 1);
			npc->SetDissonance(15*npc->GetLevel() + 1);
		}
		npc->SetPrefixTitle(row[72]);
		npc->SetSuffixTitle(row[73]);
		npc->SetLastName(row[74]);

		// xpack+holiday value handled at top at position 75+76

		int8 disableSounds = atoul(row[77]);
		npc->SetSoundsDisabled(disableSounds);

		npc->SetMerchantLevelRange(atoul(row[78]), atoul(row[79]));
		
		npc->SetAAXPRewards(atoul(row[80]));

		info->set_water_type(atoul(row[81]));
		info->set_flying_type(atoul(row[82]));

		npc->SetLootTier(atoul(row[83]));
		
		npc->SetLootDropType(atoul(row[84]));
		
		npc->SetScaredByStrongPlayers(atoul(row[85]));
		
		if(row[86]){
			std::string action_state_str = std::string(row[86]);
			npc->GetInfoStruct()->set_action_state(action_state_str);
		}
		
		zone->AddNPC(id, npc);
		total++;
		LogWrite(NPC__DEBUG, 5, "NPC", "---Loading NPC: '%s' (%u)", npc->appearance.name, id);
	}
	LogWrite(NPC__INFO, 0, "NPC", "--Loaded %i NPC(s).", total);
	LogWrite(NPC__INFO, 0, "NPC", "--Loaded %i NPC Skill(s).", LoadNPCSkills(zone));
	LogWrite(NPC__INFO, 0, "NPC", "--Loaded %i NPC Equipment Piece(s).", LoadNPCEquipment(zone));
	LogWrite(NPC__INFO, 0, "NPC", "--Loaded %i NPC Appearance(s).", LoadAppearances(zone));	
	LogWrite(NPC__INFO, 0, "NPC", "--Loaded %i NPC Equipment Appearance(s).", LoadNPCAppearanceEquipmentData(zone));	
}


void WorldDatabase::LoadSpiritShards(ZoneServer* zone){
	Query query;
	MYSQL_ROW row;
	int32 id = 0;
	int32 total = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT,"SELECT unix_timestamp(timestamp), name, level, race, gender, adventure_class, model_type, soga_model_type, hair_type, hair_face_type, wing_type, chest_type, legs_type, soga_hair_type, soga_hair_face_type, hide_hood, size, collision_radius, action_state, visual_state, mood_state, emote_state, pos_state, activity_status, sub_title, prefix_title, suffix_title, lastname, x, y, z, heading, gridid, id, charid\n"
													"FROM character_spirit_shards\n"
													"WHERE zoneid = %u and (instanceid = 0 or instanceid = %u)",
													zone->GetZoneID(), zone->GetInstanceID());
	while(result && (row = mysql_fetch_row(result))){
		/*npc->SetAppearanceID(atoi(row[12]));
		AppearanceData* appearance = world.GetNPCAppearance(npc->GetAppearanceID());
		if(appearance)
		memcpy(&npc->appearance, appearance, sizeof(AppearanceData));
		*/
		sint64 timestamp = 0;
#ifdef WIN32
			timestamp = _strtoui64(row[0], NULL, 10);
#else
			timestamp = strtoull(row[0], 0, 10);
#endif
		
		if(!row[1])
			continue;

		NPC* shard = new NPC();
		
		shard->SetShardCreatedTimestamp(timestamp);
		strcpy(shard->appearance.name, row[1]);

		shard->appearance.level =	atoul(row[2]);
		shard->appearance.race = atoul(row[3]);
		shard->appearance.gender = atoul(row[4]);
		shard->appearance.adventure_class = atoul(row[5]);
		shard->appearance.model_type = atoul(row[6]);
		shard->appearance.soga_model_type = atoul(row[7]);
		shard->appearance.display_name = 1;
		shard->features.hair_type = atoul(row[8]);
		shard->features.hair_face_type = atoul(row[9]);
		shard->features.wing_type = atoul(row[10]);
		shard->features.chest_type = atoul(row[11]);
		shard->features.legs_type = atoul(row[12]);
		shard->features.soga_hair_type = atoul(row[13]);
		shard->features.soga_hair_face_type = atoul(row[14]);
		shard->appearance.attackable = 0;
		shard->appearance.show_level = 1;
		shard->appearance.targetable = 1;
		shard->appearance.show_command_icon = 1;
		shard->appearance.display_hand_icon = 0;
		shard->appearance.hide_hood = atoul(row[15]);
		shard->size = atoul(row[16]);
		shard->appearance.pos.collision_radius = atoul(row[17]);
		shard->appearance.action_state = atoul(row[18]);
		shard->appearance.visual_state = atoul(row[19]); // ghostly look
		shard->appearance.mood_state = atoul(row[20]);
		shard->appearance.emote_state = atoul(row[21]);
		shard->appearance.pos.state = atoul(row[22]);
		shard->appearance.activity_status = atoul(row[23]);

		if(row[24])
			strncpy(shard->appearance.sub_title, row[24], sizeof(shard->appearance.sub_title));

		if(row[25])
			shard->SetPrefixTitle(row[25]);

		if(row[26])
			shard->SetSuffixTitle(row[26]);

		if(row[27])
			shard->SetLastName(row[27]);

		shard->SetX(atof(row[28]));
		shard->SetY(atof(row[29]));
		shard->SetZ(atof(row[30]));
		shard->SetHeading(atof(row[31]));
		shard->SetSpawnOrigX(shard->GetX());
		shard->SetSpawnOrigY(shard->GetY());
		shard->SetSpawnOrigZ(shard->GetZ());
		shard->SetSpawnOrigHeading(shard->GetHeading());
		shard->SetLocation(atoul(row[32]));
		shard->SetShardID(atoul(row[33]));
		shard->SetShardCharID(atoul(row[34]));
		shard->SetAlive(false);
		
		const char* script = rule_manager.GetGlobalRule(R_Combat, SpiritShardSpawnScript)->GetString();

		if(script)
		{
			shard->SetSpawnScript(script);
			zone->CallSpawnScript(shard, SPAWN_SCRIPT_PRESPAWN);
		}
		
		zone->AddSpawn(shard);

		if(script)
			zone->CallSpawnScript(shard, SPAWN_SCRIPT_SPAWN);
		
		total++;
		LogWrite(NPC__DEBUG, 5, "NPC", "---Loading Player Spirit Shard: '%s' (%u)", shard->appearance.name, id);
	}
	LogWrite(NPC__INFO, 0, "NPC", "--Loaded %i Spirit Shard(s).", total);
}

void WorldDatabase::LoadSigns(ZoneServer* zone){
	Query query;
	MYSQL_ROW row;
	Sign* sign = 0;
	int32 id = 0;
	int32 total = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT ss.spawn_id, s.name, s.model_type, s.size, s.show_command_icon, ss.widget_id, ss.widget_x, ss.widget_y, ss.widget_z, s.command_primary, s.command_secondary, s.collision_radius, ss.icon, ss.type, ss.title, ss.description, ss.sign_distance, ss.zone_id, ss.zone_x, ss.zone_y, ss.zone_z, ss.zone_heading, ss.include_heading, ss.include_location, s.transport_id, s.size_offset, s.display_hand_icon, s.visual_state, s.expansion_flag, s.holiday_flag, s.disable_sounds, s.merchant_min_level, s.merchant_max_level, s.aaxp_rewards, s.loot_tier, s.loot_drop_type, ss.language\n"
												  "FROM spawn s\n"
												  "INNER JOIN spawn_signs ss\n"
												  "ON s.id = ss.spawn_id\n"
												  "INNER JOIN spawn_location_entry le\n"
												  "ON ss.spawn_id = le.spawn_id\n"
												  "INNER JOIN spawn_location_placement lp\n"
												  "ON le.spawn_location_id = lp.spawn_location_id\n"
												  "WHERE lp.zone_id = %u and (lp.instance_id = 0 or lp.instance_id = %u)\n"
												  "GROUP BY s.id",
												  zone->GetZoneID(), zone->GetInstanceID());

	while(result && (row = mysql_fetch_row(result))){
		int32 signXpackFlag = atoul(row[28]);
		int32 signHolidayFlag = atoul(row[29]);

		id = atoul(row[0]);
		if(zone->GetSign(id, true))
			continue;
		sign = new Sign();
		
		if (!CheckExpansionFlags(zone, signXpackFlag) || !CheckHolidayFlags(zone, signHolidayFlag))
			sign->SetOmittedByDBFlag(true);

		sign->SetDatabaseID(id);
		strcpy(sign->appearance.name, row[1]);
		sign->appearance.model_type = atoi(row[2]);
		sign->SetSize(atoi(row[3]));
		sign->appearance.show_command_icon = atoi(row[4]);
		sign->SetWidgetID(atoul(row[5]));
		sign->SetWidgetX(atof(row[6]));
		sign->SetWidgetY(atof(row[7]));
		sign->SetWidgetZ(atof(row[8]));
		vector<EntityCommand*>* primary_command_list = zone->GetEntityCommandList(atoul(row[9]));
		if(primary_command_list){
			sign->SetPrimaryCommands(primary_command_list);
			sign->primary_command_list_id = atoul(row[9]);
		}

		vector<EntityCommand*>* secondary_command_list = zone->GetEntityCommandList(atoul(row[10]));
		if (secondary_command_list){
			sign->SetSecondaryCommands(secondary_command_list);
			sign->secondary_command_list_id = atoul(row[10]);
		}

		sign->appearance.pos.collision_radius = atoi(row[11]);
		sign->SetSignIcon(atoi(row[12]));
		if(strncasecmp(row[13],"Generic", 7) == 0)
			sign->SetSignType(SIGN_TYPE_GENERIC);
		else if(strncasecmp(row[13],"Zone", 4) == 0)
			sign->SetSignType(SIGN_TYPE_ZONE);
		sign->SetSignTitle(row[14]);
		sign->SetSignDescription(row[15]);
		sign->SetSignDistance(atof(row[16]));
		sign->SetSignZoneID(atoul(row[17]));
		sign->SetSignZoneX(atof(row[18]));
		sign->SetSignZoneY(atof(row[19]));
		sign->SetSignZoneZ(atof(row[20]));
		sign->SetSignZoneHeading(atof(row[21]));
		sign->SetIncludeHeading(atoi(row[22]) == 1);
		sign->SetIncludeLocation(atoi(row[23]) == 1);
		sign->SetTransporterID(atoul(row[24]));
		sign->SetSizeOffset(atoi(row[25]));
		sign->appearance.display_hand_icon = atoi(row[26]);
		sign->SetVisualState(atoi(row[27]));

		// xpack+holiday value handled at top at position 28+29

		int8 disableSounds = atoul(row[30]);
		sign->SetSoundsDisabled(disableSounds);

		sign->SetMerchantLevelRange(atoul(row[31]), atoul(row[32]));
		
		sign->SetAAXPRewards(atoul(row[33]));

		sign->SetLootTier(atoul(row[34]));
		
		sign->SetLootDropType(atoul(row[35]));

		sign->SetLanguage(atoul(row[36]));
		
		zone->AddSign(id, sign);
		total++;

		LogWrite(SIGN__DEBUG, 5, "Sign", "---Loading Sign: '%s' (%u).", sign->appearance.name, id);

	}
	LogWrite(SIGN__DEBUG, 0, "Sign", "--Loaded %i Sign(s)", total);
}

void WorldDatabase::LoadWidgets(ZoneServer* zone){
	Query query;
	MYSQL_ROW row;
	Widget* widget = 0;
	int32 id = 0;
	int32 total = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT sw.spawn_id, s.name, s.model_type, s.size, s.show_command_icon, sw.widget_id, sw.widget_x, sw.widget_y, sw.widget_z, s.command_primary, s.command_secondary, s.collision_radius, sw.include_heading, sw.include_location, sw.icon, sw.type, sw.open_heading, sw.open_y, sw.action_spawn_id, sw.open_sound_file, sw.close_sound_file, sw.open_duration, sw.closed_heading, sw.linked_spawn_id, sw.close_y, s.transport_id, s.size_offset, sw.house_id, sw.open_x, sw.open_z, sw.close_x, sw.close_z, s.display_hand_icon, s.expansion_flag, s.holiday_flag, s.disable_sounds, s.merchant_min_level, s.merchant_max_level, s.aaxp_rewards, s.loot_tier, s.loot_drop_type\n"
												  "FROM spawn s\n"
												  "INNER JOIN spawn_widgets sw\n"
												  "ON s.id = sw.spawn_id\n"
												  "INNER JOIN spawn_location_entry le\n"
												  "ON sw.spawn_id = le.spawn_id\n"
												  "INNER JOIN spawn_location_placement lp\n"
												  "ON le.spawn_location_id = lp.spawn_location_id\n"
												  "WHERE lp.zone_id = %u and (lp.instance_id = 0 or lp.instance_id = %u)\n"
												  "GROUP BY s.id",
												  zone->GetZoneID(), zone->GetInstanceID());
	while(result && (row = mysql_fetch_row(result))){
		int32 widgetXpackFlag = atoul(row[33]);
		int32 widgetHolidayFlag = atoul(row[34]);

		id = atoul(row[0]);
		if(zone->GetWidget(id, true))
			continue;
		widget = new Widget();

		if (!CheckExpansionFlags(zone, widgetXpackFlag) || !CheckHolidayFlags(zone, widgetHolidayFlag))
			widget->SetOmittedByDBFlag(true);

		widget->SetDatabaseID(id);
		strcpy(widget->appearance.name, row[1]);
		widget->appearance.model_type = atoi(row[2]);
		widget->SetSize(atoi(row[3]));
		widget->appearance.show_command_icon = atoi(row[4]);

		if (row[5] == NULL)
			widget->SetWidgetID(0xFFFFFFFF);
		else
			widget->SetWidgetID(atoul(row[5]));

		widget->SetWidgetX(atof(row[6]));
		widget->SetWidgetY(atof(row[7]));
		widget->SetWidgetZ(atof(row[8]));
		vector<EntityCommand*>* primary_command_list = zone->GetEntityCommandList(atoul(row[9]));
		if(primary_command_list){
			widget->SetPrimaryCommands(primary_command_list);
			widget->primary_command_list_id = atoul(row[9]);
		}

		vector<EntityCommand*>* secondary_command_list = zone->GetEntityCommandList(atoul(row[10]));
		if (secondary_command_list) {
			widget->SetSecondaryCommands(secondary_command_list);
			widget->secondary_command_list_id = atoul(row[10]);
		}

		widget->appearance.pos.collision_radius = atoi(row[11]);
		widget->SetIncludeHeading(atoi(row[12]) == 1);
		widget->SetIncludeLocation(atoi(row[13]) == 1);
		widget->SetWidgetIcon(atoi(row[14]));
		if (strncasecmp(row[15], "Generic", 7) == 0)
			widget->SetWidgetType(WIDGET_TYPE_GENERIC);
		else if (strncasecmp(row[15], "Door", 4) == 0)
			widget->SetWidgetType(WIDGET_TYPE_DOOR);
		else if (strncasecmp(row[15], "Lift", 4) == 0)
			widget->SetWidgetType(WIDGET_TYPE_LIFT);

		widget->SetOpenHeading(atof(row[16]));
		widget->SetOpenY(atof(row[17]));
		widget->SetActionSpawnID(atoul(row[18]));
		if(row[19] && strlen(row[19]) > 5)
			widget->SetOpenSound(row[19]);
		if(row[20] && strlen(row[20]) > 5)
			widget->SetCloseSound(row[20]);
		widget->SetOpenDuration(atoi(row[21]));
		widget->SetClosedHeading(atof(row[22]));
		widget->SetLinkedSpawnID(atoul(row[23]));
		widget->SetCloseY(atof(row[24]));
		widget->SetTransporterID(atoul(row[25]));
		widget->SetSizeOffset(atoi(row[26]));
		widget->SetHouseID(atoul(row[27]));
		widget->SetOpenX(atof(row[28]));
		widget->SetOpenZ(atof(row[29]));
		widget->SetCloseX(atof(row[30]));
		widget->SetCloseZ(atof(row[31]));
		widget->appearance.display_hand_icon = atoi(row[32]);

		// xpack+holiday value handled at top at position 33+34

		int8 disableSounds = atoul(row[35]);
		widget->SetSoundsDisabled(disableSounds);

		widget->SetMerchantLevelRange(atoul(row[36]), atoul(row[37]));
		
		widget->SetAAXPRewards(atoul(row[38]));

		widget->SetLootTier(atoul(row[39]));
		
		widget->SetLootDropType(atoul(row[40]));

		zone->AddWidget(id, widget);
		total++;

		LogWrite(WIDGET__DEBUG, 5, "Widget", "---Loading Widget: '%s' (%u).", widget->appearance.name, id);

	}
	LogWrite(WIDGET__DEBUG, 0, "Widget", "--Loaded %i Widget(s)", total);
}

void WorldDatabase::LoadObjects(ZoneServer* zone){
	Query query;
	MYSQL_ROW row;
	Object* object = 0;
	int32 id = 0;
	int32 total = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT so.spawn_id, s.name, s.race, s.model_type, s.command_primary, s.command_secondary, s.targetable, s.size, s.show_name, s.visual_state, s.attackable, s.show_level, s.show_command_icon, s.display_hand_icon, s.faction_id, s.collision_radius, s.transport_id, s.size_offset, so.device_id, s.expansion_flag, s.holiday_flag, s.disable_sounds, s.merchant_min_level, s.merchant_max_level, s.aaxp_rewards, s.loot_tier, s.loot_drop_type\n"
												  "FROM spawn s\n"
												  "INNER JOIN spawn_objects so\n"
												  "ON s.id = so.spawn_id\n"
												  "INNER JOIN spawn_location_entry le\n"
												  "ON so.spawn_id = le.spawn_id\n"
												  "INNER JOIN spawn_location_placement lp\n"
												  "ON le.spawn_location_id = lp.spawn_location_id\n"
												  "WHERE lp.zone_id = %u and (lp.instance_id = 0 or lp.instance_id = %u)\n"
												  "GROUP BY s.id",
												  zone->GetZoneID(), zone->GetInstanceID());

	while(result && (row = mysql_fetch_row(result))){

		int32 objXpackFlag = atoul(row[19]);
		int32 objHolidayFlag = atoul(row[20]);

		id = atoul(row[0]);
		if(zone->GetObject(id, true))
			continue;
		object = new Object();
		
		if (!CheckExpansionFlags(zone, objXpackFlag) || !CheckHolidayFlags(zone, objHolidayFlag))
			object->SetOmittedByDBFlag(true);

		object->SetDatabaseID(id);
		strcpy(object->appearance.name, row[1]);
		vector<EntityCommand*>* primary_command_list = zone->GetEntityCommandList(atoul(row[4]));
		vector<EntityCommand*>* secondary_command_list = zone->GetEntityCommandList(atoul(row[5]));
		if(primary_command_list){
			object->SetPrimaryCommands(primary_command_list);
			object->primary_command_list_id = atoul(row[4]);
		}
		if(secondary_command_list){
			object->SetSecondaryCommands(secondary_command_list);
			object->secondary_command_list_id = atoul(row[5]);
		}
		object->appearance.race = atoi(row[2]);
		object->appearance.model_type = atoi(row[3]);
		object->appearance.targetable = atoi(row[6]);
		object->size = atoi(row[7]);
		object->appearance.display_name = atoi(row[8]);
		object->appearance.visual_state = atoi(row[9]);
		object->appearance.attackable = atoi(row[10]);
		object->appearance.show_level = atoi(row[11]);
		object->appearance.show_command_icon = atoi(row[12]);
		object->appearance.display_hand_icon = atoi(row[13]);
		object->faction_id = atoul(row[14]);
		object->appearance.pos.collision_radius = atoi(row[15]);
		object->SetTransporterID(atoul(row[16]));
		object->SetSizeOffset(atoi(row[17]));
		object->SetDeviceID(atoi(row[18]));

		// xpack value handled at top at position 19

		int8 disableSounds = atoul(row[21]);
		object->SetSoundsDisabled(disableSounds);

		object->SetMerchantLevelRange(atoul(row[22]), atoul(row[23]));

		object->SetAAXPRewards(atoul(row[24]));

		object->SetLootTier(atoul(row[25]));
		
		object->SetLootDropType(atoul(row[26]));

		zone->AddObject(id, object);
		total++;

		LogWrite(OBJECT__DEBUG, 5, "Object", "---Loading Object: '%s' (%u)", object->appearance.name, id);

	}
	LogWrite(OBJECT__DEBUG, 0, "Object", "--Loaded %i Object(s)", total);
}

void WorldDatabase::LoadGroundSpawns(ZoneServer* zone){
	Query query;
	MYSQL_ROW row;
	GroundSpawn* spawn = 0;
	int32 id = 0;
	int32 total = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT sg.spawn_id, s.name, s.race, s.model_type, s.command_primary, s.command_secondary, s.targetable, s.size, s.show_name, s.visual_state, s.attackable, s.show_level, s.show_command_icon, s.display_hand_icon, s.faction_id, s.collision_radius, sg.number_harvests, sg.num_attempts_per_harvest, sg.groundspawn_id, sg.collection_skill, s.size_offset, s.expansion_flag, s.holiday_flag, s.disable_sounds, s.aaxp_rewards, s.loot_tier, s.loot_drop_type\n"
												  "FROM spawn s\n"
												  "INNER JOIN spawn_ground sg\n"
												  "ON s.id = sg.spawn_id\n"
												  "INNER JOIN spawn_location_entry le\n"
												  "ON sg.spawn_id = le.spawn_id\n"
												  "INNER JOIN spawn_location_placement lp\n"
												  "ON le.spawn_location_id = lp.spawn_location_id\n"
												  "WHERE lp.zone_id = %u and (lp.instance_id = 0 or lp.instance_id = %u)\n"
												  "GROUP BY s.id",
												  zone->GetZoneID(), zone->GetInstanceID());
	while(result && (row = mysql_fetch_row(result))){

		int32 gsXpackFlag = atoul(row[21]);
		int32 gsHolidayFlag = atoul(row[22]);

		id = atoul(row[0]);
		if(zone->GetGroundSpawn(id, true))
			continue;
		spawn = new GroundSpawn();
		
		if (!CheckExpansionFlags(zone, gsXpackFlag) || !CheckHolidayFlags(zone, gsHolidayFlag))
			spawn->SetOmittedByDBFlag(true);

		spawn->SetDatabaseID(id);
		spawn->forceMapCheck = true;

		strcpy(spawn->appearance.name, row[1]);
		vector<EntityCommand*>* primary_command_list = zone->GetEntityCommandList(atoul(row[4]));
		vector<EntityCommand*>* secondary_command_list = zone->GetEntityCommandList(atoul(row[5]));
		if(primary_command_list){
			spawn->SetPrimaryCommands(primary_command_list);
			spawn->primary_command_list_id = atoul(row[4]);
		}
		if(secondary_command_list){
			spawn->SetSecondaryCommands(secondary_command_list);
			spawn->secondary_command_list_id = atoul(row[5]);
		}
		spawn->appearance.race = atoi(row[2]);
		spawn->appearance.model_type = atoi(row[3]);
		spawn->appearance.targetable = atoi(row[6]);
		spawn->size = atoi(row[7]);
		spawn->appearance.display_name = atoi(row[8]);
		spawn->appearance.visual_state = atoi(row[9]);
		spawn->appearance.attackable = atoi(row[10]);
		spawn->appearance.show_level = atoi(row[11]);
		spawn->appearance.show_command_icon = atoi(row[12]);
		spawn->appearance.display_hand_icon = atoi(row[13]);
		spawn->faction_id = atoul(row[14]);
		spawn->appearance.pos.collision_radius = atoi(row[15]);
		spawn->SetNumberHarvests(atoi(row[16]));
		spawn->SetAttemptsPerHarvest(atoi(row[17]));
		spawn->SetGroundSpawnEntryID(atoul(row[18]));
		spawn->SetCollectionSkill(row[19]);
		spawn->SetSizeOffset(atoi(row[20]));

		// xpack+holiday value handled at top at position 21+22

		int8 disableSounds = atoul(row[23]);
		spawn->SetSoundsDisabled(disableSounds);

		spawn->SetAAXPRewards(atoul(row[24]));

		spawn->SetLootTier(atoul(row[25]));

		spawn->SetLootDropType(atoul(row[26]));
		
		zone->AddGroundSpawn(id, spawn);
		total++;
		LogWrite(GROUNDSPAWN__DEBUG, 5, "GSpawn", "---Loading GroundSpawn: '%s' (%u)", spawn->appearance.name, id);
	}
	LogWrite(GROUNDSPAWN__DEBUG, 0, "GSpawn", "--Loaded %i GroundSpawn(s)", total);
}

void WorldDatabase::LoadGroundSpawnItems(ZoneServer* zone) {
 	Query query;
 	MYSQL_ROW row;
 	int32 total = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT groundspawn_id, item_id, is_rare, grid_id FROM groundspawn_items;");
	while(result && (row = mysql_fetch_row(result)))
	{
		zone->AddGroundSpawnItem(atoul(row[0]), atoul(row[1]), atoi(row[2]), atoul(row[3]));
		LogWrite(GROUNDSPAWN__DEBUG, 5, "GSpawn", "---Loading GroundSpawn Items: ID: %u\n", atoul(row[0]));
		LogWrite(GROUNDSPAWN__DEBUG, 5, "GSpawn", "---item: %ul, rare: %i, grid: %ul", atoul(row[1]), atoi(row[2]), atoul(row[3]));
 		total++;
 	}
	LogWrite(GROUNDSPAWN__DEBUG, 0, "GSpawn", "--Loaded %i GroundSpawn Item%s.", total, total == 1 ? "" : "s");
}

void WorldDatabase::LoadGroundSpawnEntries(ZoneServer* zone) {
 	Query query;
 	MYSQL_ROW row;
 	int32 total = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT groundspawn_id, min_skill_level, min_adventure_level, bonus_table, harvest1, harvest3, harvest5, harvest_imbue, harvest_rare, harvest10, harvest_coin FROM groundspawns WHERE enabled = 1;");
	while(result && (row = mysql_fetch_row(result)))
	{
		// this is getting ridonkulous...
		LogWrite(GROUNDSPAWN__DEBUG, 5, "GSpawn", "---Loading GroundSpawn ID: %u\n" \
			"---min_skill_level: %i, min_adventure_level: %i, bonus_table: %i\n" \
			"---harvest1: %.2f, harvest3: %.2f, harvest5: %.2f\n" \
			"---harvest_imbue: %.2f, harvest_rare: %.2f, harvest10: %.2f\n" \
			"---harvest_coin: %u", atoul(row[0]), atoi(row[1]), atoi(row[2]), atoi(row[3]), atof(row[4]), atof(row[5]), atof(row[6]), atof(row[7]), atof(row[8]), atof(row[9]), atoul(row[10]));

		zone->AddGroundSpawnEntry(atoul(row[0]), atoi(row[1]), atoi(row[2]), atoi(row[3]), atof(row[4]), atof(row[5]), atof(row[6]), atof(row[7]), atof(row[8]), atof(row[9]), atoul(row[10]));
 		total++;
 	}
	LogWrite(GROUNDSPAWN__DEBUG, 0, "GSpawn", "--Loaded %i GroundSpawn Entr%s.", total, total == 1 ? "y" : "ies");
	LoadGroundSpawnItems(zone);
}

bool WorldDatabase::LoadCharacterStats(int32 id, int32 account_id, Client* client)
{
	DatabaseResult result;

	if( database_new.Select(&result, "SELECT * FROM character_details WHERE char_id = %i LIMIT 0, 1", id) )
	{
		LogWrite(PLAYER__DEBUG, 0, "Player", "Loading character_details for '%s' (char_id: %u)", client->GetPlayer()->GetName(), id);

		while( result.Next() )
		{
			InfoStruct* info = client->GetPlayer()->GetInfoStruct();

			// we need stats assigned before we try to assign hp/power
			info->set_str_base(result.GetInt16Str("str"));
			info->set_sta_base(result.GetInt16Str("sta"));
			info->set_agi_base(result.GetInt16Str("agi"));
			info->set_wis_base(result.GetInt16Str("wis"));
			info->set_intel_base(result.GetInt16Str("intel"));
			info->set_sta(info->get_sta_base());
			info->set_agi(info->get_agi_base());
			info->set_str(info->get_str_base());
			info->set_wis(info->get_wis_base());
			info->set_intel(info->get_intel_base());
			
			// must have totals up top before we set the current 'hp' / 'power'
			client->GetPlayer()->CalculatePlayerHPPower();
			
			client->GetPlayer()->SetHP(result.GetSInt32Str("hp"));
			client->GetPlayer()->SetPower(result.GetSInt32Str("power"));
			info->set_max_concentration_base(result.GetInt8Str("max_concentration"));
			
			if (info->get_max_concentration_base() == 0)
				info->set_max_concentration_base(5);

			info->set_attack_base(result.GetInt16Str("attack"));
			info->set_mitigation_base(result.GetInt16Str("mitigation"));
			info->set_avoidance_base(result.GetInt16Str("avoidance"));
			info->set_parry_base(result.GetInt16Str("parry"));
			info->set_deflection_base(result.GetInt16Str("deflection"));
			info->set_block_base(result.GetInt16Str("block"));

			// old resist types
			info->set_heat_base(result.GetInt16Str("heat"));
			info->set_cold_base(result.GetInt16Str("cold"));
			info->set_magic_base(result.GetInt16Str("magic"));
			info->set_mental_base(result.GetInt16Str("mental"));
			info->set_divine_base(result.GetInt16Str("divine"));
			info->set_disease_base(result.GetInt16Str("disease"));
			info->set_poison_base(result.GetInt16Str("poison"));

			//

			info->set_coin_copper(result.GetInt32Str("coin_copper"));
			info->set_coin_silver(result.GetInt32Str("coin_silver"));
			info->set_coin_gold(result.GetInt32Str("coin_gold"));
			info->set_coin_plat(result.GetInt32Str("coin_plat"));
			const char* bio = result.GetStringStr("biography");
			if(bio && strlen(bio) > 0)
				info->set_biography(std::string(bio));
			info->set_status_points(result.GetInt32Str("status_points"));
			client->GetPlayer()->GetPlayerInfo()->SetBindZone(result.GetInt32Str("bind_zone_id"));
			client->GetPlayer()->GetPlayerInfo()->SetBindX(result.GetFloatStr("bind_x"));
			client->GetPlayer()->GetPlayerInfo()->SetBindY(result.GetFloatStr("bind_y"));
			client->GetPlayer()->GetPlayerInfo()->SetBindZ(result.GetFloatStr("bind_z"));
			client->GetPlayer()->GetPlayerInfo()->SetBindHeading(result.GetFloatStr("bind_heading"));
			client->GetPlayer()->GetPlayerInfo()->SetHouseZone(result.GetInt32Str("house_zone_id"));
			client->GetPlayer()->SetAssignedAA(result.GetInt16Str("assigned_aa"));
			client->GetPlayer()->SetUnassignedAA(result.GetInt16Str("unassigned_aa"));
			client->GetPlayer()->SetTradeskillAA(result.GetInt16Str("tradeskill_aa"));
			client->GetPlayer()->SetUnassignedTradeskillAA(result.GetInt16Str("unassigned_tradeskill_aa"));
			client->GetPlayer()->SetPrestigeAA(result.GetInt16Str("prestige_aa"));
			client->GetPlayer()->SetUnassignedPrestigeAA(result.GetInt16Str("unassigned_prestige_aa"));
			client->GetPlayer()->SetTradeskillPrestigeAA(result.GetInt16Str("tradeskill_prestige_aa"));
			client->GetPlayer()->SetUnassignedTradeskillPrestigeAA(result.GetInt16Str("unassigned_tradeskill_prestige_aa"));
			info->set_xp(result.GetInt32Str("xp"));

			info->set_xp_needed(result.GetInt32Str("xp_needed"));

			if(info->get_xp_needed()== 0)
				client->GetPlayer()->SetNeededXP();

			info->set_xp_debt(result.GetFloatStr("xp_debt"));
			info->set_xp_vitality(result.GetFloatStr("xp_vitality"));
			info->set_ts_xp(result.GetInt32Str("tradeskill_xp"));
			info->set_ts_xp_needed(result.GetInt32Str("tradeskill_xp_needed"));

			if (info->get_ts_xp_needed() == 0)
				client->GetPlayer()->SetNeededTSXP();

			info->set_tradeskill_xp_vitality(result.GetFloatStr("tradeskill_xp_vitality"));
			info->set_bank_coin_copper(result.GetInt32Str("bank_copper"));
			info->set_bank_coin_silver(result.GetInt32Str("bank_silver"));
			info->set_bank_coin_gold(result.GetInt32Str("bank_gold"));
			info->set_bank_coin_plat(result.GetInt32Str("bank_plat"));

			client->GetPlayer()->SetCombatVoice(result.GetInt16Str("combat_voice"));
			client->GetPlayer()->SetEmoteVoice(result.GetInt16Str("emote_voice"));
			client->GetPlayer()->SetBiography(result.GetStringStr("biography"));
			client->GetPlayer()->GetInfoStruct()->set_flags(result.GetInt32Str("flags"));
			client->GetPlayer()->GetInfoStruct()->set_flags2(result.GetInt32Str("flags2"));
			client->GetPlayer()->SetLastName(result.GetStringStr("last_name"));

			// new resist types
			info->set_elemental_base(result.GetInt16Str("elemental"));
			info->set_arcane_base(result.GetInt16Str("arcane"));
			info->set_noxious_base(result.GetInt16Str("noxious"));
			// new savagery and dissonance
			client->GetPlayer()->SetSavagery(result.GetSInt16Str("savagery"));
			client->GetPlayer()->SetDissonance(result.GetSInt16Str("dissonance"));
			client->GetPlayer()->SetTotalSavageryBase(client->GetPlayer()->GetTotalSavagery());
			client->GetPlayer()->SetTotalDissonanceBase(client->GetPlayer()->GetTotalDissonance());
			
			client->GetPlayer()->SetCurrentLanguage(result.GetInt32Str("current_language"));
			
			std::string petName = result.GetStringStr("pet_name");
			boost::algorithm::to_lower(petName);
			
			if(petName != "no pet") { // some reason we default to "no pet", the code handles an empty string as setting a random pet name when its summoned
				client->GetPlayer()->GetInfoStruct()->set_pet_name(result.GetStringStr("pet_name"));
			}
		}

		return true;
	}
	else
	{
		LogWrite(PLAYER__ERROR, 0, "Player", "Error loading character_details for '%s' (char_id: %u)", client->GetPlayer()->GetName(), id);
		return false;
	}
}

bool WorldDatabase::loadCharacter(const char* ch_name, int32 account_id, Client* client){
	Query query, query4;
	MYSQL_ROW row, row4;
	int32 id = 0;
	query.escaped_name = getEscapeString(ch_name);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, current_zone_id, x, y, z, heading, admin_status, race, model_type, class, deity, level, gender, tradeskill_class, tradeskill_level, wing_type, hair_type, chest_type, legs_type, soga_wing_type, soga_hair_type, soga_chest_type, soga_legs_type, 0xFFFFFFFF - crc32(name), facial_hair_type, soga_facial_hair_type, instance_id, group_id, last_saved, DATEDIFF(curdate(), created_date) as accage, alignment, first_world_login, zone_duplicating_id FROM characters where name='%s' and account_id=%i AND deleted = 0", query.escaped_name, account_id);
	// no character found
	if ( result == NULL ) {
		LogWrite(PLAYER__ERROR, 0, "Player", "Error loading character for '%s'", ch_name);
		return false;
	}

	if (mysql_num_rows(result) == 1){
		row = mysql_fetch_row(result);

		id = strtoul(row[0], NULL, 0);
		LogWrite(PLAYER__DEBUG, 0, "Player", "Loading character for '%s' (char_id: %u)", ch_name, id);

		client->SetCharacterID(id);
		client->GetPlayer()->SetCharacterID(id);
		client->SetAccountID(account_id);
		client->GetPlayer()->SetName(ch_name);
		client->GetPlayer()->SetX(atof(row[2]));
		client->GetPlayer()->SetY(atof(row[3]));
		client->GetPlayer()->SetZ(atof(row[4]));
		client->GetPlayer()->SetHeading(atof(row[5]));
		client->SetAdminStatus(atoi(row[6]));
		client->GetPlayer()->SetRace(atoi(row[7]));
		client->GetPlayer()->SetModelType(atoi(row[8]));
		client->GetPlayer()->SetAdventureClass(atoi(row[9]));
		client->GetPlayer()->SetDeity(atoi(row[10]));
		client->GetPlayer()->SetLevel(atoi(row[11]));
		
		client->GetPlayer()->SetGender(atoi(row[12]));
		client->GetPlayer()->SetTradeskillClass(atoi(row[13]));
		client->GetPlayer()->SetTSLevel(atoi(row[14]));

		client->GetPlayer()->features.wing_type = atoi(row[15]);
		client->GetPlayer()->features.hair_type = atoi(row[16]);
		client->GetPlayer()->features.chest_type = atoi(row[17]);
		client->GetPlayer()->features.legs_type = atoi(row[18]);

		client->GetPlayer()->features.wing_type = atoi(row[19]);
		client->GetPlayer()->features.soga_hair_type = atoi(row[20]);
		client->GetPlayer()->features.soga_chest_type = atoi(row[21]);
		client->GetPlayer()->features.soga_legs_type = atoi(row[22]);
		client->SetNameCRC(atoul(row[23]));
		client->GetPlayer()->features.hair_face_type = atoi(row[24]);
		client->GetPlayer()->features.soga_hair_face_type = atoi(row[25]);
		int32 instanceid = atoi(row[26]);

		int32 groupid = atoi(row[27]);
		client->SetRejoinGroupID(groupid);

		int32 zoneid = atoul(row[1]);
/*
JA Notes on SOGA: I think there are many more settings to add than were commented out here,
because I can load a SOGA model player, but some features are missing (Barbarian WOAD, Skin tons, etc)
SOGA chars looked ok in LoginServer screen tho... odd.
*/

		// load character instances here
		if ( LoadCharacterInstances(client) )
			client->UpdateCharacterInstances();

		int32 zone_duplicate_id = atoul(row[32]);
		InstanceData* data = client->GetPlayer()->GetCharacterInstances()->FindInstanceByZoneID(zoneid);
		// housing doesn't have a data pointer here is why the data check was removed.. hmm
		if (instanceid > 0)
			client->SetCurrentZoneByInstanceID(instanceid, zoneid);
		else
			client->SetCurrentZone(zoneid, zone_duplicate_id);

		int32 lastsavedtime = atoi(row[28]);
		client->SetLastSavedTimeStamp(lastsavedtime);

		if (row[29])
			client->GetPlayer()->GetPlayerInfo()->SetAccountAge(atoi(row[29]));

		client->GetPlayer()->GetInfoStruct()->set_alignment(atoi(row[30]));
		
		client->GetPlayer()->GetInfoStruct()->set_first_world_login(atoi(row[31]));

		LoadCharacterFriendsIgnoreList(client->GetPlayer());
		MYSQL_RES* result4 = query4.RunQuery2(Q_SELECT, "SELECT `guild_id` FROM `guild_members` WHERE `char_id`=%u", id);
		if (result4 && (row4 = mysql_fetch_row(result4))) {
			Guild* guild = guild_list.GetGuild(atoul(row4[0]));
			if (guild) {
				client->GetPlayer()->SetGuild(guild);
				string subtitle;
				subtitle.append("<").append(guild->GetName()).append(">");
				client->GetPlayer()->SetSubTitle(subtitle.c_str());
			}
		}

		LoadCharacterHistory(id, client->GetPlayer());
		LoadCharacterLUAHistory(id, client->GetPlayer());
		LoadPlayerStatistics(client->GetPlayer(), id);
		LoadPlayerCollections(client->GetPlayer());
		LoadPlayerRecipes(client->GetPlayer());
		//LoadPlayerAchievements(client->GetPlayer());
		LoadPlayerAchievementsUpdates(client->GetPlayer());
		LoadAppearances(client->GetCurrentZone(), client);
		return LoadCharacterStats(id, account_id, client);
	}

	// should not be here...
	LogWrite(PLAYER__ERROR, 0, "Player", "Error loading character for '%s'", ch_name);
	return false;
}
std::string WorldDatabase::loadCharacterFromLogin(ZoneChangeDetails* details, int32 char_id, int32 account_id) {
	std::string name("");
		Query query;
		MYSQL_ROW row;
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT name, current_zone_id, instance_id FROM characters where id=%u and account_id=%u AND deleted = 0", char_id, account_id);
	// no character found
	if ( result == NULL ) {
		LogWrite(PLAYER__ERROR, 0, "Player", "Error loading character from login for char id '%u'", char_id);
		return name;
	}

	if (mysql_num_rows(result) == 1){
		row = mysql_fetch_row(result);

		int32 current_zone_id = atoul(row[1]);
		int32 instance_id = atoul(row[2]);
		LogWrite(PLAYER__DEBUG, 0, "Player", "Loading character from login for '%s' (char_id: %u)", row[0], char_id);
		
		int32 success = 0;
		details->peerId = std::string("");
		details->instanceId = instance_id;
		details->zoneId = current_zone_id;
		if(instance_id || current_zone_id) {
			if(!instance_id) {
				if((zone_list.GetZone(details, current_zone_id, "", false, false, false, true)))
					success = 1;
			}
			else {
				if((zone_list.GetZoneByInstance(details, instance_id, current_zone_id, false, false, false, true)))
					success = 1;
			}
		}
		if(success) {
			name = std::string(row[0]);
		}
		return name;
	}
	return name;
}

void WorldDatabase::LoadCharacterQuestRewards(Client* client) {
		Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT indexed, quest_id, is_temporary, is_collection, has_displayed, tmp_coin, tmp_status, description FROM character_quest_rewards where char_id = %u ORDER BY indexed asc", client->GetCharacterID());
	int8 count = 0;
	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			int32 index = atoul(row[0]);
			int32 quest_id = atoul(row[1]);

			bool is_temporary = atoul(row[2]);

			bool is_collection = atoul(row[3]);

			bool has_displayed = atoul(row[4]);

			
		int64 tmp_coin = 0;
#ifdef WIN32
			tmp_coin = _strtoui64(row[5], NULL, 10);
#else
			tmp_coin = strtoull(row[5], 0, 10);
#endif


			int32 tmp_status = atoul(row[6]);
			
			std::string description = std::string("");
			
			if(row[7]) {
				std::string description = std::string(row[7]);
			}
			
			if(is_collection) { 			
				map<int32, Collection*>* collections = client->GetPlayer()->GetCollectionList()->GetCollections();
				map<int32, Collection*>::iterator itr;
				Collection* collection = 0;
				for (itr = collections->begin(); itr != collections->end(); itr++) {
					collection = itr->second;
				if (collection->GetIsReadyToTurnIn()) {
						client->GetPlayer()->SetPendingCollectionReward(collection);
						break;
					}
				}
			}
			if(is_temporary) {
				LoadCharacterQuestTemporaryRewards(client, quest_id);
			}
			client->QueueQuestReward(quest_id, is_temporary, is_collection, has_displayed, tmp_coin, tmp_status, description, true, index);
			count++;
		}
	}
	
	if(count) {
		client->SetQuestUpdateState(true);
	}
}


void WorldDatabase::LoadCharacterQuestTemporaryRewards(Client* client, int32 quest_id) {
		Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, count FROM character_quest_temporary_rewards where char_id = %u and quest_id = %u", client->GetCharacterID(), quest_id);
	int8 count = 0;
	if(result)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			int32 item_id = atoul(row[0]);
			int16 item_count = atoul(row[1]);
			client->GetPlayer()->AddQuestTemporaryReward(quest_id, item_id, item_count);
		}
	}
}

bool WorldDatabase::InsertCharacterStats(int32 character_id, int8 class_id, int8 race_id){
	Query query1;
	Query query2;
	Query query3;
	Query query4;
	Query query5;

	/* Blank record */
	query1.RunQuery2(Q_INSERT, "INSERT INTO `character_details` (`char_id`) VALUES (%u)", character_id);

	/* Using the class id and race id */
	query2.RunQuery2(Q_UPDATE, "UPDATE character_details c, starting_details s SET c.max_hp = s.max_hp, c.hp = s.max_hp, c.max_power = s.max_power, c.power = s.max_power, c.str = s.str, c.sta = s.sta, c.agi = s.agi, c.wis = s.wis, c.intel = s.intel,c.heat = s.heat, c.cold = s.cold, c.magic = s.magic, c.mental = s.mental, c.divine = s.divine, c.disease = s.disease, c.poison = s.poison, c.coin_copper = s.coin_copper, c.coin_silver = s.coin_silver, c.coin_gold = s.coin_gold, c.coin_plat = s.coin_plat, c.status_points = s.status_points WHERE s.race_id = %d AND class_id = %d AND char_id = %u", race_id, class_id, character_id);
	if (query2.GetAffectedRows() > 0)
		return true;

	/* Using the class id and race id = 255 */
	query3.RunQuery2(Q_UPDATE, "UPDATE character_details c, starting_details s SET c.max_hp = s.max_hp, c.hp = s.max_hp, c.max_power = s.max_power, c.power = s.max_power, c.str = s.str, c.sta = s.sta, c.agi = s.agi, c.wis = s.wis, c.intel = s.intel,c.heat = s.heat, c.cold = s.cold, c.magic = s.magic, c.mental = s.mental, c.divine = s.divine, c.disease = s.disease, c.poison = s.poison, c.coin_copper = s.coin_copper, c.coin_silver = s.coin_silver, c.coin_gold = s.coin_gold, c.coin_plat = s.coin_plat, c.status_points = s.status_points WHERE s.race_id = 255 AND class_id = %d AND char_id = %u", class_id, character_id);
	if (query3.GetAffectedRows() > 0)
		return true;

	/* Using class id = 255 and the race id */
	query4.RunQuery2(Q_UPDATE, "UPDATE character_details c, starting_details s SET c.max_hp = s.max_hp, c.hp = s.max_hp, c.max_power = s.max_power, c.power = s.max_power, c.str = s.str, c.sta = s.sta, c.agi = s.agi, c.wis = s.wis, c.intel = s.intel,c.heat = s.heat, c.cold = s.cold, c.magic = s.magic, c.mental = s.mental, c.divine = s.divine, c.disease = s.disease, c.poison = s.poison, c.coin_copper = s.coin_copper, c.coin_silver = s.coin_silver, c.coin_gold = s.coin_gold, c.coin_plat = s.coin_plat, c.status_points = s.status_points WHERE s.race_id = %d AND class_id = 255 AND char_id = %u", race_id, character_id);
	if (query4.GetAffectedRows() > 0)
		return true;

	/* Using class id = 255 and race id = 255 */
	query5.RunQuery2(Q_UPDATE, "UPDATE character_details c, starting_details s SET c.max_hp = s.max_hp, c.hp = s.max_hp, c.max_power = s.max_power, c.power = s.max_power, c.str = s.str, c.sta = s.sta, c.agi = s.agi, c.wis = s.wis, c.intel = s.intel,c.heat = s.heat, c.cold = s.cold, c.magic = s.magic, c.mental = s.mental, c.divine = s.divine, c.disease = s.disease, c.poison = s.poison, c.coin_copper = s.coin_copper, c.coin_silver = s.coin_silver, c.coin_gold = s.coin_gold, c.coin_plat = s.coin_plat, c.status_points = s.status_points WHERE s.race_id = 255 AND class_id = 255 AND char_id = %u", character_id);
	if (query5.GetAffectedRows() > 0)
		return true;

	return false;
}

int32 WorldDatabase::GetCharacterTimeStamp(int32 character_id, int32 account_id,bool* char_exists){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT unix_timestamp FROM characters where id=%i and account_id=%i",character_id,account_id);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		*char_exists = true;
		return atoi(row[0]); // Return timestamp
	}
	else
		LogWrite(WORLD__ERROR, 0, "World", "Error in GetCharacterTimeStamp query '%s': %s", query.GetQuery(), query.GetError());

	*char_exists = false;
	return 0;
}

int32 WorldDatabase::GetCharacterTimeStamp(int32 character_id) {
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *result = query.RunQuery2(Q_SELECT, "SELECT unix_timestamp FROM characters WHERE id=%u", character_id);
	int32 ret = 0;

	if (result && (row = mysql_fetch_row(result)))
		ret = atoul(row[0]);

	return ret;
}

bool WorldDatabase::UpdateCharacterTimeStamp(int32 account_id, int32 character_id, int32 timestamp_update){
	Query query;
	string update_charts = string("update characters set unix_timestamp=%i where id=%i and account_id=%i");
	query.RunQuery2(Q_UPDATE, update_charts.c_str(),timestamp_update,character_id,account_id);
	if(!query.GetAffectedRows())
	{
		LogWrite(WORLD__ERROR, 0, "World", "Error in UpdateCharacterTimeStamp query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}

	return true;
}

bool WorldDatabase::insertCharacterProperty(Client* client, char* propName, char* propValue) {
	Query query, query2;

	string update_status = string("update character_properties set propvalue='%s' where charid=%i and propname='%s'");
	query.RunQuery2(Q_UPDATE, update_status.c_str(), propValue, client->GetCharacterID(), propName);
	if (!query.GetAffectedRows())
	{
		query2.RunQuery2(Q_UPDATE, "insert into character_properties (charid, propname, propvalue) values(%i, '%s', '%s')", client->GetCharacterID(), propName, propValue);
		if (query2.GetErrorNumber() && query2.GetError() && query2.GetErrorNumber() < 0xFFFFFFFF) {
			LogWrite(WORLD__ERROR, 0, "World", "Error in insertCharacterProperty query '%s': %s", query.GetQuery(), query.GetError());
			return false;
		}
	}
	return true;
}

bool WorldDatabase::loadCharacterProperties(Client* client) {
	Query query;
	MYSQL_ROW row;
	int32 id = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT propname, propvalue FROM character_properties where charid = %i", client->GetCharacterID());
	// no character found
	if (result == NULL) {
		LogWrite(PLAYER__ERROR, 0, "Player", "Error loading character properties for '%s'", client->GetPlayer()->GetName());
		return false;
	}

	while (result && (row = mysql_fetch_row(result))) {
		char* prop_name = row[0];
		char* prop_value = row[1];

		if (!prop_name || !prop_value)
			continue;

		if (!stricmp(prop_name, CHAR_PROPERTY_SPEED))
		{
			float new_speed = atof(prop_value);
			client->GetPlayer()->SetSpeed(new_speed, true);
			client->GetPlayer()->SetCharSheetChanged(true);
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_FLYMODE))
		{
			int8 flymode = atoul(prop_value);
			if (flymode) // avoid fly mode notification unless enabled
				ClientPacketFunctions::SendFlyMode(client, flymode, false);
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_INVUL))
		{
			int8 invul = atoul(prop_value);
			client->GetPlayer()->SetInvulnerable(invul == 1);
			if (client->GetPlayer()->GetInvulnerable())
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are now invulnerable!");
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_GMVISION))
		{
			int8 val = atoul(prop_value);
			client->GetPlayer()->SetGMVision(val == 1);
			client->GetCurrentZone()->SendAllSpawnsForVisChange(client, false);
			if (val)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "GM Vision Enabled!");
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_REGIONDEBUG))
		{
			int8 val = atoul(prop_value);

			client->SetRegionDebug(val == 1);
			if (val)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Region Debug Enabled!");
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_LUADEBUG))
		{
			int8 val = atoul(prop_value);
			if (val)
			{
				client->SetLuaDebugClient(true);
				if (lua_interface)
					lua_interface->UpdateDebugClients(client);

				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You will now receive LUA error messages.");
			}
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_GROUPLOOTMETHOD))
		{
			int8 val = atoul(prop_value);
			client->GetPlayer()->GetInfoStruct()->set_group_loot_method(val);
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_GROUPLOOTITEMRARITY))
		{
			int8 val = atoul(prop_value);
			client->GetPlayer()->GetInfoStruct()->set_group_loot_items_rarity(val);
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_GROUPAUTOSPLIT))
		{
			int8 val = atoul(prop_value);
			client->GetPlayer()->GetInfoStruct()->set_group_auto_split(val);
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_GROUPDEFAULTYELL))
		{
			int8 val = atoul(prop_value);
			client->GetPlayer()->GetInfoStruct()->set_group_default_yell(val);
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_GROUPAUTOLOCK))
		{
			int8 val = atoul(prop_value);
			client->GetPlayer()->GetInfoStruct()->set_group_autolock(val);
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_GROUPSOLOAUTOLOCK))
		{
			int8 val = atoul(prop_value);
			client->GetPlayer()->GetInfoStruct()->set_group_solo_autolock(val);
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_AUTOLOOTMETHOD))
		{
			int8 val = atoul(prop_value);
			client->GetPlayer()->GetInfoStruct()->set_group_auto_loot_method(val);
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_GROUPLOCKMETHOD))
		{
			int8 val = atoul(prop_value);
			client->GetPlayer()->GetInfoStruct()->set_group_lock_method(val);
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_ASSISTAUTOATTACK))
		{
			int8 val = atoul(prop_value);
			client->GetPlayer()->GetInfoStruct()->set_assist_auto_attack(val);
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_SETACTIVEFOOD))
		{
			int32 val = atoul(prop_value);
			client->GetPlayer()->SetActiveFoodUniqueID(val, false);
		}
		else if (!stricmp(prop_name, CHAR_PROPERTY_SETACTIVEDRINK))
		{
			int32 val = atoul(prop_value);
			client->GetPlayer()->SetActiveDrinkUniqueID(val, false);
		}
	}

	return true;
}

//gets the name FROM the db with the right letters in caps
string WorldDatabase::GetPlayerName(char* name){
	Query query;
	string ret = "";
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT name FROM characters where name='%s'", getSafeEscapeString(name).c_str());
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		if(row[0])
			ret = string(row[0]);
	}
	return ret;
}

int32 WorldDatabase::GetCharacterID(const char* name) {
	int32 id = 0;
	Query query;
	if (name) {
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `id` FROM `characters` WHERE `name`='%s'", name);
		if (result && mysql_num_rows(result) > 0) {
			MYSQL_ROW row;
			row = mysql_fetch_row(result);
			id = atoul(row[0]);
		}
	}
	return id;
}

int32 WorldDatabase::GetCharacterCurrentZoneID(int32 character_id) {
	int32 id = 0;
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `current_zone_id` FROM `characters` WHERE `id`=%u", character_id);
	if (result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		id = atoul(row[0]);
	}
	return id;
}

int32 WorldDatabase::GetCharacterAccountID(int32 character_id) {
	int32 id = 0;
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `account_id` FROM `characters` WHERE `id`=%u", character_id);
	if (result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		id = atoul(row[0]);
	}
	return id;
}

sint16 WorldDatabase::GetHighestCharacterAdminStatus(int32 account_id){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT max(admin_status) FROM characters where account_id=%i ",account_id);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		if ( row[0] != NULL )
			return atoi(row[0]); // Return characters status
		else
			return 0;
	}

	return 0;
}

sint16 WorldDatabase::GetLowestCharacterAdminStatus(int32 account_id){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT min(admin_status) FROM characters where account_id=%i ",account_id);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		if ( row[0] != NULL )
			return atoi(row[0]); // Return characters status
		else
			return 0;
	}

	return 0;
}

sint16 WorldDatabase::GetCharacterAdminStatus(char* character_name){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT admin_status FROM characters where name='%s'", getSafeEscapeString(character_name).c_str());
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		return atoi(row[0]); // Return characters level
	}
	else
		LogWrite(WORLD__ERROR, 0, "World", "Error in GetCharacterAdminStatus query '%s': %s", query.GetQuery(), query.GetError());

	return -10;
}

sint16 WorldDatabase::GetCharacterAdminStatus(int32 account_id , int32 char_id){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT admin_status FROM characters where account_id=%i and id=%i",account_id,char_id);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		return atoi(row[0]); // Return characters status
	}
	else{
		Query query2;
		result = query2.RunQuery2(Q_SELECT, "SELECT count(id) FROM characters where account_id=%i and id=%i",account_id,char_id);
		if(result && mysql_num_rows(result) > 0) {
			MYSQL_ROW row;
			row = mysql_fetch_row(result);
			if(atoi(row[0]) == 0) //old character, needs to be deleted FROM login server
				return -10;
			return -8;
		}
		else
			LogWrite(WORLD__ERROR, 0, "World", "Error in GetCharacterAdminStatus query '%s': %s", query.GetQuery(), query.GetError());
	}
	return PLAY_ERROR_PROBLEM;
}

bool WorldDatabase::UpdateAdminStatus(char* character_name, sint16 flag){
	Query query;
	string update_status = string("update characters set admin_status=%i where name='%s'");
	query.RunQuery2(Q_UPDATE, update_status.c_str(),flag,character_name);
	if(!query.GetAffectedRows())
	{
		LogWrite(WORLD__ERROR, 0, "World", "Error in UpdateAdminStatus query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}

	return true;
}

void WorldDatabase::SaveCharacterFloats(int32 char_id, const char* type, float float1, float float2, float float3, float multiplier){
	Query query;
	string create_char = string("insert into char_colors (char_id, type, red, green, blue, signed_value) values(%i,'%s',%i,%i,%i, 1)");
	query.RunQuery2(Q_INSERT, create_char.c_str(), char_id, type, (sint8)(float1*multiplier), (sint8)(float2*multiplier), (sint8)(float3*multiplier));
	if(query.GetError() && strlen(query.GetError()) > 0){
		LogWrite(WORLD__ERROR, 0, "World", "Error in SaveCharacterFloats query '%s': %s", query.GetQuery(), query.GetError());
	}
}

void WorldDatabase::SaveCharacterColors(int32 char_id, const char* type, EQ2_Color color){
	Query query;
	string create_char = string("insert into char_colors (char_id, type, red, green, blue) values(%i,'%s',%i,%i,%i)");
	query.RunQuery2(Q_INSERT, create_char.c_str(), char_id, type, color.red, color.green, color.blue);
	if(query.GetError() && strlen(query.GetError()) > 0){
		LogWrite(WORLD__ERROR, 0, "World", "Error in SaveCharacterColors query '%s': %s", query.GetQuery(), query.GetError());
	}
}

void WorldDatabase::SaveNPCAppearanceEquipment(int32 spawn_id, int8 slot_id, int16 type, int8 red, int8 green, int8 blue, int8 hred, int8 hgreen, int8 hblue){
	Query query;
	string appearance = string("INSERT INTO npc_appearance_equip (spawn_id, slot_id, equip_type, red, green, blue, highlight_red, highlight_green, highlight_blue) values (%i, %i, %i, %i, %i, %i, %i, %i, %i) ON DUPLICATE KEY UPDATE equip_type=%i, red=%i, green=%i, blue=%i, highlight_red=%i, highlight_green=%i, highlight_blue=%i");
	query.RunQuery2(Q_INSERT, appearance.c_str(), spawn_id, slot_id, type, red, green, blue, hred, hgreen, hblue, type, red, green, blue, hred, hgreen, hblue);
	if(query.GetError() && strlen(query.GetError()) > 0){
		LogWrite(WORLD__ERROR, 0, "World", "Error in SaveNPCAppearanceEquipment query '%s': %s", query.GetQuery(), query.GetError());
	}
}

int32 WorldDatabase::LoadNPCAppearanceEquipmentData(ZoneServer* zone){
	Query query;
	MYSQL_ROW row;

	int32 spawn_id = 0, new_spawn_id = 0, count = 0;
	NPC* npc = 0;
	int8 slot = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT spawn_id, slot_id, equip_type, red, green, blue, highlight_red, highlight_green, highlight_blue FROM npc_appearance_equip ORDER BY spawn_id");
	while(result && (row = mysql_fetch_row(result))){
		new_spawn_id = atoul(row[0]);
		if(new_spawn_id != spawn_id){
			npc = zone->GetNPC(new_spawn_id, true);
			if(!npc)
				continue;
			if(spawn_id > 0)
				count++;
			spawn_id = new_spawn_id;
		}
		slot = atoul(row[1]);
		if(slot < NUM_SLOTS){
			npc->SetEquipment(slot, atoul(row[2]), atoul(row[3]), atoul(row[4]), atoul(row[5]), atoul(row[6]), atoul(row[7]), atoul(row[8]));
		}
	}
	if(query.GetError() && strlen(query.GetError()) > 0)
		LogWrite(WORLD__ERROR, 0, "World", "Error in LoadNPCAppearanceEquipmentData query '%s': %s", query.GetQuery(), query.GetError());
	return count;
}

int16 WorldDatabase::GetAppearanceID(string name){
	int32 id = 0;
	Query query;
	MYSQL_ROW row;
	query.escaped_name = getEscapeString(name.c_str());
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT appearance_id FROM appearances where name='%s'", query.escaped_name);
	if(result && mysql_num_rows(result) == 1){
		row = mysql_fetch_row(result);
		id = atoi(row[0]);
	}
	return id;
}

vector<int16>* WorldDatabase::GetAppearanceIDsLikeName(string name, bool filtered) {
	vector<int16>* ids = 0;
	Query query;
	MYSQL_ROW row;
	query.escaped_name = getEscapeString(name.c_str());
	MYSQL_RES* result;
	if (filtered)
		result = query.RunQuery2(Q_SELECT, "SELECT `appearance_id` FROM `appearances` WHERE `name` RLIKE '%s' AND `name` NOT RLIKE 'ghost' AND `name` NOT RLIKE 'headless' AND `name` NOT RLIKE 'elemental' AND `name` NOT RLIKE 'test' AND `name` NOT RLIKE 'zombie' AND `name` NOT RLIKE 'vampire'", query.escaped_name);
	else
		result = query.RunQuery2(Q_SELECT, "SELECT `appearance_id` FROM `appearances` WHERE `name` RLIKE '%s' AND `name` NOT RLIKE 'ghost' AND `name`", query.escaped_name);
	while (result && (row = mysql_fetch_row(result))) {
		if (!ids)
			ids = new vector<int16>;
		ids->push_back(atoi(row[0]));
	}
	return ids;
}

string WorldDatabase::GetAppearanceName(int16 appearance_id) {
	Query query;
	MYSQL_ROW row;
	string name;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `name` FROM `appearances` WHERE `appearance_id`=%u", appearance_id);
	if (result && (row = mysql_fetch_row(result)))
		name = string(row[0]);
	return name;
}

void WorldDatabase::UpdateRandomize(int32 spawn_id, sint32 value) {
	Query query;
	query.RunQuery2(Q_UPDATE, "UPDATE `spawn_npcs` SET `randomize`=`randomize` + %i WHERE `spawn_id`=%u", value, spawn_id);
}

int32 WorldDatabase::SaveCharacter(PacketStruct* create, int32 loginID){
	Query query;
	int8 race_id = create->getType_int8_ByName("race");
	int8 orig_class_id = create->getType_int8_ByName("class");//Normal server
	int8 class_id = orig_class_id;
	if ( create->GetVersion() <= 546 ) {
		class_id = 0; //Classic Server Only
	}
	
	create->PrintPacket();
	
	int8 gender_id = create->getType_int8_ByName("gender");
	sint16 auto_admin_status = 0;

	// fetch rules related to setting auto-admin status for server
	bool auto_admin_players = rule_manager.GetGlobalRule(R_World, AutoAdminPlayers)->GetBool();
	bool auto_admin_gm = rule_manager.GetGlobalRule(R_World, AutoAdminGMs)->GetBool();
	
	/* 
		The way I think this is supposed to work :) is if any of your chars are already GM, and AutoAdminGMs rule is true,
		set the new character's admin_status to your highest status for any character active on your loginID.
			- If status > 0, new character > 0
			- If status = 0, new character = 0
			- If status < 0, new character < 0, too... even if auto_admin_gm is true

		If we're not a GM (status = 0) but AutoAdminPlayers rule is true,
		set the new character's admin_status to the default set in AutoAdminStatusValue rule.

		Else, if both rules are False, set everyone to 0 like normal.

	*/

	auto_admin_status = GetHighestCharacterAdminStatus(loginID);

	if( auto_admin_status > 0 && auto_admin_gm ) {
		LogWrite(WORLD__WARNING, 0, "World", "New character '%s' granted GM status (%i) from accountID: %i", create->getType_EQ2_16BitString_ByName("name").data.c_str(), auto_admin_status, loginID);
	}
	else if( auto_admin_players )
	{
		auto_admin_status = rule_manager.GetGlobalRule(R_World, AutoAdminStatusValue)->GetSInt16();
		LogWrite(WORLD__DEBUG, 0, "World", "New character '%s' granted AutoAdminPlayer status: %i", create->getType_EQ2_16BitString_ByName("name").data.c_str(), auto_admin_status);
	}
	else {
		auto_admin_status = 0;
	}

	string create_char = string("Insert into characters (account_id, server_id, name, race, class, gender, deity, body_size, body_age, soga_wing_type, soga_chest_type, soga_legs_type, soga_hair_type, soga_model_type, legs_type, chest_type, wing_type, hair_type, model_type, facial_hair_type, soga_facial_hair_type, created_date, last_saved, admin_status, first_world_login) values(%i, %i, '%s', %i, %i, %i, %i, %f, %f, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, now(), unix_timestamp(), %i, 1)");

	query.RunQuery2(Q_INSERT, create_char.c_str(), 
						loginID, 
						create->getType_int32_ByName("server_id"), 
						create->getType_EQ2_16BitString_ByName("name").data.c_str(), 
						race_id, 
						class_id,
						gender_id, 
						create->getType_int8_ByName("deity"), 
						create->getType_float_ByName("body_size"), 
						create->getType_float_ByName("body_age"), 
						GetAppearanceID(create->getType_EQ2_16BitString_ByName("soga_wing_file").data),
						GetAppearanceID(create->getType_EQ2_16BitString_ByName("soga_chest_file").data), 
						GetAppearanceID(create->getType_EQ2_16BitString_ByName("soga_legs_file").data), 
						GetAppearanceID(create->getType_EQ2_16BitString_ByName("soga_hair_file").data),
						GetAppearanceID(create->getType_EQ2_16BitString_ByName("soga_race_file").data), 
						GetAppearanceID(create->getType_EQ2_16BitString_ByName("legs_file").data), 
						GetAppearanceID(create->getType_EQ2_16BitString_ByName("chest_file").data),
						GetAppearanceID(create->getType_EQ2_16BitString_ByName("wing_file").data), 
						GetAppearanceID(create->getType_EQ2_16BitString_ByName("hair_file").data), 
						GetAppearanceID(create->getType_EQ2_16BitString_ByName("race_file").data), 
						GetAppearanceID(create->getType_EQ2_16BitString_ByName("face_file").data), 
						GetAppearanceID(create->getType_EQ2_16BitString_ByName("soga_face_file").data),
						auto_admin_status);

	if(query.GetError() && strlen(query.GetError()) > 0)
	{
		LogWrite(PLAYER__ERROR, 0, "Player", "Error in SaveCharacter query '%s': %s", query.GetQuery(), query.GetError());
		return 0;
	}

	int32 last_insert_id = query.GetLastInsertedID();
	int32 char_id = last_insert_id;
	UpdateStartingFactions(char_id, create->getType_int8_ByName("starting_zone"));
	UpdateStartingZone(char_id, class_id, race_id, create);
	UpdateStartingItems(char_id, class_id, race_id);
	UpdateStartingSkills(char_id, class_id, race_id);
	UpdateStartingSpells(char_id, class_id, race_id);
	UpdateStartingSkillbar(char_id, class_id, race_id);
	UpdateStartingTitles(char_id, class_id, race_id, gender_id);
	InsertCharacterStats(char_id, class_id, race_id);
	UpdateStartingLanguage(char_id, race_id, create->getType_int8_ByName("starting_zone"));
	
	LoadClaimItems(char_id); //insert claim items, from claim_items to the character_ version.

	AddNewPlayerToServerGuild(loginID, char_id);

	if (create->GetVersion() <= 561) {
		float classic_multiplier = 250.0f;
		SaveCharacterFloats(char_id, "skin_color", create->getType_float_ByName("skin_color", 0), create->getType_float_ByName("skin_color", 1), create->getType_float_ByName("skin_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "eye_color", create->getType_float_ByName("eye_color", 0), create->getType_float_ByName("eye_color", 1), create->getType_float_ByName("eye_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_color1", create->getType_float_ByName("hair_color1", 0), create->getType_float_ByName("hair_color1", 1), create->getType_float_ByName("hair_color1", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_color2", create->getType_float_ByName("hair_color2", 0), create->getType_float_ByName("hair_color2", 1), create->getType_float_ByName("hair_color2", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_highlight", create->getType_float_ByName("hair_highlight", 0), create->getType_float_ByName("hair_highlight", 1), create->getType_float_ByName("hair_highlight", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_type_color", create->getType_float_ByName("hair_type_color", 0), create->getType_float_ByName("hair_type_color", 1), create->getType_float_ByName("hair_type_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_type_highlight_color", create->getType_float_ByName("hair_type_highlight_color", 0), create->getType_float_ByName("hair_type_highlight_color", 1), create->getType_float_ByName("hair_type_highlight_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_type_color", create->getType_float_ByName("hair_type_color", 0), create->getType_float_ByName("hair_type_color", 1), create->getType_float_ByName("hair_type_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_type_highlight_color", create->getType_float_ByName("hair_type_highlight_color", 0), create->getType_float_ByName("hair_type_highlight_color", 1), create->getType_float_ByName("hair_type_highlight_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_face_color", create->getType_float_ByName("hair_face_color", 0), create->getType_float_ByName("hair_face_color", 1), create->getType_float_ByName("hair_face_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_face_highlight_color", create->getType_float_ByName("hair_face_highlight_color", 0), create->getType_float_ByName("hair_face_highlight_color", 1), create->getType_float_ByName("hair_face_highlight_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "shirt_color", create->getType_float_ByName("shirt_color", 0), create->getType_float_ByName("shirt_color", 1), create->getType_float_ByName("shirt_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "unknown_chest_color", create->getType_float_ByName("unknown_chest_color", 0), create->getType_float_ByName("unknown_chest_color", 1), create->getType_float_ByName("unknown_chest_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "pants_color", create->getType_float_ByName("pants_color", 0), create->getType_float_ByName("pants_color", 1), create->getType_float_ByName("pants_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "unknown_legs_color", create->getType_float_ByName("unknown_legs_color", 0), create->getType_float_ByName("unknown_legs_color", 1), create->getType_float_ByName("unknown_legs_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "unknown9", create->getType_float_ByName("unknown9", 0), create->getType_float_ByName("unknown9", 1), create->getType_float_ByName("unknown9", 2), classic_multiplier);
	}
	else {
		SaveCharacterColors(char_id, "skin_color", create->getType_EQ2_Color_ByName("skin_color"));
		SaveCharacterColors(char_id, "model_color", create->getType_EQ2_Color_ByName("model_color"));
		SaveCharacterColors(char_id, "eye_color", create->getType_EQ2_Color_ByName("eye_color"));
		SaveCharacterColors(char_id, "hair_color1", create->getType_EQ2_Color_ByName("hair_color1"));
		SaveCharacterColors(char_id, "hair_color2", create->getType_EQ2_Color_ByName("hair_color2"));
		SaveCharacterColors(char_id, "hair_highlight", create->getType_EQ2_Color_ByName("hair_highlight"));
		SaveCharacterColors(char_id, "hair_type_color", create->getType_EQ2_Color_ByName("hair_type_color"));
		SaveCharacterColors(char_id, "hair_type_highlight_color", create->getType_EQ2_Color_ByName("hair_type_highlight_color"));
		SaveCharacterColors(char_id, "hair_face_color", create->getType_EQ2_Color_ByName("hair_face_color"));
		SaveCharacterColors(char_id, "hair_face_highlight_color", create->getType_EQ2_Color_ByName("hair_face_highlight_color"));
		SaveCharacterColors(char_id, "wing_color1", create->getType_EQ2_Color_ByName("wing_color1"));
		SaveCharacterColors(char_id, "wing_color2", create->getType_EQ2_Color_ByName("wing_color2"));
		SaveCharacterColors(char_id, "shirt_color", create->getType_EQ2_Color_ByName("shirt_color"));
		SaveCharacterColors(char_id, "unknown_chest_color", create->getType_EQ2_Color_ByName("unknown_chest_color"));
		SaveCharacterColors(char_id, "pants_color", create->getType_EQ2_Color_ByName("pants_color"));
		SaveCharacterColors(char_id, "unknown_legs_color", create->getType_EQ2_Color_ByName("unknown_legs_color"));
		SaveCharacterColors(char_id, "unknown9", create->getType_EQ2_Color_ByName("unknown9"));		

		SaveCharacterColors(char_id, "soga_skin_color", create->getType_EQ2_Color_ByName("soga_skin_color"));
		SaveCharacterColors(char_id, "soga_model_color", create->getType_EQ2_Color_ByName("soga_model_color"));
		SaveCharacterColors(char_id, "soga_eye_color", create->getType_EQ2_Color_ByName("soga_eye_color"));
		SaveCharacterColors(char_id, "soga_hair_color1", create->getType_EQ2_Color_ByName("soga_hair_color1"));
		SaveCharacterColors(char_id, "soga_hair_color2", create->getType_EQ2_Color_ByName("soga_hair_color2"));
		SaveCharacterColors(char_id, "soga_hair_highlight", create->getType_EQ2_Color_ByName("soga_hair_highlight"));
		SaveCharacterColors(char_id, "soga_hair_type_color", create->getType_EQ2_Color_ByName("soga_hair_type_color"));
		SaveCharacterColors(char_id, "soga_hair_type_highlight_color", create->getType_EQ2_Color_ByName("soga_hair_type_highlight_color"));
		SaveCharacterColors(char_id, "soga_hair_face_color", create->getType_EQ2_Color_ByName("soga_hair_face_color"));
		SaveCharacterColors(char_id, "soga_hair_face_highlight_color", create->getType_EQ2_Color_ByName("soga_hair_face_highlight_color"));
		SaveCharacterColors(char_id, "soga_wing_color1", create->getType_EQ2_Color_ByName("soga_wing_color1"));
		SaveCharacterColors(char_id, "soga_wing_color2", create->getType_EQ2_Color_ByName("soga_wing_color2"));
		SaveCharacterColors(char_id, "soga_shirt_color", create->getType_EQ2_Color_ByName("soga_shirt_color"));
		SaveCharacterColors(char_id, "soga_unknown_chest_color", create->getType_EQ2_Color_ByName("soga_unknown_chest_color"));
		SaveCharacterColors(char_id, "soga_pants_color", create->getType_EQ2_Color_ByName("soga_pants_color"));
		SaveCharacterColors(char_id, "soga_unknown_legs_color", create->getType_EQ2_Color_ByName("soga_unknown_legs_color"));
		SaveCharacterColors(char_id, "soga_unknown13", create->getType_EQ2_Color_ByName("soga_unknown13"));
		SaveCharacterFloats(char_id, "soga_eye_type", create->getType_float_ByName("soga_eyes2", 0), create->getType_float_ByName("soga_eyes2", 1), create->getType_float_ByName("soga_eyes2", 2));
		SaveCharacterFloats(char_id, "soga_ear_type", create->getType_float_ByName("soga_ears", 0), create->getType_float_ByName("soga_ears", 1), create->getType_float_ByName("soga_ears", 2));
		SaveCharacterFloats(char_id, "soga_eye_brow_type", create->getType_float_ByName("soga_eye_brows", 0), create->getType_float_ByName("soga_eye_brows", 1), create->getType_float_ByName("soga_eye_brows", 2));
		SaveCharacterFloats(char_id, "soga_cheek_type", create->getType_float_ByName("soga_cheeks", 0), create->getType_float_ByName("soga_cheeks", 1), create->getType_float_ByName("soga_cheeks", 2));
		SaveCharacterFloats(char_id, "soga_lip_type", create->getType_float_ByName("soga_lips", 0), create->getType_float_ByName("soga_lips", 1), create->getType_float_ByName("soga_lips", 2));
		SaveCharacterFloats(char_id, "soga_chin_type", create->getType_float_ByName("soga_chin", 0), create->getType_float_ByName("soga_chin", 1), create->getType_float_ByName("soga_chin", 2));
		SaveCharacterFloats(char_id, "soga_nose_type", create->getType_float_ByName("soga_nose", 0), create->getType_float_ByName("soga_nose", 1), create->getType_float_ByName("soga_nose", 2));
	}
	SaveCharacterFloats(char_id, "eye_type", create->getType_float_ByName("eyes2", 0), create->getType_float_ByName("eyes2", 1), create->getType_float_ByName("eyes2", 2));
	SaveCharacterFloats(char_id, "ear_type", create->getType_float_ByName("ears", 0), create->getType_float_ByName("ears", 1), create->getType_float_ByName("ears", 2));
	SaveCharacterFloats(char_id, "eye_brow_type", create->getType_float_ByName("eye_brows", 0), create->getType_float_ByName("eye_brows", 1), create->getType_float_ByName("eye_brows", 2));
	SaveCharacterFloats(char_id, "cheek_type", create->getType_float_ByName("cheeks", 0), create->getType_float_ByName("cheeks", 1), create->getType_float_ByName("cheeks", 2));
	SaveCharacterFloats(char_id, "lip_type", create->getType_float_ByName("lips", 0), create->getType_float_ByName("lips", 1), create->getType_float_ByName("lips", 2));
	SaveCharacterFloats(char_id, "chin_type", create->getType_float_ByName("chin", 0), create->getType_float_ByName("chin", 1), create->getType_float_ByName("chin", 2));
	SaveCharacterFloats(char_id, "nose_type", create->getType_float_ByName("nose", 0), create->getType_float_ByName("nose", 1), create->getType_float_ByName("nose", 2));
	SaveCharacterFloats(char_id, "body_size", create->getType_float_ByName("body_size", 0), 0, 0);
	return char_id;
}

int8 WorldDatabase::CheckNameFilter(const char* name, int8 min_length, int8 max_length) {
	// the minimum 4 is enforced by the client too
	if(!name || strlen(name) < min_length || strlen(name) > max_length) // Even 20 char length is long...
		return BADNAMELENGTH_REPLY;
	uchar* checkname = (uchar*)name;
	for (int32 i = 0; i < strlen(name); i++)
	{
		if(!alpha_check(checkname[i]))
			return NAMEINVALID_REPLY;
	}
	Query query;
	LogWrite(WORLD__DEBUG, 0, "World", "Name check on: %s", name);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT count(*) FROM characters WHERE name='%s'",name);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		if(row[0] != 0 && atoi(row[0]) > 0)
			return NAMETAKEN_REPLY;
	}
	else
		LogWrite(WORLD__ERROR, 0, "World", "Error in CheckNameFilter (name exist check) (Name query '%s': %s", query.GetQuery(), query.GetError());

	Query query3;
	LogWrite(WORLD__DEBUG, 0, "World", "Name check on: %s (Bots table)", name);
	MYSQL_RES* result3 = query3.RunQuery2(Q_SELECT, "SELECT count(*) FROM bots WHERE name='%s'", name);
	if (result3 && mysql_num_rows(result3) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result3);
		if (row[0] != 0 && atoi(row[0]) > 0)
			return NAMETAKEN_REPLY;
	}
	else
		LogWrite(WORLD__ERROR, 0, "World", "Error in CheckNameFilter (name exist check, bot table) (Name query '%s': %s", query3.GetQuery(), query3.GetError());



	Query query2;
	MYSQL_RES* result2 = query2.RunQuery2(Q_SELECT, "SELECT count(*) FROM name_filter WHERE '%s' like name",name);
	if(result2 && mysql_num_rows(result2) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result2);
		if(row[0] != 0 && atoi(row[0]) > 0)
			return NAMEFILTER_REPLY;
		else if(row[0] != 0 && atoi(row[0]) == 0)
			return CREATESUCCESS_REPLY;
	}
	else
		LogWrite(WORLD__ERROR, 0, "World", "Error in CheckNameFilter (name_filter check) query '%s': %s", query.GetQuery(), query.GetError());

	return UNKNOWNERROR_REPLY;
}

char* WorldDatabase::GetCharacterName(int32 character_id){

	LogWrite(WORLD__TRACE, 9, "World", "Enter: %s", __FUNCTION__);

	Query query;
	char* name = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT name FROM characters where id=%u",character_id);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		if(row[0] && strlen(row[0]) > 0)
		{
			name = new char[strlen(row[0])+1];
			memset(name,0, strlen(row[0])+1);
			strcpy(name, row[0]);
		}
	}
	else
		LogWrite(WORLD__ERROR, 0, "World", "Error in GetCharacterName query '%s': %s", query.GetQuery(), query.GetError());

	LogWrite(WORLD__TRACE, 9, "World", "Exit: %s", __FUNCTION__);
	return name;
}

int8 WorldDatabase::GetCharacterLevel(int32 character_id){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT level FROM characters where id=%u",character_id);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		return atoi(row[0]); // Return characters level
	}
	else
		LogWrite(WORLD__ERROR, 0, "World", "Error in GetCharacterLevel query '%s': %s", query.GetQuery(), query.GetError());

	return 0;
}

int16 WorldDatabase::GetCharacterModelType(int32 character_id){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT model_type FROM characters where id=%u",character_id);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		return atoi(row[0]); // Return characters race
	}
	else
		LogWrite(WORLD__ERROR, 0, "World", "Error in GetCharacterModelType query '%s': %s", query.GetQuery(), query.GetError());

	return 0;
}

int8 WorldDatabase::GetCharacterClass(int32 character_id){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT class FROM characters where id=%u",character_id);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		return atoi(row[0]); // Return characters class
	}
	else
		LogWrite(WORLD__ERROR, 0, "World", "Error in GetCharacterClass query '%s': %s", query.GetQuery(), query.GetError());

	return 0;
}

int8 WorldDatabase::GetCharacterGender(int32 character_id){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT gender FROM characters where id=%u",character_id);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		return atoi(row[0]); // Return characters gender
	}
	else
		LogWrite(WORLD__ERROR, 0, "World", "Error in GetCharacterGender query '%s': %s", query.GetQuery(), query.GetError());

	return 0;
}

void WorldDatabase::DeleteCharacterQuest(int32 quest_id, int32 char_id, bool repeated_quest) {
	if (repeated_quest) {
		if (!database_new.Query("UPDATE `character_quests` SET `given_date` = `completed_date` WHERE `char_id` = %u AND `quest_id` = %u", char_id, quest_id))
			LogWrite(DATABASE__ERROR, 0, "DBNew", "Error (%u) in DeleteCharacterQuest query:\n%s", database_new.GetError(), database_new.GetErrorMsg());
	}
	else {
		if (!database_new.Query("DELETE FROM `character_quests` WHERE `char_id` = %u AND `quest_id` = %u", char_id, quest_id))
			LogWrite(DATABASE__ERROR, 0, "DBNew", "Error (%u) in DeleteCharacterQuest query:\n%s", database_new.GetError(), database_new.GetErrorMsg());
	}

	if (!database_new.Query("DELETE FROM `character_quest_progress` WHERE `char_id` = %u AND `quest_id` = %u", char_id, quest_id))
		LogWrite(DATABASE__ERROR, 0, "DBNew", "Error (%u) in DeleteCharacterQuest query:\n%s", database_new.GetError(), database_new.GetErrorMsg());
}

void WorldDatabase::SaveCharacterSkills(Client* client){
	vector<Skill*>* skills = client->GetPlayer()->GetSkills()->GetSaveNeededSkills();
	if(skills){
		Query query;
		if(skills->size() > 0){
			Skill* skill = 0;
			for(int32 i=0;i<skills->size();i++){
				skill = skills->at(i);
				query.AddQueryAsync(client->GetCharacterID(),this,Q_REPLACE, "replace into character_skills (char_id, skill_id, current_val, max_val) values(%u, %u, %i, %i)", client->GetCharacterID(), skill->skill_id, skill->current_val, skill->max_val);
			}
			if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF)
				LogWrite(WORLD__ERROR, 0, "World", "Error in SaveCharacterSkills query '%s': %s", query.GetQuery(), query.GetError());
		}
		safe_delete(skills);
	}
}

void WorldDatabase::SaveCharacterQuests(Client* client){
	Query query;
	map<int32, Quest*>::iterator itr;
	master_quest_list.LockQuests(); //prevent reloading until we are done
	client->GetPlayer()->MPlayerQuests.writelock(__FUNCTION__, __LINE__); //prevent all quest modifications until we are done
	map<int32, Quest*>* quests = client->GetPlayer()->GetPlayerQuests();
	for(itr = quests->begin(); itr != quests->end(); itr++){
		if(client->GetCurrentQuestID() == itr->first){
			query.AddQueryAsync(client->GetCharacterID(),this,Q_UPDATE, "update character_quests set current_quest = 0 where char_id = %u", client->GetCharacterID());
			query.AddQueryAsync(client->GetCharacterID(), this,Q_UPDATE, "update character_quests set current_quest = 1 where char_id = %u and quest_id = %u", client->GetCharacterID(), itr->first);
		}
		if(itr->second && itr->second->GetSaveNeeded()){
			query.AddQueryAsync(client->GetCharacterID(), this,Q_INSERT, "insert ignore into character_quests (char_id, quest_id, given_date, quest_giver) values(%u, %u, now(), %u)", client->GetCharacterID(), itr->first, itr->second->GetQuestGiver());
			query.AddQueryAsync(client->GetCharacterID(), this,Q_UPDATE, "update character_quests set tracked = %i, quest_flags = %u, hidden = %i, complete_count = %u where char_id = %u and quest_id = %u", itr->second->IsTracked() ? 1 : 0, itr->second->GetQuestFlags(), itr->second->IsHidden() ? 1 : 0, itr->second->GetCompleteCount(), client->GetCharacterID(), itr->first);
			SaveCharacterQuestProgress(client, itr->second);
			itr->second->SetSaveNeeded(false);
		}
	}
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF)
		LogWrite(WORLD__ERROR, 0, "World", "Error in SaveCharacterQuests query '%s': %s", query.GetQuery(), query.GetError());
	quests = client->GetPlayer()->GetCompletedPlayerQuests();
	for(itr = quests->begin(); itr != quests->end(); itr++){
		if(itr->second && itr->second->GetSaveNeeded()){
			query.AddQueryAsync(client->GetCharacterID(), this,Q_DELETE, "delete FROM character_quest_progress where char_id = %u and quest_id = %u", client->GetCharacterID(), itr->first);

			/* incase the quest is completed before the quest could be inserted in the PlayerQuests loop, we first try to insert it.  If it already exists then we can just update
			 * the completed_date */
			query.AddQueryAsync(client->GetCharacterID(), this,Q_INSERT, "INSERT INTO character_quests (char_id, quest_id, quest_giver, current_quest, given_date, completed_date, complete_count) values (%u,%u,%u,0, now(),now(), %u) ON DUPLICATE KEY UPDATE completed_date = now(), complete_count = %u, current_quest = 0", client->GetCharacterID(), itr->first, itr->second->GetQuestGiver(), itr->second->GetCompleteCount(), itr->second->GetCompleteCount());
			itr->second->SetSaveNeeded(false);
		}
	}
	client->GetPlayer()->MPlayerQuests.releasewritelock(__FUNCTION__, __LINE__);
	master_quest_list.UnlockQuests();

}

void WorldDatabase::SaveCharRepeatableQuest(Client* client, int32 quest_id, int16 quest_complete_count) {
	if (!database_new.Query("UPDATE `character_quests` SET `given_date` = now(), complete_count = %u WHERE `char_id` = %u AND `quest_id` = %u", quest_complete_count, client->GetCharacterID(), quest_id))
		LogWrite(DATABASE__ERROR, 0, "DBNew", "DB Error %u\n%s", database_new.GetError(), database_new.GetErrorMsg());
}

void WorldDatabase::SaveCharacterQuestProgress(Client* client, Quest* quest){
	vector<QuestStep*>* steps = quest->GetQuestSteps();
	vector<QuestStep*>::iterator itr;
	QuestStep* step = 0;
	quest->MQuestSteps.readlock(__FUNCTION__, __LINE__);
	if(steps){
		for(itr = steps->begin(); itr != steps->end(); itr++){
			step = *itr;
			if(step && step->GetQuestCurrentQuantity() > 0) {
				Query query;
				query.RunQuery2(Q_REPLACE, "replace into character_quest_progress (char_id, quest_id, step_id, progress) values(%u, %u, %u, %i)", client->GetCharacterID(), quest->GetQuestID(), step->GetStepID(), step->GetQuestCurrentQuantity());
							
				if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF)
					LogWrite(WORLD__ERROR, 0, "World", "Error in SaveCharacterQuestProgress query '%s': %s", query.GetQuery(), query.GetError());
			}
		}
	}
	quest->MQuestSteps.releasereadlock(__FUNCTION__, __LINE__);
}

void WorldDatabase::LoadCharacterQuestProgress(Client* client){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT character_quest_progress.quest_id, step_id, progress FROM character_quest_progress, character_quests where character_quest_progress.char_id=%u and character_quest_progress.quest_id = character_quests.quest_id and character_quest_progress.char_id = character_quests.char_id ORDER BY character_quest_progress.quest_id",client->GetCharacterID());
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		Quest* quest = 0;
		int32 quest_id = 0;
		map<int32, int32>* progress_map = new map<int32, int32>();
		while(result && (row = mysql_fetch_row(result))){
			if(quest_id != atoul(row[0])){
				if(quest_id > 0){
					quest = client->GetPlayer()->GetQuest(quest_id);
					if(quest)
						client->SetPlayerQuest(quest, progress_map);
				}
				quest_id = atoul(row[0]);
				progress_map->clear();
			}
			(*progress_map)[atoul(row[1])] = atoul(row[2]);
		}
		if(progress_map->size() > 0){
			quest = client->GetPlayer()->GetQuest(quest_id);
			if(quest)
				client->SetPlayerQuest(quest, progress_map);
		}
		safe_delete(progress_map);
	}
	else if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF)
		LogWrite(WORLD__ERROR, 0, "World", "Error in LoadCharacterQuestProgress query '%s': %s", query.GetQuery(), query.GetError());
}

void WorldDatabase::LoadCharacterQuests(Client* client){
	LogWrite(PLAYER__DEBUG, 0, "Player", "Loading Character Quests...");
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT quest_id, DAY(given_date), MONTH(given_date), YEAR(given_date), DAY(completed_date), MONTH(completed_date), YEAR(completed_date), quest_giver, tracked, quest_flags, hidden, UNIX_TIMESTAMP(given_date), UNIX_TIMESTAMP(completed_date), complete_count FROM character_quests WHERE char_id=%u ORDER BY current_quest", client->GetCharacterID());
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		Quest* quest = 0;
		while(result && (row = mysql_fetch_row(result))){
			quest = master_quest_list.GetQuest(atoul(row[0]));
			if(quest) {

				LogWrite(PLAYER__DEBUG, 5, "Player", "\tLoading quest_id: %u", atoul(row[0]));
				bool addQuest = true;

				if(row[4] && atoi(row[4]) > 0){
					quest->SetTurnedIn(true);
					if(row[4])
						quest->SetDay(atoi(row[4]));
					if(row[5])
						quest->SetMonth(atoi(row[5]));
					if(row[6] && atoi(row[6]) > 2000)
						quest->SetYear(atoi(row[6]) - 2000);
					client->GetPlayer()->AddCompletedQuest(quest);

					// Added timestamps to quickly compare given and completed dates
					int32 given_timestamp = atoul(row[11]);
					int32 completed_timestamp = atoul(row[12]);

					// If given timestamp is greater then completed then this is a repeatable quest we are working on
					// so get a fresh quest object to add as an active quest
					if (given_timestamp > completed_timestamp)
					{
						lua_interface->SetLuaUserDataStale(quest);
						safe_delete(quest);
						quest = master_quest_list.GetQuest(atoul(row[0]));
					}
					else
						addQuest = false;

					quest->SetCompleteCount(atoi(row[13]));
				}

				if (addQuest) {
					if(row[1])
						quest->SetDay(atoi(row[1]));
					if(row[2])
						quest->SetMonth(atoi(row[2]));
					if(row[3] && atoi(row[3]) > 2000)
						quest->SetYear(atoi(row[3]) - 2000);
					quest->SetQuestGiver(atoul(row[7]));
					quest->SetTracked(atoi(row[8]) == 1 ? true : false);
					quest->SetQuestFlags(atoul(row[9]));
					quest->SetHidden(atoi(row[10]) == 1 ? true : false);
					client->AddPlayerQuest(quest, false, false);
				}
				quest->SetSaveNeeded(false);

				// Changed this to call reload with step = 0 for all quests and not
				// just quests with quest flags to allow customized set up if needed
				if (lua_interface)
					lua_interface->CallQuestFunction(quest, "Reload", client->GetPlayer(), 0);
			}
		}
		LoadCharacterQuestProgress(client);
	}
}

void WorldDatabase::LoadCharacterFriendsIgnoreList(Player* player) {
	if (player) {
		Query query;
		MYSQL_ROW row;
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `name`, `type` FROM `character_social` WHERE `char_id`=%u", player->GetCharacterID());
		while (result && (row = mysql_fetch_row(result))) {
			if (strncmp(row[1], "FRIEND", 6) == 0)
				player->AddFriend(row[0], false);
			else
				player->AddIgnore(row[0], false);
		}
	}
}

void WorldDatabase::LoadZoneInfo(ZoneServer* zone, int32 minLevel, int32 maxLevel, int32 avgLevel, int32 firstLevel){
	Query query;
	int32 ruleset_id;
	zone->setGroupRaidLevels(minLevel, maxLevel, avgLevel, firstLevel);
	
	char* escaped = getEscapeString(zone->GetZoneName());
	std::shared_ptr<ZoneInfoMemory> zoneInfo = world.GetZoneInfoByName(escaped);
	
	MYSQL_RES* result = nullptr;
	
	if(zoneInfo == nullptr)
		result = query.RunQuery2(Q_SELECT, "SELECT id, file, description, underworld, safe_x, safe_y, safe_z, min_status, min_level, max_level, instance_type+0, shutdown_timer, zone_motd, default_reenter_time, default_reset_time, default_lockout_time, force_group_to_zone, safe_heading, xp_modifier, ruleset_id, expansion_id, weather_allowed, sky_file, can_bind, can_gate, city_zone, can_evac FROM zones where name='%s'",escaped);
	if((zoneInfo || (result && mysql_num_rows(result) > 0))) {
		if(result && zoneInfo == nullptr) 
		{	
			MYSQL_ROW row;
			row = mysql_fetch_row(result);
		
			zoneInfo = std::make_shared<ZoneInfoMemory>();
			zoneInfo->LoadFromDatabaseRow(row);
			world.AddZoneInfo(zoneInfo->zoneID, zoneInfo);
		}
		
		if(!result && !zoneInfo) {
			LogWrite(ZONE__ERROR, 0, "Zones", "Failed to get zone info for %s.", escaped);
			safe_delete_array(escaped);
			return;
		}
		zone->SetZoneName(escaped);
		zone->SetZoneID(zoneInfo->zoneID);
		zone->SetZoneFile((char*)zoneInfo->zoneFile.c_str());
		zone->SetZoneDescription((char*)zoneInfo->zoneDescription.c_str());
		zone->SetUnderWorld(zoneInfo->underworld);
		zone->SetSafeX(zoneInfo->safeX);
		zone->SetSafeY(zoneInfo->safeY);
		zone->SetSafeZ(zoneInfo->safeZ);
		zone->SetMinimumStatus(zoneInfo->minimumVersion);
		zone->SetMinimumLevel(zoneInfo->minimumLevel);
		zone->SetMaximumLevel(zoneInfo->maximumLevel);
		zone->SetInstanceType(zoneInfo->instanceType);
		zone->SetShutdownTimer(zoneInfo->shutdownTime);
		zone->SetZoneMOTD(zoneInfo->zoneMotd);

		zone->SetDefaultReenterTime(zoneInfo->defReenterTime);
		zone->SetDefaultResetTime(zoneInfo->defResetTime);
		zone->SetDefaultLockoutTime(zoneInfo->defLockoutTime);
		zone->SetForceGroupZoneOption(zoneInfo->groupZoneOption);
		zone->SetSafeHeading(zoneInfo->safeHeading);
		zone->SetXPModifier(zoneInfo->xpModifier);

		// check data_version to see if client has proper expansion to enter a zone
		zone->SetMinimumVersion(zoneInfo->minimumVersion);
		zone->SetWeatherAllowed(zoneInfo->weatherAllowed);

		zone->SetZoneSkyFile((char*)zoneInfo->zoneSkyFile.c_str());

		if (zone->IsInstanceZone())
		{
			if ( zone->GetInstanceID() < 1 ) {
				zone->SetupInstance(CreateNewInstance(zone->GetZoneID(), minLevel, maxLevel, avgLevel, firstLevel));
				zone->setGroupRaidLevels(minLevel, maxLevel, avgLevel, firstLevel);
			}
			else {
				zone->SetupInstance(zone->GetInstanceID());
				LoadZonePlayerLevels(zone);
			}
		}
		zone->SetCanBind(zoneInfo->canBind);
		zone->SetCanGate(zoneInfo->canGate);
		zone->SetCityZone(zoneInfo->cityZone);
		zone->SetCanEvac(zoneInfo->canEvac);
	}
	safe_delete_array(escaped);
}


void WorldDatabase::LoadZonePlayerLevels(ZoneServer* zone) {
	Query query;
	int32 ruleset_id;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT player_minlevel, player_maxlevel, player_avglevel, player_firstlevel FROM instances WHERE id = %u and zone_id = %u", zone->GetInstanceID(), zone->GetZoneID());
	if (result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		zone->setGroupRaidLevels(atoul(row[0]), atoul(row[1]), atoul(row[2]), atoul(row[3]));
	}
}

void WorldDatabase::LoadZoneInfo(ZoneInfo* zone_info) {
	Query query;
	int32 ruleset_id;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT name, file, description, underworld, safe_x, safe_y, safe_z, min_status, min_level, max_level, instance_type, shutdown_timer, zone_motd, default_reenter_time, default_reset_time, default_lockout_time, force_group_to_zone, lua_script, xp_modifier, ruleset_id, expansion_id, always_loaded, city_zone, start_zone, zone_type, weather_allowed, sky_file FROM zones WHERE id = %u", zone_info->id);
	if (result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		strncpy(zone_info->name, row[0], sizeof(zone_info->name));
		strncpy(zone_info->file, row[1], sizeof(zone_info->file));
		strncpy(zone_info->description, row[2], sizeof(zone_info->description));
		zone_info->underworld = atof(row[3]);
		zone_info->safe_x = atof(row[4]);
		zone_info->safe_y = atof(row[5]);
		zone_info->safe_z = atof(row[6]);
		zone_info->min_status = atoi(row[7]);
		zone_info->min_level = atoi(row[8]);
		zone_info->max_level = atoi(row[9]);
		zone_info->instance_type = (atoi(row[10]) == 0) ? 0 : atoi(row[10]) - 1;
		zone_info->shutdown_timer = atoul(row[11]);
		row[12] == NULL ? strncpy(zone_info->zone_motd, "", sizeof(zone_info->zone_motd)) : strncpy(zone_info->zone_motd, row[12], sizeof(zone_info->zone_motd));
		zone_info->default_reenter_time = atoi(row[13]);
		zone_info->default_reset_time = atoi(row[14]);
		zone_info->default_lockout_time = atoi(row[15]);
		zone_info->force_group_to_zone = atoi(row[16]);
		row[17] == NULL ? strncpy(zone_info->lua_script, "", sizeof(zone_info->lua_script)) : strncpy(zone_info->lua_script, row[17], sizeof(zone_info->lua_script));
		zone_info->xp_modifier = atof(row[18]);
		zone_info->ruleset_id = atoul(row[19]);
		if ((ruleset_id = atoul(row[19])) > 0 && !rule_manager.SetZoneRuleSet(zone_info->id, ruleset_id))
			LogWrite(ZONE__ERROR, 0, "Zones", "Error setting rule set for zone '%s' (%u). A rule set with ID %u does not exist.", zone_info->name, zone_info->id, ruleset_id);

		zone_info->expansion_id = atoi(row[20]);
		zone_info->min_version = GetMinimumClientVersion(zone_info->expansion_id);
		zone_info->always_loaded	= atoi(row[21]);
		zone_info->city_zone		= atoi(row[22]);
		zone_info->start_zone		= atoi(row[23]);
		row[24] == NULL ? strncpy(zone_info->zone_type, "", sizeof(zone_info->zone_type)) : strncpy(zone_info->zone_type, row[24], sizeof(zone_info->zone_type));
		zone_info->weather_allowed	= atoi(row[25]);

		strncpy(zone_info->sky_file, row[26], sizeof(zone_info->sky_file));
	}
}

void WorldDatabase::SaveZoneInfo(int32 zone_id, const char* field, sint32 value) {
	Query query;
	query.RunQuery2(Q_UPDATE, "UPDATE `zones` SET `%s`=%i WHERE `id`=%u", field, value, zone_id);
}

void WorldDatabase::SaveZoneInfo(int32 zone_id, const char* field, float value) {
	Query query;
	query.RunQuery2(Q_UPDATE, "UPDATE `zones` SET `%s`=%f WHERE `id`=%u", field, value, zone_id);
}

void WorldDatabase::SaveZoneInfo(int32 zone_id, const char* field, const char* value) {
	Query query;
	query.RunQuery2(Q_UPDATE, "UPDATE `zones` SET `%s`='%s' WHERE `id`=%u", field, const_cast<char*>(getEscapeString(value)), zone_id);
}

int32 WorldDatabase::GetZoneID(const char* name) {
	int32 zone_id = 0;
	Query query;
	char* escaped = getEscapeString(name);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `id` FROM zones WHERE `name`=\"%s\"", escaped);
	if (result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		zone_id = atoi(row[0]);
	}
	safe_delete_array(escaped);
	return zone_id;
}

bool WorldDatabase::GetZoneRequirements(const char* zoneName, sint16* minStatus, int16* minLevel, int16* maxLevel, int16* minVersion) {	
	Query query;
	char* escaped = getEscapeString(zoneName);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT min_status, min_level, max_level, expansion_id FROM zones where name='%s'",escaped);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		sint16 status = (sint16)atoi(row[0]);
		int16 levelMin = (int16)atoi(row[1]);
		int16 levelMax = (int16)atoi(row[2]);
		int8 expansion_id = (int8)atoi(row[3]);

		*minStatus = status;
		*minLevel = levelMin;
		*maxLevel = levelMax;

		if( expansion_id >= 40 ) // lowest client we support is RoK - exp04
			*minVersion = GetMinimumClientVersion(expansion_id);
		else
			*minVersion = 0;

		safe_delete_array(escaped);
		return true;
	}
	safe_delete_array(escaped);

	return false;
}

int16 WorldDatabase::GetMinimumClientVersion(int8 expansion_id)
{
	/*
	1	n/a		Classic	Expansion
	2	adv01	Bloodline Chronicles
	3	adv02	Splitpaw Saga
	10	exp01	Desert of Flames
	20	exp02	Kingdom of Sky
	21	adv04	Fallen Dynasty
	30	exp03	Echoes of Faydwer
	40	exp04	Rise of Kunark
	50	exp05	The Shadow Odyssey
	60	exp06	Sentinel's Fate
	61	halas	Halas Reborn
	70	exp07	Destiny of Velious
	80	exp08	Age of Discovery
	*/

	int16 minVer = 0;

	// TODO: eventually replace this with reading values from eq2expansions table
	switch(expansion_id)
	{
		case 40: // ROK
			{
				minVer = 843;
				break;
			}
		case 50: // TSO
			{
				minVer = 908;
				break;
			}
		case 60: // SF
			{
				minVer = 1008;
				break;
			}
		case 61: // Halas
			{
				minVer = 1045;
				break;
			}
		case 70: // DoV
			{
				minVer = 1096;
				break;
			}
		case 80: // AoD
			{
				minVer = 1144;
				break;
			}
		case 90: // CoE
			{
				minVer = 1188;
				break;
			}
	}

	return minVer;
}

// returns Expansion Name depending on the connected client's data version
string WorldDatabase::GetExpansionIDByVersion(int16 version)
{
	/*
	0		n/a		Classic	Expansion
	0		adv01	Bloodline Chronicles
	0		adv02	Splitpaw Saga
	0		exp01	Desert of Flames
	0		exp02	Kingdom of Sky
	0		adv04	Fallen Dynasty
	0		exp03	Echoes of Faydwer
	843		exp04	Rise of Kunark
	908		exp05	The Shadow Odyssey
	1008	exp06	Sentinel's Fate
	1045	halas	Halas Reborn
	1096	exp07	Destiny of Velious
	1142	exp08	Age of Discovery
	1188	exp09	Chains of Eternity 
	9999	(and beyond)
	*/
	string ret = "";

	if( version >= 9999 )
		ret = "Unknown";
	else if( version >= 1188 )
		ret = "Chains of Eternity";
	else if( version >= 1142 )
		ret = "Age of Discovery";
	else if( version >= 1096 )
		ret = "Destiny of Velious";
	else if( version >= 1045 )
		ret = "Halas Reborn";
	else if( version >= 1008 )
		ret = "Sentinel's Fate";
	else if( version >= 908 )
		ret = "The Shadow Odyssey";
	else if( version >= 843 )
		ret = "Rise of Kunark";
	else
		ret = "Any";

	return ret;
}


void WorldDatabase::LoadSpecialZones(){
	LogWrite(ZONE__INFO, 0, "Zone", "Starting static zones...");
	Query query;
	ZoneServer* zone = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, name, always_loaded, peer_priority FROM zones where always_loaded = 1");
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		while(result && (row = mysql_fetch_row(result))){
			int16 peer_priority_req = 0;
			if(row[3]) {
				peer_priority_req = atoul(row[3]);
			}
			
			ZoneChangeDetails zone_details_peer;
			ZoneChangeDetails zone_details_self;
			bool gotZonePeer = zone_list.GetZone(&zone_details_peer, 0, std::string(row[1]), /*loadZone*/false, /*skip_existing_zones*/false, /*increment_zone*/false, /*check_peers*/true, /*check_instances*/false, 
											/*only_always_loaded*/true, /*skip_self*/true);
			bool gotZoneSelf = zone_list.GetZone(&zone_details_self, 0, std::string(row[1]), /*loadZone*/false, /*skip_existing_zones*/false, /*increment_zone*/false, /*check_peers*/false, /*check_instances*/false, 
											/*only_always_loaded*/true, /*skip_self*/false);
			bool hasPriorityPeer = peer_manager.hasPriorityPeer(peer_priority_req);
			if(!gotZonePeer && !gotZoneSelf && peer_priority_req == net.GetPeerPriority()) {
				LogWrite(ZONE__INFO, 0, "Zone", "Starting static zone %s.", row[1]);
				zone = new ZoneServer(row[1]);
				LoadZoneInfo(zone);
				zone->Init();

				zone->SetAlwaysLoaded(atoul(row[2]) == 1);
			}
			else if(!gotZonePeer && gotZoneSelf && zone_details_self.zonePtr && !((ZoneServer*)zone_details_self.zonePtr)->AlwaysLoaded()) {
				LogWrite(ZONE__INFO, 0, "Zone", "Making zone %s static as we lost peer to handle the zone.", row[1]);
				((ZoneServer*)zone_details_self.zonePtr)->SetAlwaysLoaded(true);
			}
			else if(gotZonePeer && gotZoneSelf) {
				std::shared_ptr<Peer> peer = peer_manager.getPeerById(zone_details_peer.peerId);
				if(peer && peer->healthCheck.status == HealthStatus::OK) {
					if(peer_priority_req == peer->peerPriority || peer->peerPriority < net.GetPeerPriority()) {
						ZoneServer* zone = (ZoneServer*)zone_details_self.zonePtr;
						if(zone) {
							LogWrite(ZONE__INFO, 0, "Zone", "Static zone %s will be spundown due to another peer taking over.", row[1]);
							zone->SetAlwaysLoaded(false);
							zone->SetDuplicatedZone(true);
							zone->SetDuplicatedID(zone_list.GetHighestDuplicateID(std::string(zone->GetZoneName()), zone->GetZoneID()));
						}
					}
				}
			}
			else if(!gotZonePeer && !gotZoneSelf && net.is_primary && (!hasPriorityPeer || peer_priority_req == USHRT_MAX)) {
				gotZonePeer = zone_list.GetZone(&zone_details_peer, 0, std::string(row[1]), /*loadZone*/true, /*skip_existing_zones*/true, /*increment_zone*/false, /*check_peers*/true, /*check_instances*/false, 
												/*only_always_loaded*/true, /*skip_self*/true);
				if(!gotZonePeer) {
					bool gotZoneSelf = zone_list.GetZone(&zone_details_self, 0, std::string(row[1]), /*loadZone*/true, /*skip_existing_zones*/true, /*increment_zone*/false, /*check_peers*/false, /*check_instances*/false, 
													/*only_always_loaded*/true, /*skip_self*/false);
				}
			}
		}
	}
}

bool WorldDatabase::SpawnGroupRemoveAssociation(int32 group1, int32 group2){
	Query query;
	query.RunQuery2(Q_DELETE, "delete FROM spawn_location_group_associations where (group_id1 = %u and group_id2 = %u) or (group_id1 = %u and group_id2 = %u)", group1, group2, group2, group1);
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(WORLD__ERROR, 0, "World", "Error in SpawnGroupRemoveSpawn query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	return true;
}

bool WorldDatabase::SpawnGroupAddAssociation(int32 group1, int32 group2){
	Query query;
	query.RunQuery2(Q_INSERT, "insert ignore into spawn_location_group_associations (group_id1, group_id2) values(%u, %u)", group1, group2);
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(WORLD__ERROR, 0, "World", "Error in SpawnGroupAddAssociation query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	return true;
}

bool WorldDatabase::SpawnGroupAddSpawn(Spawn* spawn, int32 group_id){
	Query query;
	query.RunQuery2(Q_INSERT, "insert ignore into spawn_location_group (group_id, placement_id) values(%u, %u)", group_id, spawn->GetSpawnLocationPlacementID());
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(WORLD__ERROR, 0, "World", "Error in SpawnGroupAddSpawn query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	return true;
}

bool WorldDatabase::SpawnGroupRemoveSpawn(Spawn* spawn, int32 group_id){
	Query query;
	query.RunQuery2(Q_DELETE, "delete FROM spawn_location_group where group_id = %u and placement_id = %u", group_id, spawn->GetSpawnLocationPlacementID());
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(WORLD__ERROR, 0, "World", "Error in SpawnGroupRemoveSpawn query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT count(group_id) FROM spawn_location_group where group_id = %u", group_id);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		if((row = mysql_fetch_row(result))){
			if(atoul(row[0]) == 0)
				DeleteSpawnGroup(group_id);
		}
	}
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(WORLD__ERROR, 0, "World", "Error in SpawnGroupRemoveSpawn query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	return true;
}

int32 WorldDatabase::CreateSpawnGroup(Spawn* spawn, string name)
{
	int32 group_id = 0;
	Query query;

	// JA: As of 0.7.1, DB Milestone 2, Content Team needs to use group_id's from Raw Data, so start any manual group_id's > 100,000
	query.RunQuery2(Q_INSERT, "INSERT INTO spawn_location_group (group_id, placement_id, name) SELECT IF(ISNULL(MAX(group_id))=1, 100000, MAX(group_id)+1), %u, '%s' FROM spawn_location_group", spawn->GetSpawnLocationPlacementID(), getSafeEscapeString(name.c_str()).c_str());
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF)
		return 0;
	
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT max(group_id) FROM spawn_location_group");
	if(result && mysql_num_rows(result) > 0)
	{
		MYSQL_ROW row;

		if((row = mysql_fetch_row(result)))
		{
			if(row[0])
				group_id = atoul(row[0]);
		}
	}

	return group_id;
}

void WorldDatabase::DeleteSpawnGroup(int32 id){
	Query query;
	query.RunQuery2(Q_DELETE, "delete FROM spawn_location_group where group_id = %u", id);
}

bool WorldDatabase::SetGroupSpawnChance(int32 id, float chance){
	Query query;
	query.RunQuery2(Q_UPDATE, "replace into spawn_location_group_chances (group_id, percentage) values(%u, %f)", id, chance);
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(WORLD__ERROR, 0, "World", "Error in SetGroupSpawnChance query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	return true;
}

int32 WorldDatabase::LoadSpawnGroupChances(ZoneServer* zone){
	Query query;
	int32 count = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT slgc.group_id, slgc.percentage FROM spawn_location_group_chances slgc, spawn_location_group slg, spawn_location_placement slp where slgc.group_id = slg.group_id and slg.placement_id = slp.id and slp.zone_id = %u", zone->GetZoneID());
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		int32 group_id = 0;
		float percent = 0;
		while(result && (row = mysql_fetch_row(result))){
			group_id = atoul(row[0]);
			percent = atof(row[1]);
			zone->AddSpawnGroupChance(group_id, percent);
			count++;
		}
	}
	return count;
}

int32 WorldDatabase::LoadSpawnLocationGroupAssociations(ZoneServer* zone){
	Query query;
	int32 count = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT distinct slga.group_id1, slga.group_id2 FROM spawn_location_group_associations slga, spawn_location_group slg, spawn_location_placement slp where (slg.group_id = slga.group_id1 or slg.group_id = slga.group_id2) and slg.placement_id = slp.id and slp.zone_id = %u", zone->GetZoneID());
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		while(result && (row = mysql_fetch_row(result))){
			zone->AddSpawnGroupAssociation(atoul(row[0]), atoul(row[1]));
			count++;
		}
	}
	return count;
}

int32 WorldDatabase::LoadSpawnLocationGroups(ZoneServer* zone){
	Query query;
	int32 count = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT slg.group_id, slg.placement_id, slp.spawn_location_id FROM spawn_location_group slg, spawn_location_placement slp WHERE slg.placement_id = slp.id and slp.zone_id = %u", zone->GetZoneID());
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		int32 placement_id = 0;
		int32 group_id = 0;
		int32 spawn_location_id = 0;
		while(result && (row = mysql_fetch_row(result))){
			group_id = atoul(row[0]);
			placement_id = atoul(row[1]);
			spawn_location_id = atoul(row[2]);
			zone->AddSpawnGroupLocation(group_id, placement_id, spawn_location_id);
			count++;
		}
	}
	return count;
}

int32 WorldDatabase::ProcessSpawnLocations(ZoneServer* zone, const char* sql_query, int8 type){
	int32 number = 0;
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, sql_query, zone->GetZoneID(), zone->GetInstanceID());
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		int32 spawn_location_id = 0xFFFFFFFF;
		SpawnLocation* spawn_location = 0;
		while(result && (row = mysql_fetch_row(result))){
			if((spawn_location_id == 0xFFFFFFFF) || atoul(row[0]) != spawn_location_id){
				if(spawn_location){
					zone->AddSpawnLocation(spawn_location_id, spawn_location);
					number++;
				}
				spawn_location = new SpawnLocation();
			}
			SpawnEntry* entry = new SpawnEntry;

			spawn_location_id = atoul(row[0]);
			entry->spawn_location_id = spawn_location_id;
			entry->spawn_entry_id = atoul(row[1]);
			entry->spawn_type = type;
			entry->spawn_id = atoul(row[9]);
			entry->spawn_percentage = atof(row[10]);
			entry->respawn = atoul(row[11]);
			entry->respawn_offset_low = atoi(row[12]);
			entry->respawn_offset_high = atoi(row[13]);
			entry->duplicated_spawn = atoul(row[14]);
			entry->expire_time = atoul(row[17]);
			entry->expire_offset = atoul(row[18]);
			//devn00b add stat overrides. Just a slight increase in size. Used in ZoneServer::AddNPCSpawn.
			entry->lvl_override = atoul(row[22]);
			entry->hp_override = atoul(row[23]);
			entry->mp_override = atoul(row[24]);
			entry->str_override = atoul(row[25]);
			entry->sta_override = atoul(row[26]);
			entry->wis_override = atoul(row[27]);
			entry->int_override = atoul(row[28]);
			entry->agi_override = atoul(row[29]);
			entry->heat_override = atoul(row[30]);
			entry->cold_override = atoul(row[31]);
			entry->magic_override = atoul(row[32]);
			entry->mental_override = atoul(row[33]);
			entry->divine_override = atoul(row[34]);
			entry->disease_override = atoul(row[35]);
			entry->poison_override = atoul(row[36]);
			entry->difficulty_override = atoul(row[37]);
			spawn_location->x = atof(row[2]);
			spawn_location->y = atof(row[3]);
			spawn_location->z = atof(row[4]);
			spawn_location->x_offset = atof(row[5]);
			spawn_location->y_offset = atof(row[6]);
			spawn_location->z_offset = atof(row[7]);
			spawn_location->heading = atof(row[8]);
			spawn_location->pitch = atof(row[19]);
			spawn_location->roll = atof(row[20]);
			spawn_location->conditional = atoi(row[21]);
			spawn_location->total_percentage += entry->spawn_percentage;
			spawn_location->grid_id = strtoul(row[15], NULL, 0);
			spawn_location->placement_id = strtoul(row[16], NULL, 0);
			spawn_location->AddSpawn(entry);

		}
		if(spawn_location){
			zone->AddSpawnLocation(spawn_location_id, spawn_location);
			number++;
		}
	}
	return number;
}

void WorldDatabase::ResetDatabase(){
	Query query;
	query.RunQuery2("delete FROM table_versions where name != 'table_versions'", Q_DELETE);
}

void WorldDatabase::EnableConstraints(){
	Query query;
	query.RunQuery2("/*!40101 SET SQL_MODE=@OLD_SQL_MODE */", Q_DBMS);
	query.RunQuery2("/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */", Q_DBMS);
}

void WorldDatabase::DisableConstraints(){
	Query query;
	query.RunQuery2("/*!40101 SET NAMES utf8 */", Q_DBMS);
	query.RunQuery2("/*!40101 SET SQL_MODE=''*/", Q_DBMS);
	query.RunQuery2("/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */", Q_DBMS);
	query.RunQuery2("/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */", Q_DBMS);
}

void WorldDatabase::LoadSpawns(ZoneServer* zone)
{
	Query query;
	int32 npcs = 0, objects = 0, widgets = 0, signs = 0, ground_spawns = 0, spawn_groups = 0, spawn_group_associations = 0, spawn_group_chances = 0;

	LogWrite(SPAWN__TRACE, 0, "Spawn", "Enter LoadSpawns");

	if(zone->GetInstanceID() == 0) {
		npcs = ProcessSpawnLocations(zone, "SELECT sln.id, sle.id, slp.x, slp.y, slp.z, slp.x_offset, slp.y_offset, slp.z_offset, slp.heading, sle.spawn_id, sle.spawnpercentage, slp.respawn, slp.respawn_offset_low, slp.respawn_offset_high, slp.duplicated_spawn, slp.grid_id, slp.id, slp.expire_timer, slp.expire_offset, slp.pitch, slp.roll, sle.condition, slp.lvl_override, slp.hp_override, slp.mp_override, slp.str_override, slp.sta_override, slp.wis_override, slp.int_override, slp.agi_override, slp.heat_override, slp.cold_override, slp.magic_override, slp.mental_override, slp.divine_override, slp.disease_override, slp.poison_override, difficulty_override FROM spawn_location_placement slp, spawn_location_name sln, spawn_location_entry sle, spawn_npcs sn where sn.spawn_id = sle.spawn_id and sln.id = sle.spawn_location_id and sln.id = slp.spawn_location_id and slp.zone_id=%u and slp.instance_id=%u ORDER BY sln.id, sle.id", SPAWN_ENTRY_TYPE_NPC);
		objects = ProcessSpawnLocations(zone, "SELECT sln.id, sle.id, slp.x, slp.y, slp.z, slp.x_offset, slp.y_offset, slp.z_offset, slp.heading, sle.spawn_id, sle.spawnpercentage, slp.respawn, slp.respawn_offset_low, slp.respawn_offset_high, slp.duplicated_spawn, slp.grid_id, slp.id, slp.expire_timer, slp.expire_offset, slp.pitch, slp.roll, sle.condition, slp.lvl_override, slp.hp_override, slp.mp_override, slp.str_override, slp.sta_override, slp.wis_override, slp.int_override, slp.agi_override, slp.heat_override, slp.cold_override, slp.magic_override, slp.mental_override, slp.divine_override, slp.disease_override, slp.poison_override, difficulty_override FROM spawn_location_placement slp, spawn_location_name sln, spawn_location_entry sle, spawn_objects so where so.spawn_id = sle.spawn_id and sln.id = sle.spawn_location_id and sln.id = slp.spawn_location_id and slp.zone_id=%u and slp.instance_id=%u ORDER BY sln.id, sle.id", SPAWN_ENTRY_TYPE_OBJECT);
		widgets = ProcessSpawnLocations(zone, "SELECT sln.id, sle.id, slp.x, slp.y, slp.z, slp.x_offset, slp.y_offset, slp.z_offset, slp.heading, sle.spawn_id, sle.spawnpercentage, slp.respawn, slp.respawn_offset_low, slp.respawn_offset_high, slp.duplicated_spawn, slp.grid_id, slp.id, slp.expire_timer, slp.expire_offset, slp.pitch, slp.roll, sle.condition, slp.lvl_override, slp.hp_override, slp.mp_override, slp.str_override, slp.sta_override, slp.wis_override, slp.int_override, slp.agi_override, slp.heat_override, slp.cold_override, slp.magic_override, slp.mental_override, slp.divine_override, slp.disease_override, slp.poison_override, difficulty_override FROM spawn_location_placement slp, spawn_location_name sln, spawn_location_entry sle, spawn_widgets sw where sw.spawn_id = sle.spawn_id and sln.id = sle.spawn_location_id and sln.id = slp.spawn_location_id and slp.zone_id=%u and slp.instance_id=%u ORDER BY sln.id, sle.id", SPAWN_ENTRY_TYPE_WIDGET);
		signs = ProcessSpawnLocations(zone, "SELECT sln.id, sle.id, slp.x, slp.y, slp.z, slp.x_offset, slp.y_offset, slp.z_offset, slp.heading, sle.spawn_id, sle.spawnpercentage, slp.respawn, slp.respawn_offset_low, slp.respawn_offset_high, slp.duplicated_spawn, slp.grid_id, slp.id, slp.expire_timer, slp.expire_offset, slp.pitch, slp.roll, sle.condition, slp.lvl_override, slp.hp_override, slp.mp_override, slp.str_override, slp.sta_override, slp.wis_override, slp.int_override, slp.agi_override, slp.heat_override, slp.cold_override, slp.magic_override, slp.mental_override, slp.divine_override, slp.disease_override, slp.poison_override, difficulty_override FROM spawn_location_placement slp, spawn_location_name sln, spawn_location_entry sle, spawn_signs ss where ss.spawn_id = sle.spawn_id and sln.id = sle.spawn_location_id and sln.id = slp.spawn_location_id and slp.zone_id=%u and slp.instance_id=%u ORDER BY sln.id, sle.id", SPAWN_ENTRY_TYPE_SIGN);
		ground_spawns = ProcessSpawnLocations(zone, "SELECT sln.id, sle.id, slp.x, slp.y, slp.z, slp.x_offset, slp.y_offset, slp.z_offset, slp.heading, sle.spawn_id, sle.spawnpercentage, slp.respawn, slp.respawn_offset_low, slp.respawn_offset_high, slp.duplicated_spawn, slp.grid_id, slp.id, slp.expire_timer, slp.expire_offset, slp.pitch, slp.roll, sle.condition, slp.lvl_override, slp.hp_override, slp.mp_override, slp.str_override, slp.sta_override, slp.wis_override, slp.int_override, slp.agi_override, slp.heat_override, slp.cold_override, slp.magic_override, slp.mental_override, slp.divine_override, slp.disease_override, slp.poison_override, difficulty_override FROM spawn_location_placement slp, spawn_location_name sln, spawn_location_entry sle, spawn_ground sg where sg.spawn_id = sle.spawn_id and sln.id = sle.spawn_location_id and sln.id = slp.spawn_location_id and slp.zone_id=%u and slp.instance_id=%u ORDER BY sln.id, sle.id", SPAWN_ENTRY_TYPE_GROUNDSPAWN);
		spawn_groups = LoadSpawnLocationGroups(zone);
		spawn_group_associations = LoadSpawnLocationGroupAssociations(zone);
		spawn_group_chances = LoadSpawnGroupChances(zone);
	}
	else {
		npcs = ProcessSpawnLocations(zone, "SELECT sln.id, sle.id, slp.x, slp.y, slp.z, slp.x_offset, slp.y_offset, slp.z_offset, slp.heading, sle.spawn_id, sle.spawnpercentage, slp.respawn, slp.respawn_offset_low, slp.respawn_offset_high, slp.duplicated_spawn, slp.grid_id, slp.id, slp.expire_timer, slp.expire_offset, slp.pitch, slp.roll, sle.condition, slp.lvl_override, slp.hp_override, slp.mp_override, slp.str_override, slp.sta_override, slp.wis_override, slp.int_override, slp.agi_override, slp.heat_override, slp.cold_override, slp.magic_override, slp.mental_override, slp.divine_override, slp.disease_override, slp.poison_override, difficulty_override FROM spawn_location_placement slp, spawn_location_name sln, spawn_location_entry sle, spawn_npcs sn where sn.spawn_id = sle.spawn_id and sln.id = sle.spawn_location_id and sln.id = slp.spawn_location_id and slp.zone_id=%i and (slp.instance_id = 0 or slp.instance_id=%u) ORDER BY sln.id, sle.id", SPAWN_ENTRY_TYPE_NPC);
		objects = ProcessSpawnLocations(zone, "SELECT sln.id, sle.id, slp.x, slp.y, slp.z, slp.x_offset, slp.y_offset, slp.z_offset, slp.heading, sle.spawn_id, sle.spawnpercentage, slp.respawn, slp.respawn_offset_low, slp.respawn_offset_high, slp.duplicated_spawn, slp.grid_id, slp.id, slp.expire_timer, slp.expire_offset, slp.pitch, slp.roll, sle.condition, slp.lvl_override, slp.hp_override, slp.mp_override, slp.str_override, slp.sta_override, slp.wis_override, slp.int_override, slp.agi_override, slp.heat_override, slp.cold_override, slp.magic_override, slp.mental_override, slp.divine_override, slp.disease_override, slp.poison_override, difficulty_override FROM spawn_location_placement slp, spawn_location_name sln, spawn_location_entry sle, spawn_objects so where so.spawn_id = sle.spawn_id and sln.id = sle.spawn_location_id and sln.id = slp.spawn_location_id and slp.zone_id=%i and (slp.instance_id = 0 or slp.instance_id=%u) ORDER BY sln.id, sle.id", SPAWN_ENTRY_TYPE_OBJECT);
		widgets = ProcessSpawnLocations(zone, "SELECT sln.id, sle.id, slp.x, slp.y, slp.z, slp.x_offset, slp.y_offset, slp.z_offset, slp.heading, sle.spawn_id, sle.spawnpercentage, slp.respawn, slp.respawn_offset_low, slp.respawn_offset_high, slp.duplicated_spawn, slp.grid_id, slp.id, slp.expire_timer, slp.expire_offset, slp.pitch, slp.roll, sle.condition, slp.lvl_override, slp.hp_override, slp.mp_override, slp.str_override, slp.sta_override, slp.wis_override, slp.int_override, slp.agi_override, slp.heat_override, slp.cold_override, slp.magic_override, slp.mental_override, slp.divine_override, slp.disease_override, slp.poison_override, difficulty_override FROM spawn_location_placement slp, spawn_location_name sln, spawn_location_entry sle, spawn_widgets sw where sw.spawn_id = sle.spawn_id and sln.id = sle.spawn_location_id and sln.id = slp.spawn_location_id and slp.zone_id=%i and (slp.instance_id = 0 or slp.instance_id=%u) ORDER BY sln.id, sle.id", SPAWN_ENTRY_TYPE_WIDGET);
		signs = ProcessSpawnLocations(zone, "SELECT sln.id, sle.id, slp.x, slp.y, slp.z, slp.x_offset, slp.y_offset, slp.z_offset, slp.heading, sle.spawn_id, sle.spawnpercentage, slp.respawn, slp.respawn_offset_low, slp.respawn_offset_high, slp.duplicated_spawn, slp.grid_id, slp.id, slp.expire_timer, slp.expire_offset, slp.pitch, slp.roll, sle.condition, slp.lvl_override, slp.hp_override, slp.mp_override, slp.str_override, slp.sta_override, slp.wis_override, slp.int_override, slp.agi_override, slp.heat_override, slp.cold_override, slp.magic_override, slp.mental_override, slp.divine_override, slp.disease_override, slp.poison_override, difficulty_override FROM spawn_location_placement slp, spawn_location_name sln, spawn_location_entry sle, spawn_signs ss where ss.spawn_id = sle.spawn_id and sln.id = sle.spawn_location_id and sln.id = slp.spawn_location_id and slp.zone_id=%i and (slp.instance_id = 0 or slp.instance_id=%u) ORDER BY sln.id, sle.id", SPAWN_ENTRY_TYPE_SIGN);
		ground_spawns = ProcessSpawnLocations(zone, "SELECT sln.id, sle.id, slp.x, slp.y, slp.z, slp.x_offset, slp.y_offset, slp.z_offset, slp.heading, sle.spawn_id, sle.spawnpercentage, slp.respawn, slp.respawn_offset_low, slp.respawn_offset_high, slp.duplicated_spawn, slp.grid_id, slp.id, slp.expire_timer, slp.expire_offset, slp.pitch, slp.roll, sle.condition, slp.lvl_override, slp.hp_override, slp.mp_override, slp.str_override, slp.sta_override, slp.wis_override, slp.int_override, slp.agi_override, slp.heat_override, slp.cold_override, slp.magic_override, slp.mental_override, slp.divine_override, slp.disease_override, slp.poison_override, difficulty_override FROM spawn_location_placement slp, spawn_location_name sln, spawn_location_entry sle, spawn_ground sg where sg.spawn_id = sle.spawn_id and sln.id = sle.spawn_location_id and sln.id = slp.spawn_location_id and slp.zone_id=%i and (slp.instance_id = 0 or slp.instance_id=%u) ORDER BY sln.id, sle.id", SPAWN_ENTRY_TYPE_GROUNDSPAWN);
		spawn_groups = LoadSpawnLocationGroups(zone);
		spawn_group_associations = LoadSpawnLocationGroupAssociations(zone);
		spawn_group_chances = LoadSpawnGroupChances(zone);

	}
	LogWrite(SPAWN__INFO, 0, "Spawn", "Loaded for zone '%s' (%u):\n\t%u NPC(s), %u Object(s), %u Widget(s)\n\t%u Sign(s), %u Ground Spawn(s), %u Spawn Group(s)\n\t%u Spawn Group Association(s), %u Spawn Group Chance(s)", zone->GetZoneName(), zone->GetZoneID(), npcs, objects, widgets, signs, ground_spawns, spawn_groups, spawn_group_associations, spawn_group_chances);
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Exit LoadSpawns");

}

bool WorldDatabase::UpdateSpawnLocationSpawns(Spawn* spawn) {
	Query query;
	query.RunQuery2(Q_UPDATE, "update spawn_location_placement set x=%f, y=%f, z=%f, heading=%f, x_offset=%f, y_offset=%f, z_offset=%f, respawn=%u, expire_timer=%u, expire_offset=%u, grid_id=%u, pitch=%f, roll=%f where id = %u",
		spawn->GetX(), spawn->GetY(), spawn->GetZ(), spawn->GetHeading(), spawn->GetXOffset(), spawn->GetYOffset(), spawn->GetZOffset(), spawn->GetRespawnTime(), spawn->GetExpireTime(), spawn->GetExpireOffsetTime(), spawn->GetLocation(), spawn->GetPitch(), spawn->GetRoll(), spawn->GetSpawnLocationPlacementID());
	if (query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF) {
		LogWrite(WORLD__ERROR, 0, "World", "Error in UpdateSpawnLocationSpawns query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	return true;
}

bool WorldDatabase::UpdateSpawnWidget(int32 widget_id, char* queryString) {
	Query query;
	query.RunQuery2(Q_UPDATE, "update spawn_widgets set %s where widget_id = %u",
		queryString, widget_id);
	if (query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF) {
		LogWrite(WORLD__ERROR, 0, "World", "Error in UpdateSpawnWidget query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	return true;
}

vector<string>* WorldDatabase::GetSpawnNameList(const char* in_name){
	Query query;
	string names = "";
	vector<string>* ret = 0;
	string name = getSafeEscapeString(in_name);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT concat(spawn.id, ', ', name) FROM spawn where name like '%%%s%%'", name.c_str());
	if(result && mysql_num_rows(result) > 0){
		ret = new vector<string>;
		MYSQL_ROW row;
		int8 num = 0;
		while(result && (row = mysql_fetch_row(result))){
			if(num >= 10)
				break;
			ret->push_back(string(row[0]));
			num++;
		}
		char total[60] = {0};
		if(mysql_num_rows(result) > 10)
			sprintf(total, "Total number of results: %u (Limited to 10)", (int32)mysql_num_rows(result));
		else
			sprintf(total, "Total number of results: %u", (int32)mysql_num_rows(result));
		ret->push_back(string(total));
	}
	return ret;
}
string WorldDatabase::GetZoneName(char* zone_description){
	string ret;
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT name FROM zones where description = '%s'", getSafeEscapeString(zone_description).c_str());
	if(result && mysql_num_rows(result) > 0){
		MYSQL_ROW row = mysql_fetch_row(result);
		ret = string(row[0]);
	}
	return ret;
}

void WorldDatabase::LoadRevivePoints(vector<RevivePoint*>* revive_points, int32 zone_id){
	if(revive_points && revive_points->size() > 0){
		LogWrite(WORLD__ERROR, 0, "World", "Revive points have already been loaded for this zone!");
		return;
	}
	else if(!revive_points || zone_id == 0){
		LogWrite(WORLD__ERROR, 0, "World", "LoadRevivePoints called with null variables!");
		return;
	}
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT respawn_zone_id, location_name, safe_x, safe_y, safe_z, heading, always_included FROM revive_points where zone_id=%u ORDER BY id asc", zone_id);
	if(revive_points && result && mysql_num_rows(result) > 0){
		MYSQL_ROW row;
		int32 id = 0;
		RevivePoint* point = 0;
		while(result && (row=mysql_fetch_row(result))){
			point = new RevivePoint;
			point->id = id;
			point->zone_id = atoul(row[0]);
			point->location_name = string(row[1]);
			point->x = atof(row[2]);
			point->y = atof(row[3]);
			point->z = atof(row[4]);
			point->heading = atof(row[5]);
			point->always_included = atoul(row[6]);
			revive_points->push_back(point);
			id++;
		}
	}
}

int32 WorldDatabase::GetNextSpawnIDInZone(int32 zone_id)
{
	Query query;
	int32 ret = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT MAX(id) FROM spawn where id LIKE '%i____'", zone_id);
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		if(row[0])
			ret = atoi(row[0]) + 1;
	}
	if( ret == 0 )
		ret = zone_id * 10000; // there are no spawns for that zone yet, to start with the first ID

	LogWrite(WORLD__DEBUG, 0, "World", "Next Spawn ID for Zone %i: %u", zone_id, ret);
	return ret;

}


bool WorldDatabase::SaveSpawnInfo(Spawn* spawn){
	Query query;
	string name = getSafeEscapeString(spawn->GetName());
	string suffix = getSafeEscapeString(spawn->GetSuffixTitle());
	string prefix = getSafeEscapeString(spawn->GetPrefixTitle());
	string last_name = getSafeEscapeString(spawn->GetLastName());
	if(spawn->GetDatabaseID() == 0){

		int8 isInstanceType = (spawn->GetZone()->GetInstanceType() == Instance_Type::PERSONAL_HOUSE_INSTANCE);

		int32 new_spawn_id = GetNextSpawnIDInZone(spawn->GetZone()->GetZoneID());

		query.RunQuery2(Q_INSERT, "insert into spawn (id, name, race, model_type, size, targetable, show_name, command_primary, command_secondary, visual_state, attackable, show_level, show_command_icon, display_hand_icon, faction_id, collision_radius, hp, power, prefix, suffix, last_name, is_instanced_spawn, merchant_min_level, merchant_max_level) values(%u, '%s', %i, %i, %i, %i, %i, %u, %u, %i, %i, %i, %i, %i, %u, %i, %u, %u, '%s', '%s', '%s', %u, %u, %u)",
			new_spawn_id, name.c_str(), spawn->GetRace(), spawn->GetModelType(), spawn->GetSize(), spawn->appearance.targetable, spawn->appearance.display_name, spawn->GetPrimaryCommandListID(), spawn->GetSecondaryCommandListID(), spawn->GetVisualState(), spawn->appearance.attackable, spawn->appearance.show_level, spawn->appearance.show_command_icon, spawn->appearance.display_hand_icon, 0, spawn->appearance.pos.collision_radius, spawn->GetTotalHP(), spawn->GetTotalPower(), prefix.c_str(), suffix.c_str(), last_name.c_str(), isInstanceType, spawn->GetMerchantMinLevel(), spawn->GetMerchantMaxLevel());

		if( new_spawn_id > 0 )
			spawn->SetDatabaseID(new_spawn_id); // use the new zone_id range
		else if( query.GetLastInsertedID() > 0 )
			spawn->SetDatabaseID(query.GetLastInsertedID()); // else fall back to last_inserted_id
		else
			return false; // else, hang your head in shame as you are an utter failure

		if(spawn->IsNPC()){
			query.RunQuery2(Q_INSERT, "insert into spawn_npcs (spawn_id, min_level, max_level, enc_level, class_, gender, min_group_size, max_group_size, hair_type_id, facial_hair_type_id, wing_type_id, chest_type_id, legs_type_id, soga_hair_type_id, soga_facial_hair_type_id, soga_model_type, heroic_flag, action_state, mood_state, initial_state, activity_status, hide_hood, emote_state) values(%u, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i)",
				spawn->GetDatabaseID(), spawn->GetLevel(), spawn->GetLevel(), spawn->GetDifficulty(), spawn->GetAdventureClass(), spawn->GetGender(), 0, 0, ((NPC*)spawn)->features.hair_type, ((NPC*)spawn)->features.hair_face_type, 
				((NPC*)spawn)->features.wing_type, ((NPC*)spawn)->features.chest_type, ((NPC*)spawn)->features.legs_type, ((NPC*)spawn)->features.soga_hair_type, ((NPC*)spawn)->features.soga_hair_face_type, spawn->appearance.soga_model_type, spawn->appearance.heroic_flag, spawn->GetActionState(), spawn->GetMoodState(), spawn->GetInitialState(), spawn->GetActivityStatus(), spawn->appearance.hide_hood, spawn->appearance.emote_state);
		}
		else if(spawn->IsObject()){
			query.RunQuery2(Q_INSERT, "insert into spawn_objects (spawn_id) values(%u)", spawn->GetDatabaseID());
		}
		else if(spawn->IsWidget()){
			Widget* widget = (Widget*)spawn;
			query.RunQuery2(Q_INSERT, "insert into spawn_widgets (spawn_id, widget_id) values(%u, %u)", spawn->GetDatabaseID(), widget->GetWidgetID());
		}
		else if(spawn->IsSign()){
			query.RunQuery2(Q_INSERT, "insert into spawn_signs (spawn_id, description) values(%u, 'change me')", spawn->GetDatabaseID());

		}
		else if (spawn->IsGroundSpawn()) {
			query.RunQuery2(Q_INSERT, "insert into spawn_ground (spawn_id) values(%u)", spawn->GetDatabaseID());
		}
	}
	else{
		if(spawn->IsNPC()){
			query.RunQuery2(Q_UPDATE, "update spawn_npcs, spawn set name='%s', min_level=%i, max_level=%i, enc_level=%i, race=%i, model_type=%i, class_=%i, gender=%i, show_name=%i, attackable=%i, show_level=%i, targetable=%i, show_command_icon=%i, display_hand_icon=%i, hair_type_id=%i, facial_hair_type_id=%i, wing_type_id=%i, chest_type_id=%i, legs_type_id=%i, soga_hair_type_id=%i, soga_facial_hair_type_id=%i, soga_model_type=%i, size=%i, hp=%u, heroic_flag=%i, power=%u, collision_radius=%i, command_primary=%u, command_secondary=%u, visual_state=%i, action_state=%i, mood_state=%i, initial_state=%i, activity_status=%i, alignment=%i, faction_id=%u, hide_hood=%i, emote_state=%i, suffix ='%s', prefix='%s', last_name='%s', merchant_min_level = %u, merchant_max_level = %u where spawn_npcs.spawn_id = spawn.id and spawn.id = %u",
				name.c_str(), spawn->GetLevel(), spawn->GetLevel(), spawn->GetDifficulty(), spawn->GetRace(), spawn->GetModelType(),
				spawn->GetAdventureClass(), spawn->GetGender(), spawn->appearance.display_name, spawn->appearance.attackable, spawn->appearance.show_level, spawn->appearance.targetable, spawn->appearance.show_command_icon, spawn->appearance.display_hand_icon, ((NPC*)spawn)->features.hair_type, 
				((NPC*)spawn)->features.hair_face_type, ((NPC*)spawn)->features.wing_type, ((NPC*)spawn)->features.chest_type, ((NPC*)spawn)->features.legs_type, ((NPC*)spawn)->features.soga_hair_type, ((NPC*)spawn)->features.soga_hair_face_type, spawn->appearance.soga_model_type, spawn->GetSize(), 
				spawn->GetTotalHPBase(), spawn->appearance.heroic_flag, spawn->GetTotalPowerBase(), spawn->GetCollisionRadius(), spawn->GetPrimaryCommandListID(), 
				spawn->GetSecondaryCommandListID(), spawn->GetVisualState(), spawn->GetActionState(), spawn->GetMoodState(), spawn->GetInitialState(), 
				spawn->GetActivityStatus(), ((NPC*)spawn)->GetAlignment(), spawn->GetFactionID(), spawn->appearance.hide_hood, spawn->appearance.emote_state, 
				suffix.c_str(), prefix.c_str(), last_name.c_str(), spawn->GetMerchantMinLevel(), spawn->GetMerchantMaxLevel(), spawn->GetDatabaseID());
		}
		else if(spawn->IsObject()){
			query.RunQuery2(Q_UPDATE, "update spawn_objects, spawn set name='%s', model_type=%i, show_name=%i, targetable=%i, size=%i, command_primary=%u, command_secondary=%u, visual_state=%i, attackable=%i, show_level=%i, show_command_icon=%i, display_hand_icon=%i, collision_radius=%i, hp = %u, power = %u, device_id = %i, merchant_min_level = %u, merchant_max_level = %u where spawn_objects.spawn_id = spawn.id and spawn.id = %u",
				name.c_str(), spawn->GetModelType(), spawn->appearance.display_name, spawn->appearance.targetable, spawn->GetSize(), spawn->GetPrimaryCommandListID(), spawn->GetSecondaryCommandListID(), spawn->GetVisualState(), spawn->appearance.attackable, spawn->appearance.show_level, spawn->appearance.show_command_icon, spawn->appearance.display_hand_icon,
				spawn->GetCollisionRadius(), spawn->GetTotalHP(), spawn->GetTotalPower(), ((Object*)spawn)->GetDeviceID(), spawn->GetMerchantMinLevel(), spawn->GetMerchantMaxLevel(), spawn->GetDatabaseID());
		}
		else if(spawn->IsWidget()){
			Widget* widget = (Widget*)spawn;
			char* openSound = 0;
			char* closeSound = 0;
			if (widget->GetOpenSound() != NULL) openSound = (char*)widget->GetOpenSound(); else openSound = (char*)string("0").c_str();
			if (widget->GetCloseSound() != NULL) closeSound = (char*)widget->GetCloseSound(); else closeSound = (char*)string("0").c_str();
			query.RunQuery2(Q_UPDATE, "update spawn_widgets, spawn set name='%s', race=%i, model_type=%i, show_name=%i, attackable=%i, show_level=%i, show_command_icon=%i, display_hand_icon=%i, size=%i, hp=%u, power=%u, collision_radius=%i, command_primary=%u, command_secondary=%u, visual_state=%i, faction_id=%u, suffix ='%s', prefix='%s', last_name='%s',widget_id = %u,widget_x = %f,widget_y = %f,widget_z = %f,include_heading = %u,include_location = %u,icon = %u,type='%s',open_heading = %f,closed_heading = %f,open_x = %f,open_y = %f,open_z = %f,action_spawn_id = %u,open_sound_file='%s',close_sound_file='%s',open_duration = %u,close_x = %f,close_y=%f,close_z=%f,linked_spawn_id = %u,house_id = %u, merchant_min_level = %u, merchant_max_level = %u where spawn_widgets.spawn_id = spawn.id and spawn.id = %u",
				name.c_str(), spawn->GetRace(), spawn->GetModelType(), spawn->appearance.display_name, spawn->appearance.attackable, spawn->appearance.show_level, spawn->appearance.show_command_icon, spawn->appearance.display_hand_icon, spawn->GetSize(),
				spawn->GetTotalHP(), spawn->GetTotalPower(), spawn->GetCollisionRadius(), spawn->GetPrimaryCommandListID(), spawn->GetSecondaryCommandListID(), spawn->GetVisualState(), spawn->GetFactionID(), 
				suffix.c_str(), prefix.c_str(), last_name.c_str(), widget->GetWidgetID(), widget->GetX(), widget->GetY(), widget->GetZ(), widget->GetIncludeHeading(), widget->GetIncludeLocation(), widget->GetIconValue(), Widget::GetWidgetTypeNameByTypeID(widget->GetWidgetType()).c_str(),
				widget->GetOpenHeading(), widget->GetClosedHeading(), widget->GetOpenX(), widget->GetOpenY(), widget->GetOpenZ(), 
				widget->GetActionSpawnID(), openSound, closeSound,widget->GetOpenDuration(),
				widget->GetCloseX(),widget->GetCloseY(),widget->GetCloseZ(),widget->GetLinkedSpawnID(),widget->GetHouseID(),
				spawn->GetMerchantMinLevel(), spawn->GetMerchantMaxLevel(), spawn->GetDatabaseID());
		}
		else if(spawn->IsSign()){
			Sign* sign = (Sign*)spawn;
			query.RunQuery2(Q_UPDATE, "update spawn_signs, spawn set name='%s', race=%i, model_type=%i, show_name=%i, attackable=%i, show_level=%i, show_command_icon=%i, display_hand_icon=%i, size=%i, hp=%u, power=%u, collision_radius=%i, command_primary=%u, command_secondary=%u, visual_state=%i, faction_id=%u, suffix ='%s', prefix='%s', last_name='%s', `type`='%s', zone_id = %u, widget_id = %u, title='%s', widget_x = %f, widget_y = %f, widget_z = %f, icon = %u, description='%s', sign_distance = %f, zone_x = %f, zone_y = %f, zone_z = %f, zone_heading = %f, include_heading = %u, include_location = %u, merchant_min_level = %u, merchant_max_level = %u, language = %u where spawn_signs.spawn_id = spawn.id and spawn.id = %u",
				name.c_str(), spawn->GetRace(), spawn->GetModelType(), spawn->appearance.display_name, spawn->appearance.attackable, spawn->appearance.show_level, spawn->appearance.show_command_icon, spawn->appearance.display_hand_icon, spawn->GetSize(),
				spawn->GetTotalHP(), spawn->GetTotalPower(), spawn->GetCollisionRadius(), spawn->GetPrimaryCommandListID(), spawn->GetSecondaryCommandListID(), spawn->GetVisualState(), spawn->GetFactionID(),
				suffix.c_str(), prefix.c_str(), last_name.c_str(), sign->GetSignType() == SIGN_TYPE_GENERIC ? "Generic" : "Zone", sign->GetSignZoneID(), 
				sign->GetWidgetID(), sign->GetSignTitle(), sign->GetWidgetX(), sign->GetWidgetY(), sign->GetWidgetZ(), 
				sign->GetIconValue(), sign->GetSignDescription(), sign->GetSignDistance(), sign->GetSignZoneX(), 
				sign->GetSignZoneY(), sign->GetSignZoneZ(), sign->GetSignZoneHeading(), sign->GetIncludeHeading(), 
				sign->GetIncludeLocation(), spawn->GetMerchantMinLevel(), spawn->GetMerchantMaxLevel(), sign->GetLanguage(), spawn->GetDatabaseID());
		}
	}
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(SPAWN__ERROR, 0, "Spawn", "Error in SaveSpawnInfo query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	return true;
}

int32 WorldDatabase::SaveCombinedSpawnLocation(ZoneServer* zone, Spawn* in_spawn, const char* name){
	vector<Spawn*>* spawns = in_spawn->GetSpawnGroup();
	uint32 spawnLocationID = 0;
	if(spawns && spawns->size() > 0){
		vector<Spawn*>::iterator itr;
		map<Spawn*, int32>::iterator freq_itr;
		Spawn* spawn = 0;
		map<Spawn*, int32> database_spawns;
		int32 total = 0;
		float x_offset = GetSpawnLocationPlacementOffsetX(in_spawn->GetSpawnLocationID());
		float y_offset = GetSpawnLocationPlacementOffsetY(in_spawn->GetSpawnLocationID());
		float z_offset = GetSpawnLocationPlacementOffsetZ(in_spawn->GetSpawnLocationID());
		int32 spawn_location_id = GetNextSpawnLocation();
		spawnLocationID = spawn_location_id;
		if(!name)
			name = "Combine SpawnGroup Generated";
		if(!CreateNewSpawnLocation(spawn_location_id, name)){
			safe_delete(spawns);
			return 0;
		}
		for(itr = spawns->begin();itr!=spawns->end();itr++){
			spawn = *itr;
			if (spawn) {
				RemoveSpawnFromSpawnLocation(spawn);
				spawn->SetSpawnLocationID(spawn_location_id);
				bool add = true;
				for (freq_itr = database_spawns.begin(); freq_itr != database_spawns.end(); freq_itr++) {
					if (spawn->GetDatabaseID() == freq_itr->first->GetDatabaseID()) {
						freq_itr->second++;
						total++;
						add = false;
					}
				}
				if (add) {
					database_spawns[spawn] = 1;
					total++;
				}
			}
		}
		for(freq_itr = database_spawns.begin(); freq_itr != database_spawns.end(); freq_itr++){
			int8 percent = (freq_itr->second*100)/total;
			if(!SaveSpawnEntry(freq_itr->first, name, percent, x_offset, y_offset, z_offset, freq_itr->first == in_spawn, false)){
				safe_delete(spawns);
				return 0;
			}
		}
		for(itr=spawns->begin();itr!=spawns->end();itr++){
			spawn = *itr;
			zone->RemoveSpawn(spawn, true, true, true, true, true);
		}
		safe_delete(spawns);
	}
	else{
		safe_delete(spawns);
		return 0;
	}
	return spawnLocationID;
}

bool WorldDatabase::SaveSpawnEntry(Spawn* spawn, const char* spawn_location_name, int8 percent, float x_offset, float y_offset, float z_offset, bool save_zonespawn, bool create_spawnlocation){
	Query query;
	Query query2;
	int32 count = 0;
	if(create_spawnlocation){
		count = GetSpawnLocationCount(spawn->GetSpawnLocationID());
		if(count == 0){
			if(!CreateNewSpawnLocation(spawn->GetSpawnLocationID(), spawn_location_name))
				return false;
		}
	}
	query.RunQuery2(Q_INSERT, "insert into spawn_location_entry (spawn_id, spawn_location_id, spawnpercentage) values(%u, %u, %i)", spawn->GetDatabaseID(), spawn->GetSpawnLocationID(), percent);

	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(SPAWN__ERROR, 0, "Spawn", "Error in SaveSpawnEntry query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	if(save_zonespawn){
		query2.RunQuery2(Q_INSERT, "insert into spawn_location_placement (zone_id, instance_id, spawn_location_id, x, y, z, x_offset, y_offset, z_offset, heading, grid_id) values(%u, %u, %u, %f, %f, %f, %f, %f, %f, %f, %u)", spawn->GetZone()->GetZoneID(), spawn->GetZone()->GetInstanceID(), spawn->GetSpawnLocationID(), spawn->GetX(), spawn->GetY(), spawn->GetZ(),x_offset, y_offset, z_offset, spawn->GetHeading(), spawn->GetLocation());
		if(query2.GetErrorNumber() && query2.GetError() && query2.GetErrorNumber() < 0xFFFFFFFF){
			LogWrite(SPAWN__ERROR, 0, "Spawn", "Error in SaveSpawnEntry query '%s': %s", query2.GetQuery(), query2.GetError());
			return false;
		}
		spawn->SetSpawnLocationPlacementID(query2.GetLastInsertedID());
	}
	return true;
}

float WorldDatabase::GetSpawnLocationPlacementOffsetX(int32 location_id) {
	Query query;
	MYSQL_ROW row;
	float ret = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `x_offset` FROM `spawn_location_placement` WHERE `spawn_location_id`=%u", location_id);
	if (result && (row = mysql_fetch_row(result))) {
		if (row[0])
			ret = atof(row[0]);
	}
	return ret;
}

float WorldDatabase::GetSpawnLocationPlacementOffsetY(int32 location_id) {
	Query query;
	MYSQL_ROW row;
	float ret = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `y_offset` FROM `spawn_location_placement` WHERE `spawn_location_id`=%u", location_id);
	if (result && (row = mysql_fetch_row(result))) {
		if (row[0])
			ret = atof(row[0]);
	}
	return ret;
}

float WorldDatabase::GetSpawnLocationPlacementOffsetZ(int32 location_id) {
	Query query;
	MYSQL_ROW row;
	float ret = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `z_offset` FROM `spawn_location_placement` WHERE `spawn_location_id`=%u", location_id);
	if (result && (row = mysql_fetch_row(result))) {
		if (row[0])
			ret = atof(row[0]);
	}
	return ret;
}

bool WorldDatabase::CreateNewSpawnLocation(int32 id, const char* name){
	Query query;
	if(!name)
		name = "Unknown Spawn Location Name";
	string str_name = getSafeEscapeString(name);
	query.RunQuery2(Q_INSERT, "insert into spawn_location_name (id, name) values(%u, '%s')", id, str_name.c_str());
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(SPAWN__ERROR, 0, "Spawn", "Error in CreateNewSpawnLocation query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	return true;
}

int32 WorldDatabase::GetSpawnLocationCount(int32 location, Spawn* spawn){
	Query query;
	int32 ret = 0;
	MYSQL_RES* result = 0;
	if(spawn)
		result = query.RunQuery2(Q_SELECT, "SELECT count(id) FROM spawn_location_entry where spawn_location_id=%u and spawn_id=%u", location, spawn->GetDatabaseID());
	else
		result = query.RunQuery2(Q_SELECT, "SELECT count(id) FROM spawn_location_entry where spawn_location_id=%u", location);
	if(result && mysql_num_rows(result) > 0){
		MYSQL_ROW row;
		while(result && (row = mysql_fetch_row(result)) && row[0]){
			ret = strtoul(row[0], NULL, 0);
		}
	}
	return ret;
}

int32 WorldDatabase::GetNextSpawnLocation(){
	Query query;
	int32 ret = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT max(id) FROM spawn_location_name");
	if(result && mysql_num_rows(result) > 0){
		MYSQL_ROW row;
		while(result && (row = mysql_fetch_row(result)) && row[0]){
			ret = strtoul(row[0], NULL, 0);
		}
	}
	ret++;
	return ret;
}

bool WorldDatabase::RemoveSpawnFromSpawnLocation(Spawn* spawn){
	Query query;
	Query query2;
	int32 count = GetSpawnLocationCount(spawn->GetSpawnLocationID(), spawn);

	query.RunQuery2(Q_DELETE, "delete FROM spawn_location_placement where spawn_location_id=%u", spawn->GetSpawnLocationID());
	if(count == 1)
		query.RunQuery2(Q_DELETE, "delete FROM spawn_location_name where id=%u", spawn->GetSpawnLocationID());

	query2.RunQuery2(Q_DELETE, "delete FROM spawn_location_entry where spawn_id=%u and spawn_location_id = %u", spawn->GetDatabaseID(), spawn->GetSpawnLocationID());
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(WORLD__ERROR, 0, "World", "Error in RemoveSpawnFromSpawnLocation query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	else if(query2.GetErrorNumber() && query2.GetError() && query2.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(SPAWN__ERROR, 0, "Spawn", "Error in RemoveSpawnFromSpawnLocation query '%s': %s", query2.GetQuery(), query.GetError());
		return false;
	}
	return true;
}

map<int32, string>* WorldDatabase::GetZoneList(const char* name, bool is_admin)
{
	Query query;
	map<int32, string>* ret = 0;
	string zone_name = getSafeEscapeString(name);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `id`, `name` FROM zones WHERE `name` RLIKE '%s' %s", zone_name.c_str(), (is_admin)?"":" LIMIT 0,10");
	if(result && mysql_num_rows(result) > 0)
	{
		ret = new map<int32, string>;
		MYSQL_ROW row;
		while(result && (row = mysql_fetch_row(result)))
		{
			zone_name = row[1];
			(*ret)[atoi(row[0])] = zone_name;
		}
	}

	return ret;
}

void WorldDatabase::UpdateStartingFactions(int32 char_id, int8 choice){
	Query query;
	query.RunQuery2(Q_INSERT, "insert into character_factions (char_id, faction_id, faction_level) select %u, faction_id, value FROM starting_factions where starting_city=%i", char_id, choice);
}

string WorldDatabase::GetStartingZoneName(int8 choice){
	Query query;
	string zone_name = "";
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT name FROM zones where start_zone = %u", choice);
	if(result && mysql_num_rows(result) > 0){
		MYSQL_ROW row;
		while(result && (row = mysql_fetch_row(result))){
			zone_name = string(row[0]);
		}	
	}
	return zone_name;
}

void WorldDatabase::UpdateStartingZone(int32 char_id, int8 class_id, int8 race_id, PacketStruct* create)
{
	Query query,query2;

	int32 packetVersion = create->GetVersion();
	int8 choice = create->getType_int8_ByName("starting_zone"); // 0 = far journey, 1 = isle of refuge
	int8 deity = create->getType_int8_ByName("deity"); // aka 'alignment' for early DOF, 0 = evil, 1 = good
	int32 startingZoneRuleFlag = rule_manager.GetGlobalRule(R_World, StartingZoneRuleFlag)->GetInt32();
	bool enforceRacialAlignment = rule_manager.GetGlobalRule(R_World, EnforceRacialAlignment)->GetBool();
	
	if((startingZoneRuleFlag == 1 || startingZoneRuleFlag == 2) && packetVersion > 561)
	{
		LogWrite(PLAYER__INFO, 0, "Player", "Starting zone rule flag %u override choice %u to deity value of 0", startingZoneRuleFlag, choice);
		choice = 0;
	}
	
	LogWrite(PLAYER__INFO, 0, "Player", "Adding default zone for race: %i, class: %i for char_id: %u (choice: %i), deity(alignment): %u, version: %u.", race_id, class_id, char_id, choice, deity, packetVersion);

	// first, check to see if there is a starting_zones record for this race/class/choice combo (now using extended Archetype/BaseClass/Class combos
	MYSQL_RES* result = 0;
	
	string whereRuleFlag("");
	if(startingZoneRuleFlag > 0)
		whereRuleFlag = string(" AND ruleflag & " + std::to_string(startingZoneRuleFlag));
	
	string syntaxSelect("SELECT z.name, sz.zone_id, z.safe_x, z.safe_y, z.safe_z, sz.x, sz.y, sz.z, sz.heading, sz.is_instance, z.city_zone, sz.start_alignment FROM");
	if ( class_id == 0 )
		result = query.RunQuery2(Q_SELECT, "%s starting_zones sz, zones z WHERE sz.zone_id = z.id AND class_id = 255 AND race_id IN (%i, 255) AND deity IN (%i, 255) AND choice = %u AND (min_version = 0 or min_version <= %u) AND (max_version = 0 or max_version >= %u)%s",
			syntaxSelect.c_str(), race_id, deity, choice, packetVersion, packetVersion, whereRuleFlag.c_str());
		else
			result = query.RunQuery2(Q_SELECT, "%s starting_zones sz, zones z WHERE sz.zone_id = z.id AND class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255) AND deity IN (%i, 255) AND choice IN (%i, 255) AND (min_version = 0 or min_version <= %u) AND (max_version = 0 or max_version >= %u)%s",
		syntaxSelect.c_str(), classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id, deity, choice, packetVersion, packetVersion, whereRuleFlag.c_str());

	if(result && mysql_num_rows(result) > 0)
	{
		string zone_name = "ERROR";
		MYSQL_ROW row;

		bool zoneSet = false;

		float safeX = 0.0f, safeY = 0.0f, safeZ = 0.0f, x = 0.0f, y = 0.0f, z = 0.0f, heading = 0.0f;
		int8 is_instance = 0;
		int32 zone_id = 0;
		int32 instance_id = 0;
		int8 starting_city = 0;
		sint8 start_alignment = 0;

		if( result && (row = mysql_fetch_row(result)) )
		{
			int8 i=0;

			zoneSet = true;

			zone_name = string(row[i++]);

			zone_id = atoul(row[i++]);

			safeX = atof(row[i++]);
			safeY = atof(row[i++]);
			safeZ = atof(row[i++]);

			x = atof(row[i++]);
			y = atof(row[i++]);
			z = atof(row[i++]);

			if ( x == -999999.0f && y == -999999.0f && z == -999999.0f)
			{
				x = safeX;
				y = safeY;
				z = safeZ;
			}

			heading = atof(row[i++]);

			if(heading == -999999.0f )
				heading = 0.0f;
			
			is_instance = atoul(row[i++]);

			starting_city = atoul(row[i++]);
			
			start_alignment = atoi(row[i++]);
		}


		// all EQ2 clients hard-code these restrictions in some form (later clients added Neutral instead of good/evil only)
		// start with good races only
		if(enforceRacialAlignment && (race_id == DWARF || race_id == FROGLOK || race_id == HALFLING ||
		    race_id == HIGH_ELF || race_id == WOOD_ELF || race_id == FAE)) {
				start_alignment = ALIGNMENT_GOOD; // always should good
		}
		// we drop into this case because it is a special check compared to the straigt good/evil checks on top and below
		else if(start_alignment == ALIGNMENT_EVIL && deity == ALIGNMENT_GOOD) {
			if (enforceRacialAlignment && (race_id == AERAKYN || race_id == RATONGA || race_id == BARBARIAN || race_id == ERUDITE ||
				race_id == HUMAN || race_id == VAMPIRE || race_id == HALF_ELF || race_id == GNOME || race_id == KERRA)) {
					if(zone_id == 21 || zone_id == 25 || zone_id == 26 || zone_id == 27) { // far journey zones
						start_alignment = deity; // inheriting the clients alignment of 'good' due to the fact we start in far journey and it is a shared instance right now (farjourneyfreeport)
					}
					// otherwise we use the starting zone alignment since these particular races can be evil which will be set in start_alignment of 0 within starting_zones (neriak, freeport, etc.)
			}
			else if(enforceRacialAlignment) {
				LogWrite(WORLD__WARNING, 0, "World", "Starting alignment seems unexpected, zone id %u, race id %u, deity(alignment) choice: %u, start alignment: %i", zone_id, race_id, deity, start_alignment);
			}
		}
		// anyone else is simply evil with the enforcement check
		else if(enforceRacialAlignment && (race_id != DWARF && race_id != FROGLOK && race_id != HALFLING &&
		    race_id != HIGH_ELF && race_id != WOOD_ELF && race_id != FAE)) {
				start_alignment = ALIGNMENT_EVIL;
		}
			
		if(is_instance) // should only be true if we get a result
		{
			string zone_name = GetZoneName(zone_id);
			if(zone_name.size() > 0) {
				ZoneServer* tmp = new ZoneServer(zone_name.c_str());
				instance_id = CreateNewInstance(zone_id);
				tmp->SetInstanceID(instance_id);
				LoadZoneInfo(tmp);
				AddCharacterInstance(char_id, instance_id, string(tmp->GetZoneName()), tmp->GetInstanceType(), Timer::GetUnixTimeStamp(), 0, tmp->GetDefaultLockoutTime(), tmp->GetDefaultReenterTime());
				safe_delete(tmp);
			}
		}
		
		query2.RunQuery2(Q_UPDATE, "UPDATE characters SET current_zone_id = %u, x = %f, y = %f, z = %f, heading = %f, starting_city = %i, instance_id = %u, alignment = %u WHERE id = %u", 
			zone_id, x, y, z, heading, starting_city, instance_id, start_alignment, char_id);

		if(query2.GetErrorNumber() && query2.GetError() && query2.GetErrorNumber() < 0xFFFFFFFF){
			LogWrite(PLAYER__ERROR, 0, "Player", "Error in UpdateStartingZone custom starting_zones, query: '%s': %s", query2.GetQuery(), query2.GetError());
			return;
		}

		if(query2.GetAffectedRows() > 0)
		{
			LogWrite(PLAYER__INFO, 0, "Player", "Setting New Character Starting Zone to '%s' with location %f, %f, %f and heading %f FROM starting_zones table.", zone_name.c_str(), x, y, z, heading);
			return;
		}
	}
	else
	{
		// there was no matching starting_zone value, so use default 'choice' starting city
		query2.RunQuery2(Q_UPDATE, "UPDATE characters c, zones z SET c.current_zone_id = z.id, c.x = z.safe_x, c.y = z.safe_y, c.z = z.safe_z, c.starting_city = %i WHERE z.start_zone = %i and c.id = %u", 
			choice, choice, char_id);

		if(query2.GetErrorNumber() && query2.GetError() && query2.GetErrorNumber() < 0xFFFFFFFF)
		{
			LogWrite(PLAYER__ERROR, 0, "Player", "Error in UpdateStartingZone player choice, query: '%s': %s", query2.GetQuery(), query2.GetError());
			return;
		}

		if(query2.GetAffectedRows() > 0)
		{
			LogWrite(PLAYER__DEBUG, 0, "Player", "Setting New Character Starting Zone to '%s' FROM player choice.", GetStartingZoneName(choice).c_str());
			return;
		}
	}

	// if we are here, it's a bad thing. zone tables have no start_city values to match client 'choice', so throw the player into zone according to R_World::DefaultStartingZoneID rule.
	// shout a few warnings so the admin fixes this asap!
	int16 default_zone_id = rule_manager.GetGlobalRule(R_World, DefaultStartingZoneID)->GetInt16();

	LogWrite(WORLD__WARNING, 0, "World", "No Starting City defined for player choice: %i! BAD! BAD! BAD! Defaulting player to zone %i.", choice, default_zone_id);

	query.RunQuery2(Q_UPDATE, "UPDATE characters c, zones z SET c.current_zone_id = z.id, c.x = z.safe_x, c.y = z.safe_y, c.z = z.safe_z, c.heading = z.safe_heading, c.starting_city = 1 WHERE z.id = %i and c.id = %u", default_zone_id, char_id);

 	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF)
	{
		LogWrite(PLAYER__ERROR, 0, "Player", "Error in UpdateStartingZone default zone %i, query: '%s': %s", default_zone_id, query.GetQuery(), query.GetError());
		return;
 	}

	if (query.GetAffectedRows() > 0) {
		string zone_name = GetZoneName(1);
		if(zone_name.length() > 0)
			LogWrite(PLAYER__DEBUG, 0, "Player", "Setting New Character Starting Zone to '%s' due to no valid options!", zone_name.c_str());
		else
			LogWrite(PLAYER__DEBUG, 0, "Player", "Unable to set New Character Starting Zone due to no valid options!");
	}

	return;
}

void WorldDatabase::UpdateStartingItems(int32 char_id, int8 class_id, int8 race_id, bool base_class){
	LogWrite(PLAYER__DEBUG, 0, "Player", "Adding default items for race: %i, class: %i for char_id: %u", race_id, class_id, char_id);
	Query query;
	Query query2;
	vector<Item*> items;
	vector<Item*> bags;
	map<int32, int8> total_slots;
	map<int32, int8> slots_left;
	map<int8, bool> equip_slots;
	map<Item*, StartingItem> item_list;
	int32 item_id = 0;
	Item* item = 0;
	StartingItem* starting_item = 0;
	//first get a list of the starting items for the character
	MYSQL_RES* result = 0;
	/*if(!base_class)
		result = query2.RunQuery2(Q_SELECT, "SELECT `type`, item_id, creator, condition_, attuned, count FROM starting_items where (class_id=%i and race_id=%i) or (class_id=%i and race_id=255) or (class_id=255 and race_id=%i) or (class_id=255 and race_id=255) ORDER BY id", class_id, race_id, class_id, race_id);
	else
		result = query2.RunQuery2(Q_SELECT, "SELECT `type`, item_id, creator, condition_, attuned, count FROM starting_items where (class_id=%i and race_id=%i) or (class_id=%i and race_id=255) ORDER BY id", class_id, race_id, class_id);*/
	result = query2.RunQuery2(Q_SELECT, "SELECT `type`, item_id, creator, condition_, attuned, count FROM starting_items WHERE class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255) ORDER BY id", classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id);
	if(result && mysql_num_rows(result) > 0){
		MYSQL_ROW row;
		while(result && (row = mysql_fetch_row(result))){
			item_id = atoul(row[1]);
			item = master_item_list.GetItem(item_id);
			if(item){
				starting_item = &(item_list[item]);
				starting_item->type = (row[0]) ? string(row[0]) : "";
				starting_item->item_id = atoul(row[1]);
				starting_item->creator = (row[2]) ? string(row[2]) : "";
				starting_item->condition = atoi(row[3]);
				starting_item->attuned = atoi(row[4]);
				starting_item->count = atoi(row[5]);
				item = master_item_list.GetItem(starting_item->item_id);
				if(item){
					if(bags.size() < NUM_INV_SLOTS && item->IsBag() && item->details.num_slots > 0)
						bags.push_back(item);
					else
						items.push_back(item);
				}
			}
		}
	}
	slots_left[0] = NUM_INV_SLOTS;
	//next create the bags in the inventory
	for(int8 i=0;i<bags.size();i++){
		item = bags[i];
		query.RunQuery2(Q_INSERT, "insert into character_items (char_id, type, slot, item_id, creator, condition_, attuned, bag_id, count) values(%u, '%s', %i, %u, '%s', %i, %i, %u, %i)",
			char_id, item_list[item].type.c_str(), i, item_list[item].item_id, getSafeEscapeString(item_list[item].creator.c_str()).c_str(), item_list[item].condition, item_list[item].attuned, 0, item_list[item].count);
		slots_left[query.GetLastInsertedID()] = item->details.num_slots;
		total_slots[query.GetLastInsertedID()] = item->details.num_slots;
		slots_left[0]--;
	}
	map<int32, int8>::iterator itr;
	int32 inv_slot = 0;
	int8  slot = 0;
	//finally process the rest of the items, placing them in the first available slot
	for(int32 x=0;x<items.size();x++){
		item = items[x];
		if(item_list[item].type.find("NOT") < 0xFFFFFFFF){ // NOT-EQUIPPED Items
			for(itr = slots_left.begin(); itr != slots_left.end(); itr++){
				if(itr->second > 0){
					if(itr->first == 0 && slots_left.size() > 1) //we want items to go into bags first, then inventory after bags are full
						continue;
					inv_slot = itr->first;
					slot = total_slots[itr->first] - itr->second;
					itr->second--;
					if(itr->second == 0)
						slots_left.erase(itr);
					break;
				}
			}
			query.RunQuery2(Q_INSERT, "insert into character_items (char_id, type, slot, item_id, creator, condition_, attuned, bag_id, count) values(%u, '%s', %i, %u, '%s', %i, %i, %u, %i)",
				char_id, item_list[item].type.c_str(), slot, item_list[item].item_id, getSafeEscapeString(item_list[item].creator.c_str()).c_str(), item_list[item].condition, item_list[item].attuned, inv_slot, item_list[item].count);		
		}
		else{ //EQUIPPED Items
			for(int8 i=0;i<item->slot_data.size();i++){
				if(equip_slots.count(item->slot_data[i]) == 0){
					equip_slots[item->slot_data[i]] = true;
					query.RunQuery2(Q_INSERT, "insert into character_items (char_id, type, slot, item_id, creator, condition_, attuned, bag_id, count) values(%u, '%s', %i, %u, '%s', %i, %i, %u, %i)",
						char_id, item_list[item].type.c_str(), item->slot_data[i], item_list[item].item_id, getSafeEscapeString(item_list[item].creator.c_str()).c_str(), item_list[item].condition, item_list[item].attuned, 0, item_list[item].count);
					break;
				}
			}
		}
	}
}

void WorldDatabase::UpdateStartingSkills(int32 char_id, int8 class_id, int8 race_id)
{
	Query query;
	LogWrite(PLAYER__DEBUG, 0, "Player", "Adding default skills for race: %i, class: %i for char_id: %u", race_id, class_id, char_id);
	query.RunQuery2(Q_INSERT, "INSERT IGNORE INTO character_skills (char_id, skill_id, current_val, max_val) SELECT %u, skill_id, current_val, max_val FROM starting_skills WHERE class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255)", 
		char_id, classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id);
}

void WorldDatabase::UpdateStartingSpells(int32 char_id, int8 class_id, int8 race_id){
	Query query;
	LogWrite(PLAYER__DEBUG, 0, "Player", "Adding default spells for race: %i, class: %i for char_id: %u", race_id, class_id, char_id);
	query.RunQuery2(Q_INSERT, "INSERT IGNORE INTO character_spells (char_id, spell_id, tier, knowledge_slot) SELECT %u, spell_id, tier, knowledge_slot FROM starting_spells WHERE class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255)", 
		char_id, classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id);
}

void WorldDatabase::UpdateStartingSkillbar(int32 char_id, int8 class_id, int8 race_id){
	Query query;
	LogWrite(PLAYER__DEBUG, 0, "Player", "Adding default skillbar for race: %i, class: %i for char_id: %u", race_id, class_id, char_id);
	query.RunQuery2(Q_INSERT, "INSERT IGNORE INTO character_skillbar (char_id, type, hotbar, spell_id, slot, text_val) SELECT %u, type, hotbar, spell_id, slot, text_val FROM starting_skillbar WHERE class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255)", 
		char_id, classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id);
}

void WorldDatabase::UpdateStartingTitles(int32 char_id, int8 class_id, int8 race_id, int8 gender_id) {
	Query query;
	LogWrite(PLAYER__DEBUG, 0, "Player", "Adding default titles for race: %i, class: %i, gender: %i for char_id: %u", race_id, class_id, gender_id, char_id);
	query.RunQuery2(Q_INSERT, "INSERT IGNORE INTO character_titles (char_id, title_id) SELECT %u, title_id FROM  starting_titles WHERE class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255) and gender_id IN (%i, 255)", 
		char_id, classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id, gender_id);
}

string WorldDatabase::GetZoneDescription(int32 id){
	Query query;
	string ret = "";
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT description FROM zones where id = %u", id);
	if(result && mysql_num_rows(result) > 0){
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		ret = string(row[0]);
	}
	return ret;
}

string WorldDatabase::GetZoneName(int32 id){
	if (zone_names.count(id) > 0){
		return zone_names[id];
	}
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `name` FROM zones where `id` = %u", id);
	if(result && mysql_num_rows(result) > 0){
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		zone_names[id] = row[0];
		return zone_names[id];
	}
	return string("");
}

bool WorldDatabase::VerifyZone(const char* name){
	Query query;
	char* escaped = getEscapeString(name);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT name FROM zones where name='%s'",escaped);
	safe_delete_array(escaped);
	if(result && mysql_num_rows(result) > 0)
		return true;
	else
		return false;
}

int8 WorldDatabase::GetInstanceTypeByZoneID(int32 zoneID)
{

	std::map<int32, int8>::iterator itr = zone_instance_types.find(zoneID);
	if(itr != zone_instance_types.end())
		return itr->second;
	
	DatabaseResult result;
	int8 ret = 0;

	LogWrite(INSTANCE__DEBUG, 0, "Instance", "Getting instances for zone_id %u", zoneID);

	if( !database_new.Select(&result, "SELECT instance_type+0 FROM zones WHERE id = %u", zoneID) )
	{
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in GetInstanceTypeByZoneID() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
		return ret;
	}

	if( result.GetNumRows() > 0 )
	{
		result.Next();
		ret = (result.GetInt8Str("instance_type+0") == 0) ? 0 : result.GetInt8Str("instance_type+0") - 1;
		zone_instance_types.insert(make_pair(zoneID, ret));
		LogWrite(INSTANCE__DEBUG, 0, "Instance", "Found instance type %i for zone_id %u", ret, zoneID);
	}
	else
		LogWrite(INSTANCE__DEBUG, 0, "Instance", "No instances found for zone_id %u", zoneID);

	return ret;
}

void WorldDatabase::Save(Client* client){
	Query query;
	Player* player = client->GetPlayer();
	if(!player->CheckPlayerInfo())
		return;

	int32 instance_id = 0;
	if(client->IsZoning() && client->GetZoningID()) {
		instance_id = client->GetZoningInstanceID();
	}
	else if ( client->GetCurrentZone ( ) != NULL )
		instance_id = client->GetCurrentZone()->GetInstanceID();
	
	int32 zone_id = 0;
	
	if(client->IsZoning() && client->GetZoningID()) {
		zone_id = client->GetZoningID();
	}
	else if(client->GetCurrentZone())
		zone_id = client->GetCurrentZone()->GetZoneID();
	
	query.AddQueryAsync(client->GetCharacterID(), this, Q_UPDATE, "update characters set current_zone_id=%u, x=%f, y=%f, z=%f, heading=%f, level=%i,instance_id=%i,last_saved=%i, `class`=%i, `tradeskill_level`=%i, `tradeskill_class`=%i, `group_id`=%u, deity = %u, alignment = %u, zone_duplicating_id = %u where id = %u", zone_id, player->GetX(), player->GetY(), player->GetZ(), player->GetHeading(), player->GetLevel(), instance_id, client->GetLastSavedTimeStamp(), client->GetPlayer()->GetAdventureClass(), client->GetPlayer()->GetTSLevel(), client->GetPlayer()->GetTradeskillClass(), client->GetPlayer()->GetGroupMemberInfo() ? client->GetPlayer()->GetGroupMemberInfo()->group_id : client->GetRejoinGroupID(), client->GetPlayer()->GetDeity(), client->GetPlayer()->GetInfoStruct()->get_alignment(), client->GetDuplicatingZoneID(), client->GetCharacterID());
	query.AddQueryAsync(client->GetCharacterID(), this, Q_UPDATE, "update character_details set hp=%u, power=%u, str=%i, sta=%i, agi=%i, wis=%i, intel=%i, heat=%i, cold=%i, magic=%i, mental=%i, divine=%i, disease=%i, poison=%i, coin_copper=%u, coin_silver=%u, coin_gold=%u, coin_plat=%u, max_hp = %u, max_power=%u, xp = %u, xp_needed = %u, xp_debt = %f, xp_vitality = %f, tradeskill_xp = %u, tradeskill_xp_needed = %u, tradeskill_xp_vitality = %f, bank_copper = %u, bank_silver = %u, bank_gold = %u, bank_plat = %u, status_points = %u, bind_zone_id=%u, bind_x = %f, bind_y = %f, bind_z = %f, bind_heading = %f, house_zone_id=%u, combat_voice = %i, emote_voice = %i, biography='%s', flags=%u, flags2=%u, last_name='%s', assigned_aa = %i, unassigned_aa = %i, tradeskill_aa = %i, unassigned_tradeskill_aa = %i, prestige_aa = %i, unassigned_prestige_aa = %i, tradeskill_prestige_aa = %i, unassigned_tradeskill_prestige_aa = %i, pet_name = '%s' where char_id = %u",
		player->GetHP(), player->GetPower(), player->GetStrBase(), player->GetStaBase(), player->GetAgiBase(), player->GetWisBase(), player->GetIntBase(), player->GetHeatResistanceBase(), player->GetColdResistanceBase(), player->GetMagicResistanceBase(),
		player->GetMentalResistanceBase(), player->GetDivineResistanceBase(), player->GetDiseaseResistanceBase(), player->GetPoisonResistanceBase(), player->GetCoinsCopper(), player->GetCoinsSilver(), player->GetCoinsGold(), player->GetCoinsPlat(), player->GetTotalHPBase(), player->GetTotalPowerBase(), player->GetXP(), player->GetNeededXP(), player->GetXPDebt(), player->GetXPVitality(), player->GetTSXP(), player->GetNeededTSXP(), player->GetTSXPVitality(), player->GetBankCoinsCopper(),
		player->GetBankCoinsSilver(), player->GetBankCoinsGold(), player->GetBankCoinsPlat(), player->GetStatusPoints(), client->GetPlayer()->GetPlayerInfo()->GetBindZoneID(), client->GetPlayer()->GetPlayerInfo()->GetBindZoneX(), client->GetPlayer()->GetPlayerInfo()->GetBindZoneY(), client->GetPlayer()->GetPlayerInfo()->GetBindZoneZ(), client->GetPlayer()->GetPlayerInfo()->GetBindZoneHeading(), client->GetPlayer()->GetPlayerInfo()->GetHouseZoneID(), 
		client->GetPlayer()->GetCombatVoice(), client->GetPlayer()->GetEmoteVoice(), getSafeEscapeString(client->GetPlayer()->GetBiography().c_str()).c_str(), player->GetFlags(), player->GetFlags2(), client->GetPlayer()->GetLastName(), 
		client->GetPlayer()->GetAssignedAA(), client->GetPlayer()->GetUnassignedAA(), client->GetPlayer()->GetTradeskillAA(), client->GetPlayer()->GetUnassignedTradeskillAA(), client->GetPlayer()->GetPrestigeAA(), 
		client->GetPlayer()->GetUnassignedPretigeAA(), client->GetPlayer()->GetTradeskillPrestigeAA(), client->GetPlayer()->GetUnassignedTradeskillPrestigeAA(), client->GetPlayer()->GetInfoStruct()->get_pet_name().c_str(), client->GetCharacterID());
	map<string, int8>::iterator itr;
	map<string, int8>* friends = player->GetFriends();
	if(friends && friends->size() > 0){
		for(itr = friends->begin(); itr != friends->end(); itr++){
			if(itr->second == 1){
				query.AddQueryAsync(client->GetCharacterID(), this, Q_INSERT, "insert ignore into character_social (char_id, name, type) values(%u, '%s', 'FRIEND')", client->GetCharacterID(), getSafeEscapeString(itr->first.c_str()).c_str());
				itr->second = 0;
			}
			else if(itr->second == 2){
				query.AddQueryAsync(client->GetCharacterID(), this, Q_DELETE, "delete FROM character_social where char_id = %u and name = '%s'", client->GetCharacterID(), getSafeEscapeString(itr->first.c_str()).c_str());
				itr->second = 3;
			}
		}
	}
	map<string, int8>* ignored = player->GetIgnoredPlayers();
	if(ignored && ignored->size() > 0){
		for(itr = ignored->begin(); itr != ignored->end(); itr++){
			if(itr->second == 1){
				query.AddQueryAsync(client->GetCharacterID(), this, Q_INSERT, "insert ignore into character_social (char_id, name, type) values(%u, '%s', 'IGNORE')", client->GetCharacterID(), getSafeEscapeString(itr->first.c_str()).c_str());
				itr->second = 0;
			}
			else if(itr->second == 2){
				query.AddQueryAsync(client->GetCharacterID(), this, Q_DELETE, "delete FROM character_social where char_id = %u and name = '%s'", client->GetCharacterID(), getSafeEscapeString(itr->first.c_str()).c_str());
				itr->second = 3;
			}
		}
	}
	SavePlayerFactions(client);
	SaveCharacterQuests(client);
	SaveCharacterSkills(client);
	SavePlayerSpells(client);
	SavePlayerMail(client);
	SavePlayerCollections(client);
	client->SaveQuestRewardData();

	LogWrite(PLAYER__INFO, 3, "Player", "Player '%s' (%u) data saved.", player->GetName(), player->GetCharacterID());
}

void WorldDatabase::LoadEntityCommands(ZoneServer* zone) {
	int32 total = 0;
	int32 id = 0;
	EntityCommand* command = 0;

	DatabaseResult result;
	if (!database_new.Select(&result, "SELECT `command_list_id`, `command_text`, `distance`, `command`, `error_text`, `cast_time`, `spell_visual` FROM `entity_commands` ORDER BY `id`")) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}

	while (result.Next()) {
		command = new EntityCommand;

		id = result.GetInt32(0);
		command->name = result.GetString(1);
		command->distance = result.GetFloat(2);
		command->command = result.GetString(3);
		command->error_text = result.GetString(4);
		command->cast_time = result.GetInt16(5);
		command->spell_visual = result.GetInt32(6);
		command->default_allow_list = true;

		zone->SetEntityCommandList(id, command);
		LogWrite(COMMAND__DEBUG, 5, "Command", "---Loading Command: '%s' (%s)", command->name.c_str(), command->command.c_str());

		total++;
	}
	LogWrite(COMMAND__DEBUG, 0, "Command", "--Loaded %i entity command(s)", total);
}

void WorldDatabase::LoadFactionAlliances() 
{
	LogWrite(FACTION__DEBUG, 1, "Faction", "-Loading faction alliances...");
	int32 total = 0;
	int32 fTotal = 0;
	int32 hTotal = 0;
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT faction_id, friend_faction, hostile_faction FROM faction_alliances");

	if(result && mysql_num_rows(result) > 0) 
	{
		MYSQL_ROW row;
		int32 faction_id = 0;
		int32 friendly_id = 0;
		int32 hostile_id = 0;

		while(result && (row = mysql_fetch_row(result)))
		{
			faction_id = atoul(row[0]);
			friendly_id = atoul(row[1]);
			hostile_id = atoul(row[2]);

			if(friendly_id > 0) 
			{
				master_faction_list.AddFriendlyFaction(faction_id, friendly_id);
				fTotal++;
				LogWrite(FACTION__DEBUG, 5, "Faction", "---Faction %i is friendly towards %i", faction_id, friendly_id);
			}
			if(hostile_id > 0) 
			{
				master_faction_list.AddHostileFaction(faction_id, hostile_id);
				hTotal++;
				LogWrite(FACTION__DEBUG, 5, "Faction", "---Faction %i is hostile towards %i", faction_id, hostile_id);
			}
			total++;
		}
	}
	LogWrite(FACTION__DEBUG, 3, "Faction", "--Loaded %u Alliances: %i friendly, %i hostile", total, fTotal, hTotal);
}

bool WorldDatabase::UpdateSpawnScriptData(int32 spawn_id, int32 spawn_location_id, int32 spawnentry_id, const char* name){
	bool ret = false;
	if((spawn_id > 0 || spawn_location_id > 0 || spawnentry_id > 0) && name){
		Query query, query2;
		int32 row_id = 0;
		if(spawn_id > 0){
			query.RunQuery2(Q_DELETE, "DELETE FROM spawn_scripts where spawn_id=%u", spawn_id);
			query2.RunQuery2(Q_INSERT, "INSERT into spawn_scripts (spawn_id, lua_script) values(%u, '%s')", spawn_id, getSafeEscapeString(name).c_str());
			if(query2.GetErrorNumber() && query2.GetError() && query2.GetErrorNumber() < 0xFFFFFFFF)
				LogWrite(LUA__ERROR, 0, "LUA", "Error in UpdateSpawnScriptData, Query: %s, Error: %s", query2.GetQuery(), query2.GetError());
			else{
				row_id = query2.GetLastInsertedID();
				if(row_id > 0)
					world.AddSpawnScript(row_id, name);
				ret = true;
			}
		}
		else if(spawn_location_id > 0){
			query.RunQuery2(Q_DELETE, "DELETE FROM spawn_scripts where spawn_location_id=%u", spawn_location_id);
			query2.RunQuery2(Q_INSERT, "INSERT into spawn_scripts (spawn_location_id, lua_script) values(%u, '%s')", spawn_location_id, getSafeEscapeString(name).c_str());
			if(query2.GetErrorNumber() && query2.GetError() && query2.GetErrorNumber() < 0xFFFFFFFF)
				LogWrite(LUA__ERROR, 0, "LUA", "Error in UpdateSpawnScriptData, Query: %s, Error: %s", query2.GetQuery(), query2.GetError());
			else{
				row_id = query2.GetLastInsertedID();
				if(row_id > 0)
					world.AddSpawnLocationScript(row_id, name);
				ret = true;
			}
		}
		else if(spawnentry_id > 0){
			query.RunQuery2(Q_DELETE, "DELETE FROM spawn_scripts where spawnentry_id=%u", spawnentry_id);
			query2.RunQuery2(Q_INSERT, "INSERT into spawn_scripts (spawnentry_id, lua_script) values(%u, '%s')", spawnentry_id, getSafeEscapeString(name).c_str());
			if(query2.GetErrorNumber() && query2.GetError() && query2.GetErrorNumber() < 0xFFFFFFFF)
				LogWrite(LUA__ERROR, 0, "LUA", "Error in UpdateSpawnScriptData, Query: %s, Error: %s", query2.GetQuery(), query2.GetError());
			else{
				row_id = query2.GetLastInsertedID();
				if(row_id > 0)
					world.AddSpawnEntryScript(row_id, name);
				ret = true;
			}
		}
	}
	return ret;
}

void WorldDatabase::LoadSpawnScriptData() {
	int32 total = 0;
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT spawn_id, spawnentry_id, spawn_location_id, lua_script FROM spawn_scripts");
	if(result && mysql_num_rows(result) > 0) 
	{
		MYSQL_ROW row;
		int32 spawn_id = 0;
		int32 spawnentry_id = 0;
		int32 spawn_location_id = 0;

		while(result && (row = mysql_fetch_row(result))) 
		{
			spawn_id = atoul(row[0]);
			spawnentry_id = atoul(row[1]);
			spawn_location_id = atoul(row[2]);
			string spawn_script = string(row[3]);

			if(spawnentry_id > 0) 
			{
				world.AddSpawnEntryScript(spawnentry_id, row[3]);
				total++;
			}
			else if(spawn_location_id > 0) 
			{
				world.AddSpawnLocationScript(spawn_location_id, row[3]);
				total++;
			}
			else if(spawn_id > 0) 
			{
				world.AddSpawnScript(spawn_id, row[3]);
				total++;
			}
			else {
				if(row[3])
					LogWrite(LUA__ERROR, 0, "LUA", "Invalid Entry in spawn_scripts table for lua_script '%s' (spawn_id, spawnentry_id and spawn_location_id are all 0)", row[3]);
				else
					LogWrite(LUA__ERROR, 0, "LUA", "Invalid Entry in spawn_scripts table.");
			}

			spawn_id = 0;
			spawnentry_id = 0;
			spawn_location_id = 0;
		}
	}
	LogWrite(LUA__DEBUG, 0, "LUA", "\tLoaded %u SpawnScript%s", total, total == 1 ? "" : "s");
}

void WorldDatabase::LoadZoneScriptData() {
	Query query;
	int32 total = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `id`, `lua_script` FROM `zones`");
	if (result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		while(result && (row = mysql_fetch_row(result))) {
			if (row[1]) {
				int32 zone_id = atoul(row[0]);
				string zone_script = string(row[1]);
				if (zone_id > 0 && zone_script.length() > 0) {

					LogWrite(LUA__DEBUG, 5, "LUA", "ZoneScript: %s loaded.", zone_script.c_str());

					world.AddZoneScript(zone_id, zone_script.c_str());
					total++;
				}
				else if (zone_id > 0) {
					
					string tmpScript;
					tmpScript.append("ZoneScripts/");
					string zonename = GetZoneName(zone_id);
					tmpScript.append(zonename);
					tmpScript.append(".lua");
					struct stat buffer;
					bool fileExists = (stat(tmpScript.c_str(), &buffer) == 0);
					if (fileExists)
					{
						LogWrite(LUA__INFO, 0, "LUA", "No zonescript file described in the database, overriding with ZoneScript %s for Zone ID %u", (char*)tmpScript.c_str(), zone_id);
						world.AddZoneScript(zone_id, tmpScript.c_str());
					}
				}
			}
		}
	}
	LogWrite(LUA__DEBUG, 0, "LUA", "\tLoaded %u ZoneScript%s", total, total == 1 ? "" : "s");
}

int32 WorldDatabase::LoadSpellScriptData() {
	Query query;
	MYSQL_ROW row;
	int32 count;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `lua_script` FROM `spells`"); //  WHERE is_active = 1

	while (result && (row = mysql_fetch_row(result))) {
		if (row[0] && strlen(row[0]) > 0) {
			LuaSpell* testSpell = nullptr;
			if ((testSpell = lua_interface->GetSpell(row[0], false)) != nullptr) {
				LogWrite(SPELL__DEBUG, 5, "Spells", "SpellScript: %s loaded.", row[0]);
				// make the lua_state available to the queue, this isn't a real Spell just a test load
				lua_interface->RemoveCurrentSpell(testSpell->state, testSpell);
				safe_delete(testSpell);
			}
		}
	}

	count = mysql_num_rows(result);
	LogWrite(SPELL__DEBUG, 0, "Spells", "\tLoaded %i SpellScript%s", count, count == 1 ? "" : "s");

	return count;
}

void WorldDatabase::LoadFactionList() {
	int32 total = 0;
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, name, type, description, default_level, negative_change, positive_change FROM factions");
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		Faction* faction = 0;
		while(result && (row = mysql_fetch_row(result))){
			faction = new Faction;
			faction->id = atoul(row[0]);
			faction->name = string(row[1]);
			faction->type = string(row[2]);
			faction->description = string(row[3]);
			faction->default_value = atoi(row[4]);
			faction->negative_change = atoi(row[5]);
			faction->positive_change = atoi(row[6]);
			master_faction_list.AddFaction(faction);
			total++;
			LogWrite(FACTION__DEBUG, 5, "Faction", "---Loading Faction '%s' (%u)", faction->name.c_str(), faction->id);
		}
	}
	LogWrite(FACTION__DEBUG, 3, "Faction", "--Loaded %u Faction%s", total, total == 1 ? "" : "s");
	LoadFactionAlliances();
}

void WorldDatabase::SavePlayerFactions(Client* client){
	LogWrite(PLAYER__DEBUG, 3, "Player", "Saving Player Factions...");
	
	Query query;
	map<int32, sint32>* factions = client->GetPlayer()->GetFactions()->GetFactionValues();
	map<int32, sint32>::iterator itr;
	for(itr = factions->begin(); itr != factions->end(); itr++)
		query.AddQueryAsync(client->GetCharacterID(), this,Q_INSERT, "insert into character_factions (char_id, faction_id, faction_level) values(%u, %u, %i) ON DUPLICATE KEY UPDATE faction_level=%i", client->GetCharacterID(), itr->first, itr->second, itr->second);
}

bool WorldDatabase::LoadPlayerFactions(Client* client) {
	LogWrite(PLAYER__DEBUG, 0, "Player", "Loading Player Factions...");
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT faction_id, faction_level FROM character_factions where char_id=%i", client->GetCharacterID());
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		while(result && (row = mysql_fetch_row(result))){
			client->GetPlayer()->GetFactions()->SetFactionValue(atoul(row[0]), atol(row[1]));
		}
	}
	if(query.GetErrorNumber())
		return false;

	return true;
}

void WorldDatabase::SavePlayerMail(Mail* mail) {
	Query query_update;
	Query query_insert;
	if (mail) {
		if(mail->mail_id > 0)
			query_update.RunQuery2(Q_UPDATE, "UPDATE `character_mail` SET `already_read`=%u, `coin_copper`=%u, `coin_silver`=%u, `coin_gold`=%u, `coin_plat`=%u, `char_item_id`=%u, `stack`=%u WHERE `id`=%u", mail->already_read, mail->coin_copper, mail->coin_silver, mail->coin_gold, mail->coin_plat, mail->char_item_id, mail->stack, mail->mail_id);
		else if (mail->mail_id == 0 || query_update.GetAffectedRows() == 0)
		{
			query_insert.RunQuery2(Q_INSERT, "INSERT INTO `character_mail` (`player_to_id`, `player_from`, `subject`, `mail_body`, `already_read`, `mail_type`, `coin_copper`, `coin_silver`, `coin_gold`, `coin_plat`, `stack`, `postage_cost`, `attachment_cost`, `char_item_id`, `time_sent`, `expire_time`) VALUES (%u, '%s', '%s', '%s', %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u)", mail->player_to_id, mail->player_from.c_str(), getSafeEscapeString(mail->subject.c_str()).c_str(), getSafeEscapeString(mail->mail_body.c_str()).c_str(), mail->already_read, mail->mail_type, mail->coin_copper, mail->coin_silver, mail->coin_gold, mail->coin_plat, mail->stack, mail->postage_cost, mail->attachment_cost, mail->char_item_id, mail->time_sent, mail->expire_time);
			
			if(!mail->mail_id)
				mail->mail_id = query_insert.GetLastInsertedID();
		}
		mail->save_needed = false;
	}
}

void WorldDatabase::SavePlayerMail(Client* client) {
	if (client) {
		MutexMap<int32, Mail*>* mail_list = client->GetPlayer()->GetMail();
		MutexMap<int32, Mail*>::iterator itr = mail_list->begin();
		while (itr.Next()) {
			Mail* mail = itr->second;
			if (mail->save_needed)
				SavePlayerMail(mail);
		}
	}
}

void WorldDatabase::LoadPlayerMail(Client* client, bool new_only) {
	LogWrite(PLAYER__DEBUG, 0, "Player", "Loading Player Mail...");
	if (client) {
		Query query;
		MYSQL_RES* result;
		if (new_only)
			result = query.RunQuery2(Q_SELECT, "SELECT `id`, `player_to_id`, `player_from`, `subject`, `mail_body`, `already_read`, `mail_type`, `coin_copper`, `coin_silver`, `coin_gold`, `coin_plat`, `stack`, `postage_cost`, `attachment_cost`, `char_item_id`, `time_sent`, `expire_time` FROM `character_mail` WHERE `player_to_id`=%u AND `unread`=1", client->GetCharacterID());
		else
			result = query.RunQuery2(Q_SELECT, "SELECT `id`, `player_to_id`, `player_from`, `subject`, `mail_body`, `already_read`, `mail_type`, `coin_copper`, `coin_silver`, `coin_gold`, `coin_plat`, `stack`, `postage_cost`, `attachment_cost`, `char_item_id`, `time_sent`, `expire_time` FROM `character_mail` WHERE `player_to_id`=%u", client->GetCharacterID());
		if (result && mysql_num_rows(result) > 0) {
			MYSQL_ROW row;
			bool hasMail = false;
			while (result && (row = mysql_fetch_row(result))) {

				int32 time_sent = atoul(row[15]);
				if ( time_sent > Timer::GetUnixTimeStamp() )
					continue; // should have not been received yet

				hasMail = true;
				Mail* mail = new Mail;
				mail->mail_id = atoul(row[0]);
				mail->player_to_id = atoul(row[1]);
				mail->player_from = string(row[2]);
				mail->subject = string(row[3]);
				if (row[4])
					mail->mail_body = string(row[4]);
				mail->already_read = atoi(row[5]);
				mail->mail_type = atoi(row[6]);
				mail->coin_copper = atoul(row[7]);
				mail->coin_silver = atoul(row[8]);
				mail->coin_gold = atoul(row[9]);
				mail->coin_plat = atoul(row[10]);
				mail->stack = atoi(row[11]);
				mail->postage_cost = atoul(row[12]);
				mail->attachment_cost = atoul(row[13]);
				mail->char_item_id = atoul(row[14]);
				mail->time_sent = time_sent;
				mail->expire_time = atoul(row[16]);
				mail->save_needed = false;
				client->GetPlayer()->AddMail(mail);

				LogWrite(PLAYER__DEBUG, 5, "Player", "Loaded Mail ID %i, to: %i, from: %s", atoul(row[0]), atoul(row[1]), string(row[2]).c_str());

			}
			
			if(hasMail)
				client->SimpleMessage(CHANNEL_NARRATIVE, "You've got mail! :)");
		}
	}
}

void WorldDatabase::DeletePlayerMail(Mail* mail) {
	Query query;
	if (mail)
		query.RunQuery2(Q_DELETE, "DELETE FROM `character_mail` WHERE `id`=%u", mail->mail_id);
	LogWrite(PLAYER__DEBUG, 0, "Player", "Delete Player Mail...");
}

vector<int32>* WorldDatabase::GetAllPlayerIDs() {
	Query query;
	vector<int32>* ids = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `id` FROM `characters`");
	MYSQL_ROW row;
	while (result && (row = mysql_fetch_row(result))) {
		if (ids == 0)
			ids = new vector<int32>;
		ids->push_back(atoul(row[0]));
	}
	return ids;
}

void WorldDatabase::GetPetNames(ZoneServer* zone) 
{
	DatabaseResult result;
	int32 total = 0;

	if( database_new.Select(&result, "SELECT pet_name FROM spawn_pet_names") )
	{
		while(result.Next())
		{
			zone->pet_names.push_back(result.GetStringStr("pet_name"));
			total++;
			LogWrite(PET__DEBUG, 5, "Pet", "---Loading Pet Name: '%s'", result.GetStringStr("pet_name"));
		}
		LogWrite(PET__DEBUG, 0, "Pet", "--Loaded %u Pet Names", total);
	}
}


int32 WorldDatabase::GetMaxHotBarID(){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT max(id) FROM character_skillbar");
	int32 ret = 0;
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		if(row && row[0])
			ret = strtoul(row[0], NULL, 0);
	}
	return ret;
}

void WorldDatabase::SaveQuickBar(int32 char_id, vector<QuickBarItem*>* quickbar_items){
	vector<QuickBarItem*>::iterator itr;
	QuickBarItem* qbi = 0;
	for(itr = quickbar_items->begin(); itr != quickbar_items->end(); itr++){
		qbi = *itr;
		if(!qbi)
			continue;
		if(qbi->deleted == false){
			Query query;
			if(qbi->text.size > 0){
				query.AddQueryAsync(char_id, this, Q_REPLACE, "replace into character_skillbar (id, hotbar, slot, char_id, spell_id, type, text_val, tier) values(%u, %u, %u, %u, %u, %i, '%s', %i)", 
					qbi->unique_id, qbi->hotbar, qbi->slot, char_id, qbi->id, qbi->type, getSafeEscapeString(qbi->text.data.c_str()).c_str(), qbi->tier);
			}
			else{
				query.AddQueryAsync(char_id, this, Q_REPLACE, "replace into character_skillbar (id, hotbar, slot, char_id, spell_id, type, text_val, tier) values(%u, %u, %u, %u, %u, %i, 'Unused', %i)",
					qbi->unique_id, qbi->hotbar, qbi->slot, char_id, qbi->id, qbi->type, qbi->tier);
			}
		}
		else{
			Query query;
			query.AddQueryAsync(char_id, this, Q_DELETE, "delete FROM character_skillbar where hotbar=%u and slot=%u and char_id=%u", qbi->hotbar, qbi->slot, char_id);
		}
	}
}

map<int32, vector<LevelArray*> >* WorldDatabase::LoadSpellClasses(){
	map<int32, vector<LevelArray*> >* ret = new map<int32, vector<LevelArray*> >();
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT spell_id, adventure_class_id, tradeskill_class_id, level, classic_level FROM spell_classes");
	MYSQL_ROW row;
	LevelArray* level = 0;
	while(result && (row = mysql_fetch_row(result))){
		level = new LevelArray();
		level->adventure_class = atoi(row[1]);
		level->tradeskill_class = atoi(row[2]);
		level->spell_level = atoi(row[3]);
		level->classic_spell_level = atof(row[4]);
		(*ret)[atoul(row[0])].push_back(level);
	}
	return ret;
}

void WorldDatabase::LoadTraits(){
	Query query;
	MYSQL_ROW row;
	TraitData* trait;

	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `spell_id`, `level`, `class_req`, `race_req`, `isTrait`,`isInate`, `isFocusEffect`, `isTraining`,`tier`, `group`, `item_id` FROM spell_traits where is_active = 1");
	while (result && (row = mysql_fetch_row(result))){
		trait = new TraitData;
		int8 i = 0;
		trait->spellID = strtoul(row[0], NULL, 0);
		trait->level = atoi(row[(++i)]);
		trait->classReq = atoi(row[(++i)]);
		trait->raceReq = atoi(row[(++i)]);
		trait->isTrait = (atoi(row[(++i)]) == 0) ? false : true;
		trait->isInate = (atoi(row[(++i)]) == 0) ? false : true;
		trait->isFocusEffect = (atoi(row[(++i)]) == 0) ? false : true;
		trait->isTraining = (atoi(row[(++i)]) == 0) ? false : true;
		trait->tier = atoi(row[(++i)]);
		trait->group = atoi(row[(++i)]);
		trait->item_id = atoul(row[(++i)]);

		master_trait_list.AddTrait(trait);
	}

	LogWrite(SPELL__INFO, 0, "Traits", "Loaded %u Trait(s)", master_trait_list.Size());
}

void WorldDatabase::LoadSpells()
{
	DatabaseResult result;
	Spell *spell;
	SpellData *data;
	int32 t_now = Timer::GetUnixTimeStamp();
	int32 total = 0;
	map<int32, vector<LevelArray*> >* level_data = LoadSpellClasses();

	if( !database_new.Select(&result, "SELECT s.`id`, ts.spell_id, ts.index, `name`, `description`, `type`, `class_skill`, `min_class_skill_req`, `mastery_skill`, `tier`, `is_aa`,`hp_req`, `power_req`,`power_by_level`, `cast_time`, `recast`, `radius`, `max_aoe_targets`, `req_concentration`, `range`, `duration1`, `duration2`, `resistibility`, `hp_upkeep`, `power_upkeep`, `duration_until_cancel`, `target_type`, `recovery`, `power_req_percent`, `hp_req_percent`, `icon`, `icon_heroic_op`, `icon_backdrop`, `success_message`, `fade_message`, `fade_message_others`, `cast_type`, `lua_script`, `call_frequency`, `interruptable`, `spell_visual`, `effect_message`, `min_range`, `can_effect_raid`, `affect_only_group_members`, `hit_bonus`, `display_spell_tier`, `friendly_spell`, `group_spell`, `spell_book_type`, spell_type+0, s.is_active, savagery_req, savagery_req_percent, savagery_upkeep, dissonance_req, dissonance_req_percent, dissonance_upkeep, linked_timer_id, det_type, incurable, control_effect_type, cast_while_moving, casting_flags, persist_through_death, not_maintained, savage_bar, savage_bar_slot, soe_spell_crc, 0xffffffff-CRC32(s.`name`) as 'spell_name_crc', type_group_spell_id, can_fizzle, given_by "
									"FROM (spells s, spell_tiers st) "
									"LEFT JOIN spell_ts_ability_index ts "
									"ON s.`id` = ts.spell_id "
									"WHERE s.id = st.spell_id AND s.is_active = 1 "
									"ORDER BY s.`id`, `tier`") )
	{
		// error
	}
	else
	{
		while( result.Next() )
		{
			data = new SpellData;
			int32 spell_id		= result.GetInt32Str("id");
			string spell_name	= result.GetStringStr("name");

			/* General Spell info */
			data->id						= spell_id;
			data->inherited_spell_id		= spell_id;
			data->soe_spell_crc				= result.GetInt32Str("soe_spell_crc");
			data->tier						= result.GetInt8Str("tier");
			data->ts_loc_index				= result.GetInt8Str("index");
			data->name.data					= spell_name.c_str();
			data->name.size					= data->name.data.length();
			data->description.data			= result.GetStringStr("description");
			data->description.size			= data->description.data.length();
			data->icon						= result.GetSInt16Str("icon");
			data->icon_heroic_op			= result.GetInt16Str("icon_heroic_op");
			data->icon_backdrop				= result.GetInt16Str("icon_backdrop");
			data->spell_visual				= result.GetInt32Str("spell_visual");
			data->type						= result.GetInt16Str("type");
			data->target_type				= result.GetInt8Str("target_type");
			data->cast_type					= result.GetInt8Str("cast_type");
			data->spell_book_type			= result.GetInt32Str("spell_book_type");
			data->det_type                  = result.GetInt8Str("det_type");
			data->incurable                 = (result.GetInt8Str("incurable") == 1);
			data->control_effect_type       = result.GetInt8Str("control_effect_type");
			data->casting_flags             = result.GetInt32Str("casting_flags");
			data->savage_bar				= result.GetInt8Str("savage_bar");
			data->savage_bar_slot			= result.GetInt8Str("savage_bar_slot");
			data->spell_type				= result.IsNullStr("spell_type+0") ? 0 : result.GetInt8Str("spell_type+0");


			/* Toggles */
			data->interruptable				= ( result.GetInt8Str("interruptable") == 1);
			data->duration_until_cancel		= ( result.GetInt8Str("duration_until_cancel") == 1);
			data->can_effect_raid			= result.GetInt8Str("can_effect_raid");
			data->affect_only_group_members = result.GetInt8Str("affect_only_group_members");
			data->display_spell_tier		= result.GetInt8Str("display_spell_tier");
			data->friendly_spell			= result.GetInt8Str("friendly_spell");
			data->group_spell				= result.GetInt8Str("group_spell");
			data->is_active					= result.GetInt8Str("is_active");
			data->persist_through_death      = ( result.GetInt8Str("persist_through_death") == 1);
			data->cast_while_moving         = ( result.GetInt8Str("cast_while_moving") == 1);
			data->not_maintained            = ( result.GetInt8Str("not_maintained") == 1);
			data->is_aa						= (result.GetInt8Str("is_aa") == 1);

			/* Skill Requirements */
			data->class_skill				= result.GetInt32Str("class_skill");
			data->min_class_skill_req		= result.GetInt16Str("min_class_skill_req");
			data->mastery_skill				= result.GetInt32Str("mastery_skill");

			/* Cost  */
			data->req_concentration			= result.GetInt16Str("req_concentration");
			data->hp_req					= result.GetInt16Str("hp_req");
			data->hp_upkeep					= result.GetInt16Str("hp_upkeep");
			data->hp_req_percent			= result.GetInt8Str("hp_req_percent");
			data->power_req					= result.GetFloatStr("power_req");


			


			data->power_by_level			= ( result.GetInt8Str("power_by_level") == 0)? false : true;
			data->power_upkeep				= result.GetInt16Str("power_upkeep");
			data->power_req_percent			= result.GetInt8Str("power_req_percent");
			data->savagery_req				= result.GetInt16Str("savagery_req");
			data->savagery_upkeep			= result.GetInt16Str("savagery_upkeep");
			data->savagery_req_percent		= result.GetInt8Str("savagery_req_percent");
			data->dissonance_req			= result.GetInt16Str("dissonance_req");
			data->dissonance_upkeep			= result.GetInt16Str("dissonance_upkeep");
			data->dissonance_req_percent	= result.GetInt8Str("dissonance_req_percent");

			/* Spell Parameters */
			data->call_frequency			= result.GetInt32Str("call_frequency");
			data->orig_cast_time			= result.GetInt16Str("cast_time");
			data->cast_time					= result.GetInt16Str("cast_time");
			data->duration1					= result.GetInt32Str("duration1");
			data->duration2					= result.GetInt32Str("duration2");
			data->hit_bonus					= result.GetFloatStr("hit_bonus");
			data->max_aoe_targets			= result.GetInt16Str("max_aoe_targets");
			data->min_range					= result.GetFloatStr("min_range");
			data->radius					= result.GetFloatStr("radius");
			data->range						= result.GetFloatStr("range");
			data->recast					= result.GetFloatStr("recast");
			data->recovery					= result.GetFloatStr("recovery");
			data->resistibility				= result.GetFloatStr("resistibility");
			data->linked_timer				= result.GetInt32Str("linked_timer_id");
			data->spell_name_crc			= result.GetInt32Str("spell_name_crc");
			data->type_group_spell_id		= result.GetSInt32Str("type_group_spell_id");
			data->can_fizzle				= ( result.GetInt8Str("can_fizzle") == 1);

			string given_by					= result.GetStringStr("given_by");
			data->given_by.data				= given_by.c_str();
			data->given_by.size				= data->given_by.data.length();
			
			std::string givenType(given_by);
			boost::algorithm::to_lower(givenType);
			if(givenType == "unset" || givenType.length() < 1) {
				data->given_by_type == GivenByType::GivenBy_Unset;
			}
			else if(givenType == "tradeskillclass") {
				data->given_by_type = GivenByType::GivenBy_TradeskillClass;
			}
			else if(givenType == "spellscroll") {
				data->given_by_type = GivenByType::GivenBy_SpellScroll;
			}
			else if(givenType == "alternateadvancement") {
				data->given_by_type = GivenByType::GivenBy_AltAdvancement;
			}
			else if(givenType == "race") {
				data->given_by_type = GivenByType::GivenBy_Race;
			}
			else if(givenType == "racialinnate") {
				data->given_by_type = GivenByType::GivenBy_RacialInnate;
			}
			else if(givenType == "racialtradition") {
				data->given_by_type = GivenByType::GivenBy_RacialTradition;
			}
			else if(givenType == "class") {
				data->given_by_type = GivenByType::GivenBy_Class;
			}
			else if(givenType == "charactertrait") {
				data->given_by_type = GivenByType::GivenBy_CharacterTrait;
			}
			else if(givenType == "focusabilities") {
				data->given_by_type = GivenByType::GivenBy_FocusAbility;
			}
			else if(givenType == "classtraining") {
				data->given_by_type = GivenByType::GivenBy_ClassTraining;
			}
			else if(givenType == "warderspell") {
				data->given_by_type = GivenByType::GivenBy_WarderSpell;
			}
			else {
				data->given_by_type = GivenByType::GivenBy_Unset;	
			}
			
			/* Cast Messaging */
			string message					= result.GetStringStr("success_message");
			if( message.length() > 0 )
				data->success_message = message;

			message							= result.GetStringStr("fade_message");
			if( message.length() > 0 )
				data->fade_message = string(message);

			message = result.GetStringStr("fade_message_others");
			if (message.length() > 0)
				data->fade_message_others = string(message);

			message							= result.GetStringStr("effect_message");
			if( message.length() > 0 )
				data->effect_message = string(message);

			string lua_script				= result.GetStringStr("lua_script");
			if( lua_script.length() > 0 )
				data->lua_script = string(lua_script);


			/* Load spell level data */
			spell = new Spell(data);

			if(level_data && level_data->count(data->id) > 0)
			{
				vector<LevelArray*>* level_array = &((*level_data)[data->id]);

				for(int8 i=0; i<level_array->size(); i++)
				{
					spell->AddSpellLevel(level_array->at(i)->adventure_class, level_array->at(i)->tradeskill_class, level_array->at(i)->spell_level*10, level_array->at(i)->classic_spell_level);
				}
			}


			/* Add spell to master list */
			master_spell_list.AddSpell(data->id, data->tier, spell);
			total++;

			if( lua_script.length() > 0 )
				LogWrite(SPELL__DEBUG, 5, "Spells", "\t%i. %s (Tier: %i) - '%s'", spell_id, spell_name.c_str(), data->tier, lua_script.c_str()); 		
			else if(data->is_active)
				LogWrite(SPELL__WARNING, 1, "Spells", "\tSpell %s (%u, Tier: %i) set 'Active', but missing LUAScript", spell_name.c_str(), spell_id, data->tier);

		} // end while
	} // end else
	
	LogWrite(SPELL__DEBUG, 0, "Spells", "Loading Spell Effects...");
	LoadSpellEffects();

	LogWrite(SPELL__DEBUG, 0, "Spells", "Loading Spell LUA Data...");
	LoadSpellLuaData();
	
	if(lua_interface) 
	{
		LogWrite(SPELL__DEBUG, 0, "Spells", "Loading Spells Scripts...");
		LoadSpellScriptData();
	}

	if (level_data) {
		map<int32, vector<LevelArray*> >::iterator map_itr;
		vector<LevelArray*>::iterator level_itr;

		for(map_itr = level_data->begin(); map_itr != level_data->end(); map_itr++)
		{
			for(level_itr = map_itr->second.begin(); level_itr != map_itr->second.end(); level_itr++)
			{
				safe_delete(*level_itr);
			}
		}
	}

	safe_delete(level_data);

	LogWrite(SPELL__INFO, 0, "Spells", "Loaded %u Spell%s (took %u seconds)", total, total == 1 ? "" : "s", Timer::GetUnixTimeStamp() - t_now);
}

void WorldDatabase::LoadSpellLuaData(){
	Spell *spell;
	Query query;
	MYSQL_ROW row;
	int32 total = 0;
	MYSQL_RES *result = query.RunQuery2(Q_SELECT, "SELECT `spell_id`,`tier`,`value_type`,`value`,`value2`,`dynamic_helper` "
												  "FROM `spell_data` "
												  "ORDER BY `index_field`");

	while (result && (row = mysql_fetch_row(result))) {
		if ((spell = master_spell_list.GetSpell(atoul(row[0]), atoi(row[1]))) && row[2] && row[3] && row[4] && row[4]) {

			LogWrite(SPELL__DEBUG, 5, "Spells", "\tLoading Spell LUA Data for spell_id: %u", atoul(row[0]));

			if (!strcmp(row[2], "INT"))
				spell->AddSpellLuaDataInt(atoi(row[3]), atoi(row[4]), string(row[5]));
			else if (!strcmp(row[2], "FLOAT"))
				spell->AddSpellLuaDataFloat(atof(row[3]), atof(row[4]),string(row[5]));
			else if (!strcmp(row[2], "BOOL"))
				spell->AddSpellLuaDataBool(!(strncasecmp(row[3], "true", 4)), string(row[5]));
			else if (!strcmp(row[2], "STRING"))
				spell->AddSpellLuaDataString(string(row[3]), string(row[4]), string(row[5]));
			else
				LogWrite(SPELL__ERROR, 0, "Spells", "Invalid Lua Spell data '%s' for Spell ID: %u", row[2], spell->GetSpellID());
			total++;
		}
	}
	LogWrite(SPELL__DEBUG, 0, "Spells", "\tLoaded %i Spell LUA Data entr%s.", total, total == 1 ? "y" : "ies");
}

void WorldDatabase::LoadSpellEffects() {
	Spell *spell;
	Query query;
	MYSQL_ROW row;
	int32 total = 0;
	MYSQL_RES *result = query.RunQuery2(Q_SELECT, "SELECT `spell_id`,`tier`,`percentage`,`bullet`,`description` "
												  "FROM `spell_display_effects` "
												  "ORDER BY `spell_id`,`id` ASC");

	while (result && (row = mysql_fetch_row(result))) {
		if ((spell = master_spell_list.GetSpell(atoul(row[0]), atoi(row[1]))) && row[4]) {

			LogWrite(SPELL__DEBUG, 5, "Spells", "\tLoading Spell Effects for spell_id: %u", atoul(row[0]));

			spell->AddSpellEffect(atoi(row[2]), atoi(row[3]), row[4]);
			total++;
		}
	}
	LogWrite(SPELL__DEBUG, 0, "Spells", "\tLoaded %u Spell Effect%s.", total, total == 1 ? "" : "s");
}

int32 WorldDatabase::LoadPlayerSkillbar(Client* client){
	client->GetPlayer()->ClearQuickbarItems();
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, type, spell_id, slot, text_val, hotbar, tier FROM character_skillbar where char_id = %u", client->GetCharacterID());
	MYSQL_ROW row;
	int32 count = 0;
	while(result && (row = mysql_fetch_row(result))){
		count++;
		int8 tier = atoi(row[6]);
		Spell* spell = master_spell_list.GetSpell(atoul(row[2]), tier);
		if(spell)
			client->GetPlayer()->AddQuickbarItem(atoul(row[5]), atoul(row[3]), atoul(row[1]), spell->GetSpellIcon(), spell->GetSpellIconBackdrop(), spell->GetSpellID(), spell->GetSpellTier(), atoul(row[0]), row[4], false);
		else if(atoul(row[1]) == QUICKBAR_MACRO)
			client->GetPlayer()->AddQuickbarItem(atoul(row[5]), atoul(row[3]), atoul(row[1]), client->GetPlayer()->macro_icons[atoul(row[2])], 0xFFFF, atoul(row[2]), 0, atoul(row[0]), row[4], false);
		else
			client->GetPlayer()->AddQuickbarItem(atoul(row[5]), atoul(row[3]), atoul(row[1]), 0, 0, atoul(row[2]), 0, atoul(row[0]), row[4], false);
	}
	return count;	
}

bool WorldDatabase::DeleteCharacter(int32 account_id, int32 character_id){
	Guild *guild;

	if((guild = guild_list.GetGuild(GetGuildIDByCharacterID(character_id)))) {
		guild->RemoveGuildMember(character_id);
		peer_manager.sendPeersRemoveGuildMember(character_id, guild->GetID(), "*deleted_character*");
	}
	
	Query query;
	//devn00b: Update this whole thing we were missing many tables. This is ugly but swapped 99% of the delete to async to lighten server load.

	//Do character delete immediately. 
	query.RunQuery2(Q_DELETE, "DELETE FROM characters WHERE id=%u AND account_id=%u", character_id, account_id);
	
	//if no character then we shouldn't bother with the rest.
	if (!query.GetAffectedRows())
	{
		//No error just in case ppl try doing stupid stuff
		return false;
	}

	//Async
	query.AddQueryAsync(character_id, this, Q_DELETE, "DELETE FROM character_languages WHERE char_id=%u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "DELETE FROM character_quest_rewards WHERE char_id=%u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "DELETE FROM character_quest_temporary_rewards WHERE char_id=%u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "DELETE FROM character_claim_items where char_id=%u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from charactersproperties where charid = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_aa where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_achievements where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_achievements_items where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_buyback where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_collections where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_collection_items where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_details where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_factions where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_history where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_houses where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_instances where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_items where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_items_group_members where character_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_lua_history where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_macros where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_pictures where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_properties where charid = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_quests where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_quest_progress where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_recipes where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_recipe_books where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_skillbar where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_skills where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_social where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_spells where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_spell_effects where caster_char_id = %u or target_char_id = %u", character_id, character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_spell_effect_targets where caster_char_id = %u or target_char_id = %u", character_id, character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_spirit_shards where charid = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from character_titles where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from char_colors where char_id = %u", character_id);
	query.AddQueryAsync(character_id, this, Q_DELETE, "delete from statistics where char_id = %u", character_id);
		
	return true;
}

void WorldDatabase::DeleteCharacterSpell(int32 character_id, int32 spell_id) {
	if (character_id > 0 && spell_id > 0) {
		Query query;
		query.RunQuery2(Q_DELETE, "DELETE FROM character_spells WHERE char_id=%u AND spell_id=%u", character_id, spell_id);
	}
}

bool WorldDatabase::GetItemResultsToClient (Client* client, const char* varSearch, int maxResults) {
	Query query;
	MYSQL_ROW row;
	int results = 0;

	if( maxResults > 10 && client->GetAdminStatus ( ) < 100 )
		maxResults = 10;
	else if( maxResults > 20 )
		maxResults = 20;

	client->Message(CHANNEL_COLOR_YELLOW, "Item Search Results");
	client->Message(CHANNEL_COLOR_YELLOW, "ResultNum) [ItemID] ItemName");
	string itemsearch_query = string("SELECT id, name FROM items where name like '%%%s%%' limit %i");

	MYSQL_RES* result = query.RunQuery2(Q_SELECT, itemsearch_query.c_str(),getSafeEscapeString(varSearch).c_str(),maxResults);
	while(result && (row = mysql_fetch_row(result))){
		results++;
		client->Message(CHANNEL_COLOR_YELLOW, "%i) [%s] %s",results,row[0],row[1]);
	}

	if(results == 0)
	{
		client->Message(CHANNEL_COLOR_YELLOW, "No Items Found.");
		return false;
	}

	client->Message(CHANNEL_COLOR_YELLOW, "%i Items Found.",results);
	return true;
}

void WorldDatabase::SaveWorldTime(WorldTime* time){
	Query query;
	query.RunQuery2(Q_REPLACE, "replace into variables (variable_name, variable_value) values('gametime', '%i/%i/%i %i:%i')", time->month, time->day, time->year, time->hour, time->minute);
}

void WorldDatabase::SaveBugReport(const char* category, const char* subcategory, const char* causes_crash, const char* reproducible, const char* summary, const char* description, const char* version, const char* player, int32 account_id, const char* spawn_name, int32 spawn_id, int32 zone_id){
	Query query;

	int32 dbVersion = rule_manager.GetGlobalRule(R_World, DatabaseVersion)->GetInt32();

	string bug_report = string("insert into bugs (category, subcategory, causes_crash, reproducible, summary, description, version, player, account_id, spawn_name, spawn_id, zone_id, dbversion, worldversion) values('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %u, '%s', %u, %u, %u, '%s')");
	query.RunQuery2(Q_INSERT, bug_report.c_str(), getSafeEscapeString(category).c_str(), getSafeEscapeString(subcategory).c_str(),
		getSafeEscapeString(causes_crash).c_str(), getSafeEscapeString(reproducible).c_str(), getSafeEscapeString(summary).c_str(),
		getSafeEscapeString(description).c_str(), getSafeEscapeString(version).c_str(), getSafeEscapeString(player).c_str(), account_id,
		getSafeEscapeString(spawn_name).c_str(), spawn_id, zone_id, dbVersion, CURRENT_VERSION);
	FixBugReport();
	FixBugReport();
	FixBugReport();
}

void WorldDatabase::FixBugReport(){
	Query query;
	string bug_report = string("update bugs set description = REPLACE(description,SUBSTRING(description,INSTR(description,'%'), 3),char(CONV(SUBSTRING(description,INSTR(description,'%')+1, 2), 16, 10))), summary = REPLACE(summary,SUBSTRING(summary,INSTR(summary,'%'), 3),char(CONV(SUBSTRING(summary,INSTR(summary,'%')+1, 2), 16, 10)))");
	query.RunQuery2(bug_report.c_str(), Q_UPDATE);
}

int32 WorldDatabase::LoadQuests(){
	Query query;
	MYSQL_ROW row;
	std::string querystr = std::string("SELECT `quest_id`, `name`, `type`, `zone`, `level`, `enc_level`, `description`, `lua_script`, `completed_text`, `spawn_id`, `shareable_flag`, `deleteable` FROM `quests`");
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, querystr.c_str());
	Quest* quest = 0;
	char* name = 0;
	char* type = 0;
	char* zone = 0;
	int8 level = 0;
	int8 enc_level = 0;
	char* description = 0;
	char* script = 0;
	int32 total = 0;
	int32 id = 0;
	char* completed_description = 0;
	int32 return_npc_id = 0;
	if(result){
		while(result && (row = mysql_fetch_row(result))){
			id = atoul(row[0]);
			name = row[1];
			type = row[2];
			zone = row[3];
			level = atoul(row[4]);
			enc_level = atoul(row[5]);
			description = row[6];
			script = row[7];
			completed_description = row[8];
			return_npc_id = atoi(row[9]);
			if(lua_interface) {
				quest = lua_interface->LoadQuest(id, name, type, zone, level, description, script);
			}
			if(quest){

				LogWrite(QUEST__DEBUG, 5, "Quests", "\tLoading Quest: '%s' (%u)", name, id);

				LoadQuestDetails(quest);
				string compDescription;
				if (completed_description == NULL)
				{
					compDescription = string("Missing! Notify Developer");
					LogWrite(QUEST__WARNING, 5, "Quests", "\tLoading Quest MISSING completed_text in quests table for: '%s' (%u)", name, id);
				}
				else
					compDescription = string(completed_description);

				quest->SetCompletedDescription(string(compDescription));
				quest->SetQuestReturnNPC(return_npc_id);
				quest->SetEncounterLevel(enc_level);
				quest->SetQuestShareableFlag(atoul(row[10]));
				quest->SetCanDeleteQuest(atoul(row[11]));
				total++;
				master_quest_list.AddQuest(id, quest);
			}
			else
			{
				LogWrite(QUEST__ERROR, 5, "Quests", "\tFAILED LOADING QUEST: '%s' (%u), check that script file exists/permissions correct: %s", name, id, (script && strlen(script)) ? script : "Not Set, Missing!");
			}
		}
	}
	LogWrite(QUEST__DEBUG, 0, "Quest", "\tLoaded %i Quest(s)", total);
	return total;
}

void WorldDatabase::LoadQuestDetails(Quest* quest) {
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `type`, `subtype`, `value`, `faction_id`, `quantity` FROM `quest_details` WHERE `quest_id`=%u", quest->GetQuestID());
	string type;
	string subtype;
	sint64 value;
	int32 faction_id;
	int32 quantity;
	while (result && (row = mysql_fetch_row(result))) {
		type = string(row[0]);
		subtype = string(row[1]);
		value = atoi(row[2]);
		faction_id = atoi(row[3]);
		quantity = atoi(row[4]);


		LogWrite(QUEST__DEBUG, 5, "Quests", "\t- Type: %s, SubType: %s, Val: %u, Faction: %u, Qty: %u", type.c_str(), subtype.c_str(), value, faction_id, quantity);


		if (type == "Prereq") {
			if (subtype == "Class")
				quest->AddPrereqClass(value);
			else if (subtype == "Faction")
				quest->AddPrereqFaction(faction_id, value);
			else if (subtype == "Item") {
				Item* master_item = master_item_list.GetItem(value);
				if (master_item) {
					Item* item = new Item(master_item);
					quest->AddPrereqItem(item);
				}
			}
			else if (subtype == "AdvLevel")
				quest->SetPrereqLevel(value);
			else if (subtype == "Quest")
				quest->AddPrereqQuest(value);
			else if (subtype == "Race")
				quest->AddPrereqRace(value);
			else if (subtype == "TSClass")
				quest->AddPrereqTradeskillClass(value);
			else if (subtype == "TSLevel")
				quest->SetPrereqTSLevel(value);
			else if (subtype == "MaxTSLevel")
				quest->SetPrereqMaxTSLevel(value);
			else if (subtype == "MaxAdvLevel")
				quest->SetPrereqMaxLevel(value);
		}
		else if (type == "Reward") {
			if (subtype == "Item") {
				Item* master_item = master_item_list.GetItem(value);
				if (master_item) {
					Item* item = new Item(master_item);
					item->details.count = quantity;
					quest->AddRewardItem(item);
				}
			}
			else if (subtype == "Selectable") {
				Item* master_item = master_item_list.GetItem(value);
				if (master_item) {
					Item* item = new Item(master_item);
					item->details.count = quantity;
					quest->AddSelectableRewardItem(item);
				}
			}
			else if (subtype == "Coin") {
				int32 copper = 0;
				int32 silver = 0;
				int32 gold = 0;
				int32 plat = 0;
				if (value >= 1000000) {
					plat = value / 1000000;
					value -= 1000000 * plat;
				}
				if (value >= 10000) {
					gold = value / 10000;
					value -= 10000 * gold;
				}
				if (value >= 100) {
					silver = value / 100;
					value -= 100 * silver;
				}
				if (value > 0)
					copper = value;
				quest->AddRewardCoins(copper, silver, gold, plat);
			}
			else if (subtype == "MaxCoin") {
				quest->AddRewardCoinsMax(value);
			}
			else if (subtype == "Faction")
				quest->AddRewardFaction(faction_id, value);
			else if (subtype == "Experience")
				quest->SetRewardXP(value);
			else if (subtype == "TSExperience")
				quest->SetRewardTSXP(value);
		}
	}
}

void WorldDatabase::LoadMerchantInformation() {
	LogWrite(MERCHANT__DEBUG, 0, "Merchant", "\tClearing Merchant Inventory...");
	world.DeleteMerchantItems();

	LogWrite(MERCHANT__DEBUG, 0, "Merchant", "\tLoading Merchant Inventory...");
	LoadMerchantInventory();

	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT merchant_id, inventory_id FROM merchants ORDER BY merchant_id");
	int32 total = 0;
	int32 last_merchant_id = 0;
	int32 id = 0;
	MerchantInfo* merchant = 0;
	if(result) {
		while(result && (row = mysql_fetch_row(result))) {
			id = atoul(row[0]);

			LogWrite(MERCHANT__DEBUG, 5, "Merchant", "\tMerchantID: %u, InventoryID: %u", id, atoul(row[1]));

			if(id != last_merchant_id) {
				if(merchant)
					world.AddMerchantInfo(last_merchant_id, merchant);
				merchant = new MerchantInfo;
				last_merchant_id = id;
				total++;
			}
			merchant->inventory_ids.push_back(atoul(row[1]));
		}
		if(merchant)
			world.AddMerchantInfo(last_merchant_id, merchant);
	}
	LogWrite(MERCHANT__DEBUG, 0, "Merchant", "\tLoaded %i Merchant List(s)", total);
}

void WorldDatabase::LoadMerchantInventory(){
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT inventory_id, item_id, quantity, price_item_id, price_item_qty, price_item2_id, price_item2_qty, price_status, price_coins, price_stationcash FROM merchant_inventory ORDER BY inventory_id");
	int32 total = 0;
	int32 id;

	if(result) {
		while(result && (row = mysql_fetch_row(result))) {
			MerchantItemInfo ItemInfo;
			id = atoul(row[0]);
			ItemInfo.item_id = atoul(row[1]);
			ItemInfo.quantity = atoi(row[2]);
			ItemInfo.price_item_id = atoul(row[3]);
			ItemInfo.price_item_qty = atoi(row[4]);
			ItemInfo.price_item2_id = atoul(row[5]);
			ItemInfo.price_item2_qty = atoi(row[6]);
			ItemInfo.price_status = atoul(row[7]);
			ItemInfo.price_coins = atoul(row[8]);
			ItemInfo.price_stationcash = atoul(row[9]);
			LogWrite(MERCHANT__DEBUG, 5, "Merchant", "\tInventoryID: %u, ItemID: %u, Qty: %u", id, ItemInfo.item_id, ItemInfo.quantity);
			world.AddMerchantItem(id, ItemInfo);
			total++;
		}
	}
	LogWrite(MERCHANT__DEBUG, 0, "Merchant", "\tLoaded %i Merchant Inventory Item(s)", total);
}

string WorldDatabase::GetMerchantDescription(int32 merchant_id) {
	Query query;
	MYSQL_ROW row;
	string description;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `description` FROM `merchants` WHERE `merchant_id`=%u", merchant_id);
	if (result && (row = mysql_fetch_row(result)))
		description = string(row[0]);
	return description;
}

void WorldDatabase::LoadTransporters(ZoneServer* zone){
	int32 total = 0;
	zone->DeleteGlobalTransporters();
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT transport_id, transport_type, display_name, message, destination_zone_id, destination_x, destination_y, destination_z, destination_heading, trigger_location_zone_id, trigger_location_x, trigger_location_y, trigger_location_z, trigger_radius, cost, id, min_level, max_level, quest_req, quest_step_req, quest_completed, map_x, map_y, expansion_flag, holiday_flag, min_client_version, max_client_version, flight_path_id, mount_id, mount_red_color, mount_green_color, mount_blue_color FROM transporters ORDER BY transport_id");
	if(result){
		while(result && (row = mysql_fetch_row(result))){
			LogWrite(TRANSPORT__DEBUG, 5, "Transport", "---Loading Transporter ID: %u, transport_type: %s", row[0], row[1]);
			LogWrite(TRANSPORT__DEBUG, 7, "Transport", "---display_name: %s, message: %s", row[2], row[3]);
			LogWrite(TRANSPORT__DEBUG, 7, "Transport", "---destination_zone_id: %s", row[4]);
			LogWrite(TRANSPORT__DEBUG, 7, "Transport", "---destination_x: %s, destination_y: %s, destination_z: %s, destination_heading: %s", row[5], row[6], row[7], row[8]);
			LogWrite(TRANSPORT__DEBUG, 7, "Transport", "---trigger_location_zone_id: %s", row[9]);
			LogWrite(TRANSPORT__DEBUG, 7, "Transport", "---trigger_location_x: %s, trigger_location_y: %s, trigger_location_z: %s", row[10], row[11], row[12], row[13]);
			LogWrite(TRANSPORT__DEBUG, 7, "Transport", "---trigger_radius: %s, cost: %s, id: %s", row[14], row[15]); 
			string name = "";
			if(row[2])
				name = string(row[2]);
			string message = "";
			if(row[3])
				message = string(row[3]);

			if(row[1] && strcmp(row[1], "Zone") == 0)
				zone->AddTransporter(atoul(row[0]), TRANSPORT_TYPE_ZONE, name, message, atoul(row[4]), atof(row[5]), atof(row[6]), atof(row[7]), atof(row[8]), atoul(row[14]), atoul(row[15]), atoi(row[16]), atoi(row[17]), atoul(row[18]), atoi(row[19]), atoul(row[20]), atoul(row[21]), atoul(row[22]), atoul(row[23]), atoul(row[24]), atoul(row[25]), atoul(row[26]), atoul(row[27]), atoul(row[28]), atoul(row[29]), atoul(row[30]), atoul(row[31]));
			else if (row[1] && strcmp(row[1], "Flight") == 0)
				zone->AddTransporter(atoul(row[0]), TRANSPORT_TYPE_FLIGHT, name, message, atoul(row[4]), atof(row[5]), atof(row[6]), atof(row[7]), atof(row[8]), atoul(row[14]), atoul(row[15]), atoi(row[16]), atoi(row[17]), atoul(row[18]), atoi(row[19]), atoul(row[20]), atoul(row[21]), atoul(row[22]), atoul(row[23]), atoul(row[24]), atoul(row[25]), atoul(row[26]), atoul(row[27]), atoul(row[28]), atoul(row[29]), atoul(row[30]), atoul(row[31]));
			else if(row[1] && strcmp(row[1], "Location") == 0)
				zone->AddLocationTransporter(atoul(row[9]), message, atof(row[10]), atof(row[11]), atof(row[12]), atof(row[13]), atoul(row[4]), atof(row[5]), atof(row[6]), atof(row[7]), atof(row[8]), atoul(row[14]), atoul(row[15]));
			else
				zone->AddTransporter(atoul(row[0]), TRANSPORT_TYPE_GENERIC, "", message, atoul(row[4]), atof(row[5]), atof(row[6]), atof(row[7]), atof(row[8]), atoul(row[14]), atoul(row[15]), atoi(row[16]), atoi(row[17]), atoul(row[18]), atoi(row[19]), atoul(row[20]), atoul(row[21]), atoul(row[22]), atoul(row[23]), atoul(row[24]), atoul(row[25]), atoul(row[26]), atoul(row[27]), atoul(row[28]), atoul(row[29]), atoul(row[30]), atoul(row[31]));
			total++;
		}
	}
	LogWrite(TRANSPORT__DEBUG, 0, "Transport", "--Loaded %i Transporter(s)", total);
	LoadTransportMaps(zone);
}

void WorldDatabase::LoadFogInit(string zone, PacketStruct* packet)
{
	LogWrite(WORLD__TRACE, 9, "World", "Enter: %s", __FUNCTION__);

	if(!packet || zone.length() == 0)
		return;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT highest, lowest, zone_name, explored_map_name, unexplored_map_name, bounds1_x, bounds1_z, bounds2_x, bounds2_z, bounds3_x, bounds3_z, bounds4_x, bounds4_z, explored_key, unexplored_key, map_id FROM map_data where zone_name like '%s%%'", getSafeEscapeString(&zone).c_str());
	if(result){
		int count = mysql_num_rows(result);
		int i=0;
		int64 explored_key;
		int64 unexplored_key;
		packet->setArrayLengthByName("num_maps", count);
		while(result && (row = mysql_fetch_row(result))){
			packet->setDataByName("highest_z", atof(row[0]));		
			packet->setDataByName("lowest_z", atof(row[1]));
			packet->setDataByName("map_id", atoul(row[15]));
			packet->setArrayDataByName("unknown7", 1600, i);
			packet->setArrayDataByName("unknown8", 1200, i);
			packet->setArrayDataByName("zone_name", row[2], i);
			packet->setArrayDataByName("explored_map_name", row[3], i);
			packet->setArrayDataByName("unexplored_map_name", row[4], i);
			packet->setArrayDataByName("map_bounds1_x", atof(row[5]), i);
			packet->setArrayDataByName("map_bounds1_z", atof(row[6]), i);
			packet->setArrayDataByName("map_bounds2_x", atof(row[7]), i);
			packet->setArrayDataByName("map_bounds2_z", atof(row[8]), i);
			packet->setArrayDataByName("map_bounds3_x", atof(row[9]), i);
			packet->setArrayDataByName("map_bounds3_z", atof(row[10]), i);
			packet->setArrayDataByName("map_bounds4_x", atof(row[11]), i);
			packet->setArrayDataByName("map_bounds4_z", atof(row[12]), i);
#ifdef WIN32
			explored_key = _strtoui64(row[13], NULL, 10);
			unexplored_key = _strtoui64(row[14], NULL, 10);
#else
			explored_key = strtoull(row[13], 0, 10);
			unexplored_key = strtoull(row[14], 0, 10);
#endif
			packet->setArrayDataByName("explored_key", explored_key, i);
			packet->setArrayDataByName("unexplored_key", unexplored_key, i);
			i++;
		}
	}
	LogWrite(WORLD__TRACE, 9, "World", "Exit: %s", __FUNCTION__);
}

string WorldDatabase::GetColumnNames(char* name){
	Query query;
	MYSQL_ROW row;
	string columns = "";
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "show columns FROM %s", name);
	if(result && mysql_num_rows(result) > 0){
		int16 i = 0;
		while((row = mysql_fetch_row(result))){
			if(strcmp(row[0], "table_data_version") != 0){
				if(i>0)
					columns.append(",");
				columns.append(row[0]);
				i++;
			}
		}
	}
	columns.append("");
	return columns;
}

void WorldDatabase::ToggleCharacterOnline() {
	Query query;
	query.RunQuery2(Q_UPDATE, "UPDATE characters SET is_online = 0;");
}

void WorldDatabase::ToggleCharacterOnline(Client* client, int8 toggle) {
	if (client) {
		Query query;
		Player* player = client->GetPlayer();
		//if(!player->CheckPlayerInfo())
		//	return;
		if (player) 
		{
			LogWrite(PLAYER__DEBUG, 0, "Player", "Toggling Character %s", toggle ? "ONLINE!" : "OFFLINE!");
			query.RunQuery2(Q_UPDATE, "UPDATE characters SET is_online=%i WHERE id = %u;", toggle, client->GetCharacterID());
		}
	}
}

void WorldDatabase::LoadPlayerStatistics(Player* player, int32 char_id) {
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT stat_id, stat_value, stat_date FROM statistics WHERE char_id=%i", char_id);
	while (result && (row = mysql_fetch_row(result))) {
		int32 stat_id = atoi(row[0]);
		sint32 stat_value = atoi(row[1]);
		int32 stat_date = atoi(row[2]);
		player->AddPlayerStatistic(stat_id, stat_value, stat_date);
	}
}

void WorldDatabase::WritePlayerStatistic(Player *player, Statistic* stat) {
	if (player && player->GetCharacterID() > 0 && stat) {
		Query query;
		query.RunQuery2(Q_INSERT, "INSERT INTO statistics (char_id, guild_id, stat_id, stat_value, stat_date) VALUES(%i, %i, %i, %i, %i) ON DUPLICATE KEY UPDATE stat_value = %i, stat_date = %i;",
			player->GetCharacterID(), 0, stat->stat_id, stat->stat_value, stat->stat_date,
			stat->stat_value, stat->stat_date);
	}
}

void WorldDatabase::LoadServerStatistics() 
{
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT stat_id, stat_value, stat_date FROM statistics WHERE char_id=0 AND guild_id=0");
	while (result && (row = mysql_fetch_row(result))) {
		int32 stat_id = atoi(row[0]);
		sint32 stat_value = atoi(row[1]);
		int32 stat_date = atoi(row[2]);
		world.AddServerStatistic(stat_id, stat_value, stat_date);
		LogWrite(INIT__DEBUG, 5, "Stats", "Loading Stat ID %i, value: %i", stat_id, stat_value);
	}
}

void WorldDatabase::WriteServerStatistic(Statistic* stat) {
	if (stat) {
		Query query;
		query.RunQuery2(Q_INSERT, "INSERT INTO statistics (char_id, guild_id, stat_id, stat_value, stat_date) VALUES(0, 0, %i, %i, %i) ON DUPLICATE KEY UPDATE stat_value = %i, stat_date = %i;", 
			stat->stat_id, stat->stat_value, stat->stat_date, 
			stat->stat_value, stat->stat_date);
	}
}

void WorldDatabase::WriteServerStatistic(int32 stat_id, sint32 stat_value) {
	Query query;
	query.RunQuery2(Q_INSERT, "INSERT INTO statistics (char_id, guild_id, stat_id, stat_value, stat_date) VALUES(0, 0, %i, %i, %i) ON DUPLICATE KEY UPDATE stat_value = %i, stat_date = %i;", 
		stat_id, stat_value, Timer::GetUnixTimeStamp(),
		stat_value, Timer::GetUnixTimeStamp());
}

void WorldDatabase::WriteServerStatisticsNeededQueries() {
	Query query1, query2, query3;
	MYSQL_ROW row1, row2, row3;

	// Number of unique accounts
	MYSQL_RES* result1 = query1.RunQuery2(Q_SELECT, "SELECT COUNT(DISTINCT account_id) FROM characters");
	if (result1 && (row1 = mysql_fetch_row(result1)) && row1[0] != NULL)
		WriteServerStatistic(STAT_SERVER_NUM_ACCOUNTS, atoi(row1[0]));
	else
		WriteServerStatistic(STAT_SERVER_NUM_ACCOUNTS, 0);

	// Number of characters
	MYSQL_RES* result2 = query2.RunQuery2(Q_SELECT, "SELECT COUNT(id) FROM characters");
	if (result2 && (row2 = mysql_fetch_row(result2)) && row2[0] != NULL)
		WriteServerStatistic(STAT_SERVER_NUM_CHARACTERS, atoi(row2[0]));
	else
		WriteServerStatistic(STAT_SERVER_NUM_CHARACTERS, 0);

	// Average character level
	MYSQL_RES* result3 = query3.RunQuery2(Q_SELECT, "SELECT ROUND(AVG(level)) FROM characters");
	if (result3 && (row3 = mysql_fetch_row(result3)) && row3[0] != NULL)
		WriteServerStatistic(STAT_SERVER_AVG_CHAR_LEVEL, atoi(row3[0]));
	else
		WriteServerStatistic(STAT_SERVER_AVG_CHAR_LEVEL, 0);
}

map<int32,int32>* WorldDatabase::GetInstanceRemovedSpawns(int32 instance_id, int8 type)
{
	DatabaseResult result;
	map<int32,int32>* ret = NULL;

	LogWrite(SPAWN__TRACE, 1, "Spawn", "Enter %s", __FUNCTION__);

	LogWrite(INSTANCE__DEBUG, 0, "Instance", "Loading removed spawns for instance_id: %u, spawn_type: %i", instance_id, type);

	if( !database_new.Select(&result, "SELECT spawn_location_entry_id, respawn_time FROM instance_spawns_removed WHERE instance_id = %i AND spawn_type = %i", instance_id, type) )
	{
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in GetInstanceRemovedSpawns() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
		return ret;
	}
	else
	{
		if( result.GetNumRows() > 0 )
		{
			ret = new map<int32,int32>;

			while( result.Next() )
			{
				int32 spawn_location_entry_id = result.GetInt32Str("spawn_location_entry_id");
				/* 
					respawnTime == 0 - never respawn
					respawnTime = 1 - spawn now
					respawnTime > 1 (continue timer) 
				*/
				int32 respawntime = result.GetInt32Str("respawn_time"); 

				LogWrite(INSTANCE__DEBUG, 5, "Instance", "Found spawn point: %u, respawn time: %i", spawn_location_entry_id, respawntime);

				ret->insert(make_pair(spawn_location_entry_id, respawntime));
			}
		}
		else
			LogWrite(INSTANCE__DEBUG, 0, "Instance", "No removed spawns found for instance_id: %u, spawn_type: %i", instance_id, type);

	}

	LogWrite(SPAWN__TRACE, 1, "Spawn", "Exit %s", __FUNCTION__);

	return ret;
}

bool WorldDatabase::CheckVectorForValue(vector<int32>* vector, int32 value) {
	if ( vector != NULL )
	{
		for(int32 i=0;i<vector->size();i++)
		{
			int32 compare = vector->at(i);
			if ( compare == value )
				return true;
		}
	}

	return false;
}

int32 WorldDatabase::CheckSpawnRemoveInfo(map<int32,int32>* inmap, int32 spawn_location_entry_id) 
{
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Enter %s", __FUNCTION__);

	map<int32, int32>::iterator iter;

	if ( inmap != NULL )
	{
		iter = inmap->find(spawn_location_entry_id);
		if(iter != inmap->end()) {
			return (int32)iter->second;
		}
	}

	return 1;
}

int32 WorldDatabase::AddCharacterInstance(int32 char_id, int32 instance_id, string zone_name, int8 instance_type, int32 last_success, int32 last_failure, int32 success_lockout, int32 failure_lockout)
{
	int32 ret = 0;
	if( !database_new.Query("INSERT INTO character_instances (char_id, instance_id, instance_zone_name, instance_type, last_success_timestamp, last_failure_timestamp, success_lockout_time, failure_lockout_time) VALUES (%u, %u, '%s', %i, %u, %u, %u, %u) ", char_id, instance_id, database_new.EscapeStr(zone_name).c_str(), instance_type, last_success, last_failure, success_lockout, failure_lockout) )
	{
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in AddCharacterInstance() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
		return 0;
	}
	ret = database_new.LastInsertID();

	LogWrite(INSTANCE__DEBUG, 0, "Instance", "Adding character %u to instance: %u", char_id, instance_id);
	//LogWrite(INSTANCE__DEBUG, 1, "Instance", "-- Reenter: %u, Reset: %u, Lockout: %u", grant_reenter_time_left, grant_reset_time_left, lockout_time);

	return ret;
}

bool WorldDatabase::UpdateCharacterInstanceTimers(int32 char_id, int32 instance_id, int32 lockout_time, int32 reset_time, int32 reenter_time )
{
	if ( lockout_time > 0 && reset_time > 0  && reenter_time > 0 )
		database_new.Query("UPDATE character_instances SET lockout_time = %i, grant_reset_time_left = %i, grant_reenter_time_left = %i WHERE char_id = %i AND instance_id = %i", lockout_time, reset_time, reenter_time, char_id, instance_id);
	else if ( lockout_time > 0 && reset_time > 0 )
		database_new.Query("UPDATE character_instances SET lockout_time = %i, grant_reset_time_left = %i WHERE char_id = %i AND instance_id = %i", lockout_time, reset_time, char_id, instance_id);
	else if ( reset_time > 0  && reenter_time > 0 )
		database_new.Query("UPDATE character_instances SET grant_reset_time_left = %i, grant_reenter_time_left = %i WHERE char_id = %i AND instance_id = %i",reset_time, reenter_time, char_id, instance_id);
	else if ( lockout_time > 0  && reenter_time > 0 )
		database_new.Query("UPDATE character_instances SET lockout_time = %i, grant_reenter_time_left = %i WHERE char_id = %i AND instance_id = %i", lockout_time, reenter_time, char_id, instance_id);
	else if ( lockout_time > 0 )
		database_new.Query("UPDATE character_instances SET lockout_time = %i WHERE char_id = %i AND instance_id = %i", lockout_time, char_id, instance_id);
	else if ( reset_time > 0 )
		database_new.Query("UPDATE character_instances SET grant_reset_time_left = %i WHERE char_id = %i AND instance_id = %i", reset_time, char_id, instance_id);
	else if ( reenter_time > 0 )
		database_new.Query("UPDATE character_instances SET grant_reenter_time_left = %i WHERE char_id = %i AND instance_id = %i", reenter_time, char_id, instance_id);

	if( database_new.GetError() ) 
	{
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in UpdateCharacterInstanceTimers() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
		return false;
	}
	else
	{
		if ( database_new.AffectedRows() > 0 )
		{
			LogWrite(INSTANCE__DEBUG, 0, "Instance", "Updating instance timers for character %u to instance: %u", char_id, instance_id);
			LogWrite(INSTANCE__DEBUG, 1, "Instance", "-- Reenter: %u, Reset: %u, Lockout: %u", reenter_time, reset_time, lockout_time);
			return true;
		}
		else
			return false;
	}
}

bool WorldDatabase::UpdateCharacterInstance(int32 char_id, string zone_name, int32 instance_id, int8 type, int32 timestamp) {
	// type = 1 = success timestamp
	// type = 2 = failure timestamp
	if (instance_id > 0) {
		if (type == 1) {
			database_new.Query("UPDATE character_instances SET instance_id = %u, last_success_timestamp = %u WHERE char_id = %u AND instance_zone_name = '%s'", instance_id, timestamp, char_id, database_new.EscapeStr(zone_name).c_str());
		}
		else if (type == 2) {
			database_new.Query("UPDATE character_instances SET instance_id = %u, last_failure_timestamp = %u WHERE char_id = %u AND instance_zone_name = '%s'", instance_id, timestamp, char_id, database_new.EscapeStr(zone_name).c_str());
		}
		else {
			database_new.Query("UPDATE character_instances SET instance_id = %u WHERE char_id = %u AND instance_zone_name = '%s'", instance_id, char_id, database_new.EscapeStr(zone_name).c_str());
		}
	}
	else {
		if (type == 1) {
			database_new.Query("UPDATE character_instances SET last_success_timestamp = %u WHERE char_id = %u AND instance_zone_name = '%s'", timestamp, char_id, database_new.EscapeStr(zone_name).c_str());
		}
		else if (type == 2) {
			database_new.Query("UPDATE character_instances SET last_failure_timestamp = %u WHERE char_id = %u AND instance_zone_name = '%s'", timestamp, char_id, database_new.EscapeStr(zone_name).c_str());
		}
	}

	if (database_new.GetError()) {
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in UpdateCharacterInstance() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
		return false;
	}

	return true;
}

bool WorldDatabase::VerifyInstanceID(int32 char_id, int32 instance_id) {
	DatabaseResult result;
	database_new.Select(&result, "SELECT COUNT(id) as num_instances FROM instances WHERE id = %u", instance_id);

	if (result.Next() && result.GetInt32Str("num_instances") == 0) {
		DeleteCharacterFromInstance(char_id, instance_id);
		return false;
	}

	return true;
}

bool WorldDatabase::UpdateInstancedSpawnRemoved(int32 spawn_location_entry_id, int32 spawn_type, int32 respawn_time, int32 instance_id )
{
	LogWrite(INSTANCE__DEBUG, 0, "Instance", "Updating removed spawns for instance_id: %u", instance_id);
	LogWrite(INSTANCE__DEBUG, 1, "Instance", "-- Spawn Location Entry ID: %u, Type: %u, Respawn: %u", spawn_location_entry_id, spawn_type, respawn_time);

	if( !database_new.Query("UPDATE instance_spawns_removed SET respawn_time = %i WHERE spawn_location_entry_id = %i AND spawn_type = %i AND instance_id = %i", respawn_time, spawn_location_entry_id, spawn_type, instance_id) )
	{
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in UpdateInstancedSpawnRemoved() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
		return false;
	}

	if ( database_new.AffectedRows() > 0 )
	{
		LogWrite(INSTANCE__DEBUG, 0, "Instance", "Updated removed spawns for instance_id: %u", instance_id);
		return true;
	}
	else
		return false;
}

int32 WorldDatabase::CreateNewInstance(int32 zone_id, int32 playersMinLevel, int32 playersMaxLevel, int32 playersavgLevel, int32 playersfirstLevel)
{
	int32 ret = 0;

	LogWrite(INSTANCE__DEBUG, 0, "Instance", "Creating new instance for zone: %u ", zone_id);

	if( !database_new.Query("INSERT INTO instances (zone_id, player_minlevel, player_maxlevel, player_avglevel, player_firstlevel) VALUES (%u, %u, %u, %u, %u)", zone_id, playersMinLevel, playersMaxLevel, playersavgLevel, playersfirstLevel) )
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in CreateNewInstance() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
	else
		ret = database_new.LastInsertID();

	if( ret > 0 )
		LogWrite(INSTANCE__DEBUG, 0, "Instance", "Created new instance_id %u for zone: %u ", ret, zone_id);

	return ret;
}

int32 WorldDatabase::CreateInstanceSpawnRemoved(int32 spawn_location_entry_id, int32 spawn_type, int32 respawn_time, int32 instance_id )
{
	int32 ret = 0;

	LogWrite(INSTANCE__DEBUG, 3, "Instance", "Creating new instance spawn removed entries for instance_id: %u ", instance_id);
	LogWrite(INSTANCE__DEBUG, 5, "Instance", "-- Spawn Location Entry ID: %u, Type: %u, Respawn: %u", spawn_location_entry_id, spawn_type, respawn_time);

	if( !database_new.Query("INSERT INTO instance_spawns_removed (spawn_location_entry_id, spawn_type, instance_id, respawn_time) values(%u, %u, %u, %u)", spawn_location_entry_id, spawn_type, instance_id, respawn_time) )
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in CreateInstanceSpawnRemoved() query '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
	else
		ret = database_new.LastInsertID();

	// potentially spammy, if it calls for every spawn added. Set to level 3 or 5?
	if( ret > 0 )
		LogWrite(INSTANCE__DEBUG, 5, "Instance", "Created new spawn removed entry: %u for instance_id %u", ret, instance_id);

	return ret;
}

bool WorldDatabase::DeleteInstance(int32 instance_id)
{
	if( !database_new.Query("DELETE FROM instances WHERE id = %u", instance_id) )
	{
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in DeleteInstance() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
		return false;
	}
	
	/* JA: should not need this delete with FK/Constraints
	if( !database_new.Query("DELETE FROM instance_spawns_removed WHERE instance_id = %u", instance_id) )
	{
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in DeleteInstance() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
		return false;
	}
	*/

	// Remove the instance from the character_instances table
	database_new.Query("UPDATE `character_instances` SET `instance_id` = 0 WHERE `instance_id` = %u", instance_id);

	LogWrite(INSTANCE__DEBUG, 0, "Instance", "Deleted instance_id %u", instance_id);

	return true;
}

bool WorldDatabase::DeleteInstanceSpawnRemoved(int32 instance_id, int32 spawn_location_entry_id) 
{
	if( !database_new.Query("DELETE FROM instance_spawns_removed WHERE instance_id = %u AND spawn_location_entry_id = %u", instance_id, spawn_location_entry_id) )
	{
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in DeleteInstanceSpawnRemoved() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
		return false;
	}

	LogWrite(INSTANCE__DEBUG, 0, "Instance", "Deleted removed spawn: %u for instance_id %u", spawn_location_entry_id, instance_id);

	return true;
}

bool WorldDatabase::DeleteCharacterFromInstance(int32 char_id, int32 instance_id)
{
	LogWrite(INSTANCE__DEBUG, 0, "Instance", "Delete character %u from instance_id %u.", char_id, instance_id);

	if( !database_new.Query("UPDATE `character_instances` SET `instance_id` = 0 WHERE `instance_id` = %u AND `char_id` = %u", instance_id, char_id) )
	{
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in DeleteCharacterFromInstance() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
		return false;
	}

	if ( database_new.AffectedRows() == 0 ) // didn't find an instance to delete
	{
		LogWrite(INSTANCE__DEBUG, 1, "Instance", "No instance_id %u for character %u to delete.", instance_id, char_id);
		return false;
	}
	else
	{
		// delete entire instance if the last player has left
		DatabaseResult result;
		database_new.Select(&result, "SELECT count(id) as num_instances FROM character_instances where instance_id = %u",instance_id);

		if(result.Next() && result.GetInt32Str("num_instances") == 0)
		{
			LogWrite(INSTANCE__DEBUG, 0, "Instance", "No characters in instance: Delete instance_id %u.", instance_id);
			DeleteInstance(instance_id);
		}
	}

	return true;
}

bool WorldDatabase::LoadCharacterInstances(Client* client) 
{
	DatabaseResult result;
	DatabaseResult result2;

	bool addedInstance = false;

	database_new.Select(&result, "SELECT `id`, `instance_id`, `instance_zone_name`, `instance_type`, `last_success_timestamp`, `last_failure_timestamp`, `success_lockout_time`, `failure_lockout_time` FROM `character_instances` WHERE `char_id` = %u", client->GetCharacterID());

	if( result.GetNumRows() > 0 )
	{
		while( result.Next() )
		{
			int32 zone_id = 0;
			int32 instance_id = result.GetInt32Str("instance_id");
			// If `instance_id` is greater then 0 then get the zone id with it, else get the zone id from the zone name
			if (instance_id != 0) {
				if (database_new.Select(&result2, "SELECT `zone_id` FROM `instances` WHERE `id` = %u", instance_id)) {
					if (result2.Next())
						zone_id = result2.GetInt32Str("zone_id");
				}
			}
			 if (zone_id == 0)
				zone_id = GetZoneID(result.GetStringStr("instance_zone_name"));

			client->GetPlayer()->GetCharacterInstances()->AddInstance(
				result.GetInt32Str("id"),
				instance_id,
				result.GetInt32Str("last_success_timestamp"),
				result.GetInt32Str("last_failure_timestamp"),
				result.GetInt32Str("success_lockout_time"),
				result.GetInt32Str("failure_lockout_time"),
				zone_id,
				result.GetInt8Str("instance_type"),
				string(result.GetStringStr("instance_zone_name"))
				);
			addedInstance = true;
		}
	}

	return addedInstance;
}


bool WorldDatabase::DeletePersistedRespawn(int32 zone_id, int32 spawn_location_entry_id) 
{
	if( !database_new.Query("DELETE FROM persisted_respawns WHERE zone_id = %u AND spawn_location_entry_id = %u", zone_id, spawn_location_entry_id) )
	{
		LogWrite(INSTANCE__ERROR, 0, "Instance", "Error in DeletePersistedRespawn() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
		return false;
	}

	LogWrite(INSTANCE__DEBUG, 0, "Instance", "Deleted persisted spawn: %u for zone_id %u", spawn_location_entry_id, zone_id);

	return true;
}

int32 WorldDatabase::CreatePersistedRespawn(int32 spawn_location_entry_id, int32 spawn_type, int32 respawn_time, int32 zone_id)
{
	int32 ret = 0;

	LogWrite(ZONE__DEBUG, 5, "Instance", "-- Creating new persisted Location Entry ID: %u, Type: %u, Respawn: %u, ZoneID: %u", spawn_location_entry_id, spawn_type, respawn_time, zone_id);

	if( !database_new.Query("INSERT INTO persisted_respawns (spawn_location_entry_id, spawn_type, zone_id, respawn_time) values(%u, %u, %u, %u)", spawn_location_entry_id, spawn_type, zone_id, respawn_time) )
		LogWrite(ZONE__ERROR, 0, "Instance", "Error in CreatePersistedRespawn() query '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
	else
		ret = database_new.LastInsertID();

	// potentially spammy, if it calls for every spawn added. Set to level 3 or 5?
	if( ret > 0 )
		LogWrite(ZONE__DEBUG, 5, "Instance", "Created new persisted respawn entry: %u for zone_id %u", ret, zone_id);

	return ret;
}

map<int32,int32>* WorldDatabase::GetPersistedSpawns(int32 zone_id, int8 type)
{
	DatabaseResult result;
	map<int32,int32>* ret = NULL;

	LogWrite(SPAWN__TRACE, 1, "Spawn", "Enter %s", __FUNCTION__);

	LogWrite(ZONE__DEBUG, 0, "Zone", "Loading persisted spawns for zone_id: %u, spawn_type: %u", zone_id, type);

	if( !database_new.Select(&result, "SELECT spawn_location_entry_id, respawn_time FROM persisted_respawns WHERE zone_id = %u AND spawn_type = %u", zone_id, type) )
	{
		if(database_new.GetError()) {
			LogWrite(ZONE__ERROR, 0, "Zone", "Error in GetPersistedSpawns() '%s': %i", database_new.GetErrorMsg(), database_new.GetError());
		}
		else {
			
		}
		return ret;
	}
	else
	{
		if( result.GetNumRows() > 0 )
		{
			ret = new map<int32,int32>;

			while( result.Next() )
			{
				int32 spawn_location_entry_id = result.GetInt32Str("spawn_location_entry_id");
				/* 
					respawnTime == 0 - never respawn
					respawnTime = 1 - spawn now
					respawnTime > 1 (continue timer) 
				*/
				int32 respawntime = result.GetInt32Str("respawn_time"); 

				LogWrite(ZONE__ERROR, 5, "Zone", "Found persisted spawn point: %u, respawn time: %u", spawn_location_entry_id, respawntime);

				ret->insert(make_pair(spawn_location_entry_id, respawntime));
			}
		}
		else
			LogWrite(ZONE__ERROR, 0, "Zone", "No persisted spawns found for zone_id: %u, spawn_type: %u", zone_id, type);

	}

	LogWrite(SPAWN__TRACE, 1, "Spawn", "Exit %s", __FUNCTION__);

	return ret;
}


void WorldDatabase::UpdateLoginEquipment() 
{
	LogWrite(INIT__LOGIN_DEBUG, 0, "Login", "Updating `character_items` CRC in %s", __FUNCTION__);

	database_new.Query("UPDATE character_items SET login_checksum = CRC32(CRC32(type) + CRC32(slot) + CRC32(item_id)) WHERE `type` = 'EQUIPPED' AND ( slot <= 8 OR slot = 19 )");
}

MutexMap<int32, LoginEquipmentUpdate>* WorldDatabase::GetEquipmentUpdates() 
{
	DatabaseResult result;
	MutexMap<int32, LoginEquipmentUpdate>* ret = 0;
	LoginEquipmentUpdate update;
	int32 count = 0;

	LogWrite(INIT__LOGIN_DEBUG, 0, "Login", "Looking for Login Appearance Updates...");
	
	// TODO: Someday store the equipment colors in character_items, for custom colorization of gear (?)
	if( database_new.Select(&result, "SELECT ci.id, ci.char_id, ia.equip_type, ia.red, ia.green, ia.blue, ia.highlight_red, ia.highlight_green, ia.highlight_blue, ci.slot FROM characters c JOIN character_items ci ON c.id = ci.char_id JOIN item_appearances ia ON ci.item_id = ia.item_id WHERE c.deleted = 0 AND ci.type = 'EQUIPPED' AND ( ci.slot <= 8 OR ci.slot = 19 ) AND ci.login_checksum <> CRC32(CRC32(`ci`.`type`) + CRC32(ci.slot) + CRC32(ci.item_id)) ORDER BY ci.char_id, ci.slot") )
	{
		while( result.Next() )
		{
			LogWrite(INIT__LOGIN_DEBUG, 5, "Login", "Found update for char_id %i!", result.GetInt32Str("char_id"));

			if(!ret)
				ret = new MutexMap<int32, LoginEquipmentUpdate>();

			update.world_char_id	= result.GetInt32Str("char_id");
			update.equip_type		= result.GetInt16Str("equip_type");
			update.red				= result.GetInt8Str("red");
			update.green			= result.GetInt8Str("green");
			update.blue				= result.GetInt8Str("blue");
			update.highlight_red	= result.GetInt8Str("highlight_red");
			update.highlight_green	= result.GetInt8Str("highlight_green");
			update.highlight_blue	= result.GetInt8Str("highlight_blue");
			update.slot				= result.GetInt8Str("slot");
			ret->Put(result.GetInt32Str("id"), update);
			count++;

		}
	}

	if(count)
		LogWrite(INIT__LOGIN_DEBUG, 0, "Login", "Found %i Login Appearance Update%s...", count, count == 1 ? "" : "s");

	return ret;
}


MutexMap<int32, LoginEquipmentUpdate>* WorldDatabase::GetEquipmentUpdates(int32 char_id) 
{
	DatabaseResult result;
	MutexMap<int32, LoginEquipmentUpdate>* ret = 0;
	LoginEquipmentUpdate update;
	int32 count = 0;

	LogWrite(INIT__LOGIN_DEBUG, 0, "Login", "Looking for Login Appearance Updates for char_id: %u", char_id);

	// TODO: Someday store the equipment colors in character_items, for custom colorization of gear (?)
	if( database_new.Select(&result, "SELECT ci.id, ci.char_id, ia.equip_type, ia.red, green, ia.blue, ia.highlight_red, ia.highlight_green, ia.highlight_blue, ci.slot FROM characters c JOIN character_items ci ON c.id = ci.char_id JOIN item_appearances ia ON ci.item_id = ia.item_id WHERE c.deleted = 0 AND ci.type = 'EQUIPPED' AND ( ci.slot <= 8 OR ci.slot = 19 ) AND ci.login_checksum <> CRC32(CRC32(ci.type) + CRC32(ci.slot) + CRC32(ci.item_id)) AND ci.char_id = %u ORDER BY ci.slot", char_id) )
	{
		while( result.Next() )
		{
			LogWrite(INIT__LOGIN_DEBUG, 5, "Login", "Found update for char_id %i!", result.GetInt32Str("char_id"));

			if(!ret)
				ret = new MutexMap<int32, LoginEquipmentUpdate>();

			update.world_char_id	= char_id;
			update.equip_type		= result.GetInt16Str("equip_type");
			update.red				= result.GetInt8Str("red");
			update.green			= result.GetInt8Str("green");
			update.blue				= result.GetInt8Str("blue");
			update.highlight_red	= result.GetInt8Str("highlight_red");
			update.highlight_green	= result.GetInt8Str("highlight_green");
			update.highlight_blue	= result.GetInt8Str("highlight_blue");
			update.slot				= result.GetInt8Str("slot");
			ret->Put(result.GetInt32Str("id"), update);
			count++;
		}
	}
		
	if(count)
		LogWrite(INIT__LOGIN_DEBUG, 0, "Login", "Found %i Login Appearance Update%s...", count, count == 1 ? "" : "s");

	return ret;
}


void WorldDatabase::UpdateLoginZones() {
	Query query;
	LogWrite(INIT__LOGIN_DEBUG, 0, "Login", "Updating `zones` CRC in %s", __FUNCTION__);
	query.RunQuery2("UPDATE zones SET login_checksum = CRC32(CRC32(id) + CRC32(`name`) + CRC32(`file`) + CRC32(description))", Q_UPDATE);
}

MutexMap<int32, LoginZoneUpdate>* WorldDatabase::GetZoneUpdates() {
	LogWrite(INIT__LOGIN_DEBUG, 0, "Login", "Looking for Login Zone Updates...");
	MutexMap<int32, LoginZoneUpdate>* ret = 0;
	LoginZoneUpdate update;
	Query query;
	MYSQL_ROW row;
	int32 count = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, name, description FROM zones where login_checksum != crc32(crc32(id) + crc32(name) + crc32(file) + crc32(description))");
	while(result && (row = mysql_fetch_row(result))) {
		if(row[0] && row[1]) {

			LogWrite(INIT__LOGIN_DEBUG, 5, "Login", "Found update for zone_id %i!", atoi(row[0]));

			if(!ret)
				ret = new MutexMap<int32, LoginZoneUpdate>();			
			update.name = string(row[1]);
			if(row[2])
				update.description = string(row[2]);
			else
				update.description = "";
			ret->Put(atoi(row[0]), update);
			count++;
		}
	}
	if(count)
		LogWrite(INIT__LOGIN_DEBUG, 0, "Login", "Found %i Login Zone Update%s...", count, count == 1 ? "" : "s");
	return ret;
}

void WorldDatabase::LoadLocationGrids(ZoneServer* zone) {
	if (zone) {
		Query query;
		MYSQL_ROW row;
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `id`, `grid_id`, `name`, `include_y`, `discovery` FROM `locations` WHERE `zone_id`=%u", zone->GetZoneID());
		while (result && (row = mysql_fetch_row(result))) {
			LocationGrid* grid = new LocationGrid;
			grid->id = atoul(row[0]);
			grid->grid_id = atoul(row[1]);
			grid->name = string(row[2]);
			grid->include_y = (atoi(row[3]) == 1);
			grid->discovery = (atoi(row[4]) == 1);
			if (LoadLocationGridLocations(grid))
				zone->AddLocationGrid(grid);
			else
				safe_delete(grid);
		}
	}
}

bool WorldDatabase::LoadLocationGridLocations(LocationGrid* grid) {
	bool ret = false;
	if (grid) {
		Query query;
		int row_count = 0;
		MYSQL_ROW row;
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `id`, `x`, `y`, `z` FROM `location_details` WHERE `location_id`=%u", grid->id);
		if (result) {
			while (result && (row = mysql_fetch_row(result))) {
				row_count++;
				Location* location = new Location;
				location->id = atoul(row[0]);
				location->x = atof(row[1]);
				location->y = atof(row[2]);
				location->z = atof(row[3]);
				grid->locations.Add(location);
			}
			ret = true;
		}
		if(row_count > 0 && row_count < 3)
			LogWrite(WORLD__WARNING, 0, "World", "Grid '%s' only has %u location(s).  A minimum of 3 is needed for a proper location based grid.", grid->name.c_str(), row_count);
	}
	return ret;
}

int32 WorldDatabase::CreateLocation(int32 zone_id, int32 grid_id, const char* name, bool include_y) {
	int32 ret = 0;
	if (name && strlen(name) > 0) {
		Query query;
		query.RunQuery2(Q_INSERT, "INSERT INTO `locations` (`zone_id`, `grid_id`, `name`, `include_y`) VALUES (%u, %u, '%s', %u)", zone_id, grid_id, name, include_y == true ? 1 : 0);
		ret = query.GetLastInsertedID();
	}
	return ret;
}

bool WorldDatabase::AddLocationPoint(int32 location_id, float x, float y, float z) {
	bool ret = false;
	if (LocationExists(location_id)) {
		Query query;
		query.RunQuery2(Q_INSERT, "INSERT INTO `location_details` (`location_id`, `x`, `y`, `z`) VALUES (%u, %f, %f, %f)", location_id, x, y, z);
		ret = true;
	}
	return ret;
}

bool WorldDatabase::DeleteLocation(int32 location_id) {
	bool ret = false;
	if (LocationExists(location_id)) {
		Query query;
		query.RunQuery2(Q_DELETE, "DELETE FROM `locations` WHERE `id`=%u", location_id);
		ret = true;
	}
	return ret;
}

bool WorldDatabase::DeleteLocationPoint(int32 location_point_id) {
	Query query;
	query.RunQuery2(Q_DELETE, "DELETE FROM `location_details` WHERE `id`=%u", location_point_id);
	return true;
}

void WorldDatabase::ListLocations(Client* client) {
	if (client) {
		Query query;
		MYSQL_ROW row;
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Listing all locations:");
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `id`, `zone_id`, `grid_id`, `name` FROM `locations`");
		while (result && (row = mysql_fetch_row(result))) {
			int32 id = atoul(row[0]);
			int32 zone_id = atoul(row[1]);
			int32 grid_id = atoul(row[2]);
			const char* name = row[3];
			client->Message(CHANNEL_COLOR_YELLOW, "%u) Zone ID: %u  Grid ID:%u  Name: '%s'", id, zone_id, grid_id, name);
		}
	}
}

void WorldDatabase::ListLocationPoints(Client* client, int32 location_id) {
	if (client) {
		if (LocationExists(location_id)) {
			Query query;
			client->Message(CHANNEL_COLOR_YELLOW, "Listing all points for location ID %u:", location_id);
			MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `id`, `x`, `y`, `z` FROM `location_details` WHERE `location_id`=%u", location_id);
			MYSQL_ROW row;
			while (result && (row = mysql_fetch_row(result))) {
				int32 id = atoul(row[0]);
				float x = atof(row[1]);
				float y = atof(row[2]);
				float z = atof(row[3]);
				client->Message(CHANNEL_COLOR_YELLOW, "%u) (%f, %f, %f)", id, x, y, z);
			}
		}
		else
			client->Message(CHANNEL_COLOR_YELLOW, "A location with ID %u does not exist", location_id);
	}
}

bool WorldDatabase::LocationExists(int32 location_id) {
	bool ret = false;
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT COUNT(id) FROM `locations` WHERE `id`=%u", location_id);
	MYSQL_ROW row;
	if (result && (row = mysql_fetch_row(result))) {
		if (atoul(row[0]) > 0)
			ret = true;
	}
	return ret;
}

bool WorldDatabase::QueriesFromFile(const char * file) {
	return database_new.QueriesFromFile(file);
}

bool WorldDatabase::CheckBannedIPs(const char* loginIP)
{
	// til you build the table, all IPs are allowed
 	return false;
}

sint32 WorldDatabase::AddMasterTitle(const char* titleName, int8 isPrefix)
{
	if(titleName == nullptr || strlen(titleName) < 1)
	{
		LogWrite(DATABASE__ERROR, 0, "DBNew", "AddMasterTitle called with missing titleName");
		return -1;
	}

	Query query;
	Title* title = master_titles_list.GetTitleByName(titleName);
	
	if(title)
		return title->GetID();

	query.RunQuery2(Q_INSERT, "INSERT INTO titles set title='%s', prefix=%u", titleName, isPrefix);
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(DATABASE__ERROR, 0, "Database", "Error in AddMasterTitle query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}

	int32 last_insert_id = query.GetLastInsertedID();
	if(last_insert_id > 0)
	{
		title = new Title;
		title->SetID(last_insert_id);
		title->SetName(titleName);
		title->SetPrefix(isPrefix);
		master_titles_list.AddTitle(title);
		return (sint32)last_insert_id;
	}


	return -1;
}

void WorldDatabase::LoadTitles(){
	Query query;
	MYSQL_ROW row;
	int32 count = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, title, prefix FROM titles");
	if(result && mysql_num_rows(result) > 0){
		Title* title = 0;
		while(result && (row = mysql_fetch_row(result))){
			sint32 idx = atoi(row[0]);
			LogWrite(WORLD__DEBUG, 5, "World", "\tLoading Title '%s' (%u), Prefix: %i, Index: %u", row[1], idx, atoi(row[2]), count);
			title = new Title;
			title->SetID(idx);
			title->SetName(row[1]);
			title->SetPrefix(atoi(row[2]));
			master_titles_list.AddTitle(title);
			count++;
		}
	}
	LogWrite(WORLD__DEBUG, 0, "World", "\tLoaded %u Title%s", count, count == 1 ? "" : "s");
}

sint32 WorldDatabase::LoadCharacterTitles(int32 char_id, Player *player){
	LogWrite(WORLD__DEBUG, 0, "World", "Loading Titles for player '%s'...", player->GetName());
	Query query;
	MYSQL_ROW row;
	sint32 index = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT title_id, title, prefix FROM character_titles, titles WHERE character_titles.title_id = titles.id AND character_titles.char_id = %u", char_id);
	if(result && mysql_num_rows(result) > 0){
		while(result && (row = mysql_fetch_row(result))){
			LogWrite(WORLD__DEBUG, 5, "World", "\tLoading Title ID: %u, Title: '%s' Index: %u", atoul(row[0]), row[1], index);
			player->AddTitle(index, row[1], atoi(row[2]));
			index++;
		}
	}
	return index;
}

sint32 WorldDatabase::GetCharPrefixIndex(int32 char_id, Player *player){
	LogWrite(PLAYER__DEBUG, 0, "Player", "Getting current title index for player '%s'...", player->GetName());
	Query query;
	MYSQL_ROW row;
	sint32 ret = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT prefix_title FROM character_details WHERE char_id = %u", char_id);
	if(result && mysql_num_rows(result) > 0)
		while(result && (row = mysql_fetch_row(result))){
			 ret  = atoi(row[0]);
			LogWrite(PLAYER__DEBUG, 5, "Player", "\tPrefix Index: %i", ret);
		}
	return ret;
}

sint32 WorldDatabase::GetCharSuffixIndex(int32 char_id, Player *player){
	LogWrite(PLAYER__DEBUG, 0, "Player", "Getting current title index for player '%s'...", player->GetName());
	Query query;
	MYSQL_ROW row;
	sint32 ret = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT suffix_title FROM character_details WHERE char_id = %u", char_id);
	if(result && mysql_num_rows(result) > 0)
		while(result && (row = mysql_fetch_row(result))){
			ret = atoi(row[0]);
			LogWrite(PLAYER__DEBUG, 5, "Player", "\tSuffix Index: %i", ret);
		}
	return ret;
}

void WorldDatabase::SaveCharPrefixIndex(sint32 index, int32 char_id){
	Query query;
	query.RunQuery2(Q_UPDATE, "UPDATE character_details SET prefix_title = %i WHERE char_id = %u", index, char_id);
	LogWrite(PLAYER__DEBUG, 0, "Player", "Saving Prefix Index %i for character id '%u'...", index, char_id);
}

void WorldDatabase::SaveCharSuffixIndex(sint32 index, int32 char_id){
	Query query;
	query.RunQuery2(Q_SELECT, "UPDATE character_details SET suffix_title = %i WHERE char_id = %u", index, char_id);
	LogWrite(PLAYER__DEBUG, 0, "Player", "Saving Suffix Index %i for character id %u...", index, char_id);
}

sint32 WorldDatabase::AddCharacterTitle(sint32 index, int32 char_id, Spawn* player) {
	if(!player || !player->IsPlayer())
	{
		LogWrite(DATABASE__ERROR, 0, "DBNew", "AddCharacterTitle spawn is not a player: %s", player ? player->GetName() : "Unset");
		return -1;
	}

	Title* title = master_titles_list.GetTitle(index);
	if(!title)
	{
		LogWrite(DATABASE__ERROR, 0, "DBNew", "AddCharacterTitle title index %u missing from master_titles_list for player: %s (%u)", index, player ? player->GetName() : "Unset", char_id);
		return -1;
	}

	Query query;
	LogWrite(PLAYER__DEBUG, 0, "Player", "Adding titles for char_id: %u, index: %i", char_id, index);
	query.RunQuery2(Q_INSERT, "INSERT IGNORE INTO character_titles (char_id, title_id) VALUES (%u, %i)", char_id, index);
	sint32 curIndex = (sint32)((Player*)player)->GetPlayerTitles()->Size();
	((Player*)player)->AddTitle(curIndex++, title->GetName(), title->GetPrefix(), title->GetSaveNeeded());

	return curIndex;
}

void WorldDatabase::LoadLanguages()
{
	int32 count = 0;
	Query query;
	MYSQL_ROW row;

	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, language FROM languages");

	if(result && mysql_num_rows(result) > 0)
	{
		Language* language = 0;

		while(result && (row = mysql_fetch_row(result)))
		{
			LogWrite(WORLD__DEBUG, 5, "World", "\tLoading language '%s' , ID: %u", row[1], atoul(row[0]));
			language = new Language;
			language->SetID(atoul(row[0]));
			language->SetName(row[1]);
			master_languages_list.AddLanguage(language);
			count++;
		}
	}
	LogWrite(WORLD__DEBUG, 0, "World", "\tLoaded %u Language%s", count, count == 1 ? "" : "s");
}

int32 WorldDatabase::LoadCharacterLanguages(int32 char_id, Player *player)
{
	LogWrite(WORLD__DEBUG, 0, "World", "Loading Languages for player '%s'...", player->GetName());
	Query query;
	MYSQL_ROW row;
	int32 count = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT language_id, language FROM character_languages, languages WHERE character_languages.language_id = languages.id AND character_languages.char_id = %u", char_id);

	if(result && mysql_num_rows(result) > 0)
	{
		while(result && (row = mysql_fetch_row(result)))
		{
			LogWrite(WORLD__DEBUG, 5, "World", "\tLoading Language ID: %u, Language: '%s'", atoul(row[0]), row[1]);
			player->AddLanguage(atoul(row[0]), row[1]);
			count++;
		}
	}
	return count;
}

int16 WorldDatabase::GetCharacterCurrentLang(int32 char_id, Player *player)
{
	LogWrite(PLAYER__DEBUG, 0, "Player", "Getting current language for player '%s'...", player->GetName());
	Query query;
	MYSQL_ROW row;
	int16 ret = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT current_language FROM character_details WHERE char_id = %u", char_id);

	if(result && mysql_num_rows(result) > 0)
		while(result && (row = mysql_fetch_row(result)))
		{
			ret = atoi(row[0]);
			LogWrite(PLAYER__DEBUG, 5, "Player", "\tLanguage ID: %i", ret);
		}
	return ret;
}

void WorldDatabase::SaveCharacterCurrentLang(int32 id, int32 char_id, Client *client)
{
	Query query;
	query.RunQuery2(Q_UPDATE, "UPDATE character_details SET current_language = %i WHERE char_id = %u", id, char_id);
	LogWrite(PLAYER__DEBUG, 3, "Player", "Saving current language ID %i for player '%s'...", id, client->GetPlayer()->GetName());
}

void WorldDatabase::SaveCharacterLang(int32 char_id, int32 lang_id) {
	if (!database_new.Query("INSERT INTO character_languages (char_id, language_id) VALUES (%u, %u)", char_id, lang_id))
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
}

// JA - this is not used yet, lots more to consider for storing player history
void WorldDatabase::LoadCharacterHistory(int32 char_id, Player *player) 
{
	DatabaseResult result;

	// Use -1 on type and subtype to turn the enum into an int and make it a 0 index
	if (!database_new.Select(&result, "SELECT type-1, subtype-1, value, value2, location, event_id, event_date FROM character_history WHERE char_id = %u", char_id)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}

	while (result.Next()) {

		int8 type = result.GetInt8(0);
		int8 subtype = result.GetInt8(1);

		HistoryData* hd = new HistoryData;
		hd->Value = result.GetInt32(2);
		hd->Value2 = result.GetInt32(3);
		strcpy(hd->Location, result.GetString(4));
		// skipped event id as use for it has not been determined yet
		hd->EventDate = result.GetInt32(6);
		hd->needs_save = false;

		player->LoadPlayerHistory(type, subtype, hd);
	}
}

void WorldDatabase::LoadSpellErrors() {
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `version`, `error_index`, `value` FROM `spell_error_versions`");

	if (result && mysql_num_rows(result) > 0) {
		while ((row = mysql_fetch_row(result))) {
			master_spell_list.AddSpellError(atoi(row[0]), atoi(row[1]), atoi(row[2]));
		}
	}
}

void WorldDatabase::SaveCharacterHistory(Player* player, int8 type, int8 subtype, int32 value, int32 value2, char* location, int32 event_date) {
	string str_type("");
	string str_subtype("");
	switch (type) {
	case HISTORY_TYPE_NONE:
		str_type = "None";
		break;
	case HISTORY_TYPE_DEATH:
		str_type = "Death";
		break;
	case HISTORY_TYPE_DISCOVERY:
		str_type = "Discovery";
		break;
	case HISTORY_TYPE_XP:
		str_type = "XP";
		break;
	default:
		LogWrite(PLAYER__ERROR, 0, "Player", "WorldDatabase::SaveCharacterHistory() - Invalid history type given (%i) with subtype (%i), character history NOT saved.", type, subtype);
		return;
	}
	switch (subtype) {
	case HISTORY_SUBTYPE_NONE:
		str_subtype = "None";
		break;
	case HISTORY_SUBTYPE_ADVENTURE:
		str_subtype = "Adventure";
		break;
	case HISTORY_SUBTYPE_TRADESKILL:
		str_subtype = "Tradeskill";
		break;
	case HISTORY_SUBTYPE_QUEST:
		str_subtype = "Quest";
		break;
	case HISTORY_SUBTYPE_AA:
		str_subtype = "AA";
		break;
	case HISTORY_SUBTYPE_ITEM:
		str_subtype = "Item";
		break;
	case HISTORY_SUBTYPE_LOCATION:	
		str_subtype = "Location";
		break;
	default:
		LogWrite(PLAYER__ERROR, 0, "Player", "WorldDatabase::SaveCharacterHistory() - Invalid history sub type given (%i) with type (%i), character history NOT saved.", subtype, type);
		return;
	}

	LogWrite(PLAYER__DEBUG, 1, "Player", "Saving character history, type = %s (%i) subtype = %s (%i)", (char*)str_type.c_str(), type, (char*)str_subtype.c_str(), subtype);

	Query query;
	query.AddQueryAsync(player->GetCharacterID(), this, Q_REPLACE, "replace into character_history (char_id, type, subtype, value, value2, location, event_date) values (%u, '%s', '%s', %i, %i, '%s', %u)", 
		player->GetCharacterID(), str_type.c_str(), str_subtype.c_str(), value, value2, getSafeEscapeString(location).c_str(), event_date);
}

void WorldDatabase::LoadTransportMaps(ZoneServer* zone) {
	int32 total = 0;

	LogWrite(TRANSPORT__DEBUG, 0, "Transport", "-Loading Transporter Maps...");
	zone->DeleteTransporterMaps();

	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `transport_id`, `map_name` FROM `transport_maps`");
	if(result) {
		while(result && (row = mysql_fetch_row(result))){
			zone->AddTransportMap(atoul(row[0]), string(row[1]));
			total++;
		}
	}
	LogWrite(TRANSPORT__DEBUG, 0, "Transport", "--Loaded %i Transporter Maps", total);
}

bool WorldDatabase::LoadSign(ZoneServer* zone, int32 spawn_id) {
	Sign* sign = 0;
	int32 id = 0;
	DatabaseResult result;
	database_new.Select(&result, "SELECT ss.spawn_id, s.name, s.model_type, s.size, s.show_command_icon, ss.widget_id, ss.widget_x, ss.widget_y, ss.widget_z, s.command_primary, s.command_secondary, s.collision_radius, ss.icon, ss.type, ss.title, ss.description, ss.sign_distance, ss.zone_id, ss.zone_x, ss.zone_y, ss.zone_z, ss.zone_heading, ss.include_heading, ss.include_location, s.transport_id, s.size_offset, s.display_hand_icon, s.visual_state, s.disable_sounds, s.merchant_min_level, s.merchant_max_level, s.aaxp_rewards, s.loot_tier, s.loot_drop_type, ss.language\n"
								 "FROM spawn s\n"
								 "INNER JOIN spawn_signs ss\n"
								 "ON ss.spawn_id = s.id\n"
								 "WHERE s.id = %u\n",
								 spawn_id);

	if (result.GetNumRows() > 0 && result.Next()) {
		id = result.GetInt32(0);
		sign = new Sign();
		sign->SetDatabaseID(id);
		strcpy(sign->appearance.name, result.GetString(1));
		sign->appearance.model_type = result.GetInt16(2);
		sign->SetSize(result.GetInt16(3));
		sign->appearance.show_command_icon = result.GetInt8(4);
		sign->SetWidgetID(result.GetInt32(5));
		sign->SetWidgetX(result.GetFloat(6));
		sign->SetWidgetY(result.GetFloat(7));
		sign->SetWidgetZ(result.GetFloat(8));
		vector<EntityCommand*>* primary_command_list = zone->GetEntityCommandList(result.GetInt32(9));
		if(primary_command_list){
			sign->SetPrimaryCommands(primary_command_list);
			sign->primary_command_list_id = result.GetInt32(9);
		}

		vector<EntityCommand*>* secondary_command_list = zone->GetEntityCommandList(result.GetInt32(10));
		if (secondary_command_list) {
			sign->SetSecondaryCommands(secondary_command_list);
			sign->secondary_command_list_id = result.GetInt32(10);
		}

		sign->appearance.pos.collision_radius = result.GetInt16(11);
		sign->SetSignIcon(result.GetInt8(12));
		if(strncasecmp(result.GetString(13), "Generic", 7) == 0)
			sign->SetSignType(SIGN_TYPE_GENERIC);
		else if(strncasecmp(result.GetString(13), "Zone", 4) == 0)
			sign->SetSignType(SIGN_TYPE_ZONE);
		sign->SetSignTitle(result.GetString(14));
		sign->SetSignDescription(result.GetString(15));
		sign->SetSignDistance(result.GetFloat(16));
		sign->SetSignZoneID(result.GetInt32(17));
		sign->SetSignZoneX(result.GetFloat(18));
		sign->SetSignZoneY(result.GetFloat(19));
		sign->SetSignZoneZ(result.GetFloat(20));
		sign->SetSignZoneHeading(result.GetFloat(21));
		sign->SetIncludeHeading(result.GetInt8(22) == 1);
		sign->SetIncludeLocation(result.GetInt8(23) == 1);
		sign->SetTransporterID(result.GetInt32(24));
		sign->SetSizeOffset(result.GetInt8(25));
		sign->appearance.display_hand_icon = result.GetInt8(26);
		sign->SetVisualState(result.GetInt16(27));

		sign->SetSoundsDisabled(result.GetInt8(28));

		sign->SetMerchantLevelRange(result.GetInt32(29), result.GetInt32(30));
		
		sign->SetAAXPRewards(result.GetInt32(31));

		sign->SetLootTier(result.GetInt32(32));

		sign->SetLootDropType(result.GetInt32(33));
		
		sign->SetLanguage(result.GetInt8(34));
		
		zone->AddSign(id, sign);


		LogWrite(SIGN__DEBUG, 0, "Sign", "Loaded Sign: '%s' (%u).", sign->appearance.name, spawn_id);
		return true;
	}
	LogWrite(SIGN__DEBUG, 0, "Sign", "Unable to find a sign for spawn id of %u", spawn_id);
	return false;
}

bool WorldDatabase::LoadWidget(ZoneServer* zone, int32 spawn_id) {
	Widget* widget = 0;
	int32 id = 0;
	DatabaseResult result;

	database_new.Select(&result, "SELECT sw.spawn_id, s.name, s.model_type, s.size, s.show_command_icon, sw.widget_id, sw.widget_x, sw.widget_y, sw.widget_z, s.command_primary, s.command_secondary, s.collision_radius, sw.include_heading, sw.include_location, sw.icon, sw.type, sw.open_heading, sw.open_y, sw.action_spawn_id, sw.open_sound_file, sw.close_sound_file, sw.open_duration, sw.closed_heading, sw.linked_spawn_id, sw.close_y, s.transport_id, s.size_offset, sw.house_id, sw.open_x, sw.open_z, sw.close_x, sw.close_z, s.display_hand_icon, s.disable_sounds, s.merchant_min_level, s.merchant_max_level, s.aaxp_rewards, s.loot_tier, s.loot_drop_type\n"
								 "FROM spawn s\n"
								 "INNER JOIN spawn_widgets sw\n"
								 "ON sw.spawn_id = s.id\n"
								 "WHERE s.id = %u",
								 spawn_id);

	if (result.GetNumRows() > 0 && result.Next()) {
		id = result.GetInt32(0);
		widget = new Widget();
		widget->SetDatabaseID(id);
		strcpy(widget->appearance.name, result.GetString(1));
		widget->appearance.model_type = result.GetInt16(2);
		widget->SetSize(result.GetInt16(3));
		widget->appearance.show_command_icon = result.GetInt8(4);
		widget->SetWidgetID(result.GetInt32(5));
		widget->SetWidgetX(result.GetFloat(6));
		widget->SetWidgetY(result.GetFloat(7));
		widget->SetWidgetZ(result.GetFloat(8));
		vector<EntityCommand*>* primary_command_list = zone->GetEntityCommandList(result.GetInt32(9));
		if(primary_command_list){
			widget->SetPrimaryCommands(primary_command_list);
			widget->primary_command_list_id = result.GetInt32(9);
		}

		vector<EntityCommand*>* secondary_command_list = zone->GetEntityCommandList(result.GetInt32(10));
		if (secondary_command_list) {
			widget->SetSecondaryCommands(secondary_command_list);
			widget->secondary_command_list_id = result.GetInt32(10);
		}

		widget->appearance.pos.collision_radius = result.GetInt16(11);
		widget->SetIncludeHeading(result.GetInt8(12) == 1);
		widget->SetIncludeLocation(result.GetInt8(13) == 1);
		widget->SetWidgetIcon(result.GetInt8(14));
		if(strncasecmp(result.GetString(15),"Generic", 7) == 0)
			widget->SetWidgetType(WIDGET_TYPE_GENERIC);
		else if(strncasecmp(result.GetString(15),"Door", 4) == 0)
			widget->SetWidgetType(WIDGET_TYPE_DOOR);
		widget->SetOpenHeading(result.GetFloat(16));
		widget->SetOpenY(result.GetFloat(17));
		widget->SetActionSpawnID(result.GetInt32(18));
		if(!result.IsNull(19) && strlen(result.GetString(19)) > 5)
			widget->SetOpenSound(result.GetString(19));
		if(!result.IsNull(20) && strlen(result.GetString(20)) > 5)
			widget->SetCloseSound(result.GetString(20));
		widget->SetOpenDuration(result.GetInt16(21));
		widget->SetClosedHeading(result.GetFloat(22));
		widget->SetLinkedSpawnID(result.GetInt32(23));
		widget->SetCloseY(result.GetFloat(24));
		widget->SetTransporterID(result.GetInt32(25));
		widget->SetSizeOffset(result.GetInt8(26));
		widget->SetHouseID(result.GetInt32(27));
		widget->SetOpenX(result.GetFloat(28));
		widget->SetOpenZ(result.GetFloat(29));
		widget->SetCloseX(result.GetFloat(30));
		widget->SetCloseZ(result.GetFloat(31));
		widget->appearance.display_hand_icon = result.GetInt8(32);

		widget->SetSoundsDisabled(result.GetInt8(33));

		widget->SetMerchantLevelRange(result.GetInt32(34), result.GetInt32(35));
		
		widget->SetAAXPRewards(result.GetInt32(36));

		widget->SetLootTier(result.GetInt32(37));

		widget->SetLootDropType(result.GetInt32(38));
		
		zone->AddWidget(id, widget);

		LogWrite(WIDGET__DEBUG, 0, "Widget", "Loaded Widget: '%s' (%u).", widget->appearance.name, spawn_id);
		return true;
	}

	LogWrite(WIDGET__DEBUG, 0, "Widget", "Unable to find a widget for spawn id of %u", spawn_id);
	return false;
}

bool WorldDatabase::LoadObject(ZoneServer* zone, int32 spawn_id) {
	Object* object = 0;
	int32 id = 0;
	DatabaseResult result;

	database_new.Select(&result, "SELECT so.spawn_id, s.name, s.race, s.model_type, s.command_primary, s.command_secondary, s.targetable, s.size, s.show_name, s.visual_state, s.attackable, s.show_level, s.show_command_icon, s.display_hand_icon, s.faction_id, s.collision_radius, s.transport_id, s.size_offset, so.device_id, s.disable_sounds, s.merchant_min_level, s.merchant_max_level, s.aaxp_rewards, s.loot_tier, s.loot_drop_type\n"
								 "FROM spawn s\n"
								 "INNER JOIN spawn_objects so\n"
								 "ON so.spawn_id = s.id\n"
								 "WHERE s.id = %u",
								 spawn_id);

	if (result.GetNumRows() > 0 && result.Next()) {
		id = result.GetInt32(0);
		object = new Object();
		object->SetDatabaseID(id);
		strcpy(object->appearance.name, result.GetString(1));
		vector<EntityCommand*>* primary_command_list = zone->GetEntityCommandList(result.GetInt32(4));
		vector<EntityCommand*>* secondary_command_list = zone->GetEntityCommandList(result.GetInt32(5));
		if(primary_command_list){
			object->SetPrimaryCommands(primary_command_list);
			object->primary_command_list_id = result.GetInt32(4);
		}
		if(secondary_command_list){
			object->SetSecondaryCommands(secondary_command_list);
			object->secondary_command_list_id = result.GetInt32(5);
		}
		object->appearance.race = result.GetInt8(2);
		object->appearance.model_type = result.GetInt16(3);
		object->appearance.targetable = result.GetInt8(6);
		object->size = result.GetInt16(7);
		object->appearance.display_name = result.GetInt8(8);
		object->appearance.visual_state = result.GetInt16(9);
		object->appearance.attackable = result.GetInt8(10);
		object->appearance.show_level = result.GetInt8(11);
		object->appearance.show_command_icon = result.GetInt8(12);
		object->appearance.display_hand_icon = result.GetInt8(13);
		object->faction_id = result.GetInt32(14);
		object->appearance.pos.collision_radius = result.GetInt16(15);
		object->SetTransporterID(result.GetInt32(16));
		object->SetSizeOffset(result.GetInt8(17));
		object->SetDeviceID(result.GetInt8(18));
		object->SetSoundsDisabled(result.GetInt8(19));

		object->SetMerchantLevelRange(result.GetInt32(20), result.GetInt32(21));
		
		object->SetAAXPRewards(result.GetInt32(22));

		object->SetLootTier(result.GetInt32(23));

		object->SetLootDropType(result.GetInt32(24));
		
		zone->AddObject(id, object);

		LogWrite(OBJECT__DEBUG, 0, "Object", "Loaded Object: '%s' (%u).", object->appearance.name, spawn_id);
		return true;
	}

	LogWrite(OBJECT__DEBUG, 0, "Object", "Unable to find an object for spawn id of %u", spawn_id);
	return false;
}

bool WorldDatabase::LoadGroundSpawn(ZoneServer* zone, int32 spawn_id) {
	GroundSpawn* spawn = 0;
	int32 id = 0;
	DatabaseResult result;

	database_new.Select(&result, "SELECT sg.spawn_id, s.name, s.race, s.model_type, s.command_primary, s.command_secondary, s.targetable, s.size, s.show_name, s.visual_state, s.attackable, s.show_level, s.show_command_icon, s.display_hand_icon, s.faction_id, s.collision_radius, sg.number_harvests, sg.num_attempts_per_harvest, sg.groundspawn_id, sg.collection_skill, s.size_offset, s.disable_sounds, s.aaxp_rewards, s.loot_tier, s.loot_drop_type, sg.randomize_heading\n"
								 "FROM spawn s\n"
								 "INNER JOIN spawn_ground sg\n"
								 "ON sg.spawn_id = s.id\n"
								 "WHERE s.id = %u",
								 spawn_id);

	if (result.GetNumRows() > 0 && result.Next()) {
		id = result.GetInt32(0);
		spawn = new GroundSpawn();
		spawn->SetDatabaseID(id);
		strcpy(spawn->appearance.name, result.GetString(1));
		vector<EntityCommand*>* primary_command_list = zone->GetEntityCommandList(result.GetInt32(4));
		vector<EntityCommand*>* secondary_command_list = zone->GetEntityCommandList(result.GetInt32(5));
		if(primary_command_list){
			spawn->SetPrimaryCommands(primary_command_list);
			spawn->primary_command_list_id = result.GetInt32(4);
		}
		if(secondary_command_list){
			spawn->SetSecondaryCommands(secondary_command_list);
			spawn->secondary_command_list_id = result.GetInt32(5);
		}
		spawn->appearance.race = result.GetInt8(2);
		spawn->appearance.model_type = result.GetInt16(3);
		spawn->appearance.targetable = result.GetInt8(6);
		spawn->size = result.GetInt16(7);
		spawn->appearance.display_name = result.GetInt8(8);
		spawn->appearance.visual_state = result.GetInt16(9);
		spawn->appearance.attackable = result.GetInt8(10);
		spawn->appearance.show_level = result.GetInt8(11);
		spawn->appearance.show_command_icon = result.GetInt8(12);
		spawn->appearance.display_hand_icon = result.GetInt8(13);
		spawn->faction_id = result.GetInt32(14);
		spawn->appearance.pos.collision_radius = result.GetInt16(15);
		spawn->SetNumberHarvests(result.GetInt8(16));
		spawn->SetAttemptsPerHarvest(result.GetInt8(17));
		spawn->SetGroundSpawnEntryID(result.GetInt32(18));
		spawn->SetCollectionSkill(result.GetString(19));
		spawn->SetSizeOffset(result.GetInt8(20));
		
		spawn->SetSoundsDisabled(result.GetInt8(21));
		spawn->SetAAXPRewards(result.GetInt32(22));

		spawn->SetLootTier(result.GetInt32(23));

		spawn->SetLootDropType(result.GetInt32(24));
		
		spawn->SetRandomizeHeading(result.GetInt32(25));
		
		zone->AddGroundSpawn(id, spawn);

		if (!zone->GetGroundSpawnEntries(spawn->GetGroundSpawnEntryID()))
			LoadGroundSpawnEntry(zone, spawn->GetGroundSpawnEntryID());

		LogWrite(GROUNDSPAWN__DEBUG, 0, "GSpawn", "Loaded Ground Spawn: '%s' (%u).", spawn->appearance.name, spawn_id);
		return true;
	}

	LogWrite(GROUNDSPAWN__DEBUG, 0, "GSpawn", "Unable to find a ground spawn for spawn id of %u", spawn_id);
	return false;
}

void WorldDatabase::LoadGroundSpawnItems(ZoneServer* zone, int32 entry_id) {
 	DatabaseResult result;

	database_new.Select(&result, "SELECT item_id, is_rare, grid_id\n"
								 "FROM groundspawn_items\n"
								 "WHERE groundspawn_id = %u",
								 entry_id);

	if (result.GetNumRows() > 0 && result.Next()) {
		zone->AddGroundSpawnItem(entry_id, result.GetInt32(0), result.GetInt8(1), result.GetInt32(2));
		LogWrite(GROUNDSPAWN__DEBUG, 5, "GSpawn", "---Loading GroundSpawn Items: ID: %u\n", entry_id);
		LogWrite(GROUNDSPAWN__DEBUG, 5, "GSpawn", "---item: %ul, rare: %i, grid: %ul", result.GetInt32(0), result.GetInt8(1), result.GetInt32(2));
 	}
}

void WorldDatabase::LoadGroundSpawnEntry(ZoneServer* zone, int32 entry_id) {
 	DatabaseResult result;

	database_new.Select(&result, "SELECT min_skill_level, min_adventure_level, bonus_table, harvest1, harvest3, harvest5, harvest_imbue, harvest_rare, harvest10, harvest_coin\n"
								 "FROM groundspawns\n"
								 "WHERE enabled = 1 AND groundspawn_id = %u",
								 entry_id);

	if (result.GetNumRows() > 0 && result.Next()) {
		// this is getting ridonkulous...
		LogWrite(GROUNDSPAWN__DEBUG, 5, "GSpawn", "---Loading GroundSpawn ID: %u\n" \
			"---min_skill_level: %i, min_adventure_level: %i, bonus_table: %i\n" \
			"---harvest1: %.2f, harvest3: %.2f, harvest5: %.2f\n" \
			"---harvest_imbue: %.2f, harvest_rare: %.2f, harvest10: %.2f\n" \
			"---harvest_coin: %u", entry_id, result.GetInt16(0), result.GetInt16(1), result.GetInt8(2), result.GetFloat(3), result.GetFloat(4), result.GetFloat(5), result.GetFloat(6), result.GetFloat(7), result.GetFloat(8), result.GetInt32(9));

		zone->AddGroundSpawnEntry(entry_id, result.GetInt16(0), result.GetInt16(1), result.GetInt8(2), result.GetFloat(3), result.GetFloat(4), result.GetFloat(5), result.GetFloat(6), result.GetFloat(7), result.GetFloat(8), result.GetInt32(9));
		LoadGroundSpawnItems(zone, entry_id);
 	}
}

bool WorldDatabase::LoadNPC(ZoneServer* zone, int32 spawn_id) {
	NPC* npc = nullptr;
	int32 id = 0;
	DatabaseResult result;
										
	database_new.Select(&result, "SELECT npc.spawn_id, s.name, npc.min_level, npc.max_level, npc.enc_level, s.race, s.model_type, npc.class_, npc.gender, s.command_primary, s.command_secondary, s.show_name, npc.min_group_size, npc.max_group_size, npc.hair_type_id, npc.facial_hair_type_id, npc.wing_type_id, npc.chest_type_id, npc.legs_type_id, npc.soga_hair_type_id, npc.soga_facial_hair_type_id, s.attackable, s.show_level, s.targetable, s.show_command_icon, s.display_hand_icon, s.hp, s.power, s.size, s.collision_radius, npc.action_state, s.visual_state, npc.mood_state, npc.initial_state, npc.activity_status, s.faction_id, s.sub_title, s.merchant_id, s.merchant_type, s.size_offset, npc.attack_type, npc.ai_strategy+0, npc.spell_list_id, npc.secondary_spell_list_id, npc.skill_list_id, npc.secondary_skill_list_id, npc.equipment_list_id, npc.str, npc.sta, npc.wis, npc.intel, npc.agi, npc.heat, npc.cold, npc.magic, npc.mental, npc.divine, npc.disease, npc.poison, npc.aggro_radius, npc.cast_percentage, npc.randomize, npc.soga_model_type, npc.heroic_flag, npc.alignment, npc.elemental, npc.arcane, npc.noxious, s.savagery, s.dissonance, npc.hide_hood, npc.emote_state, s.prefix, s.suffix, s.last_name, s.disable_sounds, s.merchant_min_level, s.merchant_max_level, s.aaxp_rewards, s.loot_tier, s.loot_drop_type, npc.scared_by_strong_players, npc.action_state_str\n"
								 "FROM spawn s\n"
								 "INNER JOIN spawn_npcs npc\n"
								 "ON npc.spawn_id = s.id\n"
								 "WHERE s.id = %u",
								 spawn_id);

	if (result.GetNumRows() > 0 && result.Next()) {
		id = result.GetInt32(0);
		npc = new NPC();
		npc->SetDatabaseID(id);
		strcpy(npc->appearance.name, result.GetString(1));
		vector<EntityCommand*>* primary_command_list = zone->GetEntityCommandList(result.GetInt32(9));
		vector<EntityCommand*>* secondary_command_list = zone->GetEntityCommandList(result.GetInt32(10));
		if(primary_command_list){
			npc->SetPrimaryCommands(primary_command_list);
			npc->primary_command_list_id = result.GetInt32(9);
		}
		if(secondary_command_list){
			npc->SetSecondaryCommands(secondary_command_list);
			npc->secondary_command_list_id = result.GetInt32(10);
		}
		npc->appearance.min_level = result.GetInt8(2);
		npc->appearance.max_level = result.GetInt8(3);
		npc->appearance.level =		result.GetInt8(2);
		npc->appearance.difficulty = result.GetInt8(4);
		npc->appearance.race = result.GetInt8(5);
		npc->appearance.model_type = result.GetInt16(6);
		npc->appearance.soga_model_type = result.GetInt16(62);
		npc->appearance.adventure_class = result.GetInt8(7);
		npc->appearance.gender = result.GetInt8(8);
		npc->appearance.display_name = result.GetInt8(11);
		npc->features.hair_type = result.GetInt16(14);
		npc->features.hair_face_type = result.GetInt16(15);
		npc->features.wing_type = result.GetInt16(16);
		npc->features.chest_type = result.GetInt16(17);
		npc->features.legs_type = result.GetInt16(18);
		npc->features.soga_hair_type = result.GetInt16(19);
		npc->features.soga_hair_face_type = result.GetInt16(20);
		npc->appearance.attackable = result.GetInt8(21);
		npc->appearance.show_level = result.GetInt8(22);
		npc->appearance.targetable = result.GetInt8(23);
		npc->appearance.show_command_icon = result.GetInt8(24);
		npc->appearance.display_hand_icon = result.GetInt8(25);
		npc->appearance.hide_hood = result.GetInt8(70);
		npc->appearance.randomize = result.GetInt32(61);
		npc->SetTotalHP(result.GetInt32(26));
		npc->SetTotalPower(result.GetInt32(27));
		npc->SetHP(npc->GetTotalHP());
		npc->SetPower(npc->GetTotalPower());
		if(npc->GetTotalHP() == 0){
			npc->SetTotalHP(15*npc->GetLevel() + 1);
			npc->SetHP(15*npc->GetLevel() + 1);
		}
		if(npc->GetTotalPower() == 0){
			npc->SetTotalPower(15*npc->GetLevel() + 1);
			npc->SetPower(15*npc->GetLevel() + 1);
		}
		npc->size = result.GetInt16(28);
		npc->appearance.pos.collision_radius = result.GetInt16(29);
		npc->appearance.action_state = result.GetInt16(30);
		npc->appearance.visual_state = result.GetInt16(31);
		npc->appearance.mood_state = result.GetInt16(32);
		npc->appearance.emote_state = result.GetInt16(71);
		npc->appearance.pos.state = result.GetInt16(33);
		npc->appearance.activity_status = result.GetInt16(34);
		npc->faction_id = result.GetInt32(35);
		if(!result.IsNull(36)){
			std::string sub_title = std::string(result.GetString(36));
			if(sub_title.find("Collector") != std::string::npos) {
				npc->SetCollector(true);
			}
			if(strlen(result.GetString(36)) < sizeof(npc->appearance.sub_title))
				strcpy(npc->appearance.sub_title, result.GetString(36));
			else
				strncpy(npc->appearance.sub_title, result.GetString(36), sizeof(npc->appearance.sub_title));
		}
		npc->SetMerchantID(result.GetInt32(37));
		npc->SetMerchantType(result.GetInt8(38));
		npc->SetSizeOffset(result.GetInt8(39));
		npc->SetAIStrategy(result.GetInt8(41));
		npc->SetPrimarySpellList(result.GetInt32(42));
		npc->SetSecondarySpellList(result.GetInt32(43));
		npc->SetPrimarySkillList(result.GetInt32(44));
		npc->SetSecondarySkillList(result.GetInt32(45));
		npc->SetEquipmentListID(result.GetInt32(46));

		InfoStruct* info = npc->GetInfoStruct();
		info->set_attack_type(result.GetInt8(40));
		info->set_str_base(result.GetInt16(47));
		info->set_sta_base(result.GetInt16(48));
		info->set_wis_base(result.GetInt16(49));		
		info->set_intel_base(result.GetInt16(50));
		info->set_agi_base(result.GetInt16(51));
		info->set_heat_base(result.GetInt16(52));
		info->set_cold_base(result.GetInt16(53));
		info->set_magic_base(result.GetInt16(54));
		info->set_mental_base(result.GetInt16(55));
		info->set_divine_base(result.GetInt16(56));
		info->set_disease_base(result.GetInt16(57));
		info->set_poison_base(result.GetInt16(58));
		info->set_alignment(result.GetSInt8(64));

		npc->SetAggroRadius(result.GetFloat(59));
		npc->SetCastPercentage(result.GetInt8(60));
		npc->appearance.heroic_flag = result.GetInt8(63);

		info->set_elemental_base(result.GetInt16(65));
		info->set_arcane_base(result.GetInt16(66));
		info->set_noxious_base(result.GetInt16(67));
		npc->SetTotalSavagery(result.GetInt32(68));
		npc->SetTotalDissonance(result.GetInt32(69));
		npc->SetSavagery(npc->GetTotalSavagery());
		npc->SetDissonance(npc->GetTotalDissonance());
		if(npc->GetTotalSavagery() == 0){
			npc->SetTotalSavagery(15*npc->GetLevel() + 1);
			npc->SetSavagery(15*npc->GetLevel() + 1);
		}
		if(npc->GetTotalDissonance() == 0){
			npc->SetTotalDissonance(15*npc->GetLevel() + 1);
			npc->SetDissonance(15*npc->GetLevel() + 1);
		}
		npc->SetPrefixTitle(result.GetString(72));
		npc->SetSuffixTitle(result.GetString(73));
		npc->SetLastName(result.GetString(74));

		npc->SetSoundsDisabled(result.GetInt8(75));

		npc->SetMerchantLevelRange(result.GetInt32(76), result.GetInt32(77));
		
		npc->SetAAXPRewards(result.GetInt32(78));

		npc->SetLootTier(result.GetInt32(79));

		npc->SetLootDropType(result.GetInt32(80));
		
		npc->SetScaredByStrongPlayers(result.GetInt32(81));
		
		if(!result.IsNull(82)){
			std::string action_state_str = std::string(result.GetString(82));
			npc->GetInfoStruct()->set_action_state(action_state_str);
		}
		
		zone->AddNPC(id, npc);

		//skipped spells/skills/equipment as it is all loaded, the following rely on a spawn to load
		LoadAppearance(zone, spawn_id);
		LoadNPCAppearanceEquipmentData(zone, spawn_id);
		
		LogWrite(NPC__DEBUG, 0, "NPC", "Loaded NPC: '%s' (%u).", npc->appearance.name, spawn_id);
		return true;
	}

	LogWrite(NPC__DEBUG, 0, "NPC", "Unable to find a npc for spawn id of %u", spawn_id);
	return false;
}

void WorldDatabase::LoadAppearance(ZoneServer* zone, int32 spawn_id) {
	Entity* entity = zone->GetNPC(spawn_id);
	if (!entity)
		return;

	DatabaseResult result, result2;
	map<string, int8> appearance_types;
	map<int32, map<int8, EQ2_Color> > appearance_colors;
	EQ2_Color color;
	color.red = 0;
	color.green = 0;
	color.blue = 0;
	string type;

	database_new.Select(&result2, "SELECT distinct `type`\n"
								 "FROM npc_appearance\n"
								 "WHERE length(`type`) > 0 AND `spawn_id` = %u",
								 spawn_id);
	
	while(result2.Next()) {
		type = string(result2.GetString(0));
		appearance_types[type] = GetAppearanceType(type);
		if(appearance_types[type] == 255)
			LogWrite(WORLD__ERROR, 0, "Appearance", "Unknown appearance type '%s' in LoadAppearances.", type.c_str());
	}

	database_new.Select(&result, "SELECT `type`, `signed_value`, `red`, `green`, `blue`\n"
								 "FROM npc_appearance\n"
								 "WHERE length(`type`) > 0 AND `spawn_id` = %u",
								 spawn_id);
	
	while(result.Next()) {
		if(appearance_types[result.GetString(0)] < APPEARANCE_SOGA_EBT){ 
			color.red = result.GetInt8(2);
			color.green = result.GetInt8(3);
			color.blue = result.GetInt8(4);
		}
		switch(appearance_types[result.GetString(0)]){
			case APPEARANCE_SOGA_HFHC:{
				entity->features.soga_hair_face_highlight_color = color;
				break;
			}
			case APPEARANCE_SOGA_HTHC:{
				entity->features.soga_hair_type_highlight_color = color;
				break;
			}
			case APPEARANCE_SOGA_HFC:{
				entity->features.soga_hair_face_color = color;
				break;
			}
			case APPEARANCE_SOGA_HTC:{
				entity->features.soga_hair_type_color = color;
				break;
			}
			case APPEARANCE_SOGA_HH:{
				entity->features.soga_hair_highlight_color = color;
				break;
			}
			case APPEARANCE_SOGA_HC1:{
				entity->features.soga_hair_color1 = color;
				break;
			}
			case APPEARANCE_SOGA_HC2:{
				entity->features.soga_hair_color2 = color;
				break;
			}
			case APPEARANCE_SOGA_SC:{
				entity->features.soga_skin_color = color;
				break;
			}
			case APPEARANCE_SOGA_EC:{
				entity->features.soga_eye_color = color;
				break;
			}
			case APPEARANCE_HTHC:{
				entity->features.hair_type_highlight_color = color;
				break;
			}
			case APPEARANCE_HFHC:{
				entity->features.hair_face_highlight_color = color;
				break;
			}
			case APPEARANCE_HTC:{
				entity->features.hair_type_color = color;
				break;
				}
			case APPEARANCE_HFC:{
				entity->features.hair_face_color = color;
				break;
				}
			case APPEARANCE_HH:{
				entity->features.hair_highlight_color = color;
				break;
			}
			case APPEARANCE_HC1:{
				entity->features.hair_color1 = color;
				break;
				}
			case APPEARANCE_HC2:{
				entity->features.hair_color2 = color;
				break;
				}
			case APPEARANCE_WC1:{
				entity->features.wing_color1 = color;
				break;
				}
			case APPEARANCE_WC2:{
				entity->features.wing_color2 = color;
				break;
				}
			case APPEARANCE_SC:{
				entity->features.skin_color = color;
				break;
			}
			case APPEARANCE_EC:{
				entity->features.eye_color = color;
				break;
			}
			case APPEARANCE_SOGA_EBT:{
				for(int i=0;i<3;i++)
					entity->features.soga_eye_brow_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_SOGA_CHEEKT:{
				for(int i=0;i<3;i++)
					entity->features.soga_cheek_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_SOGA_NT:{
				for(int i=0;i<3;i++)
					entity->features.soga_nose_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_SOGA_CHINT:{
				for(int i=0;i<3;i++)
					entity->features.soga_chin_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_SOGA_LT:{
				for(int i=0;i<3;i++)
					entity->features.soga_lip_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_SOGA_EART:{
				for(int i=0;i<3;i++)
					entity->features.soga_ear_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_SOGA_EYET:{
				for(int i=0;i<3;i++)
					entity->features.soga_eye_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_EBT:{
				for(int i=0;i<3;i++)
					entity->features.eye_brow_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_CHEEKT:{
				for(int i=0;i<3;i++)
					entity->features.cheek_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_NT:{
				for(int i=0;i<3;i++)
					entity->features.nose_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_CHINT:{
				for(int i=0;i<3;i++)
					entity->features.chin_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_EART:{
				for(int i=0;i<3;i++)
					entity->features.ear_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_EYET:{
				for(int i=0;i<3;i++)
					entity->features.eye_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_LT:{
				for(int i=0;i<3;i++)
					entity->features.lip_type[i] = result.GetInt8(2+i);
				break;
			}
			case APPEARANCE_SHIRT:{
				entity->features.shirt_color = color;
				break;
			}
			case APPEARANCE_UCC:{
				break;
			}
			case APPEARANCE_PANTS:{
				entity->features.pants_color = color;
				break;
			}
			case APPEARANCE_ULC:{
				break;
			}
			case APPEARANCE_U9:{
				break;
			}
			case APPEARANCE_BODY_SIZE:{
				entity->features.body_size = color.red;
				break;
			}
			case APPEARANCE_SOGA_WC1:{
				break;
			}
			case APPEARANCE_SOGA_WC2:{
				break;
			}
			case APPEARANCE_SOGA_SHIRT:{				
				break;
			}
			case APPEARANCE_SOGA_UCC:{
				break;
			}
			case APPEARANCE_SOGA_PANTS:{
				break;
			}
			case APPEARANCE_SOGA_ULC:{
				break;
			}
			case APPEARANCE_SOGA_U13:{
				break;
			}
			case APPEARANCE_BODY_AGE: {
				entity->features.body_age = color.red;
				break;
			}
			case APPEARANCE_MC:{
				entity->features.model_color = color;
				break;
			}
			case APPEARANCE_SMC:{
				entity->features.soga_model_color = color;
				break;
			}
			case APPEARANCE_SBS: {
				entity->features.soga_body_size = color.red;
				break;
			}
			case APPEARANCE_SBA: {
				entity->features.soga_body_age = color.red;
				break;
			}
		}
	}

	entity->info_changed = true;
}

void WorldDatabase::LoadNPCAppearanceEquipmentData(ZoneServer* zone, int32 spawn_id) {
	NPC* npc = zone->GetNPC(spawn_id);
	if(!npc) {
		LogWrite(NPC__ERROR, 0, "NPC", "Unable to get a valid npc (%u) in %s", spawn_id, __FUNCTION__);
		return;
	}

	DatabaseResult result;
	int8 slot = 0;

	if (!database_new.Select(&result, "SELECT slot_id, equip_type, red, green, blue, highlight_red, highlight_green, highlight_blue\n"
								 "FROM npc_appearance_equip\n"
								 "WHERE spawn_id = %u\n",
								 spawn_id))
	{
		
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}

	while (result.Next()) {
		slot = result.GetInt8(0);
		if(slot < NUM_SLOTS) {
			npc->SetEquipment(slot, result.GetInt16(1), result.GetInt8(2), result.GetInt8(3), result.GetInt8(4), result.GetInt8(5), result.GetInt8(6), result.GetInt8(7));
		}
	}
}

void WorldDatabase::SaveCharacterPicture(int32 characterID, int8 type, uchar* picture, int32 picture_size) {
	stringstream ss_hex;
	stringstream ss_query;
	ss_hex.flags(ios::hex);
	for (int32 i = 0; i < picture_size; i++)
		ss_hex << setfill('0') << setw(2) << (int32)picture[i];

	ss_query << "INSERT INTO `character_pictures` (`char_id`, `pic_type`, `picture`) VALUES (" << characterID << ", " << (int32)type << ", '" << ss_hex.str() << "') ON DUPLICATE KEY UPDATE `picture` = '" << ss_hex.str() << "'";
		
  	Query query;
	query.RunQuery2(ss_query.str(), Q_REPLACE);
	
	if (query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF)
			LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error: in SaveCharacterPicture! Error Message: ", query.GetError());
}

void WorldDatabase::LoadZoneFlightPaths(ZoneServer* zone) {
	DatabaseResult result;
	int32 total = 0;

	if (!database_new.Select(&result, "SELECT id, speed, flying, early_dismount FROM flight_paths WHERE zone_id = %u", zone->GetZoneID())) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}

	while (result.Next()) {
		FlightPathInfo* info = new FlightPathInfo;
		int32 id = result.GetInt32(0);
		info->speed = result.GetFloat(1);
		info->flying = result.GetInt8(2) == 1 ? true : false;
		info->dismount = result.GetInt8(3) == 1 ? true : false;

		zone->AddFlightPath(id, info);
		total++;
	}

	LogWrite(ZONE__DEBUG, 0, "Zone", "Loaded %u flight paths for %s", total, zone->GetZoneDescription());
	LoadZoneFlightPathLocations(zone);
}

void WorldDatabase::LoadZoneFlightPathLocations(ZoneServer* zone) {
	DatabaseResult result;
	int32 total = 0;

	if (!database_new.Select(&result, "SELECT loc.flight_path, loc.x, loc.y, loc.z FROM flight_paths_locations loc\n"
									  "INNER JOIN flight_paths path\n"
									  "ON loc.flight_path = path.id\n"
									  "WHERE path.zone_id = %u\n"
									  "ORDER BY loc.id",
									  zone->GetZoneID()))
	{
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}

	while (result.Next()) {
		FlightPathLocation* loc = new FlightPathLocation;
		int32 id = result.GetInt32(0);
		loc->X = result.GetFloat(1);
		loc->Y = result.GetFloat(2);
		loc->Z = result.GetFloat(3);

		zone->AddFlightPathLocation(id, loc);
		total++;
	}

	LogWrite(ZONE__DEBUG, 0, "Zone", "Loaded %u flight path locations for %s", total, zone->GetZoneDescription());
}

void WorldDatabase::SaveCharacterLUAHistory(Player* player, int32 event_id, int32 value, int32 value2) {
	Query query;
	query.AddQueryAsync(player->GetCharacterID(), this, Q_REPLACE, "REPLACE INTO character_lua_history(char_id, event_id, value, value2) VALUES(% u, % u, % u, % u)", player->GetCharacterID(), event_id, value, value2);

//	if (!database_new.Query("REPLACE INTO character_lua_history (char_id, event_id, value, value2) VALUES (%u, %u, %u, %u)", player->GetCharacterID(), event_id, value, value2))
//		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
}

void WorldDatabase::LoadCharacterLUAHistory(int32 char_id, Player* player) {
	DatabaseResult result;
	int32 total = 0;

	if (!database_new.Select(&result, "SELECT event_id, value, value2 FROM character_lua_history WHERE char_id = %u", char_id)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}

	while (result.Next()) {
		int32 id = result.GetInt32(0);
		LUAHistory* hd = new LUAHistory;
		hd->Value = result.GetInt32(1);
		hd->Value2 = result.GetInt32(2);
		hd->SaveNeeded = false;
		player->LoadLUAHistory(id, hd);
		total++;
	}

	LogWrite(PLAYER__DEBUG, 0, "Player", "Loaded %u LUA history for %s", total, player->GetName());
}

void WorldDatabase::FindSpell(Client* client, char* findString)
{
	
	char* find_escaped = getEscapeString(findString);
	
	DatabaseResult result;
		if (!database_new.Select(&result, "SELECT s.`id`, ts.spell_id, ts.index, `name`, `tier` "
			"FROM (spells s, spell_tiers st) "
			"LEFT JOIN spell_ts_ability_index ts "
			"ON s.`id` = ts.spell_id "
			"WHERE s.id = st.spell_id and s.name like '%%%s%%' AND s.is_active = 1 "
			"ORDER BY s.`id`, `tier` limit 50", find_escaped))
		{
			// error
		}
		else
		{
			client->Message(CHANNEL_COLOR_YELLOW, "SpellID (SpellTier): SpellName for %s", findString);
			while (result.Next())
			{
				int32 spell_id = result.GetInt32Str("id");
				string spell_name = result.GetStringStr("name");
				int8 tier = result.GetInt8Str("tier");
				client->Message(CHANNEL_COLOR_YELLOW, "%i (%i): %s", spell_id, tier, spell_name.c_str());
			}
			client->Message(CHANNEL_COLOR_YELLOW, "End Spell Results for %s", find_escaped);
		}
		
	safe_delete_array(find_escaped);
}

void WorldDatabase::LoadChestTraps() {
	chest_trap_list.Clear();
	int32 index = 0;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, applicable_zone_id, chest_min_difficulty, chest_max_difficulty, spell_id, spell_tier FROM chest_traps");
	if (result && mysql_num_rows(result) > 0) {
		Title* title = 0;
		while (result && (row = mysql_fetch_row(result))) {
			int32 dbid = atoul(row[0]);
			sint32 applicable_zone_id = atoi(row[1]);
			int32 mindifficulty = atoul(row[2]);
			int32 maxdifficulty = atoul(row[3]);
			int32 spellid = atoul(row[4]);
			int32 tier = atoul(row[5]);
			ChestTrap* trap = new ChestTrap(dbid,applicable_zone_id,mindifficulty,maxdifficulty,spellid,tier);
			chest_trap_list.AddChestTrap(trap);
		}
	}
}

bool WorldDatabase::CheckExpansionFlags(ZoneServer* zone, int32 spawnXpackFlag)
{
	if (spawnXpackFlag == 0)
		return true;

	int32 globalXpackFlag = rule_manager.GetGlobalRule(R_Expansion, GlobalExpansionFlag)->GetInt32();
	int32 zoneXpackFlag = zone->GetExpansionFlag();
	// zone expansion flag takes priority
	if (zoneXpackFlag > 0 && (spawnXpackFlag & zoneXpackFlag) == 0)
		return false;
	// zone expansion flag fails, then if global expansion flag set, we see if that bit operand doesn't match, skip mob then
	else if (zoneXpackFlag == 0 && globalXpackFlag > 0 && (spawnXpackFlag & globalXpackFlag) == 0)
		return false;

	return true;
}

bool WorldDatabase::CheckHolidayFlags(ZoneServer* zone, int32 spawnHolidayFlag)
{
	if (spawnHolidayFlag == 0)
		return true;

	int32 globalHolidayFlag = rule_manager.GetGlobalRule(R_Expansion, GlobalHolidayFlag)->GetInt32();
	int32 zoneHolidayFlag = zone->GetHolidayFlag();
	// zone holiday flag takes priority
	if (zoneHolidayFlag > 0 && (spawnHolidayFlag & zoneHolidayFlag) == 0)
		return false;
	// zone holiday flag fails, then if global expansion flag set, we see if that bit operand doesn't match, skip mob then
	else if (zoneHolidayFlag == 0 && globalHolidayFlag > 0 && (spawnHolidayFlag & globalHolidayFlag) == 0)
		return false;

	return true;
}

void WorldDatabase::GetHouseSpawnInstanceData(ZoneServer* zone, Spawn* spawn)
{
	if (!spawn)
		return;

	if (zone->house_object_database_lookup.count(spawn->GetModelType()) < 1)
		zone->house_object_database_lookup.Put(spawn->GetModelType(), spawn->GetDatabaseID());

	DatabaseResult result;

	database_new.Select(&result, "SELECT pickup_item_id, pickup_unique_item_id\n"
		" FROM spawn_instance_data\n"
		" WHERE spawn_id = %u and spawn_location_id = %u",
		spawn->GetDatabaseID(),spawn->GetSpawnLocationID());

	if (result.GetNumRows() > 0 && result.Next()) {
		spawn->SetPickupItemID(result.GetInt32(0));
		spawn->SetPickupUniqueItemID(result.GetInt32(1));

		if (spawn->GetZone() != nullptr && spawn->GetMap() != nullptr && spawn->GetMap()->IsMapLoaded())
		{
			auto loc = glm::vec3(spawn->GetX(),spawn->GetZ(),spawn->GetY());
			uint32 GridID = 0;
			float new_z = spawn->FindBestZ(loc, nullptr, &GridID);
			spawn->SetPos(&(spawn->appearance.pos.grid_id), GridID);
		}
	}
}

int32 WorldDatabase::FindHouseInstanceSpawn(Spawn* spawn)
{
	DatabaseResult result;

	database_new.Select(&result, "SELECT id\n"
		" FROM spawn\n"
		" WHERE model_type = %u and is_instanced_spawn=1 limit 1",
		spawn->GetModelType());

	if (result.GetNumRows() > 0 && result.Next()) {
		return result.GetInt32(0);
	}

	return 0;
}


void WorldDatabase::LoadStartingSkills(World* world)
{
	world->MStartingLists.writelock();

	int32 total = 0;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT race_id, class_id, skill_id, current_val, max_val, progress FROM starting_skills");

	if (result)
	{
		if (mysql_num_rows(result) > 0)
		{
			Skill* skill = 0;

			while (result && (row = mysql_fetch_row(result)))
			{

				StartingSkill skill;
				skill.header.race_id = atoul(row[0]);
				skill.header.class_id = atoul(row[1]);
				skill.skill_id = atoul(row[2]);
				skill.current_val = atoul(row[3]);
				skill.max_val = atoul(row[4]);

				if (!world->starting_skills.count(skill.header.race_id))
				{
					multimap<int8, StartingSkill>* skills = new multimap<int8, StartingSkill>();
					skills->insert(make_pair(skill.header.class_id, skill));
					world->starting_skills.insert(make_pair(skill.header.race_id, skills));
				}
				else
				{
					multimap<int8, multimap<int8, StartingSkill>*>::const_iterator skills = world->starting_skills.find(skill.header.race_id);
					skills->second->insert(make_pair(skill.header.class_id, skill));
				}
				total++;
			}
		}
	}
	LogWrite(WORLD__DEBUG, 3, "World", "--Loaded %u Starting Skill(s)", total);

	world->MStartingLists.releasewritelock();
}


void WorldDatabase::LoadVoiceOvers(World* world)
{
	int32 total = 0;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT type_id, id, indexed, mp3_string, text_string, emote_string, key1, key2, garbled, garble_link_id FROM voiceovers");

	if (result)
	{
		if (mysql_num_rows(result) > 0)
		{
			Skill* skill = 0;

			while (result && (row = mysql_fetch_row(result)))
			{

				VoiceOverStruct vos;
				vos.mp3_string = std::string(row[3]);
				vos.text_string = std::string(row[4]);
				vos.emote_string = std::string(row[5]);
				vos.key1 = atoul(row[6]);
				vos.key2 = atoul(row[7]);
				vos.is_garbled = atoul(row[8]);
				vos.garble_link_id = atoul(row[9]);
				int8 type = atoul(row[0]);
				int32 id = atoul(row[1]);
				int16 index = atoul(row[2]);
				world->AddVoiceOver(type, id, index, &vos);
				total++;
			}
		}
	}
	LogWrite(WORLD__DEBUG, 3, "World", "--Loaded %u Voiceover(s)", total);
}


void WorldDatabase::LoadStartingSpells(World* world)
{
	world->MStartingLists.writelock();

	int32 total = 0;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT race_id, class_id, spell_id, tier, knowledge_slot FROM starting_spells");

	if (result)
	{
		if (mysql_num_rows(result) > 0)
		{
			Skill* skill = 0;

			while (result && (row = mysql_fetch_row(result)))
			{

				StartingSpell spell;
				spell.header.race_id = atoul(row[0]);
				spell.header.class_id = atoul(row[1]);
				spell.spell_id = atoul(row[2]);
				spell.tier = atoul(row[3]);
				spell.knowledge_slot = atoul(row[4]);

				if (!world->starting_spells.count(spell.header.race_id))
				{
					multimap<int8, StartingSpell>* spells = new multimap<int8, StartingSpell>();
					spells->insert(make_pair(spell.header.class_id, spell));
					world->starting_spells.insert(make_pair(spell.header.race_id, spells));
				}
				else
				{
					multimap<int8, multimap<int8, StartingSpell>*>::iterator spells = world->starting_spells.find(spell.header.race_id);
					spells->second->insert(make_pair(spell.header.class_id, spell));
				}
				total++;
			}
		}
	}
	LogWrite(WORLD__DEBUG, 3, "World", "--Loaded %u Starting Spell(s)", total);

	world->MStartingLists.releasewritelock();
}


bool WorldDatabase::DeleteSpiritShard(int32 id){
	Query query;
	query.RunQuery2(Q_DELETE, "delete FROM character_spirit_shards where id=%u",id);
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(WORLD__ERROR, 0, "World", "Error in DeleteSpiritShard query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}
	return true;
}

int32 WorldDatabase::CreateSpiritShard(const char* name, int32 level, int8 race, int8 gender, int8 adventure_class, 
									  int16 model_type, int16 soga_model_type, int16 hair_type, int16 hair_face_type, int16 wing_type,
									  int16 chest_type, int16 legs_type, int16 soga_hair_type, int16 soga_hair_face_type, int8 hide_hood, 
									  int16 size, int16 collision_radius, int16 action_state, int16 visual_state, int16 mood_state, int16 emote_state, 
									  int16 pos_state, int16 activity_status, char* sub_title, char* prefix_title, char* suffix_title, char* lastname, 
									  float x, float y, float z, float heading, int32 gridid, int32 charid, int32 zoneid, int32 instanceid) 
{
	LogWrite(WORLD__INFO, 3, "World", "Saving Spirit Shard %s %u", name, charid);

	Query query;
	char* name_escaped = getEscapeString(name);
	
	if(!sub_title)
		sub_title = "";
	char* subtitle_escaped = getEscapeString(sub_title);
	char* prefix_escaped = getEscapeString(prefix_title);
	char* suffix_escaped = getEscapeString(suffix_title);
	char* lastname_escaped = getEscapeString(lastname);
	string insert = string("INSERT INTO character_spirit_shards (name, level, race, gender, adventure_class, model_type, soga_model_type, hair_type, hair_face_type, wing_type, chest_type, legs_type, soga_hair_type, soga_hair_face_type, hide_hood, size, collision_radius, action_state, visual_state, mood_state, emote_state, pos_state, activity_status, sub_title, prefix_title, suffix_title, lastname, x, y, z, heading, gridid, charid, zoneid, instanceid) VALUES ('%s', %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, '%s', '%s', '%s', '%s', %f, %f, %f, %f, %u, %u, %u, %u)");
	query.RunQuery2(Q_INSERT, insert.c_str(), name_escaped, level, race, gender, adventure_class, model_type, soga_model_type, 
																hair_type, hair_face_type, wing_type, chest_type, legs_type, soga_hair_type, 
																soga_hair_face_type, hide_hood, size, collision_radius, action_state, visual_state, 
																mood_state, emote_state, pos_state, activity_status, subtitle_escaped, prefix_escaped, suffix_escaped, 
																lastname_escaped, x, y, z, heading, gridid, charid, zoneid, instanceid);

	
	safe_delete_array(name_escaped);
	safe_delete_array(subtitle_escaped);
	safe_delete_array(prefix_escaped);
	safe_delete_array(suffix_escaped);
	safe_delete_array(lastname_escaped);

	return query.GetLastInsertedID();
}

void WorldDatabase::LoadCharacterSpellEffects(int32 char_id, Client* client, int8 db_spell_type) 
{
	SpellProcess* spellProcess = client->GetCurrentZone()->GetSpellProcess();
	Player* player = client->GetPlayer();

		if(!spellProcess)
			return;
	DatabaseResult result;

	multimap<LuaSpell*, Entity*> restoreSpells;
	// Use -1 on type and subtype to turn the enum into an int and make it a 0 index
	if (!database_new.Select(&result, "SELECT name, caster_char_id, target_char_id, target_type, spell_id, effect_slot, slot_pos, icon, icon_backdrop, conc_used, tier, total_time, expire_timestamp, lua_file, custom_spell, damage_remaining, effect_bitmask, num_triggers, had_triggers, cancel_after_triggers, crit, last_spellattack_hit, interrupted, resisted, has_damaged, custom_function, caster_level FROM character_spell_effects WHERE charid = %u and db_effect_type = %u", char_id, db_spell_type)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}
	InfoStruct* info = player->GetInfoStruct();
	while (result.Next()) {
//result.GetInt8Str
		char spell_name[60];
		strncpy(spell_name, result.GetStringStr("name"), 60);

		int32 caster_char_id = result.GetInt32Str("caster_char_id");
		int32 target_char_id = result.GetInt32Str("target_char_id");
		int8 target_type = result.GetInt8Str("target_type");
		int32 spell_id = result.GetInt32Str("spell_id");
		int32 effect_slot = result.GetInt32Str("effect_slot");
		int32 slot_pos = result.GetInt32Str("slot_pos");
		int16 icon = result.GetInt32Str("icon");
		int16 icon_backdrop = result.GetInt32Str("icon_backdrop");
		int8 conc_used = result.GetInt32Str("conc_used");
		int8 tier = result.GetInt32Str("tier");
		float total_time = result.GetFloatStr("total_time");
		int32 expire_timestamp = result.GetInt32Str("expire_timestamp");

		string lua_file (result.GetStringStr("lua_file"));

		int8 custom_spell = result.GetInt32Str("custom_spell");

		int32 damage_remaining = result.GetInt32Str("damage_remaining");
		int32 effect_bitmask = result.GetInt32Str("effect_bitmask");
		int16 num_triggers = result.GetInt32Str("num_triggers");
		int8 had_triggers = result.GetInt32Str("had_triggers");
		int8 cancel_after_triggers = result.GetInt32Str("cancel_after_triggers");
		int8 crit = result.GetInt32Str("crit");
		int8 last_spellattack_hit = result.GetInt32Str("last_spellattack_hit");
		int8 interrupted = result.GetInt32Str("interrupted");
		int8 resisted = result.GetInt32Str("resisted");
		int8 has_damaged = result.GetInt32Str("has_damaged");
		std::string custom_function = std::string(result.GetStringStr("custom_function"));
		int16 caster_level = result.GetInt16Str("caster_level");
		LuaSpell* lua_spell = 0;
		if(custom_spell)
		{
			if((lua_spell = lua_interface->GetSpell(lua_file.c_str())) == nullptr)
			{
				LogWrite(LUA__WARNING, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: GetSpell(%u, %u, '%s'), custom lua script not loaded, when attempting to load.", spell_id, tier, lua_file.c_str());
			}
		}

		Spell* spell = master_spell_list.GetSpell(spell_id, tier);
		
		if(!spell)
		{
			LogWrite(LUA__ERROR, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: GetSpell(%u, %u), spell could not be found!", spell_id, tier);
			spell = master_spell_list.GetSpell(spell_id, 0);
			if(spell)
				LogWrite(LUA__WARNING, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: GetSpell(%u, %u), identified tier 0 as replacement since the GetSpell failed!", spell_id, tier);
			else
				continue;
		}
		else if(!icon) {
			LogWrite(LUA__WARNING, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: GetSpell(%u, %u), no icon set, this must be a heroic opportunity or incorrectly configured spell!", spell_id, tier);
			continue;
		}
		
		bool isMaintained = false;
		bool isExistingLuaSpell = false;
		MaintainedEffects* effect = nullptr;
		Client* tmpCaster = nullptr;
		if(caster_char_id == player->GetCharacterID() && (target_char_id == 0xFFFFFFFF || target_char_id == player->GetCharacterID()) && (effect = player->GetMaintainedSpell(spell_id)) != nullptr)
		{
			safe_delete(lua_spell);
			lua_spell = effect->spell;
			if(lua_spell)
				spell = lua_spell->spell;
			isMaintained = true;
			isExistingLuaSpell = true;
		}
		else if ( caster_char_id != player->GetCharacterID() && (tmpCaster = zone_list.GetClientByCharID(caster_char_id)) != nullptr 
					 && tmpCaster->GetPlayer() && (effect = tmpCaster->GetPlayer()->GetMaintainedSpell(spell_id)) != nullptr)
		{
			if(effect->spell && effect->spell_id == spell_id)
			{
				safe_delete(lua_spell);
				if(tmpCaster->GetCurrentZone() == player->GetZone())
					spellProcess->AddLuaSpellTarget(effect->spell, client->GetPlayer()->GetID());
				lua_spell = effect->spell;
				spell = effect->spell->spell;
				isExistingLuaSpell = true;
				isMaintained = true;
				LogWrite(LUA__WARNING, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: GetSpell(%u, %u, '%s'), effect spell id %u maintained spell recovered from %s", spell_id, tier, spell_name, effect ? effect->spell_id : 0, (tmpCaster && tmpCaster->GetPlayer()) ? tmpCaster->GetPlayer()->GetName() : "?");
			}
			else
			{
				LogWrite(LUA__WARNING, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: GetSpell(%u, %u, '%s'), something went wrong loading another characters maintained spell. Effect has spell id %u", spell_id, tier, lua_file.c_str(), effect ? effect->spell_id : 0);
				safe_delete(lua_spell);
				continue;
			}
		}
		else if(custom_spell && lua_spell)
		{
			lua_spell->spell = new Spell(spell);

			lua_interface->AddCustomSpell(lua_spell);
		}
		else if(db_spell_type == DB_TYPE_MAINTAINEDEFFECTS)
		{
			safe_delete(lua_spell);
			if(!target_char_id)
				continue;
			
			lua_spell = lua_interface->GetSpell(spell->GetSpellData()->lua_script.c_str());
			if(lua_spell)
				lua_spell->spell = spell;
		}
		
		if(!lua_spell)
		{
			LogWrite(LUA__ERROR, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: GetSpell(%u, %u, '%s'), lua_spell FAILED, when attempting to load.", spell_id, tier, lua_file.c_str());
			continue;
		}
	
		SpellScriptTimer* timer = nullptr;
		if(!isExistingLuaSpell && expire_timestamp != 0xFFFFFFFF && custom_function.size() > 0)
		{
			timer = new SpellScriptTimer;

			timer->caster = 0;
			timer->deleteWhenDone = false;
			timer->target = 0;

			timer->time = expire_timestamp;
			timer->customFunction = string(custom_function); // TODO
			timer->spell = lua_spell;
			timer->caster = (caster_char_id == player->GetCharacterID()) ? player->GetID() : 0;
			
			if(target_char_id == 0xFFFFFFFF && player->HasPet())
				timer->target = player->GetPet()->GetID();
			else
				timer->target = (target_char_id == player->GetCharacterID()) ? player->GetID() : 0;

			if(!timer->target && target_char_id)
			{
				Client* tmpClient = zone_list.GetClientByCharID(target_char_id);
				if(tmpClient && tmpClient->GetPlayer() && tmpClient->GetPlayer()->GetZone() == player->GetZone())
					timer->target = tmpClient->GetPlayer()->GetID();
			}
		}
		
		if(!isExistingLuaSpell)
		{
			lua_spell->crit = crit;
			lua_spell->damage_remaining = damage_remaining;
			lua_spell->effect_bitmask = effect_bitmask;
			lua_spell->had_dmg_remaining = (damage_remaining>0) ? true : false;
			lua_spell->had_triggers = had_triggers;
			lua_spell->initial_caster_char_id = caster_char_id; 
			lua_spell->initial_target = (target_char_id == player->GetCharacterID()) ? player->GetID() : 0;
			lua_spell->initial_target_char_id = target_char_id;
			lua_spell->interrupted = interrupted;
			lua_spell->last_spellattack_hit = last_spellattack_hit;
			lua_spell->num_triggers = num_triggers;
			lua_spell->has_damaged = has_damaged;
			lua_spell->is_damage_spell = has_damaged;
			lua_spell->initial_caster_level = caster_level;
		}

		if(lua_spell->initial_target == 0 && target_char_id == 0xFFFFFFFF && player->HasPet())
		{
			lua_spell->initial_target = player->GetPet()->GetID();
			lua_spell->initial_target_char_id = target_char_id;
		}
		//lua_spell->num_calls  ??
		//if(target_char_id == player->GetCharacterID())
		//	lua_spell->targets.push_back(player->GetID());
		
		if(db_spell_type == DB_TYPE_SPELLEFFECTS)
		{
			if (caster_char_id != player->GetCharacterID() && lua_spell->spell->GetSpellData()->group_spell && lua_spell->spell->GetSpellData()->friendly_spell)
				{
					if(!isExistingLuaSpell)
						safe_delete(lua_spell);
					continue;
				}
			
			player->MSpellEffects.writelock();
			LogWrite(LUA__WARNING, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: %s lua_spell caster %s (%u), caster char id: %u.", lua_spell->spell->GetName(), lua_spell->caster ? lua_spell->caster->GetName() : "", lua_spell->caster ? lua_spell->caster->GetID() : 0, caster_char_id);

			if(lua_spell->caster && (rule_manager.GetGlobalRule(R_Spells,EnableCrossZoneTargetBuffs)->GetInt8() || (!rule_manager.GetGlobalRule(R_Spells,EnableCrossZoneTargetBuffs)->GetInt8() && lua_spell->caster->GetZone() == player->GetZone())))
				info->spell_effects[effect_slot].caster = lua_spell->caster;
			else if(caster_char_id != player->GetCharacterID())
			{
				Client* tmpCaster = zone_list.GetClientByCharID(caster_char_id);
				if(tmpCaster)
				{
					if((rule_manager.GetGlobalRule(R_Spells,EnableCrossZoneTargetBuffs)->GetInt8() || (!rule_manager.GetGlobalRule(R_Spells,EnableCrossZoneTargetBuffs)->GetInt8() && lua_spell->caster->GetZone() == player->GetZone())))
					{
						info->spell_effects[effect_slot].caster = tmpCaster->GetPlayer();
						LogWrite(LUA__WARNING, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: %s lua_spell caster %s (%u), caster char id: %u, found player %s.", lua_spell->spell->GetName(), lua_spell->caster ? lua_spell->caster->GetName() : "", lua_spell->caster ? lua_spell->caster->GetID() : 0, caster_char_id, tmpCaster->GetPlayer()->GetName());
					}
					else
					{
						LogWrite(LUA__WARNING, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: %s lua_spell caster %s (%u), caster char id: %u, found player %s, SKIPPED due to R_Spells, EnableCrossZoneTargetBuffs.", lua_spell->spell->GetName(), lua_spell->caster ? lua_spell->caster->GetName() : "", lua_spell->caster ? lua_spell->caster->GetID() : 0, caster_char_id, tmpCaster->GetPlayer()->GetName());
						if(!isExistingLuaSpell)
						{
							safe_delete(lua_spell);
						}
						else
						{
							lua_spell->MSpellTargets.writelock(__FUNCTION__, __LINE__);
							lua_spell->char_id_targets.insert(make_pair(player->GetCharacterID(),0));
							lua_spell->MSpellTargets.releasewritelock(__FUNCTION__, __LINE__);
						}
						player->MSpellEffects.releasewritelock();
						continue;
					}

				}
			}
			else if(caster_char_id == player->GetCharacterID())
				info->spell_effects[effect_slot].caster = player;
			else
			{					
				LogWrite(LUA__ERROR, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: %s lua_spell caster %s (%u), caster char id: %u, failed to find caster will delete: %u", lua_spell->spell->GetName(), lua_spell->caster ? lua_spell->caster->GetName() : "", lua_spell->caster ? lua_spell->caster->GetID() : 0, caster_char_id, isExistingLuaSpell);
				if(!isExistingLuaSpell)
					safe_delete(lua_spell);
				continue;
			}
			if(spell->GetSpellData()->duration_until_cancel)
				info->spell_effects[effect_slot].expire_timestamp = 0xFFFFFFFF;
			else
				info->spell_effects[effect_slot].expire_timestamp = Timer::GetCurrentTime2() + expire_timestamp;
			info->spell_effects[effect_slot].icon = icon;
			info->spell_effects[effect_slot].icon_backdrop = icon_backdrop;
			info->spell_effects[effect_slot].spell_id = spell_id;
			info->spell_effects[effect_slot].tier = tier;
			info->spell_effects[effect_slot].total_time = total_time;
			info->spell_effects[effect_slot].spell = lua_spell;

			lua_spell->MSpellTargets.writelock(__FUNCTION__, __LINE__);
			multimap<int32,int8>::iterator entries;
			while((entries = lua_spell->char_id_targets.find(player->GetCharacterID())) != lua_spell->char_id_targets.end())
			{
				lua_spell->char_id_targets.erase(entries);
			}
			lua_spell->MSpellTargets.releasewritelock(__FUNCTION__, __LINE__);

			lua_spell->slot_pos = slot_pos;
			if(!isExistingLuaSpell)
				lua_spell->caster = player; // TODO: get actual player
			
			lua_spell->zone = player->GetZone();
			
			player->MSpellEffects.releasewritelock();

			if(!isMaintained)
				spellProcess->ProcessSpell(lua_spell, true, "cast", timer);
				else
				{
					// track target id when caster isnt in zone somehow
				}
		}
		else if ( db_spell_type == DB_TYPE_MAINTAINEDEFFECTS )
		{
			player->MMaintainedSpells.writelock();

			DatabaseResult targets;
			// Use -1 on type and subtype to turn the enum into an int and make it a 0 index
			if (database_new.Select(&targets, "SELECT target_char_id, target_type, db_effect_type, spell_id from character_spell_effect_targets where caster_char_id = %u and effect_slot = %u and slot_pos = %u", char_id, effect_slot, slot_pos)) {
				while (targets.Next()) {
					int32 target_char = targets.GetInt32Str("target_char_id");
					int8 maintained_target_type = targets.GetInt32Str("target_type");
					int32 in_spell_id = targets.GetInt32Str("spell_id");
					if(spell_id != in_spell_id)
						continue;
					
					int32 idToAdd = 0;

					if(target_char == 0xFFFFFFFF)
					{
						if( player->HasPet() )
						{
							restoreSpells.insert(make_pair(lua_spell, player->GetPet()));
							// target added via restoreSpells
						}
					}
					else
					{
						Client* client2 = zone_list.GetClientByCharID(target_char);
						if(client2 && client2->GetPlayer() && client2->GetCurrentZone() == client->GetCurrentZone())
						{
							if(maintained_target_type > 0)
							{
								if(client != client2)
								{
									lua_spell->MSpellTargets.writelock(__FUNCTION__, __LINE__);
									
									if(client2->GetPlayer()->GetPet() && maintained_target_type == PET_TYPE_COMBAT)
									{
										restoreSpells.insert(make_pair(lua_spell, client2->GetPlayer()->GetPet()));
										// target added via restoreSpells
									}
									if(client2->GetPlayer()->GetCharmedPet() && maintained_target_type == PET_TYPE_CHARMED)
									{
										restoreSpells.insert(make_pair(lua_spell, client2->GetPlayer()->GetCharmedPet()));
										// target added via restoreSpells
									}
									
									lua_spell->MSpellTargets.releasewritelock(__FUNCTION__, __LINE__);
								}
							} // end of pet clause
							else if(client != client2) // maintained type must be 0, so client
								restoreSpells.insert(make_pair(lua_spell, client2->GetPlayer()));
							
							lua_spell->MSpellTargets.writelock(__FUNCTION__, __LINE__);
							multimap<int32,int8>::iterator entries;
							for(entries = lua_spell->char_id_targets.begin(); entries != lua_spell->char_id_targets.end(); entries++)
							{
								int32 ent_char_id = entries->first;
								int8 ent_target_type = entries->second;
								if(ent_char_id == target_char && ent_target_type == maintained_target_type)
									entries = lua_spell->char_id_targets.erase(entries);
							}
							lua_spell->MSpellTargets.releasewritelock(__FUNCTION__, __LINE__);
						}
						else
						{
							lua_spell->MSpellTargets.writelock(__FUNCTION__, __LINE__);
							lua_spell->char_id_targets.insert(make_pair(target_char,maintained_target_type));
							lua_spell->MSpellTargets.releasewritelock(__FUNCTION__, __LINE__);
						}
					}
				}
			}
			
			Client* tmpClient = 0;
			int32 targetID = 0;
			if(target_char_id == 0xFFFFFFFF && player->HasPet())
				targetID = player->GetPet()->GetID();
			else if(target_char_id == player->GetCharacterID())
			{
				targetID = player->GetID();
				tmpClient = player->GetClient();
			}
			else if((tmpClient = zone_list.GetClientByCharID(target_char_id)) != nullptr && tmpClient->GetPlayer())
				targetID = tmpClient->GetPlayer()->GetID();
			
			info->maintained_effects[effect_slot].conc_used = conc_used;
			strncpy(info->maintained_effects[effect_slot].name, spell_name, 60);
			info->maintained_effects[effect_slot].slot_pos = slot_pos;
			info->maintained_effects[effect_slot].target = targetID;
			info->maintained_effects[effect_slot].target_type = target_type;
			if(spell->GetSpellData()->duration_until_cancel)
				info->maintained_effects[effect_slot].expire_timestamp = 0xFFFFFFFF;
			else
				info->maintained_effects[effect_slot].expire_timestamp = Timer::GetCurrentTime2() + expire_timestamp;
			info->maintained_effects[effect_slot].icon = icon;
			info->maintained_effects[effect_slot].icon_backdrop = icon_backdrop;
			info->maintained_effects[effect_slot].spell_id = spell_id;
			info->maintained_effects[effect_slot].tier = tier;
			info->maintained_effects[effect_slot].total_time = total_time;
			info->maintained_effects[effect_slot].spell = lua_spell;
			if(!isExistingLuaSpell)
				lua_spell->caster = player;
			
			lua_spell->zone = player->GetZone();
			
			player->MMaintainedSpells.releasewritelock();
		
			LogWrite(LUA__WARNING, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: %s process spell caster %s (%u), caster char id: %u, target id %u (%s).", lua_spell->spell->GetName(), lua_spell->caster ? lua_spell->caster->GetName() : "", 
											lua_spell->caster ? lua_spell->caster->GetID() : 0, caster_char_id, targetID, tmpClient ? tmpClient->GetPlayer()->GetName() : "");
			if(tmpClient && lua_spell->initial_target_char_id == tmpClient->GetCharacterID())
			{
				lua_spell->initial_target = tmpClient->GetPlayer()->GetID();
				spellProcess->AddLuaSpellTarget(lua_spell, lua_spell->initial_target, false);
			}
			
			spellProcess->ProcessSpell(lua_spell, true, "cast", timer);
		}
		if(!isExistingLuaSpell && expire_timestamp != 0xFFFFFFFF && !isMaintained)
		{
			lua_spell->timer.SetTimer(expire_timestamp);
			lua_spell->timer.SetAtTrigger(lua_spell->spell->GetSpellData()->duration1 * 100);
			lua_spell->timer.Start();
		}

		if(target_char_id == player->GetCharacterID() && lua_spell->spell->GetSpellData()->det_type)
			player->AddDetrimentalSpell(lua_spell, expire_timestamp);
		
		if(!isExistingLuaSpell)
		{
			if(timer)
				spellProcess->AddSpellScriptTimer(timer);
			
			lua_spell->num_calls = 1;
			lua_spell->restored = true;
		}
		
		if(!lua_spell->resisted && (lua_spell->spell->GetSpellDuration() > 0 || lua_spell->spell->GetSpellData()->duration_until_cancel))
				spellProcess->AddActiveSpell(lua_spell);

		if ( db_spell_type == DB_TYPE_MAINTAINEDEFFECTS )
		{
			// restore concentration on zoning/reloading characters maintained effects
			spellProcess->AddConcentration(lua_spell);
			if (num_triggers > 0)
				ClientPacketFunctions::SendMaintainedExamineUpdate(client, slot_pos, num_triggers, 0);
			if (damage_remaining > 0)
				ClientPacketFunctions::SendMaintainedExamineUpdate(client, slot_pos, damage_remaining, 1);
		}
	}

	if(db_spell_type == DB_TYPE_SPELLEFFECTS)
	{
		// if the cross_zone_target_buff option is disabled then we check for all possible targets within the current zone
		// if cross_zone_target_buff is enabled, we only check to restore pets, the players get restored by their own spell effects (we don't directly track the pets, indirectly we do through the player casting and their targets)
		int8 cross_zone_target_buff = rule_manager.GetGlobalRule(R_Spells,EnableCrossZoneTargetBuffs)->GetInt8();

		DatabaseResult targets;
		if (
			(!cross_zone_target_buff && database_new.Select(&targets, "SELECT caster_char_id, target_type, spell_id from character_spell_effect_targets where target_char_id = %u", player->GetCharacterID())) ||
			(database_new.Select(&targets, "SELECT caster_char_id, target_type, spell_id from character_spell_effect_targets where target_char_id = %u and target_type > 0", player->GetCharacterID())))
		 {
			while (targets.Next()) {
				int32 caster_char_id = targets.GetInt32Str("caster_char_id");
				int8 prev_target_type = targets.GetInt32Str("target_type");
				int32 in_spell_id = targets.GetInt32Str("spell_id");
					Client* tmpCaster = nullptr;
					MaintainedEffects* effect = nullptr;
					if (caster_char_id != player->GetCharacterID() && (tmpCaster = zone_list.GetClientByCharID(caster_char_id)) != nullptr && (cross_zone_target_buff || 
								 tmpCaster->GetCurrentZone() == player->GetZone()) && tmpCaster->GetPlayer() && (effect = tmpCaster->GetPlayer()->GetMaintainedSpell(in_spell_id)) != nullptr)
					{
						if(prev_target_type > 0)
						{
							if(player->HasPet())
							{
								if(player->GetPet() && prev_target_type == PET_TYPE_COMBAT)
									restoreSpells.insert(make_pair(effect->spell, player->GetPet()));
								if(player->GetCharmedPet() && prev_target_type == PET_TYPE_CHARMED)
									restoreSpells.insert(make_pair(effect->spell, player->GetCharmedPet()));
							}
						}
						else if(!player->GetSpellEffect(effect->spell_id, tmpCaster->GetPlayer()))
						{
							if(effect->spell->initial_target_char_id == player->GetCharacterID())
								effect->spell->initial_target = player->GetID();
							restoreSpells.insert(make_pair(effect->spell, player));
						}
					}
				}
			}
	}
	
	multimap<LuaSpell*, Entity*>::const_iterator itr;

	for (itr = restoreSpells.begin(); itr != restoreSpells.end(); itr++)
	{
		LuaSpell* tmpSpell = itr->first;
		Entity* target = itr->second;
		if(!target)
		{
			target = client->GetPlayer()->GetPet();
			if(!target)
				continue;
		}

		Entity* caster = tmpSpell->caster;
		if(!caster)
			caster = client->GetPlayer();
		
		int32 group_id_target = target->group_id;
		if(target->IsPet() && target->GetOwner())
			group_id_target = target->GetOwner()->group_id;
			
		if(caster != target && caster->GetPet() != target && 
			tmpSpell->spell->GetSpellData()->group_spell && tmpSpell->spell->GetSpellData()->friendly_spell && (caster->group_id == 0 || group_id_target == 0 || caster->group_id != group_id_target))
		{
			LogWrite(LUA__WARNING, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: %s player no longer grouped with %s to reload bonuses for spell %s (target_groupid: %u, caster_groupid: %u).", target->GetName(), caster ? caster->GetName() : "?", tmpSpell->spell->GetName(), group_id_target, caster->group_id);
			continue;
		}

		LogWrite(LUA__WARNING, 0, "LUA", "WorldDatabase::LoadCharacterSpellEffects: %s using caster %s to reload bonuses for spell %s.", player->GetName(), caster ? caster->GetName() : "?", tmpSpell->spell->GetName());
		
		spellProcess->AddLuaSpellTarget(tmpSpell, target->GetID());

		target->AddSpellEffect(tmpSpell, tmpSpell->timer.GetRemainingTime() != 0 ? tmpSpell->timer.GetRemainingTime() : 0);
		vector<BonusValues*>* sb_list = caster->GetAllSpellBonuses(tmpSpell);
		for (int32 x = 0; x < sb_list->size(); x++) {
			BonusValues* bv = sb_list->at(x);
			target->AddSpellBonus(tmpSpell, bv->type, bv->value, bv->class_req, bv->race_req, bv->faction_req);
		}
		sb_list->clear();
		safe_delete(sb_list);
		// look for a skill bonus on the caster's spell
		if(caster->IsPlayer())
		{
			SkillBonus* sb = ((Player*)caster)->GetSkillBonus(tmpSpell->spell->GetSpellID());
			if (sb) {
				map<int32, SkillBonusValue*>::iterator itr_skills;
				for (itr_skills = sb->skills.begin(); itr_skills != sb->skills.end(); itr_skills++)
				target->AddSkillBonus(sb->spell_id, (*itr_skills).second->skill_id, (*itr_skills).second->value);
			}
		}

	}

}

//devn00b: We need to handle non-found factions so the subtraction/addition works on 1st kill. Need to find a better way to handle this, but for now..
bool WorldDatabase::VerifyFactionID(int32 char_id, int32 faction_id) {
	DatabaseResult result;
	database_new.Select(&result, "SELECT COUNT(id) as faction_exists from character_factions where faction_id=%u and char_id=%u", faction_id, char_id);

	if (result.Next() && result.GetInt32Str("faction_exists") == 0) 
		return false;
	
	return true;
}

//using int/32 for starting city, should be large enough to support future zones (LOL). TODO: Will probably need more support bolted on, to support PVP and such.
void WorldDatabase::UpdateStartingLanguage(int32 char_id, uint8 race_id, int32 starting_city)
{
	int8 rule = rule_manager.GetGlobalRule(R_World, StartingZoneLanguages)->GetInt8();
	//this should never need to be done but lets make sure we got all the stuff we need. char_id at min since used for both.
	if(!char_id || (rule == 0 && !race_id) || (rule == 1 && !starting_city))
		return;

	std::string query_string = "SELECT language_id FROM starting_languages ";
	//check the rule, and that they gave us a race, if so use default entries to match live.
	if(rule == 0 && race_id >= 0) {
		query_string.append("WHERE race=" + std::to_string(race_id));
	}
	//if we have a starting city supplied, and the rule is set to use it, deal with it
	else if(rule == 1) {
		query_string.append("WHERE starting_city=" + std::to_string(starting_city) + " or (starting_city=0 and race_id=" + std::to_string(race_id) + ")");
	}
	
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT,query_string.c_str());
	LogWrite(PLAYER__DEBUG, 0, "Player", "Adding Languages for starting_city: %i based on rule R_World:StartingZoneLanguages value %u", starting_city, rule);
	if (result)	{
		if (mysql_num_rows(result) > 0)	{
			while (result && (row = mysql_fetch_row(result))){
			//add custom languages to the character_languages db.
				Query query2;
				query2.RunQuery2(Q_INSERT, "INSERT IGNORE INTO character_languages (char_id, language_id) VALUES (%u,%u)",char_id, atoul(row[0]));
			}
		}
	}
}


//devn00b: load the items the server has into a character_ db for easy access. Should be done on char create.
void WorldDatabase::LoadClaimItems(int32 char_id)
{
	if (!char_id) {
		LogWrite(WORLD__DEBUG, 3, "World", "-- There was an error in LoadClaimItems (missing char_id)");
		return;
	}

	int16 total = 0;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, item_id, max_claim, one_per_char, veteran_reward_time from claim_items");

	if (result)
	{
		if (mysql_num_rows(result) > 0)
		{
			while (result && (row = mysql_fetch_row(result)))
			{
				int32 item_id			= atoul(row[1]);
				int16 max_claim			= atoul(row[2]);
				int16 curr_claim		= max_claim;
				int8  one_per_char		= atoul(row[3]);
				int64 vet_reward_time	= atoi64(row[4]);
				int32 acct_id = GetCharacterAccountID(char_id);

				if (one_per_char == 1) {
					max_claim	= 1;
					curr_claim	= 1;
				}
				Query query2;
				MYSQL_RES* res = query2.RunQuery2(Q_INSERT, "insert ignore into character_claim_items (char_id, item_id, max_claim, curr_claim, one_per_char, veteran_reward_time, account_id) values  (%i, %i, %i, %i, %i, %I64i, %i)", char_id, item_id, max_claim, curr_claim, one_per_char, vet_reward_time, acct_id);
				total++;
			}
		}
	}
	LogWrite(WORLD__DEBUG, 3, "World", "--Loaded %u Claim Item(s)", total);
}

int16 WorldDatabase::CountCharClaimItems(int32 char_id) {
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT count(*) from character_claim_items where char_id=%i and curr_claim >0", char_id);

	if (result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		return atoi(row[0]); // Return count
	}

	return 0;
}

vector<ClaimItems> WorldDatabase::LoadCharacterClaimItems(int32 char_id) {
	vector<ClaimItems> claim;
	
	//make sure we have a charID shouldn't need this but adding anyway.
	if (!char_id)
		return claim;

	int32 acct_id = GetCharacterAccountID(char_id);
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, max_claim, curr_claim, one_per_char, last_claim, veteran_reward_time from character_claim_items where char_id=%u and curr_claim > 0", char_id);
	int16 i = 0;

	if (result)
	{
		if (mysql_num_rows(result) > 0)
		{

			while (result && (row = mysql_fetch_row(result)))
			{
				ClaimItems c;
				int8 max_claim = atoi(row[1]);
				int8 curr_claim = atoi(row[2]);

				//if one per char is set set max/cur to 1.
				if (atoi(row[3]) == 1) {
					max_claim = 1;
					curr_claim = 1;
				}
				if (atoi(row[3]) == 0) {
				
					//handle getting lowest claim amt from table, to prevent new chars skewing.
					Query query2;
					MYSQL_ROW row2;
					MYSQL_RES* result2 = query2.RunQuery2(Q_SELECT, "SELECT min(curr_claim) AS min_claim FROM character_claim_items WHERE account_id=%i and item_id=%i", acct_id, atoi(row[0]));

					//our current claim is now the lowest.
					if (result2 && (row2 = mysql_fetch_row(result2)) && row2[0] != NULL) {
					int8 curr_claim = atoi(row2[0]);
					}

				}

				c.item_id = atoi(row[0]);
				c.max_claim = max_claim;
				c.curr_claim = curr_claim;
				c.one_per_char = atoi(row[3]);
				c.vet_reward_time = atoi(row[5]);
				claim.push_back(c);
				i++;
			}
			return claim;
		}
	}
	return claim;
}

void WorldDatabase::ClaimItem(int32 char_id, int32 item_id, Client* client) {

	if (!client || !item_id || !char_id) {
		return;
	}

	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT item_id, max_claim, curr_claim, one_per_char, last_claim, veteran_reward_time, one_per_char from character_claim_items where char_id=%u and item_id=%u", char_id, item_id);

	if (result)
	{
		if (mysql_num_rows(result) > 0)
		{
			while (result && (row = mysql_fetch_row(result)))
			{
				Item* item = master_item_list.GetItem(item_id);
				Player* player = client->GetPlayer();
				InfoStruct* info = player->GetInfoStruct();

				int32 item_id = atoi(row[0]);
				int8 max_claim = atoi(row[1]);
				int8 min_claim = 0;
				int8 curr_claim = atoi(row[2]);
				int32 vet_reward_req = atoi(row[5]);
				int8 one_per_char = atoi(row[6]);
				int32 acct_id = GetCharacterAccountID(char_id);
				char claim_msg[512] = { 0 };

				//no claims left.
				if (curr_claim == 0) {
					client->Message(CHANNEL_COLOR_RED, "You have nothing to claim.");
					return;
				}

				//On live, characters must wait 6 seconds between claims. So..Lots of stuff here.
				uint32 last_claim = info->get_last_claim_time(); //load our last claim
				time_t now = time(0); //get the current epoc time
				uint32 curr_time = now; // current time.
				uint32 total_time = curr_time - last_claim; //Time since last claim (Current time - last claim)

				if (total_time < 6) {
					uint32 ttw = 6 - total_time;
					char tmp[64] = { 0 };
					sprintf(tmp, "You must wait %u more seconds.", ttw);
					client->Message(CHANNEL_COLOR_RED, tmp);
					return;
				}
				//handle the 2 different messages found from live (11/2022)				
				if (item->generic_info.item_type == 18) {
					sprintf(claim_msg, "You have consumed the item\nYou have chosen to give this character a %s", item->name.c_str());
				}
				else {
					sprintf(claim_msg, "You have consumed the item");
				}
					

				//not one per char, so remove 1 from account.
				if (one_per_char == 0) {
					Query query3;
					MYSQL_ROW row3;
					MYSQL_RES* result3 = query3.RunQuery2(Q_SELECT, "SELECT min(curr_claim) AS min_claim FROM character_claim_items WHERE account_id=%i and item_id=%i", acct_id, item_id);
					
					if (result3 && (row3 = mysql_fetch_row(result3)) && row3[0] != NULL) {
						int16 min_claim = atoi(row3[0]);

					//remove 1 from claim on one per char
					int32 acct_id = GetCharacterAccountID(char_id);
					Query query2;
					query2.RunQuery2(Q_UPDATE, "UPDATE `character_claim_items` SET curr_claim=(SELECT min(curr_claim) AS min_claim FROM character_claim_items WHERE account_id=%i and item_id=%i) - 1 , `last_claim`=%u WHERE account_id=%i and item_id=%i AND curr_claim > 0", acct_id, item_id, curr_time, acct_id, item_id);
					//give the item to the player, and update last claim time.
					bool item_added = client->AddItem(item_id);
					if (item_added == 0) {
						return;
					}
					info->set_last_claim_time(curr_time);
					client->Message(CHANNEL_COLOR_YELLOW, claim_msg);
					//reload the window to show new count.
					client->ShowClaimWindow();
					return;
					}
				}
				
				Query query4;
				query4.RunQuery2(Q_UPDATE, "UPDATE `character_claim_items` SET `curr_claim`=0 , `last_claim`=%u WHERE char_id=%i and item_id=%i", curr_time, char_id, item_id);
				//give the item to the player, and update last claim time.
				//client->AddItem(item_id);
				bool item_added = client->AddItem(item_id);
				if (item_added == 0) {
					return;
				}
				info->set_last_claim_time(curr_time);
				client->Message(CHANNEL_COLOR_YELLOW, claim_msg);
				//reload the window to show new count.
				client->ShowClaimWindow();
				return;
			}
		}
	}
	return;
}

//returns account age in seconds
int32	WorldDatabase::GetAccountAge(int32 account_id) {
	if (!account_id)
		return 0;

	int32 acct_age = 0;
	int32 acct_created = 0;
	time_t now = time(0); //get the current epoc time
	uint32 curr_time = now; // current time.

	Query query;
	//order by created date and grab the oldest one.
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT UNIX_TIMESTAMP(created_date) FROM characters WHERE account_id=%i ORDER BY created_date LIMIT 1", account_id);
	if (result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row = mysql_fetch_row(result);
		acct_created = atoi(row[0]);
	}
	if (!curr_time || !acct_created)
		return 0;

	acct_age = curr_time - acct_created;
	return acct_age;
}

void WorldDatabase::SaveSignMark(int32 char_id, int32 sign_id, char* char_name, Client* client) {

	if (!database_new.Query("update spawn_signs set char_id=%u, char_name='%s' where widget_id=%u", char_id, char_name, sign_id)) {
		LogWrite(SIGN__DEBUG, 0, "Sign", "ERROR in WorldDatabase::SaveSignMark");
		return;
	}

}

string WorldDatabase::GetSignMark(int32 char_id, int32 sign_id, char* char_name) {
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT char_name from spawn_signs where widget_id=%u", sign_id);
	char* charname = 0;

	if (result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row = mysql_fetch_row(result);

		charname = new char[strlen(row[0]) + 1];
		memset(charname, 0, strlen(row[0]) + 1);
		strcpy(charname, row[0]);
	}

	return charname;
}

int32 WorldDatabase::GetMysqlExpCurve(int level) {
    Query query;
    MYSQL_ROW row;
	//this should never happen but just in case.
	if(!level){
		level = 95;
	}
    MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT exp_needed from exp_per_level where level=%u", level);
    if (result && mysql_num_rows(result) > 0) {
        MYSQL_ROW row = mysql_fetch_row(result);
        return atoi(row[0]);
    }
	//return 1 so we dont break shit later divide by 0 and all that.
    return 1;
}