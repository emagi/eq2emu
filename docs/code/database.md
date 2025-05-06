# File: `database.h`

## Classes

- `Query`
- `Database`
- `Query`

## Functions

- `bool Init(bool silentLoad=false);`
- `bool LoadVariables();`
- `void HandleMysqlError(int32 errnum);`
- `int32 AuthenticateWebUser(char* userName, char* passwd,int32* status = 0);`
- `int32 NoAuthRoute(char* route);`
- `void AddAsyncQuery(Query* query);`
- `void RunAsyncQueries(int32 queryid);`
- `void RemoveActiveQuery(Query* query);`
- `void AddActiveQuery(Query* query);`
- `bool IsActiveQuery(int32 id, Query* skip=0);`
- `void PingAsyncDatabase();`
- `void InitVars();`
- `void PurgeDBInstances();`
- `void FreeDBInstance(Database* cur);`
- `int32		GetLastInsertedID() { return *last_insert_id; }`
- `int32		GetAffectedRows() { return *affected_rows; }`
- `int32		GetErrorNumber(){ return errnum; }`
- `void		NextRow(){`
- `void AddQueryAsync(int32 queryID, Database* db, QUERY_TYPE type, const char* format, ...);`
- `void RunQueryAsync(Database* db);`
- `QUERY_TYPE GetQueryType() {`
- `int32 GetQueryID() { return queryID; }`

## Notable Comments

- /*
- */
- *row = mysql_fetch_row(result);
- *row = mysql_fetch_row(result);
