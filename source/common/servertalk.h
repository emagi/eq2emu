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
#ifndef EQ_SOPCODES_H
#define EQ_SOPCODES_H

#define EQEMU_PROTOCOL_VERSION	"0.5.0"

#include "types.h"
#include "packet_functions.h"
#include <vector>

#define SERVER_TIMEOUT	45000	// how often keepalive gets sent
#define INTERSERVER_TIMER					10000
#define LoginServer_StatusUpdateInterval	15000
#define LoginServer_AuthStale				60000
#define AUTHCHANGE_TIMEOUT					900	// in seconds

#define ServerOP_KeepAlive			0x0001	// packet to test if port is still open
#define ServerOP_ChannelMessage		0x0002	// broadcast/guildsay
#define ServerOP_SetZone			0x0003	// client -> server zoneinfo
#define ServerOP_ShutdownAll		0x0004	// exit(0);
#define ServerOP_ZoneShutdown		0x0005	// unload all data, goto sleep mode
#define ServerOP_ZoneBootup			0x0006	// come out of sleep mode and load zone specified
#define ServerOP_ZoneStatus			0x0007	// Shows status of all zones
#define ServerOP_SetConnectInfo		0x0008	// Tells server address and port #
#define ServerOP_EmoteMessage		0x0009	// Worldfarts
#define ServerOP_ClientList			0x000A	// Update worldserver's client list, for #whos
#define ServerOP_Who				0x000B	// #who
#define ServerOP_ZonePlayer			0x000C  // #zone, or #summon
#define ServerOP_KickPlayer			0x000D  // #kick
#define ServerOP_RefreshGuild		0x000E	// Notice to all zoneservers to refresh their guild cache for ID# in packet
#define ServerOP_GuildKickAll		0x000F	// Remove all clients from this guild
#define ServerOP_GuildInvite		0x0010
#define ServerOP_GuildRemove		0x0011
#define ServerOP_GuildPromote		0x0012
#define ServerOP_GuildDemote		0x0013
#define ServerOP_GuildLeader		0x0014
#define ServerOP_GuildGMSet			0x0015
#define ServerOP_GuildGMSetRank		0x0016
#define ServerOP_FlagUpdate			0x0018	// GM Flag updated for character, refresh the memory cache
#define ServerOP_GMGoto				0x0019
#define ServerOP_MultiLineMsg		0x001A
#define ServerOP_Lock				0x001B  // For #lock/#unlock inside server
#define ServerOP_Motd				0x001C  // For changing MoTD inside server.
#define ServerOP_Uptime				0x001D
#define ServerOP_Petition			0x001E
#define	ServerOP_KillPlayer			0x001F
#define ServerOP_UpdateGM			0x0020
#define ServerOP_RezzPlayer			0x0021
#define ServerOP_ZoneReboot			0x0022
#define ServerOP_ZoneToZoneRequest	0x0023
#define ServerOP_AcceptWorldEntrance 0x0024
#define ServerOP_ZAAuth				0x0025
#define ServerOP_ZAAuthFailed		0x0026
#define ServerOP_ZoneIncClient		0x0027	// Incomming client
#define ServerOP_ClientListKA		0x0028
#define ServerOP_ChangeWID			0x0029
#define ServerOP_IPLookup			0x002A
#define ServerOP_LockZone			0x002B
#define ServerOP_ItemStatus			0x002C
#define ServerOP_OOCMute			0x002D
#define ServerOP_Revoke				0x002E
#define ServerOP_GuildJoin			0x002F
#define ServerOP_GroupIDReq			0x0030
#define ServerOP_GroupIDReply		0x0031
#define ServerOP_GroupLeave			0x0032	// for disbanding out of zone folks
#define ServerOP_RezzPlayerAccept	0x0033
#define ServerOP_SpawnCondition		0x0034
#define ServerOP_SpawnEvent			0x0035

