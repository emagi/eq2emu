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

#include <iostream>
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errmsg.h>
//#include <mysqld_error.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include <map>

// Disgrace: for windows compile
#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else
#include "unix.h"
#include <netinet/in.h>
#endif

#include "database.h"
#include "EQStream.h"
#include "packet_functions.h"
#include "emu_opcodes.h"
#ifdef WORLD
	#include "../WorldServer/WorldDatabase.h"
	#include "../WorldServer/Web/PeerManager.h"
	extern WorldDatabase database;
	extern PeerManager peer_manager;
#endif
#ifdef LOGIN
	#include "../LoginServer/LoginDatabase.h"
	extern LoginDatabase database;
#endif
#ifdef PARSER
	#include "../PacketParser/ParserDatabase.h"
	extern ParserDatabase database;
#endif

#ifdef PATCHER
	#include "../PatchServer/PatcherDatabase.h"
	extern PatcherDatabase database;
#endif
#include "../common/EQEMuError.h"
#include "../common/packet_dump.h"
#include "../common/Log.h"

#ifdef WORLD
ThreadReturnType DBAsyncQueries(void* str)
{
	// allow some buffer for multiple queries to collect
	Sleep(10);
	DBStruct* data = (DBStruct*)str;
	database.RunAsyncQueries(data->queryid);
	delete data;
	THREAD_RETURN(NULL);
}
#endif

Database::Database()
{
	InitVars();
}

bool Database::Init(bool silentLoad) {
	char host[200], user[200], passwd[200], database[200];
	unsigned int port=0;
	bool compression = false;
	bool items[6] = {false, false, false, false, false, false};
	const char* exampleIni[] = { "[Database]", "host = localhost", "user = root", "password = pass", "database = dbname", "### --- Assure each parameter is on a new line!" };

	if(!ReadDBINI(host, user, passwd, database, &port, &compression, items)) {
		//exit(1);
		return false;
	}
	
	if (!items[0] || !items[1] || !items[2] || !items[3])
	{
		LogWrite(DATABASE__ERROR, 0, "DB", "Database file %s is incomplete.", DB_INI_FILE);
		int i;
		for (i = 0; i < 4; i++)
		{
			if ( !items[i] )
				LogWrite(DATABASE__ERROR, 0, "DB", "Could not find parameter %s", exampleIni[i+1]); // offset by 1 because the [Database] entry
		}
		LogWrite(DATABASE__ERROR, 0, "DB", "Example File:");
		int length = sizeof exampleIni / sizeof exampleIni[0];
		for(i=0;i<length;i++)
		LogWrite(DATABASE__ERROR, 0, "DB", "%s", exampleIni[i]);
		//exit (1);
		return false;
	}
	
	int32 errnum = 0;
	char errbuf[MYSQL_ERRMSG_SIZE];
	if (!Open(host, user, passwd, database,port, &errnum, errbuf)) 
	{
		LogWrite(DATABASE__ERROR, 0, "DB", "Failed to connect to database: Error: %s", errbuf);
		HandleMysqlError(errnum);
		//exit(1);
		return false;
	}
	else
	{
		if (!silentLoad)
			LogWrite(DATABASE__INFO, 0, "DB", "Using database '%s' at %s", database, host);
	}

	return true;
}

map<int16, int16> Database::GetVersions(){
	map<int16, int16> opcodes;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "select distinct version_range1, version_range2 from opcodes");
	while(result && (row = mysql_fetch_row(result))){
		if(row[0] && row[1])
			opcodes[atoi(row[0])] = atoi(row[1]);
	}
	return opcodes;
}

map<string, uint16> Database::GetOpcodes(int16 version){
	map<string, uint16> opcodes;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "select name, opcode from opcodes where %i between version_range1 and version_range2 order by version_range1, id", version);
	while(result && (row = mysql_fetch_row(result))){
		opcodes[row[0]] = atoi(row[1]);
	}
	return opcodes;
}

int32 Database::AuthenticateWebUser(char* userName, char* passwd, int32* status){
	if(status) {
		*status = 0;
	}
	Query query;
	MYSQL_ROW row;
	int32 id = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "select id, status from web_users where username='%s' and passwd = sha2('%s', 512)", getSafeEscapeString(userName).c_str(), getSafeEscapeString(passwd).c_str());
	if(result && (row = mysql_fetch_row(result))){
		id = atoul(row[0]);
		if(status) {
			*status = atoul(row[1]);
		}
	}
	return id;
}

