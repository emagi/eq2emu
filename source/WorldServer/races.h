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
#ifndef RACES_H
#define RACES_H
#include "../common/types.h"
#include <map>
using namespace std;

#define BARBARIAN				0
#define DARK_ELF				1
#define DWARF					2
#define ERUDITE					3
#define	FROGLOK					4
#define GNOME					5
#define HALF_ELF				6
#define HALFLING				7
#define HIGH_ELF				8
#define	HUMAN					9
#define IKSAR				   10
#define KERRA				   11
#define OGRE				   12
#define RATONGA				   13
#define TROLL				   14
#define WOOD_ELF			   15
#define FAE					   16
#define ARASAI				   17
#define SARNAK				   18
#define VAMPIRE				   19
#define AERAKYN				   20

class Races {
public:
	Races();
	const char* GetRaceName(int8 race_id);
	const char* GetRaceNameCase(int8 race_id);
	int8 GetRaceNameGood();
	int8 GetRaceNameEvil();
	sint8 GetRaceID(const char* name);
private:
	map<string, int8> race_map;
	map<int8, string> race_map_friendly;
	map<int8, string> race_map_good;
	map<int8, string> race_map_evil;
};
#endif