#define UpdateServerOP_Verified		0x5090
#define UpdateServerOP_DisplayMsg	0x5091
#define UpdateServerOP_Completed	0x5092

#define ServerOP_LSInfo				0x1000
#define ServerOP_LSStatus			0x1001
#define ServerOP_LSClientAuth		0x1002
#define ServerOP_LSFatalError		0x1003
#define ServerOP_SystemwideMessage	0x1005
#define ServerOP_ListWorlds			0x1006
#define ServerOP_PeerConnect		0x1007

#define ServerOP_LSZoneInfo			0x3001
#define ServerOP_LSZoneStart		0x3002
#define ServerOP_LSZoneBoot			0x3003
#define ServerOP_LSZoneShutdown		0x3004
#define ServerOP_LSZoneSleep		0x3005
#define ServerOP_LSPlayerLeftWorld	0x3006
#define ServerOP_LSPlayerJoinWorld	0x3007
#define ServerOP_LSPlayerZoneChange	0x3008

#define	ServerOP_UsertoWorldReq		0xAB00
#define	ServerOP_UsertoWorldResp	0xAB01

#define ServerOP_EncapPacket		0x2007	// Packet within a packet
#define ServerOP_WorldListUpdate	0x2008
#define ServerOP_WorldListRemove	0x2009
#define ServerOP_TriggerWorldListRefresh	0x200A

#define ServerOP_WhoAll				0x0210

#define ServerOP_SetWorldTime		0x200B
#define ServerOP_GetWorldTime		0x200C
#define ServerOP_SyncWorldTime		0x200E

//EQ2 Opcodes
#define ServerOP_CharTimeStamp		0x200F

#define ServerOP_NameFilterCheck	0x2011
#define ServerOP_BasicCharUpdate	0x2012
#define ServerOP_CharacterCreate	0x2013
#define ServerOP_NameCharUpdate		0x2014
#define ServerOP_GetLatestTables	0x2015
#define ServerOP_GetTableQuery		0x2016
#define ServerOP_GetTableData		0x2017
#define ServerOP_RaceUpdate			0x2018
#define ServerOP_ZoneUpdate			0x2019
#define ServerOP_BugReport			0x201A
#define ServerOP_ResetDatabase		0x201B
#define ServerOP_ZoneUpdates		0x201C
#define ServerOP_LoginEquipment		0x201D // updates charater select screen item appearances (gear appear)
#define ServerOP_CharacterPicture	0x201E


/************ PACKET RELATED STRUCT ************/
class ServerPacket
{
public:
	~ServerPacket() { safe_delete_array(pBuffer); }
    ServerPacket(int16 in_opcode = 0, int32 in_size = 0) {
		this->compressed = false;
		size = in_size;
		opcode = in_opcode;
		if (size == 0) {
			pBuffer = 0;
		}
		else {
			pBuffer = new uchar[size];
			memset(pBuffer, 0, size);
		}
		destination = 0;
		InflatedSize = 0;
	}
	ServerPacket* Copy() {
		if (this == 0) {
			return 0;
		}
		ServerPacket* ret = new ServerPacket(this->opcode, this->size);
		if (this->size)
			memcpy(ret->pBuffer, this->pBuffer, this->size);
		ret->compressed = this->compressed;
		ret->InflatedSize = this->InflatedSize;
		return ret;
	}
	bool Deflate() {
		if (compressed)
			return false;
		if ((!this->pBuffer) || (!this->size))
			return false;
		uchar* tmp = new uchar[this->size + 128];
		int32 tmpsize = DeflatePacket(this->pBuffer, this->size, tmp, this->size + 128);
		if (!tmpsize) {
			safe_delete_array(tmp);
			return false;
		}
		this->compressed = true;
		this->InflatedSize = this->size;
		this->size = tmpsize;
		uchar* new_buffer = new uchar[this->size];
		memcpy(new_buffer, tmp, this->size);
		safe_delete_array(tmp);
		uchar* tmpdel = this->pBuffer;
		this->pBuffer = new_buffer;
		safe_delete_array(tmpdel);
		return true;
	}
	bool Inflate() {
		if (!compressed)
			return false;
		if ((!this->pBuffer) || (!this->size))
			return false;
		uchar* tmp = new uchar[InflatedSize];
		int32 tmpsize = InflatePacket(this->pBuffer, this->size, tmp, InflatedSize);
		if (!tmpsize) {
			safe_delete_array(tmp);
			return false;
		}
		compressed = false;
		this->size = tmpsize;
		uchar* tmpdel = this->pBuffer;
		this->pBuffer = tmp;
		safe_delete_array(tmpdel);
		return true;
	}
	int32	size;
	int16	opcode;
	uchar*	pBuffer;
	bool	compressed;
	int32	InflatedSize;
	int32	destination;
};

