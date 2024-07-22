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
#ifndef MUTEXVECTOR_H
#define MUTEXVECTOR_H
#include <vector>
#include "MutexHelper.h"

#define MUTEXVECTOR_PENDING_ADD		1
#define MUTEXVECTOR_PENDING_REMOVE	2
#define MUTEXVECTOR_PENDING_DELETE	3

template <typename T>
class MutexVector{
public:	
	MutexVector(){
		has_pending_data = false;
		pending_clear = false;
		changing = false;
		access_count = 0;
	}
	~MutexVector(){

	}
	class iterator {
	private:
		typename std::vector<T>::iterator itr; // Current element
		MutexVector<T>* vector;
		bool first_itr;
	public:	
		iterator(){
		}
		iterator(MutexVector<T>* vector){	
			if(vector){
				this->vector = vector;
				vector->update();
				++this->vector->access_count;
				first_itr = true;
				itr = vector->current_data.begin();
				if(itr != vector->current_data.end())
					value = *itr;
			}
			else
				this->vector = 0;
		}		
		~iterator(){
			if(vector)
				--vector->access_count;
		}

		bool HasNext(){
			return itr != vector->current_data.end();
		}

		bool Next(){
			if(vector->pending_clear)
				return false;
			if(first_itr)
				first_itr = false;
			else
				itr++;
			if(itr != vector->current_data.end()){
				value = *itr;
				if(vector->pending_data.count(value) > 0 && vector->pending_data[value] == false) //pending delete
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
	void update(){
		//if(access_count > 5)
		//	cout << "Possible error.\n";
		while(changing){
			Sleep(1);
		}
		if(pending_clear && access_count == 0){
			changing = true;
			while(access_count > 0){
				Sleep(1);
			}
			++access_count;
			current_data.clear();
			has_pending_data = (pending_data.size() > 0);
			pending_clear = false;
			--access_count;
			changing = false;
		}
		if(!pending_clear && has_pending_data && access_count == 0){
			changing = true;
			while(access_count > 0){
				Sleep(1);
			}
			++access_count;
			typename std::map<T, bool>::iterator pending_itr;
			for(pending_itr = pending_data.begin(); pending_itr != pending_data.end(); pending_itr++){
				if(pending_itr->second)
					current_data.push_back(pending_itr->first);
				else{
					typename std::vector<T>::iterator data_itr;
					for(data_itr = current_data.begin(); data_itr != current_data.end(); data_itr++){
						if(*data_itr == pending_itr->first){
							current_data.erase(data_itr);
							break;
						}
					}
				}
			}
			pending_data.clear();
			has_pending_data = false;
			--access_count;
			changing = false;		
		}
		handle_deletes.CheckDeletes();
	}
	unsigned int size(bool include_pending = false){
		if(include_pending)
			return current_data.size() + pending_data.size();
		return current_data.size();
	}
	iterator begin(){
		return iterator(this);
	}
	void clear(){
		pending_clear = true;
		update();
	}

	unsigned int count(T key){
		unsigned int ret = 0;
		while(changing){
			Sleep(1);
		}
		++access_count;
		typename std::list<T>::iterator iter;
		for(iter = current_data.begin(); iter != current_data.end(); iter++){
			if(*iter == key)
				ret++;
		}
		--access_count;
		return ret;
	}

	void Remove(T key, bool erase = false, unsigned int erase_time = 0){
		while(changing){
			Sleep(1);
		}
		++access_count;
		pending_data[key] = false;
		if(erase)
			handle_deletes.AddPendingDelete(key, erase_time);
		has_pending_data = true;
		--access_count;
		update();
	}

	void Add(T key){
		while(changing){
			Sleep(1);
		}
		++access_count;
		pending_data[key] = true;
		has_pending_data = true;
		--access_count;
		update();
	}
	T Get(unsigned int index){
		while(changing){
			Sleep(1);
		}
		return current_data[index];
	}
private:
	volatile int access_count;
	std::vector<T> current_data;
	std::map<T, bool> pending_data;
	HandleDeletes<T> handle_deletes;
	volatile bool changing;
	volatile bool has_pending_data;	
	volatile bool pending_clear;
};
#endif
