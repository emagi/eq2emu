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
#include "../common/debug.h"
#include "../common/Log.h"
#include <iostream>
using namespace std;
#include <string.h>
#include <stdio.h>
#include <iomanip>
using namespace std;
#include <stdlib.h>
#include "../common/version.h"
#include "../common/GlobalHeaders.h"
#include "../common/sha512.h"

#ifdef WIN32
#include <process.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <windows.h>

#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else // Pyro: fix for linux
#include <sys/socket.h>
#ifdef FREEBSD //Timothy Whitman - January 7, 2003
#include <sys/types.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include "../common/unix.h"

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
extern int errno;
#endif

#include "../common/servertalk.h"
#include "LoginServer.h"
#include "../common/packet_dump.h"
#include "net.h"
#include "zoneserver.h"
#include "WorldDatabase.h"
#include "Variables.h"
#include "World.h"
#include "../common/ConfigReader.h"
#include "Rules/Rules.h"
#include "Web/PeerManager.h"
#include "Web/HTTPSClientPool.h"

extern sint32			numzones;
extern sint32			numclients;
extern NetConnection net;
extern LoginServer loginserver;
extern WorldDatabase	database;
extern ZoneAuth zone_auth;
extern Variables variables;
extern ZoneList zone_list;
extern ClientList client_list;
extern volatile bool	RunLoops;
volatile bool LoginLoopRunning = false;
extern ConfigReader configReader;
extern RuleManager rule_manager;
extern PeerManager peer_manager;
extern HTTPSClientPool peer_https_pool;

bool AttemptingConnect = false;

LoginServer::LoginServer(const char* iAddress, int16 iPort) {
	LoginServerIP = ResolveIP(iAddress);
	LoginServerPort = iPort;
	statusupdate_timer = new Timer(LoginServer_StatusUpdateInterval);
	tcpc = new TCPConnection(false);
	pTryReconnect = true;
	minLockedStatus = 100;
	maxPlayers = -1;
	minGameFullStatus = 100;
	last_checked_time = 0;
	zone_updates = 0;
	loginEquip_updates = 0;
}

LoginServer::~LoginServer() {
	delete statusupdate_timer;
	delete tcpc;
}

void LoginServer::SendImmediateEquipmentUpdatesForChar(int32 char_id) {
	LogWrite(WORLD__DEBUG, 5, "World", "Sending login equipment updates for char_id: %u", char_id);

	int16 count = 0;
	if(!loginEquip_updates)
		loginEquip_updates = database.GetEquipmentUpdates(char_id);
	if(loginEquip_updates && loginEquip_updates->size() > 0)
	{
		map<int32, LoginEquipmentUpdate> send_map;
		int32 size = 0;
		MutexMap<int32, LoginEquipmentUpdate>::iterator itr = loginEquip_updates->begin();
		while(itr.Next())
		{
			send_map[itr->first] = itr->second;
			size += sizeof(EquipmentUpdate_Struct);
			loginEquip_updates->erase(itr->first);
			count++;
		}
		ServerPacket* outpack = new ServerPacket(ServerOP_LoginEquipment, size + sizeof(EquipmentUpdateList_Struct)+5);
		EquipmentUpdateList_Struct* updates = (EquipmentUpdateList_Struct*)outpack->pBuffer;
		updates->total_updates = count;
		int32 pos = sizeof(EquipmentUpdateList_Struct);
		map<int32, LoginEquipmentUpdate>::iterator send_itr;
		for(send_itr = send_map.begin(); send_itr != send_map.end(); send_itr++)
		{
			EquipmentUpdate_Struct* update = (EquipmentUpdate_Struct*)(outpack->pBuffer + pos);
			update->id				= send_itr->first;
			update->world_char_id	= send_itr->second.world_char_id;
			update->equip_type		= send_itr->second.equip_type;
			update->red				= send_itr->second.red;
			update->green			= send_itr->second.green;
			update->blue			= send_itr->second.blue;
			update->highlight_red	= send_itr->second.red;
			update->highlight_green	= send_itr->second.green;
			update->highlight_blue	= send_itr->second.blue;
			update->slot			= send_itr->second.slot;
			pos += sizeof(EquipmentUpdate_Struct);
		}
		SendPacket(outpack);
		outpack->Deflate();
		safe_delete(outpack);
	}
	if(loginEquip_updates && count)
		loginEquip_updates->clear();
	if(loginEquip_updates && loginEquip_updates->size() == 0)
	{
		database.UpdateLoginEquipment();
		safe_delete(loginEquip_updates);
	}

}