#pragma pack(1)

struct GetLatestTables_Struct{
	float	table_version;
	float	data_version;
};

struct ServerLSInfo_Struct {
	char	name[201];				// name the worldserver wants
	char	address[250];			// DNS address of the server
	char	account[31];			// account name for the worldserver
	char	password[256];			// password for the name
	char	protocolversion[25];	// Major protocol version number
	char	serverversion[64];		// minor server software version number
	int8	servertype;				// 0=world, 1=chat, 2=login, 3=MeshLogin, 4=World Debug
	int32	dbversion;				// database major+minor version from version.h (for PatchServer)
};

struct ServerLSStatus_Struct {
	sint32 status;
	sint32 num_players;
	sint32 num_zones;
	int8 world_max_level;
};

struct ServerSystemwideMessage {
	int32	lsaccount_id;
	char	key[30];		// sessionID key for verification
	int32	type;
	char	message[0];
};

struct ServerSyncWorldList_Struct {
	int32	RemoteID;
	int32	ip;
	sint32	status;
	char	name[201];
	char	address[250];
	char	account[31];
	int32	accountid;
	int8	authlevel;
	int8	servertype;		// 0=world, 1=chat, 2=login
	int32	adminid;
	int8	showdown;
	sint32  num_players;
	sint32  num_zones;
	bool	placeholder;
};

struct UsertoWorldRequest_Struct {
	int32	lsaccountid;
	int32	char_id;
	int32	worldid;
	int32	FromID;
	int32	ToID;
	char	ip_address[21];
};

struct UsertoWorldResponse_Struct {
	int32	lsaccountid;
	int32	char_id;
	int32	worldid;
	int32	access_key;
	int8	response;
	char	ip_address[80];
	int32	port;
	int32	FromID;
	int32	ToID;
};

struct ServerEncapPacket_Struct {
	int32	ToID;	// ID number of the LWorld on the other server
	int16	opcode;
	int16	size;
	uchar	data[0];
};

struct ServerEmoteMessage_Struct {
	char	to[64];
	int32	guilddbid;
	sint16	minstatus;
	int32	type;
	char	message[0];
};

/*struct TableVersion{
	char	name[64];
	int32	version;
	int32	max_table_version;
	int32	max_data_version;
	sint32	data_version;
	int8	last;
	char	column_names[1000];
};*/

typedef struct {
	char name[256];
	unsigned int name_len;
	unsigned int version;
	unsigned int data_version;
} TableVersion;

