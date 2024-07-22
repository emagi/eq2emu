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
#ifndef CHAT_CHAT_H_
#define CHAT_CHAT_H_

#include <vector>
#include "../../common/types.h"
#include "../../common/EQPacket.h"
#include "../../common/Mutex.h"
#include "../client.h"
#include "ChatChannel.h"

#ifdef DISCORD
	#ifndef WIN32
		#pragma once
		#include <dpp/dpp.h>
	#endif
#endif

using namespace std;
/*

CREATING A CHANNEL

-- OP_RemoteCmdMsg --
3/14/2012 20:17:06
192.168.1.198 -> 69.174.200.73
0000:	00 09 05 9A 2A 0E 00 0F 00 63 75 73 74 6F 6D 20 ....*....custom 
0010	70 61 73 73 77 6F 72 64                         password


TALKING IN A CHANNEL

[11:52.23] <@Xinux> -- OP_RemoteCmdMsg --
[11:52.23] <@Xinux> 3/14/2012 20:17:25
[11:52.23] <@Xinux> 192.168.1.198 -> 69.174.200.73
[11:52.23] <@Xinux> 0000:	00 09 06 2D 2A 11 00 21 00 63 75 73 74 6F 6D 20 ...-*..!.custom 
[11:52.23] <@Xinux> 0010:	20 74 68 69 73 20 69 73 20 6D 79 20 63 75 73 74  this is my cust
[11:52.23] <@Xinux> 0020	6F 6D 20 63 68 61 6E 6E 65 6C                   om channel


[08:37.46] <@Xinux_Work> 00 09 05 8B 00 3A 53 00 00 00 FF 3C 02 00 00 FF .....:S....<....
[08:37.46] <@Xinux_Work> FF FF FF FF FF FF FF 06 00 4C 65 69 68 69 61 07 .........Leihia.
[08:37.46] <@Xinux_Work> 00 4B 6F 65 63 68 6F 68 00 02 00 00 00 00 01 00 .Koechoh........
[08:37.46] <@Xinux_Work> 00 00 22 00 18 00 62 65 74 74 65 72 20 74 68 61 .."...better tha
[08:37.46] <@Xinux_Work> 6E 20 61 20 72 65 64 20 6F 6E 65 20 3A 50 09 00 n a red one :P..
[08:37.46] <@Xinux_Work> 4C 65 76 65 6C 5F 31 2D 39 01 01 00 00          Level_1-9....   

OTHERS LEAVING AND JOINING A CHANNEL

-- OP_ClientCmdMsg::OP_EqChatChannelUpdateCmd --
3/14/2012 20:17:06
69.174.200.73 -> 192.168.1.198
0000:	00 3A 18 00 00 00 FF 88 02 03 09 00 4C 65 76 65 .:..........Leve
0010	6C 5F 31 2D 39 07 00 53 68 61 77 6E 61 68       l_1-9..Shawnah


-- OP_ClientCmdMsg::OP_EqChatChannelUpdateCmd --
3/14/2012 20:17:06
69.174.200.73 -> 192.168.1.198
0000:	00 3A 16 00 00 00 FF 88 02 03 07 00 41 75 63 74 .:..........Auct
0010	69 6F 6E 07 00 53 68 61 77 6E 61 68             ion..Shawnah

OP_EqChatChannelUpdateCmd
unknown=0 unknown1=blank	join
unknown=1 unknown1=blank	leave
unknown=2 unknown2=player	join/leave?
unknown=3 unknown2=player	join/leave?
*/


class Chat{
public:
	Chat();
	virtual ~Chat();

	void AddChannel(ChatChannel *channel);
	unsigned int GetNumChannels();

	EQ2Packet * GetWorldChannelList(Client *client);

	bool ChannelExists(const char *channel_name);
	bool HasPassword(const char *channel_name);
	bool PasswordMatches(const char *channel_name, const char *password);
	bool CreateChannel(const char *channel_name);
	bool CreateChannel(const char *channel_name, const char *password);
	bool IsInChannel(Client *client, const char *channel_name);
	bool JoinChannel(Client *client, const char *channel_name);
	bool LeaveChannel(Client *client, const char *channel_name);
	bool LeaveAllChannels(Client *client);
	bool TellChannel(Client *client, const char *channel_name, const char *message, const char* name = 0);
	bool SendChannelUserList(Client *client, const char *channel_name);
	//devn00b
	int PushDiscordMsg(const char*, const char*);
	ChatChannel* GetChannel(const char* channel_name);

private:
	Mutex m_channels;
	vector<ChatChannel *> channels;
};

#endif
