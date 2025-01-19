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
#include "LuaFunctions.h"
#include "Spawn.h"
#include "WorldDatabase.h"
#include "LuaInterface.h"
#include "../common/ConfigReader.h"
#include "client.h"
#include "World.h"
#include "Commands/Commands.h"
#include "races.h"
#include "classes.h"
#include "Variables.h"
#include "SpellProcess.h"
#include "Rules/Rules.h"
#include "../common/Log.h"
#include <math.h>
#include "HeroicOp/HeroicOp.h"
#include "RaceTypes/RaceTypes.h"
#include "ClientPacketFunctions.h"
#include "Transmute.h"
#include "Titles.h"
#include <boost/algorithm/string/predicate.hpp>
#include <sstream> 
#include <boost/algorithm/string.hpp>

extern MasterFactionList master_faction_list;
extern WorldDatabase database;
extern LuaInterface* lua_interface;
extern ConfigReader configReader;
extern MasterQuestList master_quest_list;
extern MasterItemList master_item_list;
extern MasterSpellList master_spell_list;
extern World world;
extern Commands commands;
extern ZoneList zone_list;
extern Races races;
extern Classes classes;
extern Variables variables;
extern MasterSkillList master_skill_list;
extern MasterHeroicOPList master_ho_list;
extern MasterRaceTypeList race_types_list;
extern MasterLanguagesList master_languages_list;
extern MasterTitlesList master_titles_list;
extern RuleManager rule_manager;

vector<string> ParseString(string strVal, char delim) {
	stringstream ss(strVal);
	vector<string> ret;
	while (ss.good())
	{
		string substr;
		getline(ss, substr, delim);
		ret.push_back(substr);
	}
	return ret;
}

vector<unsigned int> ParseStringToInt32(string strVal, char delim) {
	stringstream ss(strVal);
	vector<unsigned int> ret;
	while (ss.good())
	{
		string substr;
		getline(ss, substr, delim);
		stringstream valss(substr);
		unsigned int val = 0;
		valss >> val;
		ret.push_back(val);
	}
	return ret;
}

map<string, signed  int> ParseStringMap(string strVal, char delim) {
	vector<string> pairs = ParseString(strVal, delim);
	vector<string>::iterator itr;
	map<string, signed int> ret;
	for (itr = pairs.begin(); itr != pairs.end(); itr++) {
		vector<string> keyvaluepair = ParseString(*itr, ':');
		if (keyvaluepair.size() == 2) {
			stringstream valss(keyvaluepair[1]);
			int32 val = 0;
			valss >> val;
			ret[keyvaluepair[0]] = val;
		}
	}	
	
	return ret;
}

map<unsigned int, unsigned short> ParseIntMap(string strVal, char delim) {
	vector<string> pairs = ParseString(strVal, delim);
	vector<string>::iterator itr;
	map<unsigned int, unsigned short> ret;
	for (itr = pairs.begin(); itr != pairs.end(); itr++) {
		vector<string> keyvaluepair = ParseString(*itr, ':');
		int32 key = 0;
		if (keyvaluepair.size() > 0) {
			stringstream keyss(keyvaluepair[0]);
			keyss >> key;
		}
		if (keyvaluepair.size() == 1) {
			ret[key] = 1;
		}
		else if (keyvaluepair.size() == 2) {
			stringstream valss(keyvaluepair[1]);
			unsigned short val = 0;
			valss >> val;
			ret[key] = val;
		}
	}
	return ret;
}

map<unsigned int, signed int> ParseSInt32Map(string strVal, char delim) {
	vector<string> pairs = ParseString(strVal, delim);
	vector<string>::iterator itr;
	map<unsigned int, signed int> ret;
	for (itr = pairs.begin(); itr != pairs.end(); itr++) {
		vector<string> keyvaluepair = ParseString(*itr, ':');
		int32 key = 0;
		if (keyvaluepair.size() > 0) {
			stringstream keyss(keyvaluepair[0]);
			keyss >> key;
		}
		if (keyvaluepair.size() == 1) {
			ret[key] = 1;
		}
		else if (keyvaluepair.size() == 2) {
			stringstream valss(keyvaluepair[1]);
			signed int val = 0;
			valss >> val;
			ret[key] = val;
		}
	}
	return ret;
}

int EQ2Emu_lua_PlayFlavor(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	string mp3_string = lua_interface->GetStringValue(state, 2);
	string text_string = lua_interface->GetStringValue(state, 3);
	string emote_string = lua_interface->GetStringValue(state, 4);
	int32 key1 = lua_interface->GetInt32Value(state, 5);
	int32 key2 = lua_interface->GetInt32Value(state, 6);
	Spawn* player = lua_interface->GetSpawn(state, 7);
	int8 language = lua_interface->GetInt8Value(state, 8);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		const char* mp3 = 0;
		const char* text = 0;
		const char* emote = 0;
		if (mp3_string.length() > 0)
			mp3 = mp3_string.c_str();
		if (text_string.length() > 0)
			text = text_string.c_str();
		if (emote_string.length() > 0)
			emote = emote_string.c_str();
		Client* client = 0;
		if (player && player->IsPlayer())
			client = ((Player*)player)->GetClient();
		if (client) {
			if (((Player*)player)->WasSentSpawn(spawn->GetID()))
				spawn->GetZone()->PlayFlavor(client, spawn, mp3, text, emote, key1, key2, language);
		}
		else
			spawn->GetZone()->PlayFlavor(spawn, mp3, text, emote, key1, key2, language);
	}
	return 0;
}

int EQ2Emu_lua_PlayFlavorID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	
	int8 type = lua_interface->GetInt8Value(state, 2);
	int32 id = lua_interface->GetInt32Value(state, 3);
	int16 index = lua_interface->GetInt16Value(state, 4);
	Spawn* player = lua_interface->GetSpawn(state, 5);
	int8 language = lua_interface->GetInt8Value(state, 6);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
			Client* client = 0;
			if (player && player->IsPlayer())
				client = ((Player*)player)->GetClient();
			if (client) {
				VoiceOverStruct non_garble, garble;
				bool garble_success = false;
				bool success = world.FindVoiceOver(type, id, index, &non_garble, &garble_success, &garble);
				client->SendPlayFlavor(spawn, language, &non_garble, &garble, success, garble_success);
			}
			else
				spawn->GetZone()->PlayFlavorID(spawn, type, id, index, language);	
	}
	return 0;
}

int EQ2Emu_lua_PlaySound(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	string sound_string = lua_interface->GetStringValue(state, 2);
	float x = lua_interface->GetFloatValue(state, 3);
	float y = lua_interface->GetFloatValue(state, 4);
	float z = lua_interface->GetFloatValue(state, 5);
	Spawn* player = lua_interface->GetSpawn(state, 6);
	lua_interface->ResetFunctionStack(state);
	if (spawn && sound_string.length() > 0) {
		Client* client = 0;
		if (player && player->IsPlayer())
			client = ((Player*)player)->GetClient();
		if (client)
			spawn->GetZone()->PlaySoundFile(client, sound_string.c_str(), x, y, z);
		else
			spawn->GetZone()->PlaySoundFile(0, sound_string.c_str(), x, y, z);

	}
	return 0;
}
int EQ2Emu_lua_SetRequiredQuest(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	int16 quest_step = lua_interface->GetInt16Value(state, 3);
	bool private_spawn = (lua_interface->GetInt8Value(state, 4) == 1);
	bool continued_access = (lua_interface->GetInt8Value(state, 5) == 1);
	int16 flag_override = lua_interface->GetInt16Value(state, 6);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SetRequiredQuest command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (quest_id > 0) {
		//Add this quest to the list of required quests for this spawn
		spawn->SetQuestsRequired(quest_id, quest_step);
		//If private spawn value set
		if (private_spawn) {
			//Set the spawn to be private when not granted access through this quest
			spawn->AddAllowAccessSpawn(spawn);
			spawn->SetPrivateQuestSpawn(true);
		}
		//This value allows access after a quest step, or the whole quest has been completed
		if (continued_access)
			spawn->SetQuestsRequiredContinuedAccess(true);
		//This value will override vis_flags in the vis packet
		if (flag_override > 0)
			spawn->SetQuestsRequiredOverride(flag_override);
	}
	return 0;
}

int EQ2Emu_lua_SpawnSetByDistance(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	float max_distance = lua_interface->GetFloatValue(state, 2);
	string variable = lua_interface->GetStringValue(state, 3);
	string value = lua_interface->GetStringValue(state, 4);
	lua_interface->ResetFunctionStack(state);
	if (max_distance > 0 && spawn && value.length() > 0 && variable.length() > 0 && spawn->GetZone())
		spawn->GetZone()->SpawnSetByDistance(spawn, max_distance, variable, value);
	return 0;
}
int	EQ2Emu_lua_PerformCameraShake(lua_State* state) {

	if (!lua_interface)
		return 0;
	Client* client = 0;
	Spawn* player = lua_interface->GetSpawn(state);
	float intensity = lua_interface->GetFloatValue(state, 2);
	int8 direction = lua_interface->GetInt8Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (!player) {
		lua_interface->LogError("LUA PerformCameraShake command error: spawn is not valid");
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("LUA PerformCameraShake command error: spawn is not a player");
		return 0;
	}

	if (player->GetZone())
		client = ((Player*)player)->GetClient();

	if (!client) {
		lua_interface->LogError("LUA PerformCameraShake command error: could not find client");
		return 0;
	}
	PacketStruct* packet = configReader.getStruct("WS_PerformCameraShakeMsg", client->GetVersion());
	if (packet) {
		/* Client Intensity Logic (does not restrict service side, but expect .01 - 1.0 range)
			v1 = *(float *)(a1 + 4);
				if ( v1 > 0.0 )
				 v2 = fminf(v1, 1.0);
			    else
				 v2 = 0.1;
		*/
		packet->setDataByName("intensity", intensity);
		if ( client->GetVersion() > 561 )
			packet->setDataByName("direction", direction);
		client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
	return 0;
}

int EQ2Emu_lua_KillSpawn(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* dead = lua_interface->GetSpawn(state);
	Spawn* killer = lua_interface->GetSpawn(state, 2);
	bool send_packet = (lua_interface->GetInt8Value(state, 3) == 1);
	lua_interface->ResetFunctionStack(state);
	if (dead && dead->Alive() && dead->GetZone())
		dead->GetZone()->KillSpawn(false, dead, killer, send_packet);
	return 0;
}

int EQ2Emu_lua_KillSpawnByDistance(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	float max_distance = lua_interface->GetFloatValue(state, 2);
	bool include_players = lua_interface->GetInt8Value(state, 3);
	bool send_packet = (lua_interface->GetInt8Value(state, 4) == 1);
	lua_interface->ResetFunctionStack(state);
	if (max_distance > 0 && spawn && spawn->GetZone())
		spawn->GetZone()->KillSpawnByDistance(spawn, max_distance, include_players, send_packet);
	return 0;
}

int EQ2Emu_lua_Despawn(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 delay = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->GetZone())
		spawn->GetZone()->Despawn(spawn, delay);
	return 0;
}

int EQ2Emu_lua_ChangeHandIcon(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 displayHandIcon = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->info_changed = true;
		spawn->SetShowHandIcon(displayHandIcon);
	}
	return 0;
}

//this function is used to force an update packet to be sent.  
//Useful if certain calculated things change after the player is sent the spawn packet, like quest flags or player has access to an object now
int EQ2Emu_lua_SetVisualFlag(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->vis_changed = true;
		spawn->GetZone()->AddChangedSpawn(spawn);
	}
	return 0;
}

//this function is used to force an update packet to be sent.  
//Useful if certain calculated things change after the player is sent the spawn packet, like quest flags or player has access to an object now
int EQ2Emu_lua_SetInfoFlag(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->info_changed = true;
		spawn->GetZone()->AddChangedSpawn(spawn);
	}
	return 0;
}

int EQ2Emu_lua_SendStateCommand(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 new_state = lua_interface->GetInt32Value(state, 2);
	Spawn* player = lua_interface->GetSpawn(state, 3);

	lua_interface->ResetFunctionStack(state);

	if (spawn) {
		if(player)
		{
			if(player->IsPlayer())
			{
				Client* client = ((Player*)player)->GetClient();
				if(client)
				{
					ClientPacketFunctions::SendStateCommand(client, client->GetPlayer()->GetIDWithPlayerSpawn(spawn), new_state);		
					lua_interface->SetBooleanValue(state, true);
					return 1;
				}
				else
					LogWrite(LUA__ERROR, 0, "LUA", "Spawn %s Error in SendStateCommand,attempted to pass player value in argument 3, but argument does not have active client.", spawn->GetName());
			}
			else
				LogWrite(LUA__ERROR, 0, "LUA", "Spawn %s Error in SendStateCommand,attempted to pass player value in argument 3, but argument is NOT a player.", spawn->GetName());
		}
		else
		{
			spawn->GetZone()->QueueStateCommandToClients(spawn->GetID(), new_state);
			lua_interface->SetBooleanValue(state, true);
			return 1;
		}
	}

	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_SpawnSet(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	string variable = lua_interface->GetStringValue(state, 2);
	string value = lua_interface->GetStringValue(state, 3);
	bool no_update = lua_interface->GetBooleanValue(state, 4); // send update is true by default in SetSpawnCommand, so allow user to specify 'true' to disable send update.
	bool temporary_flag = true;
	
	int8 num_args = (int8)lua_interface->GetNumberOfArgs(state);
	int8 index = 0;

	if(num_args >= 5)
	{
		temporary_flag = lua_interface->GetBooleanValue(state, 5); // this used to be false, but no one bothered to set it temporary, we don't need to update the DB
		
		index = lua_interface->GetInt8Value(state, 6);
	}
	
	lua_interface->ResetFunctionStack(state);

	int32 type = commands.GetSpawnSetType(variable);
	if (type != 0xFFFFFFFF && value.length() > 0 && spawn)
		commands.SetSpawnCommand(0, spawn, type, value.c_str(), !no_update, temporary_flag, nullptr, index);
	return 0;
}

int EQ2Emu_lua_GetSpawn(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 spawn_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn_id > 0) {
		Spawn* closest_spawn = spawn->GetZone()->GetClosestSpawn(spawn, spawn_id);
		if (closest_spawn) {
			lua_interface->SetSpawnValue(state, closest_spawn);
			return 1;
		}
	}
	return 0;
}

int EQ2Emu_lua_GetSpawnFromList(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	vector<Spawn*> spawns;
	int32 position = 0;
	
	if(lua_istable(state, 1)) {
        size_t len = lua_rawlen(state, 1);
        for(int i=1;i <= len; i++)
        {
            // get the entry to stack
			lua_rawgeti(state, 1, i);
			int Top = lua_gettop(state);
			
			if(lua_islightuserdata(state,Top)) {
				LUAUserData* data = (LUAUserData*)lua_touserdata(state, Top);
				if(data->IsSpawn()) {
					spawns.push_back(data->spawn);
				}
			}
            // remove entry from stack
            lua_pop(state,1);
        }
	}
	
	position = lua_interface->GetInt32Value(state, 2);
		
	lua_interface->ResetFunctionStack(state);
	
	Spawn* spawn = 0;
	if(position < spawns.size()) {
		spawn = spawns.at(position);
	}
	
	if(spawn) {
		lua_interface->SetSpawnValue(state, spawn);
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetSpawnListSize(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	vector<Spawn*> spawns;
	
	if(lua_istable(state, 1)) {
        size_t len = lua_rawlen(state, 1);
        for(int i=1;i <= len; i++)
        {
            // get the entry to stack
			lua_rawgeti(state, 1, i);
			int Top = lua_gettop(state);
			
			if(lua_islightuserdata(state,Top)) {
				LUAUserData* data = (LUAUserData*)lua_touserdata(state, Top);
				if(data->IsSpawn()) {
					spawns.push_back(data->spawn);
				}
			}
            // remove entry from stack
            lua_pop(state,1);
        }
	}
	
	lua_interface->ResetFunctionStack(state);
	
	lua_interface->SetInt32Value(state, spawns.size());
	return 1;
}

int EQ2Emu_lua_GetSpawnListBySpawnID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 spawn_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		vector<Spawn*> spawns = spawn->GetZone()->GetSpawnsByID(spawn_id);
		if (spawns.size() > 0) {
			lua_createtable(state, spawns.size(), 0);
			int newTable = lua_gettop(state);
			for (int32 i = 0; i < spawns.size(); i++) {
				lua_interface->SetSpawnValue(state, spawns.at(i));
				lua_rawseti(state, newTable, i + 1);
			}
			return 1;
		}
	}
	return 0;
}

int EQ2Emu_lua_GetSpawnListByRailID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	sint64 rail_id = lua_interface->GetSInt64Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		vector<Spawn*> spawns = spawn->GetZone()->GetSpawnsByRailID(rail_id);
		if (spawns.size() > 0) {
			lua_createtable(state, spawns.size(), 0);
			int newTable = lua_gettop(state);
			for (int32 i = 0; i < spawns.size(); i++) {
				lua_interface->SetSpawnValue(state, spawns.at(i));
				lua_rawseti(state, newTable, i + 1);
			}
			return 1;
		}
	}
	return 0;
}

int EQ2Emu_lua_GetPassengerSpawnList(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		vector<Spawn*> spawns = spawn->GetPassengersOnRail();
		if (spawns.size() > 0) {
			lua_createtable(state, spawns.size(), 0);
			int newTable = lua_gettop(state);
			for (int32 i = 0; i < spawns.size(); i++) {
				lua_interface->SetSpawnValue(state, spawns.at(i));
				lua_rawseti(state, newTable, i + 1);
			}
			return 1;
		}
	}
	return 0;
}

int EQ2Emu_lua_GetVariableValue(lua_State* state) {
	if (!lua_interface)
		return 0;
	string variable_name = lua_interface->GetStringValue(state);
	lua_interface->ResetFunctionStack(state);
	Variable* var = variables.FindVariable(variable_name);
	if (var) {
		lua_interface->SetStringValue(state, var->GetValue());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetCoinMessage(lua_State* state) {
	if (!lua_interface)
		return 0;
	int32 total_coins = lua_interface->GetInt32Value(state);
	lua_interface->ResetFunctionStack(state);
	if (total_coins == 0) {
		lua_interface->SetStringValue(state, "0 copper");
		return 1;
	}
	char tmp[64] = { 0 };
	string message = "";
	int32 val = 0;
	if (total_coins >= 1000000) {
		val = total_coins / 1000000;
		total_coins -= 1000000 * val;
		sprintf(tmp, " %u Platinum", val);
		message.append(tmp);
		memset(tmp, 0, 64);
	}
	if (total_coins >= 10000) {
		val = total_coins / 10000;
		total_coins -= 10000 * val;
		sprintf(tmp, " %u Gold", val);
		message.append(tmp);
		memset(tmp, 0, 64);
	}
	if (total_coins >= 100) {
		val = total_coins / 100;
		total_coins -= 100 * val;
		sprintf(tmp, " %u Silver", val);
		message.append(tmp);
		memset(tmp, 0, 64);
	}
	if (total_coins > 0) {
		sprintf(tmp, " %u Copper", (int32)total_coins);
		message.append(tmp);
	}
	lua_interface->SetStringValue(state, message.c_str());
	return 1;
}

int EQ2Emu_lua_GetSpawnByGroupID(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	int32 group_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (zone) {
		Spawn* spawn = zone->GetSpawnGroup(group_id);
		if (spawn) {
			lua_interface->SetSpawnValue(state, spawn);
			return 1;
		}
	}
	return 0;
}

int EQ2Emu_lua_GetSpawnByLocationID(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	int32 location_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (zone) {
		Spawn* spawn = zone->GetSpawnByLocationID(location_id);
		if (spawn) {
			lua_interface->SetSpawnValue(state, spawn);
			return 1;
		}
	}
	return 0;
}

int EQ2Emu_lua_GetID(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetID());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetSpawnID(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetDatabaseID());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetSpawnGroupID(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetSpawnGroupID());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetSpawnGroupID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 new_group_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->SetSpawnGroupID(new_group_id);
		lua_interface->SetBooleanValue(state, true);
		return 1;
	}

	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_AddSpawnToGroup(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 new_group_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		if(spawn->GetSpawnGroupID() == new_group_id) {
			lua_interface->SetBooleanValue(state, false);
			return 1;
		}
		spawn->GetZone()->AddSpawnToGroup(spawn, new_group_id);
		lua_interface->SetBooleanValue(state, true);
		return 1;
	}

	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_GetSpawnLocationID(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetSpawnLocationID());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetSpawnLocationPlacementID(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetSpawnLocationPlacementID());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetFactionAmount(lua_State* state) {
	Player* player = (Player*)lua_interface->GetSpawn(state);
	int32 faction_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer() && faction_id > 0) {
		lua_interface->SetSInt32Value(state, player->GetFactions()->GetFactionValue(faction_id));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetFactionID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 value = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->SetFactionID(value);
	}
	return 0;
}

int EQ2Emu_lua_GetFactionID(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetFactionID());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetGender(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetGender());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetTarget(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity() && ((Entity*)spawn)->GetTarget()) {
		lua_interface->SetSpawnValue(state, ((Entity*)spawn)->GetTarget());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_PlayVoice(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	string mp3_string = lua_interface->GetStringValue(state, 2);
	int32 key1 = lua_interface->GetInt32Value(state, 3);
	int32 key2 = lua_interface->GetInt32Value(state, 4);
	Spawn* player = lua_interface->GetSpawn(state, 5);
	lua_interface->ResetFunctionStack(state);
	if (spawn && mp3_string.length() > 0) {
		Client* client = 0;
		if (player && player->IsPlayer())
			client = ((Player*)player)->GetClient();
		if (client) {
			if (((Player*)player)->WasSentSpawn(spawn->GetID()))
				spawn->GetZone()->PlayVoice(client, spawn, mp3_string.c_str(), key1, key2);
		}
		else
			spawn->GetZone()->PlayVoice(spawn, mp3_string.c_str(), key1, key2);
	}
	return 0;
}

int EQ2Emu_lua_GetCurrentZoneSafeLocation(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetFloatValue(state, spawn->GetZone()->GetSafeX());
		lua_interface->SetFloatValue(state, spawn->GetZone()->GetSafeY());
		lua_interface->SetFloatValue(state, spawn->GetZone()->GetSafeZ());
		return 3;
	}
	return 0;
}

int EQ2Emu_lua_HasLootItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	if (spawn) {
		int32 item_id = lua_interface->GetInt32Value(state, 2);
		lua_interface->ResetFunctionStack(state);
		lua_interface->SetBooleanValue(state, spawn->HasLootItemID(item_id));
		return 1;
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_AddLootItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	if (spawn && spawn->IsEntity()) {
		int32 item_id = lua_interface->GetInt32Value(state, 2);
		int16 charges = lua_interface->GetInt16Value(state, 3);
		if (charges == 0)
			charges = 1;
		((Entity*)spawn)->AddLootItem(item_id, charges);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_RemoveLootItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	if (spawn && spawn->IsEntity()) {
		int32 item_id = lua_interface->GetInt32Value(state, 2);
		Item* item = spawn->LootItem(item_id);
		lua_interface->SetLuaUserDataStale(item);
		safe_delete(item);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_AddLootCoin(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	if (spawn) {
		int32 val = lua_interface->GetInt32Value(state, 2);
		spawn->AddLootCoins(val);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_GiveLoot(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* entity = lua_interface->GetSpawn(state);
	Spawn* player = lua_interface->GetSpawn(state, 2);
	if (entity && player && player->IsPlayer()) {
		int32 coins = lua_interface->GetInt32Value(state, 3);
		vector<Item*>* items = 0;
		int i = 0;
		int32 item_id = 0;
		while ((item_id = lua_interface->GetInt32Value(state, 4 + i))) {
			if (items == 0)
				items = new vector<Item*>;
			if (master_item_list.GetItem(item_id))
				items->push_back(master_item_list.GetItem(item_id));
			i++;
		}
		Client* client = 0;
		client = ((Player*)player)->GetClient();
		if (client) 
			((Player*)player)->AddPendingLootItems(entity->GetID(), items);
		if(coins > 0)
			entity->AddLootCoins(coins);
		safe_delete(items);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_HasPendingLootItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* entity = lua_interface->GetSpawn(state);
	Spawn* player = lua_interface->GetSpawn(state, 2);
	int32 item_id = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (entity && entity->IsEntity() && player && player->IsPlayer() && item_id > 0) {
		lua_interface->SetBooleanValue(state, ((Player*)player)->HasPendingLootItem(entity->GetID(), item_id));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_HasPendingLoot(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* entity = lua_interface->GetSpawn(state);
	Spawn* player = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (entity && player && player->IsPlayer()) {
		lua_interface->SetBooleanValue(state, ((Player*)player)->HasPendingLootItems(entity->GetID()));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_CreateConversation(lua_State* state) {
	if (!lua_interface)
		return 0;

	vector<ConversationOption>* conversation = lua_interface->GetConversation(state);
	lua_interface->SetLuaUserDataStale(conversation);
	safe_delete(conversation);
	lua_interface->ResetFunctionStack(state);

	conversation = new vector<ConversationOption>();
	lua_interface->SetConversationValue(state, conversation);
	return 1;
}

int EQ2Emu_lua_AddConversationOption(lua_State* state) {
	if (!lua_interface)
		return 0;
	vector<ConversationOption>* conversation = lua_interface->GetConversation(state);
	if (conversation) {
		ConversationOption conv_option;
		conv_option.option = lua_interface->GetStringValue(state, 2);
		conv_option.function = lua_interface->GetStringValue(state, 3);
		if (conv_option.option.length() > 0)
			conversation->push_back(conv_option);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_CloseConversation(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* npc = lua_interface->GetSpawn(state);
	Spawn* player = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (npc && player && player->IsPlayer() && player->GetZone()) {
		Client* client = ((Player*)player)->GetClient();
		if (client) {
			int32 conversation_id = client->GetConversationID(npc, 0);
			client->CloseDialog(conversation_id);
		}
	}
	return 0;
}

int EQ2Emu_lua_CloseItemConversation(lua_State* state) {
	if (!lua_interface)
		return 0;
	Item* item = lua_interface->GetItem(state);
	Spawn* player = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (item && player && player->IsPlayer() && player->GetZone()) {
		Client* client = ((Player*)player)->GetClient();
		if (client) {
			int32 conversation_id = client->GetConversationID(0, item);
			client->CloseDialog(conversation_id);
		}
	}
	return 0;
}

int EQ2Emu_lua_StartDialogConversation(lua_State* state) {
	if (!lua_interface)
		return 0;
	vector<ConversationOption>* conversation = lua_interface->GetConversation(state);
	Spawn* spawn = 0;
	Item* item = 0;
	int8 type = lua_interface->GetInt8Value(state, 2);
	if (type == 1 || type == 3)
		spawn = lua_interface->GetSpawn(state, 3);
	else if (type == 2 || type == 4)
		item = lua_interface->GetItem(state, 3);
	Spawn* player = lua_interface->GetSpawn(state, 4);
	string text = lua_interface->GetStringValue(state, 5);
	string mp3 = lua_interface->GetStringValue(state, 6);
	int32 key1 = lua_interface->GetInt32Value(state, 7);
	int32 key2 = lua_interface->GetInt32Value(state, 8);
	int8 language = lua_interface->GetInt8Value(state, 9);
	
	int numargs = lua_interface->GetNumberOfArgs(state);
	int8 can_close = 1;
	if(numargs > 9)
		can_close = lua_interface->GetInt32Value(state, 10);

	lua_interface->ResetFunctionStack(state);
	if (conversation && text.length() > 0 && (spawn || item) && player && player->IsPlayer()) {
		Client* client = ((Player*)player)->GetClient();
		if (client) {
			if (spawn) {
				// Need to do this so the function works the same as it did before
				if (type == 1)
					type++;

				if (mp3.length() > 0)
					client->DisplayConversation((Entity*)spawn, type, conversation, const_cast<char*>(text.c_str()), mp3.c_str(), key1, key2, language, can_close);
				else
					client->DisplayConversation((Entity*)spawn, type, conversation, const_cast<char*>(text.c_str()), nullptr, 0, 0, language, can_close);
			}
			else {
				if (mp3.length() > 0)
					client->DisplayConversation(item, conversation, const_cast<char*>(text.c_str()), type, mp3.c_str(), key1, key2, language, can_close);
				else
					client->DisplayConversation(item, conversation, const_cast<char*>(text.c_str()), type, nullptr, 0, 0, language, can_close);
			}
		}
	}
	lua_interface->SetLuaUserDataStale(conversation);
	safe_delete(conversation);
	return 0;
}

int EQ2Emu_lua_StartConversation(lua_State* state) {
	if (!lua_interface)
		return 0;
	vector<ConversationOption>* conversation = lua_interface->GetConversation(state);
	Spawn* source = lua_interface->GetSpawn(state, 2);
	Spawn* player = lua_interface->GetSpawn(state, 3);
	string text = lua_interface->GetStringValue(state, 4);
	string mp3 = lua_interface->GetStringValue(state, 5);
	int32 key1 = lua_interface->GetInt32Value(state, 6);
	int32 key2 = lua_interface->GetInt32Value(state, 7);
	int8 language = lua_interface->GetInt32Value(state, 8);
	
	int numargs = lua_interface->GetNumberOfArgs(state);
	int8 can_close = 1;
	if(numargs > 8)
		can_close = lua_interface->GetInt32Value(state, 9);
	
	lua_interface->ResetFunctionStack(state);
	if (conversation && conversation->size() > 0 && text.length() > 0 && source && player && player->IsPlayer()) {
		Client* client = ((Player*)player)->GetClient();
		if (mp3.length() > 0)
			client->DisplayConversation(source, 1, conversation, text.c_str(), mp3.c_str(), key1, key2, language, can_close);
		else
			client->DisplayConversation(source, 1, conversation, text.c_str(), nullptr, 0, 0, language, can_close);
		lua_interface->SetLuaUserDataStale(conversation);
		safe_delete(conversation);
	}
	else
		LogWrite(LUA__ERROR, 0, "LUA", "Spawn %s Error in StartConversation, potentially AddConversationOption not yet called or the StartConversation arguments are incorrect, text: %s, conversationSize: %i.", source ? source->GetName() : "UNKNOWN", text.size() ? text.c_str() : "", conversation ? conversation->size() : -1);
	return 0;
}

int EQ2Emu_lua_SetPlayerProximityFunction(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	float distance = lua_interface->GetFloatValue(state, 2);
	string in_range_function = lua_interface->GetStringValue(state, 3);
	string leaving_range_function = lua_interface->GetStringValue(state, 4);
	if (spawn && !spawn->IsPlayer() && distance > 0 && in_range_function.length() > 0)
		spawn->GetZone()->AddPlayerProximity(spawn, distance, in_range_function, leaving_range_function);
	return 0;
}

int EQ2Emu_lua_SetLocationProximityFunction(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	float x = lua_interface->GetFloatValue(state, 2);
	float y = lua_interface->GetFloatValue(state, 3);
	float z = lua_interface->GetFloatValue(state, 4);
	float max_variation = lua_interface->GetFloatValue(state, 5);
	string in_range_function = lua_interface->GetStringValue(state, 6);
	string leaving_range_function = lua_interface->GetStringValue(state, 7);
	lua_interface->ResetFunctionStack(state);
	if (zone && in_range_function.length() > 0)
		zone->AddLocationProximity(x, y, z, max_variation, in_range_function, leaving_range_function);
	return 0;
}

int EQ2Emu_lua_SetLootCoin(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	if (spawn && spawn->IsEntity()) {
		int32 val = lua_interface->GetInt32Value(state, 2);
		((Entity*)spawn)->SetLootCoins(val);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_GetLootCoin(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetInt32Value(state, ((Entity*)spawn)->GetLootCoins());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_MovementLoopAdd(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	float x = lua_interface->GetFloatValue(state, 2);
	float y = lua_interface->GetFloatValue(state, 3);
	float z = lua_interface->GetFloatValue(state, 4);
	float speed = lua_interface->GetFloatValue(state, 5);
	int32 delay = lua_interface->GetInt32Value(state, 6); //this should be given as seconds, as it is converted to ms later
	string function = lua_interface->GetStringValue(state, 7);
	
	int8 num_args = (int8)lua_interface->GetNumberOfArgs(state);
	float heading = lua_interface->GetFloatValue(state, 8);
	if (spawn) {
		spawn->AddMovementLocation(x, y, z, speed, delay, function.c_str(), heading, (num_args > 7) ? true : false );
		spawn->GetZone()->AddMovementNPC(spawn);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_IsPlayer(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetBooleanValue(state, spawn->IsPlayer());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetCharacterID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsPlayer()) {
		lua_interface->SetInt32Value(state, ((Player*)spawn)->GetCharacterID());
		return 1;
	}

	lua_interface->SetInt32Value(state, 0);
	return 1;
}

int EQ2Emu_lua_FaceTarget(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	
	int8 num_args = (int8)lua_interface->GetNumberOfArgs(state);
	bool reset_action_state = true;
	if(num_args > 2)
		reset_action_state = lua_interface->GetBooleanValue(state, 3);
	
	if (spawn && target) {
		if (spawn->IsEntity())
			// ((Entity*)spawn)->FaceTarget(target);
			static_cast<Entity*>(spawn)->FaceTarget(target, reset_action_state);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_MoveToLocation(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	float x = lua_interface->GetFloatValue(state, 2);
	float y = lua_interface->GetFloatValue(state, 3);
	float z = lua_interface->GetFloatValue(state, 4);
	float speed = lua_interface->GetFloatValue(state, 5);
	string lua_function = lua_interface->GetStringValue(state, 6);
	bool more_points = lua_interface->GetBooleanValue(state, 7);

	if (spawn) {
		if (speed == 0)
			speed = spawn->GetSpeed();

		spawn->AddRunningLocation(x, y, z, speed, 0.0f, true, !more_points, lua_function);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_ClearRunningLocations(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->ClearRunningLocations();
	}
	return 0;
}

int EQ2Emu_lua_Say(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	string message = lua_interface->GetStringValue(state, 2);
	Spawn* player = lua_interface->GetSpawn(state, 3);
	float dist = lua_interface->GetFloatValue(state, 4);
	int32 language = lua_interface->GetInt32Value(state, 5);
	if (spawn && message.length() > 0) {
		Client* client = 0;
		if (player && player->IsPlayer())
			client = ((Player*)player)->GetClient();
		if (client)
			spawn->GetZone()->HandleChatMessage(client, spawn, 0, CHANNEL_SAY, message.c_str(), (dist > 0.0f) ? dist : 30.0f, 0, true, language);
		else
			spawn->GetZone()->HandleChatMessage(spawn, 0, CHANNEL_SAY, message.c_str(), (dist > 0.0f) ? dist : 30.0f, 0, true, language);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_Shout(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	string message = lua_interface->GetStringValue(state, 2);
	Spawn* player = lua_interface->GetSpawn(state, 3);
	float dist = lua_interface->GetFloatValue(state, 4);
	int32 language = lua_interface->GetInt32Value(state, 5);
	if (spawn && message.length() > 0) {
		Client* client = 0;
		if (player && player->IsPlayer())
			client = ((Player*)player)->GetClient();
		if (client)
			spawn->GetZone()->HandleChatMessage(client, spawn, 0, CHANNEL_SHOUT, message.c_str(), (dist > 0.0f) ? dist : 30.0f, 0, true, language);
		else
			spawn->GetZone()->HandleChatMessage(spawn, 0, CHANNEL_SHOUT, message.c_str(), (dist > 0.0f) ? dist : 30.0f, 0, true, language);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_SayOOC(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	string message = lua_interface->GetStringValue(state, 2);
	Spawn* player = lua_interface->GetSpawn(state, 3);
	float dist = lua_interface->GetFloatValue(state, 4);
	int32 language = lua_interface->GetInt32Value(state, 5);
	if (spawn && message.length() > 0) {
		Client* client = 0;
		if (player && player->IsPlayer())
			client = ((Player*)player)->GetClient();
		if (client)
			spawn->GetZone()->HandleChatMessage(client, spawn, 0, CHANNEL_OUT_OF_CHARACTER, message.c_str(), (dist > 0.0f) ? dist : 30.0f, 0, true, language);
		else
			spawn->GetZone()->HandleChatMessage(spawn, 0, CHANNEL_OUT_OF_CHARACTER, message.c_str(), (dist > 0.0f) ? dist : 30.0f, 0, true, language);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_Emote(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	string message = lua_interface->GetStringValue(state, 2);
	Spawn* spawn2 = lua_interface->GetSpawn(state, 3);
	Spawn* player = lua_interface->GetSpawn(state, 4);
	char* to = 0;
	if (spawn2)
		to = spawn2->GetName();
	if (spawn && message.length() > 0) {
		Client* client = 0;
		if (player && player->IsPlayer())
			client = ((Player*)player)->GetClient();
		if (client)
			spawn->GetZone()->HandleChatMessage(client, spawn, to, CHANNEL_EMOTE, message.c_str(), 30);
		else
			spawn->GetZone()->HandleChatMessage(spawn, to, CHANNEL_EMOTE, message.c_str(), 30);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_SpellHeal(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	if(!luaspell || luaspell->resisted) {
		lua_interface->ResetFunctionStack(state);
		return 0;
	}
	Spawn* caster = luaspell->caster;
	string heal_type = lua_interface->GetStringValue(state);//power, heal ect
	int32 min_heal = lua_interface->GetInt32Value(state, 2);
	int32 max_heal = lua_interface->GetInt32Value(state, 3);
	Spawn* target = lua_interface->GetSpawn(state, 4);
	int8 crit_mod = lua_interface->GetInt32Value(state, 5);
	bool no_calcs = lua_interface->GetInt32Value(state, 6) == 1;
	string custom_spell_name = lua_interface->GetStringValue(state, 7);//custom spell name
	lua_interface->ResetFunctionStack(state);
	
	boost::to_lower(heal_type);
	if (caster && caster->IsEntity()) {
		bool success = false;
		luaspell->resisted = false;
		if (target) {
			float distance = caster->GetDistance(target, true);
			if (((Entity*)caster)->SpellHeal(target, distance, luaspell, heal_type, min_heal, max_heal, crit_mod, no_calcs, custom_spell_name))
				success = true;
		}
		if ((!success || luaspell->spell->GetSpellData()->group_spell) && luaspell->targets.size() > 0) {
			Spawn* target = 0;
			ZoneServer* zone = luaspell->caster->GetZone();
			luaspell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
			for (int32 i = 0; i < luaspell->targets.size(); i++) {
				if ((target = zone->GetSpawnByID(luaspell->targets[i]))) {
					float distance = caster->GetDistance(target, true);
					((Entity*)caster)->SpellHeal(target, distance, luaspell, heal_type, min_heal, max_heal, crit_mod, no_calcs, custom_spell_name);
				}
			}
			luaspell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
			success = true;
		}
		if (success) {
			if (caster->GetZone())
				caster->GetZone()->TriggerCharSheetTimer();
		}
	}
	return 0;
}

int EQ2Emu_lua_SpellHealPct(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	if(!luaspell || luaspell->resisted) {
		lua_interface->ResetFunctionStack(state);
		return 0;
	}
	Spawn* caster = luaspell->caster;
	string heal_type = lua_interface->GetStringValue(state);//power, heal ect
	float percentage = lua_interface->GetFloatValue(state, 2);
	bool current_value = lua_interface->GetBooleanValue(state, 3);
	bool caster_value = lua_interface->GetBooleanValue(state, 4);
	Spawn* target = lua_interface->GetSpawn(state, 5);
	int8 crit_mod = lua_interface->GetInt32Value(state, 6);
	bool no_calcs = lua_interface->GetInt32Value(state, 7) == 1;
	string custom_spell_name = lua_interface->GetStringValue(state, 8);//custom spell name
	lua_interface->ResetFunctionStack(state);

	boost::to_lower(heal_type);
	int32 min_heal = 0, max_heal = 0;
	if (caster && caster->IsEntity() && target) {
		if(percentage <= 0.0f)
		{
			LogWrite(LUA__ERROR, 0, "LUA", "Error applying SpellHealPct on '%s'.  percentage %f is less than or equal to 0.",target->GetName(),percentage);
			return 0;
		}

		if(heal_type == "power")
		{
			if(current_value)
			{
				if(caster_value)
					min_heal = max_heal = (int32)(float)caster->GetPower() * (percentage / 100.0f);
				else
					min_heal = max_heal = (int32)(float)target->GetPower() * (percentage / 100.0f);
			}
			else
			{
				if(caster_value)
					min_heal = max_heal = (int32)(float)caster->GetTotalPower() * (percentage / 100.0f);
				else
					min_heal = max_heal = (int32)(float)target->GetTotalPower() * (percentage / 100.0f);
			}

		}
		else
		{
			if(current_value)
			{
				if(caster_value)
					min_heal = max_heal = (int32)(float)caster->GetHP() * (percentage / 100.0f);
				else
					min_heal = max_heal = (int32)(float)target->GetHP() * (percentage / 100.0f);
			}
			else
			{
				if(caster_value)
					min_heal = max_heal = (int32)(float)caster->GetTotalHP() * (percentage / 100.0f);
				else
					min_heal = max_heal = (int32)(float)target->GetTotalHP() * (percentage / 100.0f);
			}
		}

		bool success = false;
		luaspell->resisted = false;
		if (target) {
			float distance = caster->GetDistance(target, true);
			if (((Entity*)caster)->SpellHeal(target, distance, luaspell, heal_type, min_heal, max_heal, crit_mod, no_calcs, custom_spell_name))
				success = true;
		}
		if ((!success || luaspell->spell->GetSpellData()->group_spell) && luaspell->targets.size() > 0) {
			Spawn* target = 0;
			ZoneServer* zone = luaspell->caster->GetZone();
			luaspell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
			for (int32 i = 0; i < luaspell->targets.size(); i++) {
				if ((target = zone->GetSpawnByID(luaspell->targets[i]))) {
					float distance = caster->GetDistance(target, true);
					((Entity*)caster)->SpellHeal(target, distance, luaspell, heal_type, min_heal, max_heal, crit_mod, no_calcs, custom_spell_name);
				}
			}
			luaspell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
			success = true;
		}
		if (success) {
			if (caster->GetZone())
				caster->GetZone()->TriggerCharSheetTimer();
		}
	}
	return 0;
}

int EQ2Emu_lua_AddItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 item_id = lua_interface->GetInt32Value(state, 2);
	int16 quantity = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);

	// default of 1 quantity to add
	if (quantity == 0)
		quantity = 1;

	if (spawn && spawn->IsPlayer()) {
		Client* client = ((Player*)spawn)->GetClient();
		if (client && item_id > 0) {
			lua_interface->SetBooleanValue(state, client->AddItem(item_id, quantity));
			return 1;
		}
	}

	lua_interface->SetBooleanValue(state, false);
	return 1;
}


int EQ2Emu_lua_SummonItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 item_id = lua_interface->GetInt32Value(state, 2);
	bool send_messages = (lua_interface->GetInt8Value(state, 3) == 1);
	string location = lua_interface->GetStringValue(state, 4);
	int16 item_count = lua_interface->GetInt16Value(state,5);
	
	//devn00b: if we dont have a count, assume 1 item.
	if(!item_count) {
		item_count = 1;
	}
	
	lua_interface->ResetFunctionStack(state);

	if (spawn && spawn->IsPlayer()) {
		Client* client = ((Player*)spawn)->GetClient();
		if (client && item_id > 0) {
			if (strncasecmp(location.c_str(), "bank", 4) == 0)
				lua_interface->SetBooleanValue(state, client->AddItemToBank(item_id, item_count));
			else
				lua_interface->SetBooleanValue(state, client->AddItem(item_id, item_count));
			if (send_messages) {
				Item* item = master_item_list.GetItem(item_id);
				if (item) {
					if(item_count > 1) {
						client->Message(CHANNEL_COLOR_YELLOW, "You receive %i %s.", item_count, item->CreateItemLink(client->GetVersion()).c_str());
						string popup_text1 = "You receive "+ item_count;
						string popup_text2 = "  " + item->name;
						string popup_text  = popup_text1 + popup_text2;
						client->SendPopupMessage(10, popup_text.c_str(), "ui_harvested_normal", 3, 0xFF, 0xFF, 0xFF);
						// return 1;
					} else {
					client->Message(CHANNEL_COLOR_YELLOW, "You receive %s.", item->CreateItemLink(client->GetVersion()).c_str());
					string popup_text = "You receive " + item->name;
					client->SendPopupMessage(10, popup_text.c_str(), "ui_harvested_normal", 3, 0xFF, 0xFF, 0xFF);
					}
				}
			}
			return 1;
		}
	}
	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_RemoveItem(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 item_id = lua_interface->GetInt32Value(state, 2);
	int16 quantity = lua_interface->GetInt16Value(state, 3);
	lua_interface->ResetFunctionStack(state);

	// default of 1 to remove
	if (quantity == 0)
		quantity = 1;

	Client* client;
	Item* item;

	if (spawn && spawn->IsPlayer() && item_id > 0) {
		if ((client = ((Player*)spawn)->GetClient())) {
			if ((item = client->GetPlayer()->item_list.GetItemFromID(item_id))) {
				if (client->RemoveItem(item, quantity)) {
					lua_interface->SetBooleanValue(state, true);
					return 1;
				}
			}
		}
	}

	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_HasItem(lua_State* state) {
	Spawn* player = lua_interface->GetSpawn(state);
	int32 item_id = lua_interface->GetInt32Value(state, 2);
	bool include_bank = lua_interface->GetInt8Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer()) {
		bool hasItem = false;
		hasItem = ((Player*)player)->item_list.HasItem(item_id, include_bank);
		if (!hasItem)
			hasItem = ((Player*)player)->GetEquipmentList()->HasItem(item_id);
		lua_interface->SetBooleanValue(state, hasItem);
		return 1;
	}
	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_Spawn(lua_State* state) {
	if (!lua_interface)
		return 0;
	ZoneServer* zone = lua_interface->GetZone(state);
	int32 spawn_id = lua_interface->GetInt32Value(state, 2);
	bool restricted_npc = (lua_interface->GetInt8Value(state, 3) == 1);
	float x = lua_interface->GetFloatValue(state, 4);
	float y = lua_interface->GetFloatValue(state, 5);
	float z = lua_interface->GetFloatValue(state, 6);
	float heading = lua_interface->GetFloatValue(state, 7);
	if (zone && spawn_id > 0 && (x != 0 || y != 0 || z != 0)) {
		Spawn* spawn = zone->GetSpawn(spawn_id);
		if (!spawn)
			lua_interface->LogError("%s: LUA Spawn command error: Could not find spawn with id of %u.", lua_interface->GetScriptName(state), spawn_id);
		else {
			spawn->SetX(x);
			spawn->SetZ(z);
			spawn->SetY(y,true,true);
			spawn->SetLocation(zone->GetClosestLocation(spawn));
			spawn->SetHeading(heading);
			if (restricted_npc)
				spawn->AddAllowAccessSpawn(spawn);

			const char* spawn_script = world.GetSpawnScript(spawn_id);
			bool scriptActive = false;
			if (spawn_script && lua_interface->GetSpawnScript(spawn_script) != 0) {
				scriptActive = true;
				spawn->SetSpawnScript(string(spawn_script));
			}

			zone->CallSpawnScript(spawn, SPAWN_SCRIPT_PRESPAWN);

			zone->AddSpawn(spawn);
			if (scriptActive) {
				zone->CallSpawnScript(spawn, SPAWN_SCRIPT_SPAWN);
			}
			lua_interface->ResetFunctionStack(state);
			lua_interface->SetSpawnValue(state, spawn);
			return 1;
		}
	}
	else {
		string output = "Invalid paramaters to LUA Spawn command: \n";
		if (!zone)
			output = output.append("\t").append("Missing zone reference. \n");
		if (spawn_id == 0)
			output = output.append("\t").append("Missing spawn_id.");
		lua_interface->LogError("%s: Error in EQ2Emu_lua_Spawn - %s", lua_interface->GetScriptName(state), output.c_str());
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_GetZoneName(lua_State* state) {
	if (!lua_interface)
		return 0;
	ZoneServer* zone = lua_interface->GetZone(state);
	lua_interface->ResetFunctionStack(state);
	if (zone) {
		lua_interface->SetStringValue(state, zone->GetZoneName());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetZoneID(lua_State* state) {
	if (!lua_interface)
		return 0;
	ZoneServer* zone = lua_interface->GetZone(state);
	lua_interface->ResetFunctionStack(state);
	if (zone) {
		lua_interface->SetInt32Value(state, zone->GetZoneID());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetZone(lua_State* state) {
	if (!lua_interface)
		return 0;
	int32 zone_id = lua_interface->GetInt32Value(state);
	ZoneChangeDetails zone_details;
	std::string zone_name;
	ZoneServer* zone = nullptr;
	
	if(zone_id < 1) {
		zone_name = lua_interface->GetStringValue(state);
	}
	bool zone_avail = zone_list.GetZone(&zone_details, zone_id, zone_name, true, false, false, false);
	if (zone_avail) {
		zone = (ZoneServer*)zone_details.zonePtr;
	}
	else {
		Spawn* spawn = lua_interface->GetSpawn(state);
		if (spawn)
			zone = spawn->GetZone();
	}
	lua_interface->ResetFunctionStack(state);
	if (zone) {
		lua_interface->SetZoneValue(state, zone);
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_AddHate(lua_State* state) {
	Spawn* entity = lua_interface->GetSpawn(state);
	Spawn* npc = lua_interface->GetSpawn(state, 2);
	sint32 amount = lua_interface->GetSInt32Value(state, 3);
	bool send_packet = lua_interface->GetInt8Value(state, 4) == 1 ? true : false;
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	
	if(luaspell && luaspell->resisted) {
		return 0;
	}
	
	if (entity && entity->IsEntity() && amount != 0) {
		if (luaspell && luaspell->caster) {
			ZoneServer* zone = luaspell->caster->GetZone();
			luaspell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
			for (int32 i = 0; i < luaspell->targets.size(); i++) {
				Spawn* spawn = zone->GetSpawnByID(luaspell->targets.at(i));
				if (spawn && spawn->IsNPC() && spawn->Alive() && spawn->GetZone()) {
					entity->CheckEncounterState((Entity*)spawn);
					((NPC*)spawn)->AddHate((Entity*)entity, amount);
					if (send_packet)
						entity->GetZone()->SendThreatPacket(entity, npc, amount, luaspell->spell->GetName());
				}
			}
			luaspell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
		}
		else if (npc && npc->IsNPC() && npc->GetZone()) {
			entity->CheckEncounterState((Entity*)npc);
			((NPC*)npc)->AddHate((Entity*)entity, amount);
		}
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}


int EQ2Emu_lua_Zone(lua_State* state) {
	if (!lua_interface)
		return 0;
	ZoneServer* zone = lua_interface->GetZone(state);
	Spawn* player = lua_interface->GetSpawn(state, 2);
	Client* client = 0;
	if (player && player->IsPlayer())
		client = ((Player*)player)->GetClient();
	float x = lua_interface->GetFloatValue(state, 3);
	float y = lua_interface->GetFloatValue(state, 4);
	float z = lua_interface->GetFloatValue(state, 5);
	float heading = lua_interface->GetFloatValue(state, 6);
	lua_interface->ResetFunctionStack(state);

	if (zone && client) {
		LogWrite(LUA__DEBUG, 0, "LUA", "LUA Zone Request by Player: '%s' (%u)", player->GetName(), player->GetID());
		LogWrite(LUA__DEBUG, 5, "LUA", "\tTo Zone: '%s' (%u)", zone->GetZoneName(), zone->GetZoneID());

		if (!client->CheckZoneAccess(zone->GetZoneName()))
		{
			LogWrite(LUA__WARNING, 0, "LUA", "CheckZoneAccess() FAILED! LUA Zone Request Denied!");
			return 0;
		}

		if (x != 0 || y != 0 || z != 0) {
			LogWrite(LUA__DEBUG, 5, "LUA", "\tTo Coordinates: %2f, %2f, %2f, %2f", x, y, z, heading);
			player->SetX(x);
			player->SetY(y);
			player->SetZ(z);
			player->SetHeading(heading);
			client->Zone(zone->GetZoneName(), false);
		}
		else
			client->Zone(zone->GetZoneName());
	}
	else
		lua_interface->LogError("%s: Error in EQ2Emu_lua_Zone: invalid zone or spawn input.", lua_interface->GetScriptName(state));
	return 0;
}

int EQ2Emu_lua_AddSpawnAccess(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* spawn2 = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);
	
	if (spawn && spawn2)
		spawn->AddAllowAccessSpawn(spawn2);
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_CastSpell(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* target = lua_interface->GetSpawn(state);
	int32 spell_id = lua_interface->GetInt32Value(state, 2);
	int8 spell_tier = lua_interface->GetInt8Value(state, 3);
	Spawn* caster = lua_interface->GetSpawn(state, 4);
	int16 custom_cast_time = lua_interface->GetInt16Value(state, 5);
	lua_interface->ResetFunctionStack(state);

	if (!target) {
		lua_interface->LogError("%s: LUA CastSpell command error: target is not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target->IsEntity()) {
		lua_interface->LogError("%s: LUA CastSpell command error: target (%s) is not an entity", lua_interface->GetScriptName(state), target->GetName());
		return 0;
	}

	if (spell_id <= 0) {
		lua_interface->LogError("%s: LUA CastSpell command error: spell id is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (caster && !caster->IsEntity()) {
		lua_interface->LogError("%s: LUA CastSpell command error: caster (%s) is not an entity", lua_interface->GetScriptName(state), caster->GetName());
		return 0;
	}

	if (spell_tier == 0)
		spell_tier = 1;

	if (!caster)
		caster = target;

	target->GetZone()->ProcessSpell(master_spell_list.GetSpell(spell_id, spell_tier), (Entity*)caster, (Entity*)target, true, false, NULL, custom_cast_time);
	return 0;
}

int EQ2Emu_lua_SpellDamage(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* target = lua_interface->GetSpawn(state);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	if(!luaspell || luaspell->resisted) {
		lua_interface->ResetFunctionStack(state);
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	Spawn* caster = luaspell->caster;
	sint32 type = lua_interface->GetSInt32Value(state, 2);
	int32 min_damage = lua_interface->GetInt32Value(state, 3);
	int32 max_damage = lua_interface->GetInt32Value(state, 4);
	int8 crit_mod = lua_interface->GetInt32Value(state, 5);
	bool no_calcs = lua_interface->GetInt32Value(state, 6) == 1;
	//lua_interface->ResetFunctionStack(state);
	int32 class_id = lua_interface->GetInt32Value(state, 7);
	vector<int16> faction_req;
	vector<int16> race_req;
	int32 class_req = 0;
	int32 i = 0;
	int8 f = 0;
	int8 r = 0;
	while ((class_id = lua_interface->GetInt32Value(state, 7 + i))) {
		if (class_id < 100) {
			class_req += pow(2.0, double(class_id - 1));
		}
		else if (class_id > 100 && class_id < 1000) {
			race_req.push_back(class_id);
			r++;
		}
		else {
			faction_req.push_back(class_id);
			f++;
		}
		i++;
	}
	lua_interface->ResetFunctionStack(state);
	if (caster && caster->IsEntity()) {
		bool race_match = false;
		bool success = false;
		luaspell->resisted = false;
		if (luaspell->targets.size() > 0) {
			ZoneServer* zone = luaspell->caster->GetZone();
			Spawn* target = 0;
			luaspell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
			for (int32 i = 0; i < luaspell->targets.size(); i++) {
				if ((target = zone->GetSpawnByID(luaspell->targets[i]))) {

					if (race_req.size() > 0) {
						for (int8 i = 0; i < race_req.size(); i++) {
						if(race_req[i] == target->GetRace() ||
							race_req[i] == race_types_list.GetRaceType(target->GetModelType()) ||
							race_req[i] == race_types_list.GetRaceBaseType(target->GetModelType())) {
								race_match = true;
							}
						}
					}
					else
						race_match = true; // if the race_req.size = 0 then there is no race requirement and the race_match will be true
					if (race_match == true) {
						float distance = caster->GetDistance(target, true);
						((Entity*)caster)->SpellAttack(target, distance, luaspell, type, min_damage, max_damage, crit_mod, no_calcs);
					}
				}
			}
			success = true;
			luaspell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
		}
		else if (target) {

			//check class and race/faction here
			if (race_req.size() > 0) {
				for (int8 i = 0; i < race_req.size(); i++) {
					if(race_req[i] == target->GetRace() ||
						race_req[i] == race_types_list.GetRaceType(target->GetModelType()) ||
						race_req[i] == race_types_list.GetRaceBaseType(target->GetModelType())) {
						race_match = true;
					}
				}
			}
			else
				race_match = true; // if the race_req.size = 0 then there is no race requirement and the race_match will be true
			if (race_match == true) {
				float distance = caster->GetDistance(target, true);
				if (((Entity*)caster)->SpellAttack(target, distance, luaspell, type, min_damage, max_damage, crit_mod, no_calcs))
					success = true;
			}
		}
		lua_interface->SetBooleanValue(state, luaspell->has_damaged);
		if (success) {
			Spell* spell = luaspell->spell;
			if (caster->IsPlayer() && spell && spell->GetSpellData()->target_type == 1 && spell->GetSpellData()->spell_book_type == 1) { //offense combat art
				((Player*)caster)->InCombat(true);
				if (caster->GetZone())
					caster->GetZone()->TriggerCharSheetTimer();
			}
		}
	}
	else {
		lua_interface->SetBooleanValue(state, false);
	}
	return 1;
}

int EQ2Emu_lua_SpellDamageExt(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* target = lua_interface->GetSpawn(state);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	if(!luaspell || luaspell->resisted) {
		lua_interface->ResetFunctionStack(state);
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	Spawn* caster = luaspell->caster;
	sint32 type = lua_interface->GetSInt32Value(state, 2);
	int32 min_damage = lua_interface->GetInt32Value(state, 3);
	int32 max_damage = lua_interface->GetInt32Value(state, 4);
	int8 crit_mod = lua_interface->GetInt32Value(state, 5);
	bool no_calcs = lua_interface->GetInt32Value(state, 6) == 1;
	int32 override_packet_type = lua_interface->GetInt32Value(state, 7);
	bool take_power = lua_interface->GetInt32Value(state, 8) == 1;
	//lua_interface->ResetFunctionStack(state);
	int32 class_id = lua_interface->GetInt32Value(state, 9);
	vector<int16> faction_req;
	vector<int16> race_req;
	int32 class_req = 0;
	int32 i = 0;
	int8 f = 0;
	int8 r = 0;
	while ((class_id = lua_interface->GetInt32Value(state, 9 + i))) {
		if (class_id < 100) {
			class_req += pow(2.0, double(class_id - 1));
		}
		else if (class_id > 100 && class_id < 1000) {
			race_req.push_back(class_id);
			r++;
		}
		else {
			faction_req.push_back(class_id);
			f++;
		}
		i++;
	}
	lua_interface->ResetFunctionStack(state);
	if (caster && caster->IsEntity()) {
		bool race_match = false;
		bool success = false;
		luaspell->resisted = false;
		if (luaspell->targets.size() > 0) {
			ZoneServer* zone = luaspell->caster->GetZone();
			Spawn* target = 0;
			luaspell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
			for (int32 i = 0; i < luaspell->targets.size(); i++) {
				if ((target = zone->GetSpawnByID(luaspell->targets[i]))) {

					if (race_req.size() > 0) {
						for (int8 i = 0; i < race_req.size(); i++) {
						if(race_req[i] == target->GetRace() ||
							race_req[i] == race_types_list.GetRaceType(target->GetModelType()) ||
							race_req[i] == race_types_list.GetRaceBaseType(target->GetModelType())) {
								race_match = true;
							}
						}
					}
					else
						race_match = true; // if the race_req.size = 0 then there is no race requirement and the race_match will be true
					if (race_match == true) {
						float distance = caster->GetDistance(target, true);
						((Entity*)caster)->SpellAttack(target, distance, luaspell, type, min_damage, max_damage, crit_mod, no_calcs, override_packet_type, take_power);
					}
				}
			}
			success = true;
			luaspell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
		}
		else if (target) {

			//check class and race/faction here
			if (race_req.size() > 0) {
				for (int8 i = 0; i < race_req.size(); i++) {
					if(race_req[i] == target->GetRace() ||
						race_req[i] == race_types_list.GetRaceType(target->GetModelType()) ||
						race_req[i] == race_types_list.GetRaceBaseType(target->GetModelType())) {
						race_match = true;
					}
				}
			}
			else
				race_match = true; // if the race_req.size = 0 then there is no race requirement and the race_match will be true
			if (race_match == true) {
				float distance = caster->GetDistance(target, true);
				if (((Entity*)caster)->SpellAttack(target, distance, luaspell, type, min_damage, max_damage, crit_mod, no_calcs, override_packet_type, take_power))
					success = true;
			}
		}
		lua_interface->SetBooleanValue(state, luaspell->has_damaged);
		if (success) {
			Spell* spell = luaspell->spell;
			if (caster->IsPlayer() && spell && spell->GetSpellData()->target_type == 1 && spell->GetSpellData()->spell_book_type == 1) { //offense combat art
				((Player*)caster)->InCombat(true);
				if (caster->GetZone())
					caster->GetZone()->TriggerCharSheetTimer();
			}
		}
	}
	else {
		lua_interface->SetBooleanValue(state, false);
	}
	return 1;
}
int EQ2Emu_lua_ModifyPower(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	sint32 value = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && value != 0) {
		if (spawn->GetPower() + value > spawn->GetTotalPower())
			spawn->SetPower(spawn->GetTotalPower());
		else
			spawn->SetPower(spawn->GetPower() + value);
	}
	return 0;
}
int EQ2Emu_lua_ModifyHP(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	sint32 value = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && value != 0) {
		if (spawn->GetHP() + value > spawn->GetTotalHP())
			spawn->SetHP(spawn->GetTotalHP());
		else
			spawn->SetHP(spawn->GetHP() + value);
	}
	return 0;
}
int EQ2Emu_lua_ModifyMaxPower(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	sint32 value = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && value != 0) {
		spawn->SetTotalPower(value);
		spawn->SetTotalPowerBaseInstance(value);
	}
	return 0;
}
int EQ2Emu_lua_ModifyMaxHP(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	sint32 value = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && value != 0) {
		spawn->SetTotalHP(value);
		spawn->SetTotalHPBaseInstance(value);	
	}
	return 0;
}
int EQ2Emu_lua_SetCurrentHP(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	sint32 value = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->SetHP(value);
		if (value > spawn->GetTotalHPBase())
			spawn->SetTotalHP(value);
	}
	return 0;
}
int EQ2Emu_lua_SetMaxHP(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	sint32 value = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity() && value > 0)
		((Entity*)spawn)->AddSpellBonus(luaspell, ITEM_STAT_HEALTH, value - spawn->GetTotalHP());

	if (spawn && spawn->IsPlayer())
		((Player*)spawn)->SetCharSheetChanged(true);

	return 0;
}

int EQ2Emu_lua_SetMaxHPBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	sint32 value = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity() && value > 0)
		((Entity*)spawn)->SetTotalHPBase(value);
	return 0;
}

int EQ2Emu_lua_SetCurrentPower(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	sint32 value = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && value > 0) {
		spawn->SetPower(value);
		if (value > spawn->GetTotalPowerBase())
			spawn->SetTotalPower(value);
	}
	return 0;
}
int EQ2Emu_lua_SetMaxPower(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	sint32 value = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity() && value > 0)
		((Entity*)spawn)->AddSpellBonus(luaspell, ITEM_STAT_POWER, value - spawn->GetTotalPower());
	return 0;
}
int EQ2Emu_lua_SetMaxPowerBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	sint32 value = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity() && value > 0)
		((Entity*)spawn)->SetTotalPowerBase(value);
	return 0;
}
int EQ2Emu_lua_SetPosition(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	float x = lua_interface->GetFloatValue(state, 2);
	float y = lua_interface->GetFloatValue(state, 3);
	float z = lua_interface->GetFloatValue(state, 4);
	float heading = lua_interface->GetFloatValue(state, 5);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->SetX(x);
		spawn->SetY(y);
		spawn->SetZ(z);
		if (heading != 0)
			spawn->SetHeading(heading);
		spawn->SetSpawnOrigX(spawn->GetX());
		spawn->SetSpawnOrigY(spawn->GetY());
		spawn->SetSpawnOrigZ(spawn->GetZ());
		spawn->SetSpawnOrigHeading(spawn->GetHeading());
		if (spawn->IsPlayer()) {
			Client* client = ((Player*)spawn)->GetClient();
			if (client) {
				EQ2Packet* packet = client->GetPlayer()->Move(x, y, z, client->GetVersion(), (heading == 0 ? -1.0f : (heading + 180.0f)));
				client->QueuePacket(packet);
			}
		}

	}
	return 0;
}
int EQ2Emu_lua_SetHeading(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	float value = lua_interface->GetFloatValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->SetHeading(value);
		if (spawn->IsPlayer()) {
			Client* client = ((Player*)spawn)->GetClient();
			if (client) {
				EQ2Packet* packet = client->GetPlayer()->Move(spawn->GetX(), spawn->GetY(), spawn->GetZ(), client->GetVersion(), value + 180.0f);
				client->QueuePacket(packet);
			}
		}
	}
	return 0;
}
int EQ2Emu_lua_SetModelType(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int16 value = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn)
		spawn->SetModelType(value);
	return 0;
}
int EQ2Emu_lua_SetAdventureClass(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 value = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		if (spawn->IsPlayer())
			((Player*)spawn)->SetPlayerAdventureClass(value);
		else
			spawn->SetAdventureClass(value);
	}
	return 0;
}
int EQ2Emu_lua_SetTradeskillClass(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 value = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->SetTradeskillClass(value);
		if (spawn->IsEntity()) {
			((Entity*)spawn)->GetInfoStruct()->set_tradeskill_class1(classes.GetTSBaseClass(spawn->GetTradeskillClass()));
			((Entity*)spawn)->GetInfoStruct()->set_tradeskill_class2(classes.GetSecondaryTSBaseClass(spawn->GetTradeskillClass()));
			((Entity*)spawn)->GetInfoStruct()->set_tradeskill_class3(spawn->GetTradeskillClass());
		}
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}
int EQ2Emu_lua_SetMount(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int16 value = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->SetMount(value);
		EQ2_Color color;
		color.red = 255;
		color.green = 255;
		color.blue = 255;
		((Entity*)spawn)->SetMountColor(&color);
		((Entity*)spawn)->SetMountSaddleColor(&color);
	}
	return 0;
}
int EQ2Emu_lua_SetMountColor(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	EQ2_Color mount_color;
	EQ2_Color saddle_color;
	mount_color.red = lua_interface->GetInt8Value(state, 2);
	mount_color.green = lua_interface->GetInt8Value(state, 3);
	mount_color.blue = lua_interface->GetInt8Value(state, 4);
	saddle_color.red = lua_interface->GetInt8Value(state, 5);
	saddle_color.green = lua_interface->GetInt8Value(state, 6);
	saddle_color.blue = lua_interface->GetInt8Value(state, 7);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->SetMountColor(&mount_color);
		((Entity*)spawn)->SetMountSaddleColor(&saddle_color);
	}
	return 0;
}
int EQ2Emu_lua_GetMount(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetInt32Value(state, ((Entity*)spawn)->GetMount());
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_GetRace(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn)
	{
		LogWrite(LUA__DEBUG, 0, "LUA", "%s - Race: %i", __FUNCTION__, spawn->GetRace());
		lua_interface->SetInt32Value(state, spawn->GetRace());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetRaceName(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetStringValue(state, races.GetRaceName(spawn->GetRace()));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetClass(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetAdventureClass());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetClassName(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetStringValue(state, classes.GetClassName(spawn->GetAdventureClass()));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetSpeed(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	float value = lua_interface->GetFloatValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->SetSpeed(value);
		
		if(spawn->IsEntity())
			((Entity*)spawn)->SetSpeed(value);
		if (spawn->IsPlayer()) {
			Client* client = ((Player*)spawn)->GetClient();
			if (client) {
				PacketStruct* packet = configReader.getStruct("WS_SetControlGhost", client->GetVersion());
				if (packet) {
					packet->setDataByName("spawn_id", 0xFFFFFFFF);
					packet->setDataByName("speed", value);
					packet->setDataByName("size", 0.51);
					EQ2Packet* app = packet->serialize();
					client->QueuePacket(app);
					safe_delete(packet);
				}
			}
		}
	}
	return 0;
}

int EQ2Emu_lua_AddSpellBonus(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	const int16 type = lua_interface->GetInt16Value(state, 2);
	const float value = lua_interface->GetFloatValue(state, 3);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	
	if(!luaspell || luaspell->resisted) {
		return 0;
	}

	int64 class_req = 0;
	int32 class_id = 0;
	vector<int16> faction_req;
	vector<int16> race_req;
	int32 i = 0;
	int8 f = 0;
	int8 r = 0;
	while ((class_id = lua_interface->GetInt32Value(state, 4 + i))) {
		if (class_id < 100) {
			class_req += pow(2.0, double(class_id - 1));
		}
		else if (class_id > 100 && class_id < 1000) {
			race_req.push_back(class_id);
			r++;
		}
		else {
			faction_req.push_back(class_id);
			f++;
		}
		i++;
	}

	if (value != 0 && type >= 0) {
		if (luaspell && luaspell->spell && luaspell->caster) {
			ZoneServer* zone = luaspell->caster->GetZone();
			luaspell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
			for (int32 i = 0; i < luaspell->targets.size(); i++) {
				Spawn* target = zone->GetSpawnByID(luaspell->targets[i]);
				if (target) {
					if (target->IsPlayer()) {
						((Player*)target)->AddSpellBonus(luaspell, type, value, class_req, race_req, faction_req);
						LogWrite(LUA__DEBUG, 0, "LUA", "Applying Spell Bonus to Player  '%s'.  Is a Group Member.", ((Player*)target)->GetName());
						if (((Player*)target)->GetGroupMemberInfo())
							((Player*)target)->UpdateGroupMemberInfo();
						((Player*)target)->SetCharSheetChanged(true);
					}
					else if (target->IsNPC())
						((NPC*)target)->AddSpellBonus(luaspell, type, value, class_req, race_req, faction_req);
					else
						lua_interface->LogError("%s: Error applying spell bonus on non entity.", lua_interface->GetScriptName(state));
				}
			}
			luaspell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_SPELLBONUS))
				luaspell->effect_bitmask += EFFECT_FLAG_SPELLBONUS;
		}
		else if (spawn && spawn->IsEntity()) {
			((Entity*)spawn)->AddSpellBonus(luaspell, type, value, class_req, race_req, faction_req);
			LogWrite(LUA__DEBUG, 0, "LUA", "Applying Spell Bonus to Entity  '%s'.  Is a Group Member.", ((Entity*)spawn)->GetName());
			if (spawn->IsPlayer())
				((Player*)spawn)->SetCharSheetChanged(true);
		}
		else
			lua_interface->LogError("%s: Unable to apply spell bonus in AddSpellBonus.", lua_interface->GetScriptName(state));
	}
	else
		lua_interface->LogError("%s: Invalid parameters for AddSpellBonus.", lua_interface->GetScriptName(state));
	return 0;
}

int EQ2Emu_lua_AddSpawnSpellBonus(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int16 type = lua_interface->GetInt16Value(state, 2);
	sint32 value = lua_interface->GetSInt32Value(state, 3);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA AddSpawnSpellBonus command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA AddSpawnSpellBonus command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (value == 0) {
		lua_interface->LogError("%s: LUA AddSpawnSpellBonus command error: value must be set", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!luaspell || !luaspell->spell) {
		lua_interface->LogError("%s: LUA AddSpawnSpellBonus command error: can only be used in a spell script", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(luaspell->resisted) {
		return 0;
	}

	int32 class_req = 0;
	vector<int16> faction_req;
	vector<int16> race_req;
	int32 class_id = 0;
	int32 i = 0;
	int8 f = 0;
	int8 r = 0;
	while ((class_id = lua_interface->GetInt32Value(state, 4 + i))) {
		if (class_id < 100) {
			class_req += pow(2.0, double(class_id - 1));
		}
		else if (class_id > 100 && class_id < 1000) {
			race_req.push_back(class_id);
			r++;
		}
		else {
			faction_req.push_back(class_id);
			f++;
		}
		i++;
	}


	((Entity*)spawn)->AddSpellBonus(luaspell, type, value, class_req, race_req, faction_req);
	if (!(luaspell->effect_bitmask & EFFECT_FLAG_SPELLBONUS))
		luaspell->effect_bitmask += EFFECT_FLAG_SPELLBONUS;
	if (spawn->IsPlayer())
		((Player*)spawn)->SetCharSheetChanged(true);

	return 0;
}

int EQ2Emu_lua_RemoveSpawnSpellBonus(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	
	if (!spawn) {
		lua_interface->LogError("%s: LUA AddSpawnSpellBonus command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA AddSpawnSpellBonus command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!luaspell || !luaspell->spell) {
		lua_interface->LogError("%s: LUA AddSpawnSpellBonus command error: can only be used in a spell script", lua_interface->GetScriptName(state));
		return 0;
	}
	
	((Entity*)spawn)->RemoveSpellBonus(luaspell);
	if (spawn->IsPlayer())
		((Player*)spawn)->SetCharSheetChanged(true);
	
	return 0;
}

int EQ2Emu_lua_RemoveSpellBonus(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->RemoveSpellBonus(luaspell);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	else if (luaspell && luaspell->spell) {
		ZoneServer* zone = nullptr;
		if (luaspell->caster != nullptr)
			zone = luaspell->caster->GetZone();
		if(!zone && spawn) {
			zone = spawn->GetZone(); // workaround to try to establish a zone to find the targets and remove the spells
		}
		Spawn* target = 0;
		if(zone) {
			luaspell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
			for (int32 i = 0; i < luaspell->targets.size(); i++) {
				target = zone->GetSpawnByID(luaspell->targets[i]);
				if (target && target->IsEntity()) {
					((Entity*)target)->RemoveSpellBonus(luaspell);
					if (target->IsPlayer())
						((Player*)target)->SetCharSheetChanged(true);
				}
			}
			luaspell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
		}
		else {
			LogWrite(LUA__ERROR, 0, "LUA", "Error removing spell bonus buff %s called by %s, zone is not available.", luaspell->spell ? luaspell->spell->GetName() : "NotSet", spawn ? spawn->GetName() : "N/A");
		}
	}
	return 0;
}

int EQ2Emu_lua_AddSkillBonus(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 skill_id = lua_interface->GetInt32Value(state, 2);
	float value = lua_interface->GetFloatValue(state, 3);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	if (value != 0) {
		int32 spell_id = 0;
		if (luaspell && luaspell->spell && luaspell->caster) {
			if(luaspell->resisted) {
				lua_interface->ResetFunctionStack(state);
				return 0;
			}
			spell_id = luaspell->spell->GetSpellID();
			ZoneServer* zone = luaspell->caster->GetZone();
			Spawn* target = 0;
			luaspell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
			for (int32 i = 0; i < luaspell->targets.size(); i++) {
				target = zone->GetSpawnByID(luaspell->targets[i]);
				if (target && target->Alive()) {
					if (target->IsPlayer()) {
						((Player*)target)->AddSkillBonus(spell_id, skill_id, value);
						Client* client = ((Player*)target)->GetClient();
						if (client) {
							EQ2Packet* packet = ((Player*)target)->GetSkills()->GetSkillPacket(client->GetVersion());
							if (packet)
								client->QueuePacket(packet);
						}
						if (!(luaspell->effect_bitmask & EFFECT_FLAG_SKILLBONUS))
							luaspell->effect_bitmask += EFFECT_FLAG_SKILLBONUS;
					}
					else if (target->IsNPC()) {
						((NPC*)target)->AddSkillBonus(spell_id, skill_id, value);
						if (!(luaspell->effect_bitmask & EFFECT_FLAG_SKILLBONUS))
							luaspell->effect_bitmask += EFFECT_FLAG_SKILLBONUS;
					}
					else
						LogWrite(LUA__ERROR, 0, "LUA", "Error applying bonus buff on '%s'.  Not a NPC or player.", target->GetName());
				}
			}
			luaspell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
		}
		else if (spawn) {
			if (spawn->IsPlayer()) {
				((Player*)spawn)->AddSkillBonus(spell_id, skill_id, value);
				Client* client = ((Player*)spawn)->GetClient();
				if (client) {
					EQ2Packet* packet = ((Player*)spawn)->GetSkills()->GetSkillPacket(client->GetVersion());
					if (packet)
						client->QueuePacket(packet);
				}
			}
			else if (spawn->IsNPC())
				((NPC*)spawn)->AddSkillBonus(spell_id, skill_id, value);
			else
				LogWrite(LUA__ERROR, 0, "LUA", "Error applying skill bonus on '%s'.  Not a NPC or player.", spawn->GetName());
		}
	}
	else
		lua_interface->LogError("%s: Invalid parameters for AddSkillBonus.", lua_interface->GetScriptName(state));
	return 0;
}

int EQ2Emu_lua_RemoveSkillBonus(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	int32 spell_id = 0;
	if (luaspell && luaspell->spell && luaspell->caster) {
		if(luaspell->resisted) {
			return 0;
		}
		spell_id = luaspell->spell->GetSpellID();
		ZoneServer* zone = luaspell->caster->GetZone();
		Spawn* target = 0;
		luaspell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int32 i = 0; i < luaspell->targets.size(); i++) {
			target = zone->GetSpawnByID(luaspell->targets[i]);
			if (target) {
				if (target->IsPlayer()) {
					((Player*)target)->RemoveSkillBonus(spell_id);
					Client* client = ((Player*)target)->GetClient();
					if (client) {
						EQ2Packet* packet = ((Player*)target)->GetSkills()->GetSkillPacket(client->GetVersion());
						if (packet)
							client->QueuePacket(packet);
					}
				}
				else if (target->IsNPC())
					((NPC*)target)->RemoveSkillBonus(spell_id);
				else
					LogWrite(LUA__ERROR, 0, "LUA", "Error removing skill bonus on '%s'.  Not a NPC or player.", spawn->GetName());
			}
		}
		luaspell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}
	else if (spawn) {
		if (spawn->IsPlayer()) {
			((Player*)spawn)->RemoveSkillBonus(spell_id);
			Client* client = ((Player*)spawn)->GetClient();
			if (client) {
				EQ2Packet* packet = ((Player*)spawn)->GetSkills()->GetSkillPacket(client->GetVersion());
				if (packet)
					client->QueuePacket(packet);
			}
		}
		else if (spawn->IsNPC())
			((NPC*)spawn)->RemoveSkillBonus(spell_id);
		else
			LogWrite(LUA__ERROR, 0, "LUA", "Error removing skill bonus on '%s'.  Not a NPC or player.", spawn->GetName());
	}
	return 0;
}

int EQ2Emu_lua_AddControlEffect(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 type = lua_interface->GetInt32Value(state, 2);
	bool only_add_spawn = lua_interface->GetInt8Value(state, 3) == 1;
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	
	if(!luaspell || luaspell->resisted) {
		return 0;
	}
	
	if (!only_add_spawn && luaspell && luaspell->spell && luaspell->caster && type != 0) {
		ZoneServer* zone = luaspell->caster->GetZone();
		Spawn* target = 0;
		luaspell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int32 i = 0; i < luaspell->targets.size(); i++) {
			target = zone->GetSpawnByID(luaspell->targets[i]);
			if (target && target->IsEntity()) {
				if (type == CONTROL_EFFECT_TYPE_MEZ) {
					((Entity*)target)->AddMezSpell(luaspell);
					if (!(luaspell->effect_bitmask & EFFECT_FLAG_MEZ))
						luaspell->effect_bitmask += EFFECT_FLAG_MEZ;
					if (target->IsNPC())
						((NPC*)target)->Brain()->AddHate(luaspell->caster, 5);
				}
				else if (type == CONTROL_EFFECT_TYPE_STIFLE) {
					((Entity*)target)->AddStifleSpell(luaspell);
					if (!(luaspell->effect_bitmask & EFFECT_FLAG_STIFLE))
						luaspell->effect_bitmask += EFFECT_FLAG_STIFLE;
					if (target->IsNPC())
						((NPC*)target)->Brain()->AddHate(luaspell->caster, 5);
				}
				else if (type == CONTROL_EFFECT_TYPE_DAZE) {
					((Entity*)target)->AddDazeSpell(luaspell);
					if (!(luaspell->effect_bitmask & EFFECT_FLAG_DAZE))
						luaspell->effect_bitmask += EFFECT_FLAG_DAZE;
					if (target->IsNPC())
						((NPC*)target)->Brain()->AddHate(luaspell->caster, 5);
				}
				else if (type == CONTROL_EFFECT_TYPE_STUN) {
					if (!(luaspell->effect_bitmask & EFFECT_FLAG_STUN))
						luaspell->effect_bitmask += EFFECT_FLAG_STUN;
					((Entity*)target)->AddStunSpell(luaspell);
					if (target->IsNPC())
						((NPC*)target)->Brain()->AddHate(luaspell->caster, 5);
				}
				else if (type == CONTROL_EFFECT_TYPE_ROOT) {
					if (!(luaspell->effect_bitmask & EFFECT_FLAG_ROOT))
						luaspell->effect_bitmask += EFFECT_FLAG_ROOT;
					((Entity*)target)->AddRootSpell(luaspell);
					if (target->IsNPC())
						((NPC*)target)->Brain()->AddHate(luaspell->caster, 5);
				}
				else if (type == CONTROL_EFFECT_TYPE_FEAR) {
					if (!(luaspell->effect_bitmask & EFFECT_FLAG_FEAR))
						luaspell->effect_bitmask += EFFECT_FLAG_FEAR;
					((Entity*)target)->AddFearSpell(luaspell);
					if (target->IsNPC())
						((NPC*)target)->Brain()->AddHate(luaspell->caster, 5);
				}
				else if (type == CONTROL_EFFECT_TYPE_WALKUNDERWATER) {
					((Entity*)target)->AddWaterwalkSpell(luaspell);
					if (!(luaspell->effect_bitmask & EFFECT_FLAG_WATERWALK))
						luaspell->effect_bitmask += EFFECT_FLAG_WATERWALK;
				}
				else if (type == CONTROL_EFFECT_TYPE_JUMPUNDERWATER) {
					((Entity*)target)->AddWaterjumpSpell(luaspell);
					if (!(luaspell->effect_bitmask & EFFECT_FLAG_WATERJUMP))
						luaspell->effect_bitmask += EFFECT_FLAG_WATERJUMP;
				}
				else if (type == CONTROL_EFFECT_TYPE_SNARE) {
					((Entity*)target)->AddSnareSpell(luaspell);
					if (!(luaspell->effect_bitmask & EFFECT_FLAG_SNARE))
						luaspell->effect_bitmask += EFFECT_FLAG_SNARE;
					if (target->IsNPC())
						((NPC*)target)->Brain()->AddHate(luaspell->caster, 5);
				}
				else if (type == CONTROL_EFFECT_TYPE_FLIGHT) {
					((Entity*)target)->AddFlightSpell(luaspell);
					if (!(luaspell->effect_bitmask & EFFECT_FLAG_FLIGHT))
						luaspell->effect_bitmask += EFFECT_FLAG_FLIGHT;
				}
				else if (type == CONTROL_EFFECT_TYPE_GLIDE) {
					((Entity*)target)->AddGlideSpell(luaspell);
					if (!(luaspell->effect_bitmask & EFFECT_FLAG_GLIDE))
						luaspell->effect_bitmask += EFFECT_FLAG_GLIDE;
				}
				else if (type == CONTROL_EFFECT_TYPE_SAFEFALL) {
					((Entity*)target)->AddSafefallSpell(luaspell);
					if (!(luaspell->effect_bitmask & EFFECT_FLAG_SAFEFALL))
						luaspell->effect_bitmask += EFFECT_FLAG_SAFEFALL;
				}
				else
					lua_interface->LogError("%s: Unhandled control effect type of %u.", lua_interface->GetScriptName(state), type);
			}
			else
				lua_interface->LogError("%s: Error applying control effect on non entity '%s'.", lua_interface->GetScriptName(state), (target != nullptr) ? target->GetName() : "NO_TARGET");
		}
		luaspell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}
	else if (only_add_spawn && spawn && spawn->IsEntity()) {
		if (type == CONTROL_EFFECT_TYPE_MEZ) {
			((Entity*)spawn)->AddMezSpell(luaspell);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_MEZ))
				luaspell->effect_bitmask += EFFECT_FLAG_MEZ;
		}
		else if (type == CONTROL_EFFECT_TYPE_STIFLE) {
			((Entity*)spawn)->AddStifleSpell(luaspell);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_STIFLE))
				luaspell->effect_bitmask += EFFECT_FLAG_STIFLE;
		}
		else if (type == CONTROL_EFFECT_TYPE_DAZE) {
			((Entity*)spawn)->AddDazeSpell(luaspell);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_DAZE))
				luaspell->effect_bitmask += EFFECT_FLAG_DAZE;
		}
		else if (type == CONTROL_EFFECT_TYPE_STUN) {
			((Entity*)spawn)->AddStunSpell(luaspell);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_STUN))
				luaspell->effect_bitmask += EFFECT_FLAG_STUN;
		}
		else if (type == CONTROL_EFFECT_TYPE_ROOT) {
			((Entity*)spawn)->AddRootSpell(luaspell);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_ROOT))
				luaspell->effect_bitmask += EFFECT_FLAG_ROOT;
		}
		else if (type == CONTROL_EFFECT_TYPE_FEAR) {
			((Entity*)spawn)->AddFearSpell(luaspell);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_FEAR))
				luaspell->effect_bitmask += EFFECT_FLAG_FEAR;
		}
		else if (type == CONTROL_EFFECT_TYPE_WALKUNDERWATER) {
			((Entity*)spawn)->AddWaterwalkSpell(luaspell);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_WATERWALK))
				luaspell->effect_bitmask += EFFECT_FLAG_WATERWALK;
		}
		else if (type == CONTROL_EFFECT_TYPE_JUMPUNDERWATER) {
			((Entity*)spawn)->AddWaterjumpSpell(luaspell);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_WATERJUMP))
				luaspell->effect_bitmask += EFFECT_FLAG_WATERJUMP;
		}
		else if (type == CONTROL_EFFECT_TYPE_SNARE) {
			((Entity*)spawn)->AddSnareSpell(luaspell);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_SNARE))
				luaspell->effect_bitmask += EFFECT_FLAG_SNARE;
		}
		else if (type == CONTROL_EFFECT_TYPE_FLIGHT) {
			((Entity*)spawn)->AddFlightSpell(luaspell);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_FLIGHT))
				luaspell->effect_bitmask += EFFECT_FLAG_FLIGHT;
		}
		else if (type == CONTROL_EFFECT_TYPE_GLIDE) {
			((Entity*)spawn)->AddGlideSpell(luaspell);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_GLIDE))
				luaspell->effect_bitmask += EFFECT_FLAG_GLIDE;
		}
		else if (type == CONTROL_EFFECT_TYPE_SAFEFALL) {
			((Entity*)spawn)->AddSafefallSpell(luaspell);
			if (!(luaspell->effect_bitmask & EFFECT_FLAG_SAFEFALL))
				luaspell->effect_bitmask += EFFECT_FLAG_SAFEFALL;
		}
		else
			lua_interface->LogError("%s: Unhandled control effect type of %u.", lua_interface->GetScriptName(state), type);
	}
	else
		lua_interface->LogError("%s: Error applying control effect on non entity '%s'.", lua_interface->GetScriptName(state), (spawn != nullptr) ? spawn->GetName() : "NO_SPAWN");
	return 0;
}

int EQ2Emu_lua_RemoveControlEffect(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 type = lua_interface->GetInt8Value(state, 2);
	bool only_remove_spawn = lua_interface->GetInt8Value(state, 3) == 1;
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		if (!only_remove_spawn && luaspell && luaspell->spell && luaspell->caster) {
			ZoneServer* zone = luaspell->caster->GetZone();
			Spawn* target = 0;
			luaspell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
			for (int32 i = 0; i < luaspell->targets.size(); i++) {
				target = zone->GetSpawnByID(luaspell->targets[i]);
				if (target) {
					if (type == CONTROL_EFFECT_TYPE_MEZ)
						((Entity*)target)->RemoveMezSpell(luaspell);
					else if (type == CONTROL_EFFECT_TYPE_STIFLE)
						((Entity*)target)->RemoveStifleSpell(luaspell);
					else if (type == CONTROL_EFFECT_TYPE_DAZE)
						((Entity*)target)->RemoveDazeSpell(luaspell);
					else if (type == CONTROL_EFFECT_TYPE_STUN)
						((Entity*)target)->RemoveStunSpell(luaspell);
					else if (type == CONTROL_EFFECT_TYPE_ROOT)
						((Entity*)target)->RemoveRootSpell(luaspell);
					else if (type == CONTROL_EFFECT_TYPE_FEAR)
						((Entity*)target)->RemoveFearSpell(luaspell);
					else if (type == CONTROL_EFFECT_TYPE_WALKUNDERWATER)
						((Entity*)target)->RemoveWaterwalkSpell(luaspell);
					else if (type == CONTROL_EFFECT_TYPE_JUMPUNDERWATER)
						((Entity*)target)->RemoveWaterjumpSpell(luaspell);
					else if (type == CONTROL_EFFECT_TYPE_SNARE)
						((Entity*)target)->RemoveSnareSpell(luaspell);
					else if (type == CONTROL_EFFECT_TYPE_FLIGHT)
						((Entity*)target)->RemoveFlightSpell(luaspell);
					else if (type == CONTROL_EFFECT_TYPE_GLIDE)
						((Entity*)target)->RemoveGlideSpell(luaspell);
					else if (type == CONTROL_EFFECT_TYPE_SAFEFALL)
						((Entity*)target)->RemoveGlideSpell(luaspell);
					else
						lua_interface->LogError("%s: Unhandled control effect type of %u.", lua_interface->GetScriptName(state), type);
				}
			}
			luaspell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
		}
		else if (only_remove_spawn) {
			if (type == CONTROL_EFFECT_TYPE_MEZ)
				((Entity*)spawn)->RemoveMezSpell(luaspell);
			else if (type == CONTROL_EFFECT_TYPE_STIFLE)
				((Entity*)spawn)->RemoveStifleSpell(luaspell);
			else if (type == CONTROL_EFFECT_TYPE_DAZE)
				((Entity*)spawn)->RemoveDazeSpell(luaspell);
			else if (type == CONTROL_EFFECT_TYPE_STUN)
				((Entity*)spawn)->RemoveStunSpell(luaspell);
			else if (type == CONTROL_EFFECT_TYPE_ROOT)
				((Entity*)spawn)->RemoveRootSpell(luaspell);
			else if (type == CONTROL_EFFECT_TYPE_FEAR)
				((Entity*)spawn)->RemoveFearSpell(luaspell);
			else if (type == CONTROL_EFFECT_TYPE_WALKUNDERWATER)
				((Entity*)spawn)->RemoveWaterwalkSpell(luaspell);
			else if (type == CONTROL_EFFECT_TYPE_JUMPUNDERWATER)
				((Entity*)spawn)->RemoveWaterjumpSpell(luaspell);
			else if (type == CONTROL_EFFECT_TYPE_SNARE)
				((Entity*)spawn)->RemoveSnareSpell(luaspell);
			else if (type == CONTROL_EFFECT_TYPE_FLIGHT)
				((Entity*)spawn)->RemoveFlightSpell(luaspell);
			else if (type == CONTROL_EFFECT_TYPE_GLIDE)
				((Entity*)spawn)->RemoveGlideSpell(luaspell);
			else if (type == CONTROL_EFFECT_TYPE_SAFEFALL)
				((Entity*)spawn)->RemoveSafefallSpell(luaspell);
			else
				lua_interface->LogError("%s: Unhandled control effect type of %u.", lua_interface->GetScriptName(state), type);
		}
	}
	return 0;
}

int EQ2Emu_lua_HasControlEffect(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 type = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	bool hasEffect = false;

	if (!spawn)
		lua_interface->LogError("%s: LUA HasControlEffect error: Could not find spawn.", lua_interface->GetScriptName(state));
	else if (!spawn->IsEntity())
		lua_interface->LogError("%s: LUA HasControlEffect error: spawn %s is not an entity!.", lua_interface->GetScriptName(state), spawn->GetName());
	else if (type < CONTROL_MAX_EFFECTS)
		hasEffect = ((Entity*)spawn)->HasControlEffect(type);
	else
		lua_interface->LogError("%s: LUA HasControlEffect unhandled control effect type of %u.", lua_interface->GetScriptName(state), type);

	lua_interface->SetBooleanValue(state, hasEffect);

	return 1;
}

int EQ2Emu_lua_GetBaseAggroRadius(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);

	float distance = 0.0f;

	lua_interface->ResetFunctionStack(state);
	
	if (!spawn)
		lua_interface->LogError("%s: LUA GetBaseAggroRadius error: Could not find spawn.", lua_interface->GetScriptName(state));
	else if (!spawn->IsNPC())
		lua_interface->LogError("%s: LUA GetBaseAggroRadius error: spawn %s is not an NPC!.", lua_interface->GetScriptName(state), spawn->GetName());
	else
		distance = ((NPC*)spawn)->GetBaseAggroRadius();

	lua_interface->SetFloatValue(state, distance);

	return 1;
}

int EQ2Emu_lua_GetAggroRadius(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);

	float distance = 0.0f;
	
	lua_interface->ResetFunctionStack(state);

	if (!spawn)
		lua_interface->LogError("%s: LUA GetAggroRadius error: Could not find spawn.", lua_interface->GetScriptName(state));
	else if (!spawn->IsNPC())
		lua_interface->LogError("%s: LUA GetAggroRadius error: spawn %s is not an NPC!.", lua_interface->GetScriptName(state), spawn->GetName());
	else
		distance = ((NPC*)spawn)->GetAggroRadius();

	lua_interface->SetFloatValue(state, distance);

	return 1;
}

int EQ2Emu_lua_SetAggroRadius(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	float distance = lua_interface->GetFloatValue(state, 2);
	bool override_ = lua_interface->GetBooleanValue(state, 3);

	bool result = false;

	lua_interface->ResetFunctionStack(state);

	if (!spawn)
		lua_interface->LogError("%s: LUA SetAggroRadius error: Could not find spawn.", lua_interface->GetScriptName(state));
	else if (!spawn->IsNPC())
		lua_interface->LogError("%s: LUA SetAggroRadius error: spawn %s is not an NPC!.", lua_interface->GetScriptName(state), spawn->GetName());
	else
	{
		((NPC*)spawn)->SetAggroRadius(distance, override_);
		result = true;
	}

	lua_interface->SetBooleanValue(state, result);

	return 1;
}

int EQ2Emu_lua_SetIntBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int16 value = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->GetInfoStruct()->set_intel_base(value);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}

int EQ2Emu_lua_SetAgiBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int16 value = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->GetInfoStruct()->set_agi_base(value);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}

int EQ2Emu_lua_SetWisBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int16 value = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->GetInfoStruct()->set_wis_base(value);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}

int EQ2Emu_lua_SetStaBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int16 value = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->GetInfoStruct()->set_sta_base(value);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}

int EQ2Emu_lua_SetStrBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int16 value = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->GetInfoStruct()->set_str_base(value);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}

int EQ2Emu_lua_SetDeity(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 value = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->SetDeity(value);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}

	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_GetDeity(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		int8 deity = ((Entity*)spawn)->GetDeity();
		lua_interface->SetInt32Value(state, deity);
		return 1;
	}
	return 0;
}


int EQ2Emu_lua_SetInt(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	sint32 value = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->AddSpellBonus(luaspell, ITEM_STAT_INT, value);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}

int EQ2Emu_lua_SetWis(lua_State* state) {
	if (!lua_interface)
		return 0;
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	Spawn* spawn = lua_interface->GetSpawn(state);
	float value = lua_interface->GetFloatValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->AddSpellBonus(luaspell, ITEM_STAT_WIS, value);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}
int EQ2Emu_lua_SetSta(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	float value = lua_interface->GetFloatValue(state, 2);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->AddSpellBonus(luaspell, ITEM_STAT_STA, value);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}
int EQ2Emu_lua_SetStr(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	float value = lua_interface->GetFloatValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->AddSpellBonus(luaspell, ITEM_STAT_STR, value);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}
int EQ2Emu_lua_SetAgi(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	float value = lua_interface->GetFloatValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		((Entity*)spawn)->AddSpellBonus(luaspell, ITEM_STAT_AGI, value);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}

int EQ2Emu_lua_GetCurrentHP(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetHP());
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_GetMaxHP(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetTotalHP());
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_GetMaxHPBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetTotalHPBase());
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_GetName(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetStringValue(state, spawn->GetName());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetLevel(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetLevel());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetDifficulty(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetDifficulty());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetCurrentPower(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetPower());
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_GetMaxPower(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetTotalPower());
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_GetMaxPowerBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetTotalPowerBase());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetDistance(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* spawn2 = lua_interface->GetSpawn(state, 2);
	bool include_radius = lua_interface->GetInt8Value(state, 3) == 1;
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn2) {
		float distance = spawn->GetDistance(spawn2, false, include_radius);

		lua_interface->SetFloatValue(state, distance);
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetX(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetFloatValue(state, spawn->GetX());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetY(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetFloatValue(state, spawn->GetY());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetZ(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetFloatValue(state, spawn->GetZ());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetHeading(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetFloatValue(state, spawn->GetHeading());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetModelType(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetModelType());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetSpeed(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetFloatValue(state, spawn->GetSpeed());
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_HasMoved(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetBooleanValue(state, ((Entity*)spawn)->HasMoved(false));
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_GetInt(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetInt32Value(state, ((Entity*)spawn)->GetInt());
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_GetWis(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetInt32Value(state, ((Entity*)spawn)->GetWis());
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_GetSta(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetInt32Value(state, ((Entity*)spawn)->GetSta());
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_GetStr(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetInt32Value(state, ((Entity*)spawn)->GetStr());
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_GetAgi(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetInt32Value(state, ((Entity*)spawn)->GetAgi());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetIntBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetInt32Value(state, ((Entity*)spawn)->GetIntBase());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetWisBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetInt32Value(state, ((Entity*)spawn)->GetWisBase());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetStaBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetInt32Value(state, ((Entity*)spawn)->GetStaBase());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetStrBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetInt32Value(state, ((Entity*)spawn)->GetStrBase());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetAgiBase(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetInt32Value(state, ((Entity*)spawn)->GetAgiBase());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetStepComplete(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	int32 step = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (!player || !player->IsPlayer()) {
		lua_interface->LogError("%s: LUA SetStepComplete command error: player is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (quest_id <= 0) {
		lua_interface->LogError("%s: LUA SetStepComplete command error: quest_id is not valid", lua_interface->GetScriptName(state));
		return 0;
	} else if ((((Player*)player)->player_quests.count(quest_id) <= 0)) {
		lua_interface->LogError("%s: LUA SetStepComplete command error: player does not have quest", lua_interface->GetScriptName(state));
		return 0;
	}
	if (step > 0) {
		Client* client = ((Player*)player)->GetClient();
		if (client)
			client->AddPendingQuestUpdate(quest_id, step);
	} else {
		lua_interface->LogError("%s: LUA SetStepComplete command error: step is not valid", lua_interface->GetScriptName(state));
	}
	return 0;
}

int EQ2Emu_lua_AddStepProgress(lua_State* state) {
	Spawn* player = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	int32 step = lua_interface->GetInt32Value(state, 3);
	int32 progress = lua_interface->GetInt32Value(state, 4);
	if (player && player->IsPlayer() && quest_id > 0 && step > 0 && progress > 0 && (((Player*)player)->player_quests.count(quest_id) > 0)) {
		Client* client = ((Player*)player)->GetClient();
		if (client)
			client->AddPendingQuestUpdate(quest_id, step, progress);
	}
	return 0;
}

int EQ2Emu_lua_GetTaskGroupStep(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	if (player && player->IsPlayer() && quest_id > 0) {
		lua_interface->SetInt32Value(state, ((Player*)player)->GetTaskGroupStep(quest_id));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_QuestStepIsComplete(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	int32 step_id = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer() && quest_id > 0) {
		lua_interface->SetBooleanValue(state, ((Player*)player)->GetQuestStepComplete(quest_id, step_id));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetQuestStep(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer() && quest_id > 0) {
		lua_interface->SetInt32Value(state, ((Player*)player)->GetQuestStep(quest_id));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_RegisterQuest(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	string name = lua_interface->GetStringValue(state, 2);
	string type = lua_interface->GetStringValue(state, 3);
	string zone = lua_interface->GetStringValue(state, 4);
	int16 level = lua_interface->GetInt16Value(state, 5);
	string description = lua_interface->GetStringValue(state, 6);
	lua_interface->ResetFunctionStack(state);
	bool load = true;
	if (!quest) {
		lua_interface->LogError("%s: Quest not given in RegisterQuest!", lua_interface->GetScriptName(state));
		load = false;
	}
	if (load && name.length() == 0) {
		lua_interface->LogError("%s: Name not given in RegisterQuest!", lua_interface->GetScriptName(state));
		load = false;
	}
	if (load && type.length() == 0) {
		lua_interface->LogError("%s: Type not given in RegisterQuest for '%s'!", lua_interface->GetScriptName(state), name.c_str());
		load = false;
	}
	if (load && zone.length() == 0) {
		lua_interface->LogError("%s: Zone not given in RegisterQuest for '%s'!", lua_interface->GetScriptName(state), name.c_str());
		load = false;
	}
	if (load && description.length() == 0) {
		lua_interface->LogError("%s: Description not given in RegisterQuest for '%s'!", lua_interface->GetScriptName(state), name.c_str());
		load = false;
	}
	if (load && level == 0) {
		lua_interface->LogError("%s: Level not given in RegisterQuest for '%s'!", lua_interface->GetScriptName(state), name.c_str());
		load = false;
	}
	if (load)
		quest->RegisterQuest(name, type, zone, level, description);
	return 0;
}

int EQ2Emu_lua_SetQuestPrereqLevel(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int8 level = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		quest->SetPrereqLevel(level);
	}
	return 0;
}

int EQ2Emu_lua_AddQuestPrereqQuest(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		quest->AddPrereqQuest(quest_id);
	}
	return 0;
}

int EQ2Emu_lua_AddQuestPrereqItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 item_id = lua_interface->GetInt32Value(state, 2);
	int8 quantity = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		if (quantity == 0)
			quantity = 1;
		Item* master_item = master_item_list.GetItem(item_id);
		if (master_item) {
			Item* item = new Item(master_item);
			item->details.count = quantity;
			quest->AddPrereqItem(item);
		}
	}
	return 0;
}
int EQ2Emu_lua_HasQuest(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if(!player || !player->IsPlayer()) {
		lua_interface->LogError("%s: LUA HasQuest command error: player is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (quest_id > 0) {
		lua_interface->SetBooleanValue(state, (((Player*)player)->HasActiveQuest(quest_id) == TRUE));
		return 1;
	} else {
		lua_interface->LogError("%s: LUA HasQuest command error: quest_id is not valid", lua_interface->GetScriptName(state));
	}
	return 0;
}

int EQ2Emu_lua_QuestReturnNPC(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 spawn_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest && spawn_id > 0)
		quest->SetQuestReturnNPC(spawn_id);
	return 0;
}

int EQ2Emu_lua_AddTimer(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 time = lua_interface->GetInt32Value(state, 2);
	string function = lua_interface->GetStringValue(state, 3);
	int32 max_count = lua_interface->GetInt32Value(state, 4);
	Spawn* player = lua_interface->GetSpawn(state, 5);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA AddTimer command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (time <= 0) {
		lua_interface->LogError("%s: LUA AddTimer command error: time is not set", lua_interface->GetScriptName(state));
		return 0;
	}

	if (function.length() == 0) {
		lua_interface->LogError("%s: LUA AddTimer command error: function is not set", lua_interface->GetScriptName(state));
		return 0;
	}

	SpawnScriptTimer* timer = new SpawnScriptTimer;
	if ( time < 10)
		time = 10;
	
	timer->timer = Timer::GetCurrentTime2() + time;
	timer->function = function;
	timer->spawn = spawn->GetID();
	timer->player = player ? player->GetID() : 0;
	if (max_count == 0)
		max_count = 1;
	timer->max_count = max_count;
	timer->current_count = 0;
	spawn->GetZone()->AddSpawnScriptTimer(timer);

	return 0;
}

int EQ2Emu_lua_StopTimer(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string function = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA StopTimer command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if(!spawn->GetZone()) {
		lua_interface->LogError("%s: LUA StopTimer command error: spawn has no zone to check spawn timers", lua_interface->GetScriptName(state));
		return 0;
	}
	spawn->GetZone()->StopSpawnScriptTimer(spawn, function);

	return 0;
}

int EQ2Emu_lua_GetQuest(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer() && quest_id > 0) {
		lua_interface->SetQuestValue(state, ((Player*)player)->player_quests[quest_id]);
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_QuestIsComplete(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer() && quest_id > 0 && (((Player*)player)->player_quests.count(quest_id) > 0)) {
		Quest* quest = ((Player*)player)->player_quests[quest_id];
		if (quest)
			lua_interface->SetBooleanValue(state, quest->GetCompleted());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_HasCompletedQuest(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer() && quest_id > 0) {
		lua_interface->SetBooleanValue(state, (((Player*)player)->HasQuestBeenCompleted(quest_id) != 0));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_ProvidesQuest(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* npc = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (npc && !npc->IsPlayer() && quest_id > 0)
		npc->AddProvidedQuest(quest_id);
	return 0;
}

int EQ2Emu_lua_OfferQuest(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* npc = lua_interface->GetSpawn(state);
	Spawn* player = lua_interface->GetSpawn(state, 2);
	int32 quest_id = lua_interface->GetInt32Value(state, 3);
	bool forced = lua_interface->GetBooleanValue(state, 4);
	lua_interface->ResetFunctionStack(state);

	/* NPC is allowed to be null */
	if (player && player->IsPlayer() && quest_id > 0) {
		Quest* master_quest = master_quest_list.GetQuest(quest_id, false);
		if (master_quest) {
			Client* client = ((Player*)player)->GetClient();
			if (!client) {
				lua_interface->LogError("%s: LUA OfferQuest command error: client is not set", lua_interface->GetScriptName(state));
			}
			Quest* quest = new Quest(master_quest);
			if (!quest) {
				lua_interface->LogError("%s: LUA OfferQuest command error: new Quest() failed.", lua_interface->GetScriptName(state));
			}
			if (client && quest) {
				if (npc)
					quest->SetQuestGiver(npc->GetDatabaseID());
				else
					quest->SetQuestGiver(0);
				client->AddPendingQuest(quest, forced);
			}
		}
		else {
			lua_interface->LogError("%s: LUA OfferQuest command error: failed to get quest %d", lua_interface->GetScriptName(state), quest_id);
		}
	}
	else {
		lua_interface->LogError("%s: LUA OfferQuest command error: player is not set or bad quest id %p %d", lua_interface->GetScriptName(state), player, quest_id);
	}
	return 0;
}

int EQ2Emu_lua_AddQuestPrereqClass(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int8 class_id = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		quest->AddPrereqClass(class_id);
	}
	return 0;
}

int EQ2Emu_lua_AddQuestPrereqRace(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int8 race = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		quest->AddPrereqRace(race);
	}
	return 0;
}

int EQ2Emu_lua_AddQuestPrereqModelType(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int16 model_type = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		quest->AddPrereqModelType(model_type);
	}
	return 0;
}

int EQ2Emu_lua_AddQuestPrereqTradeskillLevel(lua_State* state) {
	if (!lua_interface)
		return 0;

	Quest* quest = lua_interface->GetQuest(state);
	int8 level = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!quest) {
		lua_interface->LogError("%s: LUA AddQuestPrereqTradeskillLevel command error: quest is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	quest->SetPrereqTSLevel(level);
	return 0;
}

int EQ2Emu_lua_AddQuestPrereqTradeskillClass(lua_State* state) {
	if (!lua_interface)
		return 0;

	Quest* quest = lua_interface->GetQuest(state);
	int8 class_id = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!quest) {
		lua_interface->LogError("%s: LUA AddQuestPrereqTradeskillClass command error: quest is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	quest->AddPrereqTradeskillClass(class_id);
	return 0;
}

int EQ2Emu_lua_AddQuestPrereqFaction(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 faction_id = lua_interface->GetInt32Value(state, 2);
	sint32 min = lua_interface->GetSInt32Value(state, 3);
	sint32 max = lua_interface->GetSInt32Value(state, 4);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		quest->AddPrereqFaction(faction_id, min, max);
	}
	return 0;
}

int EQ2Emu_lua_AddQuestSelectableRewardItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 item_id = lua_interface->GetInt32Value(state, 2);
	int8 quantity = lua_interface->GetInt8Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		if (quantity == 0)
			quantity = 1;
		Item* master_item = master_item_list.GetItem(item_id);
		if (master_item) {
			Item* item = new Item(master_item);
			item->details.count = quantity;
			quest->AddSelectableRewardItem(item);
		}
	}
	return 0;
}

int EQ2Emu_lua_HasQuestRewardItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	if (quest) {
		int32 item_id = lua_interface->GetInt32Value(state, 2);
		vector<Item*>* items = quest->GetRewardItems();
		if (items) {
			vector<Item*>::iterator itr;
			for (itr = items->begin(); itr != items->end(); itr++) {
				if (*itr && (*itr)->details.item_id == item_id) {
					lua_interface->SetBooleanValue(state, true);
					return 1;
				}
			}
		}
	}
	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_AddQuestRewardItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 item_id = lua_interface->GetInt32Value(state, 2);
	int8 quantity = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		if (quantity == 0)
			quantity = 1;
		Item* master_item = master_item_list.GetItem(item_id);
		if (master_item) {
			Item* item = new Item(master_item);
			item->details.count = quantity;
			quest->AddRewardItem(item);
		}
	}
	return 0;
}

int EQ2Emu_lua_AddQuestRewardCoin(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 copper = lua_interface->GetInt32Value(state, 2);
	int32 silver = lua_interface->GetInt32Value(state, 3);
	int32 gold = lua_interface->GetInt32Value(state, 4);
	int32 plat = lua_interface->GetInt32Value(state, 5);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		quest->AddRewardCoins(copper, silver, gold, plat);
	}
	return 0;
}

int EQ2Emu_lua_AddQuestRewardFaction(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 faction_id = lua_interface->GetInt32Value(state, 2);
	sint32 amount = lua_interface->GetSInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (quest && faction_id > 0 && amount != 0)
		quest->AddRewardFaction(faction_id, amount);
	return 0;
}

int EQ2Emu_lua_SetQuestRewardStatus(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 status = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		quest->SetRewardStatus(status);
	}
	return 0;
}

int EQ2Emu_lua_SetStatusTmpReward(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 status = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		quest->SetStatusTmpReward(status);
	}
	return 0;
}


int EQ2Emu_lua_SetCoinTmpReward(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int64 coins = lua_interface->GetInt64Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		quest->SetCoinTmpReward(coins);
	}
	return 0;
}


int EQ2Emu_lua_SetQuestRewardComment(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	string comment = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		quest->SetRewardComment(comment);
	}
	return 0;
}

int EQ2Emu_lua_SetQuestRewardExp(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 exp = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		quest->SetRewardXP(exp);
	}
	return 0;
}

int EQ2Emu_lua_AddQuestStep(lua_State* state) {
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	int32 quantity = lua_interface->GetInt32Value(state, 4);
	float percentage = lua_interface->GetFloatValue(state, 5);
	string str_taskgroup = lua_interface->GetStringValue(state, 6);
	int16 icon = lua_interface->GetInt16Value(state, 7);
	int32 usableitemid = lua_interface->GetInt32Value(state, 8);
	if (quest) {
		const char* taskgroup = 0;
		if (str_taskgroup.length() > 0)
			taskgroup = str_taskgroup.c_str();
		
		
		int32 id = 0;
		vector<int32>* ids = 0;
		int i = 0;
		while ((id = lua_interface->GetInt32Value(state, 9 + i))) {
			if (ids == 0)
				ids = new vector<int32>;
			ids->push_back(id);
			i++;
		}

		QuestStep* quest_step = quest->AddQuestStep(step, QUEST_STEP_TYPE_NORMAL, description, ids, quantity, taskgroup, 0, 0, percentage, usableitemid);
		if (quest_step && icon && quantity > 0)
			quest_step->SetIcon(icon);
		if (quest->GetPlayer() && ((Player*)quest->GetPlayer())->GetClient()) {
			Client* client = ((Player*)quest->GetPlayer())->GetClient();
			quest->GetPlayer()->GetZone()->SendQuestUpdates(client);
		}
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}
int EQ2Emu_lua_AddQuestStepKillLogic(lua_State* state, int8 type)
{
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	int32 quantity = lua_interface->GetInt32Value(state, 4);
	float percentage = lua_interface->GetFloatValue(state, 5);
	string str_taskgroup = lua_interface->GetStringValue(state, 6);
	int16 icon = lua_interface->GetInt16Value(state, 7);
	if (quest) {
		const char* taskgroup = 0;
		if (str_taskgroup.length() > 0)
			taskgroup = str_taskgroup.c_str();
		
		int32 id = 0;
		vector<int32>* ids = 0;
		int i = 0;
		while ((id = lua_interface->GetInt32Value(state, 8 + i))) {
			if (ids == 0)
				ids = new vector<int32>;
			ids->push_back(id);
			i++;
		}
		QuestStep* quest_step = quest->AddQuestStep(step, type, description, ids, quantity, taskgroup, 0, 0, percentage, 0);
		if (quest_step && icon > 0 && quantity > 0)
			quest_step->SetIcon(icon);
		if (quest->GetPlayer() && ((Player*)quest->GetPlayer())->GetClient()) {
			Client* client = ((Player*)quest->GetPlayer())->GetClient();
			quest->GetPlayer()->GetZone()->SendQuestUpdates(client);
		}
		safe_delete(ids);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}
int EQ2Emu_lua_AddQuestStepKill(lua_State* state) {
	return EQ2Emu_lua_AddQuestStepKillLogic(state, QUEST_STEP_TYPE_KILL);
}

int EQ2Emu_lua_AddQuestStepKillByRace(lua_State* state) {
	return EQ2Emu_lua_AddQuestStepKillLogic(state, QUEST_STEP_TYPE_KILL_RACE_REQ);
}

int EQ2Emu_lua_AddQuestStepChat(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	int32 quantity = lua_interface->GetInt32Value(state, 4);
	string str_taskgroup = lua_interface->GetStringValue(state, 5);
	int16 icon = lua_interface->GetInt16Value(state, 6);
	if (quest) {
		const char* taskgroup = 0;
		if (str_taskgroup.length() > 0)
			taskgroup = str_taskgroup.c_str();
		int32 npc_id = 0;
		vector<int32>* ids = 0;
		int i = 0;
		while ((npc_id = lua_interface->GetInt32Value(state, 7 + i))) {
			if (ids == 0)
				ids = new vector<int32>;
			ids->push_back(npc_id);
			i++;
		}
		QuestStep* quest_step = quest->AddQuestStep(step, QUEST_STEP_TYPE_CHAT, description, ids, quantity, taskgroup);
		if (quest_step && icon > 0)
			quest_step->SetIcon(icon);
		if (quest->GetPlayer() && ((Player*)quest->GetPlayer())->GetClient()) {
			Client* client = ((Player*)quest->GetPlayer())->GetClient();
			if(client)
				quest->GetPlayer()->GetZone()->SendQuestUpdates(client);
		}
		safe_delete(ids);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_AddQuestStepObtainItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	int32 quantity = lua_interface->GetInt32Value(state, 4);
	float percentage = lua_interface->GetFloatValue(state, 5);
	string str_taskgroup = lua_interface->GetStringValue(state, 6);
	int16 icon = lua_interface->GetInt16Value(state, 7);
	if (quest) {
		const char* taskgroup = 0;
		if (str_taskgroup.length() > 0)
			taskgroup = str_taskgroup.c_str();
		int32 item_id = 0;
		vector<int32>* ids = 0;
		int i = 0;
		while ((item_id = lua_interface->GetInt32Value(state, 8 + i))) {
			if (ids == 0)
				ids = new vector<int32>;
			ids->push_back(item_id);
			i++;
		}
		QuestStep* quest_step = quest->AddQuestStep(step, QUEST_STEP_TYPE_OBTAIN_ITEM, description, ids, quantity, taskgroup, 0, 0, percentage, 0);
		if (quest_step && icon > 0 && quantity > 0)
			quest_step->SetIcon(icon);
		if (quest->GetPlayer() && ((Player*)quest->GetPlayer())->GetClient()) {
			Client* client = ((Player*)quest->GetPlayer())->GetClient();
			quest->GetPlayer()->GetZone()->SendQuestUpdates(client);
		}
		safe_delete(ids);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_AddQuestStepZoneLoc(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	float max_variation = lua_interface->GetFloatValue(state, 4);
	string str_taskgroup = lua_interface->GetStringValue(state, 5);
	int16 icon = lua_interface->GetInt16Value(state, 6);
	if (quest) {
		const char* taskgroup = 0;
		if (str_taskgroup.length() > 0)
			taskgroup = str_taskgroup.c_str();
		vector<Location>* locations = 0;
		int8 i = 7;
		int8 num_args = (int8)lua_interface->GetNumberOfArgs(state);
		while (true) {
			Location loc;
			loc.x = lua_interface->GetFloatValue(state, i);
			loc.y = lua_interface->GetFloatValue(state, i + 1);
			loc.z = lua_interface->GetFloatValue(state, i + 2);
			loc.zone_id = lua_interface->GetInt32Value(state, i + 3);

			if (loc.x == 0 && loc.y == 0 && loc.z == 0)
				break;
			if (locations == 0)
				locations = new vector<Location>;
			locations->push_back(loc);
			i += 4;
		}
		QuestStep* quest_step = quest->AddQuestStep(step, QUEST_STEP_TYPE_LOCATION, description, 0, 1, taskgroup, locations, max_variation);
		safe_delete(locations); // gets duplicated into new table in QuestStep constructor
		if (quest_step && icon > 0)
			quest_step->SetIcon(icon);
		if (quest->GetPlayer() && ((Player*)quest->GetPlayer())->GetClient()) {
			Client* client = ((Player*)quest->GetPlayer())->GetClient();
			quest->GetPlayer()->GetZone()->SendQuestUpdates(client);
		}
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_AddQuestStepLocation(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	float max_variation = lua_interface->GetFloatValue(state, 4);
	string str_taskgroup = lua_interface->GetStringValue(state, 5);
	int16 icon = lua_interface->GetInt16Value(state, 6);
	if (quest) {
		const char* taskgroup = 0;
		if (str_taskgroup.length() > 0)
			taskgroup = str_taskgroup.c_str();
		vector<Location>* locations = 0;
		int8 i = 7;
		int8 num_args = (int8)lua_interface->GetNumberOfArgs(state);
		while (true) {
			Location loc;
			loc.x = lua_interface->GetFloatValue(state, i);
			loc.y = lua_interface->GetFloatValue(state, i + 1);
			loc.z = lua_interface->GetFloatValue(state, i + 2);
			loc.zone_id = 0;
			
			if (loc.x == 0 && loc.y == 0 && loc.z == 0)
				break;
			if (locations == 0)
				locations = new vector<Location>;
			locations->push_back(loc);
			i += 3;
		}
		QuestStep* quest_step = quest->AddQuestStep(step, QUEST_STEP_TYPE_LOCATION, description, 0, 1, taskgroup, locations, max_variation);
		safe_delete(locations); // gets duplicated into new table in QuestStep constructor
		if (quest_step && icon > 0)
			quest_step->SetIcon(icon);
		if (quest->GetPlayer() && ((Player*)quest->GetPlayer())->GetClient()) {
			Client* client = ((Player*)quest->GetPlayer())->GetClient();
			quest->GetPlayer()->GetZone()->SendQuestUpdates(client);
		}
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}
int EQ2Emu_lua_AddQuestUsableItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	float max_variation = lua_interface->GetFloatValue(state, 4);
	string str_taskgroup = lua_interface->GetStringValue(state, 5);
	int16 icon = lua_interface->GetInt16Value(state, 6);
	if (quest) {
		const char* taskgroup = 0;
		if (str_taskgroup.length() > 0)
			taskgroup = str_taskgroup.c_str();
		vector<Location>* locations = 0;
		int i = 7;
		while (true) {
			Location loc;
			loc.x = lua_interface->GetFloatValue(state, i);
			loc.y = lua_interface->GetFloatValue(state, i + 1);
			loc.z = lua_interface->GetFloatValue(state, i + 2);
			if (loc.x == 0 && loc.y == 0 && loc.z == 0)
				break;
			if (locations == 0)
				locations = new vector<Location>;
			locations->push_back(loc);
			i += 3;
		}
		QuestStep* quest_step = quest->AddQuestStep(step, QUEST_STEP_TYPE_LOCATION, description, 0, 1, taskgroup, locations, max_variation);
		safe_delete(locations); // gets duplicated into new table in QuestStep constructor
		if (quest_step && icon > 0)
			quest_step->SetIcon(icon);
		if (quest->GetPlayer() && ((Player*)quest->GetPlayer())->GetClient()) {
			Client* client = ((Player*)quest->GetPlayer())->GetClient();
			quest->GetPlayer()->GetZone()->SendQuestUpdates(client);
		}
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}
int EQ2Emu_lua_AddQuestStepSpell(lua_State* state) {
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	int32 quantity = lua_interface->GetInt32Value(state, 4);
	float percentage = lua_interface->GetFloatValue(state, 5);
	string str_taskgroup = lua_interface->GetStringValue(state, 6);
	int16 icon = lua_interface->GetInt16Value(state, 7);
	if (quest) {
		const char* taskgroup = 0;
		if (str_taskgroup.length() > 0)
			taskgroup = str_taskgroup.c_str();
		int32 spell_id = 0;
		vector<int32>* ids = 0;
		int i = 0;
		while ((spell_id = lua_interface->GetInt32Value(state, 8 + i))) {
			if (ids == 0)
				ids = new vector<int32>;
			ids->push_back(spell_id);
			i++;
		}
		QuestStep* quest_step = quest->AddQuestStep(step, QUEST_STEP_TYPE_SPELL, description, ids, quantity, taskgroup, 0, 0, percentage, 0);
		if (quest_step && icon > 0 && quantity > 0)
			quest_step->SetIcon(icon);
		if (quest->GetPlayer() && ((Player*)quest->GetPlayer())->GetClient()) {
			Client* client = ((Player*)quest->GetPlayer())->GetClient();
			quest->GetPlayer()->GetZone()->SendQuestUpdates(client);
		}
		safe_delete(ids);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_AddQuestStepCraft(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	int32 quantity = lua_interface->GetInt32Value(state, 4);
	float percentage = lua_interface->GetFloatValue(state, 5);
	string str_taskgroup = lua_interface->GetStringValue(state, 6);
	int16 icon = lua_interface->GetInt16Value(state, 7);
	if (quest) {
		const char* taskgroup = 0;
		if (str_taskgroup.length() > 0)
			taskgroup = str_taskgroup.c_str();
		int32 item_id = 0;
		vector<int32>* ids = 0;
		int i = 0;
		while ((item_id = lua_interface->GetInt32Value(state, 8 + i))) {
			if (ids == 0)
				ids = new vector<int32>;
			ids->push_back(item_id);
			i++;
		}
		QuestStep* quest_step = quest->AddQuestStep(step, QUEST_STEP_TYPE_CRAFT, description, ids, quantity, taskgroup, 0, 0, percentage, 0);
		if (quest_step && icon > 0 && quantity > 0)
			quest_step->SetIcon(icon);
		if (quest->GetPlayer() && ((Player*)quest->GetPlayer())->GetClient()) {
			Client* client = ((Player*)quest->GetPlayer())->GetClient();
			quest->GetPlayer()->GetZone()->SendQuestUpdates(client);
		}
		safe_delete(ids);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_AddQuestStepHarvest(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	int32 quantity = lua_interface->GetInt32Value(state, 4);
	float percentage = lua_interface->GetFloatValue(state, 5);
	string str_taskgroup = lua_interface->GetStringValue(state, 6);
	int16 icon = lua_interface->GetInt16Value(state, 7);
	if (quest) {
		const char* taskgroup = 0;
		if (str_taskgroup.length() > 0)
			taskgroup = str_taskgroup.c_str();
		int32 item_id = 0;
		vector<int32>* ids = 0;
		int i = 0;
		while ((item_id = lua_interface->GetInt32Value(state, 8 + i))) {
			if (ids == 0)
				ids = new vector<int32>;
			ids->push_back(item_id);
			i++;
		}
		QuestStep* quest_step = quest->AddQuestStep(step, QUEST_STEP_TYPE_HARVEST, description, ids, quantity, taskgroup, 0, 0, percentage, 0);
		if (quest_step && icon > 0 && quantity > 0)
			quest_step->SetIcon(icon);
		if (quest->GetPlayer() && ((Player*)quest->GetPlayer())->GetClient()) {
			Client* client = ((Player*)quest->GetPlayer())->GetClient();
			quest->GetPlayer()->GetZone()->SendQuestUpdates(client);
		}
		safe_delete(ids);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_SetQuestCompleteAction(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	string action = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		if (action.length() > 0)
			quest->SetCompleteAction(action);
	}
	return 0;
}

int EQ2Emu_lua_AddQuestStepCompleteAction(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string action = lua_interface->GetStringValue(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		if (step > 0 && action.length() > 0)
			quest->AddCompleteAction(step, action);
	}
	return 0;
}

int EQ2Emu_lua_AddQuestStepProgressAction(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string action = lua_interface->GetStringValue(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (quest) {
		if (step > 0 && action.length() > 0)
			quest->AddProgressAction(step, action);
	}
	return 0;
}

int EQ2Emu_lua_UpdateQuestDescription(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	string description = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest && description.length() > 0)
		quest->SetDescription(description);
	return 0;
}

int EQ2Emu_lua_SetCompletedDescription(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	string description = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest && description.length() > 0)
		quest->SetCompletedDescription(description);
	return 0;
}

int EQ2Emu_lua_UpdateQuestTaskGroupDescription(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	bool display_bullets = (lua_interface->GetInt8Value(state, 4) == 1);
	lua_interface->ResetFunctionStack(state);
	if (quest && step > 0 && description.length() > 0) {
		quest->SetTaskGroupDescription(step, description, display_bullets);
	}
	return 0;
}

int EQ2Emu_lua_UpdateQuestStepDescription(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (quest && step > 0 && description.length() > 0) {
		quest->SetStepDescription(step, description);
	}
	return 0;
}

int EQ2Emu_lua_UpdateQuestZone(lua_State* state) {
	Quest* quest = lua_interface->GetQuest(state);
	string zone = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (quest && zone.length() > 0)
		quest->SetZone(zone);
	return 0;
}

int EQ2Emu_lua_GiveQuestReward(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	Spawn* spawn = lua_interface->GetSpawn(state, 2);

	lua_interface->ResetFunctionStack(state);
	if (quest && spawn) {
		if (spawn->IsPlayer()) {
			Client* client = ((Player*)spawn)->GetClient();
			if (client) {
				client->AddPendingQuestReward(quest);
			}
		}
	}
	return 0;
}

int EQ2Emu_lua_Harvest(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	Spawn* node = lua_interface->GetSpawn(state, 2);
	if (player && node && player->IsPlayer() && node->IsGroundSpawn()) {
		Client* client = ((Player*)player)->GetClient();
		if (client) {
			((GroundSpawn*)node)->ProcessHarvest(client);
			if (((GroundSpawn*)node)->GetNumberHarvests() == 0) {
				LuaSpell* spell = lua_interface->GetCurrentSpell(state);
				player->GetZone()->RemoveSpawn(node, true, true, true, true, (spell != nullptr) ? false : true);
			}
		}
	}
	else if (player && player->IsPlayer()) {
		Client* client = ((Player*)player)->GetClient();
		if (client)
			client->Message(CHANNEL_COLOR_RED, "Invalid target for this spell.");
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_Bind(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 zone_id = lua_interface->GetInt32Value(state, 2);
	float x = lua_interface->GetFloatValue(state, 3);
	float y = lua_interface->GetFloatValue(state, 4);
	float z = lua_interface->GetFloatValue(state, 5);
	float h = lua_interface->GetFloatValue(state, 6);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA Bind command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsPlayer()) {
		lua_interface->LogError("%s: LUA Bind command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	if (zone_id == 0) {
		Client* client = ((Player*)spawn)->GetClient();
		if (!client) {
			lua_interface->LogError("%s: LUA Bind command error: unable to get client from spawn", lua_interface->GetScriptName(state));
			return 0;
		}

		if (!client->Bind())
			client->SimpleMessage(CHANNEL_COLOR_RED, "Unable to set bind point.");
	}
	else {
		Player* player = (Player*)spawn;
		player->GetPlayerInfo()->SetBindZone(zone_id);
		player->GetPlayerInfo()->SetBindX(x);
		player->GetPlayerInfo()->SetBindY(y);
		player->GetPlayerInfo()->SetBindZ(z);
		player->GetPlayerInfo()->SetBindHeading(h);
	}

	return 0;
}

int EQ2Emu_lua_Gate(lua_State* state) {
	if (!lua_interface)
		return 0;
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		if (spawn->IsPlayer()) {
			Client* client = ((Player*)spawn)->GetClient();
			if (client) {
				if (!client->Gate((spell != nullptr) ? true : false))
					client->SimpleMessage(CHANNEL_COLOR_RED, "Unable to gate.");
			}
		}
	}
	return 0;
}

int EQ2Emu_lua_IsBindAllowed(lua_State* state) {
	if (!lua_interface)
		return 0;
	bool ret = false;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		if (spawn->IsPlayer()) {
			Client* client = ((Player*)spawn)->GetClient();
			if (client)
				ret = client->BindAllowed();
		}
	}
	lua_interface->SetBooleanValue(state, ret);
	return 1;
}

int EQ2Emu_lua_IsGateAllowed(lua_State* state) {
	if (!lua_interface)
		return 0;
	bool ret = false;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		if (spawn->IsPlayer()) {
			Client* client = ((Player*)spawn)->GetClient();
			ZoneServer* zone = lua_interface->GetZone(state);
			if (client && zone){
					ret = zone->GetCanGate();
			}
		}
	}
	lua_interface->SetBooleanValue(state, ret);
	return 1;
}

int EQ2Emu_lua_IsAlive(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetBooleanValue(state, spawn->Alive());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_IsSpawnGroupAlive(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	int32 group_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (zone) {
		lua_interface->SetBooleanValue(state, zone->IsSpawnGroupAlive(group_id));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_IsInCombat(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity()) {
		lua_interface->SetBooleanValue(state, ((Entity*)spawn)->EngagedInCombat());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SendMessage(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	string message = lua_interface->GetStringValue(state, 2);
	string color_str = lua_interface->GetStringValue(state, 3);
	lua_interface->ResetFunctionStack(state);
	int8 color = CHANNEL_NARRATIVE;
	if (spawn && spawn->IsPlayer() && message.length() > 0) {
		Client* client = ((Player*)spawn)->GetClient();
		if (client) {
			if (color_str.length() > 0) {
				// leave for backwards compat, but all future should just use the number
				if (strncasecmp(color_str.c_str(), "red", 3) == 0)
					color = CHANNEL_COLOR_RED;
				else if (strncasecmp(color_str.c_str(), "yellow", 6) == 0)
					color = CHANNEL_COLOR_YELLOW;
				else
				{
					// use a number to specify the channel as per Commands/Commands.h defines
					color = (int8)atoul(color_str.c_str());
				}
			}
			client->SimpleMessage(color, message.c_str());
		}
	}
	return 0;
}

int EQ2Emu_lua_SendPopUpMessage(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	string message = lua_interface->GetStringValue(state, 2);
	int8 red = lua_interface->GetInt8Value(state, 3);
	int8 green = lua_interface->GetInt8Value(state, 4);
	int8 blue = lua_interface->GetInt8Value(state, 5);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SendPopUpMessage command error: Spawn is not valid.", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if (!spawn->IsPlayer()) {
		lua_interface->LogError("%s: LUA SendPopUpMessage command error: Spawn is not a player.", lua_interface->GetScriptName(state));
		return 0;
	}

	int32 words = ::CountWordsInString(message.c_str());
	if (words < 5)
		words = 5;
	Client* client = ((Player*)spawn)->GetClient();
	if (client)
		client->SendPopupMessage(10, message.c_str(), "ui_harvested_normal", words, red, green, blue);
	return 0;
}

int EQ2Emu_lua_SetServerControlFlag(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 param = lua_interface->GetInt8Value(state, 2);
	int8 param_value = lua_interface->GetInt8Value(state, 3);
	int8 value = lua_interface->GetInt8Value(state, 4);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsPlayer() && (param >= 1 && param <= 5)) {
		Client* client = ((Player*)spawn)->GetClient();
		if (client) {
			PacketStruct* packet = configReader.getStruct("WS_ServerControlFlags", client->GetVersion());
			switch (param) {
			case 1: {
				packet->setDataByName("parameter1", param_value);
				break;
			}
			case 2: {
				packet->setDataByName("parameter2", param_value);
				break;
			}
			case 3: {
				packet->setDataByName("parameter3", param_value);
				break;
			}
			case 4: {
				packet->setDataByName("parameter4", param_value);
				break;
			}
			case 5: {
				packet->setDataByName("parameter5", param_value);
				break;
			}
			}
			packet->setDataByName("value", value);
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
	return 0;
}

int EQ2Emu_lua_ToggleTracking(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsPlayer()) {
		if (((Player*)spawn)->GetIsTracking())
			spawn->GetZone()->AddPlayerTracking((Player*)spawn);
		else
			spawn->GetZone()->RemovePlayerTracking((Player*)spawn, TRACKING_STOP);
	}
	return 0;
}

int EQ2Emu_lua_AddPrimaryEntityCommand(lua_State* state) {
	Spawn* player = lua_interface->GetSpawn(state);
	Spawn* spawn = lua_interface->GetSpawn(state, 2);
	string name = lua_interface->GetStringValue(state, 3);
	float distance = lua_interface->GetFloatValue(state, 4);
	string command = lua_interface->GetStringValue(state, 5);
	string error_text = lua_interface->GetStringValue(state, 6);
	int16 cast_time = lua_interface->GetInt16Value(state, 7);
	int32 spell_visual = lua_interface->GetInt32Value(state, 8);
	bool denyListDefault = (lua_interface->GetInt8Value(state, 9) == 1);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		if (distance == 0)
			distance = 10.0f;
		if (command.length() == 0)
			command = name;
		if (command.length() < 1 && name.length() < 1)
		{
			// have to run this first to send a 'blank' default command, then remove all commands from the list
			spawn->GetZone()->SendUpdateDefaultCommand(spawn, command.c_str(), distance);
			spawn->RemovePrimaryCommands();
		}
		else
		{
			spawn->AddPrimaryEntityCommand(name.c_str(), distance, command.c_str(), error_text.c_str(), cast_time, spell_visual, denyListDefault, (player && player->IsPlayer()) ? (Player*)player : NULL);
		}
	}
	return 0;
}

int EQ2Emu_lua_HasSpell(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 spellid = lua_interface->GetInt32Value(state, 2);
	int16 tier = lua_interface->GetInt16Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer()) {
		lua_interface->SetBooleanValue(state, ((Player*)player)->HasSpell(spellid, tier, true));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_AddSpellBookEntry(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 spellid = lua_interface->GetInt32Value(state, 2);
	int16 tier = lua_interface->GetInt16Value(state, 3);
	int8 num_args = (int8)lua_interface->GetNumberOfArgs(state);
	bool add_silently = lua_interface->GetBooleanValue(state, 4);
	bool add_to_hotbar = true;
	if (num_args > 4) {
		add_to_hotbar = lua_interface->GetBooleanValue(state, 5);
	}
	lua_interface->ResetFunctionStack(state);
	Spell* spell = master_spell_list.GetSpell(spellid, tier);
	if (player && spell && player->IsPlayer()) {
		Client* client = player->GetClient();
		if (client) {
			if (!client->GetPlayer()->HasSpell(spellid, tier - 1, true))
			{
				Spell* spell = master_spell_list.GetSpell(spellid, tier);
				client->GetPlayer()->AddSpellBookEntry(spellid, 1, client->GetPlayer()->GetFreeSpellBookSlot(spell->GetSpellData()->spell_book_type), spell->GetSpellData()->spell_book_type, spell->GetSpellData()->linked_timer, true);
				client->GetPlayer()->UnlockSpell(spell);
				client->SendSpellUpdate(spell, add_silently, add_to_hotbar);
			}
			else
			{
				Spell* spell = master_spell_list.GetSpell(spellid, tier);
				int8 old_slot = client->GetPlayer()->GetSpellSlot(spell->GetSpellID());
				client->GetPlayer()->RemoveSpellBookEntry(spell->GetSpellID());
				client->GetPlayer()->AddSpellBookEntry(spell->GetSpellID(), spell->GetSpellTier(), old_slot, spell->GetSpellData()->spell_book_type, spell->GetSpellData()->linked_timer, true);
				client->GetPlayer()->UnlockSpell(spell);
				client->SendSpellUpdate(spell, add_silently, add_to_hotbar);
			}



			//if (client ) {
			//	((Player*)player)->AddSpellBookEntry(spell->GetSpellID(), spell->GetSpellTier(), ((Player*)player)->GetFreeSpellBookSlot(spell->GetSpellData()->spell_book_type), spell->GetSpellData()->spell_book_type, spell->GetSpellData()->linked_timer, true);
			EQ2Packet* outapp = ((Player*)player)->GetSpellBookUpdatePacket(client->GetVersion());
			if (outapp)
				client->QueuePacket(outapp);
		}
	}
	return 0;
}

int EQ2Emu_lua_DeleteSpellBook(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int8 type_selection = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer()) {
		Client* client = player->GetClient();
		if (client) {
			((Player*)player)->DeleteSpellBook(type_selection);
			EQ2Packet* outapp = ((Player*)player)->GetSpellBookUpdatePacket(client->GetVersion());
			if (outapp)
				client->QueuePacket(outapp);
		}
	}
	return 0;
}

int EQ2Emu_lua_SendNewAdventureSpells(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer()) {
		Client* client = player->GetClient();
		if (client) {
			client->SendNewAdventureSpells();
		}
	}
	return 0;
}


int EQ2Emu_lua_SendNewTradeskillSpells(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer()) {
		Client* client = player->GetClient();
		if (client) {
			client->SendNewTradeskillSpells();
		}
	}
	return 0;
}

int EQ2Emu_lua_RemoveSpellBookEntry(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 spellid = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer()) {
		SpellBookEntry* sbe = ((Player*)player)->GetSpellBookSpell(spellid);
		Client* client = player->GetClient();
		if (sbe && client) {
			((Player*)player)->RemoveSpellBookEntry(spellid);
			EQ2Packet* outapp = ((Player*)player)->GetSpellBookUpdatePacket(client->GetVersion());
			if (outapp)
				client->QueuePacket(outapp);
		}
	}
	return 0;
}


int EQ2Emu_lua_HasFreeSlot(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer()) {
		lua_interface->SetBooleanValue(state, ((Player*)player)->item_list.HasFreeSlot());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_Attack(lua_State* state) {
	if (lua_interface) {
		Spawn* npc = lua_interface->GetSpawn(state);
		Spawn* player = lua_interface->GetSpawn(state, 2);
		lua_interface->ResetFunctionStack(state);
		if (npc && player && npc->IsNPC() && player->IsEntity())
			((NPC*)npc)->AddHate((Entity*)player, 100);
	}
	return 0;
}

int EQ2Emu_lua_ApplySpellVisual(lua_State* state) {
	if (lua_interface) {
		Spawn* target = lua_interface->GetSpawn(state);
		int32 spell_visual = lua_interface->GetInt32Value(state, 2);
		lua_interface->ResetFunctionStack(state);
		if (target && target->GetZone())
			target->GetZone()->SendCastSpellPacket(spell_visual, target);
	}
	return 0;
}

int EQ2Emu_lua_HasCollectionsToHandIn(lua_State* state) {
	Spawn* player;

	if (lua_interface) {
		player = lua_interface->GetSpawn(state);
		lua_interface->ResetFunctionStack(state);
		if (player && player->IsPlayer()) {
			lua_interface->SetBooleanValue(state, ((Player*)player)->GetCollectionList()->HasCollectionsToHandIn());
			return 1;
		}
	}

	return 0;
}

int EQ2Emu_lua_HandInCollections(lua_State* state) {
	Spawn* player;
	Client* client;

	if (lua_interface) {
		player = lua_interface->GetSpawn(state);
		lua_interface->ResetFunctionStack(state);
		if (player && ((Player*)player)->IsPlayer() && ((Player*)player)->GetCollectionList()->HasCollectionsToHandIn())
			if ((client = ((Player*)player)->GetClient()))
				client->HandInCollections();
	}

	return 0;
}

int EQ2Emu_lua_UseWidget(lua_State* state) {
	Spawn* widget;

	if (lua_interface) {
		widget = lua_interface->GetSpawn(state);
		lua_interface->ResetFunctionStack(state);
		if (widget && widget->IsWidget())
			((Widget*)widget)->HandleUse(nullptr, "");
	}

	return 0;
}

int EQ2Emu_lua_SetSpellList(lua_State* state) {
	Spawn* spawn = 0;
	int32 primary_list = 0;
	int32 secondary_list = 0;

	if (lua_interface) {
		spawn = lua_interface->GetSpawn(state);
		primary_list = lua_interface->GetInt32Value(state, 2);
		secondary_list = lua_interface->GetInt32Value(state, 3);
		lua_interface->ResetFunctionStack(state);

		if (!spawn->IsNPC()) {
			lua_interface->LogError("%s: LUA SetSpellList command error: Spawn was not a valid NPC", lua_interface->GetScriptName(state));
			return 0;
		}

		NPC* npc = (NPC*)spawn;
		npc->SetPrimarySpellList(primary_list);
		npc->SetSecondarySpellList(secondary_list);
		npc->SetSpells(world.GetNPCSpells(npc->GetPrimarySpellList(), npc->GetSecondarySpellList()));
	}
	return 0;
}

int EQ2Emu_lua_GetPet(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		if (spawn->IsEntity() && ((Entity*)spawn)->GetPet()) {
			lua_interface->SetSpawnValue(state, ((Entity*)spawn)->GetPet());
			return 1;
		}
	}
	return 0;
}

int EQ2Emu_lua_GetCharmedPet(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		if (spawn->IsEntity() && ((Entity*)spawn)->GetCharmedPet()) {
			lua_interface->SetSpawnValue(state, ((Entity*)spawn)->GetCharmedPet());
			return 1;
		}
	}
	return 0;
}

int EQ2Emu_lua_GetDeityPet(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		if (spawn->IsEntity() && ((Entity*)spawn)->GetDeityPet()) {
			lua_interface->SetSpawnValue(state, ((Entity*)spawn)->GetDeityPet());
			return 1;
		}
	}
	return 0;
}

int EQ2Emu_lua_GetCosmeticPet(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		if (spawn->IsEntity() && ((Entity*)spawn)->GetCosmeticPet()) {
			lua_interface->SetSpawnValue(state, ((Entity*)spawn)->GetCosmeticPet());
			return 1;
		}
	}
	return 0;
}
int EQ2Emu_lua_Charm(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* owner = lua_interface->GetSpawn(state);
	Spawn* pet = lua_interface->GetSpawn(state, 2);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	if (!luaspell) {
		lua_interface->LogError("%s: LUA Charm command error: Spell is not valid, charm can only be used in spell scripts.", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(luaspell->resisted) {
		return 0;
	}
	if (owner && pet && owner->IsEntity() && pet->IsNPC()) {
		((Entity*)owner)->SetCharmedPet((Entity*)pet);
		pet->SetPet(true);
		((NPC*)pet)->SetPetType(PET_TYPE_CHARMED);
		((NPC*)pet)->SetOwner((Entity*)owner);
		// If owner is player and player does not have a summoned pet set the players charsheet
		if (owner->IsPlayer() && !((Entity*)owner)->GetPet()) {
			Player* player = (Player*)owner;
			player->GetInfoStruct()->set_pet_id(player->GetIDWithPlayerSpawn(pet));
			player->GetInfoStruct()->set_pet_name(std::string(pet->GetName()));
			player->GetInfoStruct()->set_pet_movement(2);
			player->GetInfoStruct()->set_pet_behavior(3);
			player->GetInfoStruct()->set_pet_health_pct(1.0f);
			player->GetInfoStruct()->set_pet_power_pct(1.0f);
			// Make sure the values get sent to the client
			player->SetCharSheetChanged(true);
		}
		// Clear the spawns script so the charmed mob doesn't try to do anything like random walks
		pet->SetSpawnScript("");
		// Set faction to the same as the owner
		pet->SetFactionID(owner->GetFactionID());

		((NPC*)pet)->SetPetSpellID(luaspell->spell->GetSpellData()->id);
		((NPC*)pet)->SetPetSpellTier(luaspell->spell->GetSpellData()->tier);

		// Clear hate list
		((NPC*)pet)->Brain()->ClearHate();

		// Set the brain to a pet brain
		((NPC*)pet)->SetBrain(new CombatPetBrain((NPC*)pet));
	}
	return 0;
}

int EQ2Emu_lua_GetGroup(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetGroup command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	vector<Spawn*> groupMembers;
	if (!spawn->IsPlayer() && spawn->HasSpawnGroup()) {
		groupMembers = *spawn->GetSpawnGroup();
	}
	else if (spawn->IsPlayer() && ((Player*)spawn)->GetGroupMemberInfo()) {
		world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);

		deque<GroupMemberInfo*>::iterator itr;
		PlayerGroup* group = world.GetGroupManager()->GetGroup(((Player*)spawn)->GetGroupMemberInfo()->group_id);
		if (group)
		{
			group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
			deque<GroupMemberInfo*>* members = group->GetMembers();
			GroupMemberInfo* info = 0;
			for (itr = members->begin(); itr != members->end(); itr++) {
				info = *itr;
				if (info->client)
					groupMembers.push_back(info->client->GetPlayer());
			}
			group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
		}

		world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
	}
	else
		return 0;

	lua_createtable(state, groupMembers.size(), 0);
	int newTable = lua_gettop(state);
	for (int32 i = 0; i < groupMembers.size(); i++) {
		lua_interface->SetSpawnValue(state, groupMembers.at(i));
		lua_rawseti(state, newTable, i + 1);
	}
	return 1;

}

int EQ2Emu_lua_CreateOptionWindow(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	lua_interface->ResetFunctionStack(state);
	vector<OptionWindowOption>* option_window = new vector<OptionWindowOption>();
	lua_interface->SetOptionWindowValue(state, option_window);
	return 1;
}

int EQ2Emu_lua_AddOptionWindowOption(lua_State* state) {
	if (!lua_interface)
		return 0;
	vector<OptionWindowOption>* option_window = lua_interface->GetOptionWindow(state);
	if (option_window) {
		OptionWindowOption option_window_option;
		option_window_option.optionName = lua_interface->GetStringValue(state, 2);
		option_window_option.optionDescription = lua_interface->GetStringValue(state, 3);
		option_window_option.optionIconSheet = lua_interface->GetInt32Value(state, 4);
		option_window_option.optionIconID = lua_interface->GetInt16Value(state, 5);
		option_window_option.optionCommand = lua_interface->GetStringValue(state, 6);
		option_window_option.optionConfirmTitle = lua_interface->GetStringValue(state, 7);
		if (option_window_option.optionName.length() > 0 && option_window_option.optionDescription.length() > 0)
			option_window->push_back(option_window_option);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_SendOptionWindow(lua_State* state) {
	if (!lua_interface)
		return 0;

	vector<OptionWindowOption>* option_window = lua_interface->GetOptionWindow(state);
	Spawn* player = lua_interface->GetSpawn(state, 2);
	string window_title = lua_interface->GetStringValue(state, 3);
	string cancel_command = lua_interface->GetStringValue(state, 4);

	lua_interface->ResetFunctionStack(state);

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA SendOptionWindow command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	Client* client = ((Player*)player)->GetClient();
	
	if (option_window && window_title.length() > 0 && client) {
		PacketStruct* packet = configReader.getStruct("WS_SelectTradeskill", client->GetVersion());
		if (!packet)
			return 0;

		packet->setDataByName("title_text", window_title.c_str());
		if (cancel_command.length() > 0)
			packet->setDataByName("command_text_cancel", cancel_command.c_str());

		packet->setArrayLengthByName("num_selections", option_window->size());
		vector<OptionWindowOption>::iterator itr;
		int8 i = 0;
		for (itr = option_window->begin(); itr != option_window->end(); itr++) {
			OptionWindowOption opt = *itr;
			packet->setArrayDataByName("tradeskill_name", opt.optionName.c_str(), i);
			packet->setArrayDataByName("tradeskill_description", opt.optionDescription.c_str(), i);
			packet->setArrayDataByName("icon_sheet", opt.optionIconSheet, i);
			packet->setArrayDataByName("icon_id", opt.optionIconID, i);
			if (opt.optionCommand.length() > 0)
				packet->setArrayDataByName("command_text", opt.optionCommand.c_str(), i);
			if (opt.optionConfirmTitle.length() > 0)
				packet->setArrayDataByName("confirm_window_title", opt.optionConfirmTitle.c_str(), i);

			i++;
		}
		client->QueuePacket(packet->serialize());
		lua_interface->SetLuaUserDataStale(option_window);
		safe_delete(option_window);
		safe_delete(packet);
	}
	return 0;
}

int EQ2Emu_lua_GetTradeskillClass(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetTradeskillClass());
		return 1;
	}
	else
		lua_interface->LogError("%s: LUA GetTradeskillClass command error: Spawn was not valid", lua_interface->GetScriptName(state));

	return 0;
}

int EQ2Emu_lua_GetTradeskillLevel(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetInt32Value(state, spawn->GetTSLevel());
		return 1;
	}
	else
		lua_interface->LogError("%s: LUA GetTradeskillLevel command error: Spawns was not valid", lua_interface->GetScriptName(state));
	return 0;
}

int EQ2Emu_lua_GetTradeskillClassName(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		int8 class_id = spawn->GetTradeskillClass();
		// Need to add 42 for the offset in the array
		class_id += 44;
		lua_interface->SetStringValue(state, classes.GetClassNameCase(class_id).c_str());
		return 1;
	}
	else
		lua_interface->LogError("%s: LUA GetTradeskillClassName command error: Spawn was not valid", lua_interface->GetScriptName(state));
	return 0;
}

int EQ2Emu_lua_SetTradeskillLevel(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int16 level = lua_interface->GetInt8Value(state, 2);
	
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		if (spawn->IsPlayer() && ((Player*)spawn)->GetClient())
			((Player*)spawn)->GetClient()->ChangeTSLevel(spawn->GetTSLevel(), level);
		else
			spawn->SetTSLevel(level);
	}
	else
		lua_interface->LogError("%s: LUA SetTradeskillLevel command error: Spawn was not valid", lua_interface->GetScriptName(state));
	return 0;
}

int EQ2Emu_lua_SetAttackable(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 attackable = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {		
		spawn->SetAttackable(attackable);
		spawn->vis_changed = true; //some clients store this in vis instead of info, need to make sense both are updated
	}
	return 0;
}

int EQ2Emu_lua_SummonPet(lua_State* state) {
	// Check to see if we have a valid lua_interface
	if (!lua_interface)
		return 0;

	// Get the spawn that is getting the pet
	Spawn* spawn = lua_interface->GetSpawn(state);
	// Get the DB ID of the pet
	int32 pet_id = lua_interface->GetInt32Value(state, 2);
	// The max level the pet can gain
	int8 max_level = lua_interface->GetInt8Value(state, 3);
	// Get the spell that this command was called from
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	// Check to make sure the spawn pointer is valid
	if (!spawn) {
		lua_interface->LogError("%s: LUA SummonPet command error: Spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	// Check to make sure the spawn is an entity
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA SummonPet command error: Spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	// Check to make sure the spawn doesn't already have a pet of this type
	if (((Entity*)spawn)->GetPet()) {
		if (spawn->IsPlayer()) {
			Client* client = ((Player*)spawn)->GetClient();
			if (client)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You already have a pet.");
		}

		lua_interface->LogError("%s: LUA SummonPet command error: spawn already has a pet of this type", lua_interface->GetScriptName(state));
		return 0;
	}

	// Check to see if the DB ID for the pet is set
	if (pet_id == 0) {
		lua_interface->LogError("%s: LUA SummonPet command error: pet_id can not be set to 0", lua_interface->GetScriptName(state));
		return 0;
	}

	// Check to see if the pointer to the spell is valid
	if (!luaspell) {
		lua_interface->LogError("%s: LUA SummonPet command error: valid spell not found, SummonPet can only be used in spell scripts", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(luaspell->resisted) {
		return 0;
	}
	
	// Get a pointer to a spawn with the given DB ID and check if the pointer is valid
	Spawn* pet = spawn->GetZone()->GetSpawn(pet_id);
	if (!pet) {
		lua_interface->LogError("%s: LUA SummonPet command error: Could not find spawn with id of %u.", lua_interface->GetScriptName(state), pet_id);
		return 0;
	}

	// Check to make sure the pet is an npc
	if (!pet->IsNPC()) {
		lua_interface->LogError("%s: LUA SummonPet command error: id (%u) did not point to a npc", lua_interface->GetScriptName(state), pet_id);
		return 0;
	}

	// Spawn the pet at the same location as the owner
	pet->SetX(spawn->GetX());
	pet->SetY(spawn->GetY());
	pet->SetZ(spawn->GetZ());
	pet->SetLocation(spawn->GetLocation());
	pet->SetHeading(spawn->GetHeading());

	std::string petName = std::string("");
	if(spawn->IsEntity()) {
		petName = ((Entity*)spawn)->GetInfoStruct()->get_pet_name();
	}
	
	if(petName.size() < 1) {
		int16 rand_index = MakeRandomInt(0, spawn->GetZone()->pet_names.size() - 1);
		petName = spawn->GetZone()->pet_names.at(rand_index);
		LogWrite(PET__DEBUG, 0, "Pets", "Randomize Pet Name: '%s' (rand: %i)", petName.c_str(), rand_index);
	}

	// If player set various values for the char sheet (pet window)
	if (spawn->IsPlayer()) {
		Player* player = (Player*)spawn;
		
		player->GetInfoStruct()->set_pet_id(player->GetIDWithPlayerSpawn(pet));
		player->GetInfoStruct()->set_pet_name(petName);
		player->GetInfoStruct()->set_pet_movement(2);
		player->GetInfoStruct()->set_pet_behavior(3);
		player->GetInfoStruct()->set_pet_health_pct(1.0f);
		player->GetInfoStruct()->set_pet_power_pct(1.0f);
		// Make sure the values get sent to the client
		player->SetCharSheetChanged(true);
	}

	// Set the pets name
	pet->SetName(petName.c_str());
	// Set the level of the pet to the owners level or max level(if set) if owners level is greater
	if (max_level > 0)
		pet->SetLevel(spawn->GetLevel() >= max_level ? max_level : spawn->GetLevel());
	else
		pet->SetLevel(spawn->GetLevel());
	// Set the max level this pet can reach
	((NPC*)pet)->SetMaxPetLevel(max_level);
	
	((NPC*)pet)->UpdateWeapons();
	
	// Set the faction of the pet to the same faction as the owner
	pet->SetFactionID(spawn->GetFactionID());
	// Set the spawn as a pet
	pet->SetPet(true);
	// Give a pointer of the owner to the pet
	((NPC*)pet)->SetOwner((Entity*)spawn);
	// Give a pointer of the pet to the owner
	((Entity*)spawn)->SetCombatPet((Entity*)pet);
	// Set the pet type
	((NPC*)pet)->SetPetType(PET_TYPE_COMBAT);
	// Set the spell id used to create this pet
	((NPC*)pet)->SetPetSpellID(luaspell->spell->GetSpellData()->id);
	// Set the spell tier used to create this pet
	((NPC*)pet)->SetPetSpellTier(luaspell->spell->GetSpellData()->tier);
	// Set the pets spawn type to 6
	pet->SetSpawnType(6);
	// Set the pets brain
	((NPC*)pet)->SetBrain(new CombatPetBrain((NPC*)pet));
	// Check to see if the pet has a subtitle
	if (strlen(pet->GetSubTitle()) > 0) {
		// Add the players name to the front of the sub title
		string pet_subtitle;
		pet_subtitle.append(spawn->GetName()).append("'s ").append(pet->GetSubTitle());
		LogWrite(PET__DEBUG, 0, "Pets", "Pet Subtitle: '%s'", pet_subtitle.c_str());
		// Set the pets subtitle to the new one
		pet->SetSubTitle(pet_subtitle.c_str());
	}
	// Add the "Pet Options" entity command to the pet
	pet->AddSecondaryEntityCommand("Pet Options", 10.0f, "petoptions", "", 0, 0);
	
	const char* spawn_script = world.GetSpawnScript(pet_id);
	bool runScript = false;
	if(spawn_script && lua_interface->GetSpawnScript(spawn_script) != 0){
		runScript = true;
		pet->SetSpawnScript(string(spawn_script));
		spawn->GetZone()->CallSpawnScript(pet, SPAWN_SCRIPT_PRESPAWN);
	}
	
	spawn->GetZone()->AddSpawn(pet);
	
	if(runScript){
		spawn->GetZone()->CallSpawnScript(pet, SPAWN_SCRIPT_SPAWN);
	}
	
	// Set the pet as the return value for this function
	lua_interface->SetSpawnValue(state, pet);
	return 1;
}

int EQ2Emu_lua_SummonDeityPet(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 pet_id = lua_interface->GetInt32Value(state, 2);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SummonDeityPet command error: Spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA SummonDeityPet command error: Spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (((Entity*)spawn)->GetDeityPet()) {
		if (spawn->IsPlayer()) {
			Client* client = ((Player*)spawn)->GetClient();
			if (client)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You already have a deity pet.");
		}

		lua_interface->LogError("%s: LUA SummonDeityPet command error: spawn already has a pet of this type", lua_interface->GetScriptName(state));
		return 0;
	}

	if (pet_id == 0) {
		lua_interface->LogError("%s: LUA SummonDeityPet command error: pet_id can not be set to 0", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!luaspell) {
		lua_interface->LogError("%s: LUA SummonDeityPet command error: valid spell not found, SummonDeityPet can only be used in spell scripts", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(luaspell->resisted) {
		return 0;
	}

	Spawn* pet = spawn->GetZone()->GetSpawn(pet_id);
	if (!pet) {
		lua_interface->LogError("%s: LUA SummonDeityPet command error: Could not find spawn with id of %u.", lua_interface->GetScriptName(state), pet_id);
		return 0;
	}

	if (!pet->IsNPC()) {
		lua_interface->LogError("%s: LUA SummonDeityPet command error: spawn with id of %u is not a npc", lua_interface->GetScriptName(state), pet_id);
		return 0;
	}

	pet->SetX(spawn->GetX());
	pet->SetY(spawn->GetY());
	pet->SetZ(spawn->GetZ());
	pet->SetLocation(spawn->GetLocation());
	pet->SetHeading(spawn->GetHeading());
	spawn->GetZone()->AddSpawn(pet);

	string random_pet_name;
	int16 rand_index = MakeRandomInt(0, spawn->GetZone()->pet_names.size() - 1);
	random_pet_name = spawn->GetZone()->pet_names.at(rand_index);
	LogWrite(PET__DEBUG, 0, "Pets", "Randomize Pet Name: '%s' (rand: %i)", random_pet_name.c_str(), rand_index);

	pet->SetName(random_pet_name.c_str());
	pet->SetLevel(spawn->GetLevel());
	pet->SetFactionID(spawn->GetFactionID());
	pet->SetPet(true);
	((NPC*)pet)->SetPetType(PET_TYPE_DEITY);
	((NPC*)pet)->SetOwner((Entity*)spawn);
	((Entity*)spawn)->SetDeityPet((Entity*)pet);
	pet->SetSpawnType(6);
	((NPC*)pet)->SetBrain(new NonCombatPetBrain((NPC*)pet));
	((NPC*)pet)->SetPetSpellID(luaspell->spell->GetSpellData()->id);
	((NPC*)pet)->SetPetSpellTier(luaspell->spell->GetSpellData()->tier);

	if (strlen(pet->GetSubTitle()) > 0) {
		string pet_subtitle;
		pet_subtitle.append(spawn->GetName()).append("'s ").append(pet->GetSubTitle());
		LogWrite(PET__DEBUG, 0, "Pets", "Pet Subtitle: '%s'", pet_subtitle.c_str());
		pet->SetSubTitle(pet_subtitle.c_str());
	}

	// deity and cosmetic pets are not attackable
	pet->SetAttackable(false);
	pet->AddSecondaryEntityCommand("Pet Options", 10.0f, "petoptions", "", 0, 0);
	lua_interface->SetSpawnValue(state, pet);
	return 1;
}

int EQ2Emu_lua_SummonCosmeticPet(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 pet_id = lua_interface->GetInt32Value(state, 2);
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SummonCosmeticPet command error: Spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA SummonCosmeticPet command error: Spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (((Entity*)spawn)->GetCosmeticPet()) {
		if (spawn->IsPlayer()) {
			Client* client = ((Player*)spawn)->GetClient();
			if (client)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You already have a cosmetic pet.");
		}

		lua_interface->LogError("%s: LUA SummonCosmeticPet command error: spawn already has a pet of this type", lua_interface->GetScriptName(state));
		return 0;
	}

	if (pet_id == 0) {
		lua_interface->LogError("%s: LUA SummonCosmeticPet command error: pet_id can not be set to 0", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!luaspell) {
		lua_interface->LogError("%s: LUA SummonCosmeticPet command error: valid spell not found, SummonCosmeticPet can only be used in spell scripts", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(luaspell->resisted) {
		return 0;
	}
	
	Spawn* pet = spawn->GetZone()->GetSpawn(pet_id);
	if (!pet) {
		lua_interface->LogError("%s: LUA SummonCosmeticPet command error: Could not find spawn with id of %u.", lua_interface->GetScriptName(state), pet_id);
		return 0;
	}

	if (!pet->IsNPC()) {
		lua_interface->LogError("%s: LUA SummonCosmeticPet command error: spawn with id of %u is not a npc", lua_interface->GetScriptName(state), pet_id);
		return 0;
	}

	pet->SetX(spawn->GetX());
	pet->SetY(spawn->GetY());
	pet->SetZ(spawn->GetZ());
	pet->SetLocation(spawn->GetLocation());
	pet->SetHeading(spawn->GetHeading());
	spawn->GetZone()->AddSpawn(pet);

	string random_pet_name;
	int16 rand_index = MakeRandomInt(0, spawn->GetZone()->pet_names.size() - 1);
	random_pet_name = spawn->GetZone()->pet_names.at(rand_index);
	LogWrite(PET__DEBUG, 0, "Pets", "Randomize Pet Name: '%s' (rand: %i)", random_pet_name.c_str(), rand_index);

	pet->SetName(random_pet_name.c_str());
	pet->SetLevel(spawn->GetLevel());
	pet->SetFactionID(spawn->GetFactionID());
	pet->SetPet(true);
	((NPC*)pet)->SetPetType(PET_TYPE_COSMETIC);
	((NPC*)pet)->SetOwner((Entity*)spawn);
	((Entity*)spawn)->SetCosmeticPet((Entity*)pet);
	pet->SetSpawnType(6);
	((NPC*)pet)->SetBrain(new NonCombatPetBrain((NPC*)pet));
	((NPC*)pet)->SetPetSpellID(luaspell->spell->GetSpellData()->id);
	((NPC*)pet)->SetPetSpellTier(luaspell->spell->GetSpellData()->tier);

	if (strlen(pet->GetSubTitle()) > 0) {
		string pet_subtitle;
		pet_subtitle.append(spawn->GetName()).append("'s ").append(pet->GetSubTitle());
		LogWrite(PET__DEBUG, 0, "Pets", "Pet Subtitle: '%s'", pet_subtitle.c_str());
		pet->SetSubTitle(pet_subtitle.c_str());
	}

	pet->SetAttackable(false);
	pet->AddSecondaryEntityCommand("Pet Options", 10.0f, "petoptions", "", 0, 0);
	lua_interface->SetSpawnValue(state, pet);
	return 1;
}

int EQ2Emu_lua_DismissPet(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA DismissPet command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsPet()) {
		lua_interface->LogError("%s: LUA DismissPet command error: spawn is not a pet", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!((NPC*)spawn)->IsDismissing() && ((NPC*)spawn)->GetOwner())
		((NPC*)spawn)->GetOwner()->DismissPet((NPC*)spawn, false, true);

	return 0;
}

int EQ2Emu_lua_SetQuestFeatherColor(lua_State* state) {
	if (!lua_interface)
		return 0;

	Quest* quest = lua_interface->GetQuest(state);

	if (!quest) {
		lua_interface->LogError("%s: LUA SetQuestFeatherColor command error: valid quest not found, SetQuestFeatherColor can only be called from a quest script", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	int8 feather_color = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (feather_color > 0)
		quest->SetFeatherColor(feather_color);

	return 0;
}

int EQ2Emu_lua_RemoveSpawnAccess(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* spawn2 = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA RemoveSpawnAccess command error: first spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn2) {
		lua_interface->LogError("%s: LUA RemoveSpawnAccess command error: second spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	spawn->RemoveSpawnAccess(spawn2);
	return 0;
}

int EQ2Emu_lua_SpawnByLocationID(lua_State* state) {
	if (!lua_interface)
		return 0;

	ZoneServer* zone = lua_interface->GetZone(state);
	int32 location_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!zone) {
		lua_interface->LogError("%s: LUA SpawnByLocationID command error: zone is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (location_id == 0) {
		lua_interface->LogError("%s: LUA SpawnByLocationID command error: location id can not be 0", lua_interface->GetScriptName(state));
		return 0;
	}

	SpawnLocation* location = zone->GetSpawnLocation(location_id);
	if (!location) {
		lua_interface->LogError("%s: LUA SpawnByLocationID command error: no location found for the given ID (%u)", lua_interface->GetScriptName(state), location_id);
		return 0;
	}
	
	int32 rand_number = MakeRandomInt(0, location->entities.size() - 1);

	Spawn* spawn = 0;
	if (location->entities[rand_number]) {
		if (location->entities[rand_number]->spawn_type == SPAWN_ENTRY_TYPE_NPC)
			spawn = zone->AddNPCSpawn(location, location->entities[rand_number]);
		else if (location->entities[rand_number]->spawn_type == SPAWN_ENTRY_TYPE_GROUNDSPAWN)
			spawn = zone->AddGroundSpawn(location, location->entities[rand_number]);
		else if (location->entities[rand_number]->spawn_type == SPAWN_ENTRY_TYPE_OBJECT)
			spawn = zone->AddObjectSpawn(location, location->entities[rand_number]);
		else if (location->entities[rand_number]->spawn_type == SPAWN_ENTRY_TYPE_WIDGET)
			spawn = zone->AddWidgetSpawn(location, location->entities[rand_number]);
		else if (location->entities[rand_number]->spawn_type == SPAWN_ENTRY_TYPE_SIGN)
			spawn = zone->AddSignSpawn(location, location->entities[rand_number]);

		if(spawn && spawn->IsOmittedByDBFlag())
		{
			LogWrite(SPAWN__WARNING, 0, "Spawn", "Spawn (%u) was skipped due to a missing expansion / holiday flag being met.", location->entities[0]->spawn_id);
			safe_delete(spawn);
			spawn = 0;
			return 0;
		}

		if (spawn) {
			const char* script = 0;
			for (int x = 0; x < 3; x++) {
				switch (x) {
				case 0:
					script = world.GetSpawnEntryScript(location->entities[0]->spawn_entry_id);
					break;
				case 1:
					script = world.GetSpawnLocationScript(location->entities[0]->spawn_location_id);
					break;
				case 2:
					script = world.GetSpawnScript(location->entities[0]->spawn_id);
					break;
				}
				if (script && lua_interface->GetSpawnScript(script) != 0) {
					spawn->SetSpawnScript(string(script));
					break;
				}
			}

			zone->CallSpawnScript(spawn, SPAWN_SCRIPT_SPAWN);
			lua_interface->SetSpawnValue(state, spawn);
			return 1;
		}
		else {
			LogWrite(ZONE__ERROR, 0, "Zone", "Error adding spawn by location id to zone %s with location id %u.", zone->GetZoneName(), location_id);
			safe_delete(spawn);
		}
	}

	return 0;
}

int EQ2Emu_lua_CastEntityCommand(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* caster = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	int32 id = lua_interface->GetInt32Value(state, 3);
	string command = lua_interface->GetStringValue(state, 4);
	lua_interface->ResetFunctionStack(state);

	if (!caster) {
		lua_interface->LogError("%s: LUA CastEntityCommand command error: caster is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target) {
		lua_interface->LogError("%s: LUA CastEntityCommand command error: target is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!caster->IsPlayer()) {
		lua_interface->LogError("%s: LUA CastEntityCommand command error: caster is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	EntityCommand* entity_command = caster->GetZone()->GetEntityCommand(id, command);
	if (!entity_command) {
		lua_interface->LogError("%s: LUA CastEntityCommand command error: unable to get a valid EntityCommand with the given ID (%u) and name (%s)", lua_interface->GetScriptName(state), id, command.c_str());
		return 0;
	}

	Client* client = ((Player*)caster)->GetClient();
	if (!client) {
		lua_interface->LogError("%s: LUA CastEntityCommand command error: unable to get a valid client for the given caster", lua_interface->GetScriptName(state));
		return 0;
	}

	client->GetCurrentZone()->ProcessEntityCommand(entity_command, (Player*)caster, target);

	return 0;
}

int EQ2Emu_lua_SetLuaBrain(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SetLuaBrain command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA SetLuaBrain command error: spawn is not a npc", lua_interface->GetScriptName(state));
		return 0;
	}

	((NPC*)spawn)->SetBrain(new LuaBrain((NPC*)spawn));

	return 0;
}

int EQ2Emu_lua_SetBrainTick(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int16 tick = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SetBrainTick command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA SetBrainTick command error: spawn is not a valid npc", lua_interface->GetScriptName(state));
		return 0;
	}

	if (tick < 20) {
		lua_interface->LogError("%s: LUA SetBrainTick command error: tick can not be set below 20 milliseconds", lua_interface->GetScriptName(state));
		return 0;
	}

	((NPC*)spawn)->Brain()->SetTick(tick);

	return 0;
}

int EQ2Emu_lua_SetFollowTarget(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	int32 follow_distance = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SetFollowTarget command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	// Target can be null, setting follow target to 0 clears it and will cancel follow, so no need to check it

	spawn->SetFollowTarget(target, follow_distance);
	return 0;
}

int EQ2Emu_lua_GetFollowTarget(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetFollowTarget command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	Spawn* target = spawn->GetFollowTarget();
	if (target) {
		lua_interface->SetSpawnValue(state, target);
		return 1;
	}

	return 0;
}

int EQ2Emu_lua_ToggleFollow(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA ToggleFollow command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spawn->following)
		spawn->following = false;
	else
		spawn->following = true;

	return 0;
}

int EQ2Emu_lua_IsFollowing(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA IsFollowing command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, spawn->following);
	return 1;
}

int EQ2Emu_lua_SetTempVariable(lua_State* state) {
	// As this is unique among the rest of our lua functions as the 3rd param can be of multiple types
	// I will attempt to explain how this function works for future refrence

	// Fist lets make sure lua_interface is valid, if not return out
	if (!lua_interface)
		return 0;

	// Next we grab the first 2 params same as we usually would
	Spawn* spawn = lua_interface->GetSpawn(state);
	string var = lua_interface->GetStringValue(state, 2);

	// DataType will let us know the value type so we can handle it correctly, we set these ourself so the values I used are made up
	// 1 = Spawn
	// 2 = Zone
	// 3 = Item
	// 4 = Quest
	// 5 = String
	// 6 = nil (null)
	int8 dataType = 0;

	// Define pointers for each potential type
	Spawn* spawnVal = 0;
	ZoneServer* zone = 0;
	Item* item = 0;
	Quest* quest = 0;
	string val;

	// Finally we get to grabbing the third param, we will first check to see if it is light user data
	// which is custom data types, in this case it can be Spawn, Zone, Item, or Quest.  Conversation and
	// options window are also light user data be we do not handle those.
	// We check with lua_islightuserdata(lua_State*, index)
	if (lua_islightuserdata(state, 3)) {
		// It is light user data so we will grab the param with lua_touserdata(lua_State*, index)
		// and convert it to LUAUserData*
		LUAUserData* data = (LUAUserData*)lua_touserdata(state, 3);
		// Check to make sure the data we got is valid, if not give an error
		if (!data || !data->IsCorrectlyInitialized()) {
			lua_interface->LogError("%s: LUA SetTempVariable command error while processing %s", lua_interface->GetScriptName(state), lua_tostring(state, -1));
		}
		// Check if data is a Spawn, if so set our Spawn pointer and the dataType variable
		else if (data->IsSpawn()) {
			spawnVal = data->spawn;
			dataType = 1;
		}
		// Check if data is a Zone, if so set our Zone pointer and the dataType variable
		else if (data->IsZone()) {
			zone = data->zone;
			dataType = 2;
		}
		// Check if data is a Item, if so set our Item pointer and the dataType variable
		else if (data->IsItem()) {
			item = data->item;
			dataType = 3;
		}
		// Check if data is a Ques, if so set our Quest pointer and the dataType variable
		else if (data->IsQuest()) {
			quest = data->quest;
			dataType = 4;
		}
	}
	// Wasn't light user data, check if it is nil(null)
	else if (lua_isnil(state, 3)) {
		// It is nil (null) set the dataType variable, no need to set a pointer in this case
		dataType = 6;
	}
	// Wasn't light user data or nil (null), must be a string
	else {
		// Set the string and dataType variable
		val = lua_interface->GetStringValue(state, 3);
		dataType = 5;
	}
	
	lua_interface->ResetFunctionStack(state);

	// We now have all the params, lets check to make sure they are valid
	if (!spawn) {
		lua_interface->LogError("%s: LUA SetTempVariable command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (var.length() == 0) {
		lua_interface->LogError("%s: LUA SetTempVariable command error: var must be set", lua_interface->GetScriptName(state));
		return 0;
	}

	if (dataType == 0) {
		lua_interface->LogError("%s: LUA SetTempVariable command error: unknown data type", lua_interface->GetScriptName(state));
		return 0;
	}

	// All params are valid, lets set the spawns temp variable, this is where dataType variable comes in.
	// AddTempVariable has overloads for all the types of data we support, we need to make sure the third
	// param gets sent to the correct list so we check the value of dataType to know where it should go.
	switch (dataType) {
	case 1:
		// 1 = Spawn
		spawn->AddTempVariable(var, spawnVal);
		break;
	case 2:
		// 2 = Zone
		spawn->AddTempVariable(var, zone);
		break;
	case 3:
		// 3 = Item
		spawn->AddTempVariable(var, item);
		break;
	case 4:
		// 4 = Quest
		spawn->AddTempVariable(var, quest);
		break;
	case 5:
		// 5 = String
		spawn->AddTempVariable(var, val);
		break;
	case 6:
		// 6 = nil (null) so the variable is no longer set, lets remove it from the spawn
		spawn->DeleteTempVariable(var);
		break;
	}

	// And we are done so return out
	return 0;
}

int EQ2Emu_lua_GetTempVariable(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string var = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA GetTempVariable command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (var.length() == 0) {
		lua_interface->LogError("%s: LUA GetTempVariable command error: var must be set", lua_interface->GetScriptName(state));
		return 0;
	}

	// This will tell us the type of data this variable contains, uses the same values as the previous function
	int8 type = spawn->GetTempVariableType(var);

	Spawn* spawn2 = 0;
	ZoneServer* zone = 0;
	Item* item = 0;
	Quest* quest = 0;

	// Set the lua function return value based on the type of data the variable contains
	switch (type) {
	case 1:
		spawn2 = spawn->GetTempVariableSpawn(var);
		if (!spawn2)
			return 0;
		lua_interface->SetSpawnValue(state, spawn2);
		break;
	case 2:
		zone = spawn->GetTempVariableZone(var);
		if (!zone)
			return 0;
		lua_interface->SetZoneValue(state, zone);
		break;
	case 3:
		item = spawn->GetTempVariableItem(var);
		if (!item)
			return 0;
		lua_interface->SetItemValue(state, item);
		break;
	case 4:
		quest = spawn->GetTempVariableQuest(var);
		if (!quest)
			return 0;
		lua_interface->SetQuestValue(state, quest);
		break;
	case 5:
		lua_interface->SetStringValue(state, spawn->GetTempVariable(var).c_str());
		break;
	default:
		// Not a valid type then the variable was not set so return out
		return 0;
	}

	// Return value was set so return out
	return 1;
}

int EQ2Emu_lua_GiveQuestItem(lua_State* state)
{
	if (!lua_interface)
		return 0;

	Quest* quest = lua_interface->GetQuest(state);
	Spawn* spawn = lua_interface->GetSpawn(state, 2);
	string description = lua_interface->GetStringValue(state, 3);
	int32 item_id = lua_interface->GetInt32Value(state, 4);

	if (!quest) {
		lua_interface->LogError("%s: LUA GiveQuestItem command error: quest is not valid", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	if (!spawn) {
		lua_interface->LogError("%s: LUA GiveQuestItem command error: spawn is not valid", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	if (!spawn->IsPlayer()) {
		lua_interface->LogError("%s: LUA GiveQuestItem command error: spawn must be a player", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	if (item_id == 0) {
		lua_interface->LogError("%s: LUA GiveQuestItem command error: item_id is not valid", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	Client* client = ((Player*)spawn)->GetClient();
	if (!client) {
		lua_interface->LogError("%s: LUA GiveQuestItem command error: unable to get a valid client from the given player spawn", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	Item* item = master_item_list.GetItem(item_id);
	if (!item) {
		lua_interface->LogError("%s: LUA GiveQuestItem command error: unable to get an item from the given id (%u)", lua_interface->GetScriptName(state), item_id);
		lua_interface->ResetFunctionStack(state);
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	
	Item* firstItem = new Item(item);
	quest->AddTmpRewardItem(firstItem);

	int8 num_args = (int8)lua_interface->GetNumberOfArgs(state);
	bool itemsAddedSuccessfully = true;
	if(num_args > 4)
	{
		for(int8 n=5;n<num_args+1;n++)
		{
			int32 new_item = lua_interface->GetInt32Value(state, n);
			Item* tmpItem = master_item_list.GetItem(new_item);
			if(tmpItem)
			{
				Item* newTmpItem = new Item(tmpItem);
				quest->AddTmpRewardItem(newTmpItem);
			}
			else
				itemsAddedSuccessfully = false;
		}
	}
	client->AddPendingQuestReward(quest, true, true, description); // queue for display
	
	lua_interface->ResetFunctionStack(state);
	
	lua_interface->SetBooleanValue(state, itemsAddedSuccessfully);
	return 1;
}

int EQ2Emu_lua_SetQuestRepeatable(lua_State* state) {
	if (!lua_interface)
		return 0;

	Quest* quest = lua_interface->GetQuest(state);
	lua_interface->ResetFunctionStack(state);
	if (!quest) {
		lua_interface->LogError("%s: LUA SetQuestRepeatable command error: quest is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	quest->SetRepeatable(true);
	return 0;
}

int EQ2Emu_lua_GetArchetypeName(lua_State* state) {

	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetArchetypeName command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	int8 base_class = classes.GetBaseClass(spawn->GetAdventureClass());
	string ret = classes.GetClassNameCase(base_class);
	if (ret.length() > 0) {
		lua_interface->SetStringValue(state, ret.c_str());
		return 1;
	}

	return 0;
}

int EQ2Emu_lua_SendWaypoints(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer()) {
		Client* client = player->GetClient();
		if (client)
			client->SendWaypoints();
	}
	return 0;
}

int EQ2Emu_lua_AddWaypoint(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	string name = lua_interface->GetStringValue(state, 2);	
	int32 type = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (type == 0)
		type = 2;
	if (name.length() > 0) {
		if (player && player->IsPlayer()) {
			Client* client = player->GetClient();
			if (client)
				client->AddWaypoint(name, type);
		}
	}
	return 0;
}

int EQ2Emu_lua_RemoveWaypoint(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	string name = lua_interface->GetStringValue(state, 2);	
	lua_interface->ResetFunctionStack(state);
	if (name.length() > 0) {
		if (player && player->IsPlayer()) {
			Client* client = player->GetClient();
			if (client)
				client->RemoveWaypoint(name);
		}
	}
	return 0;
}

int EQ2Emu_lua_AddWard(lua_State* state) {
	if (!lua_interface)
		return 0;

	int32 damage = lua_interface->GetInt32Value(state);
	bool keepWard = (lua_interface->GetInt8Value(state, 2) == 1);
	int8 wardType = lua_interface->GetInt8Value(state, 3);
	int8 damageTypes = lua_interface->GetInt8Value(state, 4);
	int32 damageAbsorptionPercent = lua_interface->GetInt32Value(state, 5);
	int32 damageAbsorptionMaxHealthPercent = lua_interface->GetInt32Value(state, 6);
	int32 redirectDamagePercent = lua_interface->GetInt32Value(state, 7);
	int32 maxHitCount = lua_interface->GetInt32Value(state, 8);

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	
	if(!spell || spell->resisted) {
		return 0;
	}
	
	bool ward_was_added = false;

	ZoneServer* zone = spell->caster->GetZone();
	spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
	for (int32 i = 0; i < spell->targets.size(); i++) {
		Spawn* target = zone->GetSpawnByID(spell->targets.at(i));
		if (!target)
			continue;
		if (target->IsEntity()) {
			// If the ward is already active remove it
			if (((Entity*)target)->GetWard(spell->spell->GetSpellID()))
				((Entity*)target)->RemoveWard(spell->spell->GetSpellID());

			// Create new ward info
			WardInfo* ward = new WardInfo;
			ward->Spell = spell;
			ward->BaseDamage = damage;
			ward->DamageLeft = damage;
			ward->AbsorbAllDamage = (damage == 0) ? true : false;

			ward->keepWard = keepWard;
			ward->WardType = wardType;
			if (damageAbsorptionPercent > 100)
				damageAbsorptionPercent = 100;

			ward->DamageAbsorptionPercentage = damageAbsorptionPercent;

			if (damageAbsorptionMaxHealthPercent > 100)
				damageAbsorptionMaxHealthPercent = 100;

			ward->DamageAbsorptionMaxHealthPercent = damageAbsorptionMaxHealthPercent;

			ward->RedirectDamagePercent = redirectDamagePercent;

			ward->LastRedirectDamage = 0;
			ward->LastAbsorbedDamage = 0;
			ward->HitCount = 0;

			spell->num_triggers = maxHitCount;
			spell->had_triggers = true;
			spell->cancel_after_all_triggers = false;
			ward->MaxHitCount = maxHitCount;
			ward->RoundTriggered = false;

			if (wardType == WARD_TYPE_MAGICAL)
				ward->DamageType = damageTypes;

			// Add the ward to the entity
			((Entity*)target)->AddWard(spell->spell->GetSpellID(), ward);
			ward_was_added = true;
		}
	}
	spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);

	if (ward_was_added && spell->caster->IsPlayer()) {
		spell->had_dmg_remaining = true;
		ClientPacketFunctions::SendMaintainedExamineUpdate(((Player*)spell->caster)->GetClient(), spell->slot_pos, damage, 1);
	}

	return 0;
}

int EQ2Emu_lua_AddToWard(lua_State* state) {
	if (!lua_interface)
		return 0;

	int32 amount = lua_interface->GetInt32Value(state);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	WardInfo* ward = 0;
	
	if(!spell || spell->resisted) {
		return 0;
	}
	ZoneServer* zone = spell->caster->GetZone();
	spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
	if (spell->targets.size() > 0 && zone->GetSpawnByID(spell->targets.at(0))->IsEntity()) {
		Entity* target = (Entity*)zone->GetSpawnByID(spell->targets.at(0));
		ward = target->GetWard(spell->spell->GetSpellID());
		if (target && ward) {
			ward->DamageLeft += amount;
			if (ward->DamageLeft > ward->BaseDamage)
				ward->DamageLeft = ward->BaseDamage;

			for (int32 i = 0; i < spell->targets.size(); i++) {
				if (Spawn* spawn = zone->GetSpawnByID(spell->targets.at(i)))
					zone->SendHealPacket(ward->Spell->caster, spawn, HEAL_PACKET_TYPE_REGEN_ABSORB, amount, ward->Spell->spell->GetName());
			}
		}
	}
	spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);

	if (ward && spell->caster->IsPlayer())
		ClientPacketFunctions::SendMaintainedExamineUpdate(((Player*)spell->caster)->GetClient(), spell->slot_pos, ward->DamageLeft, 1);

	return 0;
}

int EQ2Emu_lua_GetWardAmountLeft(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	if (!spell) {
		lua_interface->LogError("%s: LUA GetWardAmountLeft command error: this command can only be used in a spell script", lua_interface->GetScriptName(state));
		return 0;
	}

	spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
	if (spell->caster && spell->caster->GetZone() && spell->targets.size() > 0 && spell->caster->GetZone()->GetSpawnByID(spell->targets.at(0))->IsEntity()) {
		Entity* target = (Entity*)spell->caster->GetZone()->GetSpawnByID(spell->targets.at(0));
		WardInfo* ward = target->GetWard(spell->spell->GetSpellID());
		if (ward) {
			lua_interface->SetInt32Value(state, ward->DamageLeft);
			spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
			return 1;
		}
	}
	spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	return 0;
}

int EQ2Emu_lua_GetWardValue(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	if (!spell) {
		lua_interface->LogError("%s: LUA GetWardValue command error: this command can only be used in a spell script", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	string type = lua_interface->GetStringValue(state, 2);

	lua_interface->ResetFunctionStack(state);
	
	spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
	if (spell->caster && spell->caster->GetZone() && spell->targets.size() > 0 && spell->caster->GetZone()->GetSpawnByID(spell->targets.at(0))->IsEntity()) {

		Entity* target = (Entity*)spell->caster->GetZone()->GetSpawnByID(spell->targets.at(0));
		WardInfo* ward = target->GetWard(spell->spell->GetSpellID());
		if (ward) {
			if (boost::iequals(type, "damageleft"))
				lua_interface->SetInt32Value(state, ward->DamageLeft);
			else if (boost::iequals(type, "basedamage"))
				lua_interface->SetInt32Value(state, ward->BaseDamage);
			else if (boost::iequals(type, "keepward"))
				lua_interface->SetBooleanValue(state, ward->keepWard);
			else if (boost::iequals(type, "wardtype"))
				lua_interface->SetInt32Value(state, ward->WardType);
			else if (boost::iequals(type, "dmgabsorptionpct"))
				lua_interface->SetInt32Value(state, ward->DamageAbsorptionPercentage);
			else if (boost::iequals(type, "dmgabsorptionmaxhealthpct"))
				lua_interface->SetInt32Value(state, ward->DamageAbsorptionMaxHealthPercent);
			else if (boost::iequals(type, "redirectdamagepercent"))
				lua_interface->SetInt32Value(state, ward->RedirectDamagePercent);
			else if (boost::iequals(type, "lastredirectdamage"))
				lua_interface->SetInt32Value(state, ward->LastRedirectDamage);
			else if (boost::iequals(type, "lastabsorbeddamage"))
				lua_interface->SetInt32Value(state, ward->LastAbsorbedDamage);
			else if (boost::iequals(type, "hitcount"))
				lua_interface->SetInt32Value(state, ward->HitCount);
			else if (boost::iequals(type, "maxhitcount"))
				lua_interface->SetInt32Value(state, ward->MaxHitCount);
			else
				lua_interface->LogError("%s: LUA GetWardValue command argument type '%s' did not match any options", lua_interface->GetScriptName(state), type);
			
			spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
			return 1;
		}
	}
	spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	return 0;
}

int EQ2Emu_lua_RemoveWard(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	
	if(!spell) {
		return 0;
	}
	
	if(!spell->caster) {
		lua_interface->LogError("%s: RemoveWard error: no valid caster", lua_interface->GetScriptName(state));
		return 0;
	}
	
	ZoneServer* zone = spell->caster->GetZone();
	if(!zone) {
		lua_interface->LogError("%s: RemoveWard error: no valid zone for caster", lua_interface->GetScriptName(state));
		return 0;
	}
	
	Spawn* target = 0;
	spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
	for (int32 i = 0; i < spell->targets.size(); i++) {
		target = zone->GetSpawnByID(spell->targets.at(i));
		if (target && target->IsEntity()) {
			((Entity*)target)->RemoveWard(spell->spell->GetSpellID());
		}
	}
	spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);

	return 0;
}

int EQ2Emu_lua_Interrupt(lua_State* state)
{

	if (!lua_interface)
		return 0;

	Spawn* caster = lua_interface->GetSpawn(state); // Second param in lua_interface->get functions defaults to 1
	Spawn* target = lua_interface->GetSpawn(state, 2);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!caster)
	{
		lua_interface->LogError("%s: LUA Interrupt command error: caster is not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target)
	{
		lua_interface->LogError("%s: LUA Interrupt command error: target is not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spell) {
		lua_interface->LogError("%s: LUA Interrupt command error: spell is not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target->IsEntity() && !spell)
	{
		lua_interface->LogError("%s: LUA Interrupt command error: Target is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target && spell) {
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int8 i = 0; i < spell->targets.size(); i++) {
			target = caster->GetZone()->GetSpawnByID(spell->targets.at(i));
			if (!target || !target->IsEntity())
				continue;

			((Entity*)target)->GetZone()->GetSpellProcess()->Interrupted((Entity*)target, caster, SPELL_ERROR_INTERRUPTED);
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}
	else
		caster->GetZone()->GetSpellProcess()->Interrupted((Entity*)target, caster, SPELL_ERROR_INTERRUPTED);

	return 0;
}

int EQ2Emu_lua_Stealth(lua_State* state) {
	if (!lua_interface)
		return 0;

	int8 type = lua_interface->GetInt8Value(state);
	Spawn* spawn = lua_interface->GetSpawn(state, 2);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spell) {
		lua_interface->LogError("%s: LUA Stealth command error: must be used from spell script", lua_interface->GetScriptName(state));
		return 0;
	}

	ZoneServer* zone = spell->caster->GetZone();

	if (spawn) {
		if (spawn->IsEntity()) {
			if (type == 1) {
				((Entity*)spawn)->AddStealthSpell(spell);
				if (!(spell->effect_bitmask & EFFECT_FLAG_STEALTH))
					spell->effect_bitmask += EFFECT_FLAG_STEALTH;
			}
			else if (type == 2) {
				((Entity*)spawn)->AddInvisSpell(spell);
				if (!(spell->effect_bitmask & EFFECT_FLAG_INVIS))
					spell->effect_bitmask += EFFECT_FLAG_INVIS;
			}
			return 0;
		}
		else {
			lua_interface->LogError("%s: LUA Stealth command error: target override is not Entity", lua_interface->GetScriptName(state));
			return 0;
		}
	}
	else {
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int32 i = 0; i < spell->targets.size(); i++) {
			spawn = zone->GetSpawnByID(spell->targets.at(i));
			if (!spawn || !spawn->IsEntity())
				continue;

			if (type == 1) {
				((Entity*)spawn)->AddStealthSpell(spell);
				if (!(spell->effect_bitmask & EFFECT_FLAG_STEALTH))
					spell->effect_bitmask += EFFECT_FLAG_STEALTH;
			}
			else if (type == 2) {
				((Entity*)spawn)->AddInvisSpell(spell);
				if (!(spell->effect_bitmask & EFFECT_FLAG_INVIS))
					spell->effect_bitmask += EFFECT_FLAG_INVIS;
			}
			else {
				lua_interface->LogError("%s: LUA Stealth command error: invalid stealth type given", lua_interface->GetScriptName(state));
				break;
			}
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}
	return 0;
}

int EQ2Emu_lua_IsStealthed(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA IsStealthed command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spawn->IsEntity()) {
		lua_interface->SetBooleanValue(state, ((Entity*)spawn)->IsStealthed());
		return 1;
	}
	else
		lua_interface->LogError("%s: LUA IsStealthed command error: spawn is not entity", lua_interface->GetScriptName(state));
	return 0;
}

int EQ2Emu_lua_IsInvis(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA IsInvis command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spawn->IsEntity()) {
		lua_interface->SetBooleanValue(state, ((Entity*)spawn)->IsInvis());
		return 1;
	}
	else
		lua_interface->LogError("%s: LUA IsInvis command error: spawn is not entity", lua_interface->GetScriptName(state));
	return 0;
}

int EQ2Emu_lua_HasItemEquipped(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 item_id = lua_interface->GetInt32Value(state, 2);
	
	lua_interface->ResetFunctionStack(state);
	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA HasItemEquipped command error: spawn is not player", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetBooleanValue(state, ((Player*)player)->GetEquipmentList()->HasItem(item_id));
	return 1;
}

int	EQ2Emu_lua_GetEquippedItemBySlot(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 slot = lua_interface->GetInt8Value(state, 2);

	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetEquippedItemBySlot command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA GetEquippedItemBySlot command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	Item* item = ((Entity*)spawn)->GetEquipmentList()->GetItem(slot);
	if (!item) {
		lua_interface->LogError("%s: LUA GetEquippedItemBySlot command error: item was not found in slot", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetItemValue(state, item);
	return 1;
}

int	EQ2Emu_lua_GetEquippedItemByID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 id = lua_interface->GetInt32Value(state, 2);

	lua_interface->ResetFunctionStack(state);
	if (!player) {
		lua_interface->LogError("%s: LUA GetEquippedItemByID command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA GetEquippedItemByID command error: spawn is not player", lua_interface->GetScriptName(state));
		return 0;
	}
	Item* item = ((Player*)player)->GetEquipmentList()->GetItemFromItemID(id);
	if (!item) {
		lua_interface->LogError("%s: LUA GetEquippedItemByID command error: equipped item with used id not found", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetItemValue(state, item);
	return 1;
}

int	EQ2Emu_lua_SetEquippedItemByID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 slot = lua_interface->GetInt8Value(state, 2);
	int32 item_id = lua_interface->GetInt32Value(state, 3);

	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA SetEquippedItemByID command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA SetEquippedItemByID command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	
	Item* item = master_item_list.GetItem(item_id);
	if (!item) {
		lua_interface->LogError("%s: LUA SetEquippedItemByID command error: equipped item with used id %u not found", item_id, lua_interface->GetScriptName(state));
		return 0;
	}
	
	Item* copy = new Item(item);
	bool result = ((Entity*)spawn)->GetEquipmentList()->AddItem(slot, copy);

	if(result)
	{
		((Entity*)spawn)->SetEquipment(copy, slot);
		spawn->vis_changed = true;

		if(spawn->IsPlayer())
			((Player*)spawn)->GetClient()->SendEquipOrInvUpdateBySlot(slot);
	}
	else
	{
		safe_delete(copy);
	}
	

	lua_interface->SetBooleanValue(state, result);
	return 1;
}

int	EQ2Emu_lua_SetEquippedItem(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 slot = lua_interface->GetInt8Value(state, 2);
	Item* item = lua_interface->GetItem(state, 3);

	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA SetEquippedItem command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA SetEquippedItem command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!item) {
		lua_interface->LogError("%s: LUA SetEquippedItem command error: passed item not found", lua_interface->GetScriptName(state));
		return 0;
	}
	
	bool result = ((Entity*)spawn)->GetEquipmentList()->AddItem(slot, item);
	if(result)
	{
		((Entity*)spawn)->SetEquipment(item, slot);
		spawn->vis_changed = true;

		if(spawn->IsPlayer())
			((Player*)spawn)->GetClient()->SendEquipOrInvUpdateBySlot(slot);
	}
	lua_interface->SetBooleanValue(state, result);
	return 1;
}

int	EQ2Emu_lua_UnequipSlot(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 slot = lua_interface->GetInt8Value(state, 2);
	bool no_delete_item = (lua_interface->GetBooleanValue(state, 3) == false); // if not set then we default to deleting it, otherwise if set to true we don't delete

	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA UnequipSlot command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA UnequipSlot command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	if(slot >= NUM_SLOTS) {
		lua_interface->LogError("%s: LUA UnequipSlot command error: wrong slot id given %u, max %u", lua_interface->GetScriptName(state), slot, NUM_SLOTS);
		return 0;
	}
	
	if(spawn->IsPlayer() && ((Player*)spawn)->GetClient()) {
		Item* item = ((Player*)spawn)->GetEquipmentList()->GetItem(slot);
		if(item) {
			item->save_needed = true;
			if(no_delete_item) {
				database.DeleteItem(((Player*)spawn)->GetClient()->GetCharacterID(), item, 0);
				((Player*)spawn)->GetEquipmentList()->RemoveItem(slot, no_delete_item);
			}
			else{
				Client* client = ((Player*)spawn)->GetClient();
				client->UnequipItem(item->details.index);
			}
		}
	}
	else
		((Entity*)spawn)->GetEquipmentList()->RemoveItem(slot, no_delete_item);
	
	((Entity*)spawn)->SetEquipment(nullptr, slot);
	spawn->vis_changed = true;

	if(spawn->IsPlayer())
		((Player*)spawn)->GetClient()->SendEquipOrInvUpdateBySlot(slot);

	lua_interface->SetBooleanValue(state, true);
	return 1;
}

int	EQ2Emu_lua_SetEquipment(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 slot = lua_interface->GetInt8Value(state, 2);
	int16 type = lua_interface->GetInt16Value(state, 3);
	int8 r = lua_interface->GetInt8Value(state, 4);
	int8 g = lua_interface->GetInt8Value(state, 5);
	int8 b = lua_interface->GetInt8Value(state, 6);
	int8 h_r = lua_interface->GetInt8Value(state, 7);
	int8 h_g = lua_interface->GetInt8Value(state, 8);
	int8 h_b = lua_interface->GetInt8Value(state, 9);

	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA SetEquipment command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA SetEquipment command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	
	((Entity*)spawn)->SetEquipment(slot, type, r, g, b, h_r, h_g, h_b);
	spawn->vis_changed = true;
	lua_interface->SetBooleanValue(state, true);
	return 1;
}

int	EQ2Emu_lua_GetItemByID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 id = lua_interface->GetInt32Value(state, 2);
	int8 count = lua_interface->GetInt8Value(state, 3);
	bool include_bank = lua_interface->GetInt8Value(state, 4);

	lua_interface->ResetFunctionStack(state);
	if (!player) {
		lua_interface->LogError("%s: LUA GetItemByID command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA GetItemByID command error: spawn is not player", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!count)
		count = 1;
	Item* item = ((Player*)player)->GetPlayerItemList()->GetItemFromID(id, count, include_bank);
	if (!item) {
		lua_interface->LogError("%s: LUA GetItemByID command error: item with used id not found", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetItemValue(state, item);
	return 1;
}

int EQ2Emu_lua_PlayAnimation(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 anim = lua_interface->GetInt32Value(state, 2);
	Spawn* spawn2 = lua_interface->GetSpawn(state, 3);
	int8 type = lua_interface->GetInt8Value(state, 4);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA PlayAnimation command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spawn2) {
		if (spawn2->IsPlayer()) {
			if (type != 1 && type != 2)
				spawn->GetZone()->PlayAnimation(spawn, anim, spawn2);
			else
				spawn->GetZone()->PlayAnimation(spawn, anim, spawn2, type);
			return 0;
		}
		else {
			lua_interface->LogError("%s: LUA PlayAnimation command error: second spawn not a player", lua_interface->GetScriptName(state));
			return 0;
		}
	}
	else
		spawn->GetZone()->PlayAnimation(spawn, anim);
	return 0;
}

int EQ2Emu_lua_PlayAnimationString(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	std::string name = lua_interface->GetStringValue(state, 2);
	Spawn* opt_target = lua_interface->GetSpawn(state, 3);
	bool set_no_target = lua_interface->GetBooleanValue(state, 4);
	bool use_all_spelltargets = lua_interface->GetBooleanValue(state, 5);
	bool ignore_self = lua_interface->GetBooleanValue(state, 6);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	
	if (!spawn) {
		lua_interface->LogError("%s: LUA PlayAnimationString command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if (spell && spell->caster && spell->caster->GetZone() && use_all_spelltargets) {
		Spawn* target;
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int8 i = 0; i < spell->targets.size(); i++) {
			target = spell->caster->GetZone()->GetSpawnByID(spell->targets.at(i));
			if(target && (!ignore_self || spawn != target)) {
				spell->caster->GetZone()->HandleEmote(target, name, opt_target, set_no_target);
			}
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}
	else {
		spawn->GetZone()->HandleEmote(spawn, name, opt_target, set_no_target);
	}
	return 0;
}

int EQ2Emu_lua_IsPet(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA IsPet command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetBooleanValue(state, spawn->IsPet());
	return 1;
}

int EQ2Emu_lua_GetOwner(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetOwner command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA GetOwner command error: spawn is not a NPC", lua_interface->GetScriptName(state));
		return 0;
	}
	if (((NPC*)spawn)->GetOwner()) {
		lua_interface->SetSpawnValue(state, ((NPC*)spawn)->GetOwner());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetTarget(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA SetTarget command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn) {
		lua_interface->LogError("%s: LUA SetTarget command error: target is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	spawn->SetTarget(target);
	return 0;
}

int EQ2Emu_lua_SetInCombat(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	bool val = lua_interface->GetBooleanValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA SetInCombat command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA SetInCombat command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	((Entity*)spawn)->InCombat(val);

	if (val) {
		spawn->ClearRunningLocations();
		spawn->CalculateRunningLocation(true);
	}
	return 0;
}

int EQ2Emu_lua_CompareSpawns(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn1 = lua_interface->GetSpawn(state);
	Spawn* spawn2 = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!spawn1) {
		lua_interface->LogError("%s: LUA CompareSpawns command error: first spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn2) {
		lua_interface->LogError("%s: LUA CompareSpawns command error: second spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetBooleanValue(state, (spawn1 == spawn2));
	return 1;
}

int EQ2Emu_lua_ClearRunback(lua_State* state){
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA ClearRunback command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA ClearRunback command error: spawn is not an NPC", lua_interface->GetScriptName(state));
		return 0;
	}
	((NPC*)spawn)->ClearRunback();
	return 0;
}

int EQ2Emu_lua_Runback(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA Runback command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA Runback command error: spawn is not an NPC", lua_interface->GetScriptName(state));
		return 0;
	}
	((NPC*)spawn)->Runback();
	return 0;
}

int EQ2Emu_lua_GetRunbackDistance(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetRunbackDistance command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA GetRunbackDistance command error: spawn is not an NPC", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetFloatValue(state, ((NPC*)spawn)->GetRunbackDistance());
	return 1;
}

int EQ2Emu_lua_IsCasting(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA IsCasting command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA IsCasting command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetBooleanValue(state, ((Entity*)spawn)->IsCasting());
	return 1;
}

int EQ2Emu_lua_IsMezzed(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA IsMezzed command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA IsMezzed command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetBooleanValue(state, ((Entity*)spawn)->IsMezzed());
	return 1;
}

int EQ2Emu_lua_IsStunned(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA IsStunned command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA IsStunned command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetBooleanValue(state, ((Entity*)spawn)->IsStunned());
	return 1;
}

int EQ2Emu_lua_IsMezzedOrStunned(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA IsMezzedOrStunned command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA IsMezzedOrStunned command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetBooleanValue(state, ((Entity*)spawn)->IsMezzedOrStunned());
	return 1;
}

int EQ2Emu_lua_ProcessSpell(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	float distance = lua_interface->GetFloatValue(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA ProcessSpell command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!target) {
		lua_interface->LogError("%s: LUA ProcessSpell command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA ProcessSpell command error: spawn is not an NPC", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!target->IsEntity()) {
		lua_interface->LogError("%s: LUA ProcessSpell command error: target is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetBooleanValue(state, ((NPC*)spawn)->Brain()->ProcessSpell(((Entity*)target), distance));
	return 1;
}

int EQ2Emu_lua_ProcessMelee(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	float distance = lua_interface->GetFloatValue(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA ProcessMelee command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!target) {
		lua_interface->LogError("%s: LUA ProcessMelee command error: target is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA ProcessMelee command error: spawn is not an NPC", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!target->IsEntity()) {
		lua_interface->LogError("%s: LUA ProcessMelee command error: target is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	((NPC*)spawn)->Brain()->ProcessMelee(((Entity*)target), distance);
	return 0;
}

int EQ2Emu_lua_HasRecovered(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA HasRecovered command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA HasRecovered command error: spawn is not an NPC", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetBooleanValue(state, ((NPC*)spawn)->Brain()->HasRecovered());
	return 1;
}

int EQ2Emu_lua_GetEncounterSize(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetEncounterSize command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA GetEncounterSize command error: spawn is not an NPC", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetInt32Value(state, ((NPC*)spawn)->Brain()->GetEncounterSize());
	return 1;
}

int EQ2Emu_lua_GetMostHated(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetMostHated command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA GetMostHated command error: spawn is not an NPC", lua_interface->GetScriptName(state));
		return 0;
	}

	Entity* hated = ((NPC*)spawn)->Brain()->GetMostHated();
	if (hated) {
		lua_interface->SetSpawnValue(state, hated);
		return 1;
	}

	return 0;
}

int EQ2Emu_lua_ClearHate(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* hated = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA ClearHate command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA ClearHate command error: spawn is not NPC", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!hated) {
		((NPC*)spawn)->Brain()->ClearHate();
		return 0;
	}
	else
	{
		if (!hated->IsEntity()) {
			lua_interface->LogError("%s: LUA ClearHate command error: second param is not entity", lua_interface->GetScriptName(state));
			return 0;
		}
		((NPC*)spawn)->Brain()->ClearHate(((Entity*)hated));
		return 0;
	}
	return 0;
}

int EQ2Emu_lua_ClearEncounter(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA ClearEncounter command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA ClearEncounter command error: spawn is not an NPC", lua_interface->GetScriptName(state));
		return 0;
	}
	((NPC*)spawn)->Brain()->ClearEncounter();
	return 0;
}

int EQ2Emu_lua_GetEncounter(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetEncounter command error: Spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA GetEncounter command error: spawn is not a NPC", lua_interface->GetScriptName(state));
		return 0;
	}

	// Temp list to store hate list
	vector<int32>* encounterList = ((NPC*)spawn)->Brain()->GetEncounter();

	if (encounterList->size() == 0) {
		safe_delete(encounterList);
		return 0;
	}

	lua_createtable(state, encounterList->size(), 0);
	int newTable = lua_gettop(state);
	for (int32 i = 0; i < encounterList->size(); i++) {
		Spawn* temp = spawn->GetZone()->GetSpawnByID(encounterList->at(i));
		if (temp)
			lua_interface->SetSpawnValue(state, temp);
		lua_rawseti(state, newTable, i + 1);
	}

	safe_delete(encounterList);
	return 1;
}

int EQ2Emu_lua_GetHateList(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetHateList command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsNPC()) {
		lua_interface->LogError("%s: LUA GetHateList command error: spawn is not a NPC", lua_interface->GetScriptName(state));
		return 0;
	}

	// Temp list to store hate list
	vector<Entity*>* hateList = ((NPC*)spawn)->Brain()->GetHateList();

	if (hateList->size() == 0) {
		safe_delete(hateList);
		return 0;
	}

	lua_createtable(state, hateList->size(), 0);
	int newTable = lua_gettop(state);
	for (int32 i = 0; i < hateList->size(); i++) {
		lua_interface->SetSpawnValue(state, hateList->at(i));
		lua_rawseti(state, newTable, i + 1);
	}

	safe_delete(hateList);
	return 1;
}

int EQ2Emu_lua_HasGroup(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA HasGroup command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spawn->IsPlayer()) {
		if (((Player*)spawn)->GetGroupMemberInfo() && world.GetGroupManager()->GetGroupSize(((Player*)spawn)->GetGroupMemberInfo()->group_id) > 1)
			lua_interface->SetBooleanValue(state, true);
		else
			lua_interface->SetBooleanValue(state, false);

		return 1;
	}
	else {
		lua_interface->SetBooleanValue(state, spawn->HasSpawnGroup());
		return 1;
	}
}

int EQ2Emu_lua_SetCompleteFlag(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	lua_interface->ResetFunctionStack(state);

	if (!quest) {
		lua_interface->LogError("%s: LUA SetCompleteFlag command error: quest is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	quest->SetCompletedFlag(true);
	return 0;
}

int EQ2Emu_lua_HasSpellEffect(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 spellID = lua_interface->GetInt32Value(state, 2);
	int8 tier = lua_interface->GetInt8Value(state, 3);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA HasSpellEffect command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA HasSpellEffect command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spellID == 0) {
		lua_interface->LogError("%s: LUA HasSpellEffect command error: spell id is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	SpellEffects* effect = ((Entity*)spawn)->GetSpellEffect(spellID);
	if (effect) {
		if (tier > 0) {
			// If a tier was passed chec to see if it is the same as the effect
			if (tier == effect->tier)
				lua_interface->SetBooleanValue(state, true);
			else
				lua_interface->SetBooleanValue(state, false);

			return 1;
		}
		else {
			// Have an effect but no tier was passed so return true
			lua_interface->SetBooleanValue(state, true);
		}

		return 1;
	}

	// no effect so return false
	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_AddSpawnIDAccess(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 id = lua_interface->GetInt32Value(state, 2);
	ZoneServer* zone = lua_interface->GetZone(state, 3);
	lua_interface->ResetFunctionStack(state);
	Spawn* spawn2 = 0;
	vector<Spawn*> list;

	if (!spawn) {
		lua_interface->LogError("%s: LUA AddSpawnIDAccess command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	//If zone not provided, use spawn's zone
	if (!zone)
		zone = spawn->GetZone();

	list = zone->GetSpawnsByID(id);
	if (list.size() == 0) {
		lua_interface->LogError("%s: LUA AddSpawnIDAccess command error: GetSpawnsByID returned no spawns", lua_interface->GetScriptName(state));
		return 0;
	}

	vector<Spawn*>::iterator itr = list.begin();
	for (int8 i = 0; i < list.size(); i++) {
		spawn2 = itr[i];
		if (spawn2)
			spawn2->AddAllowAccessSpawn(spawn);
	}

	return 0;
}

int EQ2Emu_lua_RemoveSpawnIDAccess(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 id = lua_interface->GetInt32Value(state, 2);
	ZoneServer* zone = lua_interface->GetZone(state, 3);
	lua_interface->ResetFunctionStack(state);
	Spawn* spawn2 = 0;

	if (!spawn) {
		lua_interface->LogError("%s: LUA RemoveSpawnIDAccess command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	//If zone not provided, use spawn's zone
	if (!zone)
		zone = spawn->GetZone();

	vector<Spawn*> list = zone->GetSpawnsByID(id);
	vector<Spawn*>::iterator itr = list.begin();
	if (list.size() == 0) {
		lua_interface->LogError("%s: LUA RemoveSpawnIDAccess command error: GetSpawnsByID returned no spawns", lua_interface->GetScriptName(state));
		return 0;
	}
	for (int8 i = 0; i < list.size(); i++) {
		spawn2 = itr[i];
		if (spawn2)
			spawn2->RemoveSpawnAccess(spawn);
	}

	return 0;
}

int EQ2Emu_lua_SetQuestYellow(lua_State* state) {
	if (!lua_interface)
		return 0;
	Quest* quest = lua_interface->GetQuest(state);
	lua_interface->ResetFunctionStack(state);
	if (!quest) {
		lua_interface->LogError("%s: LUA SetQuestYellow command error: quest is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	quest->SetYellowName(true);
	return 0;
}

int EQ2Emu_lua_CanReceiveQuest(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA CanReceieveQuest command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsPlayer()) {
		lua_interface->LogError("%s: LUA CanReceieveQuest command error: spawn is not player", lua_interface->GetScriptName(state));
		return 0;
	}
	lua_interface->SetBooleanValue(state, ((Player*)spawn)->CanReceiveQuest(quest_id));
	return 1;
}

int EQ2Emu_lua_SetSuccessTimer(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SetSuccessTimer command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsPlayer()) {
		lua_interface->LogError("%s: LUA SetSuccessTimer command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	ZoneServer* zone = spawn->GetZone();
	if (!zone) {
		lua_interface->LogError("%s: LUA SetSuccessTimer command error: unable to get a valid zone for the given spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	Instance_Type iType = zone->GetInstanceType();
	if (iType == SOLO_LOCKOUT_INSTANCE ||
		iType == GROUP_LOCKOUT_INSTANCE ||
		iType == RAID_LOCKOUT_INSTANCE ||
		iType == SOLO_PERSIST_INSTANCE ||
		iType == GROUP_PERSIST_INSTANCE ||
		iType == RAID_PERSIST_INSTANCE) {
		InstanceData* data = ((Player*)spawn)->GetCharacterInstances()->FindInstanceByZoneID(spawn->GetZone()->GetZoneID());
		if (data) {
			// Check to see if the timer has already been set, if it has return out.
			if (Timer::GetUnixTimeStamp() <= data->last_success_timestamp + data->success_lockout_time)
				return 0;


			database.UpdateCharacterInstance(((Player*)spawn)->GetCharacterID(), string(spawn->GetZone()->GetZoneName()), spawn->GetZone()->GetInstanceID(), 1, Timer::GetUnixTimeStamp());
			data->last_success_timestamp = Timer::GetUnixTimeStamp();
			
			if(spawn->IsPlayer()) {
				Client* client = ((Player*)spawn)->GetClient();
				if (client) {
					string time_msg = "";
					int32 time = data->success_lockout_time;
					int16 hour;
					int8 min;
					int8 sec;
					hour = time / 3600;
					time = time % 3600;
					min = time / 60;
					time = time % 60;
					sec = time;

					if (hour > 0) {
						char temp[10];
						sprintf(temp, " %i", hour);
						time_msg.append(temp);
						time_msg.append(" hour");
						time_msg.append((hour > 1) ? "s" : "");
					}
					if (min > 0) {
						char temp[5];
						sprintf(temp, " %i", min);
						time_msg.append(temp);
						time_msg.append(" minute");
						time_msg.append((min > 1) ? "s" : "");
					}
					// Only add seconds if minutes and hours are 0
					if (hour == 0 && min == 0 && sec > 0) {
						char temp[5];
						sprintf(temp, " %i", sec);
						time_msg.append(temp);
						time_msg.append(" second");
						time_msg.append((sec > 1) ? "s" : "");
					}

					client->Message(CHANNEL_COLOR_YELLOW, "The success zone reuse timer for %s has been set.  You can return in%s.", data->zone_name.c_str(), time_msg.c_str());
				}
				else {
						lua_interface->LogError("LUA SetSuccessTimer command error: instance id %u client missing for player %s", spawn->GetZone()->GetInstanceID(), spawn->GetName());
				}
			}
		}
		else
			lua_interface->LogError("LUA SetSuccessTimer command error: unable to get instance data for instance %u for player %s", spawn->GetZone()->GetInstanceID(), spawn->GetName());
	}
	else
		lua_interface->LogError("%s: LUA SetSuccessTimer command error: current zone for given spawn is not a lockout or persistent instance", lua_interface->GetScriptName(state));


	return 0;
}

int EQ2Emu_lua_SetFailureTimer(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SetFailureTimer command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spawn->IsPlayer()) {
		lua_interface->LogError("%s: LUA SetFailureTimer command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	ZoneServer* zone = spawn->GetZone();
	if (!zone) {
		lua_interface->LogError("%s: LUA SetFailureTimer command error: unable to get a valid zone for the given spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	Instance_Type iType = zone->GetInstanceType();
	if (iType == SOLO_LOCKOUT_INSTANCE ||
		iType == GROUP_LOCKOUT_INSTANCE ||
		iType == RAID_LOCKOUT_INSTANCE ||
		iType == SOLO_PERSIST_INSTANCE ||
		iType == GROUP_PERSIST_INSTANCE ||
		iType == RAID_PERSIST_INSTANCE) {
		InstanceData* data = ((Player*)spawn)->GetCharacterInstances()->FindInstanceByZoneID(spawn->GetZone()->GetZoneID());
		if (data) {
			// Check to see if the timer has already been set, if it has return out.
			if (Timer::GetUnixTimeStamp() <= data->last_failure_timestamp + data->failure_lockout_time)
				return 0;

			database.UpdateCharacterInstance(((Player*)spawn)->GetCharacterID(), string(spawn->GetZone()->GetZoneName()), spawn->GetZone()->GetInstanceID(), 2, Timer::GetUnixTimeStamp());
			data->last_failure_timestamp = Timer::GetUnixTimeStamp();
			
			if(spawn->IsPlayer()) {
				Client* client = ((Player*)spawn)->GetClient();
				if (client) {
					string time_msg = "";
					int32 time = data->failure_lockout_time;
					int16 hour;
					int8 min;
					int8 sec;
					hour = time / 3600;
					time = time % 3600;
					min = time / 60;
					time = time % 60;
					sec = time;

					if (hour > 0) {
						char temp[10];
						sprintf(temp, " %i", hour);
						time_msg.append(temp);
						time_msg.append(" hour");
						time_msg.append((hour > 1) ? "s" : "");
					}
					if (min > 0) {
						char temp[5];
						sprintf(temp, " %i", min);
						time_msg.append(temp);
						time_msg.append(" minute");
						time_msg.append((min > 1) ? "s" : "");
					}
					// Only add seconds if minutes and hours are 0
					if (hour == 0 && min == 0 && sec > 0) {
						char temp[5];
						sprintf(temp, " %i", sec);
						time_msg.append(temp);
						time_msg.append(" second");
						time_msg.append((sec > 1) ? "s" : "");
					}

					client->Message(CHANNEL_COLOR_YELLOW, "The failure zone reuse timer for %s has been set. You can return in%s", data->zone_name.c_str(), time_msg.c_str());
				}
			}
			else {
					lua_interface->LogError("LUA SetFailureTimer command error: instance id %u client missing for player %s", spawn->GetZone()->GetInstanceID(), spawn->GetName());
			}
		}
		else
			lua_interface->LogError("LUA SetFailureTimer command error: unable to get instance data for instance %u for player %s", spawn->GetZone()->GetInstanceID(), spawn->GetName());
	}
	else
		lua_interface->LogError("%s: LUA SetFailureTimer command error: current zone for given spawn is not a lockout or persistent instance", lua_interface->GetScriptName(state));

	return 0;
}

int EQ2Emu_lua_IsGroundSpawn(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA IsGroundSpawn command error: not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, spawn->IsGroundSpawn());
	return 1;
}

int EQ2Emu_lua_CanHarvest(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	Spawn* ground = lua_interface->GetSpawn(state, 2);

	if (!player) {
		lua_interface->LogError("%s: LUA CanHarvest command error: not a valid spawn", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA CanHarvest command error: spawn is not a player", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	if (!ground) {
		lua_interface->LogError("%s: LUA CanHarvest command error: not a valid spawn", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	if (!ground->IsGroundSpawn()) {
		lua_interface->LogError("%s: LUA CanHarvest command error: spawn is not a ground spawn", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	vector<GroundSpawnEntry*>* groundspawn_entries = player->GetZone()->GetGroundSpawnEntries(((GroundSpawn*)ground)->GetGroundSpawnEntryID());

	if (!groundspawn_entries) {
		lua_interface->LogError("LUA CanHarvest command error: No groundspawn entries assigned to groundspawn id: %u", ((GroundSpawn*)ground)->GetGroundSpawnEntryID());
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	Skill* skill = 0;
	string collection_skill = string(((GroundSpawn*)ground)->GetCollectionSkill());
	if (collection_skill == "Collecting")
		skill = ((Player*)player)->GetSkillByName("Gathering");
	else
		skill = ((Player*)player)->GetSkillByName(collection_skill.c_str());

	if (!skill) {
		lua_interface->LogError("LUA CanHarvest command error: Player '%s' lacks the skill: '%s'", player->GetName(), collection_skill.c_str());
		lua_interface->ResetFunctionStack(state);
		return 0;
	}



	vector<GroundSpawnEntry*>::iterator itr;
	GroundSpawnEntry* entry = 0;
	bool can_harvest = false;
	sint32 min_skill = -1;
	int16 totalSkill = skill->current_val;
	int32 skillID = master_item_list.GetItemStatIDByName(collection_skill);
	if(skillID != 0xFFFFFFFF)
	{
		((Entity*)player)->MStats.lock();
		totalSkill += ((Entity*)player)->stats[skillID];
		((Entity*)player)->MStats.unlock();
	}
	
	// first, iterate through groundspawn_entries, discard tables player cannot use
	for (itr = groundspawn_entries->begin(); itr != groundspawn_entries->end(); itr++)
	{
		entry = *itr;

		if (min_skill == -1 || entry->min_skill_level < min_skill)
			min_skill = entry->min_skill_level;
		// if player lacks skill, skip table
		if (entry->min_skill_level > totalSkill)
			continue;
		// if bonus, but player lacks level, skip table
		if (entry->bonus_table && (player->GetLevel() < entry->min_adventure_level))
			continue;

		can_harvest = true;
		break;
	}

	lua_interface->SetBooleanValue(state, can_harvest);

	// If false, send the message to the client
	if (!can_harvest) {
		Client* client = ((Player*)player)->GetClient();
		if (client) {
			string msg = "You do not have enough skill to ";
			if (collection_skill == "Gathering" || collection_skill == "Collecting")
				msg.append("gather");
			else if (collection_skill == "Mining")
				msg.append("mine");
			else if (collection_skill == "Trapping")
				msg.append("trap");
			else if (collection_skill == "Foresting")
				msg.append("forest");
			else if (collection_skill == "Fishing")
				msg.append("catch");

			msg.append(" the %s. It requires %i %s skill, and your skill is %i.");
			client->Message(CHANNEL_HARVESTING_WARNINGS, msg.c_str(), ground->GetName(), min_skill, skill->name.data.c_str(), totalSkill);
			// You do not have enough skill to catch the band of fish.  It requires 20 Fishing skill, and your skill is 12.
		}
	}
	lua_interface->ResetFunctionStack(state);
	return 1;
}

int EQ2Emu_lua_HasRecipeBook(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int32 recipe_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA HasRecipeBook command error, Spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA HasRecipeBook command error, Spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	bool ret = ((Player*)player)->HasRecipeBook(recipe_id);
	lua_interface->SetBooleanValue(state, ret);
	return 1;
}

int EQ2Emu_lua_SummonDumbFirePet(lua_State* state) {
	// Check to see if we have a valid lua_interface
	if (!lua_interface)
		return 0;

	// Get the spawn that is getting the pet
	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	// Get the DB ID of the pet
	int32 pet_id = lua_interface->GetInt32Value(state, 3);

	float x = lua_interface->GetFloatValue(state, 4);
	float y = lua_interface->GetFloatValue(state, 5);
	float z = lua_interface->GetFloatValue(state, 6);
	// Get the spell that this command was called from
	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	// Check to make sure the spawn pointer is valid
	if (!spawn) {
		lua_interface->LogError("%s: LUA SummonDumbFirePet command error: Spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	// Check to make sure the spawn is an entity
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA SummonDumbFirePet command error: Spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target) {
		lua_interface->LogError("%s: LUA SummonDumbFirePet command error: target is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target->IsEntity()) {
		lua_interface->LogError("%s: LUA SummonDumbFirePet command error: target is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	// Check to see if the DB ID for the pet is set
	if (pet_id == 0) {
		lua_interface->LogError("%s: LUA SummonDumbFirePet command error: pet_id can not be set to 0", lua_interface->GetScriptName(state));
		return 0;
	}

	// Check to see if the pointer to the spell is valid
	if (!luaspell) {
		lua_interface->LogError("%s: LUA SummonDumbFirePet command error: valid spell not found, SummonPet can only be used in spell scripts", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(luaspell->resisted) {
		return 0;
	}

	// Get a pointer to a spawn with the given DB ID and check if the pointer is valid
	Spawn* pet = spawn->GetZone()->GetSpawn(pet_id);
	if (!pet) {
		lua_interface->LogError("LUA SummonDumbFirePet command error: Could not find spawn with id of %u.", pet_id);
		return 0;
	}

	// Check to make sure the pet is an npc
	if (!pet->IsNPC()) {
		lua_interface->LogError("LUA SummonDumbFirePet command error: id (%u) did not point to a npc", pet_id);
		return 0;
	}

	if (x == 0)
		x = spawn->GetX();

	if (y == 0)
		y = spawn->GetY();

	if (z == 0)
		z = spawn->GetZ();

	// Spawn the pet at the same location as the owner
	pet->SetX(x);
	pet->SetY(y);
	pet->SetZ(z);
	pet->SetLocation(spawn->GetLocation());
	pet->SetHeading(spawn->GetHeading());
	spawn->GetZone()->AddSpawn(pet);
	
	const char* spawn_script = world.GetSpawnScript(pet_id);
	if(spawn_script && lua_interface->GetSpawnScript(spawn_script) != 0){
		spawn->SetSpawnScript(string(spawn_script));
		spawn->GetZone()->CallSpawnScript(spawn, SPAWN_SCRIPT_SPAWN);
	}

	std::string petName = std::string("");
	if(spawn->IsEntity()) {
		petName = ((Entity*)spawn)->GetInfoStruct()->get_pet_name();
	}
	
	if(petName.size() < 1) {
		int16 rand_index = MakeRandomInt(0, spawn->GetZone()->pet_names.size() - 1);
		petName = spawn->GetZone()->pet_names.at(rand_index);
		LogWrite(PET__DEBUG, 0, "Pets", "Randomize Pet Name: '%s' (rand: %i)", petName.c_str(), rand_index);
	}
	
	// Set the pets name
	pet->SetName(petName.c_str());
		
	// Set the level of the pet to the owners level
	pet->SetLevel(spawn->GetLevel());
	// Set the faction of the pet to the same faction as the owner
	pet->SetFactionID(spawn->GetFactionID());
	// Set the spawn as a pet
	pet->SetPet(true);
	// Give a pointer of the owner to the pet
	((NPC*)pet)->SetOwner((Entity*)spawn);

	// Set the pet type
	((NPC*)pet)->SetPetType(PET_TYPE_DUMBFIRE);
	// Set the spell id used to create this pet
	((NPC*)pet)->SetPetSpellID(luaspell->spell->GetSpellData()->id);
	// Set the spell tier used to create this pet
	((NPC*)pet)->SetPetSpellTier(luaspell->spell->GetSpellData()->tier);
	// Set the pets spawn type to 6
	pet->SetSpawnType(6);
	// Set the pets brain
	((NPC*)pet)->SetBrain(new DumbFirePetBrain((NPC*)pet, (Entity*)target, luaspell->spell->GetSpellDuration() * 100));
	// Check to see if the pet has a subtitle
	if (strlen(pet->GetSubTitle()) > 0) {
		// Add the players name to the front of the sub title
		string pet_subtitle;
		pet_subtitle.append(spawn->GetName()).append("'s ").append(pet->GetSubTitle());
		LogWrite(PET__DEBUG, 0, "Pets", "Pet Subtitle: '%s'", pet_subtitle.c_str());
		// Set the pets subtitle to the new one
		pet->SetSubTitle(pet_subtitle.c_str());
	}

	// Set the pet as the return value for this function
	lua_interface->SetSpawnValue(state, pet);
	return 1;
}

int EQ2Emu_lua_SpawnMove(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* player = lua_interface->GetSpawn(state, 2);
	float max_distance = lua_interface->GetFloatValue(state, 3);
	string type = lua_interface->GetStringValue(state, 4);
	lua_interface->ResetFunctionStack(state);

	if (!spawn || (spawn && spawn->IsPlayer())) {
		lua_interface->LogError("%s: LUA SpawnMove command error: first param spawn is not valid or is player", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player || !player->IsPlayer()) {
		lua_interface->LogError("%s: LUA SpawnMove command error: second param is not player", lua_interface->GetScriptName(state));
		return 0;
	}

	Client* client = ((Player*)player)->GetClient();

	if (!client) {
		lua_interface->LogError("%s: LUA SpawnMove command error: could not find client", lua_interface->GetScriptName(state));
		return 0;
	}

	//Set max_distance to default if not set or not proper value
	if (max_distance <= 0)
		max_distance = 500;
	PacketStruct* packet = configReader.getStruct("WS_MoveObjectMode", client->GetVersion());
	if (packet) {
		float unknown2_3 = 0;
		int8 placement_mode = 0;
		if (type == "wall") {
			placement_mode = 2;
			unknown2_3 = 150;
		}
		else if (type == "ceiling")
			placement_mode = 1;
		packet->setDataByName("placement_mode", placement_mode);
		packet->setDataByName("spawn_id", ((Player*)player)->GetIDWithPlayerSpawn(spawn));
		packet->setDataByName("model_type", spawn->GetModelType());
		packet->setDataByName("unknown", 1); //size
		packet->setDataByName("unknown2", 1); //size 2
		packet->setDataByName("unknown2", .5, 1); //size 3
		packet->setDataByName("unknown2", 3, 2);
		packet->setDataByName("unknown2", unknown2_3, 3);
		packet->setDataByName("max_distance", max_distance);
		packet->setDataByName("CoEunknown", 0xFFFFFFFF);
		client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
	return 0;
}

int EQ2Emu_lua_GetItemType(lua_State* state) {
	if (!lua_interface)
		return 0;
	Item* item = lua_interface->GetItem(state);
	lua_interface->ResetFunctionStack(state);

	if (!item) {
		lua_interface->LogError("%s: LUA GetItemType command error: item pointer is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, item->generic_info.item_type);
	return 1;
}

int EQ2Emu_lua_GetItemEffectType(lua_State* state) {
	if (!lua_interface)
		return 0;
	Item* item = lua_interface->GetItem(state);
	lua_interface->ResetFunctionStack(state);

	if (!item) {
		lua_interface->LogError("%s: LUA GetItemEffectType command error: item pointer is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, item->effect_type);
	return 1;
}

int EQ2Emu_lua_AddTransportSpawn(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA AddTransportSpawn command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spawn->GetZone())
		spawn->GetZone()->AddTransportSpawn(spawn);

	return 0;
}

int EQ2Emu_lua_IsTransportSpawn(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA AddTransportSpawn command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, spawn->IsTransportSpawn());
	return 1;
}

int EQ2Emu_lua_GetSkillValue(lua_State* state) {
	if (!lua_interface)
		return 0;

	Skill* skill = lua_interface->GetSkill(state);
	lua_interface->ResetFunctionStack(state);
	if (!skill) {
		lua_interface->LogError("%s: LUA GetSkillValue command error: skill is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, skill->current_val);
	return 1;
}

int EQ2Emu_lua_GetSkillMaxValue(lua_State* state) {
	if (!lua_interface)
		return 0;

	Skill* skill = lua_interface->GetSkill(state);
	lua_interface->ResetFunctionStack(state);
	if (!skill) {
		lua_interface->LogError("%s: LUA GetSkillMaxValue command error: skill is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, skill->max_val);
	return 1;
}

int EQ2Emu_lua_GetSkillName(lua_State* state) {
	if (!lua_interface)
		return 0;

	Skill* skill = lua_interface->GetSkill(state);
	lua_interface->ResetFunctionStack(state);
	if (!skill) {
		lua_interface->LogError("%s: LUA GetSkillName command error: skill is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetStringValue(state, skill->name.data.c_str());
	return 1;
}

int EQ2Emu_lua_SetSkillMaxValue(lua_State* state) {
	if (!lua_interface)
		return 0;

	Skill* skill = lua_interface->GetSkill(state);
	int16 value = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!skill) {
		lua_interface->LogError("%s: LUA SetSkillMaxValue command error: skill is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	skill->max_val = value;
	if (skill->max_val < skill->current_val)
		skill->current_val = skill->max_val;

	return 0;
}

int EQ2Emu_lua_SetSkillValue(lua_State* state) {
	if (!lua_interface)
		return 0;

	Skill* skill = lua_interface->GetSkill(state);
	int16 value = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!skill) {
		lua_interface->LogError("%s: LUA SetSkillValue command error: skill is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (value > skill->max_val)
		skill->current_val = skill->max_val;
	else
		skill->current_val = value;

	return 0;
}

int EQ2Emu_lua_HasSkill(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 skill_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (skill_id > 0 && player && player->IsPlayer()) {
		lua_interface->SetBooleanValue(state, ((Player*)player)->skill_list.HasSkill(skill_id));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_AddSkill(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player_spawn = lua_interface->GetSpawn(state);
	int32 skill_id = lua_interface->GetInt32Value(state, 2);
	int16 current_val = lua_interface->GetInt16Value(state, 3);
	int16 max_val = lua_interface->GetInt16Value(state, 4);
	bool more_to_add = lua_interface->GetBooleanValue(state, 5);
	lua_interface->ResetFunctionStack(state);
	if (skill_id > 0 && current_val > 0 && max_val > 0) {
		if (player_spawn && player_spawn->IsPlayer()) {
			Player* player = (Player*)player_spawn;
			bool added = false;
			if (!player->skill_list.HasSkill(skill_id)) {
				player->AddSkill(skill_id, current_val, max_val, true);		
				added = true;
			}
			if (!more_to_add) { //need to send update regardless, even if THIS skill wasn't added, otherwise if you have a list and the last item wasn't added but the previous ones were, it wouldn't send the update
				Client* client = player->GetClient();
				if (client) {
					EQ2Packet* packet = player->GetSkills()->GetSkillPacket(client->GetVersion());
					if (packet)
						client->QueuePacket(packet);
				}
			}
			if (added) {
				lua_interface->SetBooleanValue(state, true);
				return 1;
			}
		}
		else {
			lua_interface->LogError("%s: LUA AddSkill command error: Given spawn is not a player", lua_interface->GetScriptName(state));
		}
	}
	else {
		lua_interface->LogError("%s: LUA AddSkill command error: Required parameters not set", lua_interface->GetScriptName(state));
	}
	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_RemoveSkill(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player_spawn = lua_interface->GetSpawn(state);
	int32 skill_id = lua_interface->GetInt32Value(state, 2);
	bool more_to_remove = lua_interface->GetBooleanValue(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (skill_id > 0) {
		if (player_spawn && player_spawn->IsPlayer()) {
			Player* player = (Player*)player_spawn;
			if (player->skill_list.HasSkill(skill_id)) {
				player->RemovePlayerSkill(skill_id, true);
				if (!more_to_remove) {
					Client* client = player->GetClient();
					if (client) {
						EQ2Packet* packet = player->GetSkills()->GetSkillPacket(client->GetVersion());
						if (packet)
							client->QueuePacket(packet);
					}
				}
			}			
		}
		else {
			lua_interface->LogError("%s: LUA RemoveSkill command error: Given spawn is not a player", lua_interface->GetScriptName(state));
		}
	}
	else {
		lua_interface->LogError("%s: LUA RemoveSkill command error: skill_id not set", lua_interface->GetScriptName(state));
	}
	return 0;
}

int EQ2Emu_lua_IncreaseSkillCapsByType(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player_spawn = lua_interface->GetSpawn(state);
	int8 skill_type = lua_interface->GetInt8Value(state, 2);
	int16 amount = lua_interface->GetInt8Value(state, 3);
	bool more_to_increase = lua_interface->GetBooleanValue(state, 4);
	lua_interface->ResetFunctionStack(state);
	if (amount > 0 && skill_type < 100) {
		if (player_spawn && player_spawn->IsPlayer()) {
			Player* player = (Player*)player_spawn;
			player->skill_list.IncreaseSkillCapsByType(skill_type, amount);
			if (!more_to_increase) {
				Client* client = player->GetClient();
				if (client) {
					EQ2Packet* packet = player->GetSkills()->GetSkillPacket(client->GetVersion());
					if (packet)
						client->QueuePacket(packet);
				}
			}
		}
		else {
			lua_interface->LogError("%s: LUA IncreaseSkillCapsByType command error: Given spawn is not a player", lua_interface->GetScriptName(state));
		}
	}
	else {
		lua_interface->LogError("%s: LUA IncreaseSkillCapsByType command error: Invalid parameters", lua_interface->GetScriptName(state));
	}
	return 0;
}

int EQ2Emu_lua_GetSkill(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string name = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA GetSkill command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA GetSkill command error: spawn is not a valid entity", lua_interface->GetScriptName(state));
		return 0;
	}

	Skill* skill = ((Entity*)spawn)->GetSkillByName(name.c_str());
	if (skill) {
		lua_interface->SetSkillValue(state, skill);
		return 1;
	}

	return 0;
}

int EQ2Emu_lua_AddProc(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 type = lua_interface->GetInt8Value(state, 2);
	float chance = lua_interface->GetFloatValue(state, 3);
	Item* item = lua_interface->GetItem(state, 4);
	bool use_all_spelltargets = (lua_interface->GetInt8Value(state, 5) == 1);
	LuaSpell* spell = 0;

	if (!item)
		spell = lua_interface->GetCurrentSpell(state);
	
	if (!spawn && (!spell || !use_all_spelltargets)) {
		lua_interface->LogError("%s: LUA AddProc command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if ((!spell || use_all_spelltargets) && spawn && !spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA AddProc command error: spawn is not a valid entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!item && !spell) {
		lua_interface->LogError("%s: LUA AddProc command error: can only use with an item provided or inside a spell script", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(spell && spell->resisted) {
		return 0;
	}
	
	if (spell && spell->caster && spell->caster->GetZone() && use_all_spelltargets) {
		Spawn* target;
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int8 i = 0; i < spell->targets.size(); i++) {
			target = spell->caster->GetZone()->GetSpawnByID(spell->targets.at(i));
			if (!target || !target->IsEntity())
				continue;

			((Entity*)target)->AddProc(type, chance, item, spell);
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}
	else
		((Entity*)spawn)->AddProc(type, chance, item, spell);

	return 0;
}

int EQ2Emu_lua_AddProcExt(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 type = lua_interface->GetInt8Value(state, 2);
	int8 damage_type = lua_interface->GetInt8Value(state, 3);
	float chance = lua_interface->GetFloatValue(state, 4);
	int8 hp_ratio = lua_interface->GetInt8Value(state, 5);
	bool below_health = lua_interface->GetBooleanValue(state, 6);
	bool target_health = lua_interface->GetBooleanValue(state, 7);
	Item* item = lua_interface->GetItem(state, 8);
	bool use_all_spelltargets = (lua_interface->GetInt8Value(state, 9) == 1);
	bool extended_version = true;
	LuaSpell* spell = 0;

	if (!item)
		spell = lua_interface->GetCurrentSpell(state);

	if (!spawn && (!spell || !use_all_spelltargets)) {
		lua_interface->LogError("%s: LUA AddProc command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if ((!spell || use_all_spelltargets) && spawn && !spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA AddProc command error: spawn is not a valid entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!item && !spell) {
		lua_interface->LogError("%s: LUA AddProc command error: can only use with an item provided or inside a spell script", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(spell && spell->resisted) {
		return 0;
	}
	
	if (spell && spell->caster && spell->caster->GetZone() && use_all_spelltargets) {
		Spawn* target;
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int8 i = 0; i < spell->targets.size(); i++) {
			target = spell->caster->GetZone()->GetSpawnByID(spell->targets.at(i));
			if (!target || !target->IsEntity())
				continue;

			((Entity*)target)->AddProc(type, chance, item, spell, damage_type, hp_ratio, below_health, target_health, extended_version);
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}
	else
		((Entity*)spawn)->AddProc(type, chance, item, spell);

	return 0;
}

int EQ2Emu_lua_RemoveProc(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	Item* item = lua_interface->GetItem(state, 2);
	LuaSpell* spell = 0;

	if (!item)
		spell = lua_interface->GetCurrentSpell(state);
	
	lua_interface->ResetFunctionStack(state);

	if (!item && !spell) {
		lua_interface->LogError("%s: LUA RemoveProc command error: can only use with an item provided or inside a spell script", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(spawn && spawn->IsEntity()) {
		((Entity*)spawn)->RemoveProc(item, spell);
	}
	else if (spell && spell->caster && spell->caster->GetZone()) {
		Spawn* target;
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int8 i = 0; i < spell->targets.size(); i++) {
			target = spell->caster->GetZone()->GetSpawnByID(spell->targets.at(i));
			if (!target || !target->IsEntity())
				continue;

			((Entity*)target)->RemoveProc(item, spell);
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
		spell->caster->RemoveProc(item, spell);
	}
	else if (!spawn) {
		lua_interface->LogError("%s: LUA RemoveProc command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	else if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA RemoveProc command error: spawn is not a valid entity", lua_interface->GetScriptName(state));
		return 0;
	}
	return 0;
}

int EQ2Emu_lua_Knockback(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* target_spawn = lua_interface->GetSpawn(state);
	Spawn* spawn = lua_interface->GetSpawn(state, 2);
	int32 duration = lua_interface->GetInt32Value(state, 3);
	float vertical = lua_interface->GetFloatValue(state, 4);
	float horizontal = lua_interface->GetFloatValue(state, 5);
	bool use_heading = lua_interface->GetInt8Value(state, 6) == 1 ? true : false;
	lua_interface->ResetFunctionStack(state);

	if (!target_spawn) {
		lua_interface->LogError("%s: LUA Knockback command error: target_spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn) {
		lua_interface->LogError("%s: LUA Knockback command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if ((vertical != 0 || horizontal != 0)) {
		if(spawn->IsPlayer()) {
			Client* client = ((Player*)spawn)->GetClient();
			PacketStruct* packet = configReader.getStruct("WS_PlayerKnockback", client->GetVersion());
			if (packet) {
				packet->setDataByName("target_x", target_spawn->GetX());
				packet->setDataByName("target_y", target_spawn->GetY());
				packet->setDataByName("target_z", target_spawn->GetZ());
				packet->setDataByName("vertical_movement", vertical);
				packet->setDataByName("horizontal_movement", horizontal);
				if (use_heading)
					packet->setDataByName("use_player_heading", 1);

				client->QueuePacket(packet->serialize());
			}
			safe_delete(packet);
		}
		else {
			spawn->SetKnockback(target_spawn, duration, vertical, horizontal);
		}
	}

	return 0;
}

int EQ2Emu_lua_IsEpic(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA IsEpic command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, (spawn->GetHeroic() >= 2));
	return 1;
}

int EQ2Emu_lua_ProcDamage(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* caster = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	string name = lua_interface->GetStringValue(state, 3);
	int8 dmg_type = lua_interface->GetInt8Value(state, 4);
	int32 low_damage = lua_interface->GetInt32Value(state, 5);
	int32 high_damage = lua_interface->GetInt32Value(state, 6);
	string success_msg = lua_interface->GetStringValue(state, 7);
	string effect_msg = lua_interface->GetStringValue(state, 8);
	lua_interface->ResetFunctionStack(state);

	if (!caster) {
		lua_interface->LogError("%s: LUA ProcDamage command error: caster is not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!caster->IsEntity()) {
		lua_interface->LogError("%s: LUA ProcDamage command error: caster is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target) {
		lua_interface->LogError("%s: LUA ProcDamage command error: target is not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target->IsEntity()) {
		lua_interface->LogError("%s: LUA ProcDamage command error: target is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (name.length() == 0) {
		lua_interface->LogError("%s: LUA ProcDamage command error: name is empty", lua_interface->GetScriptName(state));
		return 0;
	}

	((Entity*)caster)->ProcAttack(target, dmg_type, low_damage, high_damage, name, success_msg, effect_msg);
	return 0;
}

int EQ2Emu_lua_GetSkillIDByName(lua_State* state) {
	if (!lua_interface)
		return 0;

	string name = lua_interface->GetStringValue(state);
	lua_interface->ResetFunctionStack(state);

	if (name.length() == 0) {
		lua_interface->LogError("%s: LUA GetSkillIDByName command error: name param was not set", lua_interface->GetScriptName(state));
		return 0;
	}

	Skill* skill = master_skill_list.GetSkillByName(name.c_str());
	if (!skill) {
		lua_interface->LogError("LUA GetSkillIDByName command error: skill with name of %s not found", name.c_str());
		return 0;
	}

	lua_interface->SetInt32Value(state, skill->skill_id);
	return 1;
}

int EQ2Emu_lua_IsHeroic(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA IsHeroic command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, spawn->GetHeroic() == 1);
	return 1;
}

int EQ2Emu_lua_LastSpellAttackHit(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!luaspell) {
		lua_interface->LogError("%s: LUA LastSpellAttackHit command error: this must be called from a spellscript", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, luaspell->last_spellattack_hit);
	return 1;
}
int EQ2Emu_lua_IsBehind(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA IsBehind command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA IsBehind command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target) {
		lua_interface->LogError("%s: LUA IsBehind command error: target is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, ((Entity*)spawn)->BehindTarget(target));
	return 1;
}

int EQ2Emu_lua_IsFlanking(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA IsFlanking command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA IsFlanking command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target) {
		lua_interface->LogError("%s: LUA IsFlanking command error: target is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, ((Entity*)spawn)->FlankingTarget(target));
	return 1;
}

int EQ2Emu_lua_InFront(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA InFrontSpawn command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target) {
		lua_interface->LogError("%s: LUA InFrontSpawn command error: target is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, spawn->InFrontSpawn(target, spawn->GetX(), spawn->GetZ()));
	return 1;
}

int EQ2Emu_lua_GetItemCount(lua_State* state) {
	if (!lua_interface)
		return 0;

	Item* item = lua_interface->GetItem(state);
	lua_interface->ResetFunctionStack(state);

	if (!item) {
		lua_interface->LogError("%s: LUA GetItemCount command error: item not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, item->details.count);
	return 1;
}

int EQ2Emu_lua_SetItemCount(lua_State* state) {
	if (!lua_interface)
		return 0;

	Item* item = lua_interface->GetItem(state);
	Spawn* owner = lua_interface->GetSpawn(state, 2);
	int16 new_count = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);

	if (!item) {
		lua_interface->LogError("%s: LUA SetItemCount command error: item not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!owner) {
		lua_interface->LogError("%s: LUA SetItemCount command error: spawn not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!owner->IsPlayer()) {
		lua_interface->LogError("%s: LUA SetItemCount command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	if (item->stack_count < new_count) {
		lua_interface->LogError("%s: LUA SetItemCount command error: new item count cannot be more than max stack count", lua_interface->GetScriptName(state));
		return 0;
	}

	if (new_count > 0) {
		item->details.count = new_count;
		item->save_needed = true;
	}
	else if (((Player*)owner)->GetEquipmentList()->GetItem(item->details.slot_id) == item)
		((Player*)owner)->GetEquipmentList()->RemoveItem(item->details.slot_id, true);
	else if (((Player*)owner)->GetPlayerItemList()->GetItemFromUniqueID(item->details.unique_id) == item)
		((Player*)owner)->GetPlayerItemList()->RemoveItem(item, true);
	else
	{
		lua_interface->LogError("%s: LUA SetItemCount command error: could not remove item from player", lua_interface->GetScriptName(state));
		return 0;
	}

	Client* client = ((Player*)owner)->GetClient();

	if (!client)
		return 0;

	((Player*)owner)->SendInventoryUpdate(client->GetVersion());

	EQ2Packet* app = ((Player*)owner)->GetEquipmentList()->serialize(client->GetVersion(), client->GetPlayer());
	if (app)
		client->QueuePacket(app);

	return 0;
}

int EQ2Emu_lua_AddSpellTimer(lua_State* state) {
	if (!lua_interface)
		return 0;

	int32 time = lua_interface->GetInt32Value(state);
	string function = lua_interface->GetStringValue(state, 2);
	Spawn* caster = lua_interface->GetSpawn(state, 3);
	Spawn* target = lua_interface->GetSpawn(state, 4);

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (time == 0) {
		lua_interface->LogError("%s: LUA AddSpellTimer command error: time must be set", lua_interface->GetScriptName(state));
		return 0;
	}

	if (function.length() == 0) {
		lua_interface->LogError("%s: LUA AddSpellTimer command error: function name must be set", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spell || (!spell->caster || !spell->caster->GetZone())) {
		lua_interface->LogError("%s: LUA AddSpellTimer command error: spell not found, AddSpellTimer must be used in a spell script", lua_interface->GetScriptName(state));
		return 0;
	}

	SpellScriptTimer* timer = new SpellScriptTimer;

	/* //Google tells me memsetting a string is bad, manually setting just in case - Foof
	#ifdef WIN32
		ZeroMemory(timer, sizeof(SpellScriptTimer));
	#else
		bzero(timer, sizeof(SpellScriptTimer));
	#endif*/

	timer->caster = 0;
	timer->deleteWhenDone = false;
	timer->target = 0;

	timer->time = Timer::GetCurrentTime2() + time;
	timer->customFunction = function;
	timer->spell = spell;
	if (caster)
		timer->caster = caster->GetID();
	if (target)
		timer->target = target->GetID();

	spell->caster->GetZone()->GetSpellProcess()->AddSpellScriptTimer(timer);
	return 0;
}

int EQ2Emu_lua_Resurrect(lua_State* state) {
	if (!lua_interface)
		return 0;

	float hp_perc = lua_interface->GetFloatValue(state);
	float power_perc = lua_interface->GetFloatValue(state, 2);
	bool send_window = lua_interface->GetInt32Value(state, 3) == 1;
	Spawn* target = lua_interface->GetSpawn(state, 4);
	string heal_name = lua_interface->GetStringValue(state, 5);
	int8 crit_mod = lua_interface->GetInt32Value(state, 6);
	bool no_calcs = lua_interface->GetInt32Value(state, 7) == 1;
	int32 revive_spell_id = lua_interface->GetInt32Value(state, 8);
	int8 revive_spell_tier = lua_interface->GetInt32Value(state, 9);

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	if (!spell) {
		lua_interface->LogError("%s: LUA command error: this function must be used in a spellscript", lua_interface->GetScriptName(state));
		return 0;
	}

	Entity* caster = spell->caster;
	if (!caster) {
		lua_interface->LogError("%s: LUA command error: could not find caster", lua_interface->GetScriptName(state));
		return 0;
	}

	Client* client = 0;
	PendingResurrection* rez = 0;
	ZoneServer* zone = spell->caster->GetZone();
	if (!target) {
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		if (spell->targets.size() > 0) {
			vector<int32> spell_targets = spell->targets;
			for (int8 i = 0; i < spell_targets.size(); i++) {
				target = zone->GetSpawnByID(spell_targets.at(i));
				if (!target)
					continue;
				if (!target->IsPlayer())
					continue;

				client = ((Player*)target)->GetClient();

				if (!client)
					continue;

				rez = client->GetCurrentRez();
				if (rez->active)
					continue;

				client->GetResurrectMutex()->writelock(__FUNCTION__, __LINE__);
				rez->active = true;
				rez->caster = caster->GetID();
				rez->expire_timer = new Timer;
				int32 duration = spell->spell->GetSpellDuration();
				rez->expire_timer->Start(duration * 100);
				rez->hp_perc = hp_perc;
				rez->mp_perc = power_perc;
				rez->range = spell->spell->GetSpellData()->range;
				rez->spell_name = spell->spell->GetName();
				if (heal_name.length() > 0)
					rez->heal_name = heal_name;
				else
					rez->heal_name = rez->spell_name;
				rez->orig_spell_id = spell->spell->GetSpellID();
				rez->orig_spell_tier = spell->spell->GetSpellTier();
				rez->revive_sickness_spell_id = revive_spell_id;
				rez->revive_sickness_spell_tier = revive_spell_tier;
				rez->no_calcs = no_calcs;
				rez->crit_mod = crit_mod;
				rez->spell_visual = spell->spell->GetSpellData()->spell_visual;
				if (send_window)
					client->SendResurrectionWindow();
				else {
					target->GetZone()->ResurrectSpawn(target, client);
					rez->should_delete = true;
				}
				client->GetResurrectMutex()->releasewritelock(__FUNCTION__, __LINE__);
			}
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}
	else {
		if(!target->IsPlayer())
			return 0;
		
		client = ((Player*)target)->GetClient();

		if (!client)
			return 0;

		rez = client->GetCurrentRez();
		if (rez->active)
			return 0;

		client->GetResurrectMutex()->writelock(__FUNCTION__, __LINE__);
		rez->active = true;
		rez->caster = caster->GetID();
		rez->expire_timer = new Timer;
		int32 duration = spell->spell->GetSpellDuration();
		rez->expire_timer->Start(duration * 100);
		rez->hp_perc = hp_perc;
		rez->mp_perc = power_perc;
		rez->range = spell->spell->GetSpellData()->range;
		rez->spell_name = spell->spell->GetName();
		if (heal_name.length() > 0)
			rez->heal_name = heal_name;
		else
			rez->heal_name = rez->spell_name;
		
		rez->orig_spell_id = spell->spell->GetSpellID();
		rez->orig_spell_tier = spell->spell->GetSpellTier();
		rez->revive_sickness_spell_id = revive_spell_id;
		rez->revive_sickness_spell_tier = revive_spell_tier;
		rez->no_calcs = no_calcs;
		rez->crit_mod = crit_mod;
		rez->spell_visual = spell->spell->GetSpellData()->spell_visual;
		if (send_window)
			client->SendResurrectionWindow();
		else {
			target->GetZone()->ResurrectSpawn(target, client);
			rez->should_delete = true;
		}
		client->GetResurrectMutex()->releasewritelock(__FUNCTION__, __LINE__);
	}
	return 0;
}

int EQ2Emu_lua_SetVision(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 vision = lua_interface->GetInt32Value(state, 2);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SetVision command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA SetVision command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spell && spell->targets.size() > 0) {
		ZoneServer* zone = spell->caster->GetZone();
		for (int8 i = 0; i < spell->targets.size(); i++) {
			Spawn* target = zone->GetSpawnByID(spell->targets.at(i));
			if (target && target->IsEntity()) {
				((Entity*)target)->GetInfoStruct()->set_vision(vision);
				if (target->IsPlayer())
					((Player*)target)->SetCharSheetChanged(true);
			}
		}
	}
	else {
		((Entity*)spawn)->GetInfoStruct()->set_vision(vision);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}

int EQ2Emu_lua_BlurVision(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	float intensity = lua_interface->GetFloatValue(state, 2);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA BlurVision command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA BlurVision command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spell && spell->targets.size() > 0) {
		ZoneServer* zone = spell->caster->GetZone();
		for (int8 i = 0; i < spell->targets.size(); i++) {
			Spawn* target = zone->GetSpawnByID(spell->targets.at(i));
			if (target && target->IsEntity()) {
				((Entity*)target)->GetInfoStruct()->set_drunk(intensity);
				if (target->IsPlayer())
					((Player*)target)->SetCharSheetChanged(true);
			}
		}
	}
	else {
		((Entity*)spawn)->GetInfoStruct()->set_drunk(intensity);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}

int EQ2Emu_lua_BreatheUnderwater(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	bool breatheUnderwater = lua_interface->GetBooleanValue(state, 2);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA BreathUnderwater command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA BreathUnderwater command error: spawn is not en entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spell && spell->targets.size() > 0) {
		ZoneServer* zone = spell->caster->GetZone();
		for (int8 i = 0; i < spell->targets.size(); i++) {
			Spawn* target = zone->GetSpawnByID(spell->targets.at(i));
			if (target && target->IsEntity()) {
				((Entity*)target)->GetInfoStruct()->set_breathe_underwater(breatheUnderwater);
				if (target->IsPlayer())
					((Player*)target)->SetCharSheetChanged(true);
			}
		}
	}
	else {
		((Entity*)spawn)->GetInfoStruct()->set_breathe_underwater(breatheUnderwater);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}

int EQ2Emu_lua_GetItemSkillReq(lua_State* state) {
	if (!lua_interface)
		return 0;

	Item* item = lua_interface->GetItem(state);
	int8 type = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!item) {
		lua_interface->LogError("%s: LUA GetItemSkillReq command error: item not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (type == 1)
		lua_interface->SetInt32Value(state, item->generic_info.skill_req1);
	else if (type == 2)
		lua_interface->SetInt32Value(state, item->generic_info.skill_req2);

	return 1;
}

int EQ2Emu_lua_SetSpeedMultiplier(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* target = lua_interface->GetSpawn(state);
	float val = lua_interface->GetFloatValue(state, 2);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	// Added from Gangrenous post
	if (spell && spell->resisted)
		return 0;

	// if its a percentage of 100 its a slow, we want to go at a fraction of the speed
	if (val > 1.0f)
		val = 1.0f - (val / 100.0f);

	if (spell && spell->spell && spell->targets.size() > 0) {
		ZoneServer* zone = spell->caster->GetZone();
		for (int32 i = 0; i != spell->targets.size(); i++) {
			Spawn* spawn = zone->GetSpawnByID(spell->targets.at(i));
			if (spawn && spawn->IsEntity()) {
				((Entity*)spawn)->SetSpeedMultiplier(val);
				if (spawn->IsPlayer())
					((Player*)spawn)->SetCharSheetChanged(true);
			}
		}
	}
	else {
		if (target && target->IsEntity()) {
			((Entity*)target)->SetSpeedMultiplier(val);
			if (target->IsPlayer())
				((Player*)target)->SetCharSheetChanged(true);
		}
	}

	return 0;
}

int EQ2Emu_lua_SetIllusion(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int16 model = lua_interface->GetInt16Value(state, 2);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (spell && spell->spell && spell->targets.size() > 0) {
		ZoneServer* zone = spell->caster->GetZone();
		for (int32 i = 0; i < spell->targets.size(); i++) {
			Spawn* target = zone->GetSpawnByID(spell->targets.at(i));
			if (target)
				target->SetIllusionModel(model);
		}
	}
	else {
		if (!spawn) {
			lua_interface->LogError("%s: LUA SetIllusion command error: spawn is not valid", lua_interface->GetScriptName(state));
			return 0;
		}

		spawn->SetIllusionModel(model);
	}

	return 0;
}

int EQ2Emu_lua_ResetIllusion(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (spell && spell->spell && spell->targets.size() > 0) {
		ZoneServer* zone = spell->caster->GetZone();
		for (int32 i = 0; i < spell->targets.size(); i++) {
			Spawn* target = zone->GetSpawnByID(spell->targets.at(i));
			if (target)
				target->SetIllusionModel(0);
		}
	}
	else {
		if (!spawn) {
			lua_interface->LogError("%s: LUA ResetIllusion command error: spawn is not valid", lua_interface->GetScriptName(state));
			return 0;
		}

		spawn->SetIllusionModel(0);
	}

	return 0;
}

int EQ2Emu_lua_AddThreatTransfer(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* caster = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	float chance = lua_interface->GetFloatValue(state, 3);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!caster) {
		lua_interface->LogError("%s: LUA AddThreatTransfer command error: caster is not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!caster->IsEntity()) {
		lua_interface->LogError("%s: LUA AddThreatTransfer command error: caster is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target) {
		lua_interface->LogError("%s: LUA AddThreatTransfer command error: target is not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target->IsEntity()) {
		lua_interface->LogError("%s: LUA AddThreatTransfer command error: target is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (chance <= 0) {
		lua_interface->LogError("%s: LUA AddThreatTransfer command error: chance must be greater then 0%", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spell) {
		lua_interface->LogError("%s: LUA AddThreatTransfer command error: can only be used in a spell script", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(spell->resisted) {
		return 0;
	}

	if (((Entity*)caster)->GetThreatTransfer()) {
		return 0;
	}

	ThreatTransfer* transfer = new ThreatTransfer;
	transfer->Target = target->GetID();
	transfer->Amount = chance;
	transfer->Spell = spell;

	((Entity*)caster)->SetThreatTransfer(transfer);
	return 0;
}

int EQ2Emu_lua_RemoveThreatTransfer(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn || !spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA RemoveThreatTransfer command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA RemoveThreatTransfer command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spell) {
		lua_interface->LogError("%s: LUA RemoveThreatTransfer command error: can only be used in a spell script", lua_interface->GetScriptName(state));
		return 0;
	}

	if (((Entity*)spawn)->GetThreatTransfer() && ((Entity*)spawn)->GetThreatTransfer()->Spell == spell) {
		ThreatTransfer* transfer = ((Entity*)spawn)->GetThreatTransfer();
		if(transfer && transfer->Spell != spell) {
			lua_interface->LogError("%s: LUA RemoveThreatTransfer called, but there was a different spell set for the threat transfer.", lua_interface->GetScriptName(state));
			return 0;
		}
		((Entity*)spawn)->SetThreatTransfer(nullptr);
	}

	return 0;
}

int EQ2Emu_lua_CureByType(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	int8 cure_count = lua_interface->GetInt8Value(state);
	int8 cure_type = lua_interface->GetInt8Value(state, 2);
	string cure_name = lua_interface->GetStringValue(state, 3);
	int8 cure_level = lua_interface->GetInt8Value(state, 4);
	Spawn* target = lua_interface->GetSpawn(state, 5);
	Spawn* caster = lua_interface->GetSpawn(state, 6); // optional
	lua_interface->ResetFunctionStack(state);
	
	if(spell && spell->resisted) {
		return 0;
	}
	
	if (target) {
		if (!target->IsEntity()) {
			lua_interface->LogError("%s: LUA CureByType command error: spawn override must be entity if used", lua_interface->GetScriptName(state));
			return 0;
		}

		if (((Entity*)target)->GetDetTypeCount(cure_type) > 0) {
			std::string alternate_name = "item";
			if(spell)
				alternate_name = std::string(spell->spell->GetName());
			
			if(!caster && spell)
				caster = (Spawn*)spell->caster;
			
			((Entity*)target)->CureDetrimentByType(cure_count, cure_type, cure_name.length() > 0 ? cure_name : alternate_name, (Entity*)caster, cure_level);
		}
	}
	else {
		ZoneServer* zone = spell->caster->GetZone();
		vector<int32> targets = spell->targets;
		vector<Entity*> targets_to_cure;
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int8 i = 0; i < targets.size(); i++) {
			target = zone->GetSpawnByID(targets.at(i));

			if (!target || !target->IsEntity())
				continue;

			if (((Entity*)target)->GetDetTypeCount(cure_type) > 0) {
				targets_to_cure.push_back((Entity*)target);
			}
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
		
		vector<Entity*>::iterator itr;
		for(itr = targets_to_cure.begin(); itr != targets_to_cure.end(); itr++) {
			((Entity*)*itr)->CureDetrimentByType(cure_count, cure_type, cure_name.length() > 0 ? cure_name : (string)spell->spell->GetName(), spell->caster, cure_level);
		}
	}
	return 0;
}

int EQ2Emu_lua_CureByControlEffect(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	
	if(spell->resisted) {
		lua_interface->ResetFunctionStack(state);
		return 0;
	}
	
	int8 cure_count = lua_interface->GetInt8Value(state);
	int8 cure_type = lua_interface->GetInt8Value(state, 2);
	string cure_name = lua_interface->GetStringValue(state, 3);
	int8 cure_level = lua_interface->GetInt8Value(state, 4);
	Spawn* target = lua_interface->GetSpawn(state, 5);
	lua_interface->ResetFunctionStack(state);

	if (target) {
		if (!target->IsEntity()) {
			lua_interface->LogError("%s: LUA CureByControlEffect command error: spawn override must be entity if used", lua_interface->GetScriptName(state));
			return 0;
		}

		if (((Entity*)target)->GetDetCount() > 0)
			((Entity*)target)->CureDetrimentByControlEffect(cure_count, cure_type, cure_name.length() > 0 ? cure_name : (string)spell->spell->GetName(), spell->caster, cure_level);
	}
	else {
		ZoneServer* zone = spell->caster->GetZone();
		vector<int32> targets = spell->targets;

		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int8 i = 0; i < targets.size(); i++) {
			target = zone->GetSpawnByID(targets.at(i));

			if (!target || !target->IsEntity())
				continue;

			if (((Entity*)target)->GetDetCount() > 0)
				((Entity*)target)->CureDetrimentByControlEffect(cure_count, cure_type, cure_name.length() > 0 ? cure_name : (string)spell->spell->GetName(), spell->caster, cure_level);
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}
	return 0;
}

int EQ2Emu_lua_CancelSpell(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spell) {
		lua_interface->LogError("%s: LUA CancelSpell command error: can only be use in a spell script", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spell->caster) {
		lua_interface->LogError("%s: LUA CancelSpell command error: unable to get the caster of the spell", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spell->caster->GetZone()) {
		lua_interface->LogError("%s: LUA CancelSpell command error: unable to get the zone of the caster", lua_interface->GetScriptName(state));
		return 0;
	}

	spell->caster->GetZone()->GetSpellProcess()->AddSpellCancel(spell);

	return 0;
}

int EQ2Emu_lua_RemoveStealth(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spell) {
		lua_interface->LogError("%s: LUA RemoveStealth command error: must be used from spell script", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spawn && spawn->IsEntity())
		((Entity*)spawn)->RemoveStealthSpell(spell);
	else {
		ZoneServer* zone = spell->caster->GetZone();
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int32 i = 0; i < spell->targets.size(); i++) {
			spawn = zone->GetSpawnByID(spell->targets.at(i));
			if (!spawn || !spawn->IsEntity())
				continue;

			((Entity*)spawn)->RemoveStealthSpell(spell);
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}
	return 0;
}

int EQ2Emu_lua_RemoveInvis(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spell) {
		lua_interface->LogError("%s: LUA RemoveInvis command error: must be used from spell script", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spawn && spawn->IsEntity())
		((Entity*)spawn)->RemoveInvisSpell(spell);
	else {
		ZoneServer* zone = spell->caster->GetZone();
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int32 i = 0; i < spell->targets.size(); i++) {
			spawn = zone->GetSpawnByID(spell->targets.at(i));
			if (!spawn || !spawn->IsEntity())
				continue;

			((Entity*)spawn)->RemoveInvisSpell(spell);
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}
	return 0;
}

int EQ2Emu_lua_StartHeroicOpportunity(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* caster = lua_interface->GetSpawn(state);
	int8 class_id = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!caster) {
		lua_interface->LogError("%s: LUA StartHeroicOpportunity command error: caster is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!caster->IsPlayer()) {
		lua_interface->LogError("%s: LUA StartHeroicOpportunity command error: caster must be a player", lua_interface->GetScriptName(state));
		return 0;
	}

	Spawn* target = caster->GetTarget();
	if (!target) {
		lua_interface->LogError("%s: LUA StartHeroicOpportunity command error: target is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	Client* client = ((Player*)caster)->GetClient();
	if (!client) {
		lua_interface->LogError("%s: LUA StartHeroicOpportunity command error: unable to get a client for the given caster", lua_interface->GetScriptName(state));
		return 0;
	}

	HeroicOP* ho = master_ho_list.GetHeroicOP(class_id);
	if (ho) {
		ho->SetTarget(target->GetID());
		LogWrite(SPELL__ERROR, 0, "HO", "caster: %u", caster->GetID());
		LogWrite(SPELL__ERROR, 0, "HO", "target: %u", target->GetID());
		if (((Entity*)caster)->GetGroupMemberInfo()) {
			if (caster->GetZone()->GetSpellProcess()->AddHO(client, ho)) {
				world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);

				deque<GroupMemberInfo*>::iterator itr;
				PlayerGroup* group = world.GetGroupManager()->GetGroup(((Entity*)caster)->GetGroupMemberInfo()->group_id);
				if (group)
				{
					group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
					deque<GroupMemberInfo*>* members = group->GetMembers();
					for (itr = members->begin(); itr != members->end(); itr++) {
						if ((*itr)->client)
							ClientPacketFunctions::SendHeroicOPUpdate((*itr)->client, ho);
					}
					group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
				}

				world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
			}
			else
				safe_delete(ho);
		}
		else {
			if (caster->GetZone()->GetSpellProcess()->AddHO(client, ho)) {
				ClientPacketFunctions::SendHeroicOPUpdate(client, ho);
			}
			else
				safe_delete(ho);
		}
	}

	return 0;
}

int EQ2Emu_lua_SetSpellTriggerCount(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);

	if (!spell) {
		lua_interface->LogError("%s: LUA SetSpellTriggerCount command error: you must use this function in a spellscript!", lua_interface->GetScriptName(state));
		return 0;
	}

	int16 triggerCount = lua_interface->GetInt16Value(state);
	bool cancel_after_triggers = (lua_interface->GetInt8Value(state, 2) == 1);

	if (!triggerCount) {
		lua_interface->LogError("%s: LUA SetSpellTriggerCount command error: used trigger value equals zero!", lua_interface->GetScriptName(state));
		return 0;
	}

	spell->num_triggers = triggerCount;
	spell->had_triggers = true;
	spell->cancel_after_all_triggers = cancel_after_triggers;

	return 0;
}

int EQ2Emu_lua_GetSpellTriggerCount(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spell) {
		lua_interface->LogError("%s: LUA GetSpellTriggerCount command error: you must use this function in a spellscript!", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, spell->num_triggers);

	return 1;
}

int EQ2Emu_lua_RemoveTriggerFromSpell(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);

	if (!spell) {
		lua_interface->LogError("%s: LUA RemoveTriggerFromSpell command error: you must use this function in a spellscript!", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}
	
	if (!spell->caster || !spell->caster->GetZone()) {
		lua_interface->LogError("%s: LUA RemoveTriggerFromSpell command error: caster / caster zone must be set!", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	int16 remove_count = lua_interface->GetInt16Value(state);

	lua_interface->ResetFunctionStack(state);
	
	if (!remove_count)
		remove_count = 1;

	if (remove_count >= spell->num_triggers) {
		spell->num_triggers = 0;
		if (spell->cancel_after_all_triggers)
			spell->caster->GetZone()->GetSpellProcess()->AddSpellCancel(spell);
	}
	else {
		spell->num_triggers -= remove_count;
		Client* client = spell->caster->IsPlayer() ? ((Player*)spell->caster)->GetClient() : nullptr;
		ClientPacketFunctions::SendMaintainedExamineUpdate(client, spell->slot_pos, spell->num_triggers, 0);
	}
	return 0;
}

int EQ2Emu_lua_CopySpawnAppearance(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* copy_spawn = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA CopySpawnAppearance command error: the first spawn used was not valid!", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!copy_spawn) {
		lua_interface->LogError("%s: LUA CopySpawnAppearance command error: the second spawn used was not valid!", lua_interface->GetScriptName(state));
		return 0;
	}

	spawn->CopySpawnAppearance(copy_spawn);
	return 0;
}

int EQ2Emu_lua_HasSpellImmunity(lua_State* state) {
	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 type = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA HasSpellImmunity command error: spawn does not exist.", lua_interface->GetScriptName(state));
		return 0;
	}
	else if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA HasSpellImmunity command error: spawn %s is not an entity.", lua_interface->GetScriptName(state), spawn->GetName());
		return 0;
	}
	
	lua_interface->SetBooleanValue(state, ((Entity*)spawn)->IsImmune(type));
	return 1;
}

int EQ2Emu_lua_AddImmunitySpell(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	int8 type = lua_interface->GetInt8Value(state);
	Spawn* spawn = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spell) {
		lua_interface->LogError("%s: LUA AddImmunitySpell command error: This must be used in a spellscript", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(spell->resisted) {
		return 0;
	}

	if (spawn) {
		if (!spawn->IsEntity()) {
			lua_interface->LogError("%s: LUA AddImmunitySpell command error: The spawn provided is not an entity", lua_interface->GetScriptName(state));
			return 0;
		}
		Entity* entity = ((Entity*)spawn);
		entity->AddImmunity(spell, type);
	}
	else if(spell->caster && spell->caster->GetZone()) {
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int8 i = 0; i < spell->targets.size(); i++) {
			spawn = spell->caster->GetZone()->GetSpawnByID(spell->targets.at(i));
			if (!spawn || !spawn->IsEntity())
				continue;
			Entity* entity = ((Entity*)spawn);
			entity->AddImmunity(spell, type);
			}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}

	return 0;
}

int EQ2Emu_lua_RemoveImmunitySpell(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	int8 type = lua_interface->GetInt8Value(state);
	Spawn* spawn = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spell) {
		lua_interface->LogError("%s: LUA RemoveImmunitySpell command error: This must be used in a spellscript", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spawn) {
		if (!spawn->IsEntity()) {
			lua_interface->LogError("%s: LUA RemoveImmunitySpell command error: The spawn provided is not an entity", lua_interface->GetScriptName(state));
			return 0;
		}
		Entity* entity = ((Entity*)spawn);
		entity->RemoveImmunity(spell, type);
	}
	else if(spell->caster && spell->caster->GetZone()) {
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int8 i = 0; i < spell->targets.size(); i++) {
			spawn = spell->caster->GetZone()->GetSpawnByID(spell->targets.at(i));
			if (!spawn || !spawn->IsEntity())
				continue;
			Entity* entity = ((Entity*)spawn);
			entity->RemoveImmunity(spell, type);
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}

	return 0;
}

int EQ2Emu_lua_SetSpellSnareValue(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	if(spell && spell->resisted) {
		lua_interface->ResetFunctionStack(state);
		return 0;
	}
	
	float snare = lua_interface->GetFloatValue(state);
	Spawn* spawn = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);

	// convert the val to the speed multipler value (100 - val)
	float val = 100.0 - snare;
	val /= 100.0;

	if (spawn) {
		if (!spawn->IsEntity()) {
			lua_interface->LogError("%s: LUA SetSpellSnareValue command error: spawn must be an entity.", lua_interface->GetScriptName(state));
			return 0;
		}

		((Entity*)spawn)->SetSnareValue(spell, val);
	}
	else if(spell && spell->caster && spell->caster->GetZone()) {
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int8 i = 0; i < spell->targets.size(); i++) {
			spawn = spell->caster->GetZone()->GetSpawnByID(spell->targets.at(i));
			if (!spawn || !spawn->IsEntity())
				continue;

			((Entity*)spawn)->SetSnareValue(spell, val);
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}

	return 0;
}

int EQ2Emu_lua_CheckRaceType(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int16 race_id = lua_interface->GetInt16Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA CheckRaceType command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (race_id == 0) {
		lua_interface->LogError("%s: LUA CheckRaceType command error: race id must be set", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, (race_id == race_types_list.GetRaceType(spawn->GetModelType()) || race_id == race_types_list.GetRaceBaseType(spawn->GetModelType())));
	return 1;
}

int EQ2Emu_lua_GetRaceType(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA GetRaceType command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, race_types_list.GetRaceType(spawn->GetModelType()));
	return 1;
}

int EQ2Emu_lua_GetRaceBaseType(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA GetRaceBaseType command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, race_types_list.GetRaceBaseType(spawn->GetModelType()));
	return 1;
}

int EQ2Emu_lua_GetSpellName(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);

	if (!spell) {
		lua_interface->LogError("%s: LUA GetSpellName command error: this function must be used from a spell script!", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetStringValue(state, spell->spell->GetName());
	return 1;
}

int EQ2Emu_lua_GetQuestFlags(lua_State* state) {
	if (!lua_interface)
		return 0;

	Quest* quest = lua_interface->GetQuest(state);
	lua_interface->ResetFunctionStack(state);
	if (!quest) {
		lua_interface->LogError("%s: LUA GetQuestFlags command error: quest is not valid.", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, quest->GetQuestFlags());
	return 1;
}

int EQ2Emu_lua_SetQuestFlags(lua_State* state) {
	if (!lua_interface)
		return 0;

	Quest* quest = lua_interface->GetQuest(state);
	int32 flags = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!quest) {
		lua_interface->LogError("%s: LUA SetQuestFlags command error: quest is not valid.", lua_interface->GetScriptName(state));
		return 0;
	}

	quest->SetQuestFlags(flags);
	return 0;
}

int EQ2Emu_lua_SetQuestTimer(lua_State* state) {
	if (!lua_interface)
		return 0;

	Quest* quest = lua_interface->GetQuest(state);
	Spawn* player = lua_interface->GetSpawn(state, 2);
	int32 step = lua_interface->GetInt32Value(state, 3);
	int32 duration = lua_interface->GetInt32Value(state, 4);
	string action = lua_interface->GetStringValue(state, 5);
	lua_interface->ResetFunctionStack(state);

	if (!quest) {
		lua_interface->LogError("%s: LUA SetQuestTimer command error: quest is not valid.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player) {
		lua_interface->LogError("%s: LUA SetQuestTimer command error: player is not a valid spawn.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA SetQuestTimer command error: player is not a valid player.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (step == 0) {
		lua_interface->LogError("%s: LUA SetQuestTimer command error: step must be set.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (duration == 0) {
		lua_interface->LogError("%s: LUA SetQuestTimer command error: duration must be set.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (action.length() == 0) {
		lua_interface->LogError("%s: LUA SetQuestTimer command error: failed action must be set.", lua_interface->GetScriptName(state));
		return 0;
	}

	Client* client = ((Player*)player)->GetClient();
	if (!client) {
		lua_interface->LogError("%s: LUA SetQuestTimer command error: failed to get a valid client pointer for the given player", lua_interface->GetScriptName(state));
		return 0;
	}

	quest->SetTimerStep(step);
	quest->AddFailedAction(step, action);
	quest->SetStepTimer(duration);
	client->AddQuestTimer(quest->GetQuestID());

	return 0;
}

int EQ2Emu_lua_SetQuestTimerComplete(lua_State* state) {
	if (!lua_interface)
		return 0;

	Quest* quest = lua_interface->GetQuest(state);
	Spawn* player = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!quest) {
		lua_interface->LogError("%s: LUA SetQuestTimerComplete command error: quest is not valid.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player) {
		lua_interface->LogError("%s: LUA SetQuestTimerComplete command error: player is not a valid spawn.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA SetQuestTimerComplete command error: player is not a valid player.", lua_interface->GetScriptName(state));
		return 0;
	}

	Client* client = ((Player*)player)->GetClient();
	if (!client) {
		lua_interface->LogError("%s: LUA SetQuestTimerComplete command error: failed to get a valid client pointer for the given player", lua_interface->GetScriptName(state));
		return 0;
	}

	quest->SetTimerStep(0);
	quest->SetStepTimer(0);
	client->RemoveQuestTimer(quest->GetQuestID());

	return 0;
}

int EQ2Emu_lua_RemoveQuestStep(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	Quest* quest = lua_interface->GetQuest(state, 2);
	int32 step = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA RemoveQuestStep command error: player is not a valid spawn.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA RemoveQuestStep command error: player is not a valid player.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!quest) {
		lua_interface->LogError("%s: LUA RemoveQuestStep command error: quest is not valid.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (step == 0) {
		lua_interface->LogError("%s: LUA RemoveQuestStep command error: step must be set.", lua_interface->GetScriptName(state));
		return 0;
	}

	Client* client = ((Player*)player)->GetClient();
	if (!client) {
		lua_interface->LogError("%s: LUA RemoveQuestStep command error: unable to get a valid client pointer from the given player.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (quest->RemoveQuestStep(step, client)) {
		client->QueuePacket(quest->QuestJournalReply(client->GetVersion(), client->GetNameCRC(), (Player*)player, 0, 0, 0, true));
		client->GetCurrentZone()->SendQuestUpdates(client);
	}
	else
		lua_interface->LogError("LUA RemoveQuestStep command error: unable to remove the step (%u) from the quest (%s).", step, quest->GetName());

	return 0;
}

int EQ2Emu_lua_ResetQuestStep(lua_State* state) {
	if (!lua_interface)
		return 0;

	Quest* quest = lua_interface->GetQuest(state, 1);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string desc = lua_interface->GetStringValue(state, 3);
	string task_group = lua_interface->GetStringValue(state, 4);
	lua_interface->ResetFunctionStack(state);

	if (!quest) {
		lua_interface->LogError("%s: LUA ResetQuestStep command error: quest is not valid.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (step == 0) {
		lua_interface->LogError("%s: LUA ResetQuestStep command error: step must be set.", lua_interface->GetScriptName(state));
		return 0;
	}

	QuestStep* quest_step = quest->GetQuestStep(step);
	if (!quest_step) {
		lua_interface->LogError("%s: LUA ResetQuestStep command error: unable to get a valid quest step.", lua_interface->GetScriptName(state));
		return 0;
	}

	quest_step->SetStepProgress(0);
	quest_step->SetTaskGroup(task_group);
	quest_step->SetDescription(desc);

	return 0;
}

int EQ2Emu_lua_AddQuestStepFailureAction(lua_State* state) {
	if (!lua_interface)
		return 0;

	Quest* quest = lua_interface->GetQuest(state);
	int32 step = lua_interface->GetInt32Value(state, 2);
	string action = lua_interface->GetStringValue(state, 3);
	lua_interface->ResetFunctionStack(state);

	if (!quest) {
		lua_interface->LogError("%s: LUA AddQuestStepFailureAction command error: quest is not valid.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (step == 0) {
		lua_interface->LogError("%s: LUA AddQuestStepFailureAction command error: step must be set.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (action.length() == 0) {
		lua_interface->LogError("%s: LUA AddQuestStepFailureAction command error: action must be set.", lua_interface->GetScriptName(state));
		return 0;
	}

	quest->AddFailedAction(step, action);
	return 0;
}

int EQ2Emu_lua_SetStepFailed(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	int32 step = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA SetStepFailed command error: player is not a valid spawn.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA SetStepFailed command error: player is not a valid player.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (quest_id == 0) {
		lua_interface->LogError("%s: LUA SetStepFailed command error: quest_id must be set.", lua_interface->GetScriptName(state));
		return 0;
	}

	if (step == 0) {
		lua_interface->LogError("%s: LUA SetStepFailed command error: step must be set.", lua_interface->GetScriptName(state));
		return 0;
	}

	Quest* quest = ((Player*)player)->GetQuest(quest_id);
	if (!quest) {
		lua_interface->LogError("LUA SetStepFailed command error: unable to get a valid quest from the given id (%u).", quest_id);
		return 0;
	}

	quest->StepFailed(step);
	return 0;
}

int EQ2Emu_lua_GetQuestCompleteCount(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA GetQuestCompleteCount command error: player is not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA GetQuestCompleteCount command error: player is not a valid player", lua_interface->GetScriptName(state));
		return 0;
	}

	if (quest_id == 0) {
		lua_interface->LogError("%s: LUA GetQuestCompleteCount command error: quest id is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, ((Player*)player)->GetQuestCompletedCount(quest_id));
	return 1;
}

int EQ2Emu_lua_SetServerVariable(lua_State* state) {
	if (!lua_interface)
		return 0;

	string name = lua_interface->GetStringValue(state);
	string value = lua_interface->GetStringValue(state, 2);
	string comment = lua_interface->GetStringValue(state, 3);
	lua_interface->ResetFunctionStack(state);

	if (name.length() == 0) {
		lua_interface->LogError("%s: LUA SetServerVariable command error: name is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (value.length() == 0) {
		lua_interface->LogError("%s: LUA SetServerVariable command error: value is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	string varname = string("lua_").append(name);
	Variable* var = variables.FindVariable(varname);
	if (var)
		var->SetValue(value.c_str());
	else {
		var = new Variable(varname.c_str(), value.c_str(), comment.c_str());
		variables.AddVariable(var);
	}

	database.SaveVariable(var->GetName(), var->GetValue(), var->GetComment());
	return 0;
}

int EQ2Emu_lua_GetServerVariable(lua_State* state) {
	if (!lua_interface)
		return 0;

	string name = lua_interface->GetStringValue(state);
	lua_interface->ResetFunctionStack(state);
	if (name.length() == 0) {
		lua_interface->LogError("%s: LUA GetServerVariable command error: name is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	string varname = string("lua_").append(name);
	Variable* var = variables.FindVariable(varname);
	if (var)
		lua_interface->SetStringValue(state, var->GetValue());
	else
		lua_interface->SetStringValue(state, "NULL");

	return 1;
}

int EQ2Emu_lua_HasLanguage(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int32 language_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA HasLanguage command error: player is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA HasLanguage command error: player is not a valid player", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, ((Player*)player)->HasLanguage(language_id));
	return 1;
}

int EQ2Emu_lua_AddLanguage(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int32 language_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA AddLanguage command error: player is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA AddLanguage command error: player is not a valid player", lua_interface->GetScriptName(state));
		return 0;
	}

	Language* language = master_languages_list.GetLanguage(language_id);
	if (language)
	{
		((Player*)player)->AddLanguage(language->GetID(), language->GetName(), true);
		((Player*)player)->GetClient()->SendLanguagesUpdate(language->GetID(), 0);
	}

	return 0;
}

int EQ2Emu_lua_IsNight(lua_State* state) {
	if (!lua_interface)
		return 0;

	ZoneServer* zone = lua_interface->GetZone(state);
	lua_interface->ResetFunctionStack(state);
	if (!zone) {
		lua_interface->LogError("%s: LUA IsNight command error: zone is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, zone->IsDusk());
	return 1;
}

int EQ2Emu_lua_AddMultiFloorLift(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA AddMultiFloorLift command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsWidget()) {
		lua_interface->LogError("%s: LUA AddMultiFloorLift command error: spawn is not a widget", lua_interface->GetScriptName(state));
		return 0;
	}

	((Widget*)spawn)->SetMultiFloorLift(true);

	if (spawn->GetZone())
		spawn->GetZone()->AddTransportSpawn(spawn);

	return 0;
}

int EQ2Emu_lua_StartAutoMount(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int32 path = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA StartAutoMount command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA StartAutoMount command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	if (path == 0) {
		lua_interface->LogError("%s: LUA StartAutoMount command error: path must be greater then zero", lua_interface->GetScriptName(state));
		return 0;
	}

	Client* client = ((Player*)player)->GetClient();
	if (!client) {
		lua_interface->LogError("%s: LUA StartAutoMount command error: unable to get a client from the given player", lua_interface->GetScriptName(state));
		return 0;
	}

	client->SendFlightAutoMount(path);

	return 0;
}

int EQ2Emu_lua_EndAutoMount(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	if (!player) {
		lua_interface->LogError("%s: LUA EndAutoMount command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA EndAutoMount command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	Client* client = ((Player*)player)->GetClient();
	if (!client) {
		lua_interface->LogError("%s: LUA EndAutoMount command error: unable to get a client from the given player", lua_interface->GetScriptName(state));
		return 0;
	}
	client->EndAutoMount();

	return 0;
}

int EQ2Emu_lua_IsOnAutoMount(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!player) {
		lua_interface->LogError("%s: LUA IsOnAutoMount command error: spawn in not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA IsOnAutoMount command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	Client* client = ((Player*)player)->GetClient();
	if (!client) {
		lua_interface->LogError("%s: LUA IsOnAutoMount command error: unable to get a client from the given player", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, client->GetOnAutoMount());
	return 1;
}

int EQ2Emu_lua_SetPlayerHistory(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int32 event_id = lua_interface->GetInt32Value(state, 2);
	int32 value = lua_interface->GetInt32Value(state, 3);
	int32 value2 = lua_interface->GetInt32Value(state, 4);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA SetPlayerHistory command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA SetPlayerHistory command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	((Player*)player)->UpdateLUAHistory(event_id, value, value2);
	return 0;
}

int EQ2Emu_lua_GetPlayerHistory(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int32 event_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA GetPlayerHistory command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA GetPlayerHistory command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	LUAHistory* hd = ((Player*)player)->GetLUAHistory(event_id);
	if (!hd)
		return 0;

	lua_interface->SetInt32Value(state, hd->Value);
	lua_interface->SetInt32Value(state, hd->Value2);
	return 2;
}

int EQ2Emu_lua_SetGridID(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 grid = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SetGridID command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (grid == 0) {
		lua_interface->LogError("%s: LUA SetGridID command error: grid is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	spawn->SetLocation(grid);
	return 0;
}

int EQ2Emu_lua_SetRequiredHistory(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 event_id = lua_interface->GetInt32Value(state, 2);
	int32 value1 = lua_interface->GetInt32Value(state, 3);
	int32 value2 = lua_interface->GetInt32Value(state, 4);
	bool private_spawn = (lua_interface->GetInt8Value(state, 5) == 1);
	int16 flag_override = lua_interface->GetInt16Value(state, 6);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SetRequiredHistory command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	//Add this quest to the list of required quests for this spawn
	spawn->SetRequiredHistory(event_id, value1, value2);
	//If private spawn value set
	if (private_spawn) {
		//Set the spawn to be private when not granted access via history
		spawn->AddAllowAccessSpawn(spawn);
		spawn->SetPrivateQuestSpawn(true);
	}
	//This value will override vis_flags in the vis packet
	if (flag_override > 0)
		spawn->SetQuestsRequiredOverride(flag_override);
	return 0;
}

int EQ2Emu_lua_GetQuestStepProgress(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int32 quest_id = lua_interface->GetInt32Value(state, 2);
	int32 step_id = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA GetQuestStepProgress command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA GetQuestStepProgress command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, ((Player*)player)->GetStepProgress(quest_id, step_id));

	return 1;
}

int EQ2Emu_lua_SetPlayerLevel(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int8 level = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA SetPlayerLevel command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA SetPlayerLevel command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	if (level == 0) {
		lua_interface->LogError("%s: LUA SetPlayerLevel command error: new level can't be 0", lua_interface->GetScriptName(state));
		return 0;
	}

	Client* client = ((Player*)player)->GetClient();
	if (!client) {
		lua_interface->LogError("%s: LUA SetPlayerLevel command error: unable to get a client from the given spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	client->ChangeLevel(client->GetPlayer()->GetLevel(), level);
	return 0;
}

int EQ2Emu_lua_AddCoin(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int32 amount = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA AddCoin command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA AddCoin command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	if (amount == 0) {
		lua_interface->LogError("%s: LUA AddCoin command error: amount must be greater then 0", lua_interface->GetScriptName(state));
		return 0;
	}

	((Player*)player)->AddCoins(amount);
	return 0;
}

int EQ2Emu_lua_RemoveCoin(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int32 amount = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA RemoveCoin command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA RemoveCoin command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	if (amount == 0) {
		lua_interface->LogError("%s: LUA RemoveCoin command error: amount must be greater then 0", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, ((Player*)player)->RemoveCoins(amount));
	return 1;
}

int EQ2Emu_lua_GetPlayersInZone(lua_State* state) {
	if (!lua_interface)
		return 0;

	ZoneServer* zone = lua_interface->GetZone(state);
	lua_interface->ResetFunctionStack(state);
	if (!zone) {
		lua_interface->LogError("%s: LUA GetPlayersInZone command error: zone is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	vector<Entity*> players = zone->GetPlayers();
	if (players.size() == 0)
		return 0;

	lua_createtable(state, players.size(), 0);
	int newTable = lua_gettop(state);
	for (int32 i = 0; i < players.size(); i++) {
		lua_interface->SetSpawnValue(state, players.at(i));
		lua_rawseti(state, newTable, i + 1);
	}

	return 1;
}

int EQ2Emu_lua_SpawnGroupByID(lua_State* state) {
	if (!lua_interface)
		return 0;

	ZoneServer* zone = lua_interface->GetZone(state, 1);
	if (!zone) {
		lua_interface->LogError("%s: LUA SpawnGroupByID command error: zone is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	int32 group_id = lua_interface->GetInt32Value(state, 2);
	int16 custom_level = lua_interface->GetInt32Value(state, 3);

	lua_interface->ResetFunctionStack(state);
	
	//Map of <placement_id, location_id>
	map<int32, int32>* locs = zone->GetSpawnLocationsByGroup(group_id);
	map<int32, int32>::iterator itr;

	vector<Spawn*> group;

	Spawn* leader = 0;
	if(locs == nullptr)
		return 0;
	
	for (itr = locs->begin(); itr != locs->end(); itr++) {
		SpawnLocation* location = zone->GetSpawnLocation(itr->second);
		if (!location) {
			lua_interface->LogError("%s: LUA SpawnGroupByID command error: no location found for group id %u and the given ID (%u)", lua_interface->GetScriptName(state), group_id, itr->second);
			return 0;
		}
		
		int32 rand_number = MakeRandomInt(0, location->entities.size() - 1);
		
		Spawn* spawn = 0;
		if (location->entities[rand_number]) {
			if (location->entities[rand_number]->spawn_type == SPAWN_ENTRY_TYPE_NPC)
				spawn = zone->AddNPCSpawn(location, location->entities[rand_number]);
			else if (location->entities[rand_number]->spawn_type == SPAWN_ENTRY_TYPE_GROUNDSPAWN)
				spawn = zone->AddGroundSpawn(location, location->entities[rand_number]);
			else if (location->entities[rand_number]->spawn_type == SPAWN_ENTRY_TYPE_OBJECT)
				spawn = zone->AddObjectSpawn(location, location->entities[rand_number]);
			else if (location->entities[rand_number]->spawn_type == SPAWN_ENTRY_TYPE_WIDGET)
				spawn = zone->AddWidgetSpawn(location, location->entities[rand_number]);
			else if (location->entities[rand_number]->spawn_type == SPAWN_ENTRY_TYPE_SIGN)
				spawn = zone->AddSignSpawn(location, location->entities[rand_number]);

			if(spawn && spawn->IsOmittedByDBFlag())
			{
				LogWrite(SPAWN__WARNING, 0, "Spawn", "Spawn (%u) was skipped due to a missing expansion / holiday flag being met (LUA SpawnGroupByID).", location->entities[rand_number]->spawn_id);
				safe_delete(spawn);
				continue;
			}
			if (spawn) {
				if(!leader)
					leader = spawn;
				if(leader)
					leader->AddSpawnToGroup(spawn);
				
				spawn->SetSpawnGroupID(group_id);
				
				if(custom_level > 0 && custom_level != 0xFFFF) {
						spawn->SetLevel(custom_level);
				}
				
				const char* script = 0;
				for (int x = 0; x < 3; x++) {
					switch (x) {
					case 0:
						script = world.GetSpawnEntryScript(location->entities[rand_number]->spawn_entry_id);
						break;
					case 1:
						script = world.GetSpawnLocationScript(location->entities[rand_number]->spawn_location_id);
						break;
					case 2:
						script = world.GetSpawnScript(location->entities[rand_number]->spawn_id);
						break;
					}
					if (script && lua_interface->GetSpawnScript(script) != 0) {
						spawn->SetSpawnScript(string(script));
						break;
					}
				}
				zone->CallSpawnScript(spawn, SPAWN_SCRIPT_SPAWN);
				lua_interface->SetSpawnValue(state, spawn);
				group.push_back(spawn);
			}
			else {
				LogWrite(ZONE__ERROR, 0, "Zone", "%s: Error adding spawn by group id to zone %s with location id %u.", lua_interface->GetScriptName(state), zone->GetZoneName(), group_id);
				safe_delete(spawn);
			}
		}
	}

	if (!group.empty()) {
		lua_createtable(state, group.size(), 0);
		int newTable = lua_gettop(state);
		for (int32 i = 0; i < group.size(); i++) {
			lua_interface->SetSpawnValue(state, group[i]);
			lua_rawseti(state, newTable, i + 1);
		}
	}
	else
		lua_pushnil(state);

	return 1;
}

int EQ2Emu_lua_SetSpawnAnimation(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state, 1);
	int32 anim_id = lua_interface->GetInt32Value(state, 2);
	int16 leeway = lua_interface->GetInt16Value(state, 3);
	lua_interface->ResetFunctionStack(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SetSpawnAnimation command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (anim_id == 0) {
		lua_interface->LogError("%s: LUA SetSpawnAnimation command error: anim_id is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (leeway == 0)
		leeway = 5000;

	spawn->SetSpawnAnim(anim_id);
	spawn->SetSpawnAnimLeeway(leeway);

	return 0;
}

int EQ2Emu_lua_GetClientVersion(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!player || !player->IsPlayer()) {
		return 0;
	}

	Client* client = ((Player*)player)->GetClient();
	if (!client) {
		return 0;
	}


	lua_interface->SetInt32Value(state, client->GetVersion());
	return 1;
}

int EQ2Emu_lua_GetItemID(lua_State* state) {
	if (!lua_interface)
		return 0;

	Item* item = lua_interface->GetItem(state);
	lua_interface->ResetFunctionStack(state);
	if (!item) {
		lua_interface->LogError("%s: LUA GetItemID command error: item is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, item->details.item_id);
	return 1;
}

int EQ2Emu_lua_IsEntity(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA IsEntity command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetBooleanValue(state, spawn->IsEntity());
	return 1;
}

int EQ2Emu_lua_GetOrigX(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetOrigX command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetFloatValue(state, spawn->GetSpawnOrigX());
	return 1;
}

int EQ2Emu_lua_GetOrigY(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetOrigY command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetFloatValue(state, spawn->GetSpawnOrigY());
	return 1;
}

int EQ2Emu_lua_GetOrigZ(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetOrigZ command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetFloatValue(state, spawn->GetSpawnOrigZ());
	return 1;
}

int EQ2Emu_lua_GetPCTOfHP(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	float pct = lua_interface->GetFloatValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetPCTOfHP command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (pct == 0) {
		lua_interface->LogError("%s: LUA GetPCTOfHP command error: pct is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	int32 amount = std::round(spawn->GetTotalHP() * (pct / 100));
	lua_interface->SetInt32Value(state, amount);
	return 1;
}

int EQ2Emu_lua_GetPCTOfPower(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	float pct = lua_interface->GetFloatValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetPCTOfPower command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (pct == 0) {
		lua_interface->LogError("%s: LUA GetPCTOfPower command error: pct is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	int32 amount = std::round(spawn->GetTotalPower() * (pct / 100));
	lua_interface->SetInt32Value(state, amount);
	return 1;
}

int EQ2Emu_lua_GetBoundZoneID(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetBoundZoneID command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsPlayer()) {
		lua_interface->LogError("%s: LUA GetBoundZoneID command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	lua_interface->SetInt32Value(state, ((Player*)spawn)->GetPlayerInfo()->GetBindZoneID());
	return 1;
}

int EQ2Emu_lua_Evac(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* target = lua_interface->GetSpawn(state);

	if (target) {
		float x = target->GetZone()->GetSafeX();
		float y = target->GetZone()->GetSafeY();
		float z = target->GetZone()->GetSafeZ();
		float h = target->GetZone()->GetSafeHeading();

		target->SetX(x);
		target->SetY(y);
		target->SetZ(z);
		target->SetHeading(h);

		target->SetSpawnOrigX(x);
		target->SetSpawnOrigY(y);
		target->SetSpawnOrigZ(z);
		target->SetSpawnOrigHeading(h);

		if (target->IsPlayer()) {
			Client* client = ((Player*)target)->GetClient();
			if (client) {
				client->GetCurrentZone()->ClearHate(client->GetPlayer());
				
				int numargs = lua_interface->GetNumberOfArgs(state);		
				if(numargs == 4) {
					x = lua_interface->GetFloatValue(state,1);
					y = lua_interface->GetFloatValue(state,2);
					z = lua_interface->GetFloatValue(state,3);
					h = lua_interface->GetFloatValue(state,4);
				}

				client->SetReloadingZone(true);
				target->SetX(x);
				target->SetY(y);
				target->SetZ(z);
				target->SetHeading(h);

				target->SetSpawnOrigX(x);
				target->SetSpawnOrigY(y);
				target->SetSpawnOrigZ(z);
				target->SetSpawnOrigHeading(h);

				target->SetAppearancePosition(x,y,z);
				
				client->SetZoningCoords(x,y,z,h);

				PacketStruct* packet = configReader.getStruct("WS_TeleportWithinZone", client->GetVersion());
				if (packet)
				{
					packet->setDataByName("x", x);
					packet->setDataByName("y", y);
					packet->setDataByName("z", z);
					client->QueuePacket(packet->serialize());
					safe_delete(packet);
				}

					client->GetCurrentZone()->RemoveSpawn(target, false, false, true, true);
							
					client->GetPlayer()->SetSpawnSentState(target, SpawnState::SPAWN_STATE_SENT);
			}
		}
		lua_interface->ResetFunctionStack(state);
	}
	else {

		LuaSpell* spell = lua_interface->GetCurrentSpell(state);
		
		if(!spell || !spell->caster || !spell->caster->GetZone()) {
			lua_interface->ResetFunctionStack(state);
			return 0;
		}
		
		ZoneServer* zone = spell->caster->GetZone();
		
		float x = spell->caster->GetZone()->GetSafeX();
		float y = spell->caster->GetZone()->GetSafeY();
		float z = spell->caster->GetZone()->GetSafeZ();
		float h = spell->caster->GetZone()->GetSafeHeading();

		int numargs = lua_interface->GetNumberOfArgs(state);
		
		if(numargs == 4) {
			x = lua_interface->GetFloatValue(state,1);
			y = lua_interface->GetFloatValue(state,2);
			z = lua_interface->GetFloatValue(state,3);
			h = lua_interface->GetFloatValue(state,4);
		}
	
		lua_interface->ResetFunctionStack(state);
		
		spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
		for (int32 i = 0; i < spell->targets.size(); i++) {
			Spawn* target2 = zone->GetSpawnByID(spell->targets.at(i));
			if (!target2)
				continue;

			if (target2->IsPlayer()) {
				Client* client = ((Player*)target2)->GetClient();
				if (client) {
					client->GetCurrentZone()->ClearHate(client->GetPlayer());
					
					client->SetReloadingZone(true);
					target2->SetX(x);
					target2->SetY(y);
					target2->SetZ(z);
					target2->SetHeading(h);

					target2->SetSpawnOrigX(x);
					target2->SetSpawnOrigY(y);
					target2->SetSpawnOrigZ(z);
					target2->SetSpawnOrigHeading(h);

					target2->SetAppearancePosition(x,y,z);
					
					client->SetZoningCoords(x,y,z,h);
					
					PacketStruct* packet = configReader.getStruct("WS_TeleportWithinZone", client->GetVersion());
					if (packet)
					{
						packet->setDataByName("x", x);
						packet->setDataByName("y", y);
						packet->setDataByName("z", z);
						client->QueuePacket(packet->serialize());
						safe_delete(packet);
					}

					client->GetCurrentZone()->RemoveSpawn(target2, false, false, true, true);
					
					client->GetPlayer()->SetSpawnSentState(target2, SpawnState::SPAWN_STATE_SENT);
				}
			}
		}
		spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
	}

	return 0;
}

int EQ2Emu_lua_GetSpellTier(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	if (!luaspell || !luaspell->spell) {
		lua_interface->LogError("%s: LUA GetSpellTier command error: must be used in a spell script", lua_interface->GetScriptName(state));
		return 0;
	}

	int8 tier = luaspell->spell->GetSpellTier();
	lua_interface->SetInt32Value(state, tier);
	return 1;
}

int EQ2Emu_lua_GetSpellID(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* luaspell = lua_interface->GetCurrentSpell(state);
	lua_interface->ResetFunctionStack(state);
	if (!luaspell || !luaspell->spell) {
		lua_interface->LogError("%s: LUA GetSpellID command error: must be used in a spell script", lua_interface->GetScriptName(state));
		return 0;
	}

	int32 spell_id = luaspell->spell->GetSpellID();
	lua_interface->SetInt32Value(state, spell_id);
	return 1;
}

int EQ2Emu_lua_StartTransmute(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state, 1);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: Lua StartTransmute command error: no spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsPlayer()) {
		lua_interface->LogError("%s: Lua StartTransmute command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	ZoneServer* zone = spawn->GetZone();
	if (!zone) {
		return 0;
	}

	Client* client = ((Player*)spawn)->GetClient();
	if (!client) {
		return 0;
	}

	Transmute::CreateItemRequest(client, static_cast<Player*>(spawn));
	return 0;
}

int EQ2Emu_lua_CompleteTransmute(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state, 1);
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("%s: Lua CompleteTransmute command error: no spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsPlayer()) {
		lua_interface->LogError("%s: Lua CompleteTransmute command error: spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	ZoneServer* zone = spawn->GetZone();
	if (!zone) {
		return 0;
	}

	Client* client = ((Player*)spawn)->GetClient();
	if (!client) {
		return 0;
	}

	Transmute::CompleteTransmutation(client, static_cast<Player*>(spawn));
	return 0;
}

int EQ2Emu_lua_ProcHate(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* caster = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	int32 threat_amt = lua_interface->GetInt32Value(state, 3);
	string spell_name = lua_interface->GetStringValue(state, 4);
	lua_interface->ResetFunctionStack(state);
	if (!caster) {
		lua_interface->LogError("%s: LUA ProcHate command error: caster is not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!caster->IsEntity()) {
		lua_interface->LogError("%s: LUA ProcHate command error: caster is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target) {
		lua_interface->LogError("%s: LUA ProcHate command error: target is not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target->IsEntity()) {
		lua_interface->LogError("%s: LUA ProcHate command error: target is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	static_cast<Entity*>(target)->AddHate(static_cast<Entity*>(caster), threat_amt);
	caster->GetZone()->SendThreatPacket(static_cast<Entity*>(caster), target, threat_amt, spell_name.c_str());
	return 0;
}

int EQ2Emu_lua_GiveExp(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 amount = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	
	if (player && player->IsPlayer() && amount > 0) {
		((Player*)player)->AddXP(amount);
	}
	return 0;
}

int EQ2Emu_lua_DisplayText(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int8 type = lua_interface->GetInt8Value(state, 2);
	string text = lua_interface->GetStringValue(state, 3);
	lua_interface->ResetFunctionStack(state);
	Client* client = 0;
	if (player && player->IsPlayer())
		client = ((Player*)player)->GetClient();
	if (!client || text.length() == 0) {
		lua_interface->LogError("%s: LUA DisplayText required parameters not given", lua_interface->GetScriptName(state));
		return 0;
	}
	client->SimpleMessage(type, text.c_str());
	return 0;
}

int EQ2Emu_lua_ShowLootWindow(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	Spawn* spawn = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);
	Client* client = 0;
	if (player && player->IsPlayer())
		client = ((Player*)player)->GetClient();
	if (!client || !spawn) {
		lua_interface->LogError("%s: LUA ShowLootWindow required parameters not given", lua_interface->GetScriptName(state));
		return 0;
	}
	vector<Item*>* items = ((Player*)player)->GetPendingLootItems(spawn->GetID());
	if (!items) {
		lua_interface->LogError("%s: LUA ShowLootWindow has no items", lua_interface->GetScriptName(state));
		return 0;
	}
	client->SendLootResponsePacket(spawn->GetLootCoins(), items, spawn, true);
	return 0;
}

int EQ2Emu_lua_GetRandomSpawnByID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawnref = lua_interface->GetSpawn(state);
	int32 spawn_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn_id > 0 && spawnref) {
		vector<Spawn*> spawns = spawnref->GetZone()->GetSpawnsByID(spawn_id);
		if (spawns.size() == 0) {
			lua_interface->LogError("%s: LUA EQ2Emu_lua_GetRandomSpawnByID command error: GetSpawnsByID returned no spawns", lua_interface->GetScriptName(state));
			return 0;
		}
		Spawn* spawn = 0;
		int16 index = MakeRandomInt(0, spawns.size());
		if (index >= spawns.size() || index < 0)
			index = 0;
		spawn = spawns[index];
		lua_interface->SetSpawnValue(state, spawn);
		return 1;
	}
	else {
		lua_interface->LogError("%s: LUA GetRandomSpawnByID required parameters not given", lua_interface->GetScriptName(state));
	}

	return 0;
}

int EQ2Emu_lua_AddPrimaryEntityCommandAllSpawns(lua_State* state) {
	Spawn* player = lua_interface->GetSpawn(state);
	int32 spawn_id = lua_interface->GetInt32Value(state, 2);
	string name = lua_interface->GetStringValue(state, 3);
	float distance = lua_interface->GetFloatValue(state, 4);
	string command = lua_interface->GetStringValue(state, 5);
	string error_text = lua_interface->GetStringValue(state, 6);
	int16 cast_time = lua_interface->GetInt16Value(state, 7);
	int32 spell_visual = lua_interface->GetInt32Value(state, 8);
	lua_interface->ResetFunctionStack(state);
	if (spawn_id && player && player->IsPlayer() && name.length() > 0) {
		if (distance == 0)
			distance = 10.0f;
		if (command.length() == 0)
			command = name;
		vector<Spawn*> spawns = player->GetZone()->GetSpawnsByID(spawn_id);
		if (spawns.size() == 0) {
			lua_interface->LogError("%s: LUA AddPrimaryEntityCommandAllSpawns command error: GetSpawnsByID returned no spawns", lua_interface->GetScriptName(state));
			return 0;
		}
		Spawn* spawn = 0;
		for (vector<Spawn*>::iterator itr = spawns.begin(); itr != spawns.end(); itr++) {
			spawn = *itr;
			if (spawn) {
				spawn->AddPrimaryEntityCommand(name.c_str(), distance, command.c_str(), error_text.c_str(), cast_time, spell_visual);
				player->GetZone()->SendUpdateDefaultCommand(spawn, command.c_str(), distance);
			}
		}

	}
	return 0;
}

int EQ2Emu_lua_InstructionWindowGoal(lua_State* state) {
	if (!lua_interface)
		return 0;
	Client* client = 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int8 goal_num = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer() && player->GetZone())
		client = ((Player*)player)->GetClient();
	else{
		lua_interface->LogError("LUA InstructionWindowGoal command error: player is not valid");
		return 0;
	}
	if (client) {
		PacketStruct* packet = configReader.getStruct("WS_InstructionWindow", client->GetVersion());
		if (packet) {
			packet->setDataByName("goal_num", goal_num);
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
	return 0;
}

int EQ2Emu_lua_InstructionWindowClose(lua_State* state) {
	if (!lua_interface)
		return 0;
	Client* client = 0;
	Spawn* player = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer() && player->GetZone())
		client = ((Player*)player)->GetClient();
	else {
		lua_interface->LogError("LUA InstructionWindowClose command error: player is not valid");
		return 0;
	}
	if (client && client->GetVersion() >= 374) {
		client->QueuePacket(new EQ2Packet(OP_EqInstructionWindowCloseCmd, 0, 0));
	}
	return 0;
}

int EQ2Emu_lua_InstructionWindow(lua_State* state) {
	if (!lua_interface)
		return 0;
	Client* client = 0;
	Spawn* player = lua_interface->GetSpawn(state);
	float duration = lua_interface->GetFloatValue(state, 2);
	string text = lua_interface->GetStringValue(state, 3);
	string voice = lua_interface->GetStringValue(state, 4);
	int32 voice_key1 = lua_interface->GetInt32Value(state, 5);
	int32 voice_key2 = lua_interface->GetInt32Value(state, 6);
	string signal = lua_interface->GetStringValue(state, 7);
	string goal1 = lua_interface->GetStringValue(state, 8);
	string task1 = lua_interface->GetStringValue(state, 9);
	string goal2 = lua_interface->GetStringValue(state, 10);
	string task2 = lua_interface->GetStringValue(state, 11);
	string goal3 = lua_interface->GetStringValue(state, 12);
	string task3 = lua_interface->GetStringValue(state, 13);
	string goal4 = lua_interface->GetStringValue(state, 14);
	string task4 = lua_interface->GetStringValue(state, 15);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("LUA InstructionWindow command error: spawn is not valid");
		return 0;
	}
	if (!player->IsPlayer()) {
		lua_interface->LogError("LUA InstructionWindow command error: spawn is not a player");
		return 0;
	}
	else
		client = ((Player*)player)->GetClient();

	if (!client) {
		lua_interface->LogError("LUA InstructionWindow command error: could not find client");
		return 0;
	}
	if (text.length() == 0) {
		lua_interface->LogError("LUA InstructionWindow required parameters not given");
		return 0;
	}
	if (duration >= 0 && duration < 2)
		duration = 2;
	PacketStruct* packet = configReader.getStruct("WS_InstructionWindow", client->GetVersion());
	if (packet) {
		packet->setDataByName("open_seconds_max", duration);
		packet->setDataByName("text", text.c_str());
		packet->setDataByName("voice", voice.c_str());
		int8 num_goals = 1;
		if (task2.length() > 0)
			num_goals++;
		if (task3.length() > 0)
			num_goals++;
		if (task4.length() > 0)
			num_goals++;
		packet->setArrayLengthByName("num_goals", num_goals);
		for (int8 i = 0; i < num_goals; i++) {
			packet->setSubArrayLengthByName("num_tasks", 1, i);
		}
		if (goal1.length() > 0)
			packet->setArrayDataByName("goal_text", goal1.c_str());
		if (goal2.length() > 0)
			packet->setArrayDataByName("goal_text", goal2.c_str(), 1);
		if (goal3.length() > 0)
			packet->setArrayDataByName("goal_text", goal3.c_str(), 2);
		if (goal4.length() > 0)
			packet->setArrayDataByName("goal_text", goal4.c_str(), 3);
		packet->setSubArrayDataByName("task_text", task1.c_str());
		if (task2.length() > 0)
			packet->setSubArrayDataByName("task_text", task2.c_str(), 1);
		if (task3.length() > 0)
			packet->setSubArrayDataByName("task_text", task3.c_str(), 2);
		if (task4.length() > 0)
			packet->setSubArrayDataByName("task_text", task4.c_str(), 3);
		packet->setDataByName("complete_sound", "click");
		packet->setDataByName("signal", signal.c_str());
		packet->setDataByName("voice_key1", voice_key1);
		packet->setDataByName("voice_key2", voice_key2);
		client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
	return 0;
}

int EQ2Emu_lua_ShowWindow(lua_State* state) {
	if (!lua_interface)
		return 0;
	Client* client = 0;
	Spawn* player = lua_interface->GetSpawn(state);
	string window = lua_interface->GetStringValue(state, 2);
	int8 show = lua_interface->GetInt8Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (!player) {
		lua_interface->LogError("LUA ShowWindow command error: spawn is not valid");
		return 0;
	}
	if (!player->IsPlayer()) {
		lua_interface->LogError("LUA ShowWindow command error: spawn is not a player");
		return 0;
	}
	else
		client = ((Player*)player)->GetClient();

	if (!client) {
		lua_interface->LogError("LUA ShowWindow command error: could not find client");
		return 0;
	}
	if (window.length() == 0) {
		lua_interface->LogError("LUA ShowWindow required parameters not given");
		return 0;
	}
	PacketStruct* packet = configReader.getStruct("WS_ShowWindow", client->GetVersion());
	if (packet) {
		packet->setDataByName("window", window.c_str());
		packet->setDataByName("show", show);
		client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
	return 0;
}

int EQ2Emu_lua_EnableGameEvent(lua_State* state) {
	//See GameEvents.txt for options that can be used for this function
	if (!lua_interface)
		return 0;
	Client* client = 0;
	Spawn* player = lua_interface->GetSpawn(state);
	string event_name = lua_interface->GetStringValue(state, 2);
	int8 enabled = lua_interface->GetInt8Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (!player || !player->IsPlayer()) {
		lua_interface->LogError("LUA EnableGameEvent error: player is not valid");
		return 0;
	}
	
		client = ((Player*)player)->GetClient();

	if (!client) {
		lua_interface->LogError("LUA EnableGameEvent error: could not find client");
		return 0;
	}
	PacketStruct* packet = configReader.getStruct("WS_EnableGameEvent", client->GetVersion());
	if (packet) {
		packet->setDataByName("event_name", event_name.c_str());
		packet->setDataByName("enabled", enabled);
		client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
	return 0;
}

int EQ2Emu_lua_GetTutorialStep(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer()) {
		lua_interface->SetInt32Value(state, ((Player*)player)->GetTutorialStep());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetTutorialStep(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int8 step = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer() && step > 0) {
		((Player*)player)->SetTutorialStep(step);
	}
	return 0;
}

int EQ2Emu_lua_FlashWindow(lua_State* state) {
	if (!lua_interface)
		return 0;
	Client* client = 0;
	Spawn* player = lua_interface->GetSpawn(state);
	string window = lua_interface->GetStringValue(state, 2);
	float flash_seconds = lua_interface->GetFloatValue(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (!player) {
		lua_interface->LogError("LUA FlashWindow command error: spawn is not valid");
		return 0;
	}
	if (!player->IsPlayer()) {
		lua_interface->LogError("LUA FlashWindow command error: spawn is not a player");
		return 0;
	}
	else
		client = ((Player*)player)->GetClient();

	if (!client) {
		lua_interface->LogError("LUA FlashWindow command error: could not find client");
		return 0;
	}
	if (window.length() == 0) {
		lua_interface->LogError("LUA FlashWindow required parameters not given");
		return 0;
	}
	PacketStruct* packet = configReader.getStruct("WS_FlashWindow", client->GetVersion());
	if (packet) {
		packet->setDataByName("window", window.c_str());
		packet->setDataByName("flash_seconds", flash_seconds);
		client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
	return 0;
}

int EQ2Emu_lua_CheckLOS(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && target)
		return spawn->CheckLoS(target);

	return 0;
}
int EQ2Emu_lua_CheckLOSByCoordinates(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	float x = lua_interface->GetFloatValue(state, 2);
	float y = lua_interface->GetFloatValue(state, 3);
	float z = lua_interface->GetFloatValue(state, 4);
	lua_interface->ResetFunctionStack(state);
	if (spawn)
		return spawn->CheckLoS(glm::vec3(spawn->GetX(), spawn->GetZ(), spawn->GetY() + 1.0f), glm::vec3(x, z, y+1.0f));

	return 0;
}

int EQ2Emu_lua_SetZoneExpansionFlag(lua_State* state) {
	if (!lua_interface)
		return 0;
	ZoneServer* zone = lua_interface->GetZone(state);
	int32 xpackFlag = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (zone)
		zone->SetExpansionFlag(xpackFlag);
	return 0;
}

int EQ2Emu_lua_GetZoneExpansionFlag(lua_State* state) {
	if (!lua_interface)
		return 0;
	ZoneServer* zone = lua_interface->GetZone(state);
	lua_interface->ResetFunctionStack(state);
	if (zone) {
		lua_interface->SetInt32Value(state, zone->GetExpansionFlag());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetZoneHolidayFlag(lua_State* state) {
	if (!lua_interface)
		return 0;
	ZoneServer* zone = lua_interface->GetZone(state);
	int32 holidayFlag = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (zone)
		zone->SetHolidayFlag(holidayFlag);
	return 0;
}

int EQ2Emu_lua_GetZoneHolidayFlag(lua_State* state) {
	if (!lua_interface)
		return 0;
	ZoneServer* zone = lua_interface->GetZone(state);
	lua_interface->ResetFunctionStack(state);
	if (zone) {
		lua_interface->SetInt32Value(state, zone->GetHolidayFlag());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetCanBind(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	bool canbind = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	
	if(!spawn) {
		lua_interface->LogError("%s: LUA SetCanBind command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	
	ZoneServer* zone = spawn->GetZone();
	if (zone)
		zone->SetCanBind(canbind);
	return 0;
}

int EQ2Emu_lua_GetCanBind(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	
	if(!spawn) {
		lua_interface->LogError("%s: LUA GetCanBind command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	
	ZoneServer* zone = spawn->GetZone();
	if (zone) {
		lua_interface->SetInt32Value(state, zone->GetCanBind());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetCanGate(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	Spawn* spawn = lua_interface->GetSpawn(state);
	bool cangate = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	
	if(!spawn) {
		lua_interface->LogError("%s: LUA SetCanGate command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	
	ZoneServer* zone = spawn->GetZone();
	if (zone)
		zone->SetCanGate(cangate);
	return 0;
}

int EQ2Emu_lua_GetCanGate(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	
	if(!spawn) {
		lua_interface->LogError("%s: LUA GetCanGate command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	
	ZoneServer* zone = spawn->GetZone();
	if (zone) {
		lua_interface->SetInt32Value(state, zone->GetCanGate());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetCanEvac(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	Spawn* spawn = lua_interface->GetSpawn(state);
	bool canevac = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	
	if(!spawn) {
		lua_interface->LogError("%s: LUA SetCanEvac command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	
	ZoneServer* zone = spawn->GetZone();
	if (zone)
		zone->SetCanEvac(canevac);
	return 0;
}

int EQ2Emu_lua_GetCanEvac(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	
	if(!spawn) {
		lua_interface->LogError("%s: LUA GetCanEvac command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	
	ZoneServer* zone = spawn->GetZone();
	if (zone) {
		lua_interface->SetInt32Value(state, zone->GetCanEvac());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_AddSpawnProximity(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 spawn_value = lua_interface->GetInt32Value(state, 2);
	int8 spawn_type = lua_interface->GetInt8Value(state, 3);
	float distance = lua_interface->GetFloatValue(state, 4);
	string in_range_function = lua_interface->GetStringValue(state, 5);
	string leaving_range_function = lua_interface->GetStringValue(state, 6);
	lua_interface->ResetFunctionStack(state);
	if (spawn && distance > 0 && in_range_function.length() > 0)
		spawn->AddLUASpawnProximity(spawn_value, (Spawn::SpawnProximityType)spawn_type, distance, in_range_function, leaving_range_function);
	return 0;
}

int EQ2Emu_lua_CanSeeInvis(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* target = lua_interface->GetSpawn(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn && target)
	{
		if (spawn->IsPlayer() && target->IsEntity())
		{
			lua_interface->SetBooleanValue(state, ((Player*)spawn)->CanSeeInvis((Entity*)target));
			return 1;
		}
		else if (spawn->IsEntity() && target->IsEntity())
		{
			lua_interface->SetBooleanValue(state, ((Entity*)spawn)->CanSeeInvis((Entity*)target));
			return 1;
		}
	}

	return 0;
}

int EQ2Emu_lua_SetSeeInvis(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	bool val = (lua_interface->GetInt8Value(state, 2) == 1);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity())
	{
		((Entity*)spawn)->SetSeeInvisSpell(val);
		if (spawn->IsPlayer())
		{
			Client* client = ((Player*)spawn)->GetClient();
			if (client)
				((Player*)spawn)->GetZone()->SendAllSpawnsForSeeInvisChange(client);
		}
	}

	return 0;
}

int EQ2Emu_lua_SetSeeHide(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	bool val = (lua_interface->GetInt8Value(state, 2) == 1);
	lua_interface->ResetFunctionStack(state);
	if (spawn && spawn->IsEntity())
	{
		((Entity*)spawn)->SetSeeHideSpell(val);
		if (spawn->IsPlayer())
		{
			Client* client = ((Player*)spawn)->GetClient();
			if (client)
				((Player*)spawn)->GetZone()->SendAllSpawnsForVisChange(client);
		}
	}

	return 0;
}


int EQ2Emu_lua_SetAccessToEntityCommand(lua_State* state)
{
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	Spawn* spawn = lua_interface->GetSpawn(state, 2);
	string command = lua_interface->GetStringValue(state, 3);
	bool val = (lua_interface->GetInt8Value(state, 4) == 1);

	lua_interface->ResetFunctionStack(state);
	if (spawn && player && player->IsPlayer())
	{
		EntityCommand* cmd = spawn->FindEntityCommand(string(command), true);
		bool res = false;
		if (cmd)
			res = spawn->SetPermissionToEntityCommand(cmd, (Player*)player, val);

		lua_interface->SetBooleanValue(state, res);
		return 1;
	}

	return 0;
}


int EQ2Emu_lua_SetAccessToEntityCommandByCharID(lua_State* state)
{
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 charID = lua_interface->GetInt32Value(state, 2);
	string command = lua_interface->GetStringValue(state, 3);
	bool val = (lua_interface->GetInt8Value(state, 4) == 1);

	lua_interface->ResetFunctionStack(state);
	if (spawn && charID)
	{
		EntityCommand* cmd = spawn->FindEntityCommand(string(command), true);
		bool res = false;
		if (cmd)
			res = spawn->SetPermissionToEntityCommandByCharID(cmd, charID, val);

		lua_interface->SetBooleanValue(state, res);
		return 1;
	}

	return 0;
}

int EQ2Emu_lua_RemovePrimaryEntityCommand(lua_State* state)
{
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string command = lua_interface->GetStringValue(state, 2);

	lua_interface->ResetFunctionStack(state);
	if (spawn && command.length() > 0)
		spawn->RemovePrimaryEntityCommand(command.c_str());

	return 0;
}


int EQ2Emu_lua_SendUpdateDefaultCommand(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	float distance = lua_interface->GetFloatValue(state, 2);
	string command = lua_interface->GetStringValue(state, 3);
	Spawn* player = lua_interface->GetSpawn(state, 4);

	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->GetZone()->SendUpdateDefaultCommand(spawn, command.c_str(), distance, player);
	}
	return 0;
}

int EQ2Emu_lua_SendTransporters(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	Spawn* spawn = lua_interface->GetSpawn(state);
	Spawn* player = lua_interface->GetSpawn(state, 2);
	int32 transport_id = lua_interface->GetInt32Value(state, 3);

	lua_interface->ResetFunctionStack(state);
	if (spawn && player && transport_id && player->IsPlayer()) {
		Client* client = 0;
		if (player && player->IsPlayer())
			client = ((Player*)player)->GetClient();

		if (!client)
			return 0;

		vector<TransportDestination*> destinations;
		player->GetZone()->GetTransporters(&destinations, client, transport_id);

		if (destinations.size())
		{
			client->SetTemporaryTransportID(transport_id);
			client->ProcessTeleport(spawn, &destinations, transport_id, (spell != nullptr) ? true : false);
		}
		else
			client->Message(CHANNEL_COLOR_RED, "There are no transporters available (ID: %u)", transport_id);
	}
	return 0;
}

int EQ2Emu_lua_SetTemporaryTransportID(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	int32 transport_id = lua_interface->GetInt32Value(state, 2);

	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer()) {
		Client* client = 0;
		if (player && player->IsPlayer())
			client = ((Player*)player)->GetClient();

		if (!client)
			return 0;

		client->SetTemporaryTransportID(transport_id);
	}
	return 0;
}

int EQ2Emu_lua_GetTemporaryTransportID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);

	lua_interface->ResetFunctionStack(state);
	if (player && player->IsPlayer()) {
		Client* client = 0;
		if (player && player->IsPlayer())
			client = ((Player*)player)->GetClient();

		if (!client)
			return 0;

		lua_interface->SetInt32Value(state, client->GetTemporaryTransportID());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetAlignment(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	sint32 alignment = lua_interface->GetSInt32Value(state, 2);
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);

	if (!spawn) {
		lua_interface->LogError("%s: LUA SetAlignment command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA SetAlignment command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}

	if (alignment < SCHAR_MIN || alignment > SCHAR_MAX)
	{
		lua_interface->LogError("%s: LUA SetAlignment command error: alignment value beyond supported min: %i and max: %i", lua_interface->GetScriptName(state), SCHAR_MIN, SCHAR_MAX);
		return 0;
	}

	lua_interface->ResetFunctionStack(state);

	if (spell && spell->targets.size() > 0) {
		ZoneServer* zone = spell->caster->GetZone();
		for (int8 i = 0; i < spell->targets.size(); i++) {
			Spawn* target = zone->GetSpawnByID(spell->targets.at(i));
			if (target && target->IsEntity()) {
				((Entity*)target)->GetInfoStruct()->set_alignment((sint8)alignment);
				if (target->IsPlayer())
					((Player*)target)->SetCharSheetChanged(true);
			}
		}
	}
	else {
		((Entity*)spawn)->GetInfoStruct()->set_alignment((sint8)alignment);
		if (spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}

int EQ2Emu_lua_GetAlignment(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);

	lua_interface->ResetFunctionStack(state);
	
	if (!spawn) {
		lua_interface->LogError("%s: LUA GetAlignment command error: spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!spawn->IsEntity()) {
		lua_interface->LogError("%s: LUA GetAlignment command error: spawn is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	
	lua_interface->SetSInt32Value(state, ((Entity*)spawn)->GetAlignment());
	return 1;
}


int EQ2Emu_lua_GetSpell(lua_State* state) {
	if (!lua_interface)
		return 0;
	int32 spell_id = lua_interface->GetInt32Value(state);
	int8 spell_tier = lua_interface->GetInt8Value(state, 2);
	string custom_lua_script = lua_interface->GetStringValue(state, 3);
	if (spell_id > 0) {

		if (spell_tier == 0)
			spell_tier = 1;

		Spell* spell = master_spell_list.GetSpell(spell_id, spell_tier);
		
		if(!spell) {
			lua_interface->LogError("%s: GetSpell: Failed, spell id %u spell tier %u does not exist.", lua_interface->GetScriptName(state), spell_id, spell_tier);
			lua_interface->ResetFunctionStack(state);
			return 0;
		}
		
		LuaSpell* lua_spell = 0;
		if(custom_lua_script.size() > 0)
		{
			// attempt to load the custom script since it isn't already loaded
			// we will re-obtain the lua_spell further below
			if((lua_spell = lua_interface->GetSpell(custom_lua_script.c_str())) == nullptr)
			{		
				LogWrite(LUA__WARNING, 0, "LUA", "GetSpell(%u, %u, '%s'), custom lua script not loaded, attempting to load.", spell_id, spell_tier, custom_lua_script.c_str());
			}
		}
		else
			custom_lua_script = spell->GetSpellData()->lua_script;
		

		if (!lua_spell && lua_interface)
			lua_spell = lua_interface->GetSpell(custom_lua_script.c_str());

		lua_interface->ResetFunctionStack(state);
		
		if (!lua_spell)
		{
			LogWrite(LUA__ERROR, 0, "LUA", "GetSpell(%u, %u, '%s') spell could not be loaded.", spell_id, spell_tier, custom_lua_script.c_str());
			return 0;
		}

		lua_spell->spell = new Spell(spell);

		lua_interface->AddCustomSpell(lua_spell);

		lua_interface->SetSpellValue(state, lua_spell);
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetSpellData(lua_State* state) {
	if (!lua_interface)
		return 0;
	LuaSpell* spell = lua_interface->GetSpell(state);
	string field = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!spell) {
		lua_interface->LogError("%s: Spell not given in GetSpellData!", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spell->spell || !spell->spell->GetSpellData()) {
		lua_interface->LogError("%s: Inner Spell or SpellData not given in GetSpellData!", lua_interface->GetScriptName(state));
		return 0;
	}

	boost::to_lower(field);


	return spell->spell->GetSpellData(state, field);
}


int EQ2Emu_lua_SetSpellData(lua_State* state) {
	if (!lua_interface)
		return 0;
	LuaSpell* spell = lua_interface->GetSpell(state);
	string field = lua_interface->GetStringValue(state, 2);
	int8 fieldArg = 3; // field value after the initial set

	if (!spell) {
		lua_interface->LogError("%s: Spell not given in SetSpellData!", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}
	if (!spell->spell || !spell->spell->GetSpellData()) {
		lua_interface->LogError("%s: Inner Spell or SpellData not given in SetSpellData!", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	boost::to_lower(field);

	bool valSet = false;

	spell->spell->SetSpellData(state, field, fieldArg);
	lua_interface->ResetFunctionStack(state);

	return valSet;
}

int EQ2Emu_lua_SetSpellDataIndex(lua_State* state) {
	if (!lua_interface)
		return 0;
	LuaSpell* spell = lua_interface->GetSpell(state);
	int8 idx = lua_interface->GetInt32Value(state, 2);

	if (!spell) {
		lua_interface->LogError("%s: Spell not given in SetSpellDataIndex!", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}
	if (!spell->spell || !spell->spell->GetSpellData()) {
		lua_interface->LogError("%s: Inner Spell or SpellData not given in SetSpellDataIndex!", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	if (spell->spell->lua_data.size() <= idx)
	{
		lua_interface->LogError("%s: lua_data size %i <= %i (idx passed) SetSpellDataIndex!", lua_interface->GetScriptName(state), spell->spell->lua_data.size(), idx);
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	bool setVal = true;

	LUAData* data = spell->spell->lua_data[idx];

	switch (data->type)
	{
	case 0:
	{
		sint32 value = lua_interface->GetSInt32Value(state, 3);
		sint32 value2 = lua_interface->GetSInt32Value(state, 4);
		data->int_value = value;
		data->int_value2 = value2;
		break;
	}
	case 1:
	{
		float value = lua_interface->GetFloatValue(state, 3);
		float value2 = lua_interface->GetFloatValue(state, 4);
		data->float_value = value;
		data->float_value2 = value2;
		break;
	}
	case 2:
	{
		bool value = lua_interface->GetBooleanValue(state, 3);
		data->bool_value = value;
		break;
	}
	case 3:
	{
		string value = lua_interface->GetStringValue(state, 3);
		string value2 = lua_interface->GetStringValue(state, 4);
		data->string_value = value;
		data->string_value2 = value2;
		break;
	}
	default:
		setVal = false;
	}
	
	lua_interface->ResetFunctionStack(state);

	return setVal;
}


int EQ2Emu_lua_GetSpellDataIndex(lua_State* state) {
	if (!lua_interface)
		return 0;
	LuaSpell* spell = lua_interface->GetSpell(state);
	int8 idx = lua_interface->GetInt32Value(state, 2);
	bool secondfield = lua_interface->GetBooleanValue(state, 3);
	lua_interface->ResetFunctionStack(state);

	if (!spell) {
		lua_interface->LogError("%s: Spell not given in GetSpellDataIndex!", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spell->spell || !spell->spell->GetSpellData()) {
		lua_interface->LogError("%s: Inner Spell or SpellData not given in GetSpellDataIndex!", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spell->spell->lua_data.size() <= idx)
	{
		lua_interface->LogError("%s: lua_data size %i <= %i (idx passed) GetSpellDataIndex!", lua_interface->GetScriptName(state), spell->spell->lua_data.size(), idx);
		return 0;
	}

	bool setVal = true;

	LUAData* data = spell->spell->lua_data[idx];

	switch (data->type)
	{
	case 0:
	{
		if(!secondfield)
			lua_interface->SetSInt32Value(state, data->int_value);
		else
			lua_interface->SetSInt32Value(state, data->int_value2);
		break;
	}
	case 1:
	{
		if (!secondfield)
			lua_interface->SetFloatValue(state, data->float_value);
		else
			lua_interface->SetFloatValue(state, data->float_value2);
		break;
	}
	case 2:
	{
		lua_interface->SetBooleanValue(state, data->bool_value);
		break;
	}
	case 3:
	{
		if (!secondfield)
			lua_interface->SetStringValue(state, data->string_value.c_str());
		else
			lua_interface->SetStringValue(state, data->string_value2.c_str());
		break;
	}
	default:
		setVal = false;
	}

	return setVal;
}


int EQ2Emu_lua_SetSpellDisplayEffect(lua_State* state) {
	if (!lua_interface)
		return 0;
	LuaSpell* spell = lua_interface->GetSpell(state);
	int8 idx = lua_interface->GetInt32Value(state, 2);
	string field = lua_interface->GetStringValue(state, 3);

	boost::to_lower(field);

	if (!spell) {
		lua_interface->LogError("%s: Spell not given in SetSpellDisplayEffect!", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}
	if (!spell->spell || !spell->spell->GetSpellData()) {
		lua_interface->LogError("%s: Inner Spell or SpellData not given in SetSpellDisplayEffect!", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	if (spell->spell->effects.size() <= idx)
	{
		lua_interface->LogError("%s: lua_data size %i <= %i (idx passed) SetSpellDisplayEffect!", lua_interface->GetScriptName(state), spell->spell->lua_data.size(), idx);
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	// do we need to lock? eh probably not this should only be used before use of the custom spell
	SpellDisplayEffect* effect = spell->spell->effects[idx];

	if (field == "description")
		effect->description = string(lua_interface->GetStringValue(state, 4));
	else if (field == "bullet")
		effect->subbullet = lua_interface->GetInt8Value(state, 4);
	else if (field == "percentage")
		effect->percentage = lua_interface->GetInt8Value(state, 4);
	else { // no match
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	lua_interface->ResetFunctionStack(state);
	
	return 1;
}

int EQ2Emu_lua_GetSpellDisplayEffect(lua_State* state) {
	if (!lua_interface)
		return 0;
	LuaSpell* spell = lua_interface->GetSpell(state);
	int8 idx = lua_interface->GetInt32Value(state, 2);
	string field = lua_interface->GetStringValue(state, 3);

	lua_interface->ResetFunctionStack(state);
	
	boost::to_lower(field);

	if (!spell) {
		lua_interface->LogError("%s: Spell not given in GetSpellDisplayEffect!", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!spell->spell || !spell->spell->GetSpellData()) {
		lua_interface->LogError("%s: Inner Spell or SpellData not given in GetSpellDisplayEffect!", lua_interface->GetScriptName(state));
		return 0;
	}

	if (spell->spell->effects.size() <= idx)
	{
		lua_interface->LogError("%s: lua_data size %i <= %i (idx passed) GetSpellDisplayEffect!", lua_interface->GetScriptName(state), spell->spell->lua_data.size(), idx);
		return 0;
	}

	// do we need to lock? eh probably not this should only be used before use of the custom spell
	SpellDisplayEffect* effect = spell->spell->effects[idx];

	if (field == "description")
		lua_interface->SetStringValue(state, effect->description.c_str());
	else if (field == "bullet")
		lua_interface->SetInt32Value(state, effect->subbullet);
	else if (field == "percentage")
		lua_interface->SetInt32Value(state, effect->percentage);
	else // no match
		return 0;


	return 1;
}
int EQ2Emu_lua_CastCustomSpell(lua_State* state) {
	if (!lua_interface)
		return 0;
	LuaSpell* spell = lua_interface->GetSpell(state);
	Spawn* caster = lua_interface->GetSpawn(state, 2);
	Spawn* target = lua_interface->GetSpawn(state, 3);
	lua_interface->ResetFunctionStack(state);

	if (!target) {
		lua_interface->LogError("%s: LUA CastCustomSpell command error: target is not a valid spawn", lua_interface->GetScriptName(state));
		return 0;
	}

	if (!target->IsEntity()) {
		lua_interface->LogError("%s: LUA CastCustomSpell command error: target (%s) is not an entity", lua_interface->GetScriptName(state), target->GetName());
		return 0;
	}

	if (!spell) {
		lua_interface->LogError("%s: LUA CastCustomSpell command error: spell is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	if (caster && !caster->IsEntity()) {
		lua_interface->LogError("%s: LUA CastSpell command error: caster (%s) is not an entity", lua_interface->GetScriptName(state), caster->GetName());
		return 0;
	}

	target->GetZone()->ProcessSpell(NULL, (Entity*)caster, (Entity*)target, true, false, spell, 0);
	return 0;
}

int EQ2Emu_lua_InWater(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetBooleanValue(state, spawn->InWater());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_InLava(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetBooleanValue(state, spawn->InLava());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_DamageSpawn(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* attacker = lua_interface->GetSpawn(state);
	Spawn* victim = lua_interface->GetSpawn(state, 2);
	int8 type = lua_interface->GetInt8Value(state, 3);
	int8 dmg_type = lua_interface->GetInt8Value(state, 4);
	int32 low_damage = lua_interface->GetInt32Value(state, 5);
	int32 high_damage = lua_interface->GetInt32Value(state, 6);
	string spell_name = lua_interface->GetStringValue(state, 7);
	int8 crit_mod = lua_interface->GetInt8Value(state, 8);
	bool is_tick = (lua_interface->GetInt8Value(state, 9) == 1);
	bool no_calcs = (lua_interface->GetInt8Value(state, 10) == 1);
	bool ignore_attacker = (lua_interface->GetInt8Value(state, 11) == 1);
	bool take_power = (lua_interface->GetInt8Value(state, 12) == 1);

	lua_interface->ResetFunctionStack(state);
	if (!attacker) {
		lua_interface->LogError("%s: LUA ProcDamage command error: caster is not a valid spawn", lua_interface->GetScriptName(state));
		lua_interface->SetBooleanValue(state, false);
		return 1;

	}

	if (!attacker->IsEntity()) {
		lua_interface->LogError("%s: LUA ProcDamage command error: caster is not an entity", lua_interface->GetScriptName(state));
		lua_interface->SetBooleanValue(state, false);
		return 1;

	}

	if (!victim) {
		lua_interface->LogError("%s: LUA ProcDamage command error: target is not a valid spawn", lua_interface->GetScriptName(state));
		lua_interface->SetBooleanValue(state, false);
		return 1;

	}

	if (!victim->IsEntity()) {
		lua_interface->LogError("%s: LUA ProcDamage command error: target is not an entity", lua_interface->GetScriptName(state));
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	bool has_damaged = ((Entity*)attacker)->DamageSpawn((Entity*)victim, type, dmg_type, low_damage, high_damage, spell_name.c_str(), crit_mod, is_tick, no_calcs, ignore_attacker, take_power);
	lua_interface->SetBooleanValue(state, has_damaged);
	return 1;
}

int EQ2Emu_lua_IsInvulnerable(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		lua_interface->SetBooleanValue(state, spawn->GetInvulnerable());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetInvulnerable(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	bool invul = lua_interface->GetBooleanValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		spawn->SetInvulnerable(invul);
	}
	return 0;
}

int EQ2Emu_lua_GetRuleFlagBool(lua_State* state) {
	if (!lua_interface)
		return 0;
	string category = lua_interface->GetStringValue(state);
	string name = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	Rule *ret = 0;
	if ((ret = rule_manager.GetGlobalRule(category.c_str(), name.c_str()))) {
		
		lua_interface->SetBooleanValue(state, ret->GetBool());
		return 1;
	}
	
	lua_interface->LogError("%s: LUA GetRuleFlagBool Unknown rule with category '%s' and type '%s'", lua_interface->GetScriptName(state), category.c_str(), name.c_str());
	return 0;
}

int EQ2Emu_lua_GetRuleFlagInt32(lua_State* state) {
	if (!lua_interface)
		return 0;
	string category = lua_interface->GetStringValue(state);
	string name = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	Rule *ret = 0;
	if ((ret = rule_manager.GetGlobalRule(category.c_str(), name.c_str()))) {
		
		lua_interface->SetInt32Value(state, ret->GetInt32());
		return 1;
	}
	
	lua_interface->LogError("%s: LUA GetRuleFlagInt32 Unknown rule with category '%s' and type '%s'", lua_interface->GetScriptName(state), category.c_str(), name.c_str());
	return 0;
}

int EQ2Emu_lua_GetRuleFlagFloat(lua_State* state) {
	if (!lua_interface)
		return 0;
	string category = lua_interface->GetStringValue(state);
	string name = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	Rule *ret = 0;
	if ((ret = rule_manager.GetGlobalRule(category.c_str(), name.c_str()))) {
		
		lua_interface->SetFloatValue(state, ret->GetFloat());
		return 1;
	}
	
	lua_interface->LogError("%s: LUA GetRuleFlagFloat Unknown rule with category '%s' and type '%s'", lua_interface->GetScriptName(state), category.c_str(), name.c_str());
	return 0;
}


int EQ2Emu_lua_GetAAInfo(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	string type = lua_interface->GetStringValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		int res = 1;

		boost::to_lower(type);
		if(type == "assigned_aa")
			lua_interface->SetSInt32Value(state, spawn->GetAssignedAA());
		else if ( type == "unassigned_aa")
			lua_interface->SetSInt32Value(state, spawn->GetUnassignedAA());
		else if ( type == "assigned_tradeskill_aa")
			lua_interface->SetSInt32Value(state, spawn->GetTradeskillAA());
		else if ( type == "unassigned_tradeskill_aa")
			lua_interface->SetSInt32Value(state, spawn->GetUnassignedTradeskillAA());
		else if ( type == "assigned_prestige_aa")
			lua_interface->SetSInt32Value(state, spawn->GetPrestigeAA());
		else if ( type == "unassigned_prestige_aa")
			lua_interface->SetSInt32Value(state, spawn->GetUnassignedPretigeAA());
		else if ( type == "assigned_tradeskill_prestige_aa")
			lua_interface->SetSInt32Value(state, spawn->GetTradeskillPrestigeAA());
		else if ( type == "unassigned_tradeskill_prestige_aa")
			lua_interface->SetSInt32Value(state, spawn->GetUnassignedTradeskillPrestigeAA());
		else
			res = 0;

		return res;
	}
	
	lua_interface->LogError("%s: LUA GetAAInfo spawn does not exist", lua_interface->GetScriptName(state));
	return 0;
}

int EQ2Emu_lua_SetAAInfo(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string type = lua_interface->GetStringValue(state, 2);
	sint32 value = lua_interface->GetSInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (spawn) {
		boost::to_lower(type);
		if(type == "assigned_aa")
			spawn->SetAssignedAA((sint16)value);
		else if ( type == "unassigned_aa")
			spawn->SetUnassignedAA((sint16)value);
		else if ( type == "assigned_tradeskill_aa")
			spawn->SetTradeskillAA((sint16)value);
		else if ( type == "unassigned_tradeskill_aa")
			spawn->SetUnassignedTradeskillAA((sint16)value);
		else if ( type == "assigned_prestige_aa")
			spawn->SetPrestigeAA((sint16)value);
		else if ( type == "unassigned_prestige_aa")
			spawn->SetUnassignedPrestigeAA((sint16)value);
		else if ( type == "assigned_tradeskill_prestige_aa")
			spawn->SetTradeskillPrestigeAA((sint16)value);
		else if ( type == "unassigned_tradeskill_prestige_aa")
			spawn->SetUnassignedTradeskillPrestigeAA((sint16)value);

		if(spawn->IsPlayer())
			((Player*)spawn)->SetCharSheetChanged(true);
	}
	return 0;
}

int EQ2Emu_lua_AddMasterTitle(lua_State* state) {
	if (!lua_interface)
		return 0;

	string titleName = lua_interface->GetStringValue(state);
	int8 isPrefix = lua_interface->GetInt8Value(state, 2);
	
	lua_interface->ResetFunctionStack(state);
	sint32 index = database.AddMasterTitle(titleName.c_str(), isPrefix);
	lua_interface->SetSInt32Value(state, index);

	return 1;
}

int EQ2Emu_lua_AddCharacterTitle(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string titleName = lua_interface->GetStringValue(state, 2);
	
	lua_interface->ResetFunctionStack(state);
	if(!spawn->IsPlayer())
	{
		lua_interface->LogError("%s: LUA AddCharacterTitle command error: player is not valid", lua_interface->GetScriptName(state));
		lua_interface->SetSInt32Value(state, -1);
		return 1;
	}

	Player* player = (Player*)spawn;
	// check if player already has the title, don't need to add twice
	Title* playerHasTitle = player->GetPlayerTitles()->GetTitleByName(titleName.c_str());

	if ( playerHasTitle)
	{
		lua_interface->SetSInt32Value(state, playerHasTitle->GetID());
		return 1;
	}

	Title* title = master_titles_list.GetTitleByName(titleName.c_str());
	
	if(!title)
	{
		lua_interface->LogError("%s: LUA AddCharacterTitle command error: title is not valid with name '%s'", lua_interface->GetScriptName(state), titleName.c_str());
		lua_interface->SetSInt32Value(state, -1);
		return 1;
	}
	

	sint32 returnIdx = database.AddCharacterTitle(title->GetID(), player->GetCharacterID(), player);

	if(returnIdx < 0)
	{
		lua_interface->LogError("%s: LUA AddCharacterTitle command error: got invalid index (-1) returned for database.AddCharacterTitle '%s'", lua_interface->GetScriptName(state), titleName.c_str());
	}

	lua_interface->SetSInt32Value(state, returnIdx);

	player->GetClient()->SendTitleUpdate();

	return 1;
}

int EQ2Emu_lua_SetCharacterTitleSuffix(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string titleName = lua_interface->GetStringValue(state, 2);
	
	lua_interface->ResetFunctionStack(state);
	if(!spawn->IsPlayer())
	{
		lua_interface->LogError("%s: LUA SetCharacterTitleSuffix command error: player is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	Player* player = (Player*)spawn;

	Title* title = player->GetPlayerTitles()->GetTitleByName(titleName.c_str());
	
	if(!title)
	{
		lua_interface->LogError("%s: LUA SetCharacterTitleSuffix command error: title is not valid with name '%s'", lua_interface->GetScriptName(state), titleName.c_str());
		return 0;
	}
	
	if(title->GetPrefix())
	{
		lua_interface->LogError("%s: LUA SetCharacterTitleSuffix command error: title with name '%s' is not valid as a suffix, only prefix", lua_interface->GetScriptName(state), titleName.c_str());
		return 0;
	}

	database.SaveCharSuffixIndex(title->GetID(), player->GetCharacterID());
	player->GetClient()->SendTitleUpdate();

	return 1;
}

int EQ2Emu_lua_SetCharacterTitlePrefix(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string titleName = lua_interface->GetStringValue(state, 2);
	
	lua_interface->ResetFunctionStack(state);
	if(!spawn->IsPlayer())
	{
		lua_interface->LogError("%s: LUA SetCharacterTitlePrefix command error: player is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	Player* player = (Player*)spawn;

	Title* title = player->GetPlayerTitles()->GetTitleByName(titleName.c_str());

	if(!title)
	{
		lua_interface->LogError("%s: LUA SetCharacterTitlePrefix command error: title is not valid with name '%s'", lua_interface->GetScriptName(state), titleName.c_str());
		return 0;
	}
	
	if(!title->GetPrefix())
	{
		lua_interface->LogError("%s: LUA SetCharacterTitlePrefix command error: title with name '%s' is not valid as a prefix, only suffix", lua_interface->GetScriptName(state), titleName.c_str());
		return 0;
	}
	
	database.SaveCharPrefixIndex(title->GetID(), player->GetCharacterID());
	player->GetClient()->SendTitleUpdate();

	return 1;
}

int EQ2Emu_lua_ResetCharacterTitleSuffix(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	
	lua_interface->ResetFunctionStack(state);
	if(!spawn)
	{
		lua_interface->LogError("%s: LUA ResetCharacterTitleSuffix command error: spawn is null", lua_interface->GetScriptName(state));
		return 0;
	}

	if(!spawn->IsPlayer())
	{
		lua_interface->LogError("%s: LUA ResetCharacterTitleSuffix command error: player is not valid", lua_interface->GetScriptName(state));
		return 0;
	}

	Player* player = (Player*)spawn;


	database.SaveCharSuffixIndex(-1, player->GetCharacterID());
	player->GetClient()->SendTitleUpdate();

	return 1;
}

int EQ2Emu_lua_ResetCharacterTitlePrefix(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	
	lua_interface->ResetFunctionStack(state);
	
	if(!spawn)
	{
		lua_interface->LogError("%s: LUA ResetCharacterTitlePrefix command error: spawn is null", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(!spawn->IsPlayer())
	{
		lua_interface->LogError("%s: LUA ResetCharacterTitlePrefix command error: player is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	
	Player* player = (Player*)spawn;


	database.SaveCharPrefixIndex(-1, player->GetCharacterID());
	player->GetClient()->SendTitleUpdate();

	return 1;
}

int EQ2Emu_lua_GetInfoStructString(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string field = lua_interface->GetStringValue(state, 2);
	
	lua_interface->ResetFunctionStack(state);
	if(!spawn || !spawn->IsEntity())
	{
		lua_interface->LogError("%s: LUA GetInfoStructString command error: spawn is not valid, either does not exist or is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	Entity* ent = (Entity*)spawn;
	lua_interface->SetStringValue(state, ent->GetInfoStructString(field).c_str());

	return 1;
}

int EQ2Emu_lua_GetInfoStructUInt(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string field = lua_interface->GetStringValue(state, 2);
	
	lua_interface->ResetFunctionStack(state);
	if(!spawn || !spawn->IsEntity())
	{
		lua_interface->LogError("%s: LUA GetInfoStructUInt command error: spawn is not valid, either does not exist or is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	Entity* ent = (Entity*)spawn;
	lua_interface->SetInt64Value(state, ent->GetInfoStructUInt(field));

	return 1;
}

int EQ2Emu_lua_GetInfoStructSInt(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string field = lua_interface->GetStringValue(state, 2);
	
	lua_interface->ResetFunctionStack(state);
	if(!spawn || !spawn->IsEntity())
	{
		lua_interface->LogError("%s: LUA GetInfoStructSInt command error: spawn is not valid, either does not exist or is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	Entity* ent = (Entity*)spawn;
	lua_interface->SetSInt64Value(state, ent->GetInfoStructSInt(field));

	return 1;
}

int EQ2Emu_lua_GetInfoStructFloat(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string field = lua_interface->GetStringValue(state, 2);
	
	lua_interface->ResetFunctionStack(state);
	if(!spawn || !spawn->IsEntity())
	{
		lua_interface->LogError("%s: LUA GetInfoStructFloat command error: spawn is not valid, either does not exist or is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	Entity* ent = (Entity*)spawn;
	lua_interface->SetFloatValue(state, ent->GetInfoStructFloat(field));

	return 1;
}

int EQ2Emu_lua_SetInfoStructString(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string field = lua_interface->GetStringValue(state, 2);
	string value = lua_interface->GetStringValue(state, 3);
	
	lua_interface->ResetFunctionStack(state);
	if(!spawn || !spawn->IsEntity())
	{
		lua_interface->LogError("%s: LUA SetInfoStructString command error: spawn is not valid, either does not exist or is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	Entity* ent = (Entity*)spawn;
	bool set_ = ent->SetInfoStructString(field, value);
	lua_interface->SetBooleanValue(state, set_);
	return 1;
}

int EQ2Emu_lua_SetInfoStructUInt(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string field = lua_interface->GetStringValue(state, 2);
	int64 value = lua_interface->GetInt64Value(state, 3);
	
	lua_interface->ResetFunctionStack(state);
	if(!spawn || !spawn->IsEntity())
	{
		lua_interface->LogError("%s: LUA SetInfoStructUInt command error: spawn is not valid, either does not exist or is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	Entity* ent = (Entity*)spawn;
	bool set_ = ent->SetInfoStructUInt(field, value);
	lua_interface->SetBooleanValue(state, set_);
	return 1;
}

int EQ2Emu_lua_SetInfoStructSInt(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string field = lua_interface->GetStringValue(state, 2);
	sint64 value = lua_interface->GetSInt64Value(state, 3);
	
	lua_interface->ResetFunctionStack(state);
	if(!spawn || !spawn->IsEntity())
	{
		lua_interface->LogError("%s: LUA SetInfoStructSInt command error: spawn is not valid, either does not exist or is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	Entity* ent = (Entity*)spawn;
	bool set_ = ent->SetInfoStructSInt(field, value);
	lua_interface->SetBooleanValue(state, set_);
	return 1;
}

int EQ2Emu_lua_SetInfoStructFloat(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	string field = lua_interface->GetStringValue(state, 2);
	float value = lua_interface->GetFloatValue(state, 3);
	
	lua_interface->ResetFunctionStack(state);
	if(!spawn || !spawn->IsEntity())
	{
		lua_interface->LogError("%s: LUA SetInfoStructFloat command error: spawn is not valid, either does not exist or is not an entity", lua_interface->GetScriptName(state));
		return 0;
	}
	Entity* ent = (Entity*)spawn;
	bool set_ = ent->SetInfoStructFloat(field, value);
	lua_interface->SetBooleanValue(state, set_);
	return 1;
}

int EQ2Emu_lua_SetCharSheetChanged(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	bool value = lua_interface->GetBooleanValue(state, 2);
	lua_interface->ResetFunctionStack(state);
	
	if(!spawn || !spawn->IsPlayer())
	{
		lua_interface->LogError("%s: LUA SetCharSheetChanged command error: spawn is not valid, either does not exist or is not a player", lua_interface->GetScriptName(state));
		return 0;
	}
	
	((Player*)spawn)->SetCharSheetChanged(value);

	return 0;
}

int EQ2Emu_lua_AddPlayerMail(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	std::string fromName = lua_interface->GetStringValue(state, 2);
	std::string subjectName = lua_interface->GetStringValue(state, 3);
	std::string mailBody = lua_interface->GetStringValue(state, 4);
	int8 mailType = lua_interface->GetInt8Value(state, 5);

	int32 copper = lua_interface->GetInt32Value(state, 6);
	int32 silver = lua_interface->GetInt32Value(state, 7);
	int32 gold = lua_interface->GetInt32Value(state, 8);
	int32 platinum = lua_interface->GetInt32Value(state, 9);
	
	int32 item_id = lua_interface->GetInt32Value(state, 10);

	int16 stack_size = lua_interface->GetInt32Value(state, 11);

	int32 expire_time = lua_interface->GetInt32Value(state, 12);
	
	int32 sent_time = lua_interface->GetInt32Value(state, 13);

	lua_interface->ResetFunctionStack(state);
	
	if(!spawn || !spawn->IsPlayer())
	{
		lua_interface->LogError("%s: LUA AddPlayerMail command error: spawn is not valid, either does not exist or is not a player", lua_interface->GetScriptName(state));
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	int32 time_sent = sent_time > 0 ? sent_time : Timer::GetUnixTimeStamp();

	((Player*)spawn)->GetClient()->CreateAndUpdateMail(fromName, subjectName, mailBody, mailType, copper, silver, gold, platinum, item_id, stack_size, time_sent, expire_time);
	lua_interface->SetBooleanValue(state, true);
	return 1;
}


int EQ2Emu_lua_AddPlayerMailByCharID(lua_State* state) {
	if (!lua_interface)
		return 0;
	int32 char_id = lua_interface->GetInt32Value(state);
	std::string fromName = lua_interface->GetStringValue(state, 2);
	std::string subjectName = lua_interface->GetStringValue(state, 3);
	std::string mailBody = lua_interface->GetStringValue(state, 4);
	int8 mailType = lua_interface->GetInt8Value(state, 5);

	int32 copper = lua_interface->GetInt32Value(state, 6);
	int32 silver = lua_interface->GetInt32Value(state, 7);
	int32 gold = lua_interface->GetInt32Value(state, 8);
	int32 platinum = lua_interface->GetInt32Value(state, 9);
	
	int32 item_id = lua_interface->GetInt32Value(state, 10);

	int16 stack_size = lua_interface->GetInt32Value(state, 11);

	int32 expire_time = lua_interface->GetInt32Value(state, 12);
	
	int32 sent_time = lua_interface->GetInt32Value(state, 13);

	lua_interface->ResetFunctionStack(state);
	
	int32 time_sent = sent_time > 0 ? sent_time : Timer::GetUnixTimeStamp();

	Client::CreateMail(char_id, fromName, subjectName, mailBody, mailType, copper, silver, gold, platinum, item_id, stack_size, time_sent, expire_time);
	lua_interface->SetBooleanValue(state, true);
	return 1;
}

int EQ2Emu_lua_OpenDoor(lua_State* state) {
	Spawn* widget;

	if (lua_interface) {
		widget = lua_interface->GetSpawn(state);
		bool disable_open_sound = lua_interface->GetBooleanValue(state, 2);

		lua_interface->ResetFunctionStack(state);

		if (widget && widget->IsWidget())
		{
			((Widget*)widget)->OpenDoor();

			if(!disable_open_sound && ((Widget*)widget)->IsOpen() && ((Widget*)widget)->GetOpenSound())
				widget->GetZone()->PlaySoundFile(0, ((Widget*)widget)->GetOpenSound(), ((Widget*)widget)->GetX(), ((Widget*)widget)->GetY(), ((Widget*)widget)->GetZ());
		}
		else
			lua_interface->LogError("%s: LUA OpenDoor command error: spawn is not valid, either does not exist or is not a widget", lua_interface->GetScriptName(state));
	}

	return 0;
}

int EQ2Emu_lua_CloseDoor(lua_State* state) {
	Spawn* widget;

	if (lua_interface) {
		widget = lua_interface->GetSpawn(state);
		bool disable_close_sound = lua_interface->GetBooleanValue(state, 2);
		
		lua_interface->ResetFunctionStack(state);

		if (widget && widget->IsWidget())
		{
			((Widget*)widget)->CloseDoor();

			if(!disable_close_sound && !((Widget*)widget)->IsOpen() && ((Widget*)widget)->GetCloseSound())
				widget->GetZone()->PlaySoundFile(0, ((Widget*)widget)->GetCloseSound(), ((Widget*)widget)->GetX(), ((Widget*)widget)->GetY(), ((Widget*)widget)->GetZ());
		}
		else
			lua_interface->LogError("%s: LUA CloseDoor command error: spawn is not valid, either does not exist or is not a widget", lua_interface->GetScriptName(state));
	}

	return 0;
}

int EQ2Emu_lua_IsOpen(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* widget = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (widget && widget->IsWidget())
	{
		lua_interface->SetBooleanValue(state, ((Widget*)widget)->IsOpen());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_MakeRandomInt(lua_State* state) {
	if (!lua_interface)
		return 0;

	sint32 min = lua_interface->GetSInt32Value(state);
	sint32 max = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	sint32 result = MakeRandomInt(min, max);
	lua_interface->SetSInt32Value(state, result);
	return 1;
}

int EQ2Emu_lua_MakeRandomFloat(lua_State* state) {
	if (!lua_interface)
		return 0;

	float min = lua_interface->GetFloatValue(state);
	float max = lua_interface->GetFloatValue(state, 2);
	lua_interface->ResetFunctionStack(state);

	float result = MakeRandomFloat(min, max);
	lua_interface->SetFloatValue(state, result);
	return 1;
}

int EQ2Emu_lua_AddIconValue(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 value = lua_interface->GetInt32Value(state, 2);

	lua_interface->ResetFunctionStack(state);
	
	if(!spawn)
	{
		lua_interface->LogError("%s: LUA AddIconValue command error: spawn is not valid, does not exist", lua_interface->GetScriptName(state));
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	spawn->AddIconValue(value);
	lua_interface->SetBooleanValue(state, true);

	return 1;
}

int EQ2Emu_lua_RemoveIconValue(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 value = lua_interface->GetInt32Value(state, 2);

	lua_interface->ResetFunctionStack(state);
	
	if(!spawn)
	{
		lua_interface->LogError("%s: LUA RemoveIconValue command error: spawn is not valid, does not exist", lua_interface->GetScriptName(state));
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	spawn->RemoveIconValue(value);
	lua_interface->SetBooleanValue(state, true);

	return 1;
}

int EQ2Emu_lua_GetShardID(lua_State* state) {
	Spawn* npc = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (npc && npc->IsNPC()) {
		NPC* shard = (NPC*)npc;
		int32 shardid = shard->GetShardID();
		lua_interface->SetInt32Value(state, shardid);
		return 1;
	}
	lua_interface->SetInt32Value(state, 0);
	return 1;
}

int EQ2Emu_lua_GetShardCharID(lua_State* state) {
	Spawn* npc = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (npc && npc->IsNPC()) {
		NPC* shard = (NPC*)npc;
		int32 charid = shard->GetShardCharID();
		lua_interface->SetInt32Value(state, charid);
		return 1;
	}
	lua_interface->SetInt32Value(state, 0);
	return 1;
}

int EQ2Emu_lua_GetShardCreatedTimestamp(lua_State* state) {
	Spawn* npc = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (npc && npc->IsNPC()) {
		NPC* shard = (NPC*)npc;
		int64 timestamp = shard->GetShardCreatedTimestamp();
		lua_interface->SetSInt64Value(state, timestamp);
		return 1;
	}
	lua_interface->SetSInt64Value(state, 0);
	return 1;
}

int EQ2Emu_lua_DeleteDBShardID(lua_State* state) {
	if (!lua_interface)
		return 0;
	int32 shardid = lua_interface->GetInt32Value(state);

	lua_interface->ResetFunctionStack(state);

	if(shardid < 1)
		lua_interface->SetBooleanValue(state, false);
	else
		lua_interface->SetBooleanValue(state, database.DeleteSpiritShard(shardid));
	return 1;
}

int EQ2Emu_lua_PauseMovement(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 delay_in_ms = lua_interface->GetInt32Value(state, 2);
	if (spawn) {
		spawn->PauseMovement(delay_in_ms);
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_StopMovement(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	if (spawn) {
		spawn->ResetMovement();
	}
	lua_interface->ResetFunctionStack(state);
	return 0;
}

int EQ2Emu_lua_GetArrowColor(lua_State* state) {
	Player* player = (Player*)lua_interface->GetSpawn(state);
	int8 level = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (player && player->IsPlayer() && level > 0) {
		lua_interface->SetInt32Value(state, player->GetArrowColor(level));
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_GetTSArrowColor(lua_State* state) {
	Player* player = (Player*)lua_interface->GetSpawn(state);
	int8 level = lua_interface->GetInt8Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (player && player->IsPlayer() && level > 0) {
		lua_interface->SetInt32Value(state, player->GetTSArrowColor(level));
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetSpawnByRailID(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	sint64 rail_id = lua_interface->GetSInt64Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (zone) {
		Spawn* spawn = zone->GetTransportByRailID(rail_id);
		if (spawn) {
			lua_interface->SetSpawnValue(state, spawn);
			return 1;
		}
	}
	return 0;
}

int EQ2Emu_lua_SetRailID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	sint64 rail_id = lua_interface->GetSInt64Value(state, 2);

	lua_interface->ResetFunctionStack(state);

	bool res = false;
	if(spawn && spawn->IsTransportSpawn())
	{
		//printf("Set rail id %i for %s\n",rail_id,spawn->GetName());
		spawn->SetRailID(rail_id);
		res = true;
	}
	else if (!spawn) {
		lua_interface->LogError("%s: LUA SetRailID command error: spawn is not valid, does not exist", lua_interface->GetScriptName(state));
	}
	else if(!spawn->IsTransportSpawn()) {
		lua_interface->LogError("%s: LUA SetRailID command error: spawn %s is not a transport spawn, call AddTransportSpawn(NPC) first", lua_interface->GetScriptName(state), spawn->GetName());
	}
	lua_interface->SetBooleanValue(state, res);
	return 1;
}

int EQ2Emu_lua_IsZoneLoading(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	ZoneServer* zone = lua_interface->GetZone(state);
	lua_interface->ResetFunctionStack(state);

	if (zone) {
		lua_interface->SetBooleanValue(state, zone->IsLoading());
		return 1;
	}
	return 0;
}
int EQ2Emu_lua_IsRunning(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (spawn) {
		lua_interface->SetBooleanValue(state, spawn->IsRunning());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_GetZoneLockoutTimer(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 zoneID = lua_interface->GetInt32Value(state, 2);
	bool displayClient = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);

	if (!player || !player->IsPlayer() || !player->GetClient()) {
		lua_interface->LogError("%s: LUA GetZoneLockoutTimer command error: player is not valid, does not exist", lua_interface->GetScriptName(state));
	}
	else if(!zoneID) {
		lua_interface->LogError("%s: LUA GetZoneLockoutTimer command error: zoneID is not set.");
	}
	else
	{
		lua_interface->SetStringValue(state, player->GetClient()->IdentifyInstanceLockout(zoneID, displayClient).c_str());
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetWorldTime(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	int16 newYear = lua_interface->GetInt16Value(state, 1);
	sint32 newMonth = lua_interface->GetInt16Value(state, 2);
	int16 newHour = lua_interface->GetInt16Value(state, 3);
	int16 newMinute = lua_interface->GetInt16Value(state, 4);

	lua_interface->ResetFunctionStack(state);
	
	world.MWorldTime.writelock(__FUNCTION__, __LINE__);
	world.GetWorldTimeStruct()->year = newYear;
	world.GetWorldTimeStruct()->month = newMonth;
	world.GetWorldTimeStruct()->hour = newHour;
	world.GetWorldTimeStruct()->minute = newMinute;
	world.MWorldTime.releasewritelock(__FUNCTION__, __LINE__);
	
	return 0;
}

int EQ2Emu_lua_GetWorldTimeYear(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	lua_interface->ResetFunctionStack(state);
	
	world.MWorldTime.readlock(__FUNCTION__, __LINE__);
	lua_interface->SetInt32Value(state, world.GetWorldTimeStruct()->year);
	world.MWorldTime.releasereadlock(__FUNCTION__, __LINE__);
	
	return 1;
}

int EQ2Emu_lua_GetWorldTimeMonth(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	lua_interface->ResetFunctionStack(state);
	
	world.MWorldTime.readlock(__FUNCTION__, __LINE__);
	lua_interface->SetSInt32Value(state, world.GetWorldTimeStruct()->month);
	world.MWorldTime.releasereadlock(__FUNCTION__, __LINE__);
	
	return 1;
}

int EQ2Emu_lua_GetWorldTimeHour(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	lua_interface->ResetFunctionStack(state);
	
	world.MWorldTime.readlock(__FUNCTION__, __LINE__);
	lua_interface->SetSInt32Value(state, world.GetWorldTimeStruct()->hour);
	world.MWorldTime.releasereadlock(__FUNCTION__, __LINE__);
	
	return 1;
}

int EQ2Emu_lua_GetWorldTimeMinute(lua_State* state) {
	if (!lua_interface)
		return 0;
	
	lua_interface->ResetFunctionStack(state);
	
	world.MWorldTime.readlock(__FUNCTION__, __LINE__);
	lua_interface->SetSInt32Value(state, world.GetWorldTimeStruct()->minute);
	world.MWorldTime.releasereadlock(__FUNCTION__, __LINE__);
	
	return 1;
}

int EQ2Emu_lua_SendTimeUpdate(lua_State* state) {
	if (!lua_interface)
		return 0;

	lua_interface->ResetFunctionStack(state);
	
	world.SendTimeUpdate();
	
	return 0;
}

int EQ2Emu_lua_ChangeFaction(lua_State* state) {
	bool update_result = false;
	Faction* faction = 0;	

	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 faction_id = lua_interface->GetInt32Value(state, 2);
	sint32 increase = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);

	if(!spawn || !spawn->IsPlayer()) {
		lua_interface->LogError("LUA ChangeFaction command error: player is not valid");
		return 0;
	}

	Player* player = (Player*)spawn;
	Client* client = player->GetClient();
	if (faction_id > 0) {

		bool hasfaction = database.VerifyFactionID(player->GetCharacterID(),faction_id);
		if(hasfaction == 0) {
			//they do not have the faction. Lets get the default value and feed it in.
			sint32 defaultfaction = master_faction_list.GetDefaultFactionValue(faction_id);
			//add the default faction for the player.
			player->SetFactionValue(faction_id, defaultfaction);
		}

		if(increase >= 0) {
			update_result = player->GetFactions()->IncreaseFaction(faction_id,increase);
			faction = master_faction_list.GetFaction(faction_id);

			if(faction && update_result)
				client->Message(CHANNEL_FACTION, "Your faction standing with %s got better.", faction->name.c_str());
				else if(faction)
					client->Message(CHANNEL_FACTION, "Your faction standing with %s could not possibly get any better.", faction->name.c_str());
					
				lua_interface->SetBooleanValue(state, true);
			return 1;

		}

		if(increase < 0){
			//change the negative to a positive, since thats how decreasefaction() likes it.
			increase *= -1;

			update_result = player->GetFactions()->DecreaseFaction(faction_id,increase);
			faction = master_faction_list.GetFaction(faction_id);
			if(faction && update_result)
				client->Message(CHANNEL_FACTION, "Your faction standing with %s got worse.", faction->name.c_str());
			else if(faction)
				client->Message(CHANNEL_FACTION, "Your faction standing with %s could not possibly get any worse.", faction->name.c_str());
				
			lua_interface->SetBooleanValue(state, true);
			return 1;
		}
	}
	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_HasCoin(lua_State* state) {
	bool hascoin = 0;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 coins = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
		
if (player && player->IsPlayer()) {
	hascoin = ((Player*)player)->HasCoins(coins);
	if(hascoin == 0) {
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	if(hascoin == 1) {
		lua_interface->SetBooleanValue(state, true);
		return 1;
	}

}
  return 0;
}

int EQ2Emu_lua_GetLootTier(lua_State* state) {
	int32 loot_tier = 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (spawn) {
		loot_tier = spawn->GetLootTier();
		lua_interface->SetInt32Value(state, loot_tier);
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetLootTier(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 loot_tier = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	
	if (spawn) {
		spawn->SetLootTier(loot_tier);
		lua_interface->SetBooleanValue(state, true);
		return 1;
	}

	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_GetLootDropType(lua_State* state) {
	int32 loot_drop_type = 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (spawn) {
		loot_drop_type = spawn->GetLootDropType();
		lua_interface->SetInt32Value(state, loot_drop_type);
		return 1;
	}
	return 0;
}

int EQ2Emu_lua_SetLootDropType(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 loot_drop_type = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	
	if (spawn) {
		spawn->SetLootDropType(loot_drop_type);
		lua_interface->SetBooleanValue(state, true);
		return 1;
	}

	lua_interface->SetBooleanValue(state, false);
	return 1;
}


int EQ2Emu_lua_DamageEquippedItems(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* spawn = lua_interface->GetSpawn(state);
	int8 damage_amount = lua_interface->GetInt32Value(state, 2);
	if(damage_amount > 100) {
		damage_amount = 100;
	}
	
	if (!spawn) {
		lua_interface->LogError("%s: LUA DamageEquippedItems command error: spawn is not valid", lua_interface->GetScriptName(state));
		lua_interface->ResetFunctionStack(state);
		return 0;
	}

	lua_interface->ResetFunctionStack(state);
	if (spawn->IsPlayer()) {
		if (((Player*)spawn)->GetClient() && ((Player*)spawn)->DamageEquippedItems(damage_amount, ((Player*)spawn)->GetClient()))
			lua_interface->SetBooleanValue(state, true);
		else
			lua_interface->SetBooleanValue(state, false);
	}
	else {
		lua_interface->SetBooleanValue(state, false);
	}
	
	return 1;
}

int EQ2Emu_lua_CreateWidgetRegion(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	int32 version = lua_interface->GetInt32Value(state, 2);
	
	RegionMap* region_map = world.GetRegionMap(std::string(zone->GetZoneFile()), version);
	if(region_map == nullptr) {
		lua_interface->LogError("%s: LUA CreateWidgetRegion command error: region map is not valid for version %u", lua_interface->GetScriptName(state), version);
		lua_interface->ResetFunctionStack(state);
		return 0;
	}
	string region_name = lua_interface->GetStringValue(state, 3);
	string env_name = lua_interface->GetStringValue(state, 4);
	int32 grid_id = lua_interface->GetInt32Value(state, 5);
	int32 widget_id = lua_interface->GetInt32Value(state, 6);
	float dist = lua_interface->GetFloatValue(state, 7);
	region_map->InsertRegionNode(zone, version, region_name, env_name, grid_id, widget_id, dist);
	
	lua_interface->ResetFunctionStack(state);

	lua_interface->SetBooleanValue(state, true);
	return 1;
}

int EQ2Emu_lua_RemoveRegion(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	int32 version = lua_interface->GetInt32Value(state, 2);
	
	RegionMap* region_map = world.GetRegionMap(std::string(zone->GetZoneFile()), version);
	if(region_map == nullptr) {
		lua_interface->LogError("%s: LUA RemoveRegion command error: region map is not valid for version %u", lua_interface->GetScriptName(state), version);
		lua_interface->ResetFunctionStack(state);
		return 0;
	}
	string region_name = lua_interface->GetStringValue(state, 3);
	region_map->RemoveRegionNode(region_name);
	
	lua_interface->ResetFunctionStack(state);

	lua_interface->SetBooleanValue(state, true);
	return 1;
}


int EQ2Emu_lua_SetPlayerPOVGhost(lua_State* state) {
	Client* client = nullptr;
	Spawn* player = lua_interface->GetSpawn(state);
	if (!player) {
		lua_interface->LogError("LUA SetPlayerPOVGhost command error: spawn is not valid");
		lua_interface->SetBooleanValue(state, false);
		lua_interface->ResetFunctionStack(state);
		return 1;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("LUA SetPlayerPOVGhost command error: spawn is not a player");
		lua_interface->SetBooleanValue(state, false);
		lua_interface->ResetFunctionStack(state);
		return 1;
	}

	client = player->GetClient();

	if (!client) {
		lua_interface->LogError("LUA SetPlayerPOVGhost command error: could not find client");
		lua_interface->SetBooleanValue(state, false);
		lua_interface->ResetFunctionStack(state);
		return 1;
	}
	
	Spawn* spawn = lua_interface->GetSpawn(state, 2);
	
	bool success_sight = client->SetPlayerPOVGhost(spawn);
	
	lua_interface->ResetFunctionStack(state);

	lua_interface->SetBooleanValue(state, success_sight);
	return 1;
}


int EQ2Emu_lua_SetCastOnAggroComplete(lua_State* state) {
	if (!lua_interface)
		return 0;
	bool result = false;
	
	Spawn* spawn = lua_interface->GetSpawn(state);
	bool cast_completed = (lua_interface->GetInt8Value(state, 2) == 1);
	lua_interface->ResetFunctionStack(state);

	if (!spawn)
		lua_interface->LogError("%s: LUA SetCastOnAggroComplete error: Could not find spawn.", lua_interface->GetScriptName(state));
	else if (!spawn->IsNPC())
		lua_interface->LogError("%s: LUA SetCastOnAggroComplete error: spawn %s is not an NPC!.", lua_interface->GetScriptName(state), spawn->GetName());
	else
	{
		((NPC*)spawn)->cast_on_aggro_completed = cast_completed;
		result = true;
	}

	lua_interface->SetBooleanValue(state, result);

	return 1;
}

int EQ2Emu_lua_IsCastOnAggroComplete(lua_State* state) {
	if (!lua_interface)
		return 0;
	bool result = false;
	
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (!spawn)
		lua_interface->LogError("%s: LUA IsCastOnAggroComplete error: Could not find spawn.", lua_interface->GetScriptName(state));
	else if (!spawn->IsNPC())
		lua_interface->LogError("%s: LUA IsCastOnAggroComplete error: spawn %s is not an NPC!.", lua_interface->GetScriptName(state), spawn->GetName());
	else
	{
		if(((NPC*)spawn)->cast_on_aggro_completed)
			result = true;
	}

	lua_interface->SetBooleanValue(state, result);

	return 1;
}

int EQ2Emu_lua_AddRecipeBookToPlayer(lua_State* state) {
	Client* client = nullptr;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 recipe_book_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!player) {
		lua_interface->LogError("LUA AddRecipeBookToPlayer command error: spawn is not valid");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("LUA AddRecipeBookToPlayer command error: spawn is not a player");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	client = player->GetClient();

	if (!client) {
		lua_interface->LogError("LUA AddRecipeBookToPlayer command error: could not find client");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	
	bool result = client->AddRecipeBookToPlayer(recipe_book_id);

	lua_interface->SetBooleanValue(state, result);
	return 1;
}

int EQ2Emu_lua_RemoveRecipeFromPlayer(lua_State* state) {
	Client* client = nullptr;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 recipe_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	if (!player) {
		lua_interface->LogError("LUA RemoveRecipeFromPlayer command error: spawn is not valid");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("LUA RemoveRecipeFromPlayer command error: spawn is not a player");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	client = player->GetClient();

	if (!client) {
		lua_interface->LogError("LUA RemoveRecipeFromPlayer command error: could not find client");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	
	bool result = client->RemoveRecipeFromPlayer(recipe_id);

	lua_interface->SetBooleanValue(state, result);
	return 1;
}

int EQ2Emu_lua_ReplaceWidgetFromClient(lua_State* state) {
	Client* client = nullptr;
	Spawn* player = lua_interface->GetSpawn(state);
	int32 widget_id = lua_interface->GetInt32Value(state, 2);
	bool delete_widget = (lua_interface->GetInt8Value(state, 3) == 1);
	
	// rest are all optional fields
	float x = lua_interface->GetFloatValue(state, 4);
	float y = lua_interface->GetFloatValue(state, 5);
	float z = lua_interface->GetFloatValue(state, 6);
	int32 grid_id = lua_interface->GetInt32Value(state, 7);
	
	lua_interface->ResetFunctionStack(state);
	if (!player) {
		lua_interface->LogError("LUA ReplaceWidgetFromClient command error: spawn is not valid");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	if (!player->IsPlayer()) {
		lua_interface->LogError("LUA ReplaceWidgetFromClient command error: spawn is not a player");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	if(!player->GetZone()) {
		lua_interface->LogError("LUA ReplaceWidgetFromClient command error: player is not in a zone");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	
	client = player->GetClient();

	if (!client) {
		lua_interface->LogError("LUA ReplaceWidgetFromClient command error: could not find client");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	if(!client->IsReadyForUpdates()) {
		lua_interface->LogError("LUA ReplaceWidgetFromClient command failed: client has not signaled sys_client_avatar_ready");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	
	client->SendReplaceWidget(widget_id, delete_widget, x, y, z, grid_id);
	
	lua_interface->SetBooleanValue(state, true);
	return 1;
}

int EQ2Emu_lua_RemoveWidgetFromSpawnMap(lua_State* state) {
	Client* client = nullptr;
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 widget_id = lua_interface->GetInt32Value(state, 2);
	
	lua_interface->ResetFunctionStack(state);
	if (!spawn) {
		lua_interface->LogError("LUA RemoveWidgetFromSpawnMap command error: spawn is not valid");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	if(!spawn->GetZone()) {
		lua_interface->LogError("LUA ReplaceWidgetFromClient command error: player is not in a zone");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	
	spawn->AddIgnoredWidget(widget_id);
	
	lua_interface->SetBooleanValue(state, true);
	return 1;
}

int EQ2Emu_lua_RemoveWidgetFromZoneMap(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	int32 widget_id = lua_interface->GetInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);
	
	if(!zone) {
		lua_interface->LogError("LUA RemoveWidgetFromZoneMap command error: zone is not valid");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	
	if(!zone->IsLoading()) {
		lua_interface->LogError("LUA RemoveWidgetFromZoneMap command error: can only be called during zone loading, in preinit_zone_script ZoneScript function");
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	
	zone->AddIgnoredWidget(widget_id);
	lua_interface->SetBooleanValue(state, true);
	return 1;
}

int EQ2Emu_lua_SendHearCast(lua_State* state) {
	if (!lua_interface)
		return 0;
	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	Spawn* spawn = lua_interface->GetSpawn(state);
	int32 spell_visual_id = lua_interface->GetInt32Value(state, 2);
	int16 cast_time = lua_interface->GetInt16Value(state, 3);
	Spawn* caster = lua_interface->GetSpawn(state, 4);
	Spawn* target = lua_interface->GetSpawn(state, 5);
	lua_interface->ResetFunctionStack(state);
	if(spell && spawn && spawn->IsEntity()) {
		ZoneServer* zone = spawn->GetZone();
		if(zone) {
			zone->SendCastSpellPacket(spell, caster && caster->IsEntity() ? (Entity*)caster : (Entity*)spawn, spell_visual_id, cast_time > 0 ? cast_time : 0xFFFF);
		}
	}
	else if (spawn) {
		if (spawn->IsPlayer()) {
			Client* client = ((Player*)spawn)->GetClient();
			if (client) {
				client->SendHearCast(caster ? caster : client->GetPlayer(), target ? target : client->GetPlayer(), spell_visual_id, cast_time);
			}
		}
	}
	return 0;
}

int EQ2Emu_lua_GetCharacterFlag(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	sint32 flag_id = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA HasRecipeBook command error, Spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA HasRecipeBook command error, Spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	bool ret = ((Player*)player)->get_character_flag(flag_id);
	lua_interface->SetBooleanValue(state, ret);
	return 1;
}

int EQ2Emu_lua_ToggleCharacterFlag(lua_State* state) {
	if (!lua_interface)
		return 0;

	Spawn* player = lua_interface->GetSpawn(state);
	sint32 flag_id = lua_interface->GetSInt32Value(state, 2);
	lua_interface->ResetFunctionStack(state);

	if (!player) {
		lua_interface->LogError("%s: LUA HasRecipeBook command error, Spawn is not valid", lua_interface->GetScriptName(state));
		return 0;
	}
	if (!player->IsPlayer()) {
		lua_interface->LogError("%s: LUA HasRecipeBook command error, Spawn is not a player", lua_interface->GetScriptName(state));
		return 0;
	}

	((Player*)player)->toggle_character_flag(flag_id);
	return 0;
}

int EQ2Emu_lua_GetSpellInitialTarget(lua_State* state) {
	LuaSpell* spell = lua_interface->GetSpell(state);
	
	if(!spell) {
		spell = lua_interface->GetCurrentSpell(state);
	}
	
	lua_interface->ResetFunctionStack(state);
	if (spell) {
		if(!spell->caster) {
			lua_interface->LogError("%s: LUA GetSpellInitialTarget command error, caster does not exist.", lua_interface->GetScriptName(state));
			return 0;
		}
		if(!spell->caster->GetZone()) {
			lua_interface->LogError("%s: LUA GetSpellInitialTarget command error, zone does not exist.", lua_interface->GetScriptName(state));
			return 0;
		}
		Spawn* spawn = spell->caster->GetZone()->GetSpawnByID(spell->initial_target);
		if (spawn) {
			lua_interface->SetSpawnValue(state, spawn);
			return 1;
		}
		else {
			lua_interface->LogError("%s: LUA GetSpellInitialTarget command error, could not find initial target %u to map to spawn.", lua_interface->GetScriptName(state), spell->initial_target);
		}
	}
	return 0;
}

int EQ2Emu_lua_GetSpellCaster(lua_State* state) {
	LuaSpell* spell = lua_interface->GetSpell(state);
	
	if(!spell) {
		spell = lua_interface->GetCurrentSpell(state);
	}
	
	lua_interface->ResetFunctionStack(state);
	if (spell) {
		if(!spell->caster) {
			lua_interface->LogError("%s: LUA GetSpellCaster command error, caster does not exist.", lua_interface->GetScriptName(state));
			return 0;
		}
		lua_interface->SetSpawnValue(state, spell->caster);
		return 1;
	}
	else {
		lua_interface->LogError("%s: LUA GetSpellCaster command error, could not find spell.", lua_interface->GetScriptName(state));
	}
	return 0;
}

int EQ2Emu_lua_GetCasterSpellLevel(lua_State* state) {
	LuaSpell* spell = lua_interface->GetSpell(state);
	
	if(!spell) {
		spell = lua_interface->GetCurrentSpell(state);
	}
	
	lua_interface->ResetFunctionStack(state);
	if (spell) {
		lua_interface->SetInt32Value(state, spell->initial_caster_level);
		return 1;
	}
	else {
		lua_interface->LogError("%s: LUA GetCasterSpellLevel command error, spell does not exist.", lua_interface->GetScriptName(state));
	}
	return 0;
}

int EQ2Emu_lua_GetSpellTargets(lua_State* state) {
	if (!lua_interface)
		return 0;
	LuaSpell* spell = lua_interface->GetSpell(state);
	
	if(!spell) {
		spell = lua_interface->GetCurrentSpell(state);
	}
	
	lua_interface->ResetFunctionStack(state);
	
	if (spell) {
		if(!spell->caster) {
			lua_interface->LogError("%s: LUA GetSpellTargets command error, caster does not exist.", lua_interface->GetScriptName(state));
			return 0;
		}
		if(!spell->caster->GetZone()) {
			lua_interface->LogError("%s: LUA GetSpellTargets command error, zone does not exist.", lua_interface->GetScriptName(state));
			return 0;
		}
		
		if (spell && spell->caster) {
			ZoneServer* zone = spell->caster->GetZone();
			spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
			lua_createtable(state, spell->targets.size(), 0);
			int newTable = lua_gettop(state);
			for (int32 i = 0; i < spell->targets.size(); i++) {
				Spawn* spawn = zone->GetSpawnByID(spell->targets.at(i));
				lua_interface->SetSpawnValue(state, spawn);
				lua_rawseti(state, newTable, i + 1);
			}
			spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
			return 1;
		}
	}
	return 0;
}

int EQ2Emu_lua_DespawnByLocationID(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	int32 location_id = lua_interface->GetInt32Value(state, 2);
	int32 delay = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (zone) {
		Spawn* spawn = zone->GetSpawnByLocationID(location_id);
		if (spawn) {
			vector<Spawn*> groupMembers;
			if (!spawn->IsPlayer() && spawn->HasSpawnGroup()) {
				groupMembers = *spawn->GetSpawnGroup();
				for (int32 i = 0; i < groupMembers.size(); i++) {
					Spawn* member = groupMembers.at(i);
					if(member && !member->IsPlayer() && member != spawn) {
						member->GetZone()->Despawn(member, delay);
					}
				}
			}
			
			zone->Despawn(spawn, delay);
			lua_interface->SetBooleanValue(state, true);
			return 1;
		}
	}
	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_AddRespawn(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	int32 location_id = lua_interface->GetInt32Value(state, 2);
	int32 respawn_time = lua_interface->GetInt32Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if (zone) {
			zone->AddRespawn(location_id, respawn_time);
			lua_interface->SetBooleanValue(state, true);
			return 1;
	}
	lua_interface->SetBooleanValue(state, false);
	return 1;
}



int EQ2Emu_lua_CreatePersistedRespawn(lua_State* state) {
	int32 location_id = lua_interface->GetInt32Value(state);
	int8 spawn_type = lua_interface->GetInt32Value(state, 2);
	int32 respawn_time = lua_interface->GetInt32Value(state, 3);
	int32 zone_id = lua_interface->GetInt32Value(state, 4);
	lua_interface->ResetFunctionStack(state);
	if (location_id && zone_id) {
			database.CreatePersistedRespawn(location_id,spawn_type,respawn_time,zone_id);
			lua_interface->SetBooleanValue(state, true);
			return 1;
	}
	lua_interface->SetBooleanValue(state, false);
	return 1;
}


int EQ2Emu_lua_CreateChoiceWindow(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* npc = lua_interface->GetSpawn(state);
	Spawn* spawn = lua_interface->GetSpawn(state, 2);
	std::string windowTextPrompt = lua_interface->GetStringValue(state, 3);
	std::string acceptText = lua_interface->GetStringValue(state, 4);
	std::string acceptCommand = lua_interface->GetStringValue(state, 5);
	std::string declineText = lua_interface->GetStringValue(state, 6);
	std::string declineCommand = lua_interface->GetStringValue(state, 7);
	int32 time = lua_interface->GetInt32Value(state, 8);
	int8 textBox = lua_interface->GetInt8Value(state, 9);
	int8 textBoxRequired = lua_interface->GetInt8Value(state, 10);
	int32 maxLength = lua_interface->GetInt32Value(state, 11);
	lua_interface->ResetFunctionStack(state);
	if(!npc || !npc->IsNPC())
	{
		lua_interface->LogError("%s: LUA CreateChoiceWindow command error: npc is not valid, either does not exist or is not a npc", lua_interface->GetScriptName(state));
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}
	
	if(!spawn || !spawn->IsPlayer())
	{
		lua_interface->LogError("%s: LUA CreateChoiceWindow command error: spawn is not valid, either does not exist or is not a player", lua_interface->GetScriptName(state));
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	Client* client = ((Player*)spawn)->GetClient();
	
	if(client) {
			bool success = client->SendDialogChoice(npc->GetID(), windowTextPrompt, acceptText, acceptCommand, declineText, declineCommand, time, textBox, textBoxRequired, maxLength);
			lua_interface->SetBooleanValue(state, success);
			return 1;
	}
	lua_interface->SetBooleanValue(state, false);
	return 1;
}


int EQ2Emu_lua_ClearChoice(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	std::string commandToClear = lua_interface->GetStringValue(state, 2);
	int8 clearDecline = lua_interface->GetInt8Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if(!spawn || !spawn->IsPlayer())
	{
		lua_interface->LogError("%s: LUA ClearChoice command error: spawn is not valid, either does not exist or is not a player", lua_interface->GetScriptName(state));
		lua_interface->SetBooleanValue(state, false);
		return 1;
	}

	Client* client = ((Player*)spawn)->GetClient();
	
	if(client) {
			bool success = false;
			
			if(clearDecline) {
				success = client->dialog_manager.clearDecline(commandToClear);
			}
			else {
				success = client->dialog_manager.clearAccept(commandToClear);
			}
			lua_interface->SetBooleanValue(state, success);
			return 1;
	}
	lua_interface->SetBooleanValue(state, false);
	return 1;
}

int EQ2Emu_lua_GetChoiceSpawnID(lua_State* state) {
	if (!lua_interface)
		return 0;
	Spawn* spawn = lua_interface->GetSpawn(state);
	std::string commandMatch = lua_interface->GetStringValue(state, 2);
	int8 declineValue = lua_interface->GetInt8Value(state, 3);
	lua_interface->ResetFunctionStack(state);
	if(!spawn || !spawn->IsPlayer())
	{
		lua_interface->LogError("%s: LUA GetChoiceSpawnID command error: spawn is not valid, either does not exist or is not a player", lua_interface->GetScriptName(state));
		lua_interface->SetInt32Value(state, 0);
		return 1;
	}

	Client* client = ((Player*)spawn)->GetClient();
	int32 spawn_id = 0;
	if(client) {
		if(declineValue) {
			spawn_id = client->dialog_manager.getDeclineValue(commandMatch);
		}
		else {
			spawn_id = client->dialog_manager.getAcceptValue(commandMatch);
		}
	}
	lua_interface->SetInt32Value(state, spawn_id);
	return 1;
}

int EQ2Emu_lua_GetZonePlayerMinLevel(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	lua_interface->ResetFunctionStack(state);
	
	int32 level = 0;
	if (zone) {
		level = zone->getGroupraidMinLevel();
	}
	lua_interface->SetInt32Value(state, level);
	return 1;
}

int EQ2Emu_lua_GetZonePlayerMaxLevel(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	lua_interface->ResetFunctionStack(state);
	
	int32 level = 0;
	if (zone) {
		level = zone->getGroupraidMaxLevel();
	}
	lua_interface->SetInt32Value(state, level);
	return 1;
}

int EQ2Emu_lua_GetZonePlayerAvgLevel(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	lua_interface->ResetFunctionStack(state);
	
	int32 level = 0;
	if (zone) {
		level = zone->getGroupraidAvgLevel();
	}
	lua_interface->SetInt32Value(state, level);
	return 1;
}

int EQ2Emu_lua_GetZonePlayerFirstLevel(lua_State* state) {
	ZoneServer* zone = lua_interface->GetZone(state);
	lua_interface->ResetFunctionStack(state);
	
	int32 level = 0;
	if (zone) {
		level = zone->getGroupraidFirstLevel();
	}
	lua_interface->SetInt32Value(state, level);
	return 1;
}

int EQ2Emu_lua_GetSpellRequiredLevel(lua_State* state) {
	if (!lua_interface)
		return 0;

	LuaSpell* spell = lua_interface->GetCurrentSpell(state);
	Spawn* spawn = lua_interface->GetSpawn(state);
	lua_interface->ResetFunctionStack(state);

	if (!spell) {
		lua_interface->LogError("%s: LUA GetSpellRequiredLevel command error: you must use this function in a spellscript!", lua_interface->GetScriptName(state));
		return 0;
	}
	if(!spawn) {
		lua_interface->LogError("%s: LUA GetSpellRequiredLevel command error: spawn is not provided as first argument or is invalid (not player or null)!", lua_interface->GetScriptName(state));
		return 0;
	}
	
	if(!spawn->IsPlayer()) {
		lua_interface->SetInt32Value(state, spawn->GetLevel());
	}
	else {
		lua_interface->SetInt32Value(state, spell->spell->GetLevelRequired((Player*)spawn));
	}
	
	return 1;
}