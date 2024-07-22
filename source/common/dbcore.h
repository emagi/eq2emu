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
#ifndef DBCORE_H
#define DBCORE_H

#ifdef WIN32
	#include <WinSock2.h>
	#include <windows.h>
	//#include <WinSock2.h>
#endif
#include <mysql.h>
#include "../common/types.h"
#include "../common/Mutex.h"
#include "../common/linked_list.h"
#include "../common/queue.h"
#include "../common/timer.h"
#include "../common/Condition.h"
#ifdef LOGIN
	#define DB_INI_FILE "login_db.ini"
#endif
#ifdef WORLD
	#define DB_INI_FILE "world_db.ini"
#endif
#ifdef PARSER
	#define DB_INI_FILE "parser_db.ini"
#endif
#ifdef PATCHER
	#define DB_INI_FILE "patcher_db.ini"
#endif
class DBcore{
public:
	enum eStatus { Closed, Connected, Error };
	DBcore();
	~DBcore();
	eStatus		GetStatus() { return pStatus; }
	bool		RunQuery(const char* query, int32 querylen, char* errbuf = 0, MYSQL_RES** result = 0, int32* affected_rows = 0, int32* last_insert_id = 0, int32* errnum = 0, bool retry = true);
	int32		DoEscapeString(char* tobuf, const char* frombuf, int32 fromlen);
	void		ping();
	char*		getEscapeString(const char* from_string);
	string		getSafeEscapeString(const char* from_string);
	string		getSafeEscapeString(string* from_string);
	
protected:
	bool	Open(const char* iHost, const char* iUser, const char* iPassword, const char* iDatabase, int32 iPort, int32* errnum = 0, char* errbuf = 0, bool iCompress = false, bool iSSL = false);
	bool	ReadDBINI(char *host, char *user, char *pass, char *db, unsigned int* port, bool* compress, bool *items);
private:
	bool	Open(int32* errnum = 0, char* errbuf = 0);

	MYSQL	mysql;
	Mutex	MDatabase;
	eStatus pStatus;
	
	char*	pHost;
	char*	pUser;
	char*	pPassword;
	char*	pDatabase;
	bool	pCompress;
	int32	pPort;
	bool	pSSL;
};
#endif


