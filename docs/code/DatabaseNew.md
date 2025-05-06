# File: `DatabaseNew.h`

## Classes

- `DatabaseNew`

## Functions

- `bool Connect();`
- `bool Connect(const char *host, const char *user, const char *password, const char *database, unsigned int port = 3306);`
- `bool Query(const char *query, ...);`
- `bool Select(DatabaseResult *result, const char *query, ...);`
- `int32 LastInsertID();`
- `long AffectedRows();`
- `string EscapeStr(const char *str, size_t len);`
- `string EscapeStr(const char *str);`
- `string EscapeStr(string str);`
- `bool QueriesFromFile(const char *file);`
- `void SetIgnoredErrno(unsigned int db_errno);`
- `void RemoveIgnoredErrno(unsigned int db_errno);`
- `bool IsIgnoredErrno(unsigned int db_errno);`
- `void PingNewDB();`

## Notable Comments

- //these two must free() the return char* after it's used in a query
- //does not need free()
