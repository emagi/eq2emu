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
#ifndef MUTEXMAP_H
#define MUTEXMAP_H
#include <map>
#include "MutexHelper.h"

#define MUTEXMAP_DELETE_TYPE_KEY		1
#define MUTEXMAP_DELETE_TYPE_VALUE		2

template <typename KeyT, typename ValueT>
class MutexMap{
public:	
	MutexMap(){
		has_pending_data = false;
		pending_clear = false;
		changing = false;
		access_count = 0;
		has_pending_deletes = false;
		next_delete_attempt = 0;
		delete_all = false;
	}
	~MutexMap(){
		update(true);
		PendingLock.lock();
		pending_add.clear();
		pending_remove.clear();
		PendingLock.unlock();
	}
	class iterator {
		private:
			typename std::map<KeyT, ValueT>::iterator itr; // Current element
			MutexMap* map;
			bool first_itr;
		public:
			iterator(){
			}
			iterator(MutexMap* map){				
				this->map = map;
				map->update();
				map->SetChanging();
				this->map->AddAccess();
				map->SetNotChanging();
				first_itr = true;
				itr = map->current_data.begin();
				if(itr != map->current_data.end()){
					first = itr->first;
					second = itr->second;
				}
			}		
			~iterator(){
				map->RemoveAccess();
			}

			bool HasNext(){
				return itr != map->current_data.end();
			}

			bool Next(){
				if(map->pending_clear)
					return false;
				if(first_itr)
					first_itr = false;
				else
					itr++;
				if(itr != map->current_data.end()){
					first = itr->first;
					second = itr->second;
					map->PendingLock.lock();
					if(map->pending_remove.count(first) > 0){
						map->PendingLock.unlock();
						return Next();
					}
					map->PendingLock.unlock();
					return true;
				}
				return false;
			}
			iterator* operator->() { 
			  return this;
			}
			KeyT first;
			ValueT second;			
    };
	int count(KeyT key, bool include_pending = false){
		while(changing){
			Sleep(1);
		}
		AddAccess();
		int ret = current_data.count(key);
		if(include_pending){
			PendingLock.lock();
			ret += pending_add.count(key);
			PendingLock.unlock();
		}
		RemoveAccess();
		return ret;
	}
	void clear(bool delete_all = false){
		pending_clear = true;
		if(delete_all){
			SetChanging();
			while(access_count > 0){
				Sleep(1);
			}
			AddAccess();
			PendingLock.lock();
			typename std::map<KeyT, ValueT>::iterator itr;
			for(itr = current_data.begin(); itr != current_data.end(); itr++){
				deleteData(itr->first, MUTEXMAP_DELETE_TYPE_VALUE);
			}
			PendingLock.unlock();
			RemoveAccess();
			SetNotChanging();
		}
		update();
	}
	unsigned int size(bool include_pending = false){
		if(include_pending)
			return current_data.size() + pending_add.size();
		return current_data.size();
	}
	void deleteData(KeyT key, int8 type, int32 erase_time = 0){
		DeleteData<KeyT, ValueT>* del = new DeleteData<KeyT, ValueT>();
		del->SetData(type, key, current_data[key], Timer::GetCurrentTime2() + erase_time);
		pending_deletes[del] = true;
		has_pending_deletes = true;
	}
	void erase(KeyT key, bool erase_key = false, bool erase_value = false, int32 erase_time = 0){
		while(changing){
			Sleep(1);
		}
		AddAccess();
		if(current_data.count(key) != 0){
			PendingLock.lock();
			pending_remove[key] = true;			
			if(erase_key || erase_value){
				int type = 0;			
				if(erase_key)
					type = MUTEXMAP_DELETE_TYPE_KEY;
				if(erase_value)
					type += MUTEXMAP_DELETE_TYPE_VALUE;
				deleteData(key, type, erase_time);
			}		
			has_pending_data = true;
			PendingLock.unlock();
		}
		RemoveAccess();
		update();
	}
	iterator begin(){
		return iterator(this); 
	}
	void Put(KeyT key, ValueT value){
		while(changing){
			Sleep(1);
		}
		AddAccess();
		PendingLock.lock();
		pending_add[key] = value;
		has_pending_data = true;
		PendingLock.unlock();
		RemoveAccess();
		update();
	}
	ValueT& Get(KeyT key){
		while(changing){
			Sleep(1);
		}
		AddAccess();
		if(current_data.count(key) > 0 || pending_add.count(key) == 0){
			RemoveAccess();
			return current_data[key];
		}
		RemoveAccess();
		return pending_add[key];
	}
private:
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

