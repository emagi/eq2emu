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
#ifndef EQ2_FACTIONS
#define EQ2_FACTIONS

#include "../common/ConfigReader.h"
#include "../common/Mutex.h"

struct Faction {
	int32	id;
	string	name;
	string	type;
	string	description;
	int16	negative_change;
	int16	positive_change;
	sint32	default_value;
};

class MasterFactionList{
public:
	MasterFactionList(){

	}
	~MasterFactionList(){
		Clear();
	}
	void Clear() {
		map<int32,Faction*>::iterator iter;
		for(iter = global_faction_list.begin();iter != global_faction_list.end(); iter++){
			safe_delete(iter->second);
		}

		hostile_factions.clear();
		friendly_factions.clear();
	}
	sint32 GetDefaultFactionValue(int32 faction_id){
		if(global_faction_list.count(faction_id) > 0 && global_faction_list[faction_id])
			return global_faction_list[faction_id]->default_value;
		return 0;
	}
	Faction* GetFaction(char* name){
		return faction_name_list[name];
	}
	Faction* GetFaction(int32 id){
		if(global_faction_list.count(id) > 0)
			return global_faction_list[id];
		return 0;
	}
	void AddFaction(Faction* faction){
		global_faction_list[faction->id] = faction;
		faction_name_list[faction->name] = faction;
	}
	sint32 GetIncreaseAmount(int32 faction_id){
		if(global_faction_list.count(faction_id) > 0 && global_faction_list[faction_id])
			return global_faction_list[faction_id]->positive_change;
		return 0;
	}
	sint32 GetDecreaseAmount(int32 faction_id){
		if(global_faction_list.count(faction_id) > 0 && global_faction_list[faction_id])
			return global_faction_list[faction_id]->negative_change;
		return 0;
	}
	int32 GetFactionCount(){
		return global_faction_list.size();
	}
	void AddHostileFaction(int32 faction_id, int32 hostile_faction_id){
		hostile_factions[faction_id].push_back(hostile_faction_id);
	}
	void AddFriendlyFaction(int32 faction_id, int32 friendly_faction_id){
		friendly_factions[faction_id].push_back(friendly_faction_id);
	}
	vector<int32>* GetFriendlyFactions(int32 faction_id){
		if(friendly_factions.count(faction_id) > 0)
			return &friendly_factions[faction_id];
		else
			return 0;
	}
	vector<int32>* GetHostileFactions(int32 faction_id){
		if(hostile_factions.count(faction_id) > 0)
			return &hostile_factions[faction_id];
		else
			return 0;
	}
	const char* GetFactionNameByID(int32 faction_id) {
		if (faction_id > 0 && global_faction_list.count(faction_id) > 0)
			return global_faction_list[faction_id]->name.c_str();
		return 0;
	}
private:
	map<int32, vector<int32> > friendly_factions;
	map<int32, vector<int32> > hostile_factions;
	map<int32,Faction*> global_faction_list;
	map<string,Faction*> faction_name_list;
};

class PlayerFaction{
public:
	PlayerFaction();
	sint32		GetMaxValue(sint8 con);
	sint32		GetMinValue(sint8 con);
	EQ2Packet*	FactionUpdate(int16 version);
	sint32		GetFactionValue(int32 faction_id);
	bool		ShouldIncrease(int32 faction_id);
	bool		ShouldDecrease(int32 faction_id);
	bool		IncreaseFaction(int32 faction_id, int32 amount = 0);
	bool		DecreaseFaction(int32 faction_id, int32 amount = 0);
	bool		SetFactionValue(int32 faction_id, sint32 value);
	sint8		GetCon(int32 faction_id);
	int8		GetPercent(int32 faction_id);
	map<int32, sint32>* GetFactionValues(){
		return &faction_values;
	}
	bool		ShouldAttack(int32 faction_id);

private:
	Mutex					MFactionUpdateNeeded;
	vector<int32>			faction_update_needed;
	map<int32, sint32>		faction_values;
	map<int32, int8>		faction_percent;
};
#endif

