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
#ifndef CLASSES_CH
#define CLASSES_CH
#include "../common/types.h"
#include <map>
using namespace std;

#define COMMONER		 0
#define FIGHTER			 1
#define WARRIOR			 2
#define GUARDIAN		 3
#define BERSERKER		 4
#define BRAWLER			 5
#define MONK			 6
#define BRUISER			 7
#define CRUSADER		 8
#define SHADOWKNIGHT	 9
#define PALADIN			10
#define PRIEST			11
#define CLERIC			12
#define TEMPLAR			13
#define INQUISITOR		14
#define DRUID			15
#define WARDEN			16
#define FURY			17
#define SHAMAN			18
#define MYSTIC			19
#define DEFILER			20
#define MAGE			21
#define SORCERER		22
#define WIZARD			23
#define WARLOCK			24
#define ENCHANTER		25
#define ILLUSIONIST		26
#define COERCER			27
#define SUMMONER		28
#define CONJUROR		29
#define NECROMANCER		30
#define SCOUT			31
#define ROGUE			32
#define SWASHBUCKLER	33
#define BRIGAND			34
#define BARD			35
#define TROUBADOR		36
#define DIRGE			37
#define PREDATOR		38
#define RANGER			39
#define ASSASSIN		40
#define ANIMALIST		41
#define BEASTLORD		42
#define SHAPER          43
#define CHANNELER       44

//Tradeskills
// 0 - transmuting/tinkering
#define ARTISAN			45 // 1
#define CRAFTSMAN		46 // 2
#define PROVISIONER		47 // 3
#define WOODWORKER		48 // 4
#define CARPENTER		49 // 5
#define OUTFITTER		50 // 6
#define ARMORER			51 // 7
#define WEAPONSMITH		52 // 8
#define TAILOR			53 // 9
#define SCHOLAR			54 // 10
#define JEWELER			55 // 11
#define SAGE			56 // 12
#define ALCHEMIST		57 // 13
//43 - artisan
	//44 - craftsman
	//45 - provisioner
	//46 - Woodworker
	//47 - carpenter
	//48 - armorer
	//49 - weaponsmith
	//50 - tailor
	//51 - 
	//52 - jeweler
	//53 - sage
	//54 - alch
#define CLASSIC_MAX_ADVENTURE_CLASS 40 // there is a 41, but its 'scantestbase'
#define CLASSIC_MAX_TRADESKILL_CLASS 13
#define MAX_CLASSES		58

class Classes {
public:
	Classes();
	char* GetEQClassName(int8 class_, int8 level);
	const char* GetClassName(int8 class_id);
	string GetClassNameCase(int8 class_id);
	sint8 GetClassID(const char* name);
	int8 GetBaseClass(int8 class_id);
	int8 GetSecondaryBaseClass(int8 class_id);
	int8 GetTSBaseClass(int8 class_id);
	int8 GetSecondaryTSBaseClass(int8 class_id);

private:
	map<string, int8> class_map;
};
#endif

