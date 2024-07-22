/*  
	EQ2Emulator:  Everquest II Server Emulator
	Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

	This file is part of EQ2Emulator.
*/
#ifndef CLIENT_H
#define CLIENT_H

#include "../common/linked_list.h"
#include "../common/timer.h"
#include "../common/TCPConnection.h"
#include "login_structs.h"
#include "LoginAccount.h"
#include "../common/PacketStruct.h"
#include <string>
#include <vector>

enum eLoginMode { None, Normal, Registration };
class DelayQue;
class Client
{
public:
	Client(EQStream* ieqnc);
    ~Client();
	void	SendLoginDenied();
	void	SendLoginDeniedBadVersion();
	void	SendLoginAccepted(int32 account_id = 1, int8 login_response = 0);
	void	SendWorldList();
	void	SendCharList();
	int16	AddWorldToList2(uchar* buffer, char* name, int32 id, int16* flags);
	void	GenerateChecksum(EQApplicationPacket* outapp);
	int8	LoginKey[10];	
	int8	ClientSession[25];
	bool	Process();
	void	SaveErrorsToDB(EQApplicationPacket* app, char* type, int32 version);
	void	CharacterApproved(int32 server_id,int32 char_id);
	void	CharacterRejected(int8 reason_number);
	EQStream* getConnection() { return eqnc; }
	LoginAccount* GetLoginAccount() { return login_account; }
	void	SetLoginAccount(LoginAccount* in_account) { 
		login_account = in_account; 
		if(in_account)
			account_id = in_account->getLoginAccountID();
	}
	int16	GetVersion(){ return version; }
	char*	GetKey()   { return key; }
	void	SetKey(char* in_key) { strcpy(key,in_key); }
	int32	GetIP()    { return ip; }
	int16	GetPort()  { return port; }
	int32	GetAccountID() { return account_id; }
	const char*	GetAccountName(){ return (char*)account_name.c_str(); }
	void	SetAccountName(const char* name){ account_name = string(name); }
	void	ProcessLogin(char* name, char* pass,int seq=0);
	void	QueuePacket(EQ2Packet* app);
	void	FatalError(int8 response);
	void	WorldResponse(int32 worldid, int8 response, char* ip_address, int32 port, int32 access_key);
	bool	AwaitingCharCreationRequest(){
		if(createRequest)
			return true;
		else
			return false;
	}
	Timer*	updatetimer;
	Timer*	updatelisttimer;
	Timer*	disconnectTimer;
	//Timer*  keepalive;
	//Timer*	logintimer;
	int16	packettotal;
	int32	requested_server_id;
	int32	request_num;
	LinkedList<DelayQue*> delay_que;
	void SendPlayFailed(int8 response);

	void StartDisconnectTimer();
private:
	string	pending_play_char_name;
	int32	pending_play_char_id;
	int8	update_position;
	int16	num_updates;
	vector<PacketStruct*>* update_packets;
	LoginAccount* login_account;
	EQStream* eqnc;

	int32	ip;
	int16	port;

	int32	account_id;
	string	account_name;
	char	key[10];
	int8	lsadmin;
	sint16	worldadmin;
	int		lsstatus;
	bool	kicked;
	bool	verified;
	bool	start;
	bool	needs_world_list;
	int16	version;
	char	bannedreason[30];
	bool	sent_character_list;
	eLoginMode LoginMode;
	PacketStruct* createRequest;
	Timer* playWaitTimer;
};

class ClientList
{
public:
	ClientList()	{}
	~ClientList()	{}
	
	void	Add(Client* client);
	Client*	Get(int32 ip, int16 port);
	Client* FindByLSID(int32 lsaccountid);
	void	FindByCreateRequest();
	void	SendPacketToAllClients(EQ2Packet* app);
	void	Process();
private:
	Mutex	MClientList;
	map<Client*, bool> client_list;
};
class DelayQue {
public:
	DelayQue(Timer* in_timer, EQApplicationPacket* in_packet){
		timer = in_timer;
		packet = in_packet;
	};
	Timer* timer;
	EQApplicationPacket* packet;
};
#endif
