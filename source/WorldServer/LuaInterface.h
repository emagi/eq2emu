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

#ifndef LUA_INTERFACE_H
#define LUA_INTERFACE_H

#include <mutex>
#include <shared_mutex>
#include <unordered_set>

#include "Spawn.h"
#include "Spells.h"
#include "../common/Mutex.h"
#include "Quests.h"
#include "zoneserver.h"
#include "client.h"

#include <lua.hpp>

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

enum class SpellFieldType {
	Integer,
	Float,
	Boolean,
	String
};

using SpellFieldGetter = std::function<std::string(SpellData*)>;
extern const std::unordered_map<std::string, std::function<void(Spell*, const std::string&)>> SpellFieldGenericSetters;
extern const std::unordered_map<std::string, std::pair<SpellFieldType, SpellFieldGetter>> SpellDataFieldAccessors;
extern std::unordered_map<std::string, std::function<void(Spell*, const std::string&)>> SpellDataFieldSetters;

struct LuaSpell{
	Entity*			caster;
	int32			initial_caster_char_id;
	int32			initial_target;
	int32			initial_target_char_id;
	mutable std::shared_mutex targets_mutex;
	vector<int32>	targets;
	vector<int32>	removed_targets; // previously cancelled, expired, used, so on
	mutable std::shared_mutex char_id_targets_mutex;
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
	Mutex           MScriptMutex;
	int32           effect_bitmask;
	bool			restored; // restored spell cross zone
	std::atomic<bool> has_proc;
	ZoneServer*		zone;
	int16			initial_caster_level;
	bool			is_loaded_recast;
	
	std::unordered_set<std::string> modified_fields; 
	mutable std::shared_mutex spell_modify_mutex;
	
	void AddTarget(int32 target_id) {
		std::unique_lock lock(targets_mutex);
		targets.push_back(target_id);
	}
	
	int32 GetPrimaryTargetID() const {
		std::shared_lock lock(targets_mutex);
		return targets.empty() ? -1 : targets[0];
	}

	std::optional<int32_t> GetPrimaryTarget() const {
		std::shared_lock lock(targets_mutex);
		if (!targets.empty())
			return targets[0];
		return std::nullopt;
	}


	std::vector<int32> GetTargets() {
		std::shared_lock lock(targets_mutex);
		return targets;
	}
	
	bool HasNoTargets() const {
		std::shared_lock lock(targets_mutex);
		return targets.empty();
	}
	
	int32 GetTargetCount() const {
		std::shared_lock lock(targets_mutex);
		return static_cast<int32>(targets.size());
	}
	
	bool HasTarget(int32 id) const {
		std::shared_lock lock(targets_mutex);
		return std::find(targets.begin(), targets.end(), id) != targets.end();
	}
	
	bool HasAnyTarget(const std::vector<int32>& ids) const {
		std::shared_lock lock(targets_mutex);
		return std::any_of(
			ids.begin(), ids.end(),
			[this](int32 id){
				return std::find(targets.begin(), targets.end(), id) != targets.end();
			}
		);
	}

	std::vector<int32> GetRemovedTargets() const {
		std::shared_lock lock(targets_mutex);
		return removed_targets;
	}

	void RemoveTarget(int32 target_id) {
		std::unique_lock lock(targets_mutex);
		auto& v = targets;
		v.erase(std::remove(v.begin(), v.end(), target_id), v.end());
		removed_targets.push_back(target_id);
	}
	
	void AddRemoveTarget(int32 target_id) {
		std::unique_lock lock(targets_mutex);
		removed_targets.push_back(target_id);
	}
	
	void SwapTargets(std::vector<int32>& new_targets) {
		std::unique_lock lock(targets_mutex);
		targets.swap(new_targets);
	}

	void ClearTargets() {
		std::unique_lock lock(targets_mutex);
		targets.clear();
		removed_targets.clear();
	}
	
	std::multimap<int32, int8> GetCharIDTargets() const {
		std::shared_lock lock(char_id_targets_mutex);
		return char_id_targets;
	}
	
	void AddCharIDTarget(int32 char_id, int8 value) {
		std::unique_lock lock(char_id_targets_mutex);
		char_id_targets.insert({char_id, value});
	}

	bool HasNoCharIDTargets() const {
		std::shared_lock lock(char_id_targets_mutex);
		return char_id_targets.empty();
	}
	
	void RemoveCharIDTarget(int32 char_id) {
		std::unique_lock lock(char_id_targets_mutex);
		char_id_targets.erase(char_id); // removes all entries with that key
	}

	void RemoveCharIDTargetAndType(int32 char_id, int8 type) {
		std::unique_lock lock(char_id_targets_mutex);

		auto range = char_id_targets.equal_range(char_id);
		for (auto it = range.first; it != range.second; ++it) {
			if (it->second == type) {
				char_id_targets.erase(it);
				break; // remove only one matching pair
			}
		}
	}

