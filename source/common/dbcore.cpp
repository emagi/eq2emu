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
#include "debug.h"

#include <iostream>
using namespace std;
#include <errmsg.h>
//#include <mysqld_error.h>
#include <limits.h>
#include "dbcore.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "MiscFunctions.h"
#include "Log.h"

#ifdef WIN32
	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
	#include <process.h>
#else
	#include "unix.h"
	#include <pthread.h>
#endif

#ifdef _EQDEBUG
	#define DEBUG_MYSQL_QUERIES 0
#else
	#define DEBUG_MYSQL_QUERIES 0
#endif

DBcore::DBcore() {
	mysql_init(&mysql);
	pHost = 0;
	pPort = 0;
	pUser = 0;
	pPassword = 0;
	pDatabase = 0;
	pCompress = false;
pSSL = false;
pStatus = Closed;
}

DBcore::~DBcore() {
	pStatus = Closed;
	mysql_close(&mysql);
#if MYSQL_VERSION_ID >= 50003
	mysql_library_end();
#else
	mysql_server_end();
#endif
	safe_delete_array(pHost);
	safe_delete_array(pUser);
	safe_delete_array(pPassword);
	safe_delete_array(pDatabase);
}


bool DBcore::ReadDBINI(char* host, char* user, char* passwd, char* database, unsigned int* port, bool* compress, bool* items) {
	char line[256], * key, * val;
	bool on_database_section = false;
	FILE* f;

	if ((f = fopen(DB_INI_FILE, "r")) == NULL) {
		LogWrite(DATABASE__ERROR, 0, "DBCore", "Unable to open '%s' for reading", DB_INI_FILE);
		return false;
	}

	//read each line
	while (fgets(line, sizeof(line), f) != NULL) {

		//remove any new line or carriage return
		while ((key = strstr(line, "\n")) != NULL)
			*key = '\0';
		while ((key = strstr(line, "\r")) != NULL)
			*key = '\0';

		//ignore blank lines and commented lines
		if (strlen(line) == 0 || line[0] == '#')
			continue;

		key = strtok(line, "=");

		if (key == NULL)
			continue;

		//don't do anything until we find the [Database] section
		if (!on_database_section && strncasecmp(key, "[Database]", 10) == 0)
			on_database_section = true;
		else {
			val = strtok(NULL, "=");

			if (val == NULL)
			{
				if (strcasecmp(key, "password") == 0) {
					strcpy(passwd, "");
					items[2] = true;
				}
				continue;
			}
			if (strcasecmp(key, "host") == 0) {
				strcpy(host, val);
				items[0] = true;
			}
			else if (strcasecmp(key, "user") == 0) {
				strcpy(user, val);
				items[1] = true;
			}
			else if (strcasecmp(key, "password") == 0) {
				strcpy(passwd, val);
				items[2] = true;
			}
			else if (strcasecmp(key, "database") == 0) {
				strcpy(database, val);
				items[3] = true;
			}
			else if (strcasecmp(key, "port") == 0 && port) {
				*port = atoul(val);
				items[4] = true;
			}
			else if (strcasecmp(key, "compression") == 0) {
				if (strcasecmp(val, "on") == 0) {
					if(compress) { 
						*compress = true;
						items[5] = true;
						LogWrite(DATABASE__INFO, 0, "DBCore", "DB Compression on.");
					}
				}
			}
		}
	}

	fclose(f);

	if (!on_database_section) {
		LogWrite(DATABASE__ERROR, 0, "DBCore", "[Database] section not found in '%s'", DB_INI_FILE);
		return false;
	}

	return true;
}


// Sends the MySQL server a keepalive
void DBcore::ping() {
	if (!MDatabase.trylock()) {
		// well, if's it's locked, someone's using it. If someone's using it, it doesnt need a keepalive
		return;
	}
	mysql_ping(&mysql);

	int32* errnum = new int32;
	*errnum = mysql_errno(&mysql);
	
	switch (*errnum)
	{
		case CR_COMMANDS_OUT_OF_SYNC:
		case CR_SERVER_GONE_ERROR:
		case CR_UNKNOWN_ERROR:
		{
			LogWrite(DATABASE__ERROR, 0, "DBCore", "[Database] We lost connection to the database., errno: %i", errno);
			break;
		}
	}

	safe_delete(errnum);
	MDatabase.unlock();
}

