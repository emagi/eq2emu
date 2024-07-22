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

#include "Chat.h"
#include "../../common/Log.h"
#include "../../common/ConfigReader.h"
#include "../../common/PacketStruct.h"
		#include "../Rules/Rules.h"
		extern RuleManager rule_manager;

//devn00b
#ifdef DISCORD
	#ifndef WIN32
		#include <dpp/dpp.h>
		#include "ChatChannel.h"

		extern ChatChannel channel;
	#endif
#endif


extern ConfigReader configReader;



Chat::Chat() {
	m_channels.SetName("Chat::Channels");
}

Chat::~Chat() {
	vector<ChatChannel *>::iterator itr;

	m_channels.writelock(__FUNCTION__, __LINE__);
	for (itr = channels.begin(); itr != channels.end(); itr++)
		safe_delete(*itr);
	m_channels.releasewritelock(__FUNCTION__, __LINE__);
}

void Chat::AddChannel(ChatChannel *channel) {
	m_channels.writelock(__FUNCTION__, __LINE__);
	channels.push_back(channel);
	m_channels.releasewritelock(__FUNCTION__, __LINE__);
}

unsigned int Chat::GetNumChannels() {
	unsigned int ret;

	m_channels.readlock(__FUNCTION__, __LINE__);
	ret = (unsigned int)channels.size();
	m_channels.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

EQ2Packet * Chat::GetWorldChannelList(Client *client) {
	PacketStruct *packet_struct = configReader.getStruct("WS_AvailWorldChannels", client->GetVersion());
	Player *player = client->GetPlayer();
	vector<ChatChannel *> channels_to_send;
	vector<ChatChannel *>::iterator itr;
	ChatChannel *channel;
	EQ2Packet *packet;
	int32 i = 0;
	bool add;

	if (packet_struct == NULL) {
		LogWrite(CHAT__ERROR, 0, "Chat", "Could not find packet 'WS_AvailWorldChannels' for client %s on version %i\n", player->GetName(), client->GetVersion());
		return NULL;
	}

	m_channels.readlock(__FUNCTION__, __LINE__);
	for (itr = channels.begin(); itr != channels.end(); itr++) {
		channel = *itr;

		if (channel->GetType() == CHAT_CHANNEL_TYPE_WORLD) {
			add = true;

			if (add && !channel->CanJoinChannelByLevel(player->GetLevel()))
				add = false;
			if (add && !channel->CanJoinChannelByRace(player->GetRace()))
				add = false;
			if (add && !channel->CanJoinChannelByClass(player->GetAdventureClass()))
				add = false;
			
			if (add)
				channels_to_send.push_back(channel);
		}
	}
	m_channels.releasereadlock(__FUNCTION__, __LINE__);

	packet_struct->setArrayLengthByName("num_channels", channels_to_send.size());
	for (itr = channels_to_send.begin(); itr != channels_to_send.end(); itr++, i++) {
		packet_struct->setArrayDataByName("channel_name", (*itr)->GetName(), i);
		packet_struct->setArrayDataByName("unknown", 0, i);
	}

	packet = packet_struct->serialize();
	safe_delete(packet_struct);

	return packet;
}

bool Chat::ChannelExists(const char *channel_name) {
	vector<ChatChannel *>::iterator itr;
	bool ret = false;

	m_channels.readlock(__FUNCTION__, __LINE__);
	for (itr = channels.begin(); itr != channels.end(); itr++) {
		if (strncasecmp(channel_name, (*itr)->GetName(), CHAT_CHANNEL_MAX_NAME) == 0) {
			ret = true;
			break;
		}
	}
	m_channels.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

bool Chat::HasPassword(const char *channel_name) {
	vector<ChatChannel *>::iterator itr;
	bool ret = false;

	m_channels.readlock(__FUNCTION__, __LINE__);
	for (itr = channels.begin(); itr != channels.end(); itr++) {
		if (strncasecmp(channel_name, (*itr)->GetName(), CHAT_CHANNEL_MAX_NAME) == 0) {
			ret = (*itr)->HasPassword();
			break;
		}
	}
	m_channels.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

bool Chat::PasswordMatches(const char *channel_name, const char *password) {
	vector<ChatChannel *>::iterator itr;
	bool ret = false;

	m_channels.readlock(__FUNCTION__, __LINE__);
	for (itr = channels.begin(); itr != channels.end(); itr++) {
		if (strncasecmp(channel_name, (*itr)->GetName(), CHAT_CHANNEL_MAX_NAME) == 0) {
			ret = (*itr)->PasswordMatches(password);
			break;
		}
	}
	m_channels.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

bool Chat::CreateChannel(const char *channel_name) {
	return CreateChannel(channel_name, NULL);
}

bool Chat::CreateChannel(const char *channel_name, const char *password) {
	LogWrite(CHAT__DEBUG, 0, "Chat", "Channel %s being created", channel_name);

	ChatChannel *channel = new ChatChannel();
	channel->SetName(channel_name);
	channel->SetType(CHAT_CHANNEL_TYPE_CUSTOM);
	if (password != NULL)
		channel->SetPassword(password);

	m_channels.writelock(__FUNCTION__, __LINE__);
	channels.push_back(channel);
	m_channels.releasewritelock(__FUNCTION__, __LINE__);

	return true;
}

bool Chat::IsInChannel(Client *client, const char *channel_name) {
	vector<ChatChannel *>::iterator itr;
	bool ret = false;

	m_channels.readlock(__FUNCTION__, __LINE__);
	for (itr = channels.begin(); itr != channels.end(); itr++) {
		if (strncasecmp(channel_name, (*itr)->GetName(), CHAT_CHANNEL_MAX_NAME) == 0) {
			ret = (*itr)->IsInChannel(client->GetCharacterID());
			break;
		}
	}
	m_channels.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

bool Chat::JoinChannel(Client *client, const char *channel_name) {
	vector<ChatChannel *>::iterator itr;
	bool ret = false;

	LogWrite(CHAT__DEBUG, 1, "Chat", "Client %s is joining channel %s", client->GetPlayer()->GetName(), channel_name);

	m_channels.writelock(__FUNCTION__, __LINE__);
	for (itr = channels.begin(); itr != channels.end(); itr++) {
		if (strncasecmp(channel_name, (*itr)->GetName(), CHAT_CHANNEL_MAX_NAME) == 0) {
			ret = (*itr)->JoinChannel(client);
			break;
		}
	}
	m_channels.releasewritelock(__FUNCTION__, __LINE__);

	return ret;
}

bool Chat::LeaveChannel(Client *client, const char *channel_name) {
	vector<ChatChannel *>::iterator itr;
	bool ret = false;

	LogWrite(CHAT__DEBUG, 1, "Chat", "Client %s is leaving channel %s", client->GetPlayer()->GetName(), channel_name);

	m_channels.writelock(__FUNCTION__, __LINE__);
	for (itr = channels.begin(); itr != channels.end(); itr++) {
		if (strncasecmp(channel_name, (*itr)->GetName(), CHAT_CHANNEL_MAX_NAME) == 0) {
			ret = (*itr)->LeaveChannel(client);

			if ((*itr)->GetType() == CHAT_CHANNEL_TYPE_CUSTOM && (*itr)->GetNumClients() == 0) {
				LogWrite(CHAT__DEBUG, 0, "Chat", "Custom channel %s has 0 clients left, deleting channel", channel_name);
				safe_delete(*itr);
				channels.erase(itr);
			}

			break;
		}
	}
	m_channels.releasewritelock(__FUNCTION__, __LINE__);

	return ret;
}

bool Chat::LeaveAllChannels(Client *client) {
	vector<ChatChannel *>::iterator itr;
	ChatChannel *channel;
	bool erased;

	m_channels.writelock(__FUNCTION__, __LINE__);
	itr = channels.begin();
	while (itr != channels.end()) {
		channel = *itr;
		erased = false;

		if (channel->IsInChannel(client->GetCharacterID())) {
			LogWrite(CHAT__DEBUG, 1, "Chat", "Client %s is leaving channel %s", client->GetPlayer()->GetName(), channel->GetName());
			channel->LeaveChannel(client);
			
			if (channel->GetType() == CHAT_CHANNEL_TYPE_CUSTOM && channel->GetNumClients() == 0) {
				LogWrite(CHAT__DEBUG, 0, "Chat", "Custom channel %s has 0 clients left, deleting channel", channel->GetName());
				safe_delete(*itr);
				itr = channels.erase(itr);
				erased = true;
			}
		}

		if (!erased)
			itr++;
	}
	m_channels.releasewritelock(__FUNCTION__, __LINE__);

	return true;
}

bool Chat::TellChannel(Client *client, const char *channel_name, const char *message, const char* name) {
	vector<ChatChannel *>::iterator itr;
	bool ret = false;
	bool enablediscord = rule_manager.GetGlobalRule(R_Discord, DiscordEnabled)->GetBool();
	const char* discordchan = rule_manager.GetGlobalRule(R_Discord, DiscordChannel)->GetString();

	m_channels.readlock(__FUNCTION__, __LINE__);
	for (itr = channels.begin(); itr != channels.end(); itr++) {
		if (strncasecmp(channel_name, (*itr)->GetName(), CHAT_CHANNEL_MAX_NAME) == 0) {
			if (client && name)
				ret = (*itr)->TellChannelClient(client, message, name);
			else
				ret = (*itr)->TellChannel(client, message, name);

			if(enablediscord == true && client){

				if (strcmp(channel_name, discordchan) != 0){	
					m_channels.releasereadlock(__FUNCTION__, __LINE__);
					return ret;
				}
#ifdef DISCORD
				if (client) {
                	std::string whofrom = client->GetPlayer()->GetName();
                	std::string msg = string(message);
                	ret = PushDiscordMsg(msg.c_str(), whofrom.c_str());
				}
#endif				
			}

			break;
		}
	}
	m_channels.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

bool Chat::SendChannelUserList(Client *client, const char *channel_name) {
	vector<ChatChannel *>::iterator itr;
	bool ret = false;

	m_channels.readlock(__FUNCTION__, __LINE__);
	for (itr = channels.begin(); itr != channels.end(); itr++) {
		if (strncasecmp(channel_name, (*itr)->GetName(), CHAT_CHANNEL_MAX_NAME) == 0) {
			ret = (*itr)->SendChannelUserList(client);
			break;
		}
	}
	m_channels.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

ChatChannel* Chat::GetChannel(const char *channel_name) {
	vector<ChatChannel *>::iterator itr;
	ChatChannel* ret = 0;

	m_channels.readlock(__FUNCTION__, __LINE__);
	for (itr = channels.begin(); itr != channels.end(); itr++) {
		if (strncasecmp(channel_name, (*itr)->GetName(), CHAT_CHANNEL_MAX_NAME) == 0) {
			ret = (*itr);
			break;
		}
	}
	m_channels.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

#ifdef DISCORD
//this sends chat from EQ2EMu to Discord. Currently using webhooks. Makes things simpler code wise.
int Chat::PushDiscordMsg(const char* msg, const char* from) {
	bool enablediscord = rule_manager.GetGlobalRule(R_Discord, DiscordEnabled)->GetBool();
	
	if(enablediscord == false) {
		LogWrite(INIT__INFO, 0,"Discord","Bot Disabled By Rule...");
		return 0;
	}

 	m_channels.readlock(__FUNCTION__, __LINE__);
	const char* hook = rule_manager.GetGlobalRule(R_Discord, DiscordWebhookURL)->GetString();
	std::string servername = net.GetWorldName();
	char ourmsg[4096];

	//form our message
	sprintf(ourmsg,"[%s] [%s] Says: %s",from, servername.c_str(), msg);

	/* send a message with this webhook */
	dpp::cluster bot("");
	dpp::webhook wh(hook);
	bot.execute_webhook(wh, dpp::message(ourmsg));
    m_channels.releasereadlock(__FUNCTION__, __LINE__);
	
	return 1;
}
#endif