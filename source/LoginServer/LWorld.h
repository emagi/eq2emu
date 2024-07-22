/*  
	EQ2Emulator:  Everquest II Server Emulator
	Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

	This file is part of EQ2Emulator.
*/
#ifndef LWORLD_H
#define LWORLD_H

#include "../common/Mutex.h"

#define ERROR_BADPASSWORD		"Bad password"
#define	INVALID_ACCOUNT			"Invalid Server Account."
#define ERROR_BADVERSION		"Incorrect version"
#define ERROR_UNNAMED			"Unnamed servers not allowed to connect to full login servers"
#define ERROR_NOTMASTER			"Not a master server"
#define ERROR_NOTMESH			"Not a mesh server"
#define ERROR_GHOST				"Ghost kick"
#define ERROR_UnknownServerType	"Unknown Server Type"
#define ERROR_BADNAME_SERVER	"Bad server name, name may not contain the word \"Server\" (case sensitive)"
#define ERROR_BADNAME			"Bad server name. Unknown reason."

#define WORLD_NAME_SUFFIX		" Server"

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/beast/http.hpp>
#include <sstream>
#include <string>
#include <iostream>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>

#include "../common/linked_list.h"
#include "../WorldServer/MutexList.h"
#include "../WorldServer/MutexMap.h"
#include "../common/timer.h"
#include "../common/types.h"
#include "../common/queue.h"
#include "../common/servertalk.h"
#include "../common/TCPConnection.h"
#include "client.h"

#define MAX_UPDATE_COUNT	20
#define MAX_LOGIN_APPEARANCE_COUNT	100

#ifdef WIN32
	void ServerUpdateLoop(void* tmp);
#else
	void* ServerUpdateLoop(void* tmp);
#endif

enum ConType { UnknownW, World, Chat, Login };

class LWorld
{
public:
	LWorld(TCPConnection* in_con, bool OutgoingLoginUplink = false, int32 iIP = 0, int16 iPort = 0, bool iNeverKick = false);
	LWorld(int32 in_accountid, char* in_accountname, char* in_worldname, int32 in_admin_id);
	LWorld(TCPConnection* in_RemoteLink, int32 in_ip, int32 in_RemoteID, int32 in_accountid, char* in_accountname, char* in_worldname, char* in_address, sint32 in_status, int32 in_adminid, bool in_showdown, int8 in_authlevel, bool in_placeholder, int32 iLinkWorldID);
    ~LWorld();

	static bool CheckServerName(const char* name);
	
	bool	Process();
	void	SendPacket(ServerPacket* pack);
	void	Message(const char* to, const char* message, ...);

	bool	SetupWorld(char* in_worldname, char* in_worldaddress, char* in_account, char* in_password, char* in_version);
	void	UpdateStatus(sint32 in_status, sint32 in_players, sint32 in_zones, int8 in_level) {
		// we don't want the server list to update unless something has changed
		if(status != in_status || num_players != in_players || num_zones != in_zones || world_max_level != in_level)
		{
		status = in_status;
		num_players = in_players;
		num_zones = in_zones;
		world_max_level = in_level;
		UpdateWorldList();
		}
	}
	void	UpdateWorldList(LWorld* to = 0);
	void	SetRemoteInfo(int32 in_ip, int32 in_accountid, char* in_account, char* in_name, char* in_address, int32 in_status, int32 in_adminid, sint32 in_players, sint32 in_zones);

	inline bool	IsPlaceholder()		{ return pPlaceholder; }
	inline int32	GetAccountID()	{ return accountid; }
	inline char*	GetAccount()	{ return account; }
	inline char*	GetAddress()	{ return address; }
	inline int16	GetClientPort()	{ return pClientPort; }
	inline bool		IsAddressIP()	{ return isaddressip; }
	inline char*	GetName()		{ return worldname; }
	inline sint32	GetStatus()		{ return status; }
	bool			IsLocked()		{ return status==-2; }
	inline int32	GetIP()			{ return ip; }
	inline int16	GetPort()		{ return port; }
	inline int32	GetID()			{ return ID; }
	inline int32	GetAdmin()		{ return admin_id; }
	inline bool		ShowDown()		{ return pshowdown; }
	inline bool		ShowDownActive(){ return show_down_active; }
	void	ShowDownActive(bool show){ show_down_active = show; }
	void	ShowDown(bool show){ pshowdown = show; }
	inline bool		Connected()		{ return pConnected; }
	int8			GetWorldStatus();

	void			ChangeToPlaceholder();
	void			Kick(const char* message = ERROR_GHOST, bool iSetKickedFlag = true);
	inline bool		IsKicked()				{ return kicked; }
	inline bool		IsNeverKick()			{ return pNeverKick; }
	inline  ConType	GetType()				{ return ptype; }
	inline  bool	IsOutgoingUplink()		{ return OutgoingUplink; }
	inline TCPConnection*	GetLink()		{ return Link; }
	inline int32	GetRemoteID()			{ return RemoteID; }
	inline int32	GetLinkWorldID()		{ return LinkWorldID; }
	inline sint32	GetZoneNum()			{ return num_zones; }
	inline sint32	GetPlayerNum()			{ return num_players; }
	void			SetID(int32 new_id)		{ ID = new_id; }

