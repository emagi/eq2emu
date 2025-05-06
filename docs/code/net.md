# File: `net.h`

## Classes

- `NetConnection`
- `ZoneAuthRequest`
- `ZoneAuth`

## Functions

- `ThreadReturnType EQ2ConsoleListener(void *tmp);`
- `void CatchSignal(int sig_num);`
- `void UpdateWindowTitle(char* iNewTitle);`
- `bool ReadLoginINI(int argc, char** argv);`
- `void WelcomeHeader();`
- `void SetPrimary(bool isprimary = true);`
- `std::string GetWebWorldAddress()		{ return web_worldaddress; }`
- `std::string GetWebCertFile()		{ return web_certfile; }`
- `std::string GetWebKeyFile()		{ return web_keyfile; }`
- `std::string GetWebKeyPassword()		{ return web_keypassword; }`
- `std::string GetWebHardcodeUser()		{ return web_hardcodeuser; }`
- `std::string GetWebHardcodePassword()		{ return web_hardcodepassword; }`
- `std::string GetCmdUser()		{ return web_cmduser; }`
- `std::string GetCmdPassword()		{ return web_cmdpassword; }`
- `int16 GetPeerPriority() { return web_peerpriority; }`
- `int32	GetAccountID() { return accountid; }`
- `int32	GetAccessKey() { return accesskey; }`
- `int32	GetTimeStamp() { return timestamp; }`
- `void	setFirstLogin(bool value) { firstlogin = value; }`
- `bool	isFirstLogin() { return firstlogin; }`
- `void				AddAuth(ZoneAuthRequest* zar);`
- `void				PurgeInactiveAuth();`
- `void				RemoveAuth(ZoneAuthRequest* zar);`

## Notable Comments

- /*
- */
- // Create a copy of the existing multimap