	void ClearCharTargets() {
		std::unique_lock lock(char_id_targets_mutex);
		char_id_targets.clear();
	}
	
	void MarkFieldModified(const std::string& field) {
		std::unique_lock lock(spell_modify_mutex);
		modified_fields.insert(field);
	}

	bool IsFieldModified(const std::string& field) const {
		std::shared_lock lock(spell_modify_mutex);
		return modified_fields.find(field) != modified_fields.end();
	}

	void ClearFieldModifications() {
		std::unique_lock lock(spell_modify_mutex);
		modified_fields.clear();
	}
	
	std::unordered_set<std::string> GetModifiedFieldsCopy() const {
		std::shared_lock lock(spell_modify_mutex);
		return modified_fields; // safe shallow copy
	}
	
	bool SetSpellDataGeneric(const std::string& field, int value) {
		return SetSpellDataGeneric(field, std::to_string(value));
	}

	bool SetSpellDataGeneric(const std::string& field, float value) {
		return SetSpellDataGeneric(field, std::to_string(value));
	}

	bool SetSpellDataGeneric(const std::string& field, bool value) {
		return SetSpellDataGeneric(field, value ? "1" : "0");
	}
	
	bool SetSpellDataGeneric(const std::string& field, const std::string& value) {
		auto it = SpellFieldGenericSetters.find(field);
		if (it == SpellFieldGenericSetters.end())
			return false;

		if (!spell)
			return false;

		it->second(spell, value);
		return true;
	}
	
	bool SetSpellDataIndex(int idx, const std::string& value, const std::string& value2 = "");
	
	bool SetSpellDataIndex(int idx, int value, int value2) {
		return SetSpellDataIndex(idx, std::to_string(value), std::to_string(value2));
	}

	bool SetSpellDataIndex(int idx, float value, float value2) {
		return SetSpellDataIndex(idx, std::to_string(value), std::to_string(value2));
	}

	bool SetSpellDataIndex(int idx, bool value) {
		return SetSpellDataIndex(idx, value ? "1" : "0");
	}
};

enum class LuaArgType { SINT64, INT64, SINT, INT, FLOAT, STRING, BOOL, SPAWN, ZONE, SKILL, ITEM, QUEST, SPELL /* etc */ };

struct LuaArg {
	LuaArgType type;
	union {
		sint64 si;
		sint64 i;
		int32 low_i;
		sint32 low_si;
		float f;
		bool b;
	};
	std::string s;
	Spawn* spawn = nullptr;
	ZoneServer* zone = nullptr;
	Skill* skill = nullptr;
	Item* item = nullptr;
	Quest* quest = nullptr;
	LuaSpell* spell = nullptr;

