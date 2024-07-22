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
#ifndef LUA_INTERFACE_H
#define LUA_INTERFACE_H

#include <mutex>
#include <shared_mutex>

#include "Spawn.h"
#include "Spells.h"
#include "../common/Mutex.h"
#include "Quests.h"
#include "zoneserver.h"
#include "client.h"

#include "../LUA/lua.hpp"

using namespace std;

struct ConversationOption{
	string option;
	string function;
};

struct OptionWindowOption {
	string	optionName;
	string	optionDescription;
	string	optionCommand;
	int32	optionIconSheet;
	int16	optionIconID;
	string	optionConfirmTitle;
};

//Bitmask Values
#define EFFECT_FLAG_STUN 1
#define EFFECT_FLAG_ROOT 2
#define EFFECT_FLAG_MEZ 4
#define EFFECT_FLAG_STIFLE 8
#define EFFECT_FLAG_DAZE 16
#define EFFECT_FLAG_FEAR 32
#define EFFECT_FLAG_SPELLBONUS 64
#define EFFECT_FLAG_SKILLBONUS 128
#define EFFECT_FLAG_STEALTH 256
#define EFFECT_FLAG_INVIS 512
#define EFFECT_FLAG_SNARE 1024
#define EFFECT_FLAG_WATERWALK 2048
#define EFFECT_FLAG_WATERJUMP 4096
#define EFFECT_FLAG_FLIGHT 8192
#define EFFECT_FLAG_GLIDE 16384
#define EFFECT_FLAG_AOE_IMMUNE 32768
#define EFFECT_FLAG_STUN_IMMUNE 65536
#define EFFECT_FLAG_MEZ_IMMUNE 131072
#define EFFECT_FLAG_DAZE_IMMUNE 262144
#define EFFECT_FLAG_ROOT_IMMUNE 524288
#define EFFECT_FLAG_STIFLE_IMMUNE 1048576
#define EFFECT_FLAG_FEAR_IMMUNE 2097152
#define EFFECT_FLAG_SAFEFALL 4194304

struct LuaSpell{
	Entity*			caster;
	int32			initial_caster_char_id;
	int32			initial_target;
	int32			initial_target_char_id;
	vector<int32>	targets;
	vector<int32>	removed_targets; // previously cancelled, expired, used, so on
	multimap<int32, int8> char_id_targets;
	Spell*			spell;
	lua_State*		state;
	string			file_name;
	Timer			timer;
	bool			is_recast_timer;
	int16			num_calls;
	int16           num_triggers;
	int8            slot_pos;
	int32           damage_remaining;
	bool			resisted;
	bool			has_damaged;
	bool			is_damage_spell;
	bool			interrupted;
	bool            crit;
	bool            last_spellattack_hit;
	bool            cancel_after_all_triggers;
	bool            had_triggers;
	bool            had_dmg_remaining;
	Mutex           MSpellTargets;
	int32           effect_bitmask;
	bool			restored; // restored spell cross zone

};

class LUAUserData{
public:
	LUAUserData();
	virtual ~LUAUserData(){};
	virtual bool IsCorrectlyInitialized();
	virtual bool IsConversationOption();
	virtual bool IsOptionWindow();
	virtual bool IsSpawn();
	virtual bool IsQuest();
	virtual bool IsZone();
	virtual bool IsItem();
	virtual bool IsSkill();
	virtual bool IsSpell();
	bool correctly_initialized;
	Item* item;
	ZoneServer* zone;
	Spawn* spawn;
	vector<ConversationOption>* conversation_options;
	vector<OptionWindowOption>* option_window_option;
	vector<Spawn*>* spawn_list;
	Quest* quest;
	Skill* skill;
	LuaSpell* spell;
};

class LUAConversationOptionWrapper : public LUAUserData{
public:
	LUAConversationOptionWrapper();
	bool IsConversationOption();
};

class LUAOptionWindowWrapper : public LUAUserData {
public:
	LUAOptionWindowWrapper();
	bool IsOptionWindow();
};

class LUASpawnWrapper : public LUAUserData{
public:
	LUASpawnWrapper();
	bool IsSpawn();
};

class LUAZoneWrapper : public LUAUserData{
public:
	LUAZoneWrapper();
	bool IsZone();
};

class LUAQuestWrapper : public LUAUserData{
public:
	LUAQuestWrapper();
	bool IsQuest();
};

class LUAItemWrapper : public LUAUserData{
public:
	LUAItemWrapper();
	bool IsItem();
};

class LUASkillWrapper: public LUAUserData {
public:
	LUASkillWrapper();
	bool IsSkill();
};

class LUASpellWrapper : public LUAUserData {
public:
	LUASpellWrapper();
	bool IsSpell();
};

