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


#include "../../common/Log.h"
#include "Chat.h"
#include "../WorldDatabase.h"

extern Chat chat;

void WorldDatabase::LoadChannels() {
	DatabaseResult result;
	ChatChannel *channel;

	if (database_new.Select(&result, "SELECT `name`,`password`,`level_restriction`,`classes`,`races` FROM `channels`")) {
		while (result.Next()) {
			channel = new ChatChannel();
			channel->SetName(result.GetString(0));
			if (!result.IsNull(1))
				channel->SetPassword(result.GetString(1));
			channel->SetLevelRestriction(result.GetInt16(2));
			channel->SetClassesAllowed(result.GetInt64(3));
			channel->SetRacesAllowed(result.GetInt64(4));
			channel->SetType(CHAT_CHANNEL_TYPE_WORLD);

			chat.AddChannel(channel);
		}
	}
}