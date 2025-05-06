# File: `LWorld.h`

## Classes

- `LWorld`
- `LWorldList`

## Functions

- `void ServerUpdateLoop(void* tmp);`
- `bool	Process();`
- `void	SendPacket(ServerPacket* pack);`
- `void	Message(const char* to, const char* message, ...);`
- `bool	SetupWorld(char* in_worldname, char* in_worldaddress, char* in_account, char* in_password, char* in_version);`
- `void	UpdateStatus(sint32 in_status, sint32 in_players, sint32 in_zones, int8 in_level) {`
- `void	UpdateWorldList(LWorld* to = 0);`
- `void	SetRemoteInfo(int32 in_ip, int32 in_accountid, char* in_account, char* in_name, char* in_address, int32 in_status, int32 in_adminid, sint32 in_players, sint32 in_zones);`
- `bool			IsLocked()		{ return status==-2; }`
- `void	ShowDownActive(bool show){ show_down_active = show; }`
- `void	ShowDown(bool show){ pshowdown = show; }`
- `int8			GetWorldStatus();`
- `void			ChangeToPlaceholder();`
- `void			Kick(const char* message = ERROR_GHOST, bool iSetKickedFlag = true);`
- `void			SetID(int32 new_id)		{ ID = new_id; }`
- `void	SendDeleteCharacter( int32 char_id, int32 account_id );`
- `bool	IsDevelServer(){ return devel_server; }`
- `void	Add(LWorld* worldserver);`
- `void	AddInitiateWorld ( LWorld* world );`
- `void	Process();`
- `void	ReceiveData();`
- `void	SendPacket(ServerPacket* pack, LWorld* butnotme = 0);`
- `void	SendPacketLocal(ServerPacket* pack, LWorld* butnotme = 0);`
- `void	SendPacketLogin(ServerPacket* pack, LWorld* butnotme = 0);`
- `void	SendWorldChanged(int32 server_id, bool sendtoallclients=false, Client* sendto = 0);`
- `void	UpdateWorldList(LWorld* to = 0);`
- `void	UpdateWorldStats();`
- `void	KickGhost(ConType in_type, int32 in_accountid = 0, LWorld* ButNotMe = 0);`
- `void	KickGhostIP(int32 ip, LWorld* NotMe = 0, int16 iClientPort = 0);`
- `void	RemoveByLink(TCPConnection* in_link, int32 in_id = 0, LWorld* ButNotMe = 0);`
- `void	RemoveByID(int32 in_id);`
- `void	SendWorldStatus(LWorld* chat, char* adminname);`
- `void	ConnectUplink();`
- `bool	Init();`
- `void	InitWorlds();`
- `void	Shutdown();`
- `bool	WriteXML();`
- `int32	GetCount(ConType type);`
- `void	PopulateWorldList(http::response<http::string_body>& res);`
- `void	ListWorldsToConsole();`
- `void	AddServerEquipmentUpdates(LWorld* world, map<int32, LoginEquipmentUpdate> updates);`
- `void	ProcessLSEquipUpdates();`
- `void	RequestServerEquipUpdates(LWorld* world);`
- `void	SetUpdateServerList ( bool var ) { UpdateServerList = var; }`
- `bool	ContinueServerUpdates(){ return server_update_thread; }`
- `void	ResetServerUpdates(){server_update_thread = true;}`
- `void	ProcessServerUpdates();`
- `void	RequestServerUpdates(LWorld* world);`
- `void	AddServerZoneUpdates(LWorld* world, map<int32, LoginZoneUpdate> updates);`
- `int32	GetNextID()				{ return NextID++; }`

## Notable Comments

- /*
- */
- // we don't want the server list to update unless something has changed
- //devn00b temp
- //devn00b temp
- // JohnAdams: login appearances, copied from above
- //
- ///
- // holds the world server list so we don't have to create it for every character
- // logging in
