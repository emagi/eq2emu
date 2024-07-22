/*  
	EQ2Emulator:  Everquest II Server Emulator
	Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

	This file is part of EQ2Emulator.
*/
#include "../common/debug.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock.h>
#include <process.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <string.h>
#include <iomanip>
#include <stdlib.h>
#include <assert.h>

#include "net.h"
#include "client.h"
#include "../common/EQStream.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "../common/emu_opcodes.h"
#include "../common/MiscFunctions.h"
#include "LWorld.h"
#include "LoginDatabase.h"
#include "../common/ConfigReader.h"
#include "../common/Log.h"

extern NetConnection	net;
extern LWorldList		world_list;
extern ClientList		client_list;
extern LoginDatabase	database;
extern map<int16,OpcodeManager*>EQOpcodeManager;
extern ConfigReader configReader;
using namespace std;
Client::Client(EQStream* ieqnc) {
	eqnc = ieqnc;
	ip = eqnc->GetrIP();
	port = ntohs(eqnc->GetrPort());
	account_id = 0;
	lsadmin = 0;
	worldadmin = 0;
	lsstatus = 0;
	version = 0;
	kicked = false;
	verified = false;
	memset(bannedreason, 0, sizeof(bannedreason));
	//worldresponse_timer = new Timer(10000);
	//worldresponse_timer->Disable();
	memset(key,0,10);
	LoginMode = None;
	num_updates = 0;
	updatetimer = new Timer(500);
	updatelisttimer = new Timer(10000);
	//keepalive = new Timer(5000);
	//logintimer = new Timer(500); // Give time for the servers to send updates
	//keepalive->Start();
	//updatetimer->Start();
	//logintimer->Disable();
	disconnectTimer = 0;
	memset(ClientSession,0,25);
	request_num = 0;
	login_account = 0;
	createRequest = 0;
	playWaitTimer = NULL;
	start = false;	
	update_position = 0;
	update_packets = 0;
	needs_world_list = true;
	sent_character_list = false;
}

Client::~Client() {
	//safe_delete(worldresponse_timer);
	//safe_delete(logintimer);
	safe_delete(login_account);
	eqnc->Close();
	safe_delete(playWaitTimer);
	safe_delete(createRequest);
	safe_delete(disconnectTimer);
	safe_delete(updatetimer);
}

