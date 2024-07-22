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
#ifndef __EQ2_GroundSpawn__
#define __EQ2_GroundSpawn__

#include "Spawn.h"
#include "client.h"
#include "../common/Mutex.h"

class GroundSpawn : public Spawn {
public:
	GroundSpawn();
	virtual ~GroundSpawn();
	GroundSpawn*	Copy(){
		GroundSpawn* new_spawn = new GroundSpawn();
		new_spawn->size = size;
		new_spawn->SetPrimaryCommands(&primary_command_list);
		new_spawn->SetSecondaryCommands(&secondary_command_list);
		new_spawn->database_id = database_id;
		new_spawn->primary_command_list_id = primary_command_list_id;
		new_spawn->secondary_command_list_id = secondary_command_list_id;
		memcpy(&new_spawn->appearance, &appearance, sizeof(AppearanceData));
		new_spawn->faction_id = faction_id;
		new_spawn->target = 0;
		new_spawn->SetTotalHP(GetTotalHP());
		new_spawn->SetTotalPower(GetTotalPower());
		new_spawn->SetHP(GetHP());
		new_spawn->SetPower(GetPower());
		new_spawn->SetNumberHarvests(number_harvests);
		new_spawn->SetAttemptsPerHarvest(num_attempts_per_harvest);
		new_spawn->SetGroundSpawnEntryID(groundspawn_id);
		new_spawn->SetCollectionSkill(collection_skill.c_str());
		SetQuestsRequired(new_spawn);
		new_spawn->forceMapCheck = forceMapCheck;
		new_spawn->SetOmittedByDBFlag(IsOmittedByDBFlag());
		new_spawn->SetLootTier(GetLootTier());
		new_spawn->SetLootDropType(GetLootDropType());
		new_spawn->SetRandomizeHeading(GetRandomizeHeading());
		return new_spawn;
	}
	bool IsGroundSpawn(){ return true; }
	EQ2Packet* serialize(Player* player, int16 version);
	int8 GetNumberHarvests();
	void SetNumberHarvests(int8 val);
	int8 GetAttemptsPerHarvest();
	void SetAttemptsPerHarvest(int8 val);
	int32 GetGroundSpawnEntryID();
	void SetGroundSpawnEntryID(int32 val);
	void ProcessHarvest(Client* client);
	void SetCollectionSkill(const char* val);
	const char* GetCollectionSkill();
	string GetHarvestMessageName(bool present_tense = false, bool failure = false);
	string GetHarvestSpellType();
	string GetHarvestSpellName();
	void HandleUse(Client* client, string type);
	
	void SetRandomizeHeading(bool val) { randomize_heading = val; }
	bool GetRandomizeHeading() { return randomize_heading; }
private:
	int8	number_harvests;
	int8	num_attempts_per_harvest;
	int32	groundspawn_id;
	string	collection_skill;
	Mutex	MHarvest;
	Mutex	MHarvestUse;
	bool 	randomize_heading;
};
#endif

