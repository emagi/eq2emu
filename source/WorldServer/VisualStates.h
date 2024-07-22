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
#include "../common/Log.h"
#include "../../common/MiscFunctions.h"
#include <map>

using namespace std;

// Visual States must use a hash table because of the large amount that exists and the large spacing
// between their ID's.  String and character arrays could not be used for the first iterator because
// it would require the same pointer to access it from the hash table, which is obviously not possible
// since the text is from the client.

// maximum amount of iterations it will attempt to find a entree
#define HASH_SEARCH_MAX 20

class VisualState
{
public:
	VisualState(int inID, char* inName){
		if(!inName)
			return;
		name = string(inName);
		id = inID;
	}

	int GetID() { return id; }
	const char* GetName() { return name.c_str(); }
	string GetNameString() { return name; }

private:
	int id;
	string name;
};
class Emote{
public:
	Emote(char* in_name, int in_visual_state, char* in_message, char* in_targeted_message){
		if(!in_name)
			return;
		name = string(in_name);
		visual_state = in_visual_state;
		if(in_message)
			message = string(in_message);
		if(in_targeted_message)
			targeted_message = string(in_targeted_message);
	}
	int32 GetVisualState() { return visual_state; }
	const char* GetName() { return name.c_str(); }
	const char* GetMessage() { return message.c_str(); }
	const char* GetTargetedMessage() { return targeted_message.c_str(); }

	string GetNameString() { return name; }
	string GetMessageString() { return message; }
	string GetTargetedMessageString() { return targeted_message; }
private:
	int32 visual_state;
	string name;
	string message;
	string targeted_message;
};

class EmoteVersionRange {
public:
	EmoteVersionRange(char* in_name)
	{
		name = string(in_name);
	}

	~EmoteVersionRange()
	{
		map<VersionRange*, Emote*>::iterator itr;
		for (itr = version_map.begin(); itr != version_map.end(); itr++)
		{
			VersionRange* range = itr->first;
			Emote* emote = itr->second;
			delete range;
			delete emote;
		}

		version_map.clear();
	}

	void AddVersionRange(int32 min_version, int32 max_version,
		char* in_name, int in_visual_state, char* in_message = nullptr, char* in_targeted_message = nullptr)
	{
		map<VersionRange*, Emote*>::iterator itr = FindVersionRange(min_version, max_version);
		if (itr != version_map.end())
		{
			VersionRange* range = itr->first;
			LogWrite(WORLD__ERROR, 0, "Emotes Table Error: Duplicate emote mapping of %s with range min %u max %u, Existing found with range min %u max %u\n", name.c_str(), min_version, max_version, range->GetMinVersion(), range->GetMaxVersion());
			return;
		}

		version_map.insert(make_pair(new VersionRange(min_version, max_version), new Emote(in_name, in_visual_state, in_message, in_targeted_message)));
	}

	map<VersionRange*, Emote*>::iterator FindVersionRange(int32 min_version, int32 max_version)
	{
		map<VersionRange*, Emote*>::iterator itr;
		for (itr = version_map.begin(); itr != version_map.end(); itr++)
		{
			VersionRange* range = itr->first;
			// if min and max version are both in range
			if (range->GetMinVersion() <= min_version && max_version <= range->GetMaxVersion())
				return itr;
			// if the min version is in range, but max range is 0
			else if (range->GetMinVersion() <= min_version && range->GetMaxVersion() == 0)
				return itr;
			// if min version is 0 and max_version has a cap
			else if (range->GetMinVersion() == 0 && max_version <= range->GetMaxVersion())
				return itr;
		}

		return version_map.end();
	}

	map<VersionRange*, Emote*>::iterator FindEmoteVersion(int32 version)
	{
		map<VersionRange*, Emote*>::iterator itr;
		for (itr = version_map.begin(); itr != version_map.end(); itr++)
		{
			VersionRange* range = itr->first;
			// if min and max version are both in range
			if (version >= range->GetMinVersion() && (range->GetMaxVersion() == 0 || version <= range->GetMaxVersion()))
				return itr;
		}

		return version_map.end();
	}