	void	SendDeleteCharacter( int32 char_id, int32 account_id );
	bool	IsDevelServer(){ return devel_server; }
	
	inline int8		GetMaxWorldLevel() { return world_max_level; }

	bool	IsInit;
protected:
	friend class LWorldList;
	TCPConnection*	Link;
	Timer*	pReconnectTimer;
	Timer*  pStatsTimer;
private:
	int32	ID;
	int32	ip;
	char	IPAddr[64];
	int16	port;
	bool	kicked;
	bool	pNeverKick;
	bool	pPlaceholder;
	bool	devel_server;

	int32	accountid;
	char	account[30];
	char	address[250];
	bool	isAuthenticated;
	int16	pClientPort;
	bool	isaddressip;
	char	worldname[200];
	sint32	status;
	int32	admin_id;
	bool	pshowdown;
	bool	show_down_active;
	ConType	ptype;
	bool	OutgoingUplink;
	bool	pConnected;
	sint32  num_players;
	sint32  num_zones;
	int32	RemoteID;
	int32	LinkWorldID;
	int8	world_max_level;

};

class LWorldList
{
public:

	LWorldList();
	~LWorldList();

	LWorld*	FindByID(int32 WorldID);		
	LWorld*	FindByIP(int32 ip);
	LWorld*	FindByAddress(char* address);
	LWorld*	FindByLink(TCPConnection* in_link, int32 in_id);
	LWorld*	FindByAccount(int32 in_accountid, ConType in_type = World);

	void	Add(LWorld* worldserver);
	void	AddInitiateWorld ( LWorld* world );
	void	Process();
	void	ReceiveData();
	void	SendPacket(ServerPacket* pack, LWorld* butnotme = 0);
	void	SendPacketLocal(ServerPacket* pack, LWorld* butnotme = 0);
	void	SendPacketLogin(ServerPacket* pack, LWorld* butnotme = 0);
	void	SendWorldChanged(int32 server_id, bool sendtoallclients=false, Client* sendto = 0);
	vector<PacketStruct*>* GetServerListUpdate(int16 version);
	EQ2Packet*	MakeServerListPacket(int8 lsadmin, int16 version);

	void	UpdateWorldList(LWorld* to = 0);
	void	UpdateWorldStats();
	void	KickGhost(ConType in_type, int32 in_accountid = 0, LWorld* ButNotMe = 0);
	void	KickGhostIP(int32 ip, LWorld* NotMe = 0, int16 iClientPort = 0);
	void	RemoveByLink(TCPConnection* in_link, int32 in_id = 0, LWorld* ButNotMe = 0);
	void	RemoveByID(int32 in_id);

	void	SendWorldStatus(LWorld* chat, char* adminname);

	void	ConnectUplink();
	bool	Init();
	void	InitWorlds();
	void	Shutdown();
	bool	WriteXML();
	
	int32	GetCount(ConType type);
	void	PopulateWorldList(http::response<http::string_body>& res);

	void	ListWorldsToConsole();
	//devn00b temp
	void	AddServerEquipmentUpdates(LWorld* world, map<int32, LoginEquipmentUpdate> updates);
	void	ProcessLSEquipUpdates();
	void	RequestServerEquipUpdates(LWorld* world);

	void	SetUpdateServerList ( bool var ) { UpdateServerList = var; }
	bool	ContinueServerUpdates(){ return server_update_thread; }
	void	ResetServerUpdates(){server_update_thread = true;}
	void	ProcessServerUpdates();
	void	RequestServerUpdates(LWorld* world);
	void	AddServerZoneUpdates(LWorld* world, map<int32, LoginZoneUpdate> updates);

protected:
	friend class LWorld;
	int32	GetNextID()				{ return NextID++; }

private:
	Mutex	MWorldMap;
	map<int32, map<int32, bool> > zone_updates_already_used; //used to determine if someone is trying to DOS us
	MutexMap<int32, int32> zone_update_timeouts;
	MutexMap<int32, int32> awaiting_zone_update;
	MutexMap<LWorld*, int32> last_updated;
	MutexMap<int32, map<int32, LoginZoneUpdate> > server_zone_updates;
	bool server_update_thread;
	int32	NextID;

	LinkedList<LWorld*>	list;
	
	map<int32,LWorld*> worldmap;

	TCPServer*				tcplistener;
	TCPConnection*			OutLink;

	//devn00b temp
	// JohnAdams: login appearances, copied from above
	map<int32, map<int32, bool> > equip_updates_already_used;
	MutexMap<int32, int32> equip_update_timeouts;
	MutexMap<int32, int32> awaiting_equip_update;
	MutexMap<LWorld*, int32> last_equip_updated;
	MutexMap<int32, map<int32, LoginEquipmentUpdate> > server_equip_updates;
	//
	///

	// holds the world server list so we don't have to create it for every character
	// logging in
	map<int32,EQ2Packet*> ServerListData;
	bool UpdateServerList;
};
#endif
