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

#include "emu_opcodes.h"

const char *OpcodeNames[_maxEmuOpcode+1] = {
	"OP_Unknown",
	
//a preprocessor hack so we dont have to maintain two lists
#define N(x) #x
#if !defined(LOGIN)
	#include "emu_oplist.h"
#endif
#ifdef LOGIN
	#include "login_oplist.h"
#endif
#undef N
	
	""
};


