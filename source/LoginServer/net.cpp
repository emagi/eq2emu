/*  
	EQ2Emulator:  Everquest II Server Emulator
	Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

	This file is part of EQ2Emulator.
*/
#include "../common/debug.h"

#include <iostream>
#include <string.h>
#include <time.h>
#include <signal.h>

#include <sstream>
#include <string>
#include <iostream>

#include "../common/queue.h"
#include "../common/timer.h"

#include "../common/seperator.h"

#include "net.h"
#include "client.h"

#include "LoginDatabase.h"
#include "LWorld.h"
#include "../common/packet_functions.h"
#include "../common/EQStreamFactory.h"
#include "../common/MiscFunctions.h"
#include "../common/version.h"

#include "../common/PacketStruct.h"
#include "../common/DataBuffer.h"
#include "../common/ConfigReader.h"
#include "../common/Log.h"
#include "../common/JsonParser.h"
#include "../common/Common_Defines.h"

#ifdef WIN32
	#define snprintf	_snprintf
	#define vsnprintf	_vsnprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp  _stricmp
	#include <conio.h>
#else
	#include <stdlib.h>
	#include "../common/unix.h"
#endif
EQStreamFactory eqsf(LoginStream);
map<int16,OpcodeManager*>EQOpcodeManager;
//TCPServer eqns(5999);
NetConnection net;
ClientList client_list;
LWorldList world_list;
LoginDatabase database;
ConfigReader configReader;
map<int16, int16> EQOpcodeVersions;
Timer statTimer(60000);

volatile bool RunLoops = true;
bool ReadLoginConfig();

#ifdef PUBLICLOGIN
char version[200], consoletitle[200];
#endif
#include "../common/timer.h"

#include "../common/CRC16.h"
#include <fstream>

int main(int argc, char** argv){
#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	if (signal(SIGINT, CatchSignal) == SIG_ERR) {
		cerr << "Could not set signal handler" << endl;
	}

	LogStart();

	LogParseConfigs();
	net.WelcomeHeader();

	srand(time(NULL));

	if(!net.ReadLoginConfig())
		return 1;

	net.InitWebServer(net.GetWebLoginAddress(), net.GetWebLoginPort(), net.GetWebCertFile(), net.GetWebKeyFile(), net.GetWebKeyPassword(), net.GetWebHardcodeUser(), net.GetWebHardcodePassword());
	
	const char* structList[] = { "CommonStructs.xml", "LoginStructs.xml" };

	for (int s = 0; s < sizeof(structList) / sizeof(const char*); s++)
	{
		LogWrite(INIT__INFO, 0, "Init", "Loading Structs File %s..", structList[s]);
		if (configReader.processXML_Elements(structList[s]))
			LogWrite(INIT__INFO, 0, "Init", "Loading Structs File %s completed..", structList[s]);
		else
		{
			LogWrite(INIT__ERROR, 0, "Init", "Loading Structs File %s FAILED!", structList[s]);
			return 1;
		}
	}
	

	LogWrite(INIT__INFO, 0, "Init", "Initialize World List..");
	world_list.Init();
	
	if(eqsf.listen_ip_address)
		LogWrite(INIT__INFO, 0, "Init", "Login server listening on %s port %i", eqsf.listen_ip_address, net.GetPort());
	else
		LogWrite(INIT__INFO, 0, "Init", "Login server listening on port %i", net.GetPort());
	/*}
	else {
		cout << "EQNetworkServer.Open() error" << endl;
		return 1;
	}*/
	if (!eqsf.Open(net.GetPort())) {
		LogWrite(INIT__ERROR, 0, "Init", "Failed to open port %i.", net.GetPort());
		return 1;
	}
	net.login_running = true;
	net.login_uptime = getCurrentTimestamp();
	
	net.UpdateWindowTitle();
	EQStream* eqs;
	Timer* TimeoutTimer = new Timer(5000);
	TimeoutTimer->Start();
	while(RunLoops) {
		Timer::SetCurrentTime();
		while ((eqs = eqsf.Pop())) {
			struct in_addr	in;
			in.s_addr = eqs->GetRemoteIP();

			LogWrite(LOGIN__INFO, 0, "Login", "New client from IP: %s on port %i", inet_ntoa(in), ntohs(eqs->GetRemotePort()));
			Client* client = new Client(eqs);
			eqs->SetClientVersion(0);
			client_list.Add(client);
			net.numclients++;
			net.UpdateWindowTitle();
		}
		if(TimeoutTimer->Check()){
			eqsf.CheckTimeout();
		}
		if(statTimer.Check()){
			world_list.UpdateWorldStats();
			database.RemoveOldWorldServerStats();
			database.FixBugReport();
		}
		client_list.Process();
		world_list.Process();
#ifdef WIN32
		if(kbhit())
		{
			int hitkey = getch();
			net.HitKey(hitkey);
		}
#endif
		Sleep(1);
	}
	//close
	//eqns.Close();
	eqsf.Close();
	world_list.Shutdown();
	return 0;
}
#ifdef WIN32
void NetConnection::HitKey(int keyhit)
{
	switch(keyhit)
	{
	case 'l':
	case 'L': {
		world_list.ListWorldsToConsole();
		break;
	}
	case 'v':
	case 'V':
	{
		printf("========Version Info=========\n");
		printf("%s %s\n", EQ2EMU_MODULE, CURRENT_VERSION);
		printf("Last Compiled on %s %s\n", COMPILE_DATE, COMPILE_TIME);
		printf("=============================\n\n");
		break;
	}
	case 'H':
	case 'h': {
		printf("===========Help=============\n");
		printf("Available Commands:\n");
		printf("l = Listing of World Servers\n");
		printf("v = Login Version\n");
//		printf("0 = Kick all connected world servers\n");
		printf("============================\n\n");
		break;
	}
	default:
		printf("Invalid Command.\n");
		break;
	}
}
#endif
	
