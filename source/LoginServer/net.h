/*  
	EQ2Emulator:  Everquest II Server Emulator
	Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

	This file is part of EQ2Emulator.
*/
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <winsock.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
#endif

//#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <atomic>

#include "../common/types.h"
#include "../common/Web/WebServer.h"
#include "../common/MiscFunctions.h"

void CatchSignal(int sig_num);
enum eServerMode { Standalone, Master, Slave, Mesh };

class NetConnection
{
public:
	NetConnection() {
		port = 5999;
		listening_socket = 0;
		memset(masteraddress, 0, sizeof(masteraddress));
		uplinkport = 0;
		memset(uplinkaccount, 0, sizeof(uplinkaccount));
		memset(uplinkpassword, 0, sizeof(uplinkpassword));
		LoginMode = Standalone;
		Uplink_WrongVersion = false;
		numclients = 0;
		numservers = 0;
		allowAccountCreation = true;
		
		// full support = 0x7CFF
		// 1 << 12 (-4096) = missing echoes of faydwer, disables Fae and Arasai (black portraits) and kelethin as starting city
		// 1 << 13 (-8192) = disables sarnak (black portraits) and gorowyn as starting city
		expansionFlag = 0x7CFF; // 0x4CF5 
		
		/* dword_1ECBA18 operand for race flag packs (sublevel 0,1,2?) -- (sublevel -1) controls starting zones omission 0xEE vs 0xCF (CF misses halas)
		1 = city of qeynos
		2 = city of freeport
		4 = city of kelethin
		8 = city of neriak
		16 = gorowyn
		32 = new halas
		64 = queens colony
		128 = outpost overlord
		*/
		citiesFlag = 0xFF;
		
		// sub_level 0xFFFFFFFF = blacks out all portraits for class alignments, considered non membership
		// sub_level > 0 = class alignments still required, but portraits are viewable and race selectable
		// sub_level = 2 membership, you can 'create characters on time locked servers' vs standard
		// sub_level = 0 forces popup on close to web browser
		defaultSubscriptionLevel = 0xFFFFFFFF;
		
		// disable extra races FAE(16) ARASAI (17) SARNAK (18) -- with 4096/8192 flags, no visibility of portraits
		enabledRaces = 0xFFFF; // 0xCFFF
		
		web_loginport = 0;
		
		login_webserver = nullptr;
		
		login_running = false;
		login_uptime = getCurrentTimestamp();
	}
	
	~NetConnection() {
		safe_delete(login_webserver);
	}
	
	void UpdateWindowTitle(char* iNewTitle = 0);
	bool Init();
	void ListenNewClients();
	void HitKey(int keyhit);
	char address[1024];
	int32 numclients;
	int32 numservers;

	int16	GetPort()	{ return port; }
	void	SetPort(int16 in_port) { port = in_port; }
	eServerMode	GetLoginMode()	{ return LoginMode; }

	bool	ReadLoginConfig();
	char*	GetMasterAddress()	{ return masteraddress; }
	int16	GetUplinkPort()		{ if (uplinkport != 0) return uplinkport; else return port; }
	char*	GetUplinkAccount()	{ return uplinkaccount; }
	char*	GetUplinkPassword()	{ return uplinkpassword; }

	bool	IsAllowingAccountCreation() { return allowAccountCreation; }
	int32	GetExpansionFlag() { return expansionFlag; }
	int8	GetCitiesFlag() { return citiesFlag; }
	int32	GetDefaultSubscriptionLevel() { return defaultSubscriptionLevel; }
	int32	GetEnabledRaces() { return enabledRaces; }
	std::string GetWebLoginAddress()		{ return web_loginaddress; }
	inline int16 GetWebLoginPort()			{ return web_loginport; }
	std::string GetWebCertFile()			{ return web_certfile; }
	std::string GetWebKeyFile()			{ return web_keyfile; }
	std::string GetWebKeyPassword()		{ return web_keypassword; }
	std::string GetWebHardcodeUser()		{ return web_hardcodeuser; }
	std::string GetWebHardcodePassword()		{ return web_hardcodepassword; }
	void	WelcomeHeader();
	
	void 	InitWebServer(std::string web_ipaddr, int16 web_port, std::string cert_file, std::string key_file, std::string key_password, std::string hardcode_user, std::string hardcode_password);
	
	static void	Web_loginhandle_status(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void	Web_loginhandle_worlds(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	
	bool	login_running;
	std::atomic<int64> login_uptime;
protected:
	friend class LWorld;
	bool	Uplink_WrongVersion;
private:
	int16	port;
	int		listening_socket;
	char	masteraddress[300];
	int16	uplinkport;
	char	uplinkaccount[300];
	char	uplinkpassword[300];
	eServerMode	LoginMode;
	bool	allowAccountCreation;
	int32	expansionFlag;
	int8	citiesFlag;
	int32	defaultSubscriptionLevel;
	int32	enabledRaces;
	std::string	web_loginaddress;
	std::string	web_certfile;
	std::string	web_keyfile;
	std::string	web_keypassword;
	std::string	web_hardcodeuser;
	std::string	web_hardcodepassword;
	int16	web_loginport;	
	WebServer* login_webserver;
};