bool Client::Process() {
	if(!start && !eqnc->CheckActive()){
		if(!playWaitTimer)
			playWaitTimer = new Timer(5000);
		else if(playWaitTimer->Check()){
			safe_delete(playWaitTimer);
			return false;
		}
		return true;
	}
	else if(!start){
		safe_delete(playWaitTimer);
		start = true;
	}
	
	if (disconnectTimer && disconnectTimer->Check())
	{
		safe_delete(disconnectTimer);
		getConnection()->SendDisconnect();
	}
	
	if (!kicked) {
		/************ Get all packets from packet manager out queue and process them ************/
		EQApplicationPacket *app = 0;
		/*if(logintimer && logintimer->Check())
		{
		database.LoadCharacters(GetLoginAccount());
		SendLoginAccepted();
		logintimer->Disable();
		}*/
		/*if(worldresponse_timer && worldresponse_timer->Check())
		{
		FatalError(WorldDownErrorMessage);
		worldresponse_timer->Disable();
		}*/
		
		if(playWaitTimer != NULL && playWaitTimer->Check ( ) )
		{
			SendPlayFailed(PLAY_ERROR_SERVER_TIMEOUT);
			safe_delete(playWaitTimer);
		}
		if(!needs_world_list && updatetimer && updatetimer->Check()){
			if(updatelisttimer && updatelisttimer->Check()){
				if(num_updates >= 180){ //30 minutes
					getConnection()->SendDisconnect();
				}
				else{					
					vector<PacketStruct*>::iterator itr;
					if(update_packets){
						for(itr = update_packets->begin(); itr != update_packets->end(); itr++){
							safe_delete(*itr);
						}
					}
					safe_delete(update_packets);
					update_packets = world_list.GetServerListUpdate(version);
				}
				num_updates++;
			}
			else{
				if(!update_packets){
					update_packets = world_list.GetServerListUpdate(version);
				}
				else{
					if(update_position < update_packets->size()){
						QueuePacket(update_packets->at(update_position)->serialize());
						update_position++;
					}
					else
						update_position = 0;
				}
			}
		}

		while(app = eqnc->PopPacket())
		{
			switch(app->GetOpcode())
			{
			case OP_LoginRequestMsg:{
				DumpPacket(app);
				PacketStruct* packet = configReader.getStruct("LS_LoginRequest", 1);
				if(packet && packet->LoadPacketData(app->pBuffer,app->size)){
					version = packet->getType_int16_ByName("version");
					LogWrite(LOGIN__DEBUG, 0, "Login", "Classic Client Version Provided: %i", version);

					if (version == 0 || EQOpcodeManager.count(GetOpcodeVersion(version)) == 0)
					{
						safe_delete(packet);
						packet = configReader.getStruct("LS_LoginRequest", 1208);
						if (packet && packet->LoadPacketData(app->pBuffer, app->size)) {
							version = packet->getType_int16_ByName("version");
						}
						else
							break;
					}
					//[7:19 PM] Kirmmin: Well, I very quickly learned that unknown3 in LS_LoginRequest packet is the same value as cl_eqversion in the eq2_defaults.ini file.

					LogWrite(LOGIN__DEBUG, 0, "Login", "New Client Version Provided: %i", version);

				if (EQOpcodeManager.count(GetOpcodeVersion(version)) == 0) {
					LogWrite(LOGIN__ERROR, 0, "Login", "Incompatible client version provided: %i", version);
					SendLoginDenied();
					return false;
				}
				
						if(EQOpcodeManager.count(GetOpcodeVersion(version)) > 0 && getConnection()){
						getConnection()->SetClientVersion(GetVersion());
						EQ2_16BitString username = packet->getType_EQ2_16BitString_ByName("username");
						EQ2_16BitString password = packet->getType_EQ2_16BitString_ByName("password");
						LoginAccount* acct = database.LoadAccount(username.data.c_str(),password.data.c_str(), net.IsAllowingAccountCreation());
						if(acct){
							Client* otherclient = client_list.FindByLSID(acct->getLoginAccountID());
							if(otherclient)
								otherclient->getConnection()->SendDisconnect(); // This person is already logged in, we don't want them logged in twice, kick the previous client as it might be a ghost
						}
						if(acct){
							SetAccountName(username.data.c_str());
							database.UpdateAccountIPAddress(acct->getLoginAccountID(), getConnection()->GetrIP());
							database.UpdateAccountClientDataVersion(acct->getLoginAccountID(), version);
							LogWrite(LOGIN__INFO, 0, "Login", "%s successfully logged in.", (char*)username.data.c_str());
						}
						else
						{
							if (username.size > 0)
								LogWrite(LOGIN__ERROR, 0, "Login", "%s login failed!", (char*)username.data.c_str());
							else
								LogWrite(LOGIN__ERROR, 0, "Login", "[UNKNOWN USER] login failed!");
						}

						if(!acct)
							SendLoginDenied();
						else{
							needs_world_list = true;
							SetLoginAccount(acct);							
							SendLoginAccepted();							
						}
					}
					else{
						cout << "Error bad version: " << version << endl;
						SendLoginDeniedBadVersion();
					}
				}
				else{
					cout << "Error loading LS_LoginRequest packet: \n";
					//DumpPacket(app);
				}
				safe_delete(packet);
				break;
			}
			case OP_KeymapLoadMsg:{
			//	cout << "Received OP_KeymapNoneMsg\n";
				//dunno what this is for
				break;
								  }
			case OP_AllWSDescRequestMsg:{				
				SendWorldList();
				needs_world_list = false;
				if(!sent_character_list) {
					database.LoadCharacters(GetLoginAccount(), GetVersion());
					sent_character_list = true;
				}				
				SendCharList();				
				break;
										}
			case OP_LsClientCrashlogReplyMsg:{
//				DumpPacket(app);
				SaveErrorsToDB(app, "Crash Log", GetVersion());
				break;
											 }
			case OP_LsClientVerifylogReplyMsg:{
//				DumpPacket(app);
				SaveErrorsToDB(app, "Verify Log", GetVersion());
				break;
											  }
			case OP_LsClientAlertlogReplyMsg:{
//				DumpPacket(app);
				SaveErrorsToDB(app, "Alert Log", GetVersion());
				break;
											 }
			case OP_LsClientBaselogReplyMsg:{
//				DumpPacket(app);
				SaveErrorsToDB(app, "Base Log", GetVersion());
				break;
											}
			case OP_AllCharactersDescRequestMsg:{
				break;
			}
			case OP_CreateCharacterRequestMsg:{
				PacketStruct* packet = configReader.getStruct("CreateCharacter", GetVersion());

				DumpPacket(app);
				playWaitTimer = new Timer ( 15000 );
				playWaitTimer->Start ( );
				
				LogWrite(WORLD__INFO, 1, "World", "Character creation request from account %s", GetAccountName());
				if(packet->LoadPacketData(app->pBuffer,app->size, GetVersion() <= 561 ? false : true)){
					DumpPacket(app->pBuffer, app->size);
					packet->setDataByName("account_id",GetAccountID());
					LWorld* world_server = world_list.FindByID(packet->getType_int32_ByName("server_id"));
					if(!world_server)
					{
						DumpPacket(app->pBuffer, app->size);
						cout << GetAccountName() << " attempted creation of character with an invalid server id of: " << packet->getType_int32_ByName("server_id") << "\n";
						break;
					}
					else
					{
						createRequest = packet;
						ServerPacket* outpack = new ServerPacket(ServerOP_CharacterCreate, app->size+sizeof(int16));
						int16 out_version = GetVersion();
						memcpy(outpack->pBuffer, &out_version, sizeof(int16));
						memcpy(outpack->pBuffer + sizeof(int16), app->pBuffer, app->size);
						uchar* tmp = outpack->pBuffer;	
						
						if(out_version<=283)	
							tmp+=2;	
						else if(out_version == 373) {
							tmp += 6;
						}
						else
							tmp += 7;
						
						int32 account_id = GetAccountID();
						memcpy(tmp, &account_id, sizeof(int32));
						world_server->SendPacket(outpack);
						safe_delete(outpack);
					}
				}
				else{
					LogWrite(WORLD__ERROR, 1, "World", "Error in character creation request from account %s!", GetAccountName());
					safe_delete(packet);
				}
				//	world_list.SendWorldChanged(create.profile.server_id, false, this);
				break;
											  }
			case OP_PlayCharacterRequestMsg:{
				int32 char_id = 0;
				int32 server_id = 0;
				PacketStruct* request = configReader.getStruct("LS_PlayRequest",GetVersion());
				if(request && request->LoadPacketData(app->pBuffer,app->size)){
					char_id = request->getType_int32_ByName("char_id");
					if (GetVersion() <= 283) {	
						server_id = database.GetServer(GetAccountID(), char_id, request->getType_EQ2_16BitString_ByName("name").data);	
					}	
					else {	
						server_id = request->getType_int32_ByName("server_id");	
					}
					LWorld* world = world_list.FindByID(server_id);
					string name = database.GetCharacterName(char_id,server_id,GetAccountID());
					if(world && name.length() > 0){
						pending_play_char_id = char_id;
						ServerPacket* outpack = new ServerPacket(ServerOP_UsertoWorldReq, sizeof(UsertoWorldRequest_Struct));
						UsertoWorldRequest_Struct* req = (UsertoWorldRequest_Struct*)outpack->pBuffer;
						req->char_id = char_id;
						req->lsaccountid = GetAccountID();
						req->worldid = server_id;

						struct in_addr in;
						in.s_addr = GetIP();
						strcpy(req->ip_address, inet_ntoa(in));
						world->SendPacket(outpack);
						delete outpack;

						safe_delete(playWaitTimer);

						playWaitTimer = new Timer ( 5000 );
						playWaitTimer->Start ( );
					}
					else{
						cout << GetAccountName() << " sent invalid Play Request: \n";
						SendPlayFailed(PLAY_ERROR_PROBLEM);
						DumpPacket(app);
					}
				}
				safe_delete(request);
				break;
											}
			case OP_DeleteCharacterRequestMsg:{
				PacketStruct* request = configReader.getStruct("LS_DeleteCharacterRequest", GetVersion());
				PacketStruct* response = configReader.getStruct("LS_DeleteCharacterResponse", GetVersion());
				if(request && response && request->LoadPacketData(app->pBuffer,app->size)){
					EQ2_16BitString name = request->getType_EQ2_16BitString_ByName("name");
					int32 acct_id = GetAccountID();
					int32 char_id = request->getType_int32_ByName("char_id");
					int32 server_id = request->getType_int32_ByName("server_id");
					if(database.VerifyDelete(acct_id, char_id, name.data.c_str())){
						response->setDataByName("response", 1);
						GetLoginAccount()->removeCharacter((char*)name.data.c_str(), GetVersion());
						LWorld* world_server = world_list.FindByID(server_id);
						if(world_server != NULL)
							world_server->SendDeleteCharacter ( char_id , acct_id );
					}
					else
						response->setDataByName("response", 0);
					response->setDataByName("server_id", server_id);
					response->setDataByName("char_id", char_id);
					response->setDataByName("account_id", account_id);
					response->setMediumStringByName("name", (char*)name.data.c_str());
					response->setDataByName("max_characters", 10);

					EQ2Packet* outapp = response->serialize();
					QueuePacket(outapp);
					
					this->SendCharList();
				}
				safe_delete(request);
				safe_delete(response);
				break;
											  }
			default: {
				const char* name = app->GetOpcodeName();
				if (name)
					LogWrite(OPCODE__DEBUG, 1, "Opcode", "%s Received %04X (%i)", name, app->GetRawOpcode(), app->GetRawOpcode());
				else
					LogWrite(OPCODE__DEBUG, 1, "Opcode", "Received %04X (%i)", app->GetRawOpcode(), app->GetRawOpcode());
					 }
			}
			delete app;
		}
	}

	if (!eqnc->CheckActive()) {
		return false;
	}

	return true;
}