int32 Database::NoAuthRoute(char* route){
	Query query;
	MYSQL_ROW row;
	int32 status = 0xFFFFFFFF;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "select status from web_routes where route='%s'", getSafeEscapeString(route).c_str());
	if(result && (row = mysql_fetch_row(result))){
		status = atoul(row[0]);
	}
	return status;
}

void Database::HandleMysqlError(int32 errnum) {
	switch(errnum) {
		case 0:
			break;
		case 1045: // Access Denied
		case 2001: {
			AddEQEMuError(EQEMuError_Mysql_1405, true);
			break;
		}
		case 2003: { // Unable to connect
			AddEQEMuError(EQEMuError_Mysql_2003, true);
			break;
		}
		case 2005: { // Unable to connect
			AddEQEMuError(EQEMuError_Mysql_2005, true);
			break;
		}
		case 2007: { // Unable to connect
			AddEQEMuError(EQEMuError_Mysql_2007, true);
			break;
		}
	}
}

void Database::InitVars() {

}

Database::~Database()
{
#ifdef WORLD
	DBQueryMutex.writelock(__FUNCTION__, __LINE__);
	activeQuerySessions.clear();
	DBQueryMutex.releasewritelock(__FUNCTION__, __LINE__);

	DBAsyncMutex.writelock();
	continueAsync = false;
	map<int32, deque<Query*>>::iterator itr;
	for (itr = asyncQueries.begin(); itr != asyncQueries.end(); itr++)
	{
		asyncQueriesMutex[itr->first]->writelock();
		deque<Query*> queries = itr->second;
		while (queries.size() > 0)
		{
			Query* cur = queries.front();
			queries.pop_front();
			safe_delete(cur);
		}
		asyncQueriesMutex[itr->first]->releasewritelock();
		Mutex* mutex = asyncQueriesMutex[itr->first];
		asyncQueriesMutex.erase(itr->first);
		safe_delete(mutex);
	}
	asyncQueries.clear();

	asyncQueriesMutex.clear();
	DBAsyncMutex.releasewritelock();

	PurgeDBInstances();
#endif
}

#ifdef WORLD
void Query::AddQueryAsync(int32 queryID, Database* db, QUERY_TYPE type, const char* format, ...) {
	in_type = type;
	va_list args;
	va_start(args, format);
#ifdef WIN32
	char* buffer;
	int buf_len = _vscprintf(format, args) + 1;
	buffer = new char[buf_len];
	vsprintf(buffer, format, args);
#else
	char* buffer;
	int buf_len;
	va_list argcopy;
	va_copy(argcopy, args);
	buf_len = vsnprintf(NULL, 0, format, argcopy) + 1;
	va_end(argcopy);

	buffer = new char[buf_len];
	vsnprintf(buffer, buf_len, format, args);
#endif
	va_end(args);
	query = string(buffer);

	Query* asyncQuery = new Query(this, queryID);

	safe_delete_array(buffer);

	db->AddAsyncQuery(asyncQuery);
}

void Query::RunQueryAsync(Database* db) {
	db->RunQuery(query.c_str(), query.length(), errbuf, &result, affected_rows, last_insert_id, &errnum, retry);
}
#endif

