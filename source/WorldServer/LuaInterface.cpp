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
#include "LuaInterface.h"
#include "LuaFunctions.h"
#include "WorldDatabase.h"
#include "SpellProcess.h"
#include "../common/Log.h"
#include "World.h"

#ifndef WIN32
    #include <stdio.h>
    #include <sys/types.h>
	#include <sys/stat.h>
    #include <dirent.h>
    #include <pthread.h>
#else
	#include <process.h>
#endif

extern WorldDatabase database;
extern ZoneList zone_list;


LuaInterface::LuaInterface() {
	shutting_down = false;
	lua_system_reloading = false;
	MDebugClients.SetName("LuaInterface::MDebugClients");
	MSpells.SetName("LuaInterface::MSpells");
	MSpawnScripts.SetName("LuaInterface::MSpawnScripts");
	MZoneScripts.SetName("LuaInterface::MZoneScripts");
	MQuests.SetName("LuaInterface::MQuests");
	MLUAMain.SetName("LuaInterface::MLUAMain");
	MItemScripts.SetName("LuaInterface::MItemScripts");
	MSpellDelete.SetName("LuaInterface::MSpellDelete");
	MCustomSpell.SetName("LuaInterface::MCustomSpell");
	MRegionScripts.SetName("LuaInterface::MRegionScripts");
	user_data_timer = new Timer(20000);
	user_data_timer->Start();
	spell_delete_timer = new Timer(5000);
	spell_delete_timer->Start();
}
#ifdef WIN32
vector<string>* LuaInterface::GetDirectoryListing(const char* directory) {
	vector<string>* ret = new vector<string>;
    WIN32_FIND_DATA fdata;
    HANDLE dhandle;
    char buf[MAX_PATH];
    snprintf(buf, sizeof(buf), "%s\\*", directory);
    if((dhandle = FindFirstFile(buf, &fdata)) == INVALID_HANDLE_VALUE) {
		safe_delete(ret);
		return 0;
    }

    ret->push_back(string(fdata.cFileName));

    while(1) {
        if(FindNextFile(dhandle, &fdata)) {
			ret->push_back(string(fdata.cFileName));
        } 
		else{
            if(GetLastError() == ERROR_NO_MORE_FILES) {
                break;
            } else {
				safe_delete(ret);
                FindClose(dhandle);
                return 0;
            }
        }
    }

    if(FindClose(dhandle) == 0) {
		safe_delete(ret);
        return 0;
    }
	return ret;
}
#else
vector<string>* LuaInterface::GetDirectoryListing(const char* directory) {
	vector<string>* ret = new vector<string>;
	DIR *dp;
	struct dirent *ep;
	dp = opendir (directory);
	if (dp != NULL){
		while ((ep = readdir (dp)))
			ret->push_back(string(ep->d_name));
		(void) closedir (dp);
    }
	else {
		safe_delete(ret);
		return 0;
	}

	return ret;
}
#endif

LuaInterface::~LuaInterface() {
	shutting_down = true;
	MLUAMain.lock();
	DestroySpells();
	DestroySpawnScripts();
	DestroyQuests();
	DestroyItemScripts();
	DestroyZoneScripts();
	DestroyRegionScripts();
	DeleteUserDataPtrs(true);
	DeletePendingSpells(true);
	safe_delete(user_data_timer);
	safe_delete(spell_delete_timer);
	MLUAMain.unlock();
}

int LuaInterface::GetNumberOfArgs(lua_State* state) {
	return lua_gettop(state);
}

void LuaInterface::Process() {
	if(shutting_down)
		return;
	MLUAMain.lock();
	if(user_data_timer && user_data_timer->Check())
		DeleteUserDataPtrs(false);
	if(spell_delete_timer && spell_delete_timer->Check())
		DeletePendingSpells(false);
	MLUAMain.unlock();
}

void LuaInterface::DestroySpells() {
	MSpells.lock();
	map<string, map<lua_State*, LuaSpell*> >::iterator spell_script_itr;
	for(spell_script_itr = spell_scripts.begin(); spell_script_itr != spell_scripts.end(); spell_script_itr++) {
		map<lua_State*, LuaSpell*>::iterator inner_itr;
		for(inner_itr = spell_script_itr->second.begin(); inner_itr != spell_script_itr->second.end(); inner_itr++) {
			LuaSpell* cur_spell = inner_itr->second;
			MSpellDelete.lock();
			RemoveCurrentSpell(inner_itr->first, inner_itr->second, false, true, false);
			MSpellDelete.unlock();
			lua_close(inner_itr->first);
			// spell is deleted in this context by SpellProcess::DeleteSpell because removing_all_spells = true
		}
		
		Mutex* mutex = GetSpellScriptMutex(spell_script_itr->first.c_str());
		safe_delete(mutex);
	}
	current_spells.clear();
	spell_scripts_mutex.clear();
	spell_scripts.clear();
	MSpells.unlock();
}

void LuaInterface::DestroyQuests(bool reload) {
	map<int32, lua_State*>::iterator itr;
	MQuests.lock();
	for(itr = quest_states.begin(); itr != quest_states.end(); itr++){
		safe_delete(quests[itr->first]);
		lua_close(itr->second);
	}
	quests.clear();
	quest_states.clear();
	map<int32, Mutex*>::iterator mutex_itr;
	for(mutex_itr = quests_mutex.begin(); mutex_itr != quests_mutex.end(); mutex_itr++){
		safe_delete(mutex_itr->second);
	}
	quests_mutex.clear();
	if(reload)
		database.LoadQuests();
	MQuests.unlock();
}

void LuaInterface::DestroyItemScripts() {
	map<string, map<lua_State*, bool> >::iterator itr;
	map<lua_State*, bool>::iterator state_itr;
	Mutex* mutex = 0;
	MItemScripts.writelock(__FUNCTION__, __LINE__);
	for(itr = item_scripts.begin(); itr != item_scripts.end(); itr++){
		mutex = GetItemScriptMutex(itr->first.c_str());
		mutex->writelock(__FUNCTION__, __LINE__);
		for(state_itr = itr->second.begin(); state_itr != itr->second.end(); state_itr++)
			lua_close(state_itr->first);
		mutex->releasewritelock(__FUNCTION__, __LINE__);
		safe_delete(mutex);
	}
	item_scripts.clear();
	item_inverse_scripts.clear();
	item_scripts_mutex.clear();
	MItemScripts.releasewritelock(__FUNCTION__, __LINE__);
}

void LuaInterface::DestroySpawnScripts() {
	map<string, map<lua_State*, bool> >::iterator itr;
	map<lua_State*, bool>::iterator state_itr;
	Mutex* mutex = 0;
	MSpawnScripts.writelock(__FUNCTION__, __LINE__);
	for(itr = spawn_scripts.begin(); itr != spawn_scripts.end(); itr++){
		mutex = GetSpawnScriptMutex(itr->first.c_str());
		mutex->writelock(__FUNCTION__, __LINE__);
		for(state_itr = itr->second.begin(); state_itr != itr->second.end(); state_itr++)
			lua_close(state_itr->first);
		mutex->releasewritelock(__FUNCTION__, __LINE__);
		safe_delete(mutex);
	}
	spawn_scripts.clear();
	spawn_inverse_scripts.clear();
	spawn_scripts_mutex.clear();
	MSpawnScripts.releasewritelock(__FUNCTION__, __LINE__);
}

void LuaInterface::DestroyZoneScripts()  {
	map<string, map<lua_State*, bool> >::iterator itr;
	map<lua_State*, bool>::iterator state_itr;
	Mutex* mutex = 0;
	MZoneScripts.writelock(__FUNCTION__, __LINE__);
	for (itr = zone_scripts.begin(); itr != zone_scripts.end(); itr++){
		mutex = GetZoneScriptMutex(itr->first.c_str());
		mutex->writelock(__FUNCTION__, __LINE__);
		for(state_itr = itr->second.begin(); state_itr != itr->second.end(); state_itr++)
			lua_close(state_itr->first);
		mutex->releasewritelock(__FUNCTION__, __LINE__);
		safe_delete(mutex);
	}
	zone_scripts.clear();
	zone_inverse_scripts.clear();
	zone_scripts_mutex.clear();
	MZoneScripts.releasewritelock(__FUNCTION__, __LINE__);
}

void LuaInterface::DestroyRegionScripts()  {
	map<string, map<lua_State*, bool> >::iterator itr;
	map<lua_State*, bool>::iterator state_itr;
	Mutex* mutex = 0;
	MRegionScripts.writelock(__FUNCTION__, __LINE__);
	for (itr = region_scripts.begin(); itr != region_scripts.end(); itr++){
		mutex = GetRegionScriptMutex(itr->first.c_str());
		mutex->writelock(__FUNCTION__, __LINE__);
		for(state_itr = itr->second.begin(); state_itr != itr->second.end(); state_itr++)
			lua_close(state_itr->first);
		mutex->releasewritelock(__FUNCTION__, __LINE__);
		safe_delete(mutex);
	}
	region_scripts.clear();
	region_inverse_scripts.clear();
	region_scripts_mutex.clear();
	MRegionScripts.releasewritelock(__FUNCTION__, __LINE__);
}

void LuaInterface::ReloadSpells() {
	DestroySpells();
	database.LoadSpellScriptData();
}

bool LuaInterface::LoadItemScript(string name) {
	return LoadItemScript(name.c_str());
}

bool LuaInterface::LoadItemScript(const char* name) {
	bool ret = false;
	if(name){
		lua_State* state = LoadLuaFile(name);
		if(state){
			MItemScripts.writelock(__FUNCTION__, __LINE__);
			item_scripts[name][state] = false;
			MItemScripts.releasewritelock(__FUNCTION__, __LINE__);
			ret = true;
		}
	}
	return ret;
}

bool LuaInterface::LoadSpawnScript(const char* name) {
	bool ret = false;
	if(name){
		lua_State* state = LoadLuaFile(name);
		if(state){
			MSpawnScripts.writelock(__FUNCTION__, __LINE__);
			spawn_scripts[name][state] = false;
			MSpawnScripts.releasewritelock(__FUNCTION__, __LINE__);
			ret = true;
		}
	}
	return ret;
}

bool LuaInterface::LoadZoneScript(const char* name)  {
	bool ret = false;
	if (name) {
		lua_State* state = LoadLuaFile(name);
		if (state) {
			MZoneScripts.writelock(__FUNCTION__, __LINE__);
			zone_scripts[name][state] = false;
			MZoneScripts.releasewritelock(__FUNCTION__, __LINE__);
			ret = true;
		}
	}
	return ret;
}

bool LuaInterface::LoadRegionScript(const char* name)  {
	bool ret = false;
	if (name) {
		lua_State* state = LoadLuaFile(name);
		if (state) {
			MRegionScripts.writelock(__FUNCTION__, __LINE__);
			region_scripts[name][state] = false;
			MRegionScripts.releasewritelock(__FUNCTION__, __LINE__);
			ret = true;
		}
	}
	return ret;
}

void LuaInterface::ProcessErrorMessage(const char* message) {
	MDebugClients.lock();
	vector<Client*> delete_clients;
	map<Client*, int32>::iterator itr;
	for(itr = debug_clients.begin(); itr != debug_clients.end(); itr++){
		if((Timer::GetCurrentTime2() - itr->second) > 60000)
			delete_clients.push_back(itr->first);
		else
			itr->first->Message(CHANNEL_COLOR_RED, "LUA Error: %s", message);
	}
	for(int32 i=0;i<delete_clients.size();i++)
		debug_clients.erase(delete_clients[i]);
	MDebugClients.unlock();
}

void LuaInterface::RemoveDebugClients(Client* client) {
	MDebugClients.lock();
	debug_clients.erase(client);
	MDebugClients.unlock();
}

void LuaInterface::UpdateDebugClients(Client* client) {
	MDebugClients.lock();
	debug_clients[client] = Timer::GetCurrentTime2();
	MDebugClients.unlock();
}

Mutex*  LuaInterface::GetQuestMutex(Quest* quest) {
	Mutex* ret = 0;
	MQuests.lock();
	if(quests_mutex.count(quest->GetQuestID()) == 0){
		ret = new Mutex();
		quests_mutex[quest->GetQuestID()] = ret;
		ret->SetName(string("Quest::").append(quest->GetName()));
	}
	else
		ret = quests_mutex[quest->GetQuestID()];
	MQuests.unlock();
	return ret;
}

bool LuaInterface::CallQuestFunction(Quest* quest, const char* function, Spawn* player, int32 step_id, int32* returnValue) {
	if(shutting_down)
		return false;
	lua_State* state = 0;
	if(quest){
		LogWrite(LUA__DEBUG, 0, "LUA", "Quest: %s, function: %s", quest->GetName(), function);
		Mutex* mutex = GetQuestMutex(quest);
		mutex->readlock(__FUNCTION__, __LINE__);
		if(quest_states.count(quest->GetQuestID()) > 0)
			state = quest_states[quest->GetQuestID()];
		bool success = false; // if no state then we return false
		if(state){
			int8 arg_count = 3;
			lua_getglobal(state, function);
			
			if (!lua_isfunction(state, lua_gettop(state))){
				lua_pop(state, 1);
				mutex->releasereadlock(__FUNCTION__);
				return false;
			}
			
			SetQuestValue(state, quest);
			Spawn* spawn = player->GetZone()->GetSpawnByDatabaseID(quest->GetQuestGiver());
			SetSpawnValue(state, spawn);
			SetSpawnValue(state, player);
			if(step_id != 0xFFFFFFFF){
				SetInt32Value(state, step_id);
				arg_count++;
			}
			
			success = CallScriptInt32(state, arg_count, returnValue);
		}
		mutex->releasereadlock(__FUNCTION__, __LINE__);
		LogWrite(LUA__DEBUG, 0, "LUA", "Done!");
		return success;
	}
	return false;
}

Quest* LuaInterface::LoadQuest(int32 id, const char* name, const char* type, const char* zone, int8 level, const char* description, char* script_name) {
	if(shutting_down)
		return 0;
	lua_State* state = LoadLuaFile(script_name);
	Quest* quest = 0;
	if(state){
		quest = new Quest(id);
		if (name)
			quest->SetName(string(name));
		if (type)
			quest->SetType(string(type));
		if (zone)
			quest->SetZone(string(zone));
		quest->SetLevel(level);
		if (description)
			quest->SetDescription(string(description));
		lua_getglobal(state, "Init");
		SetQuestValue(state, quest);
		if(lua_pcall(state, 1, 0, 0) != 0){
			LogError("Error processing Quest \"%s\" (%u): %s", name ? name : "unknown", id, lua_tostring(state, -1));
			lua_pop(state, 1);
			SetLuaUserDataStale(quest);
			safe_delete(quest);
			return 0;
		}
		if(!quest->GetName()){
			SetLuaUserDataStale(quest);
			safe_delete(quest);	
			return 0;
		}
		quest_states[id] = state;
		quests[id] = quest;
	}
	return quest;
}

const char* LuaInterface::GetScriptName(lua_State* state)
{
	map<lua_State*, string>::iterator itr;
	MItemScripts.writelock(__FUNCTION__, __LINE__);
	itr = item_inverse_scripts.find(state);
	if (itr != item_inverse_scripts.end())
	{
		const char* scriptName = itr->second.c_str();
		MItemScripts.releasewritelock(__FUNCTION__, __LINE__);
		return scriptName;
	}
	MItemScripts.releasewritelock(__FUNCTION__, __LINE__);

	MSpawnScripts.writelock(__FUNCTION__, __LINE__);
	itr = spawn_inverse_scripts.find(state);
	if (itr != spawn_inverse_scripts.end())
	{
		const char* scriptName = itr->second.c_str();
		MSpawnScripts.releasewritelock(__FUNCTION__, __LINE__);
		return scriptName;
	}
	MSpawnScripts.releasewritelock(__FUNCTION__, __LINE__);

	MZoneScripts.writelock(__FUNCTION__, __LINE__);
	itr = zone_inverse_scripts.find(state);
	if (itr != zone_inverse_scripts.end())
	{
		const char* scriptName = itr->second.c_str();
		MZoneScripts.releasewritelock(__FUNCTION__, __LINE__);
		return scriptName;
	}
	MZoneScripts.releasewritelock(__FUNCTION__, __LINE__);
	
	MRegionScripts.writelock(__FUNCTION__, __LINE__);
	itr = region_inverse_scripts.find(state);
	if (itr != region_inverse_scripts.end())
	{
		const char* scriptName = itr->second.c_str();
		MRegionScripts.releasewritelock(__FUNCTION__, __LINE__);
		return scriptName;
	}
	MRegionScripts.releasewritelock(__FUNCTION__, __LINE__);

	MSpells.lock();
	LuaSpell* spell = GetCurrentSpell(state, false);
	if (spell)
	{
		const char* fileName = (spell->file_name.length() > 0) ? spell->file_name.c_str() : "";
		MSpells.unlock();
		return fileName;
	}
	MSpells.unlock();

	return "";
}

