/*  
	EQ2Emulator:  Everquest II Server Emulator
	Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

	This file is part of EQ2Emulator.
*/
#include "LoginAccount.h"

LoginAccount::LoginAccount(){
}
LoginAccount::~LoginAccount(){
	vector<CharSelectProfile*>::iterator iter;
	for(iter = charlist.begin(); iter != charlist.end(); iter++){
		safe_delete(*iter);
	}
}

void LoginAccount::flushCharacters ( )
{
	vector<CharSelectProfile*>::iterator iter;
	for(iter = charlist.begin(); iter != charlist.end(); iter++){
		safe_delete(*iter);
	}

	charlist.clear ( );
}

CharSelectProfile* LoginAccount::getCharacter(char* name){
	vector<CharSelectProfile*>::iterator char_iterator;
	CharSelectProfile* profile = 0;
	EQ2_16BitString temp;
	for(char_iterator = charlist.begin(); char_iterator  != charlist.end(); char_iterator++){
		profile = *char_iterator;
		temp = profile->packet->getType_EQ2_16BitString_ByName("name");
		if(strcmp(temp.data.c_str(), name)==0)
			return profile;
	}
	return 0;
}
void LoginAccount::removeCharacter(char* name, int16 version){
	vector<CharSelectProfile*>::iterator iter;
	CharSelectProfile* profile = 0;
	EQ2_16BitString temp;
	for(iter = charlist.begin(); iter != charlist.end(); iter++){
		profile = *iter;
		temp = profile->packet->getType_EQ2_16BitString_ByName("name");
		if(strcmp(temp.data.c_str(), name)==0){
			if(version <= 561) {
				profile->deleted = true; // workaround for char select crash on old clients
			}
			else {
				safe_delete(*iter);
				charlist.erase(iter);
			}
			return;
		}
	}
}