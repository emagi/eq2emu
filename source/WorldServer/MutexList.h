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
#ifndef MUTEXLIST_H
#define MUTEXLIST_H
#include <list>
#include "MutexHelper.h"

#define MUTEXLIST_PENDING_ADD		1
#define MUTEXLIST_PENDING_REMOVE	2
#define MUTEXLIST_PENDING_DELETE	3

template <typename T>
class MutexList{
public:	
	MutexList(){
		pending_changing = false;
		has_pending_data = false;
		pending_clear = false;
		changing = false;
		access_count = 0;
		access_pending = 0;
	}
	MutexList(const MutexList& list){
		pending_changing = false;
		has_pending_data = false;
		pending_clear = false;
		changing = false;
		access_count = 0;
		access_pending = 0;
		/*if(list.has_pending_data)
			pending_data = list.pending_data;
		current_data = list.current_data;		*/
	}
	~MutexList(){
		while(!update(true)){
			Sleep(1);
		}
	}
	class iterator {
	private:
		typename std::list<T>::iterator itr; // Current element
		MutexList<T>* list;
		bool first_itr;
	public:	
		iterator(){
		}
		iterator(MutexList<T>* list){	
			if(list){
				this->list = list;
				list->update();
				this->list->AddAccess();
				first_itr = true;
				itr = list->current_data.begin();
				if(itr != list->current_data.end())
					value = *itr;
			}
			else
				this->list = 0;
		}		
		~iterator(){
			if(list)
				list->RemoveAccess();
		}

		bool HasNext(){
			return itr != list->current_data.end();
		}

		bool Next(){
			if(list->pending_clear)
				return false;
			if(first_itr)
				first_itr = false;
			else
				itr++;
			if(itr != list->current_data.end()){
				value = *itr;
				if(list->PendingContains(value)) //pending delete
					return Next();
				return true;
			}
			return false;
		}
		iterator* operator->() { 
		  return this;
		}
		T value;			
};
	void SetChanging(){
		ChangingLock.lock();
		changing = true;
		ChangingLock.unlock();
	}

	void SetNotChanging(){
		ChangingLock.lock();
		changing = false;
		ChangingLock.unlock();
	}

	void AddAccess(){
		AccessLock.lock();
		++access_count;
		AccessLock.unlock();
	}

	void RemoveAccess(){
		AccessLock.lock();
		--access_count;
		AccessLock.unlock();
	}

	unsigned int size(bool include_pending = false){
		if(include_pending){
			update();
			return current_data.size() + pending_data.size();
		}
		return current_data.size();
	}
	iterator begin(){
		return iterator(this);
	}
	void clear(bool erase_all = false){
		pending_clear = true;
		if(erase_all){
			AddAccess();
			PendingLock.lock();
			typename std::list<T>::iterator itr;
			for(itr = current_data.begin(); itr != current_data.end(); itr++){
				RemoveData(*itr);
			}
			PendingLock.unlock();
			RemoveAccess();
		}
		update();
	}

	bool PendingContains(T key){
		if(!has_pending_data)
			return false;
		bool ret = false;
		PendingLock.lock();
		ret = (pending_data.count(key) > 0 && pending_data[key] == false);
		PendingLock.unlock();
		return ret;
	}

	unsigned int count(T key){
		unsigned int ret = 0;
		while(changing){
			Sleep(1);
		}
		AddAccess();
		bool retry = false;
		if(!changing){
			typename std::list<T>::iterator iter;
			for(iter = current_data.begin(); iter != current_data.end(); iter++){
				if(*iter == key)
					ret++;
			}
		}
		else
			retry = true;
		RemoveAccess();
		if(retry)
			return count(key); //only occurs whenever we change to changing state at the same time as a reading state
		return ret;
	}

	void RemoveData(T key, int32 erase_time = 0){
		handle_deletes.AddPendingDelete(key, Timer::GetCurrentTime2() + erase_time);
	}

	void Remove(T key, bool erase = false, int32 erase_time = 0){
		while(changing){
			Sleep(1);
		}
		AddAccess();
		PendingLock.lock();
		pending_data[key] = false;
		PendingLock.unlock();
		if(erase)
			RemoveData(key, erase_time);
		has_pending_data = true;
		RemoveAccess();
		update();
	}

	void Add(T key){
		if(count(key) > 0)
			return;
		while(changing){
			Sleep(1);
		}
		AddAccess();
		PendingLock.lock();
		pending_data[key] = true;
		PendingLock.unlock();
		has_pending_data = true;
		RemoveAccess();
		update();
	}
private:
	bool update(bool force = false){
		//if(access_count > 5)
		//	cout << "Possible error.\n";
		while(changing){
			Sleep(1);
		}		
		if(pending_clear && access_count == 0){
			SetChanging();
			while(access_count > 0){
				Sleep(1);
			}
			AddAccess();		
			PendingLock.lock();
			current_data.clear();
			has_pending_data = (pending_data.size() > 0);
			PendingLock.unlock();
			pending_clear = false;
			RemoveAccess();
			SetNotChanging();
		}
		if(!pending_clear && has_pending_data && access_count == 0){
			SetChanging();
			while(access_count > 0){
				Sleep(1);
			}
			AddAccess();
			PendingLock.lock();
			typename std::map<T, bool>::iterator pending_itr;
			for(pending_itr = pending_data.begin(); pending_itr != pending_data.end(); pending_itr++){
				if(pending_itr->second)
					current_data.push_back(pending_itr->first);
				else
					current_data.remove(pending_itr->first);
			}
			pending_data.clear();
			PendingLock.unlock();
			has_pending_data = false;
			RemoveAccess();
			SetNotChanging();
		}
		handle_deletes.CheckDeletes(force);
		return !pending_clear && !has_pending_data;
	}
	Locker PendingLock;
	Locker AccessLock;
	Locker ChangingLock;
	volatile int access_count;	
	std::list<T> current_data;
	std::map<T, bool> pending_data;
	HandleDeletes<T> handle_deletes;
	volatile int access_pending;
	volatile bool pending_changing;
	volatile bool changing;
	volatile bool has_pending_data;	
	volatile bool pending_clear;
};
#endif
