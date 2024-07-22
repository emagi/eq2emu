/*  
	EQ2Emulator:  Everquest II Server Emulator
	Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

	This file is part of EQ2Emulator.
*/
#ifndef _LOGINACCOUNT_
#define _LOGINACCOUNT_

#include <iostream>
#include <string>
#include <vector>
#include "../common/linked_list.h"
#include "PacketHeaders.h"
#include "../common/PacketStruct.h"

using namespace std;
class LoginAccount {
public:
	LoginAccount();
	LoginAccount(int32 id, const char* in_name, const char* in_pass){ 
		account_id = id;
		strcpy(name, in_name);
		strcpy(password, in_pass);
	}
	~LoginAccount();
	bool SaveAccount(LoginAccount* acct);
	vector<CharSelectProfile*> charlist;
	void setName(const char* in_name) { strcpy(name, in_name); }
	void setPassword(const char* in_pass) { strcpy(password, in_pass); }
	void setAuthenticated(bool in_auth) { authenticated=in_auth; }
	void setAccountID(int32 id){ account_id = id; }
	void addCharacter(CharSelectProfile* profile){
		charlist.push_back(profile);
	}
	void removeCharacter(PacketStruct* profile);
	void removeCharacter(char* name, int16 version);
	void serializeCharacter(uchar* buffer, CharSelectProfile* profile);

	void flushCharacters ( );

	CharSelectProfile* getCharacter(char* name);
	int32 getLoginAccountID(){ return account_id; }
	char* getLoginName() { return name; }
	char* getLoginPassword() { return password; }
	bool  getLoginAuthenticated() { return authenticated; }

private:
	int32 account_id;
	char name[32];
	char password[32];
	bool authenticated;
};
#endif