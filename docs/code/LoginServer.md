# File: `LoginServer.h`

## Classes

- `LoginServer`

## Functions

- `void AutoInitLoginServer(void *tmp);`
- `bool InitLoginServer();`
- `bool Process();`
- `bool Connect(const char* iAddress = 0, int16 iPort = 0);`
- `bool ConnectToUpdateServer(const char* iAddress = 0, int16 iPort = 0);`
- `void SendInfo();`
- `void SendStatus();`
- `void GetLatestTables();`
- `void SendPacket(ServerPacket* pack) { tcpc->SendPacket(pack); }`
- `int8 GetState() { return tcpc->GetState(); }`
- `bool Connected() { return tcpc->Connected(); }`
- `void SendFilterNameResponse ( int8 resp , int32 acct_id , int32 char_id );`
- `void SendDeleteCharacter ( CharacterTimeStamp_Struct* cts );`
- `int32 DetermineCharacterLoginRequest ( UsertoWorldRequest_Struct* utwr, ZoneChangeDetails* details, std::string name);`
- `void SendCharApprovedLogin(int8 response, std::string peerAddress, std::string peerInternalAddress, std::string clientIP, int16 peerPort, int32 account_id, int32 char_id, int32 key, int32 world_id, int32 from_id);`
- `void InitLoginServerVariables();`
- `void SendImmediateEquipmentUpdatesForChar(int32 char_id);`
- `bool CanReconnect() { return pTryReconnect; }`

## Notable Comments

- /*
- */