void Client::SaveErrorsToDB(EQApplicationPacket* app, char* type, int32 version){
	int32 size = 0;
	z_stream zstream;
	if (version >= 546) {
		memcpy(&size, app->pBuffer + sizeof(int32), sizeof(int32));
		zstream.next_in = app->pBuffer + 8;
		zstream.avail_in = app->size - 8;
	}
	else { //box set
		size = 0xFFFF;
		zstream.next_in = app->pBuffer + 2;
		zstream.avail_in = app->size - 2;
	}
	size++;
	char* message = new char[size];
	memset(message, 0, size);
	
	int zerror = 0;
	
	zstream.next_out	= (BYTE*)message;
	zstream.avail_out	= size;
	zstream.zalloc    = Z_NULL;
	zstream.zfree     = Z_NULL;
	zstream.opaque		= Z_NULL;

	zerror = inflateInit( &zstream); 
	if(zerror != Z_OK) {
		safe_delete_array(message);
		return;
	}
	zerror = inflate( &zstream, 0 );
	if(message && strlen(message) > 0)
		database.SaveClientLog(type, message, GetLoginAccount()->getLoginName(), GetVersion());
	safe_delete_array(message);
}

void Client::CharacterApproved(int32 server_id,int32 char_id)
{
	if(createRequest && server_id == createRequest->getType_int32_ByName("server_id")){
		LWorld* world_server = world_list.FindByID(server_id);
		if(!world_server)
			return;

		PacketStruct* packet = configReader.getStruct("LS_CreateCharacterReply", GetVersion());
		if(packet){
			packet->setDataByName("account_id", GetAccountID());
			packet->setDataByName("unknown", 0xFFFFFFFF);
			packet->setDataByName("response", CREATESUCCESS_REPLY);
			packet->setMediumStringByName("name", (char*)createRequest->getType_EQ2_16BitString_ByName("name").data.c_str());
			EQ2Packet* outapp = packet->serialize();
			QueuePacket(outapp);
			safe_delete(packet);
			database.SaveCharacter(createRequest, GetLoginAccount(),char_id, GetVersion());

			// refresh characters for this account
			database.LoadCharacters(GetLoginAccount(), GetVersion());
			
			SendCharList();

			if (GetVersion() <= 561)
			{
				pending_play_char_id = char_id;
				ServerPacket* outpack = new ServerPacket(ServerOP_UsertoWorldReq, sizeof(UsertoWorldRequest_Struct));
				UsertoWorldRequest_Struct* req = (UsertoWorldRequest_Struct*)outpack->pBuffer;
				req->char_id = char_id;
				req->lsaccountid = GetAccountID();
				req->worldid = server_id;

				struct in_addr in;
				in.s_addr = GetIP();
				strcpy(req->ip_address, inet_ntoa(in));
				world_server->SendPacket(outpack);
				delete outpack;
			}
		}
	}
	else{
		cout << GetAccountName() << " received invalid CharacterApproval from server: " << server_id << endl;
	}
	safe_delete(createRequest);
}

