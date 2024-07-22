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
#include "Factions.h"
#include "client.h"

extern MasterFactionList master_faction_list;
extern ConfigReader configReader;


PlayerFaction::PlayerFaction(){
	MFactionUpdateNeeded.SetName("PlayerFaction::MFactionUpdateNeeded");
}

sint32 PlayerFaction::GetMaxValue(sint8 con){
	if(con < 0)
		return con * 10000;
	else
		return (con * 10000) + 9999;
}

sint32 PlayerFaction::GetMinValue(sint8 con){
	if(con <= 0)
		return (con * 10000) - 9999;
	else
		return (con * 10000);
}

bool PlayerFaction::ShouldAttack(int32 faction_id){
	return (GetCon(faction_id) <= -4);
}

sint8 PlayerFaction::GetCon(int32 faction_id){
	if(faction_id <= 10){
		if(faction_id == 0)
			return 0;
		return (faction_id-5);
	}

	sint32 value = GetFactionValue(faction_id);
	if(value >= -9999 && value <= 9999)
		return 0;
	else{
		if(value>= 40000)
			return 4;
		else if(value <= -40000)
			return -4;
		return (sint8)(value/10000);
	}
}

int8 PlayerFaction::GetPercent(int32 faction_id){
	if(faction_id <= 10)
		return 0;
	sint8 con = GetCon(faction_id);
	sint32 value = GetFactionValue(faction_id);
	if(con != 0){
		if(value <= 0)
			value *= -1;
		if(con < 0)
			con *= -1;
		value -= con * 10000;
		value *= 100;
		return value / 10000;
	}
	else{
		value += 10000;
		value *= 100;
		return value / 20000;
	}
}

EQ2Packet* PlayerFaction::FactionUpdate(int16 version){
	EQ2Packet* ret = 0;
	Faction* faction = 0;
	PacketStruct* packet = configReader.getStruct("WS_FactionUpdate", version);
	MFactionUpdateNeeded.lock();
	if(packet){
		packet->setArrayLengthByName("num_factions", faction_update_needed.size());
		for(int32 i=0;i<faction_update_needed.size();i++){
			faction = master_faction_list.GetFaction(faction_update_needed[i]);
			if(faction){
				packet->setArrayDataByName("faction_id", faction->id, i);
				packet->setArrayDataByName("name", faction->name.c_str(), i);
				packet->setArrayDataByName("description", faction->description.c_str(), i);
				packet->setArrayDataByName("category", faction->type.c_str(), i);
				packet->setArrayDataByName("con", GetCon(faction->id), i);
				packet->setArrayDataByName("percentage", GetPercent(faction->id), i);
				packet->setArrayDataByName("value", GetFactionValue(faction->id), i);
			}
		}
		ret = packet->serialize();
		safe_delete(packet);
	}
	faction_update_needed.clear();
	MFactionUpdateNeeded.unlock();
	return ret;
}

sint32 PlayerFaction::GetFactionValue(int32 faction_id){
	if(faction_id <= 10)
		return 0;

	//devn00b: This always seems to return 1, even if the player infact has no faction. since we handle this via a check in zoneserver.cpp (processfaction)
	//if(faction_values.count(faction_id) == 0)
	//return master_faction_list.GetDefaultFactionValue(faction_id); //faction_values[faction_id] = master_faction_list.GetDefaultFactionValue(faction_id);

	return faction_values[faction_id];
}

bool PlayerFaction::ShouldIncrease(int32 faction_id){
	if(faction_id <= 10 || master_faction_list.GetIncreaseAmount(faction_id) == 0)
		return false;
	return true;
}

bool PlayerFaction::ShouldDecrease(int32 faction_id){
	if(faction_id <= 10 || master_faction_list.GetDecreaseAmount(faction_id) == 0)
		return false;
	return true;
}

bool PlayerFaction::IncreaseFaction(int32 faction_id, int32 amount){
	if(faction_id <= 10)
		return true;
	bool ret = true;
	if(amount == 0)
		amount = master_faction_list.GetIncreaseAmount(faction_id);
	faction_values[faction_id] += amount;
	if(faction_values[faction_id] >= 50000){
		faction_values[faction_id] = 50000;
		ret = false;
	}
	MFactionUpdateNeeded.lock();
	faction_update_needed.push_back(faction_id);
	MFactionUpdateNeeded.unlock();
	return ret;
}

bool PlayerFaction::DecreaseFaction(int32 faction_id, int32 amount){
	if(faction_id <= 10)
		return true;
	bool ret = true;
	if(amount == 0)
		amount = master_faction_list.GetDecreaseAmount(faction_id);
	if(amount != 0){
		faction_values[faction_id] -= amount;
		if(faction_values[faction_id] <= -50000){
			faction_values[faction_id] = -50000;
			ret = false;
		}
	}
	else
		ret = false;
	MFactionUpdateNeeded.lock();
	faction_update_needed.push_back(faction_id);
	MFactionUpdateNeeded.unlock();
	return ret;
}

bool PlayerFaction::SetFactionValue(int32 faction_id, sint32 value){
	faction_values[faction_id] = value;
	MFactionUpdateNeeded.lock();
	faction_update_needed.push_back(faction_id);
	MFactionUpdateNeeded.unlock();
	return true;
}