MYSQL_RES* Query::RunQuery2(QUERY_TYPE type, const char* format, ...){
	va_list args;
	va_start( args, format );
	#ifdef WIN32
		char * buffer;
		int buf_len = _vscprintf( format, args ) + 1;
		buffer = new char[buf_len];
		vsprintf( buffer, format, args );
	#else
		char* buffer;
		int buf_len; 
		va_list argcopy;
		va_copy(argcopy, args);
		buf_len = vsnprintf(NULL, 0, format, argcopy) + 1;
		va_end(argcopy);
		
		buffer = new char[buf_len];
		vsnprintf(buffer, buf_len, format, args);
	#endif
	va_end(args);
	query = string(buffer);

	
	safe_delete_array( buffer );
	

	return RunQuery2(query.c_str(), type);
}
MYSQL_RES* Query::RunQuery2(string in_query, QUERY_TYPE type){
	switch(type){
		case Q_SELECT:
			break;
		case Q_DBMS:
		case Q_REPLACE:
		case Q_DELETE:
		case Q_UPDATE:
			safe_delete(affected_rows);
			affected_rows = new int32;
			break;
		case Q_INSERT:
			safe_delete(last_insert_id);
			last_insert_id = new int32;
	}
	if(result){
		if(!multiple_results)
			multiple_results = new vector<MYSQL_RES*>();
		multiple_results->push_back(result);
	}	
	query = in_query;

#if defined WORLD && defined _DEBUG
	if (type == Q_UPDATE || type == Q_INSERT || type == Q_DELETE || type == Q_REPLACE)
	{
		char* filteredTables[] = { " characters", " character_", " `character_", " statistics", " variables", " char_colors", " `guild", " bugs" };

		bool match = false;
		for (int i = 0; i < sizeof(filteredTables) / sizeof(filteredTables[0]); i++)
		{
			if (query.find(filteredTables[i]) != std::string::npos) {
				match = true;
				break;
			}
		}
		try
		{
			if (!match)
			{
				FILE* pFile;
				pFile = fopen("sql_updates.sql", "a+");
				fwrite(query.c_str(), 1, query.length(), pFile);
				fwrite(";", sizeof(char), 1, pFile);
				fwrite("\n", sizeof(char), 1, pFile);
				fclose(pFile);
			}
		}
		catch (...) {}
	}
#endif

	
	database.RunQuery(query.c_str(), query.length(), errbuf, &result, affected_rows, last_insert_id, &errnum, retry); 
	return result;
}

#ifdef WORLD
void Database::RunAsyncQueries(int32 queryid)
{
	Database* asyncdb = FindFreeInstance();
	DBAsyncMutex.writelock();
	map<int32, deque<Query*>>::iterator itr = asyncQueries.find(queryid);
	if (itr == asyncQueries.end())
	{
		DBAsyncMutex.releasewritelock();
		return;
	}

	asyncQueriesMutex[queryid]->writelock();
	deque<Query*> queries;
	while (itr->second.size())
	{
		Query* cur = itr->second.front();
		queries.push_back(cur);
		itr->second.pop_front();
	}
	itr->second.clear();
	asyncQueries.erase(itr);
	DBAsyncMutex.releasewritelock();
	asyncQueriesMutex[queryid]->releasewritelock();

	int32 count = 0;
	while (queries.size() > 0)
	{
		Query* cur = queries.front();
		cur->RunQueryAsync(asyncdb);
		this->RemoveActiveQuery(cur);
		queries.pop_front();
		safe_delete(cur);
	}
	FreeDBInstance(asyncdb);

	bool isActive = LocalIsActiveQuery(queryid);
	if (isActive)
	{
		continueAsync = true;
		DBStruct* tmp = new DBStruct;
		tmp->queryid = queryid;
#ifdef WIN32
		_beginthread(DBAsyncQueries, 0, (void*)tmp);
#else
		pthread_t t1;
		pthread_create(&t1, NULL, DBAsyncQueries, (void*)tmp);
		pthread_detach(t1);
#endif
	}
}

void Database::AddAsyncQuery(Query* query)
{
	DBAsyncMutex.writelock();
	map<int32, Mutex*>::iterator mutexItr = asyncQueriesMutex.find(query->GetQueryID());
	if (mutexItr == asyncQueriesMutex.end())
	{
		Mutex* queryMutex = new Mutex();
		queryMutex->SetName("AsyncQuery" + query->GetQueryID());
		asyncQueriesMutex.insert(make_pair(query->GetQueryID(), queryMutex));
	}
	map<int32, deque<Query*>>::iterator itr = asyncQueries.find(query->GetQueryID());
	asyncQueriesMutex[query->GetQueryID()]->writelock();

	if ( itr != asyncQueries.end())
		itr->second.push_back(query);
	else
	{
		deque<Query*> queue;
		queue.push_back(query);
		asyncQueries.insert(make_pair(query->GetQueryID(), queue));
	}

	AddActiveQuery(query);

	asyncQueriesMutex[query->GetQueryID()]->releasewritelock();
	DBAsyncMutex.releasewritelock();

	bool isActive = LocalIsActiveQuery(query->GetQueryID(), query);
	if (!isActive)
	{
	continueAsync = true;
	DBStruct* tmp = new DBStruct;
	tmp->queryid = query->GetQueryID();
#ifdef WIN32
	_beginthread(DBAsyncQueries, 0, (void*)tmp);
#else
	pthread_t t1;
	pthread_create(&t1, NULL, DBAsyncQueries, (void*)tmp);
	pthread_detach(t1);
#endif
	}
}