void Client::CharacterRejected(int8 reason_number)
{
	PacketStruct* packet = configReader.getStruct("LS_CreateCharacterReply", GetVersion());
	if(createRequest && packet){
		packet->setDataByName("account_id", GetAccountID());
		int8 clientReasonNum = reason_number;
		// reason numbers change and instead of updating the world server
		// the login server will hold the up to date #'s
/*
		switch(reason_number)
		{
			// these error codes seem to be removed now, they shutdown the client rather immediately
			// for now we are just going to play a joke on them and say they can't create a new character.
		case INVALIDRACE_REPLY:
		case INVALIDGENDER_REPLY:
			clientReasonNum = 8;
			break;
		case BADNAMELENGTH_REPLY:
			clientReasonNum = 9;
			break;
		case NAMEINVALID_REPLY:
			clientReasonNum = 10;
			break;
		case NAMEFILTER_REPLY:
			clientReasonNum = 11;
			break;
		case NAMETAKEN_REPLY:
			clientReasonNum = 12;
			break;
		case OVERLOADEDSERVER_REPLY:
			clientReasonNum = 13;
			break;
		}
*/
		packet->setDataByName("response", clientReasonNum);
		packet->setMediumStringByName("name", "");
		EQ2Packet* outapp = packet->serialize();
		QueuePacket(outapp);
		safe_delete(packet);
	}
	/*LS_CreateCharacterReply reply(GetAccountID(), reason_number, create.profile.name.data);
	EQ2Packet* outapp = reply.serialize();
	QueuePacket(outapp);
	create.Clear();*/
}