template<class Type> void AddPtrData(string* buffer, Type& data){
	buffer->append((char*)&data, sizeof(Type));
}
template<class Type> void AddPtrData(string* buffer, Type* data, int16 size){
	buffer->append(data, size);
}
class LatestTableVersions {
public:
	LatestTableVersions(){
		tables = 0;
		current_index = 0;
		total_tables = 0;
		data_version = 0;
	}
	~LatestTableVersions(){
		safe_delete_array(tables);
	}
	void SetTableSize(int16 size){ 
		total_tables = size;
		tables = new TableVersion[total_tables];
	}
	void AddTable(char* name, int32 version, int32 data_version){
		strcpy(tables[current_index].name, name);
		tables[current_index].version = version;
		tables[current_index].data_version = data_version;
		current_index++;
	}
	int16 GetTotalSize(){
		return total_tables * sizeof(TableVersion) + sizeof(int16);
	}
	int16 GetTotalTables(){
		return total_tables;
	}
	TableVersion* GetTables(){
		return tables;
	}
	TableVersion GetTable(int16 index){
		return tables[index];
	}
	string Serialize(){
		AddPtrData(&buffer, total_tables);
		for(int16 i=0;i<total_tables;i++){
			AddPtrData(&buffer, tables[i].name, sizeof(tables[i].name));
			AddPtrData(&buffer, tables[i].version);
			AddPtrData(&buffer, tables[i].data_version);
		}
		return buffer;
	}
	void DeSerialize(uchar* data){
		uchar* ptr = data;
		memcpy(&total_tables, ptr, sizeof(total_tables));
		ptr+= sizeof(total_tables);
		tables = new TableVersion[total_tables];
		for(int16 i=0;i<total_tables;i++){
			memcpy(&tables[i].name, ptr, sizeof(tables[i].name));
			ptr+= sizeof(tables[i].name);
			memcpy(&tables[i].version, ptr, sizeof(tables[i].version));
			ptr+= sizeof(tables[i].version);
			memcpy(&tables[i].data_version, ptr, sizeof(tables[i].data_version));
			ptr+= sizeof(tables[i].data_version);
		}
	}
	int32			data_version;
private:
	int16			current_index;
	int16			total_tables;
	TableVersion*	tables;
	string			buffer;
};
struct TableData{
	int16	size;
	char*	query;
};
class TableQuery {
public:
	TableQuery(){
		try_delete = true;
		num_queries = 0;
		data_version = 0;
		current_index = 0;
		latest_version = 0;
		your_version = 0;
		total_size = sizeof(num_queries) + sizeof(latest_version) + sizeof(your_version) + sizeof(tablename);
	}
	~TableQuery(){
		if(try_delete){
			for(int16 i=0;i<tmp_queries.size();i++){
				safe_delete_array(tmp_queries[i]);
			}
		}
	}
	string GetQueriesString(){
		string query_string ;
		for(int32 i=0;i<tmp_queries.size();i++){
			query_string.append(tmp_queries[i]).append("\n");
		}
		return query_string;
	}
	void AddQuery(char* query){
		num_queries++;
		total_size += strlen(query) + 1;
		tmp_queries.push_back(query);
	}
	int16 GetTotalSize(){
		return total_size;
	}
	int16 GetTotalQueries(){
		return num_queries;
	}
	char* GetQuery(int16 index){
		return tmp_queries[index];
	}
	string Serialize(){
		num_queries = tmp_queries.size();
		AddPtrData(&buffer, num_queries);
		AddPtrData(&buffer, latest_version);
		AddPtrData(&buffer, your_version);
		AddPtrData(&buffer, data_version);
		AddPtrData(&buffer, tablename, sizeof(tablename));
		for(int16 i=0;i<GetTotalQueries();i++)
			AddPtrData(&buffer, tmp_queries[i], strlen(tmp_queries[i]) + 1);
		return buffer;
	}
	void DeSerialize(uchar* data){
		try_delete = false;
		uchar* ptr = data;
		memcpy(&num_queries, ptr, sizeof(num_queries));
		ptr+= sizeof(num_queries);
		memcpy(&latest_version, ptr, sizeof(latest_version));
		ptr+= sizeof(latest_version);
		memcpy(&your_version, ptr, sizeof(your_version));
		ptr+= sizeof(your_version);
		memcpy(&data_version, ptr, sizeof(data_version));
		ptr+= sizeof(data_version);
		memcpy(&tablename, ptr, sizeof(tablename));
		ptr+= sizeof(tablename);
		for(int16 i=0;i<GetTotalQueries();i++){
			tmp_queries.push_back((char*)ptr);
			ptr += strlen((char*)ptr) + 1;
		}
	}
	int16			current_index;
	int16			num_queries;
	int32			latest_version;
	int32			your_version;
	int32			data_version;
	bool			try_delete;
	char			tablename[64];
	int32			total_size;
	string			buffer;
private:
	vector<char*>	tmp_queries;
};
class TableDataQuery{
public:
	TableDataQuery(char* table_name){
		if( strlen(table_name) >= sizeof(tablename) )
			return;
		strcpy(tablename, table_name);
		num_queries = 0;
		columns_size = 0;
		columns = 0;
		version = 0;
		table_size = 0;
	}
	TableDataQuery(){
		num_queries = 0;
		columns_size = 0;
		columns = 0;
		version = 0;
		table_size = 0;
	}
	~TableDataQuery(){
		safe_delete_array(columns);
		for(int32 i=0;i<num_queries;i++){
			safe_delete_array(queries[i]->query);
			safe_delete(queries[i]);
		}
	}
	int32 GetTotalQueries(){
		return num_queries;
	}
	string* Serialize(){
		buffer = "";
		num_queries = queries.size();
		if(GetTotalQueries() == 0)
			return 0;
		table_size = strlen(tablename);
		AddPtrData(&buffer, table_size);
		AddPtrData(&buffer, tablename, table_size + 1);
		AddPtrData(&buffer, version);
		if(num_queries > 200){
			int32 max_queries = 200;
			AddPtrData(&buffer, max_queries);
		}
		else
			AddPtrData(&buffer, num_queries);
		AddPtrData(&buffer, columns_size);
		AddPtrData(&buffer, columns, columns_size);
		vector<TableData*>::iterator query_iterator;
		int16 count = 0;
		for(int i=GetTotalQueries() - 1;i >=0 && count < 200;i--){
			AddPtrData(&buffer, queries[i]->size);
			AddPtrData(&buffer, queries[i]->query, queries[i]->size);
			safe_delete_array(queries[i]->query);
			safe_delete(queries[i]);
			queries.pop_back();
			count++;
		}
		return &buffer;
	}
	void DeSerialize(uchar* data){
		uchar* ptr = data;

		memcpy(&table_size, ptr, sizeof(table_size));
		ptr+= sizeof(table_size);
		memcpy(&tablename, ptr, table_size + 1);
		ptr+= table_size + 1;

		memcpy(&version, ptr, sizeof(version));
		ptr+= sizeof(version);

		memcpy(&num_queries, ptr, sizeof(num_queries));
		ptr+= sizeof(num_queries);

		memcpy(&columns_size, ptr, sizeof(columns_size));
		ptr+= sizeof(columns_size);
		columns = new char[columns_size + 1];
		memcpy(columns, ptr, columns_size + 1);
		ptr+= columns_size;

		for(int32 i=0;i<GetTotalQueries();i++)
		{
			TableData* new_query = new TableData;
			try {
				memcpy(&new_query->size, ptr, sizeof(new_query->size));
				ptr+= sizeof(new_query->size);
				new_query->query = new char[new_query->size + 1];
				memcpy(new_query->query, ptr, new_query->size);
				ptr+= new_query->size;
				queries.push_back(new_query);
			}
			catch( bad_alloc &ba )
			{
				cout << ba.what() << endl;
				if( NULL != new_query )
					delete new_query;
			}
		}
	}
	string			buffer;
	int32			num_queries;
	int32			version;
	int16			table_size;
	char			tablename[64];
	int16			columns_size;
	char*			columns;
	vector<TableData*> queries;
};