Database* Database::FindFreeInstance()
{
	Database* db_inst = 0;
	map<Database*, bool>::iterator itr;
	DBInstanceMutex.writelock(__FUNCTION__, __LINE__);
	for (itr = dbInstances.begin(); itr != dbInstances.end(); itr++) {
		if (!itr->second)
		{
			db_inst = itr->first;
			itr->second = true;
			break;
		}
	}

	if (!db_inst)
	{
		WorldDatabase* tmp = new WorldDatabase();
		db_inst = (Database*)tmp;
		tmp->Init();
		tmp->ConnectNewDatabase();
		dbInstances.insert(make_pair(db_inst, true));
	}
	DBInstanceMutex.releasewritelock(__FUNCTION__, __LINE__);

	return db_inst;
}

void Database::PurgeDBInstances()
{
	map<Database*, bool>::iterator itr;
	DBInstanceMutex.writelock(__FUNCTION__, __LINE__);
	for (itr = dbInstances.begin(); itr != dbInstances.end(); itr++) {
		WorldDatabase* tmpInst = (WorldDatabase*)itr->first;
		safe_delete(tmpInst);
	}
	dbInstances.clear();
	DBInstanceMutex.releasewritelock(__FUNCTION__, __LINE__);
}


void Database::PingAsyncDatabase()
{
	map<Database*, bool>::iterator itr;
	DBInstanceMutex.readlock(__FUNCTION__, __LINE__);
	for (itr = dbInstances.begin(); itr != dbInstances.end(); itr++) {
		Database* tmpInst = itr->first;
		tmpInst->ping();
	}
	DBInstanceMutex.releasereadlock(__FUNCTION__, __LINE__);
}

void Database::FreeDBInstance(Database* cur)
{
	DBInstanceMutex.writelock(__FUNCTION__, __LINE__);
	dbInstances[cur] = false;
	DBInstanceMutex.releasewritelock(__FUNCTION__, __LINE__);
}

void Database::RemoveActiveQuery(Query* query)
{
	DBQueryMutex.writelock(__FUNCTION__, __LINE__);

	vector<Query*>::iterator itr;
	for (itr = activeQuerySessions.begin(); itr != activeQuerySessions.end(); itr++)
	{
		Query* curQuery = *itr;
		if (query == curQuery)
		{
			activeQuerySessions.erase(itr);
			break;
		}
	}
	DBQueryMutex.releasewritelock(__FUNCTION__, __LINE__);
	
	bool isActive = LocalIsActiveQuery(query->GetQueryID());
	if(!isActive) {
		peer_manager.sendPeersActiveQuery(query->GetQueryID(), true);
	}
}

void Database::AddActiveQuery(Query* query)
{
	peer_manager.sendPeersActiveQuery(query->GetQueryID(), false);
	DBQueryMutex.writelock(__FUNCTION__, __LINE__);
	activeQuerySessions.push_back(query);
	DBQueryMutex.releasewritelock(__FUNCTION__, __LINE__);
}

bool Database::IsActiveQuery(int32 id, Query* skip) {
	if (LocalIsActiveQuery(id, skip))
		return true;

	{
		auto now = std::chrono::steady_clock::now();
		std::lock_guard<std::mutex> lock(_peerMtx);

		// remove any entries older than timeout
		for (auto it = _peerActive.begin(); it != _peerActive.end(); ) {
			if (now - it->second > kStaleTimeout)
				it = _peerActive.erase(it);
			else
				++it;
		}

		// if this id is still in the map, it's active
		if (_peerActive.find(id) != _peerActive.end())
			return true;
	}

	return false;
}

bool Database::LocalIsActiveQuery(int32 id, Query* skip)
{
	bool isActive = false;
	DBQueryMutex.readlock(__FUNCTION__, __LINE__);
	for (auto query : activeQuerySessions) {
		if (query == skip) continue;
		if (query->GetQueryID() == id) {
			isActive = true;
			break;
		}
	}
	DBQueryMutex.releasereadlock(__FUNCTION__, __LINE__);
	return isActive;
}

void Database::AddPeerActiveQuery(int32 charID) {
	auto now = std::chrono::steady_clock::now();
	std::lock_guard<std::mutex> lock(_peerMtx);
	_peerActive[charID] = now;  // inserts or updates timestamp
}

void Database::RemovePeerActiveQuery(int32 charID) {
	std::lock_guard<std::mutex> lock(_peerMtx);
	_peerActive.erase(charID);
}
#endif