void Client::SendCharList(){
	/*PacketStruct* packet = configReader.getStruct("LS_CreateCharacterReply");
	packet->setDataByName("account_id", GetAccountID());
	packet->setDataByName("response", reason_number);
	packet->setDataByName("name", &create.profile.name);
	EQ2Packet* outapp = packet->serialize();
	QueuePacket(outapp);
	safe_delete(packet);*/
	LogWrite(LOGIN__INFO, 0, "Login", "[%s] sending character list.", GetAccountName());
	LS_CharSelectList list;
	list.loadData(GetAccountID(), GetLoginAccount()->charlist, GetVersion()); 
	EQ2Packet* outapp = list.serialize(GetVersion());
	DumpPacket(outapp->pBuffer, outapp->size);
	QueuePacket(outapp);

}
void Client::SendLoginDeniedBadVersion(){
	EQ2Packet* app = new EQ2Packet(OP_LoginReplyMsg, 0, sizeof(LS_LoginResponse));
	LS_LoginResponse* ls_response = (LS_LoginResponse*)app->pBuffer;
	ls_response->reply_code = 6;
	ls_response->unknown03 = 0xFFFFFFFF;
	ls_response->unknown04 = 0xFFFFFFFF;
	QueuePacket(app);
	StartDisconnectTimer();
}
void Client::SendLoginDenied(){
	EQ2Packet* app = new EQ2Packet(OP_LoginReplyMsg, 0, sizeof(LS_LoginResponse));
	LS_LoginResponse* ls_response = (LS_LoginResponse*)app->pBuffer;
	ls_response->reply_code = 1;
	// reply_codes for AoM:
	/* 1 = Login rejected: Invalid username or password. Please try again.
	   2 = Login rejected: Server thinks your account is currently playing; you may have to wait "
              "a few minutes for it to clear, then try again
	   6 = Login rejected: The client's version does not match the server's. Please re-run the patcher.
	   7 = Login rejected: You have no scheduled playtimes.
	   8 = Your account does not have the features required to play on this server.
	   11 = The client's build does not match the server's. Please re-run the patcher.
	   12 = You must update your password in order to log in.  Pressing OK will op"
              "en your web browser to the SOE password management page
		Other Value > 1 = Login rejected for an unknown reason.
	 */
	ls_response->unknown03 = 0xFFFFFFFF;
	ls_response->unknown04 = 0xFFFFFFFF;
	QueuePacket(app);
	StartDisconnectTimer();
}

