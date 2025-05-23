# File: `LoginDatabase.h`

## Classes

- `LoginDatabase`

## Functions

- `void FixBugReport();`
- `void UpdateAccountIPAddress(int32 account_id, int32 address);`
- `void UpdateWorldIPAddress(int32 world_id, int32 address);`
- `void SaveBugReport(int32 world_id, char* category, char* subcategory, char* causes_crash, char* reproducible, char* summary, char* description, char* version, char* player, int32 account_id, char* spawn_name, int32 spawn_id, int32 zone_id);`
- `int32 GetAccountIDByName(const char* name);`
- `int32 CheckServerAccount(char* name, char* passwd);`
- `bool IsServerAccountDisabled(char* name);`
- `bool IsIPBanned(char* ipaddr);`
- `void  GetServerAccounts(vector<LWorld*>* server_list);`
- `bool  VerifyDelete(int32 account_id, int32 character_id, const char* name);`
- `void SetServerZoneDescriptions(int32 server_id, map<int32, LoginZoneUpdate> zone_descriptions);`
- `int32 GetServer(int32 accountID, int32 charID, string name);`
- `void LoadCharacters(LoginAccount* acct, int16 version);`
- `void CheckCharacterTimeStamps(LoginAccount* acct);`
- `string GetCharacterName(int32 char_id , int32 server_id, int32 account_id);`
- `void SaveCharacterColors(int32 char_id, char* type, EQ2_Color color);`
- `void SaveCharacterFloats(int32 char_id, char* type, float float1, float float2, float float3, float multiplier=100.0f);`
- `int16 GetAppearanceID(string name);`
- `void DeactivateCharID(int32 server_id, int32 char_id, int32 exception_id);`
- `int32 SaveCharacter(PacketStruct* create, LoginAccount* acct, int32 world_charid, int32 client_version);`
- `void LoadAppearanceData(int32 char_id, PacketStruct* char_select_packet);`
- `bool UpdateCharacterTimeStamp(int32 account_id, int32 character_id, int32 timestamp_update, int32 server_id);`
- `bool UpdateCharacterLevel(int32 account_id, int32 character_id, int8 in_level, int32 server_id);`
- `bool UpdateCharacterRace(int32 account_id, int32 character_id, int16 in_racetype, int8 in_race, int32 server_id);`
- `bool UpdateCharacterClass(int32 account_id, int32 character_id, int8 in_class, int32 server_id);`
- `bool UpdateCharacterName(int32 account_id, int32 character_id, char* newName, int32 server_id);`
- `bool UpdateCharacterZone(int32 account_id, int32 character_id, int32 zone_id, int32 server_id);`
- `bool UpdateCharacterGender(int32 account_id, int32 character_id, int8 in_gender, int32 server_id);`
- `int32 GetRaceID(char* name);`
- `void UpdateRaceID(char* name);`
- `bool DeleteCharacter(int32 account_id, int32 character_id, int32 server_id);`
- `void SaveClientLog(const char* type, const char* message, const char* player_name, int16 version);`
- `bool CheckVersion(char* version);`
- `void GetLatestTableVersions(LatestTableVersions* table_versions);`
- `bool VerifyDataTable(char* name);`
- `sint16 GetDataVersion(char* name);`
- `void SetZoneInformation(int32 server_id, int32 zone_id, int32 version, PacketStruct* packet);`
- `string GetZoneDescription(char* name);`
- `string GetColumnNames(char* name);`
- `void UpdateWorldServerStats( LWorld* world, sint32 status);`
- `bool ResetWorldServerStatsConnectedTime( LWorld* world );`
- `void RemoveOldWorldServerStats();`
- `void ResetWorldStats();`
- `bool ConnectNewDatabase();`
- `void SetServerEquipmentAppearances(int32 server_id, map<int32, LoginEquipmentUpdate> equip_updates); // JohnAdams: login appearances`
- `int32 GetLoginCharacterIDFromWorldCharID(int32 server_id, int32 char_id); // JohnAdams: login appearances`
- `void RemoveDeletedCharacterData();`
- `int8 GetMaxCharsSetting();`
- `int16 GetAccountBonus(int32 acct_id);`
- `void UpdateWorldVersion(int32 world_id, char* version);`
- `void UpdateAccountClientDataVersion(int32 account_id, int16 version);`
- `void SaveCharacterPicture(int32 account_id, int32 character_id, int32 server_id, int16 picture_size, uchar* picture);`

## Notable Comments

- /*
- */
- //devn00b temp