// Max number of equipment updates to send at once
struct EquipmentUpdateRequest_Struct
{
	int16 max_per_batch;
};

// Login's structure of equipment data
struct LoginEquipmentUpdate
{
	int32	world_char_id;
	int16	equip_type;
	int8	red;
	int8	green;
	int8	blue;
	int8	highlight_red;
	int8	highlight_green;
	int8	highlight_blue;
	int32	slot;
};

// World's structure of equipment data
struct EquipmentUpdate_Struct
{
	int32	id;			// unique record identifier per world
	int32	world_char_id;
	int16	equip_type;
	int8	red;
	int8	green;
	int8	blue;
	int8	highlight_red;
	int8	highlight_green;
	int8	highlight_blue;
	int32	slot;
};

// How many equipmment updates are there to send?
struct EquipmentUpdateList_Struct
{
	sint16	total_updates;
};

struct ZoneUpdateRequest_Struct{
	int16 max_per_batch;
};

struct LoginZoneUpdate{
	string	name;
	string	description;
};

struct ZoneUpdate_Struct{
	int32 zone_id;
	int8  zone_name_length; 
	int8  zone_desc_length;
	char data[0];
};

struct ZoneUpdateList_Struct{
	uint16	total_updates;
	char data[0];
};

//EQ2 Specific Structures Login -> World (Image)
struct CharacterTimeStamp_Struct {
int32	char_id;
int32	account_id;
int32	unix_timestamp;
};

