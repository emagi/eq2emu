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
#include "../common/debug.h"
#include "races.h"
#include "../common/MiscFunctions.h"

Races::Races(){
	race_map["BARBARIAN"] = 0;
	race_map["DARKELF"] = 1;
	race_map["DWARF"] = 2;
	race_map["ERUDITE"] = 3;
	race_map["FROGLOK"] = 4;
	race_map["GNOME"] = 5;
	race_map["HALFELF"] = 6;
	race_map["HALFLING"] = 7;
	race_map["HIGHELF"] = 8;
	race_map["HUMAN"] = 9;
	race_map["IKSAR"] = 10;
	race_map["KERRA"] = 11;
	race_map["OGRE"] = 12;
	race_map["RATONGA"] = 13;
	race_map["TROLL"] = 14;
	race_map["WOODELF"] = 15;
	race_map["FAE_LIGHT"] = 16;
	race_map["FAE_DARK"] = 17;
	race_map["SARNAK"] = 18;
	race_map["VAMPIRE"] = 19;
	race_map["AERAKYN"] = 20;

	race_map_friendly[0] = "Barbarian";
	race_map_friendly[1] = "Dark Elf";
	race_map_friendly[2] = "Dwarf";
	race_map_friendly[3] = "Erudite";
	race_map_friendly[4] = "Froglok";
	race_map_friendly[5] = "Gnome";
	race_map_friendly[6] = "Half Elf";
	race_map_friendly[7] = "Halfling";
	race_map_friendly[8] = "High Elf";
	race_map_friendly[9] = "Human";
	race_map_friendly[10] = "Iksar";
	race_map_friendly[11] = "Kerra";
	race_map_friendly[12] = "Ogre";
	race_map_friendly[13] = "Ratonga";
	race_map_friendly[14] = "Troll";
	race_map_friendly[15] = "Wood Elf";
	race_map_friendly[16] = "Fae";
	race_map_friendly[17] = "Arasai";
	race_map_friendly[18] = "Sarnak";
	race_map_friendly[19] = "Vampire";
	race_map_friendly[20] = "Aerakyn";

	// "Neutral" races are in both lists - this is for /randomize RACE
	race_map_good[0]  = "DWARF";
	race_map_good[1]  = "FAE_LIGHT";
	race_map_good[2]  = "FROGLOK";
	race_map_good[3]  = "HALFLING";
	race_map_good[4]  = "HIGHELF";
	race_map_good[5]  = "WOODELF";
	race_map_good[6]  = "BARBARIAN";
	race_map_good[7]  = "ERUDITE";
	race_map_good[8]  = "GNOME";
	race_map_good[9]  = "HALFELF";
	race_map_good[10] = "HUMAN";
	race_map_good[11] = "KERRA";
	race_map_good[12] = "VAMPIRE";
	race_map_good[13] = "AERAKYN";

	race_map_evil[0]  = "FAE_DARK";
	race_map_evil[1]  = "DARKELF";
	race_map_evil[2]  = "IKSAR";
	race_map_evil[3]  = "OGRE";
	race_map_evil[4]  = "RATONGA";
	race_map_evil[5]  = "SARNAK";
	race_map_evil[6]  = "TROLL";
	race_map_evil[7]  = "BARBARIAN";
	race_map_evil[8]  = "ERUDITE";
	race_map_evil[9]  = "GNOME";
	race_map_evil[10] = "HALFELF";
	race_map_evil[11] = "HUMAN";
	race_map_evil[12] = "KERRA";
	race_map_evil[13] = "VAMPIRE";
	race_map_evil[14] = "AERAKYN";
}

sint8 Races::GetRaceID(const char* name){
	string race_name = string(name);
	race_name = ToUpper(race_name);
	if(race_map.count(race_name) == 1)
		return race_map[race_name];
	else
		return -1;
}

const char* Races::GetRaceName(int8 race_id){
	map<string, int8>::iterator itr;
	for(itr = race_map.begin(); itr != race_map.end(); itr++){
		if(itr->second == race_id)
			return itr->first.c_str();
	}
	return 0;
}

const char* Races::GetRaceNameCase(int8 race_id) {
	map<int8, string>::iterator itr;
	for(itr = race_map_friendly.begin(); itr != race_map_friendly.end(); itr++){
		if(itr->first == race_id)
			return itr->second.c_str();
	}
	return 0;
}

int8 Races::GetRaceNameGood() {
	int8 random = MakeRandomInt(0,13); // 12 good races
	map<int8, string>::iterator itr;
	for(itr = race_map_good.begin(); itr != race_map_good.end(); itr++){
		if(itr->first == random)
			return GetRaceID(itr->second.c_str());
	}
	return 9; // default to Human race if error finding another
}

int8 Races::GetRaceNameEvil() {
	int8 random = MakeRandomInt(0,14); // 13 evil races
	map<int8, string>::iterator itr;
	for(itr = race_map_evil.begin(); itr != race_map_evil.end(); itr++){
		if(itr->first == random) 
			return GetRaceID(itr->second.c_str());
	}
	return 9; // default to Human race if error finding another
}