bool LoginServer::Process() {
	if(last_checked_time > Timer::GetCurrentTime2())
		return true;
	last_checked_time = Timer::GetCurrentTime2() + 50;
	bool ret = true;
	if (statusupdate_timer->Check()) {
		this->SendStatus();
	}

	/************ Get all packets from packet manager out queue and process them ************/
	ServerPacket *pack = 0;
	while((pack = tcpc->PopPacket()))
	{
		switch(pack->opcode)
		{
		case ServerOP_LSFatalError:
			{
				LogWrite(OPCODE__DEBUG, 1, "Opcode", "Opcode 0x%X (%i): ServerOP_LSFatalError", pack->opcode, pack->opcode);

				LogWrite(WORLD__ERROR, 0, "World", "Login Server returned a fatal error: %s\n", pack->pBuffer);
				tcpc->Disconnect();
				ret = false;
				//net.ReadLoginINI(); // can't properly support with command line args now
				break;
			}
		case ServerOP_CharTimeStamp: 
			{
				LogWrite(OPCODE__DEBUG, 1, "Opcode", "Opcode 0x%X (%i): ServerOP_CharTimeStamp", pack->opcode, pack->opcode);

				if(pack->size != sizeof(CharacterTimeStamp_Struct))
					break;

				CharacterTimeStamp_Struct* cts = (CharacterTimeStamp_Struct*) pack->pBuffer;

				// determine if the character exists and retrieve its latest timestamp from the world server
				bool char_exist = false;
				//int32 character_timestamp = database.GetCharacterTimeStamp(cts->char_id,cts->account_id,&char_exist);

				if(!char_exist)
				{
					//Character doesn't exist, get rid of it
					SendDeleteCharacter ( cts );
					break;
				}

				break;
			}

		// Push Character Select "item appearances" to login_equipment table
		case ServerOP_LoginEquipment:{
			LogWrite(OPCODE__DEBUG, 1, "Opcode", "Opcode 0x%X (%i): ServerOP_LoginEquipment", pack->opcode, pack->opcode);

			LogWrite(MISC__TODO, 0, "TODO", "Implement map<character id <map<slot id, updatestruct> > method to update Login.\n%s, %s, %i", __FILE__, __FUNCTION__, __LINE__);

			if( pack->size == sizeof(EquipmentUpdateRequest_Struct) )
			{
				int16 max = ((EquipmentUpdateRequest_Struct*)pack->pBuffer)->max_per_batch;
				int16 count = 0;
				if(!loginEquip_updates)
					loginEquip_updates = database.GetEquipmentUpdates();
				if(loginEquip_updates && loginEquip_updates->size() > 0)
				{
					map<int32, LoginEquipmentUpdate> send_map;
					int32 size = 0;
					MutexMap<int32, LoginEquipmentUpdate>::iterator itr = loginEquip_updates->begin();
					while(itr.Next() && count < max)
					{
						send_map[itr->first] = itr->second;
						size += sizeof(EquipmentUpdate_Struct);
						loginEquip_updates->erase(itr->first);
						count++;
					}
					ServerPacket* outpack = new ServerPacket(ServerOP_LoginEquipment, size + sizeof(EquipmentUpdateList_Struct)+5);
					EquipmentUpdateList_Struct* updates = (EquipmentUpdateList_Struct*)outpack->pBuffer;
					updates->total_updates = count;
					int32 pos = sizeof(EquipmentUpdateList_Struct);
					map<int32, LoginEquipmentUpdate>::iterator send_itr;
					for(send_itr = send_map.begin(); send_itr != send_map.end(); send_itr++)
					{
						EquipmentUpdate_Struct* update = (EquipmentUpdate_Struct*)(outpack->pBuffer + pos);
						update->id				= send_itr->first;
						update->world_char_id	= send_itr->second.world_char_id;
						update->equip_type		= send_itr->second.equip_type;
						update->red				= send_itr->second.red;
						update->green			= send_itr->second.green;
						update->blue			= send_itr->second.blue;
						update->highlight_red	= send_itr->second.red;
						update->highlight_green	= send_itr->second.green;
						update->highlight_blue	= send_itr->second.blue;
						update->slot			= send_itr->second.slot;
						pos += sizeof(EquipmentUpdate_Struct);
					}
					SendPacket(outpack);
					outpack->Deflate();
					safe_delete(outpack);
				}
				if(loginEquip_updates && count < max)
					loginEquip_updates->clear();
				if(loginEquip_updates && loginEquip_updates->size() == 0)
				{
					database.UpdateLoginEquipment();
					safe_delete(loginEquip_updates);
				}
			}
			break;
		}

		case ServerOP_ZoneUpdates:{
			LogWrite(OPCODE__DEBUG, 1, "Opcode", "Opcode 0x%X (%i): ServerOP_ZoneUpdates", pack->opcode, pack->opcode);

			if(pack->size == sizeof(ZoneUpdateRequest_Struct)){
				int16 max = ((ZoneUpdateRequest_Struct*)pack->pBuffer)->max_per_batch;
				int16 count = 0;
				if(!zone_updates)
					zone_updates = database.GetZoneUpdates();
				if(zone_updates && zone_updates->size() > 0){
					map<int32, LoginZoneUpdate> send_map;
					int32 size = 0;
					MutexMap<int32, LoginZoneUpdate>::iterator itr = zone_updates->begin();
					while(itr.Next() && count < max){
						send_map[itr->first] = itr->second;
						size += sizeof(ZoneUpdate_Struct) + itr->second.name.length() + itr->second.description.length();
						zone_updates->erase(itr->first);
						count++;
					}
					ServerPacket* outpack = new ServerPacket(ServerOP_ZoneUpdates, size + sizeof(ZoneUpdateList_Struct)+5);
					ZoneUpdateList_Struct* updates = (ZoneUpdateList_Struct*)outpack->pBuffer;
					updates->total_updates = count;
					int32 pos = sizeof(ZoneUpdateList_Struct);
					map<int32, LoginZoneUpdate>::iterator send_itr;
					for(send_itr = send_map.begin(); send_itr != send_map.end(); send_itr++){
						ZoneUpdate_Struct* update = (ZoneUpdate_Struct*)(outpack->pBuffer + pos);
						update->zone_id = send_itr->first;
						update->zone_name_length = send_itr->second.name.length();
						update->zone_desc_length = send_itr->second.description.length();
						strcpy(update->data, send_itr->second.name.c_str());
						strcpy(update->data + send_itr->second.name.length(), send_itr->second.description.c_str());
						pos += sizeof(ZoneUpdate_Struct) + send_itr->second.name.length() + send_itr->second.description.length();
					}
					SendPacket(outpack);
					outpack->Deflate();
					safe_delete(outpack);
				}
				if(zone_updates && count < max)
					zone_updates->clear();
				if(zone_updates && zone_updates->size() == 0){
					database.UpdateLoginZones();
					safe_delete(zone_updates);
				}
			}
			break;
		}
		case ServerOP_CharacterCreate:{
			LogWrite(OPCODE__DEBUG, 1, "Opcode", "Opcode 0x%X (%i): ServerOP_CharacterCreate", pack->opcode, pack->opcode);

			int16 version = 1;
			if(pack->pBuffer[0] > 0)
				memcpy(&version, pack->pBuffer, sizeof(int16));
			//DumpPacket(pack->pBuffer,pack->size);
			PacketStruct* packet = configReader.getStruct("CreateCharacter", version);
			int8 resp = 0;
			int32 acct_id = 0;
			int32 char_id = 0;
			// have to load packet to clear the buffer
			if(packet && packet->LoadPacketData(pack->pBuffer+sizeof(int16),pack->size - sizeof(int16), version <= 561 ? false : true)){
				if(net.world_locked) {
					resp = NOSERVERSAVAIL_REPLY; // no new characters when locked
				}
				else {
					EQ2_16BitString name = packet->getType_EQ2_16BitString_ByName("name");
					resp = database.CheckNameFilter(name.data.c_str());
					acct_id = packet->getType_int32_ByName("account_id");
					LogWrite(WORLD__DEBUG, 0, "World", "Response: %i", (int)resp);
					
					sint16 lowestStatus = database.GetLowestCharacterAdminStatus(acct_id);
					if(lowestStatus == -2)
						resp = UNKNOWNERROR_REPLY2;
					else if(resp == CREATESUCCESS_REPLY)
						char_id = database.SaveCharacter(packet, acct_id);
				}
			}
			else{
				LogWrite(WORLD__ERROR, 0, "World", "Invalid creation request!");
				resp = UNKNOWNERROR_REPLY;
			}
			// send name filter response data back to the login server
			SendFilterNameResponse ( resp , acct_id , char_id );
			safe_delete(packet);
			break;
			}
		case ServerOP_BasicCharUpdate: {
			LogWrite(OPCODE__DEBUG, 1, "Opcode", "Opcode 0x%X (%i): ServerOP_BasicCharUpdate", pack->opcode, pack->opcode);

			if(pack->size != sizeof(CharDataUpdate_Struct))
				break;

			CharDataUpdate_Struct* cdu = (CharDataUpdate_Struct*) pack->pBuffer;

			switch(cdu->update_field)
			{
			case DELETE_UPDATE_FLAG:
				{
					LogWrite(WORLD__DEBUG, 0, "World", "Delete character request: %i %i",cdu->account_id,cdu->char_id );
					database.DeleteCharacter(cdu->account_id,cdu->char_id);
					break;			
				}
			}
			break;
									   }
		case ServerOP_UsertoWorldReq:{
			LogWrite(OPCODE__DEBUG, 0, "Opcode", "Opcode 0x%X (%i): ServerOP_UsertoWorldReq", pack->opcode, pack->opcode);

			UsertoWorldRequest_Struct* utwr = (UsertoWorldRequest_Struct*) pack->pBuffer;	
			/*int32 id = database.GetAccountIDFromLSID(utwr->lsaccountid);
			sint16 status = database.CheckStatus(id);
			*/

			int32 access_key = 0;


			ZoneChangeDetails details;
			std::string name = database.loadCharacterFromLogin(&details, utwr->char_id, utwr->lsaccountid);
			// if it is a accepted login, we add the zone auth request
			access_key = DetermineCharacterLoginRequest ( utwr, &details, name);

			if ( access_key != 0 )
			{
				zone_auth.PurgeInactiveAuth();
				char* characterName = database.GetCharacterName( utwr->char_id );
				if(characterName != 0){
					ZoneAuthRequest* zar = new ZoneAuthRequest(utwr->lsaccountid,characterName,access_key);
					zar->setFirstLogin ( true );
					zone_auth.AddAuth(zar);
					safe_delete_array(characterName);
				}
			}
			break;
			}
		case ServerOP_ResetDatabase:{
			LogWrite(OPCODE__DEBUG, 1, "Opcode", "Opcode 0x%X (%i): ServerOP_ResetDatabase", pack->opcode, pack->opcode);

			database.ResetDatabase();
			break;
		}

		default:
			{
				LogWrite(WORLD__ERROR, 0, "World", "Unhandled opcode: %i", pack->opcode);
				DumpPacket(pack);
			}
		}
		safe_delete(pack);

		// break out if ret is now false
		if (!ret)
			break;
	}
	return ret;
}