bool LuaInterface::LoadSpawnScript(string name) {
	return LoadSpawnScript(name.c_str());
}

bool LuaInterface::LoadZoneScript(string name) {
	return LoadZoneScript(name.c_str());
}

bool LuaInterface::LoadRegionScript(string name) {
	return LoadRegionScript(name.c_str());
}

LuaSpell* LuaInterface::LoadSpellScript(string name) {
	return LoadSpellScript(name.c_str());
}

std::string LuaInterface::AddSpawnPointers(LuaSpell* spell, bool first_cast, bool precast, const char* function, SpellScriptTimer* timer, bool passLuaSpell, Spawn* altTarget) {
	std::string functionCalled = string(""); 
	if (function)
	{
		functionCalled = string(function);
		lua_getglobal(spell->state, function);
	}
	else if (precast)
	{
		functionCalled = "precast";
		lua_getglobal(spell->state, "precast");
	}
	else if(first_cast)
	{
		functionCalled = "cast";
		lua_getglobal(spell->state, "cast");
	}
	else
	{
		functionCalled = "tick";
		lua_getglobal(spell->state, "tick");
	}
	
	LogWrite(SPELL__DEBUG, 0, "Spell", "LuaInterface::AddSpawnPointers spell %s (%u) function %s, caster %s.", spell->spell ? spell->spell->GetName() : "UnknownUnset", spell->spell ? spell->spell->GetSpellID() : 0, functionCalled.c_str(), spell->caster ? spell->caster->GetName() : "Unknown");

	if (!lua_isfunction(spell->state, lua_gettop(spell->state))){
		lua_pop(spell->state, 1);
		return string("");
	}

	if(passLuaSpell)
		SetSpellValue(spell->state, spell);

	Spawn* temp_spawn = 0;
	if (timer && timer->caster && spell->caster && spell->caster->GetZone())
		temp_spawn = spell->caster->GetZone()->GetSpawnByID(timer->caster);

	bool spawnSet = true;
	if (temp_spawn)
		SetSpawnValue(spell->state, temp_spawn);
	else if (spell->caster)
		SetSpawnValue(spell->state, spell->caster);
	else
		spawnSet = false;
	
	if(!spawnSet) {
		SetSpawnValue(spell->state, 0);
	}

	temp_spawn = 0;
	spawnSet = true;
	
	if (timer && timer->target && spell->caster && spell->caster->GetZone())
		temp_spawn = spell->caster->GetZone()->GetSpawnByID(timer->target);

	if (temp_spawn)
		SetSpawnValue(spell->state, temp_spawn);
	else {
		if(altTarget)
		{
			SetSpawnValue(spell->state, altTarget);
		}
		else if(spell->caster && spell->caster->GetZone() != nullptr && spell->initial_target)
		{
			// easier to debug target id as ptr
			Spawn* new_target = spell->caster->GetZone()->GetSpawnByID(spell->initial_target);
			SetSpawnValue(spell->state, new_target);
		}
		else if(spell->caster && spell->caster->GetTarget())
			SetSpawnValue(spell->state, spell->caster->GetTarget());
		else
			SetSpawnValue(spell->state, 0);
	}

	return functionCalled;
}

LuaSpell* LuaInterface::GetCurrentSpell(lua_State* state, bool needsLock) {
	LuaSpell* spell = 0;
	
	if(needsLock)
		MSpells.lock();
	
	if(current_spells.count(state) > 0)
		spell = current_spells[state];
	
	if(needsLock)
		MSpells.unlock();
	
	return spell;
}

void LuaInterface::RemoveCurrentSpell(lua_State* state, LuaSpell* cur_spell, bool needsLock, bool removeCurSpell, bool removeSpellScript) {
	if(needsLock) {
		MSpells.lock();
		MSpellDelete.lock();
	}
	map<lua_State*, LuaSpell*>::iterator itr = current_spells.find(state);
	if(removeSpellScript && itr->second) {
		MSpellScripts.writelock(__FUNCTION__, __LINE__);
		map<string, map<lua_State*, LuaSpell*> >::iterator spell_script_itr = spell_scripts.find(cur_spell->file_name);
		if(spell_script_itr != spell_scripts.end()) {
			LogWrite(SPELL__DEBUG, 9, "Spell", "LuaInterface::RemoveCurrentSpell spell %s.  Queue Entries %u.", cur_spell->file_name.c_str(), spell_script_itr->second.size());
			Mutex* mutex = GetSpellScriptMutex(cur_spell->file_name.c_str());
			mutex->writelock(__FUNCTION__, __LINE__);
			map<lua_State*, LuaSpell*>::iterator spell_script_itr2 = spell_script_itr->second.find(state);
			if(spell_script_itr2 != spell_script_itr->second.end()) {
				spell_script_itr2->second = nullptr;
			}
			mutex->releasewritelock(__FUNCTION__, __LINE__);
		}
		MSpellScripts.releasewritelock(__FUNCTION__, __LINE__);
	}
	if(itr != current_spells.end() && removeCurSpell)
		current_spells.erase(itr);
	if(needsLock) {
		MSpellDelete.unlock();
		MSpells.unlock();
	}
}

bool LuaInterface::CallSpellProcess(LuaSpell* spell, int8 num_parameters, std::string customFunction) {
	if(shutting_down || !spell || !spell->caster)
		return false;
	
	LogWrite(SPELL__DEBUG, 0, "Spell", "LuaInterface::CallSpellProcess spell %s (%u) function %s, caster %s.", spell->spell ? spell->spell->GetName() : "UnknownUnset", spell->spell ? spell->spell->GetSpellID() : 0, customFunction.c_str(), spell->caster->GetName());
	
	if(lua_pcall(spell->state, num_parameters, 0, 0) != 0){
		LogError("Error running function '%s' in %s: %s", customFunction.c_str(), spell->spell->GetName(), lua_tostring(spell->state, -1));
		lua_pop(spell->state, 1);
		ResetFunctionStack(spell->state);
		RemoveSpell(spell, false); // may be in a lock
		return false;
	}
	ResetFunctionStack(spell->state);
	return true;
}

void LuaInterface::RemoveSpawnScript(const char* name) {
	lua_State* state = 0;
	Mutex* mutex = GetSpawnScriptMutex(name);
	while((state = GetSpawnScript(name, false))){
		mutex->writelock(__FUNCTION__, __LINE__);
		lua_close(state);
		spawn_scripts[name].erase(state);
		mutex->releasewritelock(__FUNCTION__, __LINE__);
	}
	MSpawnScripts.writelock(__FUNCTION__, __LINE__);
	spawn_scripts.erase(name);
	MSpawnScripts.releasewritelock(__FUNCTION__, __LINE__);
}

bool LuaInterface::CallItemScript(lua_State* state, int8 num_parameters, std::string* returnValue) {
	if(shutting_down)
		return false;
	if(!state || lua_pcall(state, num_parameters, 1, 0) != 0){
		if (state){
			const char* err = lua_tostring(state, -1);
			LogError("%s: %s", GetScriptName(state), err);
			lua_pop(state, 1);
		}
		return false;
	}

	std::string result = std::string("");
	
	if(lua_isstring(state, -1)){
		size_t size = 0;
		const char* str = lua_tolstring(state, -1, &size);
		if(str)
			result = string(str);
	}
	
	if(returnValue)
		*returnValue = std::string(result);
	
	return true;
}

bool LuaInterface::CallItemScript(lua_State* state, int8 num_parameters, sint64* returnValue) {
	if(shutting_down)
		return false;
	if(!state || lua_pcall(state, num_parameters, 1, 0) != 0){
		if (state){
			const char* err = lua_tostring(state, -1);
			LogError("%s: %s", GetScriptName(state), err);
			lua_pop(state, 1);
		}
		return false;
	}

	sint64 result = 0;
	
	if (lua_isnumber(state, -1))
	{
		result = (sint64)lua_tonumber(state, -1);
		lua_pop(state, 1);
	}
	
	if(returnValue)
		*returnValue = result;
	
	return true;
}

bool LuaInterface::CallSpawnScript(lua_State* state, int8 num_parameters) {
	if(shutting_down || lua_system_reloading)
		return false;
	if(!state || lua_pcall(state, num_parameters, 0, 0) != 0){
		if (state){
			const char* err = lua_tostring(state, -1);
			LogError("%s: %s", GetScriptName(state), err);
			lua_pop(state, 1);
		}
		return false;
	}
	return true;
}

bool LuaInterface::CallScriptInt32(lua_State* state, int8 num_parameters, int32* returnValue) {
	if(shutting_down)
		return false;
	if (!state || lua_pcall(state, num_parameters, 1, 0) != 0) {
		if (state){
			const char* err = lua_tostring(state, -1);
			LogError("%s: %s", GetScriptName(state), err);
			lua_pop(state, 1);
		}
		return false;
	}
	
	int32 result = 0;
	
	if (lua_isnumber(state, -1))
	{
		result = (int32)lua_tonumber(state, -1);
		lua_pop(state, 1);
	}
	
	if(returnValue)
		*returnValue = result;
	
	return true;
}

bool LuaInterface::CallScriptSInt32(lua_State* state, int8 num_parameters, sint32* returnValue) {
	if(shutting_down)
		return false;
	if (!state || lua_pcall(state, num_parameters, 1, 0) != 0) {
		if (state){
			const char* err = lua_tostring(state, -1);
			LogError("%s: %s", GetScriptName(state), err);
			lua_pop(state, 1);
		}
		return false;
	}
	
	sint32 result = 0;
	
	if (lua_isnumber(state, -1))
	{
		result = (sint32)lua_tointeger(state, -1);
		lua_pop(state, 1);
	}
	
	if(returnValue)
		*returnValue = result;
	
	return true;
}

bool LuaInterface::CallRegionScript(lua_State* state, int8 num_parameters, int32* returnValue) {
	if(shutting_down)
		return false;
	if (!state || lua_pcall(state, num_parameters, 1, 0) != 0) {
		if (state){
			const char* err = lua_tostring(state, -1);
			LogError("%s: %s", GetScriptName(state), err);
			lua_pop(state, 1);
		}
		return false;
	}
	
	int32 result = 0;
	
	if (lua_isnumber(state, -1))
	{
		result = (int32)lua_tonumber(state, -1);
		lua_pop(state, 1);
	}
	
	if(returnValue)
		*returnValue = result;
	
	return true;
}

lua_State* LuaInterface::LoadLuaFile(const char* name) {
	if(shutting_down)
		return 0;
	lua_State* state = luaL_newstate();
	luaL_openlibs(state);
	if(luaL_dofile(state, name) == 0){
		RegisterFunctions(state);
		return state;
	}
	else{
		LogError("Error loading %s (file name: '%s')", lua_tostring(state, -1), name);
		lua_pop(state, 1);
		lua_close(state);
	}
	return 0;
}

void LuaInterface::RemoveSpell(LuaSpell* spell, bool call_remove_function, bool can_delete, string reason, bool removing_all_spells) {
	if(call_remove_function){
		lua_getglobal(spell->state, "remove");
		if (!lua_isfunction(spell->state, lua_gettop(spell->state))){
			lua_pop(spell->state, 1);
		}
		else {
			LUASpawnWrapper* spawn_wrapper = new LUASpawnWrapper();
			spawn_wrapper->spawn = spell->caster;
			AddUserDataPtr(spawn_wrapper, spawn_wrapper->spawn);
			lua_pushlightuserdata(spell->state, spawn_wrapper);
			if(spell->caster && (spell->initial_target || spell->caster->GetTarget())){
				spawn_wrapper = new LUASpawnWrapper();
				if(!spell->initial_target)
					spawn_wrapper->spawn = spell->caster->GetTarget();
				else if(spell->caster->GetZone()) {
					spawn_wrapper->spawn = spell->caster->GetZone()->GetSpawnByID(spell->initial_target);
				}
				else {
					spawn_wrapper->spawn = nullptr; // we need it set to something or else the ptr could be loose
				}
				AddUserDataPtr(spawn_wrapper, spawn_wrapper->spawn);
				lua_pushlightuserdata(spell->state, spawn_wrapper);
			}
			else
				lua_pushlightuserdata(spell->state, 0);

			if (spell->caster && !spell->caster->Alive())
				reason = "dead";

			lua_pushstring(spell->state, (char*)reason.c_str());
			
			lua_pcall(spell->state, 3, 0, 0); 
			ResetFunctionStack(spell->state);
		}
	}
	
	spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
	if(spell->caster) {
		for (int8 i = 0; i < spell->targets.size(); i++) {
			if(!spell->caster->GetZone())
				continue;
			
			Spawn* target = spell->caster->GetZone()->GetSpawnByID(spell->targets.at(i));
			if (!target || !target->IsEntity())
				continue;

			((Entity*)target)->RemoveProc(0, spell);
			((Entity*)target)->RemoveSpellEffect(spell);
			((Entity*)target)->RemoveSpellBonus(spell);
		}
	}

	multimap<int32,int8>::iterator entries;
	for(entries = spell->char_id_targets.begin(); entries != spell->char_id_targets.end(); entries++)
	{
		Client* tmpClient = zone_list.GetClientByCharID(entries->first);
		if(tmpClient && tmpClient->GetPlayer())
		{
			tmpClient->GetPlayer()->RemoveProc(0, spell);
			tmpClient->GetPlayer()->RemoveSpellEffect(spell);
			tmpClient->GetPlayer()->RemoveSpellBonus(spell);
		}
	}
	spell->char_id_targets.clear(); // TODO: reach out to those clients in different
	spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);

	if(removing_all_spells) {
		if(spell->caster && spell->caster->GetZone()) {
			spell->caster->GetZone()->GetSpellProcess()->RemoveSpellScriptTimerBySpell(spell);
			spell->caster->GetZone()->GetSpellProcess()->DeleteSpell(spell);
		}
	}
	else {
		AddPendingSpellDelete(spell);
		if (spell->caster)
		{
			if(spell->caster->GetZone()) {
				spell->caster->GetZone()->GetSpellProcess()->RemoveSpellScriptTimerBySpell(spell);
			}
			spell->caster->RemoveProc(0, spell);
			spell->caster->RemoveMaintainedSpell(spell);

			int8 spell_type = spell->spell->GetSpellData()->spell_type;
			if(spell->caster->IsPlayer() && !removing_all_spells)
			{
				Player* player = (Player*)spell->caster;
				switch(spell_type)
				{
					case SPELL_TYPE_FOOD:
						if(player->get_character_flag(CF_FOOD_AUTO_CONSUME) && player->GetClient())
						{
							Item* item = nullptr;
							if(player->GetActiveFoodUniqueID()) {
								item = player->item_list.GetItemFromUniqueID(player->GetActiveFoodUniqueID());
							}
							if(item == nullptr) {
								item = player->GetEquipmentList()->GetItem(EQ2_FOOD_SLOT);
							}
							if(item && player->GetClient()->CheckConsumptionAllowed(EQ2_FOOD_SLOT, false))
								player->GetClient()->ConsumeFoodDrink(item, EQ2_FOOD_SLOT);
						}
					break;
					case SPELL_TYPE_DRINK:
						if(player->get_character_flag(CF_DRINK_AUTO_CONSUME) && player->GetClient())
						{
							Item* item = nullptr;
							if(player->GetActiveDrinkUniqueID()) {
								item = player->item_list.GetItemFromUniqueID(player->GetActiveDrinkUniqueID());
							}
							if(item == nullptr) {
								item = player->GetEquipmentList()->GetItem(EQ2_FOOD_SLOT);
							}
							
							item = player->GetEquipmentList()->GetItem(EQ2_DRINK_SLOT);
							if(item && player->GetClient()->CheckConsumptionAllowed(EQ2_DRINK_SLOT, false))
								player->GetClient()->ConsumeFoodDrink(item, EQ2_DRINK_SLOT);
						}
					break;
				}
			}
		}
	}
}

