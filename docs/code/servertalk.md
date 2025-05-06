# File: `servertalk.h`

## Classes

- `ServerPacket`
- `GetLatestTables_Struct`
- `ServerLSInfo_Struct`
- `ServerLSStatus_Struct`
- `ServerSystemwideMessage`
- `ServerSyncWorldList_Struct`
- `UsertoWorldRequest_Struct`
- `UsertoWorldResponse_Struct`
- `ServerEncapPacket_Struct`
- `ServerEmoteMessage_Struct`
- `LatestTableVersions`
- `TableData`
- `TableQuery`
- `TableDataQuery`
- `EquipmentUpdateRequest_Struct`
- `LoginEquipmentUpdate`
- `EquipmentUpdate_Struct`
- `EquipmentUpdateList_Struct`
- `ZoneUpdateRequest_Struct`
- `LoginZoneUpdate`
- `ZoneUpdate_Struct`
- `ZoneUpdateList_Struct`
- `CharacterTimeStamp_Struct`
- `CharDataUpdate_Struct`
- `BugReport`
- `RaceUpdate_Struct`
- `CharNameUpdate_Struct`
- `CharZoneUpdate_Struct`
- `WorldCharCreate_Struct`
- `WorldCharNameFilter_Struct`
- `WorldCharNameFilterResponse_Struct`
- `CharPictureUpdate_Struct`

## Functions

- `bool Deflate() {`
- `bool Inflate() {`
- `void SetTableSize(int16 size){`
- `void AddTable(char* name, int32 version, int32 data_version){`
- `int16 GetTotalSize(){`
- `int16 GetTotalTables(){`
- `TableVersion GetTable(int16 index){`
- `string Serialize(){`
- `void DeSerialize(uchar* data){`
- `string GetQueriesString(){`
- `void AddQuery(char* query){`
- `int16 GetTotalSize(){`
- `int16 GetTotalQueries(){`
- `string Serialize(){`
- `void DeSerialize(uchar* data){`
- `int32 GetTotalQueries(){`
- `void DeSerialize(uchar* data){`

## Notable Comments

- /*
- */
- //EQ2 Opcodes
- /************ PACKET RELATED STRUCT ************/
- /*struct TableVersion{
- // Max number of equipment updates to send at once
- // Login's structure of equipment data
- // World's structure of equipment data
- // How many equipmment updates are there to send?
- //EQ2 Specific Structures Login -> World (Image)
