#include <string.h>
#include "../../common/Log.h"
#include "../../common/ConfigReader.h"
#include "../../common/PacketStruct.h"
#include "../World.h"
#include "ChatChannel.h"

extern ConfigReader configReader;
extern ZoneList zone_list;

#define CHAT_CHANNEL_JOIN			0
#define CHAT_CHANNEL_LEAVE			1
#define CHAT_CHANNEL_OTHER_JOIN		2
#define CHAT_CHANNEL_OTHER_LEAVE	3

ChatChannel::ChatChannel() {
	memset(name, 0, sizeof(name));
	memset(password, 0, sizeof(password));
	type = CHAT_CHANNEL_TYPE_NONE;
	level_restriction = 0;
	races = 0;
	classes = 0;
}

ChatChannel::~ChatChannel() {
}

bool ChatChannel::IsInChannel(int32 character_id) {
	vector<int32>::iterator itr;

	for (itr = clients.begin(); itr != clients.end(); itr++) {
		if (character_id == *itr)
			return true;
	}

	return false;
}

bool ChatChannel::JoinChannel(Client *client) {
	PacketStruct *packet_struct;
	vector<int32>::iterator itr;
	Client *to_client;

	//send the player join packet to the joining client
	if ((packet_struct = configReader.getStruct("WS_ChatChannelUpdate", client->GetVersion())) == NULL) {
		LogWrite(CHAT__ERROR, 0, "Chat", "Could not find packet 'WS_ChatChannelUpdate' when client %s was trying to join channel %s", client->GetPlayer()->GetName(), name);
		return false;
	}

	packet_struct->setDataByName("action", CHAT_CHANNEL_JOIN);
	packet_struct->setDataByName("channel_name", name);
	client->QueuePacket(packet_struct->serialize());
	safe_delete(packet_struct);
	clients.push_back(client->GetCharacterID());

	//loop through everyone else in the channel and send the "other" player join packet
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		if (client->GetCharacterID() == *itr)
			continue;

		if ((to_client = zone_list.GetClientByCharID(*itr)) == NULL)
			continue;

		if ((packet_struct = configReader.getStruct("WS_ChatChannelUpdate", to_client->GetVersion())) == NULL)
			continue;

		packet_struct->setDataByName("action", CHAT_CHANNEL_OTHER_JOIN);
		packet_struct->setDataByName("channel_name", name);
		packet_struct->setDataByName("player_name", client->GetPlayer()->GetName());
		to_client->QueuePacket(packet_struct->serialize());
		safe_delete(packet_struct);
	}

	return true;
}

bool ChatChannel::LeaveChannel(Client *client) {
	vector<int32>::iterator itr;
	PacketStruct *packet_struct;
	Client *to_client;
	bool ret = false;

	for (itr = clients.begin(); itr != clients.end(); itr++) {
		if (client->GetCharacterID() == *itr) {
			clients.erase(itr);
			ret = true;
			break;
		}
	}

	if (ret) {
		//send the packet to the leaving client
		if ((packet_struct = configReader.getStruct("WS_ChatChannelUpdate", client->GetVersion())) == NULL)
			return false;

		packet_struct->setDataByName("action", CHAT_CHANNEL_LEAVE);
		packet_struct->setDataByName("channel_name", name);

		client->QueuePacket(packet_struct->serialize());
		safe_delete(packet_struct);

		//send the leave packet to all other clients in the channel
		for (itr = clients.begin(); itr != clients.end(); itr++) {
			if ((to_client = zone_list.GetClientByCharID(*itr)) == NULL)
				continue;
			if (to_client == client) // don't need to send to self.
				continue;

			if ((packet_struct = configReader.getStruct("WS_ChatChannelUpdate", to_client->GetVersion())) == NULL)
				continue;

			packet_struct->setDataByName("action", CHAT_CHANNEL_OTHER_LEAVE);
			packet_struct->setDataByName("channel_name", name);
			packet_struct->setDataByName("player_name", client->GetPlayer()->GetName());
			to_client->QueuePacket(packet_struct->serialize());
			safe_delete(packet_struct);
		}
	}

	return ret;
}

