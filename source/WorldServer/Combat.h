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
#ifndef __EQ2_COMBAT_H__
#define __EQ2_COMBAT_H__
#include "Player.h"
#include "../common/timer.h"
#include "NPC_AI.h"
#include "MutexList.h"

#define COMBAT_NORMAL_FIGHTER	0
#define COMBAT_ADD_FIGHTER		1
#define COMBAT_REMOVE_FIGHTER	2
// replace with rule rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat()
//#define MAX_COMBAT_RANGE		3

class ZoneServer;
class SpellProcess;
class LuaSpell;


#endif