//EQ2 Specific Structures World -> Login (Image)

/**UPDATE_FIELD TYPES**
These will be stored beside the timestamp on the world server to determine what has changed on between the timestamp, when the update is sent, it will remove the flag.

8 bits in a byte:
Example: 01001100
0 Level Flag
1 Race Flag
0 Class Flag
0 Gender Flag
1 Zone Flag
1 Armor Flag
0 Name Flag
0 Delete Flag
**/
#define LEVEL_UPDATE_FLAG	1
#define	RACE_UPDATE_FLAG	2
#define CLASS_UPDATE_FLAG	4
#define GENDER_UPDATE_FLAG	8
#define ZONE_UPDATE_FLAG	16
#define	ARMOR_UPDATE_FLAG	32
#define	NAME_UPDATE_FLAG	64
#define DELETE_UPDATE_FLAG	128
//This structure used for basic changes such as level,class,gender, and deletes that are not able to be backed up
struct CharDataUpdate_Struct {
int32	account_id;
int32	char_id;
int8	update_field;
int32	update_data;
};
struct BugReport{
char category[64];
char subcategory[64];
char causes_crash[64];
char reproducible[64];
char summary[128];
char description[2000];
char version[32];
char player[64];
int32 account_id;
char spawn_name[64];
int32 spawn_id;
int32 zone_id;
};

struct RaceUpdate_Struct {
int32	account_id;
int32	char_id;
int16	model_type;
int8	race;
};

//If this structure comes in with more than 74 bytes, should probably discard (leaves 65 bytes for new_name)
#define	CHARNAMEUPDATESTRUCT_MAXSIZE	74	
struct CharNameUpdate_Struct {
int32	account_id;
int32	char_id;
int8	name_length; // If its longer than 64, something is wrong :-/
char	new_name[0];
};

//If this structure comes in with more than 78 bytes, should probably discard (leaves 65 bytes for new_zone)
#define CHARZONESTRUCT_MAXSIZE			78
struct CharZoneUpdate_Struct {
int32	account_id;
int32	char_id;
int32	zone_id;
int8	zone_length; // If its longer than 64, something is wrong :-/
char	new_zone[0];
};

struct WorldCharCreate_Struct {
int32	account_id;
int32	char_id;
int16	model_type;
int16	char_size;
uchar	character[0];
};

struct WorldCharNameFilter_Struct {
int32	account_id;
int16	name_length;
uchar	name[0];
};

struct WorldCharNameFilterResponse_Struct {
int32	account_id;
int32	char_id;
int8	response;
};

#define CHARPICSTRUCT_MINSIZE 10
// Should only be used for the headshot picture
struct CharPictureUpdate_Struct {
	int32 account_id;
	int32 char_id;
	int16 pic_size;
	char pic[0];
};

#pragma pack()

#endif