void CatchSignal(int sig_num) {
	cout << "Got signal " << sig_num << endl;
	RunLoops = false;
}

bool NetConnection::ReadLoginConfig() {
	JsonParser parser(MAIN_CONFIG_FILE);
	if(!parser.IsLoaded()) {
		LogWrite(INIT__ERROR, 0, "Init", "Failed to find %s in server directory..", MAIN_CONFIG_FILE);
		return false;
	}
	std::string serverport = parser.getValue("loginconfig.serverport");
	std::string serverip = parser.getValue("loginconfig.serverip");
	
    if (!parser.convertStringToUnsignedShort(serverport, port)) {
		LogWrite(INIT__ERROR, 0, "Init", "Failed to translate loginconfig.serverport..");
		return false;
	}
	
	if(serverip.size() > 0) {
		eqsf.listen_ip_address = new char[serverip.size() + 1];
		strcpy(eqsf.listen_ip_address, serverip.c_str());
	}
	else {
		safe_delete(eqsf.listen_ip_address);
		eqsf.listen_ip_address = nullptr;
	}
	
	std::string acctcreate_str = parser.getValue("loginconfig.accountcreation");
	int16 allow_acct = 0;
    parser.convertStringToUnsignedShort(acctcreate_str, allow_acct);
	allowAccountCreation = allow_acct > 0 ? true : false;
	
	std::string expflag_str = parser.getValue("loginconfig.expansionflag");
	parser.convertStringToUnsignedInt(expflag_str, expansionFlag);
	
	std::string citiesflag_str = parser.getValue("loginconfig.citiesflag");
	parser.convertStringToUnsignedChar(citiesflag_str, citiesFlag);
	
	std::string defaultsublevel_str = parser.getValue("loginconfig.defaultsubscriptionlevel");
	parser.convertStringToUnsignedInt(defaultsublevel_str, defaultSubscriptionLevel);
	
	std::string enableraces_str = parser.getValue("loginconfig.enabledraces");
	parser.convertStringToUnsignedInt(enableraces_str, enabledRaces);
	
	web_loginaddress = parser.getValue("loginconfig.webloginaddress");
	web_certfile = parser.getValue("loginconfig.webcertfile");
	web_keyfile = parser.getValue("loginconfig.webkeyfile");
	web_keypassword = parser.getValue("loginconfig.webkeypassword");
	web_hardcodeuser = parser.getValue("loginconfig.webhardcodeuser");
	web_hardcodepassword = parser.getValue("loginconfig.webhardcodepassword");
	
	std::string webloginport_str = parser.getValue("loginconfig.webloginport");
    parser.convertStringToUnsignedShort(webloginport_str, web_loginport);

	LogWrite(INIT__INFO, 0, "Init", "%s loaded..", MAIN_CONFIG_FILE);


	LogWrite(INIT__INFO, 0, "Init", "Database init begin..");
	//remove this when all database calls are using the new database class
	if (!database.Init()) {
		LogWrite(INIT__ERROR, 0, "Init", "Database init FAILED!");
		LogStop();
		return false;
	}

	LogWrite(INIT__INFO, 0, "Init", "Loading opcodes 2.0..");
	EQOpcodeVersions = database.GetVersions();
	map<int16,int16>::iterator version_itr2;
	int16 version1 = 0;
	for (version_itr2 = EQOpcodeVersions.begin(); version_itr2 != EQOpcodeVersions.end(); version_itr2++) {
		version1 = version_itr2->first;
		EQOpcodeManager[version1] = new RegularOpcodeManager();
		map<string, uint16> eq = database.GetOpcodes(version1);
		if(!EQOpcodeManager[version1]->LoadOpcodes(&eq)) {
			LogWrite(INIT__ERROR, 0, "Init", "Loading opcodes failed. Make sure you have sourced the opcodes.sql file!");
			return false;
		}
	}

	return true;
}