void LuaInterface::RegisterFunctions(lua_State* state) {
	lua_register(state, "SetHP", EQ2Emu_lua_SetCurrentHP);
	lua_register(state, "SetMaxHP", EQ2Emu_lua_SetMaxHP);
	lua_register(state, "SetMaxHPBase", EQ2Emu_lua_SetMaxHPBase);
	lua_register(state, "SetPower", EQ2Emu_lua_SetCurrentPower);
	lua_register(state, "SetMaxPower", EQ2Emu_lua_SetMaxPower);
	lua_register(state, "SetMaxPowerBase", EQ2Emu_lua_SetMaxPowerBase);
	lua_register(state, "ModifyMaxHP", EQ2Emu_lua_ModifyMaxHP);
	lua_register(state, "ModifyMaxPower", EQ2Emu_lua_ModifyMaxPower);
	lua_register(state, "SetPosition", EQ2Emu_lua_SetPosition);
	lua_register(state, "SetHeading", EQ2Emu_lua_SetHeading);
	lua_register(state, "SetModelType", EQ2Emu_lua_SetModelType);
	lua_register(state, "SetAdventureClass", EQ2Emu_lua_SetAdventureClass);
	lua_register(state, "SetTradeskillClass", EQ2Emu_lua_SetTradeskillClass);
	lua_register(state, "SetMount", EQ2Emu_lua_SetMount);
	lua_register(state, "SetMountColor", EQ2Emu_lua_SetMountColor);
	lua_register(state, "GetMount", EQ2Emu_lua_GetMount);
	lua_register(state, "GetRace", EQ2Emu_lua_GetRace);
	lua_register(state, "GetRaceName", EQ2Emu_lua_GetRaceName);
	lua_register(state, "GetClass", EQ2Emu_lua_GetClass);
	lua_register(state, "GetClassName", EQ2Emu_lua_GetClassName);
	lua_register(state, "GetArchetypeName", EQ2Emu_lua_GetArchetypeName);
	lua_register(state, "SetSpeed", EQ2Emu_lua_SetSpeed);
	lua_register(state, "ModifyPower", EQ2Emu_lua_ModifyPower);
	lua_register(state, "ModifyHP", EQ2Emu_lua_ModifyHP);

	lua_register(state, "GetDistance", EQ2Emu_lua_GetDistance);
	lua_register(state, "GetHeading", EQ2Emu_lua_GetHeading);
	lua_register(state, "GetLevel", EQ2Emu_lua_GetLevel);
	lua_register(state, "GetDifficulty", EQ2Emu_lua_GetDifficulty);
	lua_register(state, "GetHP", EQ2Emu_lua_GetCurrentHP);
	lua_register(state, "GetMaxHP", EQ2Emu_lua_GetMaxHP);
	lua_register(state, "GetMaxHPBase", EQ2Emu_lua_GetMaxHPBase);
	lua_register(state, "GetMaxPower", EQ2Emu_lua_GetMaxPower);
	lua_register(state, "GetMaxPowerBase", EQ2Emu_lua_GetMaxPowerBase);
	lua_register(state, "GetName", EQ2Emu_lua_GetName);
	lua_register(state, "GetPower", EQ2Emu_lua_GetCurrentPower);
	lua_register(state, "GetX", EQ2Emu_lua_GetX);
	lua_register(state, "GetY", EQ2Emu_lua_GetY);
	lua_register(state, "GetZ", EQ2Emu_lua_GetZ);
	lua_register(state, "GetSpawnID", EQ2Emu_lua_GetSpawnID);
	lua_register(state, "GetSpawnGroupID", EQ2Emu_lua_GetSpawnGroupID);
	lua_register(state, "SetSpawnGroupID", EQ2Emu_lua_SetSpawnGroupID);
	lua_register(state, "AddSpawnToGroup", EQ2Emu_lua_AddSpawnToGroup);
	lua_register(state, "GetSpawnLocationID", EQ2Emu_lua_GetSpawnLocationID);
	lua_register(state, "GetSpawnLocationPlacementID", EQ2Emu_lua_GetSpawnLocationPlacementID);
	lua_register(state, "GetSpawnListBySpawnID", EQ2Emu_lua_GetSpawnListBySpawnID);
	lua_register(state, "GetSpawnListByRailID", EQ2Emu_lua_GetSpawnListByRailID);
	lua_register(state, "GetPassengerSpawnList", EQ2Emu_lua_GetPassengerSpawnList);
	lua_register(state, "GetSpawnFromList", EQ2Emu_lua_GetSpawnFromList);
	lua_register(state, "GetSpawnListSize", EQ2Emu_lua_GetSpawnListSize);	
	lua_register(state, "SetFactionID", EQ2Emu_lua_SetFactionID);
	lua_register(state, "GetFactionID", EQ2Emu_lua_GetFactionID);
	lua_register(state, "GetFactionAmount", EQ2Emu_lua_GetFactionAmount);
	lua_register(state, "ChangeFaction", EQ2Emu_lua_ChangeFaction);
	lua_register(state, "GetGender", EQ2Emu_lua_GetGender);
	lua_register(state, "GetTarget", EQ2Emu_lua_GetTarget);
	lua_register(state, "HasFreeSlot", EQ2Emu_lua_HasFreeSlot);
	lua_register(state, "HasItemEquipped", EQ2Emu_lua_HasItemEquipped);
	lua_register(state, "GetEquippedItemByID", EQ2Emu_lua_GetEquippedItemByID);
	lua_register(state, "SetEquippedItemByID", EQ2Emu_lua_SetEquippedItemByID);
	lua_register(state, "SetEquippedItem", EQ2Emu_lua_SetEquippedItem);
	lua_register(state, "UnequipSlot", EQ2Emu_lua_UnequipSlot);
	lua_register(state, "SetEquipment", EQ2Emu_lua_SetEquipment);
	lua_register(state, "GetEquippedItemBySlot", EQ2Emu_lua_GetEquippedItemBySlot);
	lua_register(state, "GetItemByID", EQ2Emu_lua_GetItemByID);
	lua_register(state, "GetItemType", EQ2Emu_lua_GetItemType);
	lua_register(state, "GetItemEffectType", EQ2Emu_lua_GetItemEffectType);
	lua_register(state, "GetSpellName", EQ2Emu_lua_GetSpellName);
	lua_register(state, "PerformCameraShake", EQ2Emu_lua_PerformCameraShake);
	lua_register(state, "GetModelType", EQ2Emu_lua_GetModelType);
	lua_register(state, "GetSpeed", EQ2Emu_lua_GetSpeed);
	lua_register(state, "HasMoved", EQ2Emu_lua_HasMoved);
	lua_register(state, "SpellDamage", EQ2Emu_lua_SpellDamage);
	lua_register(state, "SpellDamageExt", EQ2Emu_lua_SpellDamageExt);
	lua_register(state, "CastSpell", EQ2Emu_lua_CastSpell);	
	lua_register(state, "SpellHeal", EQ2Emu_lua_SpellHeal);
	lua_register(state, "SpellHealPct", EQ2Emu_lua_SpellHealPct);
	lua_register(state, "AddItem", EQ2Emu_lua_AddItem);
	lua_register(state, "SummonItem", EQ2Emu_lua_SummonItem);
	lua_register(state, "RemoveItem", EQ2Emu_lua_RemoveItem);
	lua_register(state, "HasItem", EQ2Emu_lua_HasItem);
	lua_register(state, "SpawnMob", EQ2Emu_lua_Spawn);
	lua_register(state, "SummonPet", EQ2Emu_lua_SummonPet);
	lua_register(state, "AddSpawnAccess", EQ2Emu_lua_AddSpawnAccess);
	lua_register(state, "GetZone", EQ2Emu_lua_GetZone);
	lua_register(state, "GetZoneName", EQ2Emu_lua_GetZoneName);
	lua_register(state, "GetZoneID", EQ2Emu_lua_GetZoneID);
	lua_register(state, "Zone", EQ2Emu_lua_Zone);
	lua_register(state, "AddHate", EQ2Emu_lua_AddHate);
	lua_register(state, "IsAlive", EQ2Emu_lua_IsAlive);
	lua_register(state, "IsSpawnGroupAlive", EQ2Emu_lua_IsSpawnGroupAlive);
	lua_register(state, "IsInCombat", EQ2Emu_lua_IsInCombat);
	lua_register(state, "Attack", EQ2Emu_lua_Attack);
	lua_register(state, "ApplySpellVisual", EQ2Emu_lua_ApplySpellVisual);
	
	
	lua_register(state, "IsPlayer", EQ2Emu_lua_IsPlayer);
	lua_register(state, "GetCharacterID", EQ2Emu_lua_GetCharacterID);
	lua_register(state, "FaceTarget", EQ2Emu_lua_FaceTarget);
	lua_register(state, "MoveToLocation", EQ2Emu_lua_MoveToLocation);
	lua_register(state, "ClearRunningLocations", EQ2Emu_lua_ClearRunningLocations);
	lua_register(state, "Shout", EQ2Emu_lua_Shout);
	lua_register(state, "Say", EQ2Emu_lua_Say);
	lua_register(state, "SayOOC", EQ2Emu_lua_SayOOC);
	lua_register(state, "Emote", EQ2Emu_lua_Emote);
	lua_register(state, "MovementLoopAddLocation", EQ2Emu_lua_MovementLoopAdd); // do not remove this function, it is already heavily used by the content team
	lua_register(state, "MovementLoopAdd", EQ2Emu_lua_MovementLoopAdd);
//	lua_register(state, "GetCurrentZoneSafeLocation", EQ2Emu_lua_GetCurrentZoneSafeLocation); // This is already added below.
	lua_register(state, "AddTimer", EQ2Emu_lua_AddTimer);
	lua_register(state, "StopTimer", EQ2Emu_lua_StopTimer);
	lua_register(state, "Harvest", EQ2Emu_lua_Harvest);
	lua_register(state, "SetAttackable", EQ2Emu_lua_SetAttackable);

	
	lua_register(state, "AddSpellBonus", EQ2Emu_lua_AddSpellBonus);
	lua_register(state, "RemoveSpellBonus", EQ2Emu_lua_RemoveSpellBonus);
	lua_register(state, "AddSkillBonus", EQ2Emu_lua_AddSkillBonus);
	lua_register(state, "RemoveSkillBonus", EQ2Emu_lua_RemoveSkillBonus);
	lua_register(state, "AddControlEffect", EQ2Emu_lua_AddControlEffect);
	lua_register(state, "RemoveControlEffect", EQ2Emu_lua_RemoveControlEffect);
	lua_register(state, "HasControlEffect", EQ2Emu_lua_HasControlEffect);

	lua_register(state, "GetBaseAggroRadius", EQ2Emu_lua_GetBaseAggroRadius);
	lua_register(state, "GetAggroRadius", EQ2Emu_lua_GetAggroRadius);
	lua_register(state, "SetAggroRadius", EQ2Emu_lua_SetAggroRadius);

	lua_register(state, "GetCurrentZoneSafeLocation", EQ2Emu_lua_GetCurrentZoneSafeLocation);


	lua_register(state, "SetDeity", EQ2Emu_lua_SetDeity);
	lua_register(state, "GetDeity", EQ2Emu_lua_GetDeity);


	lua_register(state, "GetInt", EQ2Emu_lua_GetInt);
	lua_register(state, "GetWis", EQ2Emu_lua_GetWis);
	lua_register(state, "GetSta", EQ2Emu_lua_GetSta);
	lua_register(state, "GetStr", EQ2Emu_lua_GetStr);
	lua_register(state, "GetAgi", EQ2Emu_lua_GetAgi);
	lua_register(state, "SetInt", EQ2Emu_lua_SetInt);
	lua_register(state, "SetWis", EQ2Emu_lua_SetWis);
	lua_register(state, "SetSta", EQ2Emu_lua_SetSta);
	lua_register(state, "SetStr", EQ2Emu_lua_SetStr);
	lua_register(state, "SetAgi", EQ2Emu_lua_SetAgi);
	lua_register(state, "GetIntBase", EQ2Emu_lua_GetIntBase);
	lua_register(state, "GetWisBase", EQ2Emu_lua_GetWisBase);
	lua_register(state, "GetStaBase", EQ2Emu_lua_GetStaBase);
	lua_register(state, "GetStrBase", EQ2Emu_lua_GetStrBase);
	lua_register(state, "GetAgiBase", EQ2Emu_lua_GetAgiBase);
	lua_register(state, "SetIntBase", EQ2Emu_lua_SetIntBase);
	lua_register(state, "SetWisBase", EQ2Emu_lua_SetWisBase);
	lua_register(state, "SetStaBase", EQ2Emu_lua_SetStaBase);
	lua_register(state, "SetStrBase", EQ2Emu_lua_SetStrBase);
	lua_register(state, "SetAgiBase", EQ2Emu_lua_SetAgiBase);
	lua_register(state, "GetSpawn", EQ2Emu_lua_GetSpawn);
	lua_register(state, "GetVariableValue", EQ2Emu_lua_GetVariableValue);
	lua_register(state, "GetCoinMessage", EQ2Emu_lua_GetCoinMessage);
	lua_register(state, "GetSpawnByGroupID", EQ2Emu_lua_GetSpawnByGroupID);
	lua_register(state, "GetSpawnByLocationID", EQ2Emu_lua_GetSpawnByLocationID);
	lua_register(state, "PlayFlavor", EQ2Emu_lua_PlayFlavor);
	lua_register(state, "PlayFlavorID", EQ2Emu_lua_PlayFlavorID);
	lua_register(state, "PlaySound", EQ2Emu_lua_PlaySound);
	lua_register(state, "PlayVoice", EQ2Emu_lua_PlayVoice);
	lua_register(state, "PlayAnimation", EQ2Emu_lua_PlayAnimation);
	lua_register(state, "AddLootItem", EQ2Emu_lua_AddLootItem);
	lua_register(state, "HasLootItem", EQ2Emu_lua_HasLootItem);
	lua_register(state, "RemoveLootItem", EQ2Emu_lua_RemoveLootItem);
	lua_register(state, "AddLootCoin", EQ2Emu_lua_AddLootCoin);
	lua_register(state, "GiveLoot", EQ2Emu_lua_GiveLoot);
	lua_register(state, "HasPendingLootItem", EQ2Emu_lua_HasPendingLootItem);
	lua_register(state, "HasPendingLoot", EQ2Emu_lua_HasPendingLoot);
	lua_register(state, "SetLootCoin", EQ2Emu_lua_SetLootCoin);
	lua_register(state, "HasCoin", EQ2Emu_lua_HasCoin);
	lua_register(state, "GetLootCoin", EQ2Emu_lua_GetLootCoin);
	lua_register(state, "SetPlayerProximityFunction", EQ2Emu_lua_SetPlayerProximityFunction);
	lua_register(state, "SetLocationProximityFunction", EQ2Emu_lua_SetLocationProximityFunction);
	lua_register(state, "CreateConversation", EQ2Emu_lua_CreateConversation);
	lua_register(state, "AddConversationOption", EQ2Emu_lua_AddConversationOption);
	lua_register(state, "StartConversation", EQ2Emu_lua_StartConversation);
	lua_register(state, "CloseConversation", EQ2Emu_lua_CloseConversation);
	lua_register(state, "CloseItemConversation", EQ2Emu_lua_CloseItemConversation);
	//lua_register(state, "StartItemConversation", EQ2Emu_lua_StartItemConversation);
	lua_register(state, "StartDialogConversation", EQ2Emu_lua_StartDialogConversation);	
	lua_register(state, "SendStateCommand", EQ2Emu_lua_SendStateCommand);
	lua_register(state, "SpawnSet", EQ2Emu_lua_SpawnSet);
	lua_register(state, "SpawnSetByDistance", EQ2Emu_lua_SpawnSetByDistance);
	lua_register(state, "SpawnMove", EQ2Emu_lua_SpawnMove);
	lua_register(state, "KillSpawn", EQ2Emu_lua_KillSpawn); 
	lua_register(state, "KillSpawnByDistance", EQ2Emu_lua_KillSpawnByDistance);
	lua_register(state, "Despawn", EQ2Emu_lua_Despawn);
	lua_register(state, "ChangeHandIcon", EQ2Emu_lua_ChangeHandIcon);
	lua_register(state, "SetVisualFlag", EQ2Emu_lua_SetVisualFlag);
	lua_register(state, "SetInfoFlag", EQ2Emu_lua_SetInfoFlag);
	lua_register(state, "IsBindAllowed", EQ2Emu_lua_IsBindAllowed);
	lua_register(state, "IsGateAllowed", EQ2Emu_lua_IsGateAllowed);
	lua_register(state, "Bind", EQ2Emu_lua_Bind);
	lua_register(state, "Gate", EQ2Emu_lua_Gate);
	lua_register(state, "SendMessage", EQ2Emu_lua_SendMessage);
	lua_register(state, "SendPopUpMessage", EQ2Emu_lua_SendPopUpMessage);
	lua_register(state, "SetServerControlFlag", EQ2Emu_lua_SetServerControlFlag);
	lua_register(state, "ToggleTracking", EQ2Emu_lua_ToggleTracking);
	lua_register(state, "AddPrimaryEntityCommand", EQ2Emu_lua_AddPrimaryEntityCommand);
	lua_register(state, "AddSpellBookEntry", EQ2Emu_lua_AddSpellBookEntry);
	lua_register(state, "DeleteSpellBook", EQ2Emu_lua_DeleteSpellBook);
	lua_register(state, "RemoveSpellBookEntry", EQ2Emu_lua_RemoveSpellBookEntry);
	lua_register(state, "SendNewAdventureSpells", EQ2Emu_lua_SendNewAdventureSpells);
	lua_register(state, "SendNewTradeskillSpells", EQ2Emu_lua_SendNewTradeskillSpells);
	lua_register(state, "HasSpell", EQ2Emu_lua_HasSpell);
	lua_register(state, "Interrupt", EQ2Emu_lua_Interrupt);
	lua_register(state, "Stealth", EQ2Emu_lua_Stealth);
	lua_register(state, "IsInvis", EQ2Emu_lua_IsInvis);
	lua_register(state, "IsStealthed", EQ2Emu_lua_IsStealthed);
	lua_register(state, "AddSpawnIDAccess", EQ2Emu_lua_AddSpawnIDAccess);
	lua_register(state, "RemoveSpawnIDAccess", EQ2Emu_lua_RemoveSpawnIDAccess);
	lua_register(state, "HasRecipeBook", EQ2Emu_lua_HasRecipeBook);

	lua_register(state, "SetRequiredQuest", EQ2Emu_lua_SetRequiredQuest);
	lua_register(state, "SetRequiredHistory", EQ2Emu_lua_SetRequiredHistory);
	lua_register(state, "SetStepComplete", EQ2Emu_lua_SetStepComplete);
	lua_register(state, "AddStepProgress", EQ2Emu_lua_AddStepProgress);
	lua_register(state, "UpdateQuestTaskGroupDescription", EQ2Emu_lua_UpdateQuestTaskGroupDescription);
	lua_register(state, "GetTaskGroupStep", EQ2Emu_lua_GetTaskGroupStep);
	lua_register(state, "GetQuestStep", EQ2Emu_lua_GetQuestStep);
	lua_register(state, "QuestStepIsComplete", EQ2Emu_lua_QuestStepIsComplete);
	lua_register(state, "RegisterQuest", EQ2Emu_lua_RegisterQuest);
	lua_register(state, "SetQuestPrereqLevel", EQ2Emu_lua_SetQuestPrereqLevel);
	lua_register(state, "AddQuestPrereqQuest", EQ2Emu_lua_AddQuestPrereqQuest);
	lua_register(state, "AddQuestPrereqItem", EQ2Emu_lua_AddQuestPrereqItem);
	lua_register(state, "AddQuestPrereqFaction", EQ2Emu_lua_AddQuestPrereqFaction);
	lua_register(state, "AddQuestPrereqRace", EQ2Emu_lua_AddQuestPrereqRace);
	lua_register(state, "AddQuestPrereqModelType", EQ2Emu_lua_AddQuestPrereqModelType);
	lua_register(state, "AddQuestPrereqClass", EQ2Emu_lua_AddQuestPrereqClass);
	lua_register(state, "AddQuestPrereqTradeskillLevel", EQ2Emu_lua_AddQuestPrereqTradeskillLevel);
	lua_register(state, "AddQuestPrereqTradeskillClass", EQ2Emu_lua_AddQuestPrereqTradeskillClass);
	lua_register(state, "AddQuestSelectableRewardItem", EQ2Emu_lua_AddQuestSelectableRewardItem);
	lua_register(state, "HasQuestRewardItem", EQ2Emu_lua_HasQuestRewardItem);
	lua_register(state, "AddQuestRewardItem", EQ2Emu_lua_AddQuestRewardItem);
	lua_register(state, "AddQuestRewardCoin", EQ2Emu_lua_AddQuestRewardCoin);
	lua_register(state, "AddQuestRewardFaction", EQ2Emu_lua_AddQuestRewardFaction);
	lua_register(state, "SetQuestRewardStatus", EQ2Emu_lua_SetQuestRewardStatus);
	lua_register(state, "SetStatusTmpReward", EQ2Emu_lua_SetStatusTmpReward);
	lua_register(state, "SetCoinTmpReward", EQ2Emu_lua_SetCoinTmpReward);
	lua_register(state, "SetQuestRewardComment", EQ2Emu_lua_SetQuestRewardComment);
	lua_register(state, "SetQuestRewardExp", EQ2Emu_lua_SetQuestRewardExp);
	lua_register(state, "AddQuestStepKill", EQ2Emu_lua_AddQuestStepKill);
	lua_register(state, "AddQuestStepKillByRace", EQ2Emu_lua_AddQuestStepKillByRace);
	lua_register(state, "AddQuestStep", EQ2Emu_lua_AddQuestStep);
	lua_register(state, "AddQuestStepChat", EQ2Emu_lua_AddQuestStepChat);
	lua_register(state, "AddQuestStepObtainItem", EQ2Emu_lua_AddQuestStepObtainItem);
	lua_register(state, "AddQuestStepZoneLoc", EQ2Emu_lua_AddQuestStepZoneLoc);
	lua_register(state, "AddQuestStepLocation", EQ2Emu_lua_AddQuestStepLocation);
	lua_register(state, "AddQuestStepSpell", EQ2Emu_lua_AddQuestStepSpell);
	lua_register(state, "AddQuestStepCraft", EQ2Emu_lua_AddQuestStepCraft);
	lua_register(state, "AddQuestStepHarvest", EQ2Emu_lua_AddQuestStepHarvest);
	lua_register(state, "AddQuestStepCompleteAction", EQ2Emu_lua_AddQuestStepCompleteAction);
	lua_register(state, "AddQuestStepProgressAction", EQ2Emu_lua_AddQuestStepProgressAction);
	lua_register(state, "SetQuestCompleteAction", EQ2Emu_lua_SetQuestCompleteAction);
	lua_register(state, "GiveQuestReward", EQ2Emu_lua_GiveQuestReward);
	lua_register(state, "UpdateQuestStepDescription", EQ2Emu_lua_UpdateQuestStepDescription);
	lua_register(state, "UpdateQuestDescription", EQ2Emu_lua_UpdateQuestDescription);
	lua_register(state, "UpdateQuestZone", EQ2Emu_lua_UpdateQuestZone);
	lua_register(state, "SetCompletedDescription", EQ2Emu_lua_SetCompletedDescription);
	lua_register(state, "OfferQuest", EQ2Emu_lua_OfferQuest);
	lua_register(state, "ProvidesQuest", EQ2Emu_lua_ProvidesQuest);	
	lua_register(state, "HasQuest", EQ2Emu_lua_HasQuest);	
	lua_register(state, "HasCompletedQuest", EQ2Emu_lua_HasCompletedQuest);	
	lua_register(state, "QuestIsComplete", EQ2Emu_lua_QuestIsComplete);	
	lua_register(state, "QuestReturnNPC", EQ2Emu_lua_QuestReturnNPC);		
	lua_register(state, "GetQuest", EQ2Emu_lua_GetQuest);
	lua_register(state, "HasCollectionsToHandIn", EQ2Emu_lua_HasCollectionsToHandIn);
	lua_register(state, "HandInCollections", EQ2Emu_lua_HandInCollections);
	lua_register(state, "UseWidget", EQ2Emu_lua_UseWidget);
	lua_register(state, "SetSpellList", EQ2Emu_lua_SetSpellList);
	lua_register(state, "GetPet", EQ2Emu_lua_GetPet);
	lua_register(state, "Charm", EQ2Emu_lua_Charm);
	lua_register(state, "GetGroup", EQ2Emu_lua_GetGroup);
	lua_register(state, "SetCompleteFlag", EQ2Emu_lua_SetCompleteFlag);
	lua_register(state, "SetQuestYellow", EQ2Emu_lua_SetQuestYellow);
	lua_register(state, "CanReceiveQuest", EQ2Emu_lua_CanReceiveQuest);
	lua_register(state, "AddTransportSpawn", EQ2Emu_lua_AddTransportSpawn);
	lua_register(state, "IsTransportSpawn", EQ2Emu_lua_IsTransportSpawn);

	// Option window
	lua_register(state, "CreateOptionWindow", EQ2Emu_lua_CreateOptionWindow);
	lua_register(state, "AddOptionWindowOption", EQ2Emu_lua_AddOptionWindowOption);
	lua_register(state, "SendOptionWindow", EQ2Emu_lua_SendOptionWindow);

	lua_register(state, "GetTradeskillClass", EQ2Emu_lua_GetTradeskillClass);
	lua_register(state, "GetTradeskillLevel", EQ2Emu_lua_GetTradeskillLevel);
	lua_register(state, "GetTradeskillClassName", EQ2Emu_lua_GetTradeskillClassName);
	lua_register(state, "SetTradeskillLevel", EQ2Emu_lua_SetTradeskillLevel);

	lua_register(state, "SummonDeityPet", EQ2Emu_lua_SummonDeityPet);
	lua_register(state, "SummonCosmeticPet", EQ2Emu_lua_SummonCosmeticPet);
	lua_register(state, "DismissPet", EQ2Emu_lua_DismissPet);

	lua_register(state, "GetCharmedPet", EQ2Emu_lua_GetCharmedPet);
	lua_register(state, "GetDeityPet", EQ2Emu_lua_GetDeityPet);
	lua_register(state, "GetCosmeticPet", EQ2Emu_lua_GetCosmeticPet);

	lua_register(state, "SetQuestFeatherColor", EQ2Emu_lua_SetQuestFeatherColor);
	lua_register(state, "RemoveSpawnAccess", EQ2Emu_lua_RemoveSpawnAccess);
	lua_register(state, "SpawnByLocationID", EQ2Emu_lua_SpawnByLocationID);
	lua_register(state, "CastEntityCommand", EQ2Emu_lua_CastEntityCommand);
	lua_register(state, "SetLuaBrain", EQ2Emu_lua_SetLuaBrain);
	lua_register(state, "SetBrainTick", EQ2Emu_lua_SetBrainTick);
	lua_register(state, "SetFollowTarget", EQ2Emu_lua_SetFollowTarget);
	lua_register(state, "GetFollowTarget", EQ2Emu_lua_GetFollowTarget);
	lua_register(state, "ToggleFollow", EQ2Emu_lua_ToggleFollow);
	lua_register(state, "IsFollowing", EQ2Emu_lua_IsFollowing);
	lua_register(state, "SetTempVariable", EQ2Emu_lua_SetTempVariable);
	lua_register(state, "GetTempVariable", EQ2Emu_lua_GetTempVariable);
	lua_register(state, "GiveQuestItem", EQ2Emu_lua_GiveQuestItem);
	lua_register(state, "SetQuestRepeatable", EQ2Emu_lua_SetQuestRepeatable);

	lua_register(state, "AddWaypoint", EQ2Emu_lua_AddWaypoint);
	lua_register(state, "RemoveWaypoint", EQ2Emu_lua_RemoveWaypoint);
	lua_register(state, "SendWaypoints", EQ2Emu_lua_SendWaypoints);

	lua_register(state, "AddWard", EQ2Emu_lua_AddWard);
	lua_register(state, "AddToWard", EQ2Emu_lua_AddToWard);
	lua_register(state, "RemoveWard", EQ2Emu_lua_RemoveWard);
	lua_register(state, "GetWardAmountLeft", EQ2Emu_lua_GetWardAmountLeft);
	lua_register(state, "GetWardValue", EQ2Emu_lua_GetWardValue);

	lua_register(state, "SetTarget", EQ2Emu_lua_SetTarget);
	lua_register(state, "IsPet", EQ2Emu_lua_IsPet);
	lua_register(state, "GetOwner", EQ2Emu_lua_GetOwner);
	lua_register(state, "SetInCombat", EQ2Emu_lua_SetInCombat);
	lua_register(state, "CompareSpawns", EQ2Emu_lua_CompareSpawns);
	lua_register(state, "ClearRunback", EQ2Emu_lua_ClearRunback);
	lua_register(state, "Runback", EQ2Emu_lua_Runback);
	lua_register(state, "GetRunbackDistance", EQ2Emu_lua_GetRunbackDistance);
	lua_register(state, "IsCasting", EQ2Emu_lua_IsCasting);
	lua_register(state, "IsMezzed", EQ2Emu_lua_IsMezzed);
	lua_register(state, "IsStunned", EQ2Emu_lua_IsStunned);
	lua_register(state, "IsMezzedOrStunned", EQ2Emu_lua_IsMezzedOrStunned);
	lua_register(state, "ProcessSpell", EQ2Emu_lua_ProcessSpell);
	lua_register(state, "ProcessMelee", EQ2Emu_lua_ProcessMelee);
	lua_register(state, "HasRecovered", EQ2Emu_lua_HasRecovered);
	lua_register(state, "GetEncounterSize", EQ2Emu_lua_GetEncounterSize);
	lua_register(state, "GetMostHated", EQ2Emu_lua_GetMostHated);
	lua_register(state, "ClearHate", EQ2Emu_lua_ClearHate);
	lua_register(state, "ClearEncounter", EQ2Emu_lua_ClearEncounter);
	lua_register(state, "GetEncounter", EQ2Emu_lua_GetEncounter);
	lua_register(state, "GetHateList", EQ2Emu_lua_GetHateList);
	lua_register(state, "HasGroup", EQ2Emu_lua_HasGroup);
	lua_register(state, "HasSpellEffect", EQ2Emu_lua_HasSpellEffect);

	lua_register(state, "SetSuccessTimer", EQ2Emu_lua_SetSuccessTimer);
	lua_register(state, "SetFailureTimer", EQ2Emu_lua_SetFailureTimer);
	lua_register(state, "IsGroundSpawn", EQ2Emu_lua_IsGroundSpawn);
	lua_register(state, "CanHarvest", EQ2Emu_lua_CanHarvest);
	lua_register(state, "SummonDumbFirePet", EQ2Emu_lua_SummonDumbFirePet);

	lua_register(state, "GetSkillValue", EQ2Emu_lua_GetSkillValue);
	lua_register(state, "GetSkillMaxValue", EQ2Emu_lua_GetSkillMaxValue);
	lua_register(state, "GetSkillName", EQ2Emu_lua_GetSkillName);
	lua_register(state, "SetSkillMaxValue", EQ2Emu_lua_SetSkillMaxValue);
	lua_register(state, "SetSkillValue", EQ2Emu_lua_SetSkillValue);
	lua_register(state, "GetSkill", EQ2Emu_lua_GetSkill);
	lua_register(state, "GetSkillIDByName", EQ2Emu_lua_GetSkillIDByName);
	lua_register(state, "HasSkill", EQ2Emu_lua_HasSkill);
	lua_register(state, "AddSkill", EQ2Emu_lua_AddSkill);
	lua_register(state, "IncreaseSkillCapsByType", EQ2Emu_lua_IncreaseSkillCapsByType);
	lua_register(state, "RemoveSkill", EQ2Emu_lua_RemoveSkill);
	lua_register(state, "AddProc", EQ2Emu_lua_AddProc);
	lua_register(state, "AddProcExt", EQ2Emu_lua_AddProcExt);
	lua_register(state, "RemoveProc", EQ2Emu_lua_RemoveProc);
	lua_register(state, "Knockback", EQ2Emu_lua_Knockback);

	lua_register(state, "IsEpic", EQ2Emu_lua_IsEpic);
	lua_register(state, "IsHeroic", EQ2Emu_lua_IsHeroic);
	lua_register(state, "ProcDamage", EQ2Emu_lua_ProcDamage);
	lua_register(state, "LastSpellAttackHit", EQ2Emu_lua_LastSpellAttackHit);
	lua_register(state, "IsBehind", EQ2Emu_lua_IsBehind);
	lua_register(state, "IsFlanking", EQ2Emu_lua_IsFlanking);
	lua_register(state, "InFront", EQ2Emu_lua_InFront);
	lua_register(state, "AddSpellTimer", EQ2Emu_lua_AddSpellTimer);
	lua_register(state, "GetItemCount", EQ2Emu_lua_GetItemCount);
	lua_register(state, "SetItemCount", EQ2Emu_lua_SetItemCount);
	lua_register(state, "Resurrect", EQ2Emu_lua_Resurrect);
	lua_register(state, "BreatheUnderwater", EQ2Emu_lua_BreatheUnderwater);
	lua_register(state, "BlurVision", EQ2Emu_lua_BlurVision);
	lua_register(state, "SetVision", EQ2Emu_lua_SetVision);
	lua_register(state, "GetItemSkillReq", EQ2Emu_lua_GetItemSkillReq);
	lua_register(state, "SetSpeedMultiplier", EQ2Emu_lua_SetSpeedMultiplier);
	lua_register(state, "SetIllusion", EQ2Emu_lua_SetIllusion);
	lua_register(state, "ResetIllusion", EQ2Emu_lua_ResetIllusion);
	lua_register(state, "AddThreatTransfer", EQ2Emu_lua_AddThreatTransfer);
	lua_register(state, "RemoveThreatTransfer", EQ2Emu_lua_RemoveThreatTransfer);
	lua_register(state, "CureByType", EQ2Emu_lua_CureByType);
	lua_register(state, "CureByControlEffect", EQ2Emu_lua_CureByControlEffect);
	lua_register(state, "AddSpawnSpellBonus", EQ2Emu_lua_AddSpawnSpellBonus);
	lua_register(state, "RemoveSpawnSpellBonus", EQ2Emu_lua_RemoveSpawnSpellBonus);
	lua_register(state, "CancelSpell", EQ2Emu_lua_CancelSpell);
	lua_register(state, "RemoveStealth", EQ2Emu_lua_RemoveStealth);
	lua_register(state, "RemoveInvis", EQ2Emu_lua_RemoveInvis);
	lua_register(state, "StartHeroicOpportunity", EQ2Emu_lua_StartHeroicOpportunity);
	lua_register(state, "CopySpawnAppearance", EQ2Emu_lua_CopySpawnAppearance);
	lua_register(state, "SetSpellTriggerCount", EQ2Emu_lua_SetSpellTriggerCount);
	lua_register(state, "GetSpellTriggerCount", EQ2Emu_lua_GetSpellTriggerCount);
	lua_register(state, "RemoveTriggerFromSpell", EQ2Emu_lua_RemoveTriggerFromSpell);
	lua_register(state, "HasSpellImmunity", EQ2Emu_lua_HasSpellImmunity);
	lua_register(state, "AddImmunitySpell", EQ2Emu_lua_AddImmunitySpell);
	lua_register(state, "RemoveImmunitySpell", EQ2Emu_lua_RemoveImmunitySpell);
	lua_register(state, "SetSpellSnareValue", EQ2Emu_lua_SetSpellSnareValue);
	lua_register(state, "CheckRaceType", EQ2Emu_lua_CheckRaceType);
	lua_register(state, "GetRaceType", EQ2Emu_lua_GetRaceType);
	lua_register(state, "GetRaceBaseType", EQ2Emu_lua_GetRaceBaseType);
	lua_register(state, "GetQuestFlags", EQ2Emu_lua_GetQuestFlags);
	lua_register(state, "SetQuestFlags", EQ2Emu_lua_SetQuestFlags);
	lua_register(state, "SetQuestTimer", EQ2Emu_lua_SetQuestTimer);
	lua_register(state, "RemoveQuestStep", EQ2Emu_lua_RemoveQuestStep);
	lua_register(state, "ResetQuestStep", EQ2Emu_lua_ResetQuestStep);
	lua_register(state, "SetQuestTimerComplete", EQ2Emu_lua_SetQuestTimerComplete);
	lua_register(state, "AddQuestStepFailureAction", EQ2Emu_lua_AddQuestStepFailureAction);
	lua_register(state, "SetStepFailed", EQ2Emu_lua_SetStepFailed);
	lua_register(state, "GetQuestCompleteCount", EQ2Emu_lua_GetQuestCompleteCount);
	lua_register(state, "SetServerVariable", EQ2Emu_lua_SetServerVariable);
	lua_register(state, "GetServerVariable", EQ2Emu_lua_GetServerVariable);
	lua_register(state, "HasLanguage", EQ2Emu_lua_HasLanguage);
	lua_register(state, "AddLanguage", EQ2Emu_lua_AddLanguage);
	lua_register(state, "IsNight", EQ2Emu_lua_IsNight);
	lua_register(state, "AddMultiFloorLift", EQ2Emu_lua_AddMultiFloorLift);
	lua_register(state, "StartAutoMount", EQ2Emu_lua_StartAutoMount);
	lua_register(state, "EndAutoMount", EQ2Emu_lua_EndAutoMount);
	lua_register(state, "IsOnAutoMount", EQ2Emu_lua_IsOnAutoMount);
	lua_register(state, "SetPlayerHistory", EQ2Emu_lua_SetPlayerHistory);
	lua_register(state, "GetPlayerHistory", EQ2Emu_lua_GetPlayerHistory);
	lua_register(state, "SetGridID", EQ2Emu_lua_SetGridID);
	lua_register(state, "GetQuestStepProgress", EQ2Emu_lua_GetQuestStepProgress);
	lua_register(state, "SetPlayerLevel", EQ2Emu_lua_SetPlayerLevel);
	lua_register(state, "AddCoin", EQ2Emu_lua_AddCoin);
	lua_register(state, "RemoveCoin", EQ2Emu_lua_RemoveCoin);
	lua_register(state, "GetPlayersInZone", EQ2Emu_lua_GetPlayersInZone);
	lua_register(state, "SpawnGroupByID", EQ2Emu_lua_SpawnGroupByID);
	lua_register(state, "SetSpawnAnimation", EQ2Emu_lua_SetSpawnAnimation);
	lua_register(state, "GetClientVersion", EQ2Emu_lua_GetClientVersion);
	lua_register(state, "GetItemID", EQ2Emu_lua_GetItemID);
	lua_register(state, "IsEntity", EQ2Emu_lua_IsEntity);
	lua_register(state, "GetOrigX", EQ2Emu_lua_GetOrigX);
	lua_register(state, "GetOrigY", EQ2Emu_lua_GetOrigY);
	lua_register(state, "GetOrigZ", EQ2Emu_lua_GetOrigZ);
	lua_register(state, "GetPCTOfHP", EQ2Emu_lua_GetPCTOfHP);
	lua_register(state, "GetPCTOfPower", EQ2Emu_lua_GetPCTOfPower);
	lua_register(state, "GetBoundZoneID", EQ2Emu_lua_GetBoundZoneID);
	lua_register(state, "Evac", EQ2Emu_lua_Evac);
	lua_register(state, "GetSpellTier", EQ2Emu_lua_GetSpellTier);
	lua_register(state, "GetSpellID", EQ2Emu_lua_GetSpellID);
	lua_register(state, "StartTransmute", EQ2Emu_lua_StartTransmute);
	lua_register(state, "CompleteTransmute", EQ2Emu_lua_CompleteTransmute);
	lua_register(state, "ProcHate", EQ2Emu_lua_ProcHate);

	lua_register(state, "GetRandomSpawnByID", EQ2Emu_lua_GetRandomSpawnByID);
	lua_register(state, "ShowLootWindow", EQ2Emu_lua_ShowLootWindow);
	lua_register(state, "AddPrimaryEntityCommandAllSpawns", EQ2Emu_lua_AddPrimaryEntityCommandAllSpawns);
	lua_register(state, "InstructionWindow", EQ2Emu_lua_InstructionWindow);
	lua_register(state, "InstructionWindowClose", EQ2Emu_lua_InstructionWindowClose);
	lua_register(state, "InstructionWindowGoal", EQ2Emu_lua_InstructionWindowGoal);
	lua_register(state, "ShowWindow", EQ2Emu_lua_ShowWindow);
	lua_register(state, "FlashWindow", EQ2Emu_lua_FlashWindow);
	lua_register(state, "EnableGameEvent", EQ2Emu_lua_EnableGameEvent);
	lua_register(state, "GetTutorialStep", EQ2Emu_lua_GetTutorialStep);
	lua_register(state, "SetTutorialStep", EQ2Emu_lua_SetTutorialStep);
	lua_register(state, "DisplayText", EQ2Emu_lua_DisplayText);
	lua_register(state, "GiveExp", EQ2Emu_lua_GiveExp);

	lua_register(state, "CheckLOS", EQ2Emu_lua_CheckLOS);
	lua_register(state, "CheckLOSByCoordinates", EQ2Emu_lua_CheckLOSByCoordinates);

	lua_register(state, "SetZoneExpansionFlag", EQ2Emu_lua_SetZoneExpansionFlag);
	lua_register(state, "GetZoneExpansionFlag", EQ2Emu_lua_GetZoneExpansionFlag);
	lua_register(state, "SetZoneHolidayFlag", EQ2Emu_lua_SetZoneHolidayFlag);
	lua_register(state, "GetZoneHolidayFlag", EQ2Emu_lua_GetZoneHolidayFlag);
	
	lua_register(state, "SetCanBind", EQ2Emu_lua_SetCanBind);
	lua_register(state, "GetCanBind", EQ2Emu_lua_GetCanBind);

	lua_register(state, "GetCanGate", EQ2Emu_lua_GetCanGate);
    lua_register(state, "SetCanGate", EQ2Emu_lua_SetCanGate);

	lua_register(state, "SetCanEvac", EQ2Emu_lua_SetCanEvac);
	lua_register(state, "GetCanEvac", EQ2Emu_lua_GetCanEvac);

	lua_register(state, "AddSpawnProximity", EQ2Emu_lua_AddSpawnProximity);

	lua_register(state, "CanSeeInvis", EQ2Emu_lua_CanSeeInvis);
	lua_register(state, "SetSeeInvis", EQ2Emu_lua_SetSeeInvis);
	lua_register(state, "SetSeeHide", EQ2Emu_lua_SetSeeHide);

	lua_register(state, "SetAccessToEntityCommand", EQ2Emu_lua_SetAccessToEntityCommand);
	lua_register(state, "SetAccessToEntityCommandByCharID", EQ2Emu_lua_SetAccessToEntityCommandByCharID);
	lua_register(state, "RemovePrimaryEntityCommand", EQ2Emu_lua_RemovePrimaryEntityCommand);
	lua_register(state, "SendUpdateDefaultCommand", EQ2Emu_lua_SendUpdateDefaultCommand);

	lua_register(state, "SendTransporters", EQ2Emu_lua_SendTransporters);
	lua_register(state, "SetTemporaryTransportID", EQ2Emu_lua_SetTemporaryTransportID);
	lua_register(state, "GetTemporaryTransportID", EQ2Emu_lua_GetTemporaryTransportID);

	lua_register(state, "SetAlignment", EQ2Emu_lua_SetAlignment);
	lua_register(state, "GetAlignment", EQ2Emu_lua_GetAlignment);

	lua_register(state, "GetSpell", EQ2Emu_lua_GetSpell);
	lua_register(state, "GetSpellData", EQ2Emu_lua_GetSpellData);
	lua_register(state, "SetSpellData", EQ2Emu_lua_SetSpellData);
	lua_register(state, "CastCustomSpell", EQ2Emu_lua_CastCustomSpell);

	lua_register(state, "SetSpellDataIndex", EQ2Emu_lua_SetSpellDataIndex);
	lua_register(state, "GetSpellDataIndex", EQ2Emu_lua_GetSpellDataIndex);

	lua_register(state, "SetSpellDisplayEffect", EQ2Emu_lua_SetSpellDisplayEffect);
	lua_register(state, "GetSpellDisplayEffect", EQ2Emu_lua_GetSpellDisplayEffect);

	lua_register(state, "InWater", EQ2Emu_lua_InWater);
	lua_register(state, "InLava", EQ2Emu_lua_InLava);
	
	lua_register(state, "DamageSpawn", EQ2Emu_lua_DamageSpawn);
	lua_register(state, "IsInvulnerable", EQ2Emu_lua_IsInvulnerable);
	lua_register(state, "SetInvulnerable", EQ2Emu_lua_SetInvulnerable);
	
	lua_register(state, "GetRuleFlagBool", EQ2Emu_lua_GetRuleFlagBool);
	lua_register(state, "GetRuleFlagInt32", EQ2Emu_lua_GetRuleFlagInt32);
	lua_register(state, "GetRuleFlagFloat", EQ2Emu_lua_GetRuleFlagFloat);
	
	lua_register(state, "GetAAInfo", EQ2Emu_lua_GetAAInfo);
	lua_register(state, "SetAAInfo", EQ2Emu_lua_SetAAInfo);
	
	lua_register(state, "AddMasterTitle", EQ2Emu_lua_AddMasterTitle);
	lua_register(state, "AddCharacterTitle", EQ2Emu_lua_AddCharacterTitle);
	lua_register(state, "SetCharacterTitleSuffix", EQ2Emu_lua_SetCharacterTitleSuffix);
	lua_register(state, "SetCharacterTitlePrefix", EQ2Emu_lua_SetCharacterTitlePrefix);
	lua_register(state, "ResetCharacterTitleSuffix", EQ2Emu_lua_ResetCharacterTitleSuffix);
	lua_register(state, "ResetCharacterTitlePrefix", EQ2Emu_lua_ResetCharacterTitlePrefix);
	
	lua_register(state, "GetInfoStructString", EQ2Emu_lua_GetInfoStructString);
	lua_register(state, "GetInfoStructUInt", EQ2Emu_lua_GetInfoStructUInt);
	lua_register(state, "GetInfoStructSInt", EQ2Emu_lua_GetInfoStructSInt);
	lua_register(state, "GetInfoStructFloat", EQ2Emu_lua_GetInfoStructFloat);
	
	lua_register(state, "SetInfoStructString", EQ2Emu_lua_SetInfoStructString);
	lua_register(state, "SetInfoStructUInt", EQ2Emu_lua_SetInfoStructUInt);
	lua_register(state, "SetInfoStructSInt", EQ2Emu_lua_SetInfoStructSInt);
	lua_register(state, "SetInfoStructFloat", EQ2Emu_lua_SetInfoStructFloat);
	
	lua_register(state, "SetCharSheetChanged", EQ2Emu_lua_SetCharSheetChanged);
	
	lua_register(state, "AddPlayerMail", EQ2Emu_lua_AddPlayerMail);
	lua_register(state, "AddPlayerMailByCharID", EQ2Emu_lua_AddPlayerMailByCharID);
	
	lua_register(state, "OpenDoor", EQ2Emu_lua_OpenDoor);
	lua_register(state, "CloseDoor", EQ2Emu_lua_CloseDoor);
	lua_register(state, "IsOpen", EQ2Emu_lua_IsOpen);
	
	lua_register(state, "MakeRandomInt", EQ2Emu_lua_MakeRandomInt);
	lua_register(state, "MakeRandomFloat", EQ2Emu_lua_MakeRandomFloat);
	
	lua_register(state, "AddIconValue", EQ2Emu_lua_AddIconValue);
	lua_register(state, "RemoveIconValue", EQ2Emu_lua_RemoveIconValue);
	
	lua_register(state, "GetShardID", EQ2Emu_lua_GetShardID);
	lua_register(state, "GetShardCharID", EQ2Emu_lua_GetShardCharID);
	lua_register(state, "GetShardCreatedTimestamp", EQ2Emu_lua_GetShardCreatedTimestamp);
	lua_register(state, "DeleteDBShardID", EQ2Emu_lua_DeleteDBShardID);
	
	lua_register(state, "PauseMovement", EQ2Emu_lua_PauseMovement);
	lua_register(state, "StopMovement", EQ2Emu_lua_StopMovement);
	
	lua_register(state, "GetArrowColor", EQ2Emu_lua_GetArrowColor);
	lua_register(state, "GetTSArrowColor", EQ2Emu_lua_GetTSArrowColor);
	
	lua_register(state, "GetSpawnByRailID", EQ2Emu_lua_GetSpawnByRailID);
	lua_register(state, "SetRailID", EQ2Emu_lua_SetRailID);
	lua_register(state, "IsZoneLoading", EQ2Emu_lua_IsZoneLoading);
	lua_register(state, "IsRunning", EQ2Emu_lua_IsRunning);
	
	lua_register(state, "GetZoneLockoutTimer", EQ2Emu_lua_GetZoneLockoutTimer);
	
	lua_register(state, "SetWorldTime", EQ2Emu_lua_SetWorldTime);
	lua_register(state, "GetWorldTimeYear", EQ2Emu_lua_GetWorldTimeYear);
	lua_register(state, "GetWorldTimeMonth", EQ2Emu_lua_GetWorldTimeMonth);
	lua_register(state, "GetWorldTimeHour", EQ2Emu_lua_GetWorldTimeHour);
	lua_register(state, "GetWorldTimeMinute", EQ2Emu_lua_GetWorldTimeMinute);
	lua_register(state, "SendTimeUpdate", EQ2Emu_lua_SendTimeUpdate);
	
	lua_register(state, "GetLootTier", EQ2Emu_lua_GetLootTier);
	lua_register(state, "SetLootTier", EQ2Emu_lua_SetLootTier);
	lua_register(state, "GetLootDropType", EQ2Emu_lua_GetLootDropType);
	lua_register(state, "SetLootDropType", EQ2Emu_lua_SetLootDropType);
	
	lua_register(state, "DamageEquippedItems", EQ2Emu_lua_DamageEquippedItems);
	
	lua_register(state, "CreateWidgetRegion", EQ2Emu_lua_CreateWidgetRegion);
	lua_register(state, "RemoveRegion", EQ2Emu_lua_RemoveRegion);
	
	lua_register(state, "SetPlayerPOVGhost", EQ2Emu_lua_SetPlayerPOVGhost);
	
	lua_register(state, "SetCastOnAggroComplete", EQ2Emu_lua_SetCastOnAggroComplete);
	lua_register(state, "IsCastOnAggroComplete", EQ2Emu_lua_IsCastOnAggroComplete);
	
	lua_register(state, "AddRecipeBookToPlayer", EQ2Emu_lua_AddRecipeBookToPlayer);
	lua_register(state, "RemoveRecipeFromPlayer", EQ2Emu_lua_RemoveRecipeFromPlayer);
	
	lua_register(state, "ReplaceWidgetFromClient", EQ2Emu_lua_ReplaceWidgetFromClient);
	lua_register(state, "RemoveWidgetFromSpawnMap", EQ2Emu_lua_RemoveWidgetFromSpawnMap);
	lua_register(state, "RemoveWidgetFromZoneMap", EQ2Emu_lua_RemoveWidgetFromZoneMap);
	
	lua_register(state, "SendHearCast", EQ2Emu_lua_SendHearCast);
	
	lua_register(state, "GetCharacterFlag", EQ2Emu_lua_GetCharacterFlag);
	lua_register(state, "ToggleCharacterFlag", EQ2Emu_lua_ToggleCharacterFlag);
	
	lua_register(state, "GetSpellInitialTarget", EQ2Emu_lua_GetSpellInitialTarget);
	
	lua_register(state,"DespawnByLocationID", EQ2Emu_lua_DespawnByLocationID);
}