// this should always be called in a new thread
#ifdef WIN32
void AutoInitLoginServer(void *tmp) {
#else
void *AutoInitLoginServer(void *tmp) {
#endif
	if (loginserver.GetState() == TCPS_Ready) {
		InitLoginServer();
	}
#ifndef WIN32
	return 0;
#endif
}

bool InitLoginServer() {
	if (loginserver.GetState() != TCPS_Ready) {
		LogWrite(WORLD__ERROR, 0, "World", "InitLoginServer() while already attempting connect.");
		return false;
	}
	if (!net.LoginServerInfo) {
		LogWrite(WORLD__ERROR, 0, "World", "Login server info not loaded.");
		return false;
	}

	AttemptingConnect = true;
	int16 port;
	char* address = net.GetLoginInfo(&port);
	LogWrite(WORLD__INFO, 0, "World", "InitLoginServer() attempt connect to %s on port %u.", address, port);
	loginserver.Connect(address, port);
	return true;
}

void LoginServer::InitLoginServerVariables() 
{
	minLockedStatus		= rule_manager.GetGlobalRule(R_World, ServerLockedOverrideStatus)->GetSInt16();
	maxPlayers			= rule_manager.GetGlobalRule(R_World, MaxPlayers)->GetSInt16();
	minGameFullStatus	= rule_manager.GetGlobalRule(R_World, MaxPlayersOverrideStatus)->GetSInt16();
}



bool LoginServer::Connect(const char* iAddress, int16 iPort) {
	if(!pTryReconnect)
		return false;

	char errbuf[TCPConnection_ErrorBufferSize];
	memset(errbuf, 0, TCPConnection_ErrorBufferSize);
	if (iAddress == 0) {
		LogWrite(WORLD__ERROR, 0, "World", "LoginServer::Connect: address == 0");
		return false;
	}
	else {
		if ((LoginServerIP = ResolveIP(iAddress, errbuf)) == 0) {
			LogWrite(WORLD__ERROR, 0, "World", "LoginServer::Connect: Resolving IP address: '%s'", errbuf);
			return false;
		}
	}
	if (iPort != 0)
		LoginServerPort = iPort;

	if (LoginServerIP == 0 || LoginServerPort == 0) {
		LogWrite(WORLD__ERROR, 0, "World", "LoginServer::Connect: Connect info incomplete, cannot connect");
		return false;
	}

	if (tcpc->Connect(LoginServerIP, LoginServerPort, errbuf)) {
		LogWrite(WORLD__INFO, 0, "World", "Connected to LoginServer: %s: %i", iAddress, LoginServerPort);
		SendInfo();
		SendStatus();
		return true;
	}
	else {
		LogWrite(WORLD__ERROR, 0, "World", "LoginServer::Connect: '%s'", errbuf);
		return false;
	}
}
void LoginServer::GetLatestTables(){
	ServerPacket* pack = new ServerPacket(ServerOP_GetLatestTables, sizeof(GetLatestTables_Struct));
	GetLatestTables_Struct* data = (GetLatestTables_Struct*)pack->pBuffer;
	data->table_version = CURRENT_DATABASE_MAJORVERSION*100 + CURRENT_DATABASE_MINORVERSION;
	data->data_version = CURRENT_DATABASE_MAJORVERSION*100 + CURRENT_DATABASE_MINORVERSION;
	SendPacket(pack);
	delete pack;
}
void LoginServer::SendInfo() {
	ServerPacket* pack = new ServerPacket;
	pack->opcode = ServerOP_LSInfo;
	pack->size = sizeof(ServerLSInfo_Struct);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerLSInfo_Struct* lsi = (ServerLSInfo_Struct*) pack->pBuffer;
	strcpy(lsi->protocolversion, EQEMU_PROTOCOL_VERSION);
	strcpy(lsi->serverversion, CURRENT_VERSION);
	strcpy(lsi->name, net.GetWorldName());
	strcpy(lsi->account, net.GetWorldAccount());
	lsi->dbversion = CURRENT_DATABASE_MAJORVERSION*100 + CURRENT_DATABASE_MINORVERSION;
#ifdef _DEBUG
	lsi->servertype = 4;
#endif
	string passwdSha512 = sha512(net.GetWorldPassword());
	memcpy(lsi->password, (char*)passwdSha512.c_str(), passwdSha512.length());
	strcpy(lsi->address, net.GetWorldAddress());
	SendPacket(pack);
	delete pack;
}

void LoginServer::SendStatus() {
	statusupdate_timer->Start();
	ServerPacket* pack = new ServerPacket;
	pack->opcode = ServerOP_LSStatus;
	pack->size = sizeof(ServerLSStatus_Struct);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerLSStatus_Struct* lss = (ServerLSStatus_Struct*) pack->pBuffer;

	if (net.world_locked)
		lss->status = -2;
	else if(loginserver.maxPlayers > -1 && numclients >= loginserver.maxPlayers)
		lss->status = -3;
	else
		lss->status = 1;

	lss->num_zones = numzones;
	lss->num_players = numclients;
	lss->world_max_level = rule_manager.GetGlobalRule(R_Player, MaxLevel)->GetInt8();
	SendPacket(pack);
	delete pack;
}

void LoginServer::SendDeleteCharacter ( CharacterTimeStamp_Struct* cts ) {
	ServerPacket* outpack = new ServerPacket(ServerOP_BasicCharUpdate, sizeof(CharDataUpdate_Struct));
	CharDataUpdate_Struct* cdu = (CharDataUpdate_Struct*)outpack->pBuffer;
	cdu->account_id = cts->account_id;
	cdu->char_id = cts->char_id;
	cdu->update_field = DELETE_UPDATE_FLAG;
	cdu->update_data = 1;
	SendPacket(outpack);
	safe_delete(outpack);
}

void LoginServer::SendFilterNameResponse ( int8 resp, int32 acct_id , int32 char_id ) {
	ServerPacket* outpack = new ServerPacket(ServerOP_CharacterCreate, sizeof(WorldCharNameFilterResponse_Struct));
	WorldCharNameFilterResponse_Struct* wcfr = (WorldCharNameFilterResponse_Struct*)outpack->pBuffer;
	wcfr->response = resp;
	wcfr->account_id = acct_id;
	wcfr->char_id = char_id;
	SendPacket(outpack);
	safe_delete(outpack);
}

int32 LoginServer::DetermineCharacterLoginRequest ( UsertoWorldRequest_Struct* utwr, ZoneChangeDetails* details, std::string name) {
	LogWrite(LOGIN__TRACE, 9, "Login", "Enter: %s", __FUNCTION__);
	int32 timestamp = Timer::GetUnixTimeStamp();
	int32 key = static_cast<unsigned int>(MakeRandomFloat(0.01,1.0) * UINT32_MAX);
	int8 response = 0;

	sint16 lowestStatus = database.GetLowestCharacterAdminStatus( utwr->lsaccountid );

	sint16 status = 0;
	
	if(lowestStatus == -2)
		status = -1;
	else
		status = database.GetCharacterAdminStatus ( utwr->lsaccountid , utwr->char_id );

	if(status < 100 && zone_list.ClientConnected(utwr->lsaccountid))
		status = -9;
	if(status < 0){
		LogWrite(WORLD__ERROR, 0, "World", "Login Rejected based on PLAY_ERROR (UserStatus) (MinStatus: %i), UserStatus: %i, CharID: %i",loginserver.minLockedStatus,status,utwr->char_id );
		switch(status){
			case -10:
				response = PLAY_ERROR_CHAR_NOT_LOADED;
				break;
			case -9:
				response = 0;//PLAY_ERROR_ACCOUNT_IN_USE;
				break;
			case -8:
				response = PLAY_ERROR_LOADING_ERROR;
				break;
			case -1:
				response = PLAY_ERROR_ACCOUNT_BANNED;
				break;
			default:
				response = PLAY_ERROR_PROBLEM;
		}
	}
	else if(net.world_locked == true){
		LogWrite(WORLD__INFO, 0, "World", "Login Lock Check (MinStatus: %i):, UserStatus: %i, CharID: %i",loginserver.minLockedStatus,status,utwr->char_id );

		// has high enough status, allow it
		if(status >= loginserver.minLockedStatus)
			response = 1;
	}
	else if(loginserver.maxPlayers > -1 && ((sint16)client_list.Count()) >= loginserver.maxPlayers)
	{
		LogWrite(WORLD__INFO, 0, "World", "Login GameFull Check (MinStatus: %i):, UserStatus: %i, CharID: %i",loginserver.minGameFullStatus,status,utwr->char_id );

		// has high enough status, allow it
		if(status >= loginserver.minGameFullStatus)
		{
			response = 1;
		}
		else
			response = -3; // server full response is -3
	}
	else
		response = 1;

	bool attemptedPeer = false;
	if(response == 1 && details->peerId.size() > 0 && details->peerId != "self" && name.size() > 0) {
			boost::property_tree::ptree root;
			root.put("account_id", utwr->lsaccountid);
			root.put("character_name", std::string(name));
			root.put("character_id", std::to_string(utwr->char_id));
			root.put("zone_id", std::to_string(details->zoneId));
			root.put("instance_id", std::to_string(details->instanceId));
			root.put("login_key", std::to_string(key));
			root.put("client_ip", std::string(utwr->ip_address));
			root.put("world_id", std::to_string(utwr->worldid));
			root.put("from_id", std::to_string(utwr->FromID));
			root.put("first_login", true);
			std::ostringstream jsonStream;
			boost::property_tree::write_json(jsonStream, root);
			std::string jsonPayload = jsonStream.str();
			LogWrite(PEERING__INFO, 0, "Peering", "%s: Sending AddCharAuth for %s to peer %s:%u for existing zone %s", __FUNCTION__, name, details->peerWebAddress.c_str(), details->peerWebPort, details->zoneName.c_str());
			attemptedPeer = true;
			peer_https_pool.sendPostRequestToPeerAsync(details->peerId, details->peerWebAddress, std::to_string(details->peerWebPort), "/addcharauth", jsonPayload);
	}
	else if(response == 1 && details->peerId == "") {
			std::shared_ptr<Peer> peer = peer_manager.getHealthyPeerWithLeastClients();
			if(peer != nullptr) {
				boost::property_tree::ptree root;
				char* characterName = database.GetCharacterName( utwr->char_id );
				if(!characterName) {
					LogWrite(PEERING__ERROR, 0, "Peering", "%s: AddCharAuth failed to identify character name for char id %u to peer %s:%u", __FUNCTION__, utwr->char_id, peer->webAddr.c_str(), peer->webPort);
				}
				else {
					root.put("account_id", utwr->lsaccountid);
					root.put("character_name", std::string(characterName));
					root.put("character_id", std::to_string(utwr->char_id));
					root.put("zone_id", std::to_string(details->zoneId));
					root.put("instance_id", std::to_string(details->instanceId));
					root.put("login_key", std::to_string(key));
					root.put("client_ip", std::string(utwr->ip_address));
					root.put("world_id", std::to_string(utwr->worldid));
					root.put("from_id", std::to_string(utwr->FromID));
					root.put("first_login", true);
					std::ostringstream jsonStream;
					boost::property_tree::write_json(jsonStream, root);
					std::string jsonPayload = jsonStream.str();
					LogWrite(PEERING__INFO, 0, "Peering", "%s: Sending AddCharAuth for %s to peer %s:%u for new zone %s", __FUNCTION__, characterName, peer->webAddr.c_str(), peer->webPort, details->zoneName.c_str());
					attemptedPeer = true;
					peer_https_pool.sendPostRequestToPeerAsync(peer->id, peer->webAddr, std::to_string(peer->webPort), "/addcharauth", jsonPayload);
				}
			}
			else if(peer_manager.hasPeers()) {
					LogWrite(PEERING__WARNING, 0, "Peering", "%s: AddCharAuth failed to find healthy peer for char id %u", __FUNCTION__, utwr->char_id);
			}
	}
	
	/*sint32 x = database.CommandRequirement("$MAXCLIENTS");
	if( (sint32)numplayers >= x && x != -1 && x != 255 && status < 80)
	utwrs->response = -3;

	if(status == -1)
	utwrs->response = -1;
	if(status == -2)
	utwrs->response = -2;
	*/
	//printf("Response is %i for %i\n",utwrs->response,id);struct sockaddr_in sa;
	
	if(!attemptedPeer) {
		SendCharApprovedLogin(response, "", "", std::string(utwr->ip_address), 0, utwr->lsaccountid, utwr->char_id, key, utwr->worldid, utwr->FromID);
	}

	LogWrite(LOGIN__TRACE, 9, "Login", "Exit: %s with timestamp=%u", __FUNCTION__, timestamp);
	// depending on the response determined above, this could return 0 (for failure)
	return (attemptedPeer) ? 0 : key;
}

void LoginServer::SendCharApprovedLogin(int8 response, std::string peerAddress, std::string peerInternalAddress, std::string clientIP, int16 peerPort, int32 account_id, int32 char_id, int32 key, int32 world_id, int32 from_id) {
	ServerPacket* outpack = new ServerPacket;
	outpack->opcode = ServerOP_UsertoWorldResp;
	outpack->size = sizeof(UsertoWorldResponse_Struct);
	outpack->pBuffer = new uchar[outpack->size];
	memset(outpack->pBuffer, 0, outpack->size);
	UsertoWorldResponse_Struct* utwrs = (UsertoWorldResponse_Struct*) outpack->pBuffer;
	utwrs->response = response;
	utwrs->lsaccountid = account_id;
	utwrs->char_id = char_id;
	utwrs->ToID = from_id;
	utwrs->access_key = key;
	
	int32 ipv4addr = 0;
	int result = 0;
	#ifdef WIN32
		struct sockaddr_in myaddr;
		ZeroMemory(&myaddr, sizeof(myaddr));
		result = InetPton(AF_INET, clientIP.c_str(), &(myaddr.sin_addr));
		if(result)
			ipv4addr = ntohl(myaddr.sin_addr.s_addr);

	#else
		result = inet_pton(AF_INET, clientIP.c_str(), &ipv4addr);
		if(result)
			ipv4addr = ntohl(ipv4addr);
	#endif
	
	std::string internalAddress = std::string(net.GetInternalWorldAddress());
	std::string address = std::string(net.GetWorldAddress());
	int16 worldport = net.GetWorldPort();
	if(peerAddress.size() > 0 && peerPort > 0) {
		internalAddress = peerInternalAddress;
		address = peerAddress;
		worldport = peerPort;
	}
		if (((result > 0 && IsPrivateAddress(ipv4addr)) || (strcmp(address.c_str(), clientIP.c_str()) == 0)) && (internalAddress.size() > 0))
		strcpy(utwrs->ip_address, internalAddress.c_str());
	else
		strcpy(utwrs->ip_address, address.c_str());
	
	LogWrite(CCLIENT__INFO, 0, "World", "New client login attempt from %s, providing %s:%u as the world server address.",clientIP.c_str(), utwrs->ip_address, worldport );

	utwrs->port = worldport;
	utwrs->worldid = world_id;
	SendPacket(outpack);
	delete outpack;
}
