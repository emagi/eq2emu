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
#ifndef WIN32
#ifndef __UNIX_H__
#define __UNIX_H__
	#ifndef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
		#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP {0, 0, 0, PTHREAD_MUTEX_RECURSIVE_NP, __LOCK_INITIALIZER}
	#endif
#include <unistd.h>

typedef int SOCKET;

void Sleep(unsigned int x);
char* strupr(char* tmp);
char* strlwr(char* tmp);
#endif
#endif
