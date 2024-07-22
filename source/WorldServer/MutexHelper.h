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
#ifndef MUTEXHELPER_H
#define MUTEXHELPER_H

#include "../common/timer.h"
#include "../common/Mutex.h"
#include <list>
#include <map>

template<typename T>
class IsPointer { 
public:
	static bool ValidPointer(T key){
		return false;
	}

	static void Delete(T key){
	}
};

class Locker{
public:
	Locker(){
		#ifdef WIN32
			InitializeCriticalSection(&CSMutex);
		#else
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
			pthread_mutex_init(&CSMutex, &attr);
			pthread_mutexattr_destroy(&attr);
		#endif
	}
	Locker(const Locker& locker){
		#ifdef WIN32
			InitializeCriticalSection(&CSMutex);
		#else
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
			pthread_mutex_init(&CSMutex, &attr);
			pthread_mutexattr_destroy(&attr);
		#endif
	}
	~Locker(){
		#ifdef WIN32
			DeleteCriticalSection(&CSMutex);
		#else
		//	pthread_mutex_destroy(&CSMutex);
		#endif
	}
	void lock(){
		#ifdef WIN32
			EnterCriticalSection(&CSMutex);
		#else
			pthread_mutex_lock(&CSMutex);
		#endif
	}
	void unlock(){
		#ifdef WIN32
			LeaveCriticalSection(&CSMutex);
		#else
			pthread_mutex_unlock(&CSMutex);
		#endif
	}

private:
	#ifdef WIN32
		CRITICAL_SECTION CSMutex;
	#else
		pthread_mutex_t CSMutex;
	#endif
};

template<typename T>
class IsPointer<T*> { 
public:
	static bool ValidPointer(T* key){
		return true;
	}
	static void Delete(T* key){
		if(key){
			delete key;
			key = 0;
		}
	}
};

template <typename KeyT, typename ValueT>
class DeleteData{
public:

	void SetData(int type, KeyT key, ValueT value, unsigned int time){
		this->type = type;
		this->key = key;
		this->value = value;
		this->time = time;
	}

	void DeleteKey(){
		IsPointer<KeyT>::Delete(key);
	}

	void DeleteValue(){
		IsPointer<ValueT>::Delete(value);
	}

	unsigned int GetTime(){
		return time;
	}

	int GetType(){
		return type;
	}
private:
	int type;
	KeyT key;
	ValueT value;
	unsigned int time;	
};

template<typename T>
class HandleDeletes {
public:
	HandleDeletes(){
		access_count = 0;
		next_delete_attempt = 0;
		changing = false;
		has_pending_deletes = false;
	}
	~HandleDeletes(){
		CheckDeletes(true);
	}
	void AddPendingDelete(T value, unsigned int time){
		if(IsPointer<T>::ValidPointer(value)){
			while(changing){
				Sleep(1);
			}
			++access_count;
			pending_deletes[value] = time;
			has_pending_deletes = true;
			--access_count;
		}
	}

	void CheckDeletes(bool force = false){
		while(changing){
			Sleep(1);
		}
		if(has_pending_deletes && (force || (Timer::GetCurrentTime2() > next_delete_attempt && access_count == 0))){
			changing = true;			
			while(access_count > 0){
				Sleep(1);
			}
			++access_count;
			next_delete_attempt = Timer::GetCurrentTime2();
			std::list<T> deletes;
			typename std::map<T, unsigned int>::iterator pending_delete_itr;
			for(pending_delete_itr = pending_deletes.begin(); pending_delete_itr != pending_deletes.end(); pending_delete_itr++){
				if(force || next_delete_attempt >= pending_delete_itr->second)
					deletes.push_back(pending_delete_itr->first);
			}
			if(deletes.size() > 0){
				typename std::list<T>::iterator delete_itr;
				for(delete_itr = deletes.begin(); delete_itr != deletes.end(); delete_itr++){
					IsPointer<T>::Delete(*delete_itr);
					pending_deletes.erase(*delete_itr);
				}
				has_pending_deletes = (pending_deletes.size() > 0);
			}
			next_delete_attempt += 1000;
			--access_count;
			changing = false;
		}
	}

private:
	volatile bool changing;
	volatile int access_count;
	volatile unsigned int next_delete_attempt;
	volatile bool has_pending_deletes;	
	std::map<T, unsigned int> pending_deletes;
};
#endif
