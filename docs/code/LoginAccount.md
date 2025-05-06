# File: `LoginAccount.h`

## Classes

- `LoginAccount`

## Functions

- `bool SaveAccount(LoginAccount* acct);`
- `void setName(const char* in_name) { strcpy(name, in_name); }`
- `void setPassword(const char* in_pass) { strcpy(password, in_pass); }`
- `void setAuthenticated(bool in_auth) { authenticated=in_auth; }`
- `void setAccountID(int32 id){ account_id = id; }`
- `void addCharacter(CharSelectProfile* profile){`
- `void removeCharacter(PacketStruct* profile);`
- `void removeCharacter(char* name, int16 version);`
- `void serializeCharacter(uchar* buffer, CharSelectProfile* profile);`
- `void flushCharacters ( );`
- `int32 getLoginAccountID(){ return account_id; }`
- `bool  getLoginAuthenticated() { return authenticated; }`

## Notable Comments

- /*
- */
