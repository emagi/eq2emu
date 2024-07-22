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
#ifndef MYMUTEX_H
#define MYMUTEX_H
#ifdef WIN32
	#include <WinSock2.h>
	#include <windows.h>
#else
	#include <pthread.h>
	#include "../common/unix.h"
#endif	
#include "../common/types.h"
#include <string>
#include <map>

#define MUTEX_ATTRIBUTE_FAST      1
#define MUTEX_ATTRIBUTE_RECURSIVE 2
#define MUTEX_ATTRIBUTE_ERRORCHK  3
#define MUTEX_TIMEOUT_MILLISECONDS     10000

class CriticalSection {
public:
	CriticalSection(int attribute = MUTEX_ATTRIBUTE_FAST);
	~CriticalSection();
	void lock();
	void unlock();
	bool trylock();
private:
#ifdef WIN32
	CRITICAL_SECTION CSMutex;
#else
	pthread_mutex_t CSMutex;
	pthread_mutexattr_t type_attribute;
#endif
};

class Mutex {
public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();
	bool trylock();

	void readlock(const char* function = 0, int32 line = 0);
	void releasereadlock(const char* function = 0, int32 line = 0);
	bool tryreadlock(const char* function = 0);

	void writelock(const char* function = 0, int32 line = 0);
	void releasewritelock(const char* function = 0, int32 line = 0);
	bool trywritelock(const char* function = 0);

	void waitReaders(const char* function = 0, int32 line = 0);

	void SetName(string in_name);
private:
	CriticalSection  CSRead;
	CriticalSection  CSWrite;
	CriticalSection* CSLock;

#ifdef DEBUG	//Used for debugging only
	CriticalSection  CSStack;
	map<string, int32> stack;
#endif

	int readers;
	bool writing;
	volatile bool mlocked;
	string name;
};


class LockMutex {
public:
	LockMutex(Mutex* in_mut, bool iLock = true);
	~LockMutex();
	void unlock();
	void lock();
private:
	bool	locked;
	Mutex*	mut;
};


#endif