void LuaInterface::LogError(const char* error, ...)  {
	va_list argptr;
	char buffer[4096];

	va_start(argptr, error);
	vsnprintf(buffer, sizeof(buffer), error, argptr);
	va_end(argptr);
	SimpleLogError(buffer);
}

void LuaInterface::SimpleLogError(const char* error) {
	ProcessErrorMessage(error);
	LogWrite(LUA__ERROR, 0, "LUA", "%s", error);
}

void LuaInterface::ResetFunctionStack(lua_State* state) {
	lua_settop(state, 0);
}

void LuaInterface::AddUserDataPtr(LUAUserData* data, void* data_ptr) {
	std::unique_lock lock(MLUAUserData);
	if(data_ptr) {
		user_data_ptr[data_ptr] = data;
	}
	user_data[data] = Timer::GetCurrentTime2() + 300000; //allow a function to use this pointer for 5 minutes
}

void LuaInterface::DeletePendingSpells(bool all) {
	MSpells.lock();
	MSpellDelete.lock();
	if (spells_pending_delete.size() > 0) {
		int32 time = Timer::GetCurrentTime2();
		map<LuaSpell*, int32>::iterator itr;
		vector<LuaSpell*> tmp_deletes;
		vector<LuaSpell*>::iterator del_itr;
		for (itr = spells_pending_delete.begin(); itr != spells_pending_delete.end(); itr++) {
			if (all || time >= itr->second)
				tmp_deletes.push_back(itr->first);
		}
		LuaSpell* spell = 0;
		for (del_itr = tmp_deletes.begin(); del_itr != tmp_deletes.end(); del_itr++) {
			spell = *del_itr;
			
			if(!all) {
				if (spell->caster && spell->caster->GetZone()) {
					spell->caster->GetZone()->GetSpellProcess()->DeleteActiveSpell(spell);
				}
				else if(spell->targets.size() > 0 && spell->caster && spell->caster->GetZone()) {
					spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
					for (int8 i = 0; i < spell->targets.size(); i++) {
						Spawn* target = spell->caster->GetZone()->GetSpawnByID(spell->targets.at(i));
						if (!target || !target->IsEntity())
							continue;
						target->GetZone()->GetSpellProcess()->DeleteActiveSpell(spell);
					}
					spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
				}
			}
			
			spells_pending_delete.erase(spell);

			if (spell->spell->IsCopiedSpell())
			{
				RemoveCustomSpell(spell->spell->GetSpellID());
				safe_delete(spell->spell);
			}

			SetLuaUserDataStale(spell);
			RemoveCurrentSpell(spell->state, spell, false);
			safe_delete(spell);
		}
	}
	MSpellDelete.unlock();
	MSpells.unlock();
}

