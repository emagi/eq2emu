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
#ifndef TITLES_H_
#define TITLES_H_

#include <string>
#include <map>
#include <vector>
#include "../common/Mutex.h"
#include "../common/types.h"

using namespace std;

class Title {
public:
	Title();
	Title(Title* title);
	~Title();
	void			SetID(int32 id) {this->id = id;}
	void			SetName(const char *name) {strncpy(this->name, name, sizeof(this->name));}
	void			SetPrefix(int8 prefix) {this->prefix = prefix;}
	void			SetSaveNeeded(bool save_needed) {this->save_needed = save_needed;}

	sint32			GetID() {return id;}
	const char*		GetName() {return name;}
	int8			GetPrefix() {return prefix;}
	bool			GetSaveNeeded() {return save_needed;}
	
private:
	sint32	id;
	int8	prefix;
	char	name[256];
	bool	save_needed;
};

class MasterTitlesList {
public:
	MasterTitlesList();
	~MasterTitlesList();
	void Clear();
	int32 Size();
	void AddTitle(Title* title);
	Title* GetTitle(sint32 id);
	Title* GetTitleByName(const char* title_name);

private:
	map<sint32,Title*> titles_list;
	Mutex MMasterTitleMutex;
};

class PlayerTitlesList {
public:
	PlayerTitlesList();
	~PlayerTitlesList();
	Title* GetTitle(sint32 index);
	Title* GetTitleByName(const char* title_name);
	vector<Title*>* GetAllTitles();
	void Add(Title* title);

	int32 Size();
	void ReleaseReadLock() { MPlayerTitleMutex.releasereadlock(); }
private:
	vector<Title*> player_titles_list;
	Mutex MPlayerTitleMutex;
};
#endif