	const char* GetName() { return name.c_str(); }
	string GetNameString() { return name; }

	map<VersionRange*, Emote*>::iterator GetRangeEnd() { return version_map.end(); }
private:
	map<VersionRange*, Emote*> version_map;
	string name;
};

class VisualStates
{
public:
	~VisualStates(){
		Reset();
	}

	void Reset(){
		ClearVisualStates();
		ClearEmotes();
		ClearSpellVisuals();
	}
	
	void ClearEmotes(){
		map<string, EmoteVersionRange*>::iterator map_list;
		for(map_list = emoteMap.begin(); map_list != emoteMap.end(); map_list++ )
			safe_delete(map_list->second);
		emoteMap.clear();
	}

	void ClearVisualStates(){
		map<string, VisualState*>::iterator map_list;
		for(map_list = visualStateMap.begin(); map_list != visualStateMap.end(); map_list++ )
			safe_delete(map_list->second);
		visualStateMap.clear();
	}

	void InsertVisualState(VisualState* vs){
		visualStateMap[vs->GetNameString()] = vs;
	}

	VisualState* FindVisualState(string var){
		if(visualStateMap.count(var) > 0)
			return visualStateMap[var];
		return 0;
	}

	void InsertEmoteRange(EmoteVersionRange* emote) {
		emoteMap[emote->GetName()] = emote;
	}

	EmoteVersionRange* FindEmoteRange(string var) {
		if (emoteMap.count(var) > 0)
		{
			return emoteMap[var];
		}
		return 0;
	}

	Emote* FindEmote(string var, int32 version){
		if (emoteMap.count(var) > 0)
		{
			map<VersionRange*,Emote*>::iterator itr = emoteMap[var]->FindEmoteVersion(version);

			if (itr != emoteMap[var]->GetRangeEnd())
			{
				Emote* emote = itr->second;
				return emote;
			}
		}
		return 0;
	}
	
	void InsertSpellVisualRange(EmoteVersionRange* emote, int32 spell_visual_id) {
		spellMap[emote->GetName()] = emote;
		spellMapID[spell_visual_id] = emote;
	}

	EmoteVersionRange* FindSpellVisualRange(string var) {
		if (spellMap.count(var) > 0)
		{
			return spellMap[var];
		}
		return 0;
	}

	EmoteVersionRange* FindSpellVisualRangeByID(int32 id) {
		if (spellMapID.count(id) > 0)
		{
			return spellMapID[id];
		}
		return 0;
	}

	Emote* FindSpellVisual(string var, int32 version){
		if (spellMap.count(var) > 0)
		{
			map<VersionRange*,Emote*>::iterator itr = spellMap[var]->FindEmoteVersion(version);

			if (itr != spellMap[var]->GetRangeEnd())
			{
				Emote* emote = itr->second;
				return emote;
			}
		}
		return 0;
	}
	
	Emote* FindSpellVisualByID(int32 visual_id, int32 version){
		if (spellMapID.count(visual_id) > 0)
		{
			map<VersionRange*,Emote*>::iterator itr = spellMapID[visual_id]->FindEmoteVersion(version);

			if (itr != spellMapID[visual_id]->GetRangeEnd())
			{
				Emote* emote = itr->second;
				return emote;
			}
		}
		return 0;
	}

	void ClearSpellVisuals(){
		map<string, EmoteVersionRange*>::iterator map_list;
		for(map_list = spellMap.begin(); map_list != spellMap.end(); map_list++ )
			safe_delete(map_list->second);
		spellMap.clear();
		spellMapID.clear();
	}
private:
	map<string,VisualState*> visualStateMap;
	map<string,EmoteVersionRange*> emoteMap;
	map<string,EmoteVersionRange*> spellMap;
	map<int32,EmoteVersionRange*> spellMapID;
};