void LuaInterface::DeletePendingSpell(LuaSpell* spell) {
	MSpellDelete.lock();
	if (spells_pending_delete.size() > 0) {
		map<LuaSpell*, int32>::iterator itr = spells_pending_delete.find(spell);
		if (itr != spells_pending_delete.end())
			spells_pending_delete.erase(itr);
	}
	MSpellDelete.unlock();
}

void LuaInterface::DeleteUserDataPtrs(bool all) {
	std::unique_lock lock(MLUAUserData);
	if(user_data.size() > 0){
		map<LUAUserData*, int32>::iterator itr;
		int32 time = Timer::GetCurrentTime2();
		vector<LUAUserData*> tmp_deletes;
		vector<LUAUserData*>::iterator del_itr;
		for(itr = user_data.begin(); itr != user_data.end(); itr++){
			if(all || time >= itr->second)
				tmp_deletes.push_back(itr->first);
		}
		LUAUserData* data = 0;
		for(del_itr = tmp_deletes.begin(); del_itr != tmp_deletes.end(); del_itr++){
			data = *del_itr;
			
			void* target = 0;
			if(data->IsConversationOption()) {
				target = data->conversation_options;
			}
			else if(data->IsOptionWindow()) {
				target = data->option_window_option;
			}
			else if(data->IsSpawn()) {
				target = data->spawn;
			}
			else if(data->IsQuest()) {
				target = data->quest;
			}
			else if(data->IsZone()) {
				target = data->zone;
			}
			else if(data->IsItem()) {
				target = data->item;
			}
			else if(data->IsSkill()) {
				target = data->skill;
			}
			else if(data->IsSpell()) {
				target = data->spell;
			}
			if(target) {
				std::map<void*, LUAUserData*>::iterator itr = user_data_ptr.find(target);
				if(itr != user_data_ptr.end()) {
					user_data_ptr.erase(itr);
				}
			}
			user_data.erase(data);
			safe_delete(data);
		}
	}
}