class LuaInterface {
public:
	LuaInterface();
	~LuaInterface();
	int				GetNumberOfArgs(lua_State* state);
	bool			LoadLuaSpell(const char* name);
	bool			LoadLuaSpell(string name);
	bool			LoadItemScript(string name);
	bool			LoadItemScript(const char* name);
	bool			LoadSpawnScript(string name);
	bool			LoadSpawnScript(const char* name);
	bool			LoadZoneScript(string name);
	bool			LoadZoneScript(const char* name);
	bool			LoadRegionScript(string name);
	bool			LoadRegionScript(const char* name);
	void			RemoveSpell(LuaSpell* spell, bool call_remove_function = true, bool can_delete = true, string reason = "", bool removing_all_spells = false);
	Spawn*			GetSpawn(lua_State* state, int8 arg_num = 1);
	Item*			GetItem(lua_State* state, int8 arg_num = 1);
	Quest*			GetQuest(lua_State* state, int8 arg_num = 1);
	ZoneServer*		GetZone(lua_State* state, int8 arg_num = 1);
	Skill*			GetSkill(lua_State* state, int8 arg_num = 1);
	LuaSpell*		GetSpell(lua_State* state, int8 arg_num = 1);
	vector<ConversationOption>*	GetConversation(lua_State* state, int8 arg_num = 1);
	
	vector<OptionWindowOption>* GetOptionWindow(lua_State* state, int8 arg_num = 1);
	int8			GetInt8Value(lua_State* state, int8 arg_num = 1);
	int16			GetInt16Value(lua_State* state, int8 arg_num = 1);
	int32			GetInt32Value(lua_State* state, int8 arg_num = 1);
	sint32			GetSInt32Value(lua_State* state, int8 arg_num = 1);
	int64			GetInt64Value(lua_State* state, int8 arg_num = 1);
	sint64			GetSInt64Value(lua_State* state, int8 arg_num = 1);
	float			GetFloatValue(lua_State* state, int8 arg_num = 1);
	string			GetStringValue(lua_State* state, int8 arg_num = 1);
	bool			GetBooleanValue(lua_State*state, int8 arg_num = 1);

	void			Process();

	void			SetInt32Value(lua_State* state, int32 value);
	void			SetSInt32Value(lua_State* state, sint32 value);
	void			SetInt64Value(lua_State* state, int64 value);
	void			SetSInt64Value(lua_State* state, sint64 value);
	void			SetFloatValue(lua_State* state, float value);
	void			SetBooleanValue(lua_State* state, bool value);
	void			SetStringValue(lua_State* state, const char* value);
	void			SetSpawnValue(lua_State* state, Spawn* spawn);
	void			SetSkillValue(lua_State* state, Skill* skill);
	void			SetItemValue(lua_State* state, Item* item);
	void			SetQuestValue(lua_State* state, Quest* quest);
	void			SetZoneValue(lua_State* state, ZoneServer* zone);
	void			SetSpellValue(lua_State* state, LuaSpell* spell);
	void			SetConversationValue(lua_State* state, vector<ConversationOption>* conversation);
	void			SetOptionWindowValue(lua_State* state, vector<OptionWindowOption>* optionWindow);

	std::string		AddSpawnPointers(LuaSpell* spell, bool first_cast, bool precast = false, const char* function = 0, SpellScriptTimer* timer = 0, bool passLuaSpell=false, Spawn* altTarget = 0);
	LuaSpell*		GetCurrentSpell(lua_State* state, bool needsLock = true);
	void			RemoveCurrentSpell(lua_State* state, bool needsLock = true);
	bool			CallSpellProcess(LuaSpell* spell, int8 num_parameters, std::string functionCalled);
	LuaSpell*		GetSpell(const char* name);
	void			UseItemScript(const char* name, lua_State* state, bool val);
	void			UseSpawnScript(const char* name, lua_State* state, bool val);
	void			UseZoneScript(const char* name, lua_State* state, bool val);
	void			UseRegionScript(const char* name, lua_State* state, bool val);
	lua_State*		GetItemScript(const char* name, bool create_new = true, bool use = false);
	lua_State*		GetSpawnScript(const char* name, bool create_new = true, bool use = false);
	lua_State*		GetZoneScript(const char* name, bool create_new = true, bool use = false);
	lua_State*		GetRegionScript(const char* name, bool create_new = true, bool use = false);
	Quest*			LoadQuest(int32 id, const char* name, const char* type, const char* zone, int8 level, const char* description, char* script_name);

	const char*		GetScriptName(lua_State* state);