void NetConnection::UpdateWindowTitle(char* iNewTitle) {
#ifdef WIN32
	char tmp[500];
	if (iNewTitle) {
		snprintf(tmp, sizeof(tmp), "Login: %s", iNewTitle);
	}
	else {
		snprintf(tmp, sizeof(tmp), "%s, Version: %s: %i Server(s), %i Client(s) Connected", EQ2EMU_MODULE, CURRENT_VERSION, net.numservers, net.numclients);
	}
	SetConsoleTitle(tmp);
#endif
}

void NetConnection::WelcomeHeader()
{
#ifdef _WIN32
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console, FOREGROUND_WHITE_BOLD);
#endif
	printf("Module: %s, Version: %s", EQ2EMU_MODULE, CURRENT_VERSION);
#ifdef _WIN32
	SetConsoleTextAttribute(console, FOREGROUND_YELLOW_BOLD);
#endif
	printf("\n\nCopyright (C) 2007-2021 EQ2Emulator. https://www.eq2emu.com \n\n");
	printf("EQ2Emulator is free software: you can redistribute it and/or modify\n");
	printf("it under the terms of the GNU General Public License as published by\n");
	printf("the Free Software Foundation, either version 3 of the License, or\n");
	printf("(at your option) any later version.\n\n");
	printf("EQ2Emulator is distributed in the hope that it will be useful,\n");
	printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	printf("GNU General Public License for more details.\n\n");
#ifdef _WIN32
	SetConsoleTextAttribute(console, FOREGROUND_GREEN_BOLD);
#endif
	printf(" /$$$$$$$$  /$$$$$$   /$$$$$$  /$$$$$$$$                        \n");
	printf("| $$_____/ /$$__  $$ /$$__  $$| $$_____/                        \n");
	printf("| $$      | $$  \\ $$|__/  \\ $$| $$       /$$$$$$/$$$$  /$$   /$$\n");
	printf("| $$$$$   | $$  | $$  /$$$$$$/| $$$$$   | $$_  $$_  $$| $$  | $$\n");
	printf("| $$__/   | $$  | $$ /$$____/ | $$__/   | $$ \\ $$ \\ $$| $$  | $$\n");
	printf("| $$      | $$/$$ $$| $$      | $$      | $$ | $$ | $$| $$  | $$\n");
	printf("| $$$$$$$$|  $$$$$$/| $$$$$$$$| $$$$$$$$| $$ | $$ | $$|  $$$$$$/\n");
	printf("|________/ \\____ $$$|________/|________/|__/ |__/ |__/ \\______/ \n");
	printf("                \\__/                                            \n\n");
#ifdef _WIN32
	SetConsoleTextAttribute(console, FOREGROUND_MAGENTA_BOLD);
#endif
	printf(" Website     : https://eq2emu.com \n");
	printf(" Wiki        : https://wiki.eq2emu.com \n");
	printf(" Git         : https://git.eq2emu.com \n");
	printf(" Discord     : https://discord.gg/5Cavm9NYQf \n\n");
#ifdef _WIN32
	SetConsoleTextAttribute(console, FOREGROUND_WHITE_BOLD);
#endif
	printf("For more detailed logging, modify 'Level' param the log_config.xml file.\n\n");
#ifdef _WIN32
	SetConsoleTextAttribute(console, FOREGROUND_WHITE);
#endif

	fflush(stdout);
}

void NetConnection::InitWebServer(std::string web_ipaddr, int16 web_port, std::string cert_file, std::string key_file, std::string key_password, std::string hardcode_user, std::string hardcode_password) {
	if(web_ipaddr.size() > 0 && web_port > 0) {
		try {
			login_webserver = new WebServer(web_ipaddr, web_port, cert_file, key_file, key_password, hardcode_user, hardcode_password);
			
			login_webserver->register_route("/status", NetConnection::Web_loginhandle_status);
			login_webserver->register_route("/worlds", NetConnection::Web_loginhandle_worlds);
			login_webserver->run();
			LogWrite(INIT__INFO, 0, "Init", "Login Web Server is listening on %s:%u..", web_ipaddr.c_str(), web_port);
		}
		catch (const std::exception& e) {
			LogWrite(INIT__ERROR, 0, "Init", "Login Web Server failed to listen on %s:%u due to reason %s", web_ipaddr.c_str(), web_port, e.what());
		}
	}
}