	void SetChanging(){		
		ChangingLock.lock();
		changing = true;
	}

	void SetNotChanging(){
		changing = false;
		ChangingLock.unlock();
	}

	void update(bool force = false){
		if(pending_clear && (force || access_count == 0)){
			SetChanging();
			while(access_count > 0){
				Sleep(1);
			}
			AddAccess();
			PendingLock.lock();
			current_data.clear();
			has_pending_data = (pending_add.size() > 0 || pending_remove.size() > 0);
			pending_clear = false;
			PendingLock.unlock();
			RemoveAccess();
			SetNotChanging();
		}
		if(!pending_clear && has_pending_data && (force || access_count == 0)){
			SetChanging();
			while(access_count > 0){
				Sleep(1);
			}
			AddAccess();
			PendingLock.lock();
			typename std::map<KeyT, bool>::iterator remove_itr;
			for(remove_itr = pending_remove.begin(); remove_itr != pending_remove.end(); remove_itr++){
				current_data.erase(remove_itr->first);
			}
			typename std::map<KeyT, ValueT>::iterator add_itr;
			for(add_itr = pending_add.begin(); add_itr != pending_add.end(); add_itr++){
				current_data[add_itr->first] = add_itr->second;
			}
			pending_add.clear();
			pending_remove.clear();
			has_pending_data = false;
			PendingLock.unlock();
			RemoveAccess();
			SetNotChanging();			
		}
		if(has_pending_deletes && (force || (Timer::GetCurrentTime2() > next_delete_attempt && access_count == 0))){
			SetChanging();
			while(access_count > 0){
				Sleep(1);
			}
			AddAccess();
			PendingLock.lock();
			unsigned int time = Timer::GetCurrentTime2();
			typename std::list<DeleteData<KeyT, ValueT>*> deleteData;
			typename std::map<DeleteData<KeyT, ValueT>*, bool>::iterator remove_itr;
			for(remove_itr = pending_deletes.begin(); remove_itr != pending_deletes.end(); remove_itr++){
				if(force || time >= remove_itr->first->GetTime())
					deleteData.push_back(remove_itr->first);
			}
			DeleteData<KeyT, ValueT>* data = 0;
			typename std::list<DeleteData<KeyT, ValueT>*>::iterator remove_data_itr;
			for(remove_data_itr = deleteData.begin(); remove_data_itr != deleteData.end(); remove_data_itr++){
				data = *remove_data_itr;				
				if((data->GetType() & MUTEXMAP_DELETE_TYPE_KEY) == MUTEXMAP_DELETE_TYPE_KEY){
					data->DeleteKey();
				}
				if((data->GetType() & MUTEXMAP_DELETE_TYPE_VALUE) == MUTEXMAP_DELETE_TYPE_VALUE){
					data->DeleteValue();
				}
				pending_deletes.erase(data);
				delete data;
			}
			next_delete_attempt = Timer::GetCurrentTime2() + 1000;
			PendingLock.unlock();
			RemoveAccess();
			SetNotChanging();
			has_pending_deletes = (pending_deletes.size() > 0);
		}
	}
	Locker PendingLock;
	Locker AccessLock;
	Locker ChangingLock;
	std::map<KeyT, ValueT> current_data;
	std::map<KeyT, ValueT> pending_add;
	std::map<DeleteData<KeyT, ValueT>*, bool > pending_deletes;
	std::map<KeyT, bool> pending_remove;
	volatile unsigned int next_delete_attempt;
	volatile int access_count;
	volatile bool delete_all;
	volatile bool changing;
	volatile bool has_pending_data;	
	volatile bool has_pending_deletes;	
	volatile bool pending_clear;
};
#endif