bool DBcore::RunQuery(const char* query, int32 querylen, char* errbuf, MYSQL_RES** result, int32* affected_rows, int32* last_insert_id, int32* errnum, bool retry) {
	if (errnum)
		*errnum = 0;
	if (errbuf)
		errbuf[0] = 0;
	bool ret = false;
	LockMutex lock(&MDatabase);
	if (pStatus != Connected)
		Open();

	LogWrite(DATABASE__QUERY, 0, "DBCore", query);
	if (mysql_real_query(&mysql, query, querylen)) {
		if (mysql_errno(&mysql) == CR_SERVER_GONE_ERROR)
			pStatus = Error;
		if (mysql_errno(&mysql) == CR_SERVER_LOST || mysql_errno(&mysql) == CR_SERVER_GONE_ERROR) {
			if (retry) {
				LogWrite(DATABASE__ERROR, 0, "DBCore", "Lost connection, attempting to recover...");
				ret = RunQuery(query, querylen, errbuf, result, affected_rows, last_insert_id, errnum, false);
			}
			else {
				pStatus = Error;
				if (errnum)
					*errnum = mysql_errno(&mysql);
				if (errbuf)
					snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));
				LogWrite(DATABASE__ERROR, 0, "DBCore", "#%i: %s\nQuery:\n%s", mysql_errno(&mysql), mysql_error(&mysql), query);
				ret = false;
			}
		}
		else {
			if (errnum)
				*errnum = mysql_errno(&mysql);
			if (errbuf)
				snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));
			LogWrite(DATABASE__ERROR, 0, "DBCore", "#%i: %s\nQuery:\n%s", mysql_errno(&mysql), mysql_error(&mysql), query);
			ret = false;
		}
	}
	else {
		if (result && mysql_field_count(&mysql)) {
			*result = mysql_store_result(&mysql);
		}
		else if (result)
			*result = 0;
		if (affected_rows)
			*affected_rows = mysql_affected_rows(&mysql);
		if (last_insert_id)
			*last_insert_id = mysql_insert_id(&mysql);
		if (result) {
			if (*result) {
				ret = true;
			}
			else {
				if (errnum)
					*errnum = UINT_MAX;
				if (errbuf){
					if((!affected_rows || (affected_rows && *affected_rows == 0)) && (!last_insert_id  || (last_insert_id && *last_insert_id == 0)))
						LogWrite(DATABASE__RESULT, 1, "DBCore", "No Result.");
				}
				ret = false;
			}
		}
		else {
			ret = true;
		}
	}

	if (ret) 
	{
		char tmp1[200] = {0};
		char tmp2[200] = {0};
		if (result && (*result))
			snprintf(tmp1, sizeof(tmp1), ", %i rows returned", (int) mysql_num_rows(*result));
		if (affected_rows)
			snprintf(tmp2, sizeof(tmp2), ", %i rows affected", (*affected_rows));

		LogWrite(DATABASE__DEBUG, 0, "DBCore", "Query Successful%s%s", tmp1, tmp2);
	}
	else
		LogWrite(DATABASE__DEBUG, 0, "DBCore", "Query returned no results in %s!\n%s", __FUNCTION__, query);

	return ret;
}

int32 DBcore::DoEscapeString(char* tobuf, const char* frombuf, int32 fromlen) {
	LockMutex lock(&MDatabase);
	return mysql_real_escape_string(&mysql, tobuf, frombuf, fromlen);
}

bool DBcore::Open(const char* iHost, const char* iUser, const char* iPassword, const char* iDatabase,int32 iPort, int32* errnum, char* errbuf, bool iCompress, bool iSSL) {
	LockMutex lock(&MDatabase);
	safe_delete_array(pHost);
	safe_delete_array(pUser);
	safe_delete_array(pPassword);
	safe_delete_array(pDatabase);
	pHost = new char[strlen(iHost) + 1];
	strcpy(pHost, iHost);
	pUser = new char[strlen(iUser) + 1];
	strcpy(pUser, iUser);
	pPassword = new char[strlen(iPassword) + 1];
	strcpy(pPassword, iPassword);
	pDatabase = new char[strlen(iDatabase) + 1];
	strcpy(pDatabase, iDatabase);
	pCompress = iCompress;
	pPort = iPort;
	pSSL = iSSL;
	return Open(errnum, errbuf);
}

bool DBcore::Open(int32* errnum, char* errbuf) {
	if (errbuf)
		errbuf[0] = 0;
	LockMutex lock(&MDatabase);
	if (GetStatus() == Connected)
		return true;
	if (GetStatus() == Error)
		mysql_close(&mysql);
	if (!pHost)
		return false;
	/*
	Quagmire - added CLIENT_FOUND_ROWS flag to the connect
	otherwise DB update calls would say 0 rows affected when the value already equalled
	what the function was tring to set it to, therefore the function would think it failed 
	*/
	int32 flags = CLIENT_FOUND_ROWS;
	if (pCompress)
		flags |= CLIENT_COMPRESS;
	if (pSSL)
		flags |= CLIENT_SSL;
	if (mysql_real_connect(&mysql, pHost, pUser, pPassword, pDatabase, pPort, 0, flags)) {
		pStatus = Connected;
		return true;
	}
	else {
		if (errnum)
			*errnum = mysql_errno(&mysql);
		if (errbuf)
			snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(&mysql), mysql_error(&mysql));
		pStatus = Error;
		return false;
	}
}

char* DBcore::getEscapeString(const char* from_string){
	if(!from_string)
		from_string ="";
	int orig_size = strlen(from_string);
	int escape_size = (orig_size * 2) + 1;
	char* escaped = new char[escape_size];
	memset(escaped, 0, escape_size);
	DoEscapeString(escaped, from_string, orig_size);
	return escaped;
}

string DBcore::getSafeEscapeString(const char* from_string){
	if(!from_string)
		from_string ="";
	int orig_size = strlen(from_string);
	int escape_size = (orig_size * 2) + 1;
	char* escaped = new char[escape_size];
	memset(escaped, 0, escape_size);
	DoEscapeString(escaped, from_string, orig_size);
	string ret = string(escaped);
	safe_delete_array(escaped);
	return ret;
}

string DBcore::getSafeEscapeString(string* from_string){
	if(!from_string)
		return "";
	int orig_size = from_string->length();
	int escape_size = (orig_size * 2) + 1;
	char* escaped = new char[escape_size];
	memset(escaped, 0, escape_size);
	DoEscapeString(escaped, from_string->c_str(), orig_size);
	string ret = string(escaped);
	safe_delete_array(escaped);
	return ret;
}

