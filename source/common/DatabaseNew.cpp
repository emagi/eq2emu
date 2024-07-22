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
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "Log.h"
#include "DatabaseNew.h"
#include <errmsg.h>

//increase this if large queries are being run frequently to make less calls to malloc()
#define QUERY_INITIAL_SIZE	512

#if defined WORLD
#define DB_INI	"world_db.ini"
#elif defined LOGIN
#define DB_INI	"login_db.ini"
#elif defined PARSER
#define DB_INI	"parser_db.ini"
#endif

DatabaseNew::DatabaseNew() {
	mysql_init(&mysql);
	int timeout = 10;
	mysql_options(&mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
	MMysql.SetName("DatabaseNew::mysql");
}

DatabaseNew::~DatabaseNew() {
	mysql_close(&mysql);
#if MYSQL_VERSION_ID >= 50003
	mysql_library_end();
#else
	mysql_server_end();
#endif
}

bool DatabaseNew::Connect() {
	char line[256], *key, *val;
	char host[256], user[64], password[64], database[64], port[64];
	bool found_section = false;
	FILE *f;

	if ((f = fopen(DB_INI, "r")) == NULL) {
		LogWrite(DATABASE__ERROR, 0, "Database", "Unable to read %s\n", DB_INI);
		return false;
	}

	memset(host, 0, sizeof(host));
	memset(user, 0, sizeof(user));
	memset(password, 0, sizeof(password));
	memset(database, 0, sizeof(database));
	memset(port, 0, sizeof(port));

	while (fgets(line, sizeof(line), f) != NULL) {
		if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
			continue;

		if (!found_section) {
			if (strncasecmp(line, "[Database]", 10) == 0)
				found_section = true;
		}
		else {
			if ((key = strtok(line, "=")) != NULL) {
				if ((val = strtok(NULL, "\r\n")) != NULL) {
					if (strncasecmp(line, "host", 4) == 0)
						strncpy(host, val, sizeof(host) - 1);
					else if (strncasecmp(line, "user", 4) == 0)
						strncpy(user, val, sizeof(user) - 1);
					else if (strncasecmp(line, "password", 8) == 0)
						strncpy(password, val, sizeof(password) - 1);
					else if (strncasecmp(line, "database", 8) == 0)
						strncpy(database, val, sizeof(database) - 1);
					else if (strncasecmp(line, "port", 4) == 0)
						strncpy(port, val, sizeof(port) - 1);
				}
			}
		}
	}

	fclose(f);

	if (host[0] == '\0') {
		LogWrite(DATABASE__ERROR, 0, "Database", "Unknown 'host' in '%s'\n", DB_INI);
		return false;
	}
	if (user[0] == '\0') {
		LogWrite(DATABASE__ERROR, 0, "Database", "Unknown 'user' in '%s'\n", DB_INI);
		return false;
	}
	if (password[0] == '\0') {
		LogWrite(DATABASE__ERROR, 0, "Database", "Unknown 'password' in '%s'\n", DB_INI);
		return false;
	}
	if (database[0] == '\0') {
		LogWrite(DATABASE__ERROR, 0, "Database", "Unknown 'database' in '%s'\n", DB_INI);
		return false;
	}

	unsigned int portnum = atoul(port);
	return Connect(host, user, password, database, portnum);
}

bool DatabaseNew::Connect(const char *host, const char *user, const char *password, const char *database, unsigned int port) {
	if (mysql_real_connect(&mysql, host, user, password, database, port, NULL, 0) == NULL) {
		LogWrite(DATABASE__ERROR, 0, "Database", "Unable to connect to MySQL server at %s:%u: %s\n", host, port, mysql_error(&mysql));
		return false;
	}

	return true;
}

bool DatabaseNew::Query(const char *query, ...) {
	char *buf;
	size_t size = QUERY_INITIAL_SIZE;
	int num_chars;
	va_list args;
	bool ret = true;

	MMysql.writelock(__FUNCTION__, __LINE__);
	while (true) {
		if ((buf = (char *)malloc(size)) == NULL) {
			LogWrite(DATABASE__ERROR, 0, "Database", "Out of memory trying to allocate database query of %u bytes\n", size);
			MMysql.releasewritelock(__FUNCTION__, __LINE__);
			return false;
		}

		va_start(args, query);
		num_chars = vsnprintf(buf, size, query, args);
		va_end(args);

		if (num_chars > -1 && (size_t)num_chars < size)
			break;

		if (num_chars > -1)
			size = num_chars + 1;
		else
			size *= 2;

		free(buf);
	}

	if (mysql_real_query(&mysql, buf, num_chars) != 0) {

		if (mysql_errno(&mysql) == CR_SERVER_LOST || mysql_errno(&mysql) == CR_SERVER_GONE_ERROR) {
			LogWrite(DATABASE__ERROR, 0, "Database", "Lost connection, attempting to recover and retry query...");
			Connect();

			// retry attempt of previous query (1 try and we give up)
			if (mysql_real_query(&mysql, buf, num_chars) != 0) {
				ret = false;
			}
		}
		else if (!IsIgnoredErrno(mysql_errno(&mysql))) {
			LogWrite(DATABASE__ERROR, 0, "Database", "Error %i running MySQL query: %s\n%s\n", mysql_errno(&mysql), mysql_error(&mysql), buf);
			ret = false;
		}
	}
	free(buf);

	MMysql.releasewritelock(__FUNCTION__, __LINE__);
	return ret;
}

bool DatabaseNew::Select(DatabaseResult *result, const char *query, ...) {
	char *buf;
	size_t size = QUERY_INITIAL_SIZE;
	int num_chars;
	va_list args;
	MYSQL_RES *res;
	bool ret = true;

	MMysql.writelock(__FUNCTION__, __LINE__);
	while (true) {
		if ((buf = (char *)malloc(size)) == NULL) {
			LogWrite(DATABASE__ERROR, 0, "Database", "Out of memory trying to allocate database query of %u bytes\n", size);
			MMysql.releasewritelock(__FUNCTION__, __LINE__);
			return false;
		}

		va_start(args, query);
		num_chars = vsnprintf(buf, size, query, args);
		va_end(args);

		if (num_chars > -1 && (size_t)num_chars < size)
			break;

		if (num_chars > -1)
			size = num_chars + 1;
		else
			size *= 2;

		free(buf);
	}

	if (mysql_real_query(&mysql, buf, (unsigned long)num_chars) != 0) {

		if (mysql_errno(&mysql) == CR_SERVER_LOST || mysql_errno(&mysql) == CR_SERVER_GONE_ERROR) {
			LogWrite(DATABASE__ERROR, 0, "Database", "Lost connection, attempting to recover and retry query...");

			mysql_close(&mysql);
			Connect();

			// retry attempt of previous query (1 try and we give up)
			if (mysql_real_query(&mysql, buf, (unsigned long)num_chars) != 0) {
				ret = false;
			}
		}
		else if (!IsIgnoredErrno(mysql_errno(&mysql))) {
			LogWrite(DATABASE__ERROR, 0, "Database", "Error %i running MySQL query: %s\n%s\n", mysql_errno(&mysql), mysql_error(&mysql), buf);
			ret = false;
		}
	}

	if (ret && !IsIgnoredErrno(mysql_errno(&mysql))) {
		res = mysql_store_result(&mysql);

		if (res != NULL)
		{
			// Grab number of rows and number of fields from the query
			uint8 num_rows = mysql_affected_rows(&mysql);
			uint8 num_fields = mysql_field_count(&mysql);

			ret = result->StoreResult(res, num_fields, num_rows);
		}
		else {
			LogWrite(DATABASE__ERROR, 0, "Database", "Error storing MySql query result (%d): %s\n%s", mysql_errno(&mysql), mysql_error(&mysql), buf);
			ret = false;
		}
	}

	free(buf);

	MMysql.releasewritelock(__FUNCTION__, __LINE__);
	return ret;
}

int32 DatabaseNew::LastInsertID()
{
	return (int32)mysql_insert_id(&mysql);
}

long DatabaseNew::AffectedRows()
{
	return mysql_affected_rows(&mysql);
}

char * DatabaseNew::Escape(const char *str, size_t len) {
	char *buf = (char *)malloc(len * 2 + 1);

	if (buf == NULL) {
		LogWrite(DATABASE__ERROR, 0, "Database", "Out of memory trying to allocate %u bytes in %s:%u\n", len * 2 + 1, __FUNCTION__, __LINE__);
		return NULL;
	}

	mysql_real_escape_string(&mysql, buf, str, len);
	return buf;
}

char * DatabaseNew::Escape(const char *str) {
	return Escape(str, strlen(str));
}

string DatabaseNew::EscapeStr(const char *str, size_t len) {
	char *buf = (char *)malloc(len * 2 + 1);
	string ret;

	if (buf == NULL) {
		LogWrite(DATABASE__ERROR, 0, "Database", "Out of memory trying to allocate %u bytes in %s:%u\n", len * 2 + 1, __FUNCTION__, __LINE__);
		return NULL;
	}

	mysql_real_escape_string(&mysql, buf, str, len);
	ret.append(buf);
	free(buf);

	return ret;
}

string DatabaseNew::EscapeStr(const char *str) {
	return EscapeStr(str, strlen(str));
}

string DatabaseNew::EscapeStr(string str) {
	return EscapeStr(str.c_str(), str.length());
}

bool DatabaseNew::QueriesFromFile(const char * file) {
	bool success = true;
	long size;
	char *buf;
	int ret;
	MYSQL_RES *res;
	FILE *f;

	f = fopen(file, "rb");
	if (f == NULL) {
		LogWrite(DATABASE__ERROR, 0, "Database", "Unable to open '%s' for reading: %s", file, strerror(errno));
		return false;
	}

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);

	buf = (char *)malloc(size + 1);
	if (buf == NULL) {
		fclose(f);
		LogWrite(DATABASE__ERROR, 0, "Database", "Out of memory trying to allocate %u bytes in %s:%u\n", size + 1, __FUNCTION__, __LINE__);
		return false;
	}

	if (fread(buf, sizeof(*buf), size, f) != (size_t)size) {
		LogWrite(DATABASE__ERROR, 0, "Database", "Failed to read from '%s': %s", file, strerror(errno));
		fclose(f);
		free(buf);
		return false;
	}

	buf[size] = '\0';
	fclose(f);

	mysql_set_server_option(&mysql, MYSQL_OPTION_MULTI_STATEMENTS_ON);
	ret = mysql_real_query(&mysql, buf, size);
	free(buf);

	if (ret != 0) {
		LogWrite(DATABASE__ERROR, 0, "Database", "Error running MySQL queries from file '%s' (%d): %s", file, mysql_errno(&mysql), mysql_error(&mysql));
		success = false;
	}
	else {
		//all results must be processed
		do {
			res = mysql_store_result(&mysql);
			if (res != NULL)
				mysql_free_result(res);
			ret = mysql_next_result(&mysql);

			if (ret > 0) {
				LogWrite(DATABASE__ERROR, 0, "Database", "Error running MySQL queries from file '%s' (%d): %s", file, mysql_errno(&mysql), mysql_error(&mysql));
				success = false;
			}

		} while (ret == 0);
	}
	mysql_set_server_option(&mysql, MYSQL_OPTION_MULTI_STATEMENTS_OFF);

	return success;
}