bool ChatChannel::TellChannel(Client *client, const char *message, const char* name2) {
	vector<int32>::iterator itr;
	PacketStruct *packet_struct;
	Client *to_client;

	for (itr = clients.begin(); itr != clients.end(); itr++) {
		if ((to_client = zone_list.GetClientByCharID(*itr)) == NULL)
			continue;
		if ((packet_struct = configReader.getStruct("WS_HearChat", to_client->GetVersion())) == NULL)
			continue;

		packet_struct->setDataByName("unknown", 0);
		packet_struct->setDataByName("from_spawn_id", 0xFFFFFFFF);
		packet_struct->setDataByName("to_spawn_id", 0xFFFFFFFF);

		if (client != NULL){
			packet_struct->setDataByName("from", client->GetPlayer()->GetName());
		} else {
			char name3[128];
			sprintf(name3,"[%s] from discord",name2);
			packet_struct->setDataByName("from", name3);
		}

		packet_struct->setDataByName("to", to_client->GetPlayer()->GetName());
		packet_struct->setDataByName("channel", to_client->GetMessageChannelColor(CHANNEL_CUSTOM_CHANNEL));

		if(client != NULL){
			packet_struct->setDataByName("language", client->GetPlayer()->GetCurrentLanguage());
		}else{
			packet_struct->setDataByName("language", 0);
		}
		packet_struct->setDataByName("message", message);
		packet_struct->setDataByName("channel_name", name);
		packet_struct->setDataByName("show_bubble", 1);

		if(client != NULL){
			if (client->GetPlayer()->GetCurrentLanguage() == 0 || to_client->GetPlayer()->HasLanguage(client->GetPlayer()->GetCurrentLanguage())) {
				packet_struct->setDataByName("understood", 1);
			}
		} else {
			packet_struct->setDataByName("understood", 1);
		}

		packet_struct->setDataByName("unknown4", 0);
	
		to_client->QueuePacket(packet_struct->serialize());
		safe_delete(packet_struct);
	}

	return true;
}

bool ChatChannel::TellChannelClient(Client* to_client, const char* message, const char* name2) {
	PacketStruct *packet_struct;

	if (string(name2).find('[') != string::npos)
		return true;

	packet_struct = configReader.getStruct("WS_HearChat", to_client->GetVersion());
	if (packet_struct) {
		packet_struct->setDataByName("unknown", 0);
		packet_struct->setDataByName("from_spawn_id", 0xFFFFFFFF);
		packet_struct->setDataByName("to_spawn_id", 0xFFFFFFFF);
		packet_struct->setDataByName("from", name2);
		packet_struct->setDataByName("to", to_client->GetPlayer()->GetName());
		packet_struct->setDataByName("channel", to_client->GetMessageChannelColor(CHANNEL_CUSTOM_CHANNEL));
		packet_struct->setDataByName("language", 0);
		packet_struct->setDataByName("message", message);
		packet_struct->setDataByName("channel_name", name);
		packet_struct->setDataByName("show_bubble", 1);
		packet_struct->setDataByName("understood", 1);
		packet_struct->setDataByName("unknown4", 0);

		to_client->QueuePacket(packet_struct->serialize());
	}
	safe_delete(packet_struct);

	return true;
}

bool ChatChannel::SendChannelUserList(Client *client) {
	vector<int32>::iterator itr;
	PacketStruct *packet_struct;
	Client *to_client;
	int8 i = 0;

	if ((packet_struct = configReader.getStruct("WS_WhoChannelQueryReply", client->GetVersion())) == NULL)
		return false;

	packet_struct->setDataByName("channel_name", name);
	packet_struct->setDataByName("unknown", 0);
	packet_struct->setArrayLengthByName("num_players", clients.size());
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		if ((to_client = zone_list.GetClientByCharID(*itr)) != NULL)
			packet_struct->setArrayDataByName("player_name", client->GetPlayer()->GetName(), i++);
		else
			packet_struct->setArrayDataByName("player_name", "<Unknown>", i++);
	}

	client->QueuePacket(packet_struct->serialize());
	safe_delete(packet_struct);

	return true;
} 

