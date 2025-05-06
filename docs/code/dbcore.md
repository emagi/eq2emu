# File: `dbcore.h`

## Classes

- `DBcore`

## Functions

- `eStatus		GetStatus() { return pStatus; }`
- `bool		RunQuery(const char* query, int32 querylen, char* errbuf = 0, MYSQL_RES** result = 0, int32* affected_rows = 0, int32* last_insert_id = 0, int32* errnum = 0, bool retry = true);`
- `int32		DoEscapeString(char* tobuf, const char* frombuf, int32 fromlen);`
- `void		ping();`
- `string		getSafeEscapeString(const char* from_string);`
- `string		getSafeEscapeString(string* from_string);`
- `bool	Open(const char* iHost, const char* iUser, const char* iPassword, const char* iDatabase, int32 iPort, int32* errnum = 0, char* errbuf = 0, bool iCompress = false, bool iSSL = false);`
- `bool	ReadDBINI(char *host, char *user, char *pass, char *db, unsigned int* port, bool* compress, bool *items);`
- `bool	Open(int32* errnum = 0, char* errbuf = 0);`

## Notable Comments

- /*
- */
- //#include <WinSock2.h>