void DatabaseNew::SetIgnoredErrno(unsigned int db_errno) {
	vector<unsigned int>::iterator itr;

	for (itr = ignored_errnos.begin(); itr != ignored_errnos.end(); itr++) {
		if ((*itr) == db_errno)
			return;
	}

	ignored_errnos.push_back(db_errno);
}

void DatabaseNew::RemoveIgnoredErrno(unsigned int db_errno) {
	vector<unsigned int>::iterator itr;

	for (itr = ignored_errnos.begin(); itr != ignored_errnos.end(); itr++) {
		if ((*itr) == db_errno) {
			ignored_errnos.erase(itr);
			break;
		}
	}
}

bool DatabaseNew::IsIgnoredErrno(unsigned int db_errno) {
	vector<unsigned int>::iterator itr;

	for (itr = ignored_errnos.begin(); itr != ignored_errnos.end(); itr++) {
		if ((*itr) == db_errno)
			return true;
	}

	return false;
}

// Sends the MySQL server a keepalive
void DatabaseNew::PingNewDB() {
	MMysql.writelock(__FUNCTION__, __LINE__);
	mysql_ping(&mysql);

	int32* errnum = new int32;
	*errnum = mysql_errno(&mysql);

	switch (*errnum)
	{
		case CR_COMMANDS_OUT_OF_SYNC:
		case CR_SERVER_GONE_ERROR:
		case CR_UNKNOWN_ERROR:
		{
			LogWrite(DATABASE__ERROR, 0, "Database", "[Database] We lost connection to the database., errno: %i", errno);
			break;
		}
	}

	safe_delete(errnum);
	MMysql.releasewritelock(__FUNCTION__, __LINE__);
}