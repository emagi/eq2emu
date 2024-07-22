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

#ifndef LANGUAGES_H_
#define LANGUAGES_H_

#include <string>
#include <list>
#include "../common/types.h"

using namespace std;

class Language {
public:
	Language();
	Language(Language* language);
	void		SetID(int32 id) {this->id = id;}
	void		SetName(const char *name) {strncpy(this->name, name, sizeof(this->name));}
	void		SetSaveNeeded(bool save_needed) {this->save_needed = save_needed;}

	int32		GetID() {return id;}
	const char*	GetName() {return name;}
	bool		GetSaveNeeded() {return save_needed;}

private:
	int32	id;
	char	name[50];
	bool	save_needed;
};

class MasterLanguagesList {
public:
	MasterLanguagesList();
	~MasterLanguagesList();
	void	Clear();
	int32	Size();
	void	AddLanguage(Language* language);
	Language*	GetLanguage(int32 id);
	Language*	GetLanguageByName(const char* name);
	list<Language*>*	GetAllLanguages();

private:
	list<Language*> languages_list;
};

class PlayerLanguagesList {
public:
	PlayerLanguagesList();
	~PlayerLanguagesList();
	void Clear();
	void Add(Language* language);
	Language*	GetLanguage(int32 id);
	Language*	GetLanguageByName(const char* name);
	list<Language*>* GetAllLanguages();

private:
	list<Language*> player_languages_list;
};
#endif