Spawn* LuaInterface::GetSpawn(lua_State* state, int8 arg_num) {
	std::shared_lock lock(MLUAUserData);
	Spawn* ret = 0;
	if (lua_islightuserdata(state, arg_num)){
		LUAUserData* data = (LUAUserData*)lua_touserdata(state, arg_num);
		if(!data || !data->IsCorrectlyInitialized()){
			LogError("%s: GetSpawn error while processing %s", GetScriptName(state), lua_tostring(state, -1));
		}
		else if(!data->IsSpawn()){
			lua_Debug ar;
			lua_getstack (state, 1, &ar);
			lua_getinfo(state, "Sln", &ar);
			LogError("%s: Invalid data type used for GetSpawn in %s (line %d)", GetScriptName(state), ar.source, ar.currentline);
		}
		else
			ret = data->spawn;
	}
	return ret;
}

vector<ConversationOption>*	LuaInterface::GetConversation(lua_State* state, int8 arg_num) {
	std::shared_lock lock(MLUAUserData);
	vector<ConversationOption>* ret = 0;
	if(lua_islightuserdata(state, arg_num)){
		LUAUserData* data = (LUAUserData*)lua_touserdata(state, arg_num);
		if(!data || !data->IsCorrectlyInitialized()){
			LogError("%s: GetConversation error while processing %s", GetScriptName(state), lua_tostring(state, -1));
		}
		else if(!data->IsConversationOption()){
			lua_Debug ar;
			lua_getstack (state, 1, &ar);
			lua_getinfo(state, "Sln", &ar);
			LogError("%s: Invalid data type used for GetConversation in %s (line %d)", GetScriptName(state), ar.source, ar.currentline);
		}
		else
			ret = data->conversation_options;
	}
	return ret;
}

vector<OptionWindowOption>*	LuaInterface::GetOptionWindow(lua_State* state, int8 arg_num) {
	std::shared_lock lock(MLUAUserData);
	vector<OptionWindowOption>* ret = 0;
	if(lua_islightuserdata(state, arg_num)){
		LUAUserData* data = (LUAUserData*)lua_touserdata(state, arg_num);
		if(!data || !data->IsCorrectlyInitialized()){
			LogError("%s: GetOptionWindow error while processing %s", GetScriptName(state), lua_tostring(state, -1));
		}
		else if(!data->IsOptionWindow()){
			lua_Debug ar;
			lua_getstack (state, 1, &ar);
			lua_getinfo(state, "Sln", &ar);
			LogError("%s: Invalid data type used for GetOptionWindow in %s (line %d)", GetScriptName(state), ar.source, ar.currentline);
		}
		else
			ret = data->option_window_option;
	}
	return ret;
}

Quest* LuaInterface::GetQuest(lua_State* state, int8 arg_num) {
	std::shared_lock lock(MLUAUserData);
	Quest* ret = 0;
	if(lua_islightuserdata(state, arg_num)){
		LUAUserData* data = (LUAUserData*)lua_touserdata(state, arg_num);
		if(!data || !data->IsCorrectlyInitialized()){
			LogError("%s: GetQuest error while processing %s", GetScriptName(state), lua_tostring(state, -1));
		}
		else if(!data->IsQuest()){
			lua_Debug ar;
 			lua_getstack (state, 1, &ar);
			lua_getinfo(state, "Sln", &ar);
			LogError("%s: Invalid data type used for GetQuest in %s (line %d)", GetScriptName(state), ar.source, ar.currentline);
		}
		else
			ret = data->quest;
	}
	return ret;
}

Item* LuaInterface::GetItem(lua_State* state, int8 arg_num) {
	std::shared_lock lock(MLUAUserData);
	Item* ret = 0;
	if(lua_islightuserdata(state, arg_num)){
		LUAUserData* data = (LUAUserData*)lua_touserdata(state, arg_num);
		if(!data || !data->IsCorrectlyInitialized()){
			LogError("%s: GetItem error while processing %s", GetScriptName(state), lua_tostring(state, -1));
		}
		else if(!data->IsItem()){
			lua_Debug ar;
 			lua_getstack (state, 1, &ar);
			lua_getinfo(state, "Sln", &ar);
			LogError("%s: Invalid data type used for GetItem in %s (line %d)", GetScriptName(state), ar.source, ar.currentline);
		}
		else
			ret = data->item;
	}
	return ret;
}

Skill* LuaInterface::GetSkill(lua_State* state, int8 arg_num) {
	std::shared_lock lock(MLUAUserData);
	Skill* ret = 0;
	if (lua_islightuserdata(state, arg_num)) {
		LUAUserData* data = (LUAUserData*)lua_touserdata(state, arg_num);
		if (!data || !data->IsCorrectlyInitialized()) {
			LogError("%s: GetSkill error while processing %s", GetScriptName(state), lua_tostring(state, -1));
		}
		else if (!data->IsSkill()) {
			lua_Debug ar;
			lua_getstack(state, 1, &ar);
			lua_getinfo(state, "Sln", &ar);
			LogError("%s: Invalid data type used for GetSkill in %s (line %d)", GetScriptName(state), ar.source, ar.currentline);
		}
		else
			ret = data->skill;
	}
	return ret;
}

LuaSpell* LuaInterface::GetSpell(lua_State* state, int8 arg_num) {
	std::shared_lock lock(MLUAUserData);
	LuaSpell* ret = 0;
	if (lua_islightuserdata(state, arg_num)) {
		LUAUserData* data = (LUAUserData*)lua_touserdata(state, arg_num);
		if (!data || !data->IsCorrectlyInitialized()) {
			LogError("%s: GetSpell error while processing %s", GetScriptName(state), lua_tostring(state, -1));
		}
		else if (!data->IsSpell()) {
			lua_Debug ar;
			lua_getstack(state, 1, &ar);
			lua_getinfo(state, "Sln", &ar);
			LogError("%s: Invalid data type used for GetSpell in %s (line %d)", GetScriptName(state), ar.source, ar.currentline);
		}
		else
			ret = data->spell;
	}
	return ret;
}

ZoneServer* LuaInterface::GetZone(lua_State* state, int8 arg_num) {
	std::shared_lock lock(MLUAUserData);
	ZoneServer* ret = 0;
	if(lua_islightuserdata(state, arg_num)){
		LUAUserData* data = (LUAUserData*)lua_touserdata(state, arg_num);
		if(!data || !data->IsCorrectlyInitialized()){
			LogError("%s: GetZone error while processing %s", GetScriptName(state), lua_tostring(state, -1));
		}
		else if(!data->IsZone()){
			lua_Debug ar;
			lua_getstack (state, 1, &ar);
			lua_getinfo(state, "Sln", &ar);
			LogError("%s: Invalid data type used for GetZone in %s (line %d)", GetScriptName(state), ar.source, ar.currentline);
		}
		else
			ret = data->zone;
	}
	return ret;
}

