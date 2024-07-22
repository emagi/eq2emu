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
#include "Titles.h"
#include "../common/MiscFunctions.h"

Title::Title(){
	id = 0;
	memset(name, 0, sizeof(name));
	prefix = 0;
	save_needed = false;
}

Title::Title(Title* title){
	id = title->id;
	strncpy(name, title->GetName(), sizeof(name));
	prefix = title->prefix;
	save_needed = title->save_needed;
}

Title::~Title(){
}

MasterTitlesList::MasterTitlesList(){
	MMasterTitleMutex.SetName("MasterTitlesList::MMasterTitleMutex");
}

MasterTitlesList::~MasterTitlesList(){
	Clear();
}

void MasterTitlesList::Clear(){
	MMasterTitleMutex.writelock();
	map<sint32, Title*>::iterator itr;
	for(itr = titles_list.begin(); itr != titles_list.end(); itr++)
		safe_delete(itr->second);
	titles_list.clear();
	MMasterTitleMutex.releasewritelock();
}

void MasterTitlesList::AddTitle(Title* title){
	assert(title);
	MMasterTitleMutex.writelock();
	if(titles_list.count(title->GetID()) == 0)
		titles_list[title->GetID()] = title;
	MMasterTitleMutex.releasewritelock();
}

int32 MasterTitlesList::Size(){
	int32 size = 0;
	MMasterTitleMutex.readlock();
	size = titles_list.size();
	MMasterTitleMutex.releasereadlock();

	return size;
}

Title* MasterTitlesList::GetTitle(sint32 id){
	Title* title = 0;

	MMasterTitleMutex.readlock();
	if(titles_list.count(id) > 0)
		title = titles_list[id];
	MMasterTitleMutex.releasereadlock();

	return title;
}

Title* MasterTitlesList::GetTitleByName(const char* title_name){
	Title* title = 0;
	map<sint32, Title*>::iterator itr;
	MMasterTitleMutex.readlock();
	for(itr = titles_list.begin(); itr != titles_list.end(); itr++){
		Title* current_title = itr->second;
		if(::ToLower(string(current_title->GetName())) == ::ToLower(string(title_name))){
			title = current_title;
			break;
		}
	}
	MMasterTitleMutex.releasereadlock();
	return title;
}

PlayerTitlesList::PlayerTitlesList(){
	MPlayerTitleMutex.SetName("PlayerTitlesList::MPlayerTitleMutex");
}

PlayerTitlesList::~PlayerTitlesList(){
	MPlayerTitleMutex.writelock();
	vector<Title*>::iterator itr;
	for (itr = player_titles_list.begin(); itr != player_titles_list.end(); itr++)
		safe_delete(*itr);

	player_titles_list.clear();
	MPlayerTitleMutex.releasewritelock();
}

Title* PlayerTitlesList::GetTitle(sint32 index){
	MPlayerTitleMutex.readlock();
	Title* title = 0;
	Title* ret = 0;
	if ( index < player_titles_list.size() )
		ret = player_titles_list[index];
	
	MPlayerTitleMutex.releasereadlock();
	return ret;
}

Title* PlayerTitlesList::GetTitleByName(const char* title_name){
	Title* resTitle = 0;
	vector<Title*>::iterator itr;
	MPlayerTitleMutex.readlock();
	for(itr = player_titles_list.begin(); itr != player_titles_list.end(); itr++){
		Title* title = *itr;
		if(::ToLower(string(title->GetName())) == ::ToLower(string(title_name))){
			resTitle = title;
			break;
		}
	}
	MPlayerTitleMutex.releasereadlock();
	return resTitle;
}


vector<Title*>* PlayerTitlesList::GetAllTitles(){
	MPlayerTitleMutex.readlock();
	return &player_titles_list;
}

void PlayerTitlesList::Add(Title* title){
	MPlayerTitleMutex.writelock();
	player_titles_list.push_back(title);
	MPlayerTitleMutex.releasewritelock();
}

int32 PlayerTitlesList::Size(){
	int32 size = 0;
	MPlayerTitleMutex.readlock();
	size = player_titles_list.size();
	MPlayerTitleMutex.releasereadlock();

	return size;
}