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

#include "LogTypes.h"

#ifndef VERSION_H
#define VERSION_H

#define CURRENT_DATABASE_MINORVERSION 43
#define CURRENT_DATABASE_MAJORVERSION 730
#if defined(LOGIN)
	#define EQ2EMU_MODULE	"EQ2EMu LoginServer"
#elif defined(PATCHER)
	#define EQ2EMU_MODULE	"EQ2EMu PatchServer"
#elif defined(CHAT)
	#define EQ2EMU_MODULE	"EQ2EMu ChatServer"
#elif defined(ZONE)
	#define EQ2EMU_MODULE	"EQ2EMu ZoneServer"
#else
	#define EQ2EMU_MODULE	"EQ2EMu WorldServer"
#endif

#if defined(LOGIN)
#define CURRENT_VERSION	"0.9.7-thetascorpii-DR2"
#elif defined(WORLD)
#define CURRENT_VERSION	"0.9.7-thetascorpii-DR2"
#else
#define CURRENT_VERSION	"0.9.7-thetascorpii-DR2"
#endif

#define COMPILE_DATE	__DATE__
#define COMPILE_TIME	__TIME__
#ifndef WIN32
	#define LAST_MODIFIED	__TIME__
#else
	#define LAST_MODIFIED	__TIMESTAMP__
#endif

#endif