sint64 LuaInterface::GetSInt64Value(lua_State* state, int8 arg_num) {
	sint64 val = 0;
	if(lua_isnumber(state, arg_num)){
		val = (sint64)lua_tointeger(state, arg_num);
	}
	return val;
}

int64 LuaInterface::GetInt64Value(lua_State* state, int8 arg_num) {
	int64 val = 0;
	if(lua_isnumber(state, arg_num)){
		val = (int64)lua_tonumber(state, arg_num);
	}
	return val;
}

sint32 LuaInterface::GetSInt32Value(lua_State* state, int8 arg_num) {
	sint32 val = 0;
	if(lua_isnumber(state, arg_num)){
		val = lua_tointeger(state, arg_num);
	}
	return val;
}

int32 LuaInterface::GetInt32Value(lua_State* state, int8 arg_num) {
	int32 val = 0;
	if(lua_isnumber(state, arg_num)){
		val = (int32)lua_tonumber(state, arg_num);
	}
	return val;
}

int16 LuaInterface::GetInt16Value(lua_State* state, int8 arg_num) {
	int16 val = 0;
	if(lua_isnumber(state, arg_num)){
		val = lua_tointeger(state, arg_num);
	}
	return val;
}

int8 LuaInterface::GetInt8Value(lua_State* state, int8 arg_num) {
	int8 val = 0;
	if(lua_isnumber(state, arg_num)){
		val = lua_tointeger(state, arg_num);
	}
	return val;
}

float LuaInterface::GetFloatValue(lua_State* state, int8 arg_num) {
	float val = 0;
	if(lua_isnumber(state, arg_num))
		val = (float)lua_tonumber(state, arg_num);
	return val;
}

string LuaInterface::GetStringValue(lua_State* state, int8 arg_num) {
	string val;
	if(lua_isstring(state, arg_num)){
		size_t size = 0;
		const char* str = lua_tolstring(state, arg_num, &size);
		if(str)
			val = string(str);
	}
	return val;
}

bool LuaInterface::GetBooleanValue(lua_State* state, int8 arg_num) {
	return lua_toboolean(state, arg_num) == 1;
}

void LuaInterface::SetStringValue(lua_State* state, const char* value) {
	lua_pushstring(state, value);
}

void LuaInterface::SetBooleanValue(lua_State* state, bool value) {
	lua_pushboolean(state, value);
}

void LuaInterface::SetFloatValue(lua_State* state, float value) {
	lua_pushnumber(state, value);
}

void LuaInterface::SetInt32Value(lua_State* state, int32 value) {
	lua_pushinteger(state, value);
}

void LuaInterface::SetSInt32Value(lua_State* state, sint32 value) {
	lua_pushinteger(state, value);
}

void LuaInterface::SetInt64Value(lua_State* state, int64 value) {
	lua_pushinteger(state, value);
}

void LuaInterface::SetSInt64Value(lua_State* state, sint64 value) {
	lua_pushinteger(state, value);
}

void LuaInterface::SetSpawnValue(lua_State* state, Spawn* spawn) {
	LUASpawnWrapper* spawn_wrapper = new LUASpawnWrapper();
	spawn_wrapper->spawn = spawn;
	AddUserDataPtr(spawn_wrapper, spawn);
	lua_pushlightuserdata(state, spawn_wrapper);
}

void LuaInterface::SetConversationValue(lua_State* state, vector<ConversationOption>* conversation) {
	LUAConversationOptionWrapper* conv_wrapper = new LUAConversationOptionWrapper();
	conv_wrapper->conversation_options = conversation;
	AddUserDataPtr(conv_wrapper, conversation);
	lua_pushlightuserdata(state, conv_wrapper);
}

void LuaInterface::SetOptionWindowValue(lua_State* state, vector<OptionWindowOption>* optionWindow) {
	LUAOptionWindowWrapper* option_wrapper = new LUAOptionWindowWrapper();
	option_wrapper->option_window_option = optionWindow;
	AddUserDataPtr(option_wrapper, optionWindow);
	lua_pushlightuserdata(state, option_wrapper);
}

void LuaInterface::SetItemValue(lua_State* state, Item* item) {
	LUAItemWrapper* item_wrapper = new LUAItemWrapper();
	item_wrapper->item = item;
	AddUserDataPtr(item_wrapper, item);
	lua_pushlightuserdata(state, item_wrapper);
}

void LuaInterface::SetSkillValue(lua_State* state, Skill* skill) {
	LUASkillWrapper* skill_wrapper = new LUASkillWrapper();
	skill_wrapper->skill = skill;
	AddUserDataPtr(skill_wrapper, skill);
	lua_pushlightuserdata(state, skill_wrapper);
}

void LuaInterface::SetQuestValue(lua_State* state, Quest* quest) {
	LUAQuestWrapper* quest_wrapper = new LUAQuestWrapper();
	quest_wrapper->quest = quest;
	AddUserDataPtr(quest_wrapper, quest);
	lua_pushlightuserdata(state, quest_wrapper);
}

void LuaInterface::SetZoneValue(lua_State* state, ZoneServer* zone) {
	LUAZoneWrapper* zone_wrapper = new LUAZoneWrapper();
	zone_wrapper->zone = zone;
	AddUserDataPtr(zone_wrapper, zone);
	lua_pushlightuserdata(state, zone_wrapper);
}

void LuaInterface::SetSpellValue(lua_State* state, LuaSpell* spell) {
	LUASpellWrapper* spell_wrapper = new LUASpellWrapper();
	spell_wrapper->spell = spell;
	AddUserDataPtr(spell_wrapper, spell);
	lua_pushlightuserdata(state, spell_wrapper);
}

LuaSpell* LuaInterface::LoadSpellScript(const char* name)  {
	LuaSpell* spell = nullptr;
	string lua_script = string(name);
	if (lua_script.find(".lua") == string::npos)
		lua_script.append(".lua");
	lua_State* state = LoadLuaFile(lua_script.c_str());
	if(state) {
		spell = new LuaSpell;
		spell->file_name = lua_script;
		spell->state = state;
		spell->spell = 0;
		spell->caster = 0;
		spell->initial_target = 0;
		spell->resisted = false;
		spell->has_damaged = false;
		spell->is_damage_spell = false;
		spell->interrupted = false;
		spell->last_spellattack_hit = false;
		spell->crit = false;
		spell->MSpellTargets.SetName("LuaSpell.MSpellTargets");
		spell->cancel_after_all_triggers = false;
		spell->num_triggers = 0;
		spell->num_calls = 0;
		spell->is_recast_timer = false;
		spell->had_triggers = false;
		spell->had_dmg_remaining = false;
		spell->slot_pos = 0;
		spell->damage_remaining = 0;
		spell->effect_bitmask = 0;
		spell->restored = false;
		spell->has_proc = false;
		spell->initial_caster_char_id = 0;
		spell->initial_target_char_id = 0;
		
		MSpells.lock();
		current_spells[spell->state] = spell;
		MSpells.unlock();
		
		MSpellScripts.writelock(__FUNCTION__, __LINE__);
		spell_scripts[lua_script][state] = spell;
		MSpellScripts.releasewritelock(__FUNCTION__, __LINE__);
	}
	return spell;
}

Mutex* LuaInterface::GetItemScriptMutex(const char* name) {
	Mutex* mutex = 0;
	if(item_scripts_mutex.count(name) > 0)
		mutex = item_scripts_mutex[name];
	if(!mutex){
		mutex = new Mutex();
		item_scripts_mutex[name] = mutex;
	}
	return mutex;
}

Mutex* LuaInterface::GetSpawnScriptMutex(const char* name) {
	Mutex* mutex = 0;
	if(spawn_scripts_mutex.count(string(name)) > 0)
		mutex = spawn_scripts_mutex[name];
	if(!mutex){
		mutex = new Mutex();
		spawn_scripts_mutex[name] = mutex;
	}
	return mutex;
}

Mutex* LuaInterface::GetZoneScriptMutex(const char* name) {
	Mutex* mutex = 0;
	if(zone_scripts_mutex.count(name) > 0)
		mutex = zone_scripts_mutex[name];
	if(!mutex){
		mutex = new Mutex();
		zone_scripts_mutex[name] = mutex;
	}
	return mutex;
}

Mutex* LuaInterface::GetRegionScriptMutex(const char* name) {
	Mutex* mutex = 0;
	if(region_scripts_mutex.count(name) > 0)
		mutex = region_scripts_mutex[name];
	if(!mutex){
		mutex = new Mutex();
		region_scripts_mutex[name] = mutex;
	}
	return mutex;
}

Mutex* LuaInterface::GetSpellScriptMutex(const char* name) {
	Mutex* mutex = 0;
	if(spell_scripts_mutex.count(name) > 0)
		mutex = spell_scripts_mutex[name];
	if(!mutex){
		mutex = new Mutex();
		spell_scripts_mutex[name] = mutex;
	}
	return mutex;
}

void LuaInterface::UseItemScript(const char* name, lua_State* state, bool val) {
	MItemScripts.writelock(__FUNCTION__, __LINE__);
	item_scripts[name][state] = val;
	item_inverse_scripts[state] = name;
	MItemScripts.releasewritelock(__FUNCTION__, __LINE__);
}

void LuaInterface::UseSpawnScript(const char* name, lua_State* state, bool val) {
	MSpawnScripts.writelock(__FUNCTION__, __LINE__);
	spawn_scripts[name][state] = val;
	spawn_inverse_scripts[state] = name;
	MSpawnScripts.releasewritelock(__FUNCTION__, __LINE__);
}

void LuaInterface::UseZoneScript(const char* name, lua_State* state, bool val) {

	MZoneScripts.writelock(__FUNCTION__, __LINE__);
	zone_scripts[name][state] = val;
	zone_inverse_scripts[state] = name;
	MZoneScripts.releasewritelock(__FUNCTION__, __LINE__);
}

void LuaInterface::UseRegionScript(const char* name, lua_State* state, bool val) {

	MRegionScripts.writelock(__FUNCTION__, __LINE__);
	region_scripts[name][state] = val;
	region_inverse_scripts[state] = name;
	MRegionScripts.releasewritelock(__FUNCTION__, __LINE__);
}

lua_State* LuaInterface::GetItemScript(const char* name, bool create_new, bool use) {
	map<string, map<lua_State*, bool> >::iterator itr;
	map<lua_State*, bool>::iterator item_script_itr;
	lua_State* ret = 0;
	Mutex* mutex = 0;

	itr = item_scripts.find(name);
	if(itr != item_scripts.end()) {
		mutex = GetItemScriptMutex(name);
		mutex->readlock(__FUNCTION__, __LINE__);
		for(item_script_itr = itr->second.begin(); item_script_itr != itr->second.end(); item_script_itr++){
			if(!item_script_itr->second){ //not in use
				ret = item_script_itr->first;

				if (use)
				{
					item_script_itr->second = true;
					break; // don't keep iterating, we already have our result
				}
			}
		}
		mutex->releasereadlock(__FUNCTION__, __LINE__);
	}
	if(!ret && create_new){
		if(name && LoadItemScript(name))
			ret = GetItemScript(name, create_new, use);
		else{
			LogError("Error LUA Item Script '%s'", name);
			return 0;
		}
	}
	return ret;
}

lua_State* LuaInterface::GetSpawnScript(const char* name, bool create_new, bool use) {
	if (lua_system_reloading)
		return 0;
	map<string, map<lua_State*, bool> >::iterator itr;
	map<lua_State*, bool>::iterator spawn_script_itr;
	lua_State* ret = 0;
	Mutex* mutex = 0;

	itr = spawn_scripts.find(name);
	if(itr != spawn_scripts.end()) {
		mutex = GetSpawnScriptMutex(name);
		mutex->readlock(__FUNCTION__, __LINE__);
		for(spawn_script_itr = itr->second.begin(); spawn_script_itr != itr->second.end(); spawn_script_itr++){
			if(!spawn_script_itr->second){ //not in use
				ret = spawn_script_itr->first;

				if (use)
				{
					spawn_script_itr->second = true;
					break; // don't keep iterating, we already have our result
				}
			}
		}
		mutex->releasereadlock(__FUNCTION__, __LINE__);
	}
	if(!ret && create_new){
		if(name && LoadSpawnScript(name))
			ret = GetSpawnScript(name, create_new, use);
		else{
			LogError("Error LUA Spawn Script '%s'", name);
			return 0;
		}
	}
	return ret;
}

lua_State* LuaInterface::GetZoneScript(const char* name, bool create_new, bool use)  {
	map<string, map<lua_State*, bool> >::iterator itr;
	map<lua_State*, bool>::iterator zone_script_itr;
	lua_State* ret = 0;
	Mutex* mutex = 0;

	itr = zone_scripts.find(name);
	if(itr != zone_scripts.end()) {
		mutex = GetZoneScriptMutex(name);
		mutex->readlock(__FUNCTION__, __LINE__);
		for(zone_script_itr = itr->second.begin(); zone_script_itr != itr->second.end(); zone_script_itr++){
			if(!zone_script_itr->second){ //not in use
				ret = zone_script_itr->first;

				if (use)
				{
					zone_script_itr->second = true;
					break; // don't keep iterating, we already have our result
				}
			}
		}
		mutex->releasereadlock(__FUNCTION__, __LINE__);
	}
	if(!ret && create_new){
		if(name && LoadZoneScript(name))
			ret = GetZoneScript(name, create_new, use);
		else{
			LogError("Error LUA Zone Script '%s'", name);
			return 0;
		}
	}
	return ret;
}

lua_State* LuaInterface::GetRegionScript(const char* name, bool create_new, bool use)  {
	map<string, map<lua_State*, bool> >::iterator itr;
	map<lua_State*, bool>::iterator region_script_itr;
	lua_State* ret = 0;
	Mutex* mutex = 0;

	itr = region_scripts.find(name);
	if(itr != region_scripts.end()) {
		mutex = GetRegionScriptMutex(name);
		mutex->readlock(__FUNCTION__, __LINE__);
		for(region_script_itr = itr->second.begin(); region_script_itr != itr->second.end(); region_script_itr++){
			if(!region_script_itr->second){ //not in use
				ret = region_script_itr->first;

				if (use)
				{
					region_script_itr->second = true;
					break; // don't keep iterating, we already have our result
				}
			}
		}
		mutex->releasereadlock(__FUNCTION__, __LINE__);
	}
	if(!ret && create_new){
		if(name && LoadRegionScript(name))
			ret = GetRegionScript(name, create_new, use);
		else{
			LogError("Error LUA Zone Script '%s'", name);
			return 0;
		}
	}
	return ret;
}

LuaSpell* LuaInterface::GetSpellScript(const char* name, bool create_new, bool use)  {
	map<string, map<lua_State*, LuaSpell*> >::iterator itr;
	map<lua_State*, LuaSpell*>::iterator spell_script_itr;
	LuaSpell* ret = 0;
	Mutex* mutex = 0;

	MSpells.lock();
	MSpellScripts.writelock(__FUNCTION__, __LINE__);
	itr = spell_scripts.find(name);
	if(itr != spell_scripts.end()) {
		mutex = GetSpellScriptMutex(name);
		mutex->writelock(__FUNCTION__, __LINE__);
		for(spell_script_itr = itr->second.begin(); spell_script_itr != itr->second.end(); spell_script_itr++){
			if(spell_script_itr->second == nullptr){ //not in use
				if (use)
				{
					lua_State* state = spell_script_itr->first;
					ret = CreateSpellScript(name, state);
					break; // don't keep iterating, we already have our result
				}
			}
		}
		mutex->releasewritelock(__FUNCTION__, __LINE__);
	}
	MSpellScripts.releasewritelock(__FUNCTION__, __LINE__);
	MSpells.unlock();
	
	if(!ret && create_new){
		if(!name || (ret = LoadSpellScript(name)) == nullptr) {
			LogError("Error LUA Spell Script '%s'", name == nullptr ? "unknown" : name);
		}
	}
	return ret;
}