void Client::SendLoginAccepted(int32 account_id, int8 login_response) {
	PacketStruct* packet = configReader.getStruct("LS_LoginReplyMsg", GetVersion());
	int i = 0;
	if (packet)
	{
		packet->setDataByName("account_id", account_id);
		
		packet->setDataByName("login_response", login_response);
		
		packet->setDataByName("do_not_force_soga", 1);
		
		// sub_level 0xFFFFFFFF = blacks out all portraits for class alignments, considered non membership
		// sub_level > 0 = class alignments still required, but portraits are viewable and race selectable
		// sub_level = 2 membership, you can 'create characters on time locked servers' vs standard
		// sub_level = 0 forces popup on close to web browser
		packet->setDataByName("sub_level", net.GetDefaultSubscriptionLevel());
		packet->setDataByName("race_flag", 0x1FFFFF);
		packet->setDataByName("class_flag", 0x7FFFFFE);
		packet->setMediumStringByName("username", GetAccountName());
		packet->setMediumStringByName("password", GetAccountName());

		// unknown5
		// full support = 0x7CFF
		// 1 << 12 (-4096) = missing echoes of faydwer, disables Fae and Arasai (black portraits) and kelethin as starting city
		// 1 << 13 (-8192) = disables sarnak (black portraits) and gorowyn as starting city
		packet->setDataByName("unknown5", net.GetExpansionFlag());
		packet->setDataByName("unknown6", 0xFF);
		packet->setDataByName("unknown6", 0xFF, 1);
		packet->setDataByName("unknown6", 0xFF, 2);
		
		// controls class access / playable characters
		packet->setDataByName("unknown10", 0xFF);

	//	packet->setDataByName("unknown7a", 0x0101);
	//	packet->setDataByName("race_unknown", 0x01);
		packet->setDataByName("unknown7", net.GetEnabledRaces()); // 0x01-0xFF disable extra races FAE(16) ARASAI (17) SARNAK (18) -- with 4096/8192 flags, no visibility of portraits
		packet->setDataByName("unknown7a", 0xEE);
		packet->setDataByName("unknown8", net.GetCitiesFlag(), 1); // dword_1ECBA18 operand for race flag packs (sublevel 0,1,2?) -- (sublevel -1) controls starting zones omission 0xEE vs 0xCF (CF misses halas)

		/*
		1 = city of qeynos
		2 = city of freeport
		4 = city of kelethin
		8 = city of neriak
		16 = gorowyn
		32 = new halas
		64 = queens colony
		128 = outpost overlord
		*/

		EQ2Packet* outapp = packet->serialize();
		QueuePacket(outapp);
		safe_delete(packet);
	}
}

void Client::SendWorldList(){
	EQ2Packet* pack = world_list.MakeServerListPacket(lsadmin, version);
	EQ2Packet* dupe = pack->Copy();
	DumpPacket(dupe->pBuffer,dupe->size);
	QueuePacket(dupe);

	SendLoginAccepted(0, 10); // triggers a different code path in the client to set certain flags
	return;
}

void Client::QueuePacket(EQ2Packet* app){
	eqnc->EQ2QueuePacket(app);
}

void Client::WorldResponse(int32 worldid, int8 response, char* ip_address, int32 port, int32 access_key)
{
	LWorld* world = world_list.FindByID(worldid);
	if(world == 0) {
		FatalError(0);
		return;
	}
	if(response != 1){
		if(response == PLAY_ERROR_CHAR_NOT_LOADED){
			string pending_play_char_name = database.GetCharacterName(pending_play_char_id, worldid, GetAccountID());
			if(database.VerifyDelete(GetAccountID(), pending_play_char_id, pending_play_char_name.c_str())){
				GetLoginAccount()->removeCharacter((char*)pending_play_char_name.c_str(), GetVersion());
			}
		}
		FatalError(response);
		return;
	}

	PacketStruct* response_packet = configReader.getStruct("LS_PlayResponse", GetVersion());
	if(response_packet){
		safe_delete(playWaitTimer);
		response_packet->setDataByName("response", 1);
		response_packet->setSmallStringByName("server", ip_address);
		response_packet->setDataByName("port", port);
		response_packet->setDataByName("account_id", GetAccountID());
		response_packet->setDataByName("access_code", access_key);
		EQ2Packet* outapp = response_packet->serialize();
		QueuePacket(outapp);
		safe_delete(response_packet);
	}
	return;
}
void Client::FatalError(int8 response) {
	safe_delete(playWaitTimer);
	SendPlayFailed(response);
}