	LuaArg(sint64 val) : type(LuaArgType::SINT64), si(val) {}
	LuaArg(sint32 val) : type(LuaArgType::SINT), low_si(val) {}
	LuaArg(sint16 val) : type(LuaArgType::SINT), low_si(val) {}
	LuaArg(sint8 val) : type(LuaArgType::SINT), low_si(val) {}
	LuaArg(int64 val) : type(LuaArgType::INT64), i(val) {}
	LuaArg(int32 val) : type(LuaArgType::INT), low_i(val) {}
	LuaArg(int16 val) : type(LuaArgType::INT), low_i(val) {}
	LuaArg(int8 val) : type(LuaArgType::INT), low_i(val) {}
	LuaArg(float val) : type(LuaArgType::FLOAT), f(val) {}
	LuaArg(bool val) : type(LuaArgType::BOOL), b(val) {}
	LuaArg(const std::string& val) : type(LuaArgType::STRING), s(val) {}
	LuaArg(Spawn* val) : type(LuaArgType::SPAWN), spawn(val) {}
	LuaArg(ZoneServer* val) : type(LuaArgType::ZONE), zone(val) {}
	LuaArg(Skill* val) : type(LuaArgType::SKILL), skill(val) {}
	LuaArg(Item* val) : type(LuaArgType::ITEM), item(val) {}
	LuaArg(Quest* val) : type(LuaArgType::QUEST), quest(val) {}
	LuaArg(LuaSpell* val) : type(LuaArgType::SPELL), spell(val) {}
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
	bool			LoadItemScript(string name);
	bool			LoadItemScript(const char* name);
	bool			LoadSpawnScript(string name);
	bool			LoadSpawnScript(const char* name);
	bool			LoadZoneScript(string name);
	bool			LoadZoneScript(const char* name);
	bool			LoadPlayerScript(string name);
	bool			LoadPlayerScript(const char* name);
	bool			LoadRegionScript(string name);
	bool			LoadRegionScript(const char* name);
	LuaSpell*		LoadSpellScript(string name);
	LuaSpell*		LoadSpellScript(const char* name);
	void			RemoveSpawnFromSpell(LuaSpell* spell, Spawn* spawn);
	void			RemoveSpell(LuaSpell* spell, bool call_remove_function = true, bool can_delete = true, string reason = "", bool removing_all_spells = false, bool return_after_call_remove = false, Spawn* overrideTarget = nullptr);
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
	void			RemoveCurrentSpell(lua_State* state, LuaSpell* cur_spell, bool needsLock = true, bool removeCurSpell = true, bool removeSpellScript = true);
	bool			CallSpellProcess(LuaSpell* spell, int8 num_parameters, std::string functionCalled);
	LuaSpell*		GetSpell(const char* name, bool use = true);
	void			UseItemScript(const char* name, lua_State* state, bool val);
	void			UseSpawnScript(const char* name, lua_State* state, bool val);
	void			UseZoneScript(const char* name, lua_State* state, bool val);
	void			UsePlayerScript(const char* name, lua_State* state, bool val);
	void			UseRegionScript(const char* name, lua_State* state, bool val);
	lua_State*		GetItemScript(const char* name, bool create_new = true, bool use = false);
	lua_State*		GetSpawnScript(const char* name, bool create_new = true, bool use = false);
	lua_State*		GetZoneScript(const char* name, bool create_new = true, bool use = false);
	lua_State*		GetPlayerScript(const char* name, bool create_new = true, bool use = false);
	lua_State*		GetRegionScript(const char* name, bool create_new = true, bool use = false);
	LuaSpell*		GetSpellScript(const char* name, bool create_new = true, bool use = true);
	LuaSpell*		CreateSpellScript(const char* name, lua_State* existState);
	
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
	bool			RunPlayerScriptWithReturn(const string script_name, const char* function_name, const std::vector<LuaArg>& args, sint32* returnValue = 0);
	bool			CallScriptInt32(lua_State* state, int8 num_parameters, int32* returnValue = 0);
	bool			CallScriptSInt32(lua_State* state, int8 num_parameters, sint32* returnValue = 0);
	bool			RunRegionScript(string script_name, const char* function_name, ZoneServer* zone, Spawn* spawn = 0, sint32 int32_arg1 = 0, int32* returnValue = 0);
	bool			CallRegionScript(lua_State* state, int8 num_parameters, int32* returnValue);
	void			ResetFunctionStack(lua_State* state);
	void			DestroySpells();
	void			DestroySpawnScripts();
	void			DestroyItemScripts();
	void			DestroyQuests(bool reload = false);
	void			DestroyZoneScripts();
	void			DestroyPlayerScripts();
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
	void			DeletePendingSpells(bool all, ZoneServer* zone = nullptr);
	void			DeletePendingSpell(LuaSpell* spell);
	Mutex*			GetSpawnScriptMutex(const char* name);
	Mutex*			GetItemScriptMutex(const char* name);
	Mutex*			GetZoneScriptMutex(const char* name);
	Mutex*			GetPlayerScriptMutex(const char* name);
	Mutex*			GetRegionScriptMutex(const char* name);
	Mutex*			GetSpellScriptMutex(const char* name);
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
	bool			IsLuaUserDataValid(void* ptr);
	
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
	map<lua_State*, string> inverse_spells;

	map<int32, Quest*>		quests;
	map<int32, lua_State*> quest_states;
	map<string, map<lua_State*, bool> > item_scripts;
	map<string, map<lua_State*, bool> > spawn_scripts;
	map<string, map<lua_State*, bool> > zone_scripts;
	map<string, map<lua_State*, bool> > player_scripts;
	map<string, map<lua_State*, bool> > region_scripts;
	map<string, map<lua_State*, LuaSpell*> > spell_scripts;

	map<int32, LuaSpell*> custom_spells;
	std::deque<int32> custom_free_spell_ids;

	map<lua_State*, string> item_inverse_scripts;
	map<lua_State*, string> spawn_inverse_scripts;
	map<lua_State*, string> zone_inverse_scripts;
	map<lua_State*, string> player_inverse_scripts;
	map<lua_State*, string> region_inverse_scripts;

	map<string, Mutex*> item_scripts_mutex;
	map<string, Mutex*> spawn_scripts_mutex;
	map<string, Mutex*> zone_scripts_mutex;
	map<string, Mutex*> player_scripts_mutex;
	map<int32, Mutex*> quests_mutex;
	map<string, Mutex*> region_scripts_mutex;
	map<string, Mutex*> spell_scripts_mutex;

	Mutex			MDebugClients;
	Mutex			MSpells;
	Mutex			MSpawnScripts;
	Mutex			MItemScripts;
	Mutex			MZoneScripts;
	Mutex			MPlayerScripts;
	Mutex			MQuests;
	Mutex			MLUAMain;
	Mutex			MSpellDelete;
	Mutex			MCustomSpell;
	Mutex			MRegionScripts;
	Mutex			MSpellScripts;
	
	mutable std::shared_mutex	MLUAUserData;
};
#endif