	void			RemoveSpawnScript(const char* name);
	bool			RunItemScript(string script_name, const char* function_name, Item* item, Spawn* spawn = 0, Spawn* target = 0, sint64* returnValue = 0);
	bool			RunItemScriptWithReturnString(string script_name, const char* function_name, Item* item, Spawn* spawn = 0, std::string* returnValue = 0);
	bool			CallItemScript(lua_State* state, int8 num_parameters, std::string* returnValue = 0);
	bool			CallItemScript(lua_State* state, int8 num_parameters, sint64* returnValue = 0);
	bool			RunSpawnScript(string script_name, const char* function_name, Spawn* npc, Spawn* spawn = 0, const char* message = 0, bool is_door_open = false, sint32 input_value = 0, sint32* return_value = 0);
	bool			CallSpawnScript(lua_State* state, int8 num_parameters);
	bool			RunZoneScript(string script_name, const char* function_name, ZoneServer* zone, Spawn* spawn = 0, int32 int32_arg1 = 0, const char* str_arg1 = 0, Spawn* spawn_arg1 = 0, int32 int32_arg2 = 0, const char* str_arg2 = 0, Spawn* spawn_arg2 = 0);
	bool			RunZoneScriptWithReturn(string script_name, const char* function_name, ZoneServer* zone, Spawn* spawn, int32 int32_arg1, int32 int32_arg2, int32 int32_arg3, int32* returnValue = 0);
	bool			CallScriptInt32(lua_State* state, int8 num_parameters, int32* returnValue = 0);
	bool			CallScriptSInt32(lua_State* state, int8 num_parameters, sint32* returnValue = 0);
	bool			RunRegionScript(string script_name, const char* function_name, ZoneServer* zone, Spawn* spawn = 0, sint32 int32_arg1 = 0, int32* returnValue = 0);
	bool			CallRegionScript(lua_State* state, int8 num_parameters, int32* returnValue);
	void			ResetFunctionStack(lua_State* state);
	void			DestroySpells();
	void			DestroySpawnScripts();
	void			DestroyItemScripts();
	void			ReloadSpells();
	void			DestroyQuests(bool reload = false);
	void			DestroyZoneScripts();
	void			DestroyRegionScripts();
	void			SimpleLogError(const char* error);
	void			LogError(const char* error, ...);


	bool			CallQuestFunction(Quest* quest, const char* function, Spawn* player, int32 step_id = 0xFFFFFFFF, int32* returnValue = 0);
	void			RemoveDebugClients(Client* client);
	void			UpdateDebugClients(Client* client);
	void			ProcessErrorMessage(const char* message);
	map<Client*, int32> GetDebugClients(){ return debug_clients; }
	void			AddUserDataPtr(LUAUserData* data, void* data_ptr = 0);
	void			DeleteUserDataPtrs(bool all);
	void			DeletePendingSpells(bool all);
	void			DeletePendingSpell(LuaSpell* spell);
	Mutex*			GetSpawnScriptMutex(const char* name);
	Mutex*			GetItemScriptMutex(const char* name);
	Mutex*			GetZoneScriptMutex(const char* name);
	Mutex*			GetRegionScriptMutex(const char* name);
	Mutex*			GetQuestMutex(Quest* quest);

	void			SetLuaSystemReloading(bool val) { lua_system_reloading = val; }
	bool			IsLuaSystemReloading() { return lua_system_reloading; }
	
	void			AddPendingSpellDelete(LuaSpell* spell);

	void			AddCustomSpell(LuaSpell* spell);
	void			RemoveCustomSpell(int32 id);

	void			FindCustomSpellLock() { MCustomSpell.readlock(); }
	void			FindCustomSpellUnlock() { MCustomSpell.releasereadlock(); }
	LuaSpell*		FindCustomSpell(int32 id);

	int32			GetFreeCustomSpellID();
	
	void			SetLuaUserDataStale(void* ptr);
	
private:
	bool			shutting_down;
	bool			lua_system_reloading;
	map<LuaSpell*, int32> spells_pending_delete;
	Timer*			user_data_timer;
	Timer*			spell_delete_timer;
	map<LUAUserData*, int32> user_data;
	map<void*, LUAUserData*> user_data_ptr;
	map<Client*, int32>	debug_clients;
	map<lua_State*, LuaSpell*> current_spells;
	vector<string>*	GetDirectoryListing(const char* directory);
	lua_State*		LoadLuaFile(const char* name);
	void			RegisterFunctions(lua_State* state);
	map<string, LuaSpell*> spells;
	map<lua_State*, string> inverse_spells;

	map<int32, Quest*>		quests;
	map<int32, lua_State*> quest_states;
	map<string, map<lua_State*, bool> > item_scripts;
	map<string, map<lua_State*, bool> > spawn_scripts;
	map<string, map<lua_State*, bool> > zone_scripts;
	map<string, map<lua_State*, bool> > region_scripts;

	map<int32, LuaSpell*> custom_spells;
	std::deque<int32> custom_free_spell_ids;

	map<lua_State*, string> item_inverse_scripts;
	map<lua_State*, string> spawn_inverse_scripts;
	map<lua_State*, string> zone_inverse_scripts;
	map<lua_State*, string> region_inverse_scripts;

	map<string, Mutex*> item_scripts_mutex;
	map<string, Mutex*> spawn_scripts_mutex;
	map<string, Mutex*> zone_scripts_mutex;
	map<int32, Mutex*> quests_mutex;
	map<string, Mutex*> region_scripts_mutex;

	Mutex			MDebugClients;
	Mutex			MSpells;
	Mutex			MSpawnScripts;
	Mutex			MItemScripts;
	Mutex			MZoneScripts;
	Mutex			MQuests;
	Mutex			MLUAMain;
	Mutex			MSpellDelete;
	Mutex			MCustomSpell;
	Mutex			MRegionScripts;
	
	mutable std::shared_mutex	MLUAUserData;
};
#endif