LuaSpell* LuaInterface::CreateSpellScript(const char* name, lua_State* existState) {
	LuaSpell* new_spell = new LuaSpell;
	new_spell->state = existState;
	spell_scripts[std::string(name)][new_spell->state] = new_spell;
	new_spell->file_name = string(name);
	new_spell->resisted = false;
	new_spell->is_damage_spell = false;
	new_spell->has_damaged = false;
	new_spell->interrupted = false;
	new_spell->crit = false;
	new_spell->last_spellattack_hit = false;
	new_spell->MSpellTargets.SetName("LuaSpell.MSpellTargets");
	new_spell->cancel_after_all_triggers = false;
	new_spell->num_triggers = 0;
	new_spell->num_calls = 0;
	new_spell->is_recast_timer = false;
	new_spell->had_triggers = false;
	new_spell->had_dmg_remaining = false;
	new_spell->slot_pos = 0;
	new_spell->damage_remaining = 0;
	new_spell->effect_bitmask = 0;
	new_spell->caster = 0;
	new_spell->initial_target = 0;
	new_spell->spell = 0;
	new_spell->restored = false;
	new_spell->has_proc = false;
	new_spell->initial_caster_char_id = 0;
	new_spell->initial_target_char_id = 0;
	
	current_spells[new_spell->state] = new_spell;
	return new_spell;
}

LuaSpell* LuaInterface::GetSpell(const char* name) {
	return GetSpellScript(name, true);
}

bool LuaInterface::RunItemScript(string script_name, const char* function_name, Item* item, Spawn* spawn, Spawn* target, sint64* returnValue) {
	if(!item)
		return false;
	lua_State* state = GetItemScript(script_name.c_str(), true, true);
	if(state){
		Mutex* mutex = GetItemScriptMutex(script_name.c_str());
		if(mutex)
			mutex->readlock(__FUNCTION__, __LINE__);
		else{
			LogError("Error getting lock for '%s'", script_name.c_str());
			UseItemScript(script_name.c_str(), state, false);
			return false;
		}
		lua_getglobal(state, function_name);
		if (!lua_isfunction(state, lua_gettop(state))){
			lua_pop(state, 1);
			mutex->releasereadlock(__FUNCTION__);
			UseItemScript(script_name.c_str(), state, false);
			return false;
		}
		SetItemValue(state, item);
		int8 num_parms = 1;
		if(spawn){
			SetSpawnValue(state, spawn);
			num_parms++;
		}
		if(target){
			SetSpawnValue(state, target);
			num_parms++;
		}
		if(!CallItemScript(state, num_parms, returnValue)){
			if(mutex)
				mutex->releasereadlock(__FUNCTION__, __LINE__);
			UseItemScript(script_name.c_str(), state, false);
			return false;
		}
		if(mutex)
			mutex->releasereadlock(__FUNCTION__, __LINE__);
		UseItemScript(script_name.c_str(), state, false);
		return true;
	}
	else
		return false;
}

bool LuaInterface::RunItemScriptWithReturnString(string script_name, const char* function_name, Item* item, Spawn* spawn, std::string* returnValue) {
	if(!item)
		return false;
	lua_State* state = GetItemScript(script_name.c_str(), true, true);
	if(state){
		Mutex* mutex = GetItemScriptMutex(script_name.c_str());
		if(mutex)
			mutex->readlock(__FUNCTION__, __LINE__);
		else{
			LogError("Error getting lock for '%s'", script_name.c_str());
			UseItemScript(script_name.c_str(), state, false);
			return false;
		}
		lua_getglobal(state, function_name);
		if (!lua_isfunction(state, lua_gettop(state))){
			lua_pop(state, 1);
			mutex->releasereadlock(__FUNCTION__);
			UseItemScript(script_name.c_str(), state, false);
			return false;
		}
		SetItemValue(state, item);
		int8 num_parms = 1;
		if(spawn){
			SetSpawnValue(state, spawn);
			num_parms++;
		}
		if(!CallItemScript(state, num_parms, returnValue)){
			if(mutex)
				mutex->releasereadlock(__FUNCTION__, __LINE__);
			UseItemScript(script_name.c_str(), state, false);
			return false;
		}
		if(mutex)
			mutex->releasereadlock(__FUNCTION__, __LINE__);
		UseItemScript(script_name.c_str(), state, false);
		return true;
	}
	else
		return false;
}


bool LuaInterface::RunSpawnScript(string script_name, const char* function_name, Spawn* npc, Spawn* spawn, const char* message, bool is_door_open, sint32 input_value, sint32* return_value) {
	if(!npc || lua_system_reloading)
		return false;

	bool isUseDoorFunction = false;
	if(!strcmp(function_name,"usedoor"))
		isUseDoorFunction = true;
		
	lua_State* state = GetSpawnScript(script_name.c_str(), true, true);
	if(state){
		Mutex* mutex = GetSpawnScriptMutex(script_name.c_str());
		if(mutex)
			mutex->readlock(__FUNCTION__, __LINE__);
		else{
			LogError("Error getting lock for '%s'", script_name.c_str());
			UseSpawnScript(script_name.c_str(), state, false);
			return false;
		}
		lua_getglobal(state, function_name);
		if (!lua_isfunction(state, lua_gettop(state))){
			lua_pop(state, 1);
			mutex->releasereadlock(__FUNCTION__);
			UseSpawnScript(script_name.c_str(), state, false);
			return false;
		}
		SetSpawnValue(state, npc);
		int8 num_parms = 1;
		// we always send spawn, even if null (nil) when its 'usedoor' function
		if(spawn || isUseDoorFunction){
			SetSpawnValue(state, spawn);
			num_parms++;
		}

		// usedoor function always passes just npc, spawn and is_door_open
		if(isUseDoorFunction){
			SetBooleanValue(state, is_door_open);
			num_parms++;
		}
		else {
			if(message){
			SetStringValue(state, message);
			num_parms++;
			}

			if(!strcmp(function_name,"healthchanged"))
			{
				// passes as damage dealt
				SetSInt32Value(state, input_value);
				num_parms++;
			}
		}
		if(!CallScriptSInt32(state, num_parms, return_value)){
			if(mutex)
				mutex->releasereadlock(__FUNCTION__, __LINE__);
			UseSpawnScript(script_name.c_str(), state, false);
			return false;
		}
		if(mutex)
			mutex->releasereadlock(__FUNCTION__, __LINE__);
		UseSpawnScript(script_name.c_str(), state, false);
		return true;
	}
	else
		return false;
}

bool LuaInterface::RunZoneScript(string script_name, const char* function_name, ZoneServer* zone, Spawn* spawn, int32 int32_arg1, const char* str_arg1, Spawn* spawn_arg1, int32 int32_arg2, const char* str_arg2, Spawn* spawn_arg2) {
	if (!zone)
		return false;
	lua_State* state = GetZoneScript(script_name.c_str(), true, true);
	if (state) {
		Mutex* mutex = GetZoneScriptMutex(script_name.c_str());
		if (mutex)
			mutex->readlock(__FUNCTION__, __LINE__);
		else {
			LogError("Error getting lock for '%s'", script_name.c_str());
			UseZoneScript(script_name.c_str(), state, false);
			return false;
		}
		lua_getglobal(state, function_name);
		if (!lua_isfunction(state, lua_gettop(state))) {
			lua_pop(state, 1);
			mutex->releasereadlock(__FUNCTION__);
			UseZoneScript(script_name.c_str(), state, false);
			return false;
		}
		SetZoneValue(state, zone);
		int8 num_params = 1;
		if (spawn) {
			SetSpawnValue(state, spawn);
			num_params++;
		}
		if (int32_arg1 > 0) {
			SetInt32Value(state, int32_arg1);
			num_params++;
		}
		if (str_arg1) {
			SetStringValue(state, str_arg1);
			num_params++;
		}
		if (spawn_arg1) {
			SetSpawnValue(state, spawn_arg1);
			num_params++;
		}
		if (int32_arg2 > 0) {
			SetInt32Value(state, int32_arg2);
			num_params++;
		}
		if (str_arg2) {
			SetStringValue(state, str_arg2);
			num_params++;
		}
		if (spawn_arg2) {
			SetSpawnValue(state, spawn_arg2);
			num_params++;
		}
		if (!CallScriptInt32(state, num_params)) {
			if (mutex)
				mutex->releasereadlock(__FUNCTION__, __LINE__);
			UseZoneScript(script_name.c_str(), state, false);
			return false;
		}
		if (mutex)
			mutex->releasereadlock(__FUNCTION__, __LINE__);
		UseZoneScript(script_name.c_str(), state, false);
		return true;
	}
	else
		return false;
}

bool LuaInterface::RunZoneScriptWithReturn(string script_name, const char* function_name, ZoneServer* zone, Spawn* spawn, int32 int32_arg1, int32 int32_arg2, int32 int32_arg3, int32* returnValue) {
	if (!zone)
		return false;
	lua_State* state = GetZoneScript(script_name.c_str(), true, true);
	if (state) {
		Mutex* mutex = GetZoneScriptMutex(script_name.c_str());
		if (mutex)
			mutex->readlock(__FUNCTION__, __LINE__);
		else {
			LogError("Error getting lock for '%s'", script_name.c_str());
			UseZoneScript(script_name.c_str(), state, false);
			return false;
		}
		lua_getglobal(state, function_name);
		if (!lua_isfunction(state, lua_gettop(state))) {
			lua_pop(state, 1);
			mutex->releasereadlock(__FUNCTION__);
			UseZoneScript(script_name.c_str(), state, false);
			return false;
		}
		SetZoneValue(state, zone);
		int8 num_params = 1;
		SetSpawnValue(state, spawn);
		num_params++;
		SetInt32Value(state, int32_arg1);
		num_params++;
		SetInt32Value(state, int32_arg2);
		num_params++;
		SetInt32Value(state, int32_arg3);
		num_params++;
		if (!CallScriptInt32(state, num_params, returnValue)) {
			if (mutex)
				mutex->releasereadlock(__FUNCTION__, __LINE__);
			UseZoneScript(script_name.c_str(), state, false);
			return false;
		}
		if (mutex)
			mutex->releasereadlock(__FUNCTION__, __LINE__);
		UseZoneScript(script_name.c_str(), state, false);
		return true;
	}
	else
		return false;
}


bool LuaInterface::RunRegionScript(string script_name, const char* function_name, ZoneServer* zone, Spawn* spawn, sint32 int32_arg1, int32* returnValue) {
	if (!zone)
		return false;
	lua_State* state = GetRegionScript(script_name.c_str(), true, true);
	if (state) {
		Mutex* mutex = GetRegionScriptMutex(script_name.c_str());
		if (mutex)
			mutex->readlock(__FUNCTION__, __LINE__);
		else {
			LogError("Error getting lock for '%s'", script_name.c_str());
			UseRegionScript(script_name.c_str(), state, false);
			return false;
		}
		lua_getglobal(state, function_name);
		if (!lua_isfunction(state, lua_gettop(state))) {
			lua_pop(state, 1);
			mutex->releasereadlock(__FUNCTION__);
			UseRegionScript(script_name.c_str(), state, false);
			return false;
		}
		SetZoneValue(state, zone);
		int8 num_params = 1;
		if (spawn) {
			SetSpawnValue(state, spawn);
			num_params++;
		}
		if (int32_arg1 > 0) {
			SetSInt32Value(state, int32_arg1);
			num_params++;
		}
		if (!CallRegionScript(state, num_params, returnValue)) {
			if (mutex)
				mutex->releasereadlock(__FUNCTION__, __LINE__);
			UseRegionScript(script_name.c_str(), state, false);
			return false;
		}
		if (mutex)
			mutex->releasereadlock(__FUNCTION__, __LINE__);
		UseRegionScript(script_name.c_str(), state, false);
		return true;
	}
	else
		return false;
}

void LuaInterface::AddPendingSpellDelete(LuaSpell* spell) {
	MSpellDelete.lock();
	if ( spells_pending_delete.count(spell) == 0 )
		spells_pending_delete[spell] = Timer::GetCurrentTime2() + 10000;
	MSpellDelete.unlock();
}

void LuaInterface::AddCustomSpell(LuaSpell* spell)
{
	MCustomSpell.writelock();
	custom_spells[spell->spell->GetSpellID()] = spell;
	MCustomSpell.releasewritelock();
}

void LuaInterface::RemoveCustomSpell(int32 id)
{
	MCustomSpell.writelock();
	map<int32, LuaSpell*>::iterator itr = custom_spells.find(id);
	if (itr != custom_spells.end())
	{
		custom_spells.erase(itr);
		custom_free_spell_ids.push_front(id);
	}
	MCustomSpell.releasewritelock();
}

// prior to calling FindCustomSpell you should call FindCustomSpellLock and after FindCustomSpellUnlock
LuaSpell* LuaInterface::FindCustomSpell(int32 id)
{
	LuaSpell* spell = 0;
	map<int32, LuaSpell*>::iterator itr = custom_spells.find(id);
	if (itr != custom_spells.end())
		spell = itr->second;
	return spell;
}

int32 LuaInterface::GetFreeCustomSpellID()
{ 
	int32 id = 0;
	MCustomSpell.writelock();
	if (!custom_free_spell_ids.empty())
	{
		id = custom_free_spell_ids.front();
		custom_free_spell_ids.pop_front();
	}
	MCustomSpell.releasewritelock();
	return id;
}


void LuaInterface::SetLuaUserDataStale(void* ptr) {
	std::unique_lock lock(MLUAUserData);
	std::map<void*, LUAUserData*>::iterator itr = user_data_ptr.find(ptr);
	if(itr != user_data_ptr.end()) {
		itr->second->correctly_initialized = false;
	}
}

LUAUserData::LUAUserData(){
	correctly_initialized = false;
	quest = 0;
	conversation_options = 0;
	spawn = 0;
	zone = 0;
	skill = 0;
	option_window_option = 0;
	item = 0;
}

bool LUAUserData::IsCorrectlyInitialized(){
	return correctly_initialized;
}

bool LUAUserData::IsConversationOption(){
	return false;
}

bool LUAUserData::IsOptionWindow() {
	return false;
}

bool LUAUserData::IsSpawn(){
	return false;
}

bool LUAUserData::IsQuest(){
	return false;
}

bool LUAUserData::IsZone(){
	return false;
}

bool LUAUserData::IsItem(){
	return false;
}

bool LUAUserData::IsSkill() {
	return false;
}

bool LUAUserData::IsSpell() {
	return false;
}

LUAConversationOptionWrapper::LUAConversationOptionWrapper(){
	correctly_initialized = true;
}

bool LUAConversationOptionWrapper::IsConversationOption(){
	return true;
}

LUAOptionWindowWrapper::LUAOptionWindowWrapper() {
	correctly_initialized = true;
}

bool LUAOptionWindowWrapper::IsOptionWindow() {
	return true;
}

LUASpawnWrapper::LUASpawnWrapper(){
	correctly_initialized = true;
}

bool LUASpawnWrapper::IsSpawn(){
	return true;
}

LUAZoneWrapper::LUAZoneWrapper(){
	correctly_initialized = true;
}

bool LUAZoneWrapper::IsZone(){
	return true;
}

LUAQuestWrapper::LUAQuestWrapper(){
	correctly_initialized = true;
}

bool LUAQuestWrapper::IsQuest(){
	return true;
}

LUAItemWrapper::LUAItemWrapper(){
	correctly_initialized = true;
}

bool LUAItemWrapper::IsItem(){
	return true;
}

LUASkillWrapper::LUASkillWrapper() {
	correctly_initialized = true;
}

bool LUASkillWrapper::IsSkill() {
	return true;
}

LUASpellWrapper::LUASpellWrapper() {
	correctly_initialized = true;
}

bool LUASpellWrapper::IsSpell() {
	return true;
}