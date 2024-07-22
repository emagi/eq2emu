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

#include <assert.h>
#include <string.h>
#include "Languages.h"

Language::Language(){
	id = 0;
	memset(name, 0, sizeof(name));
	save_needed = false;
}

Language::Language(Language* language){
	id = language->id;
	strncpy(name, language->GetName(), sizeof(name));
	save_needed = language->save_needed;
}

MasterLanguagesList::MasterLanguagesList(){
}

MasterLanguagesList::~MasterLanguagesList(){
	Clear();
}


// don't bother calling this beyond its deconstructor its not thread-safe
void MasterLanguagesList::Clear(){
	list<Language*>::iterator itr;
	Language* language = 0;
	for(itr = languages_list.begin(); itr != languages_list.end(); itr++){
		language = *itr;
		safe_delete(language);
	}
	languages_list.clear();
}

int32 MasterLanguagesList::Size(){
	return languages_list.size();
}

void MasterLanguagesList::AddLanguage(Language* language){
	assert(language);
	languages_list.push_back(language);
}

Language* MasterLanguagesList::GetLanguage(int32 id){
	list<Language*>::iterator itr;
	Language* language = 0;
	Language* ret = 0;
	for(itr = languages_list.begin(); itr != languages_list.end(); itr++){
		language = *itr;
		if(language->GetID() == id){
			ret = language;
			break;
		}
	}
	return ret;
}

Language* MasterLanguagesList::GetLanguageByName(const char* name){
	list<Language*>::iterator itr;
	Language* language = 0;
	Language* ret = 0;
	for(itr = languages_list.begin(); itr != languages_list.end(); itr++){
		language = *itr;
		if(!language)
			continue;
		if(!strncmp(language->GetName(), name, strlen(name))){
			ret = language;
			break;
		}
	}
	return ret;
}

list<Language*>* MasterLanguagesList::GetAllLanguages(){
	return &languages_list;
}

PlayerLanguagesList::PlayerLanguagesList(){
}

PlayerLanguagesList::~PlayerLanguagesList(){
}

void PlayerLanguagesList::Clear() {
	list<Language*>::iterator itr;
	Language* language = 0;
	Language* ret = 0;
	for(itr = player_languages_list.begin(); itr != player_languages_list.end(); itr++){
		language = *itr;
		safe_delete(language);
	}
	
	player_languages_list.clear();
}

void PlayerLanguagesList::Add(Language* language){
	player_languages_list.push_back(language);
}

Language* PlayerLanguagesList::GetLanguage(int32 id){
	list<Language*>::iterator itr;
	Language* language = 0;
	Language* ret = 0;
	for(itr = player_languages_list.begin(); itr != player_languages_list.end(); itr++){
		language = *itr;
		if(language->GetID() == id){
			ret = language;
			break;
		}
	}
	return ret;
}

Language* PlayerLanguagesList::GetLanguageByName(const char* name){
	list<Language*>::iterator itr;
	Language* language = 0;
	Language* ret = 0;
	for(itr = player_languages_list.begin(); itr != player_languages_list.end(); itr++){
		language = *itr;
		if(!language)
			continue;
		if(!strncmp(language->GetName(), name, strlen(name))){
			ret = language;
			break;
		}
	}
	return ret;
}

list<Language*>* PlayerLanguagesList::GetAllLanguages(){
	return &player_languages_list;
}
