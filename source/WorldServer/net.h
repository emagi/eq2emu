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
#ifndef __EQ2_NET__
#define	__EQ2_NET__
#ifndef WIN32
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <errno.h>
	#include <fcntl.h>
#else
	#include <cerrno>
	#include <fcntl.h>
	#include <WinSock2.h>
	#include <windows.h>
#endif

#include "../common/linked_list.h"
#include "../common/types.h"

ThreadReturnType EQ2ConsoleListener(void *tmp);
void CatchSignal(int sig_num);
void UpdateWindowTitle(char* iNewTitle);

#define PORT		9000
#define LOGIN_PORT	9100

class NetConnection
{
public:
	NetConnection() {
		world_locked = false;
		for (int i=0; i<4; i++) {
			memset(loginaddress[i], 0, sizeof(loginaddress[i]));
			loginport[i] = LOGIN_PORT;
		}
		listening_socket = 0;
		memset(worldname, 0, sizeof(worldname));
		memset(worldaccount, 0, sizeof(worldaccount));
		memset(worldpassword, 0, sizeof(worldpassword));
		memset(worldaddress, 0, sizeof(worldaddress));
		memset(internalworldaddress, 0, sizeof(internalworldaddress));
		worldport = PORT;
		DEFAULTSTATUS=0;
		LoginServerInfo = 0;//ReadLoginINI();
		UpdateStats = false;
		web_worldport = 0;
	}
	~NetConnection() { }

	bool ReadLoginINI();
	void WelcomeHeader();

	bool LoginServerInfo;
	bool UpdateStats;
	char* GetLoginInfo(int16* oPort);
	inline char* GetLoginAddress(int8 i)	{ return loginaddress[i]; }
	inline int16 GetLoginPort(int8 i)		{ return loginport[i]; }
	inline char* GetWorldName()			{ return worldname; }
	inline char* GetWorldAccount()			{ return worldaccount; }
	inline char* GetWorldPassword()		{ return worldpassword; }
	inline char* GetWorldAddress()			{ return worldaddress; }
	inline char* GetInternalWorldAddress()	{ return internalworldaddress; }
	inline int16 GetWorldPort()				{ return worldport; }
	inline int8 GetDefaultStatus()			{ return DEFAULTSTATUS; }
	std::string GetWebWorldAddress()		{ return web_worldaddress; }
	inline int16 GetWebWorldPort()			{ return web_worldport; }
	std::string GetWebCertFile()		{ return web_certfile; }
	std::string GetWebKeyFile()		{ return web_keyfile; }
	std::string GetWebKeyPassword()		{ return web_keypassword; }
	std::string GetWebHardcodeUser()		{ return web_hardcodeuser; }
	std::string GetWebHardcodePassword()		{ return web_hardcodepassword; }
	bool world_locked;
private:
	int		listening_socket;
	char	loginaddress[4][255];
	int16	loginport[4];
	char	worldname[201];
	char	worldaccount[31];
	char	worldpassword[31];
	char	worldaddress[255];
	char	internalworldaddress[21];
	int16	worldport;
	int8    DEFAULTSTATUS;
	std::string	web_worldaddress;
	std::string	web_certfile;
	std::string	web_keyfile;
	std::string	web_keypassword;
	std::string	web_hardcodeuser;
	std::string	web_hardcodepassword;
	int16	web_worldport;

};

class ZoneAuthRequest
{
public:
	ZoneAuthRequest(int32 account_id, char* name, int32 access_key);
	~ZoneAuthRequest( );
	int32	GetAccountID() { return accountid; }
	const char*	GetCharacterName() { return character_name.c_str(); }
	int32	GetAccessKey() { return accesskey; }
	int32	GetTimeStamp() { return timestamp; }
	void	setFirstLogin(bool value) { firstlogin = value; }
	bool	isFirstLogin() { return firstlogin; }
private:
	int32 accountid;
	string character_name;
	int32 accesskey;
	int32 timestamp;
	bool firstlogin;
};

class ZoneAuth
{
public:
	void				AddAuth(ZoneAuthRequest* zar);
	ZoneAuthRequest*	GetAuth(int32 account_id, int32 access_key);
	void				PurgeInactiveAuth();
	void				RemoveAuth(ZoneAuthRequest* zar);
private:
	LinkedList<ZoneAuthRequest*> list;
};
#endif
