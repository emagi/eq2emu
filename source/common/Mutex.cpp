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
#include "../common/Log.h"
#include "../common/debug.h"
#include "../common/Mutex.h"

Mutex::Mutex() {
	readers = 0;
	mlocked = false;
	writing = false;
	name = "";
#ifdef DEBUG
	stack.clear();
#endif
	//CSLock is a pointer so we can use a different attribute type on create
	CSLock = new CriticalSection(MUTEX_ATTRIBUTE_RECURSIVE);
}

Mutex::~Mutex() {
	safe_delete(CSLock);
#ifdef DEBUG
	stack.clear();
#endif
}

void Mutex::SetName(string in_name) {
#ifdef DEBUG
	name = in_name;
#endif
}

void Mutex::lock() {
#ifdef DEBUG
	int i = 0;
#endif
	if (name.length() > 0) {
		while (mlocked) {
#ifdef DEBUG
			if (i > MUTEX_TIMEOUT_MILLISECONDS) {
				LogWrite(MUTEX__ERROR, 0, "Mutex", "Possible deadlock attempt by '%s'!", name.c_str());
				return;
			}
			i++;
#endif
			Sleep(1);
		}
	}
	mlocked = true;
	CSLock->lock();
}

bool Mutex::trylock() {
	return CSLock->trylock();
}

void Mutex::unlock() {
	CSLock->unlock();
	mlocked = false;
}

void Mutex::readlock(const char* function, int32 line) {
#ifdef DEBUG
	int32 i = 0;
#endif
	while (true) {
		//Loop until there isn't a writer, then we can read!
		CSRead.lock();
		if (!writing) {
			readers++;
			CSRead.unlock();
#ifdef DEBUG
			CSStack.lock();
			if (function)
				stack[(string)function]++;
			CSStack.unlock();
#endif
			return;
		}
		CSRead.unlock();
#ifdef DEBUG
		if (i > MUTEX_TIMEOUT_MILLISECONDS) {
			LogWrite(MUTEX__ERROR, 0, "Mutex", "The mutex %s called from %s at line %u timed out waiting for a readlock!", name.c_str(), function ? function : "name_not_provided", line);
			LogWrite(MUTEX__ERROR, 0, "Mutex", "The following functions had locks:");
			map<string, int32>::iterator itr;
			CSStack.lock();
			for (itr = stack.begin(); itr != stack.end(); itr++) {
				if (itr->second > 0 && itr->first.length() > 0)
					LogWrite(MUTEX__ERROR, 0, "Mutex", "%s, number of locks = %u", itr->first.c_str(), itr->second);
			}
			CSStack.unlock();
			i = 0;
			continue;
		}
		i++;
#endif
		Sleep(1);
	}
}

void Mutex::releasereadlock(const char* function, int32 line) {
	//Wait for the readcount lock
	CSRead.lock();
	//Lower the readcount by one, when readcount is 0 writers may start writing
	readers--;
	CSRead.unlock();
#ifdef DEBUG
	CSStack.lock();
	if (function) {
		map<string, int32>::iterator itr = stack.find((string)function);
		if (itr != stack.end()) {
			if (--(itr->second) == 0) {
				stack.erase(itr);
			}
		}
	}
	CSStack.unlock();
#endif
}

bool Mutex::tryreadlock(const char* function) {
	//This returns true if able to instantly obtain a readlock, false if not
	CSRead.lock();
	if (!writing) {
		readers++;
		CSRead.unlock();
	}
	else {
		CSRead.unlock();
		return false;
	}

#ifdef DEBUG
	CSStack.lock();
	if (function)
		stack[(string)function]++;
	CSStack.unlock();
#endif

	return true;
}

void Mutex::writelock(const char* function, int32 line) {
	//Wait until the writer lock becomes available, then we can be the only writer!
#ifdef DEBUG
	int32 i = 0;
#endif
	while (!CSWrite.trylock()) {
#ifdef DEBUG
		if (i > MUTEX_TIMEOUT_MILLISECONDS) {
			LogWrite(MUTEX__ERROR, 0, "Mutex", "The mutex %s called from %s at line %u timed out waiting on another writelock!", name.c_str(), function ? function : "name_not_provided", line);
			LogWrite(MUTEX__ERROR, 0, "Mutex", "The following functions had locks:");
			map<string, int32>::iterator itr;
			CSStack.lock();
			for (itr = stack.begin(); itr != stack.end(); itr++) {
				if (itr->second > 0 && itr->first.length() > 0)
					LogWrite(MUTEX__ERROR, 0, "Mutex", "%s, number of locks = %u", itr->first.c_str(), itr->second);
			}
			CSStack.unlock();
			i = 0;
			continue;
		}
		i++;
#endif
		Sleep(1);
	}
	waitReaders(function, line);
#ifdef DEBUG
	CSStack.lock();
	if (function)
		stack[(string)function]++;
	CSStack.unlock();
#endif
}

