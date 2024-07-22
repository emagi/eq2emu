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

#ifndef CHAT_CHATCHANNEL_H_
#define CHAT_CHATCHANNEL_H_

#include "../../common/types.h"
#include "../client.h"
#include <vector>

using namespace std;

#define CHAT_CHANNEL_MAX_NAME		100
#define CHAT_CHANNEL_MAX_PASSWORD	100

enum ChatChannelType {
	CHAT_CHANNEL_TYPE_NONE = 0,
	CHAT_CHANNEL_TYPE_WORLD,
	CHAT_CHANNEL_TYPE_CUSTOM
};

class ChatChannel {
public:
	ChatChannel();
	virtual ~ChatChannel();

	void SetName(const char *name) {strncpy(this->name, name, CHAT_CHANNEL_MAX_NAME);}
	void SetPassword(const char *password) {strncpy(this->password, password, CHAT_CHANNEL_MAX_PASSWORD);}
	void SetType(ChatChannelType type) {this->type = type;}
	void SetLevelRestriction(int16 level_restriction) {this->level_restriction = level_restriction;}
	void SetRacesAllowed(int64 races) {this->races = races;}
	void SetClassesAllowed(int64 classes) {this->classes = classes;}

	const char * GetName() {return name;}
	ChatChannelType GetType() {return type;}
	unsigned int GetNumClients() {return clients.size();}

	bool HasPassword() {return password[0] != '\0';}
	bool PasswordMatches(const char *password) {return strncmp(this->password, password, CHAT_CHANNEL_MAX_PASSWORD) == 0;}
	bool CanJoinChannelByLevel(int16 level) {return level >= level_restriction;}
	bool CanJoinChannelByRace(int8 race_id) {return races == 0 || (1 << race_id) & races;}
	bool CanJoinChannelByClass(int8 class_id) {return classes == 0 || (1 << class_id) & classes;}

	bool IsInChannel(int32 character_id);
	bool JoinChannel(Client *client);
	bool LeaveChannel(Client *client);
	bool TellChannel(Client *client, const char *message, const char* name2 = 0);
	bool TellChannelClient(Client* to_client, const char* message, const char* name2 = 0);
	bool SendChannelUserList(Client *client);


private:
	char name[CHAT_CHANNEL_MAX_NAME + 1];
	char password[CHAT_CHANNEL_MAX_PASSWORD + 1];
	ChatChannelType type;
	vector<int32> clients;
	int16 level_restriction;
	int64 races;
	int64 classes;
};

#endif
