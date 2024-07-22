#ifndef COMMON_DATABASE_H_
#define COMMON_DATABASE_H_

#include <string>
#include "DatabaseResult.h"

using namespace std;

class DatabaseNew {
public:
	DatabaseNew();
	virtual ~DatabaseNew();

	unsigned int GetError() {return mysql_errno(&mysql);}
	const char * GetErrorMsg() {return mysql_error(&mysql);}

	bool Connect();
	bool Connect(const char *host, const char *user, const char *password, const char *database, unsigned int port = 3306);

	bool Query(const char *query, ...);
	bool Select(DatabaseResult *result, const char *query, ...);

	int32 LastInsertID();
	long AffectedRows();

	//these two must free() the return char* after it's used in a query
	char * Escape(const char *str, size_t len);
	char * Escape(const char *str);

	//does not need free()
	string EscapeStr(const char *str, size_t len);
	string EscapeStr(const char *str);
	string EscapeStr(string str);

	bool QueriesFromFile(const char *file);
	void SetIgnoredErrno(unsigned int db_errno);
	void RemoveIgnoredErrno(unsigned int db_errno);
	bool IsIgnoredErrno(unsigned int db_errno);

	void PingNewDB();
private:
	MYSQL mysql;
	Mutex MMysql;

	vector<unsigned int> ignored_errnos;
};

#endif