void Mutex::releasewritelock(const char* function, int32 line) {
	//Wait for the readcount lock
	CSRead.lock();
	//Readers are aloud again
	writing = false;
	CSRead.unlock();
	//Allow other writers to write
	CSWrite.unlock();
#ifdef DEBUG
	CSStack.lock();
	if (function) {
		map<string, int32>::iterator itr = stack.find((string)function);
		if (itr != stack.end()) {
			if (--(itr->second) == 0) {
				stack.erase(itr);
			}
		}
	}
	CSStack.unlock();
#endif
}

bool Mutex::trywritelock(const char* function) {
	//This returns true if able to instantly obtain a writelock, false if not
	if (CSWrite.trylock()) {
		CSRead.lock();
		if (readers == 0)
			writing = true;
		CSRead.unlock();
		if (!writing) {
			CSWrite.unlock();
			return false;
		}
	}
	else
		return false;

#ifdef DEBUG
	CSStack.lock();
	if (function)
		stack[(string)function]++;
	CSStack.unlock();
#endif

	return true;
}

void Mutex::waitReaders(const char* function, int32 line)
{
	//Wait for all current readers to stop, then we can write!
#ifdef DEBUG
	int32 i = 0;
#endif
	while (true)
	{
		CSRead.lock();
		if (readers == 0)
		{
			writing = true;
			CSRead.unlock();
			break;
		}
		CSRead.unlock();
#ifdef DEBUG
		if (i > MUTEX_TIMEOUT_MILLISECONDS) {
			LogWrite(MUTEX__ERROR, 0, "Mutex", "The mutex %s called from %s at line %u timed out while waiting on readers!", name.c_str(), function ? function : "name_not_provided", line);
			LogWrite(MUTEX__ERROR, 0, "Mutex", "The following functions had locks:");
			map<string, int32>::iterator itr;
			CSStack.lock();
			for (itr = stack.begin(); itr != stack.end(); itr++) {
				if (itr->second > 0 && itr->first.length() > 0)
					LogWrite(MUTEX__ERROR, 0, "Mutex", "%s, number of locks = %u", itr->first.c_str(), itr->second);
			}
			CSStack.unlock();
			i = 0;
			continue;
		}
		i++;
#endif
		Sleep(1);
	}
}

LockMutex::LockMutex(Mutex* in_mut, bool iLock) {
	mut = in_mut;
	locked = iLock;
	if (locked) {
		mut->lock();
	}
}

LockMutex::~LockMutex() {
	if (locked) {
		mut->unlock();
	}
}

void LockMutex::unlock() {
	if (locked)
		mut->unlock();
	locked = false;
}

void LockMutex::lock() {
	if (!locked)
		mut->lock();
	locked = true;
}

CriticalSection::CriticalSection(int attribute) {
#ifdef WIN32
	InitializeCriticalSection(&CSMutex);
#else
	pthread_mutexattr_init(&type_attribute);
	switch (attribute)
	{
	case MUTEX_ATTRIBUTE_FAST:
		pthread_mutexattr_settype(&type_attribute, PTHREAD_MUTEX_FAST_NP);
		break;
	case MUTEX_ATTRIBUTE_RECURSIVE:
		pthread_mutexattr_settype(&type_attribute, PTHREAD_MUTEX_RECURSIVE_NP);
		break;
	case MUTEX_ATTRIBUTE_ERRORCHK:
		pthread_mutexattr_settype(&type_attribute, PTHREAD_MUTEX_ERRORCHECK_NP);
		break;
	default:
		LogWrite(MUTEX__DEBUG, 0, "Critical Section", "Invalid mutex attribute type! Using PTHREAD_MUTEX_FAST_NP");
		pthread_mutexattr_settype(&type_attribute, PTHREAD_MUTEX_FAST_NP);
		break;
	}
	pthread_mutex_init(&CSMutex, &type_attribute);
#endif
}

CriticalSection::~CriticalSection() {
#ifdef WIN32
	DeleteCriticalSection(&CSMutex);
#else
	pthread_mutex_destroy(&CSMutex);
	pthread_mutexattr_destroy(&type_attribute);
#endif
}

void CriticalSection::lock() {
	//Waits for a lock on this critical section
#ifdef WIN32
	EnterCriticalSection(&CSMutex);
#else
	pthread_mutex_lock(&CSMutex);
#endif
}

void CriticalSection::unlock() {
	//Gets rid of one of the current thread's locks on this critical section
#ifdef WIN32
	LeaveCriticalSection(&CSMutex);
#else
	pthread_mutex_unlock(&CSMutex);
#endif
}

bool CriticalSection::trylock() {
	//Returns true if able to instantly get a lock on this critical section, false if not
#ifdef WIN32
	return TryEnterCriticalSection(&CSMutex);
#else
	return (pthread_mutex_trylock(&CSMutex) == 0);
#endif
}