void Client::SendPlayFailed(int8 response){
	PacketStruct* response_packet = configReader.getStruct("LS_PlayResponse", GetVersion());
	if(response_packet){
		response_packet->setDataByName("response", response);
		response_packet->setSmallStringByName("server", "");
		response_packet->setDataByName("port", 0);
		response_packet->setDataByName("account_id", GetAccountID());
		response_packet->setDataByName("access_code", 0);
		EQ2Packet* outapp = response_packet->serialize();
		QueuePacket(outapp);
		safe_delete(response_packet);
	}
}

void ClientList::Add(Client* client) {
	MClientList.writelock();
	client_list[client] = true;
	MClientList.releasewritelock();
}

Client* ClientList::Get(int32 ip, int16 port) {
	Client* ret = 0;
	map<Client*, bool>::iterator itr;
	MClientList.readlock();
	for(itr = client_list.begin(); itr != client_list.end(); itr++){
		if(itr->first->GetIP() == ip && itr->first->GetPort() == port){
			ret = itr->first;
			break;
		}
	}
	MClientList.releasereadlock();
	return ret;
}

void ClientList::FindByCreateRequest(){
	Client* client = 0;
	map<Client*, bool>::iterator itr;
	MClientList.readlock();
	for(itr = client_list.begin(); itr != client_list.end(); itr++){
		if(itr->first->AwaitingCharCreationRequest()){
			if(!client)
				client = itr->first;
			else{
				client = 0;//more than 1 character waiting, dont want to send rejection to wrong one
				break;
			}
		}
	}
	MClientList.releasereadlock();
	if(client)
		client->CharacterRejected(UNKNOWNERROR_REPLY);
}

Client* ClientList::FindByLSID(int32 lsaccountid) {
	Client* client = 0;
	map<Client*, bool>::iterator itr;
	MClientList.readlock();
	for(itr = client_list.begin(); itr != client_list.end(); itr++){
		if(itr->first->GetAccountID() == lsaccountid){
			client = itr->first;
			break;
		}
	}
	MClientList.releasereadlock();
	return client;
}
void ClientList::SendPacketToAllClients(EQ2Packet* app){
	Client* client = 0;
	map<Client*, bool>::iterator itr;
	MClientList.readlock();
	if(client_list.size() > 0){
		for(itr = client_list.begin(); itr != client_list.end(); itr++){
			itr->first->QueuePacket(app->Copy());
		}
	}
	safe_delete(app);
	MClientList.releasereadlock();
}
void ClientList::Process() {
	Client* client = 0;
	vector<Client*> erase_list;
	map<Client*, bool>::iterator itr;
	MClientList.readlock();	
	for(itr = client_list.begin(); itr != client_list.end(); itr++){		
		client = itr->first;
		if(!client->Process())
			erase_list.push_back(client);		
	}
	MClientList.releasereadlock();
	if(erase_list.size() > 0){
		vector<Client*>::iterator erase_itr;
		MClientList.writelock();
		for(erase_itr = erase_list.begin(); erase_itr != erase_list.end(); erase_itr++){
			client = *erase_itr;					
			struct in_addr	in;
			in.s_addr = client->getConnection()->GetRemoteIP();
			net.numclients--;
			LogWrite(LOGIN__INFO, 0, "Login", "Removing client from ip: %s on port %i, Account Name: %s", inet_ntoa(in), ntohs(client->getConnection()->GetRemotePort()), client->GetAccountName());
			client->getConnection()->Close();
			net.UpdateWindowTitle();
			client_list.erase(client);
		}
		MClientList.releasewritelock();
	}
}


void Client::StartDisconnectTimer() {
	if (!disconnectTimer)
	{
		disconnectTimer = new Timer(1000);
		disconnectTimer->Start();
	}
}
