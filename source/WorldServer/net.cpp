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
#include "../common/debug.h"
#include "../common/Log.h"

#include <iostream>
using namespace std;
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm> 
#include <chrono>

#include <signal.h>

#include "../common/queue.h"
#include "../common/timer.h"
#include "../common/EQStreamFactory.h"
#include "../common/EQStream.h"
#include "net.h"

#include "Variables.h"
#include "WorldDatabase.h"
#include "../common/seperator.h"
#include "../common/version.h"
#include "../common/EQEMuError.h"
#include "../common/opcodemgr.h"
#include "../common/Common_Defines.h"
#include "../common/JsonParser.h"
#include "../common/Common_Defines.h"

#include "LoginServer.h"
#include "Commands/Commands.h"
#include "Factions.h"
#include "World.h"
#include "../common/ConfigReader.h"
#include "Skills.h"
#include "LuaInterface.h"
#include "Guilds/Guild.h"
#include "Commands/ConsoleCommands.h"
#include "Traits/Traits.h"
#include "Transmute.h"
#include "Zone/ChestTrap.h"

//devn00b
#ifdef DISCORD
	//linux only for the moment.
	#ifndef WIN32
		#include <dpp/dpp.h>
		#include "Chat/Chat.h"
		extern Chat chat;
	#endif
#endif

double frame_time = 0.0;

#ifdef WIN32
	#include <process.h>
	#define strncasecmp	_strnicmp
	#define strcasecmp  _stricmp
	#include <conio.h>
#else
	#include <pthread.h>
	#include "../common/unix.h"
#endif

#ifdef PROFILER
#define SHINY_PROFILER TRUE
#include "../Profiler/src/Shiny.h"
#endif

NetConnection net;
World world;
EQStreamFactory eqsf(LoginStream);
LoginServer loginserver;
LuaInterface* lua_interface = new LuaInterface();
#include "MutexList.h"

#include "Rules/Rules.h"
#include "Titles.h"
#include "Languages.h"
#include "Achievements/Achievements.h"

volatile bool RunLoops = true;
sint32 numclients = 0;
sint32 numzones = 0;
extern ClientList client_list;
extern ZoneList zone_list;
extern MasterFactionList master_faction_list;
extern WorldDatabase database;
extern MasterSpellList master_spell_list;
extern MasterTraitList master_trait_list;
extern MasterSkillList master_skill_list;
extern MasterItemList master_item_list;
extern GuildList guild_list;
extern Variables variables;
ConfigReader configReader;
int32 MasterItemList::next_unique_id = 0;
int last_signal = 0;
RuleManager rule_manager;
MasterTitlesList master_titles_list;
MasterLanguagesList master_languages_list;
ChestTrapList chest_trap_list;
extern MasterAchievementList master_achievement_list;
extern map<int16, int16> EQOpcodeVersions;


ThreadReturnType ItemLoad (void* tmp);
ThreadReturnType AchievmentLoad (void* tmp);
ThreadReturnType SpellLoad (void* tmp);
//devn00b
#ifdef DISCORD
	#ifndef WIN32
		ThreadReturnType StartDiscord (void* tmp);
	#endif
#endif

int main(int argc, char** argv) {
#ifdef PROFILER
	PROFILE_FUNC();
#endif
	int32 t_total = Timer::GetUnixTimeStamp();

	LogStart();

	LogParseConfigs();
	net.WelcomeHeader();

	LogWrite(INIT__INFO, 0, "Init", "Starting EQ2Emulator WorldServer...");
	//int32 server_startup = time(NULL);

	//remove this when all database calls are using the new database class
	if (!database.Init()) {
		LogStop();
		LogWrite(INIT__ERROR, 0, "Init", "Database init failed!");
		return EXIT_FAILURE;
	}

	if (!database.ConnectNewDatabase()) {
		LogWrite(INIT__ERROR, 0, "Init", "Cannot connect to database!");
		return EXIT_FAILURE;
	}

	#ifdef _DEBUG
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif
	
	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		LogWrite(INIT__ERROR, 0, "Init", "Could not set signal handler");
		return 0;
	}
	if (signal(SIGSEGV, CatchSignal) == SIG_ERR)	{
		LogWrite(INIT__ERROR, 0, "Init", "Could not set signal handler");
		return 0;
	}
	if (signal(SIGILL, CatchSignal) == SIG_ERR)	{
		LogWrite(INIT__ERROR, 0, "Init", "Could not set signal handler");
		return 0;
	}
	#ifndef WIN32
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)	{
		LogWrite(INIT__ERROR, 0, "Init", "Could not set signal handler");
		return 0;
	}
	#endif
	
	LogWrite(WORLD__DEBUG, 0, "World", "Randomizing World...");
	srand(time(NULL));

	net.ReadLoginINI();
		
	// JA: Grouping all System (core) data loads together for timing purposes
	LogWrite(WORLD__INFO, 0, "World", "Loading System Data...");
	int32 t_now = Timer::GetUnixTimeStamp();

	LogWrite(WORLD__DEBUG, 1, "World", "-Loading opcodes...");
	EQOpcodeVersions = database.GetVersions();
	map<int16,int16>::iterator version_itr;
	int16 version1 = 0;
	int16 prevVersion = 0;
	std::string prevString = std::string("");
	std::string builtString = std::string("");
	for (version_itr = EQOpcodeVersions.begin(); version_itr != EQOpcodeVersions.end(); version_itr++) {
		version1 = version_itr->first;
		EQOpcodeManager[version1] = new RegularOpcodeManager();
		map<string, uint16> eq = database.GetOpcodes(version1);
		std::string missingOpcodesList = std::string("");
		if(!EQOpcodeManager[version1]->LoadOpcodes(&eq, &missingOpcodesList)) {
			LogWrite(INIT__ERROR, 0, "Init", "Loading opcodes failed. Make sure you have sourced the opcodes.sql file!");
			return false;
		}
		
		if(version1 == 0) // we don't need to display version 0
			continue;
		
		if(prevString.size() > 0) {
			if(prevString == missingOpcodesList) {
				builtString += ", " + std::to_string(version1);
			}
			else {
					LogWrite(OPCODE__WARNING, 1, "Opcode", "Opcodes %s.", builtString.c_str());
					builtString = std::string("");
					prevString = std::string("");
			}
		}
		if(prevString.size() < 1) {
			prevString = std::string(missingOpcodesList);
			builtString = std::string(missingOpcodesList + " are missing from the opcodes table for version(s) " + std::to_string(version1));
		}
	}

	if(builtString.size() > 0) {
		LogWrite(OPCODE__WARNING, 1, "Opcode", "Opcodes %s.", builtString.c_str());
	}

	LogWrite(WORLD__DEBUG, 1, "World", "-Loading structs...");
	if(!configReader.LoadFile("CommonStructs.xml") || !configReader.LoadFile("WorldStructs.xml") || !configReader.LoadFile("SpawnStructs.xml") || !configReader.LoadFile("ItemStructs.xml")) {
		LogWrite(INIT__ERROR, 0, "Init", "Loading structs failed. Make sure you have CommonStructs.xml, WorldStructs.xml, SpawnStructs.xml, and ItemStructs.xml in the working directory!");
		return false;
	}

	world.init(net.GetWebWorldAddress(), net.GetWebWorldPort(), net.GetWebCertFile(), net.GetWebKeyFile(), net.GetWebKeyPassword(), net.GetWebHardcodeUser(), net.GetWebHardcodePassword());

	bool threadedLoad = rule_manager.GetGlobalRule(R_World, ThreadedLoad)->GetBool();

	LogWrite(WORLD__DEBUG, 1, "World", "-Loading EQ2 time of day...");
	loginserver.InitLoginServerVariables();

	LogWrite(WORLD__INFO, 0, "World", "Loaded System Data (took %u seconds)", Timer::GetUnixTimeStamp() - t_now);
	// JA: End System Data loading functions

	if (threadedLoad) {
		LogWrite(WORLD__WARNING, 0, "Threaded", "Using Threaded loading of static data...");
#ifdef WIN32
		_beginthread(ItemLoad, 0, &world);
		_beginthread(SpellLoad, 0, &world);
		//_beginthread(AchievmentLoad, 0, &world);
#else
		pthread_t thread;
		pthread_create(&thread, NULL, ItemLoad, &world);
		pthread_detach(thread);
		pthread_t thread2;
		pthread_create(&thread2, NULL, SpellLoad, &world);
		pthread_detach(thread2);
		//devn00b
		#ifdef DISCORD
		pthread_t thread3;
		pthread_create(&thread3, NULL, StartDiscord, &world);
		pthread_detach(thread3);
		#endif
#endif
	}

	// JA temp logger
	LogWrite(MISC__TODO, 0, "Reformat", "JA: This is as far as I got reformatting the console logs.");

	if (!threadedLoad) {
		// JA: Load all Item info
		LogWrite(ITEM__INFO, 0, "Items", "Loading Items...");
		database.LoadItemList();
		MasterItemList::ResetUniqueID(database.LoadNextUniqueItemID());
		
		LogWrite(SPELL__INFO, 0, "Spells", "Loading Spells...");
		database.LoadSpells();

		LogWrite(SPELL__INFO, 0, "Spells", "Loading Spell Errors...");
		database.LoadSpellErrors();

		// Jabantiz: Load traits
		LogWrite(WORLD__INFO, 0, "Traits", "Loading Traits...");
		database.LoadTraits();
		
		// JA: Load all Quest info
		LogWrite(QUEST__INFO, 0, "Quests", "Loading Quests...");
		database.LoadQuests();
		
		LogWrite(COLLECTION__INFO, 0, "Collect", "Loading Collections...");
		database.LoadCollections();
		
		LogWrite(MISC__TODO, 1, "TODO", "TODO loading achievements\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);
		//LogWrite(ACHIEVEMENT__INFO, 0, "Achievements", "Loading Achievements...");
		//database.LoadAchievements();
		//master_achievement_list.CreateMasterAchievementListPacket();
		
		LogWrite(MERCHANT__INFO, 0, "Merchants", "Loading Merchants...");
		database.LoadMerchantInformation();
	}

	LogWrite(GUILD__INFO, 0, "Guilds", "Loading Guilds...");
	database.LoadGuilds();

	LogWrite(TRADESKILL__INFO, 0, "Recipes", "Loading Recipe Books...");
	database.LoadRecipeBooks();
	LogWrite(TRADESKILL__INFO, 0, "Recipes", "Loading Recipes...");
	database.LoadRecipes();
	LogWrite(TRADESKILL__INFO, 0, "Tradeskills", "Loading Tradeskill Events...");
	database.LoadTradeskillEvents();
	
	LogWrite(SPELL__INFO, 0, "AA", "Loading Alternate Advancements...");
	database.LoadAltAdvancements();
	LogWrite(SPELL__INFO, 0, "AA", "Loading AA Tree Nodes...");
	database.LoadTreeNodes();
	LogWrite(WORLD__INFO, 0, "Titles", "Loading Titles...");
	database.LoadTitles();
	LogWrite(WORLD__INFO, 0, "Languages", "Loading Languages...");
	database.LoadLanguages();

	LogWrite(CHAT__INFO, 0, "Chat", "Loading channels...");
	database.LoadChannels();

	LogWrite(LUA__INFO, 0, "LUA", "Loading Spawn Scripts...");
	database.LoadSpawnScriptData();

	LogWrite(LUA__INFO, 0, "LUA", "Loading Zone Scripts...");
	database.LoadZoneScriptData();

	LogWrite(WORLD__INFO, 0, "World", "Loading House Zone Data...");
	database.LoadHouseZones();
	database.LoadPlayerHouses();

	LogWrite(WORLD__INFO, 0, "World", "Loading Heroic OP Data...");
	database.LoadHOStarters();
	database.LoadHOWheel();

	LogWrite(WORLD__INFO, 0, "World", "Loading Race Types Data...");
	database.LoadRaceTypes();

	LogWrite(WORLD__INFO, 0, "World", "Loading Transmuting Data...");
	database.LoadTransmuting();

	LogWrite(WORLD__INFO, 0, "World", "Loading Chest Trap Data...");
	database.LoadChestTraps();

	LogWrite(WORLD__INFO, 0, "World", "Loading NPC Spells...");
	database.LoadNPCSpells();
	
	if (threadedLoad) {
		LogWrite(WORLD__INFO, 0, "World", "Waiting for load threads to finish.");
		while (!world.items_loaded || !world.spells_loaded /*|| !world.achievments_loaded*/)
			Sleep(10);
		LogWrite(WORLD__INFO, 0, "World", "Load threads finished.");
	}

	LogWrite(WORLD__INFO, 0, "World", "Total World startup time: %u seconds.", Timer::GetUnixTimeStamp() - t_total);
	int ret_code = 0;
	if (eqsf.Open(net.GetWorldPort())) {
		if (strlen(net.GetWorldAddress()) == 0)
			LogWrite(NET__INFO, 0, "Net", "World server listening on port %i", net.GetWorldPort());
		else
			LogWrite(NET__INFO, 0, "Net", "World server listening on: %s:%i", net.GetWorldAddress(), net.GetWorldPort());

		if(strlen(net.GetInternalWorldAddress())>0)
			LogWrite(NET__INFO, 0, "Net", "World server listening on: %s:%i", net.GetInternalWorldAddress(), net.GetWorldPort());
		
		world.world_loaded = true;
		world.world_uptime = getCurrentTimestamp();
	}
	else {
		LogWrite(NET__ERROR, 0, "Net", "Failed to open port %i.", net.GetWorldPort());
		ret_code = 1;
	}
	Timer* TimeoutTimer = 0;
	if (ret_code == 0) {
		Timer InterserverTimer(INTERSERVER_TIMER); // does MySQL pings and auto-reconnect
		InterserverTimer.Trigger();
		TimeoutTimer = new Timer(5000);
		TimeoutTimer->Start();
		EQStream* eqs = 0;
		UpdateWindowTitle(0);
		LogWrite(ZONE__INFO, 0, "Zone", "Starting static zones...");
		database.LoadSpecialZones();
		map<EQStream*, int32> connecting_clients;
		map<EQStream*, int32>::iterator cc_itr;
		
		LogWrite(WORLD__DEBUG, 0, "Thread", "Starting console command thread...");
#ifdef WIN32
		_beginthread(EQ2ConsoleListener, 0, NULL);
#else
		/*pthread_t thread;
		pthread_create(&thread, NULL, &EQ2ConsoleListener, NULL);
		pthread_detach(thread);*/
#endif
		//

		// just before starting loops, announce how to get console help (only to windows)
#ifdef WIN32      
		LogWrite(WORLD__INFO, 0, "Console", "Type 'help' or '?' and press enter for menu options.");
#endif      


		std::chrono::time_point<std::chrono::system_clock> frame_prev = std::chrono::system_clock::now();
		while (RunLoops) {
			Timer::SetCurrentTime();
			std::chrono::time_point<std::chrono::system_clock> frame_now = std::chrono::system_clock::now();
			frame_time = std::chrono::duration_cast<std::chrono::duration<double>>(frame_now - frame_prev).count();
			frame_prev = frame_now;

#ifndef NO_CATCH
			try
			{
#endif
				while ((eqs = eqsf.Pop())) {
					struct in_addr	in;
					in.s_addr = eqs->GetRemoteIP();
					LogWrite(NET__DEBUG, 0, "Net", "New client from ip: %s port: %i", inet_ntoa(in), ntohs(eqs->GetRemotePort()));

					// JA: Check for BannedIPs
					if (rule_manager.GetGlobalRule(R_World, UseBannedIPsTable)->GetInt8() == 1)
					{
						LogWrite(WORLD__DEBUG, 0, "World", "Checking inbound connection %s against BannedIPs table", inet_ntoa(in));
						if (database.CheckBannedIPs(inet_ntoa(in)))
						{
							LogWrite(WORLD__DEBUG, 0, "World", "Connection from %s FAILED banned IPs check.  Closing connection.", inet_ntoa(in));
							eqs->Close(); // JA: If the inbound IP is on the banned table, close the EQStream.
						}
					}

					if (eqs && eqs->CheckActive() && client_list.ContainsStream(eqs) == false) {
						LogWrite(NET__DEBUG, 0, "Net", "Adding new client...");
						Client* client = new Client(eqs);
						client_list.Add(client);
					}
					else if (eqs && !client_list.ContainsStream(eqs)) {
						LogWrite(NET__DEBUG, 0, "Net", "Adding client to waiting list...");
						connecting_clients[eqs] = Timer::GetCurrentTime2();
					}
				}
				if (connecting_clients.size() > 0) {
					for (cc_itr = connecting_clients.begin(); cc_itr != connecting_clients.end(); cc_itr++) {
						if (cc_itr->first && cc_itr->first->CheckActive() && client_list.ContainsStream(cc_itr->first) == false) {
							LogWrite(NET__DEBUG, 0, "Net", "Removing client from waiting list...");
							Client* client = new Client(cc_itr->first);
							client_list.Add(client);
							connecting_clients.erase(cc_itr);
							break;
						}
						else if (Timer::GetCurrentTime2() >= (cc_itr->second + 10000)) {
							connecting_clients.erase(cc_itr);
							break;
						}
					}
				}
				world.Process();
				client_list.Process();
				loginserver.Process();
				if (TimeoutTimer->Check()) {
					eqsf.CheckTimeout();
				}
				if (InterserverTimer.Check()) {
					InterserverTimer.Start();
					database.ping();
					database.PingNewDB();
					database.PingAsyncDatabase();

					if (net.LoginServerInfo && loginserver.Connected() == false && loginserver.CanReconnect()) {
						LogWrite(WORLD__DEBUG, 0, "Thread", "Starting autoinit loginserver thread...");
#ifdef WIN32
						_beginthread(AutoInitLoginServer, 0, NULL);
#else
						pthread_t thread;
						pthread_create(&thread, NULL, &AutoInitLoginServer, NULL);
						pthread_detach(thread);
#endif
					}
				}
#ifndef NO_CATCH
			}
			catch (...) {
				LogWrite(WORLD__ERROR, 0, "World", "Exception caught in net main loop!");
			}
#endif
			if (numclients == 0) {
				Sleep(10);
				continue;
			}
			Sleep(1);
		}
	}
	LogWrite(WORLD__DEBUG, 0, "World", "The world is ending!");

	LogWrite(WORLD__DEBUG, 0, "World", "Shutting down zones...");
	zone_list.ShutDownZones();

	LogWrite(WORLD__DEBUG, 0, "World", "Shutting down LUA interface...");
	safe_delete(lua_interface);
	safe_delete(TimeoutTimer);
	eqsf.Close();
	map<int16, OpcodeManager*>::iterator opcode_itr;
	for(opcode_itr=EQOpcodeManager.begin();opcode_itr!=EQOpcodeManager.end();opcode_itr++){
		safe_delete(opcode_itr->second);
	}
	CheckEQEMuErrorAndPause();

#ifdef PROFILER
	PROFILER_UPDATE();
	PROFILER_OUTPUT();
#endif

	LogWrite(WORLD__INFO, 0, "World", "Exiting... we hope you enjoyed your flight.");
	LogStop();
	return ret_code;
}

ThreadReturnType ItemLoad (void* tmp)
{
	LogWrite(WORLD__WARNING, 0, "Thread", "Item Loading Thread started.");
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#endif
	if (tmp == 0) {
		ThrowError("ItemLoad(): tmp = 0!");
		THREAD_RETURN(NULL);
	}
	World* world = (World*) tmp;
	WorldDatabase db;
	db.Init();
	db.ConnectNewDatabase();
	
	LogWrite(ITEM__INFO, 0, "Items", "Loading Items...");
	db.LoadItemList();
	MasterItemList::ResetUniqueID(db.LoadNextUniqueItemID());

	// Relies on the item list so needs to be in the item thread
	LogWrite(COLLECTION__INFO, 0, "Collect", "Loading Collections...");
	db.LoadCollections();

	LogWrite(MERCHANT__INFO, 0, "Merchants", "Loading Merchants...");
	db.LoadMerchantInformation();

	LogWrite(QUEST__INFO, 0, "Quests", "Loading Quests...");
	db.LoadQuests();

	world->items_loaded = true;
	LogWrite(WORLD__WARNING, 0, "Thread", "Item Loading Thread completed.");

	mysql_thread_end();
	THREAD_RETURN(NULL);
}

ThreadReturnType SpellLoad (void* tmp)
{
	LogWrite(WORLD__WARNING, 0, "Thread", "Spell Loading Thread started.");
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#endif
	if (tmp == 0) {
		ThrowError("ItemLoad(): tmp = 0!");
		THREAD_RETURN(NULL);
	}
	World* world = (World*) tmp;
	WorldDatabase db;
	db.Init();
	db.ConnectNewDatabase();
	LogWrite(SPELL__INFO, 0, "Spells", "Loading Spells...");
	db.LoadSpells();

	LogWrite(SPELL__INFO, 0, "Spells", "Loading Spell Errors...");
	db.LoadSpellErrors();

	LogWrite(WORLD__INFO, 0, "Traits", "Loading Traits...");
	db.LoadTraits();

	world->spells_loaded = true;
	LogWrite(WORLD__WARNING, 0, "Thread", "Spell Loading Thread completed.");

	mysql_thread_end();
	THREAD_RETURN(NULL);
}

ThreadReturnType AchievmentLoad (void* tmp)
{
	LogWrite(WORLD__WARNING, 0, "Thread", "Achievement Loading Thread started.");
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#endif
	if (tmp == 0) {
		ThrowError("ItemLoad(): tmp = 0!");
		THREAD_RETURN(NULL);
	}
	World* world = (World*) tmp;
	WorldDatabase db;
	db.Init();
	db.ConnectNewDatabase();

	LogWrite(ACHIEVEMENT__INFO, 0, "Achievements", "Loading Achievements...");
	int32 t_now = Timer::GetUnixTimeStamp();
	db.LoadAchievements();
	master_achievement_list.CreateMasterAchievementListPacket();
	LogWrite(ACHIEVEMENT__INFO, 0, "Achievements", "Achievements loaded (took %u seconds)", Timer::GetUnixTimeStamp() - t_now);

	world->achievments_loaded = true;
	LogWrite(WORLD__WARNING, 0, "Thread", "Achievement Loading Thread completed.");
	
	mysql_thread_end();
	THREAD_RETURN(NULL);
}

ThreadReturnType EQ2ConsoleListener(void* tmp)
{
	char cmd[300]; 
	size_t i = 0;
	size_t len;

	while( RunLoops )
	{
		// Read in single line from "stdin"
		memset( cmd, 0, sizeof( cmd ) ); 
		if( fgets( cmd, 300, stdin ) == NULL )
			continue;

		if( !RunLoops )
			break;

		len = strlen(cmd);
		for( i = 0; i < len; ++i )
		{
			if(cmd[i] == '\n' || cmd[i] == '\r')
				cmd[i] = '\0';
		}

		ProcessConsoleInput(cmd);
	}
	THREAD_RETURN(NULL);
}

#include <fstream>
void CatchSignal(int sig_num) {
	// In rare cases this can be called after the log system is shut down causing a deadlock or crash
	// when the world shuts down, if this happens again comment out the LogWrite and uncomment the printf
	if (last_signal != sig_num){
		static Mutex lock;
		static std::ofstream signal_out;

		lock.lock();
		if (!signal_out.is_open())
			signal_out.open("signal_catching.log", ios::trunc);
		if (signal_out){
			signal_out << "Caught signal " << sig_num << "\n";
			signal_out.flush();
		}
		printf("Caught signal %i\n", sig_num);
		lock.unlock();

		last_signal = sig_num;
		RunLoops = false;
	}
}

bool NetConnection::ReadLoginINI() {
	JsonParser parser(MAIN_CONFIG_FILE);
	if(!parser.IsLoaded()) {
		LogWrite(INIT__ERROR, 0, "Init", "Failed to find %s in server directory..", MAIN_CONFIG_FILE);
		return false;
	}
	std::string worldname_str = parser.getValue("loginserver.worldname");
	if(worldname_str.size() < 4) {
		LogWrite(INIT__ERROR, 0, "Init", "loginserver.worldname was invalid or less than 4 characters..");
		return false;
	}
	
	std::string worldaccount_str = parser.getValue("loginserver.account");
	std::string worldpassword_str = parser.getValue("loginserver.password");
	std::string worldaddress_str = parser.getValue("loginserver.worldaddress");
	
	snprintf(worldname, sizeof(worldname), "%s", worldname_str.c_str());
	snprintf(worldaccount, sizeof(worldaccount), "%s", worldaccount_str.c_str());
	snprintf(worldpassword, sizeof(worldpassword), "%s", worldpassword_str.c_str());
	snprintf(worldaddress, sizeof(worldaddress), "%s", worldaddress_str.c_str());
	
	std::string logstats_str = parser.getValue("loginserver.logstats");
	int16 logstats = 0;
    parser.convertStringToUnsignedShort(logstats_str, logstats);
	net.UpdateStats = logstats > 0 ? true : false;
	
	std::string locked_str = parser.getValue("loginserver.locked");
	int16 locked = 0;
    parser.convertStringToUnsignedShort(locked_str, locked);
	world_locked = locked > 0 ? true : false;
	
	std::string worldport_str = parser.getValue("loginserver.worldport");
    parser.convertStringToUnsignedShort(worldport_str, worldport);
	
	for(int i=-1;i<=3;i++) {
		
		std::string loginport_str = "";
		std::string loginaddress_str = "";
		
		if(i==-1) {
			loginport_str = parser.getValue("loginserver.loginport");
			loginaddress_str = parser.getValue("loginserver.loginserver");
		}
		else {
			loginport_str = parser.getValue("loginserver.loginport" + std::to_string(i));
			loginaddress_str = parser.getValue("loginserver.loginserver" + std::to_string(i));
		}
		
		if(loginport_str.size() < 1 || loginaddress_str.size() < 1)
			continue;
		
		parser.convertStringToUnsignedShort(loginport_str, loginport[i+1]);
		snprintf(loginaddress[i+1], sizeof(loginaddress[i+1]), "%s", loginaddress_str.c_str());
		LogWrite(INIT__INFO, 0, "Init", "Login Server %s:%u...", loginaddress[i+1], loginport[i+1]);
	}
	
	if(!loginaddress[0][0]) {
		LogWrite(INIT__ERROR, 0, "Init", "loginserver.loginserver was missing..");
		return false;
	}
	
	web_worldaddress = parser.getValue("worldserver.webaddress");
	web_certfile = parser.getValue("worldserver.webcertfile");
	web_keyfile = parser.getValue("worldserver.webkeyfile");
	web_keypassword = parser.getValue("worldserver.webkeypassword");
	web_hardcodeuser = parser.getValue("worldserver.webhardcodeuser");
	web_hardcodepassword = parser.getValue("worldserver.webhardcodepassword");
	
	std::string webloginport_str = parser.getValue("worldserver.webport");
    parser.convertStringToUnsignedShort(webloginport_str, web_worldport);
	
	std::string defaultstatus_str = parser.getValue("worldserver.defaultstatus");
    parser.convertStringToUnsignedChar(defaultstatus_str, DEFAULTSTATUS);
	
	LogWrite(INIT__DEBUG, 0, "Init", "%s read...", MAIN_CONFIG_FILE);
	LoginServerInfo=1;
	return true;
}


char* NetConnection::GetLoginInfo(int16* oPort) {
	if (oPort == 0)
		return 0;
	if (loginaddress[0][0] == 0)
		return 0;

	int8 tmp[4] = { 0, 0, 0 };
	int8 count = 0;

	for (int i=0; i<4; i++) {
		if (loginaddress[i][0])
			tmp[count++] = i;
	}

	int x = rand() % count;

	*oPort = loginport[tmp[x]];
	return loginaddress[tmp[x]];
}

void UpdateWindowTitle(char* iNewTitle) {

	char tmp[500];
	if (iNewTitle) {
		snprintf(tmp, sizeof(tmp), "World: %s", iNewTitle);
	}
	else {
		string servername = net.GetWorldName();
		snprintf(tmp, sizeof(tmp), "%s (%s), Version: %s: %i Clients(s) in %i Zones(s)", EQ2EMU_MODULE, servername.c_str(), CURRENT_VERSION, numclients, numzones);
	}
	// Zero terminate ([max - 1] = 0) the string to prevent a warning 
	tmp[499] = 0;
	#ifdef WIN32
		SetConsoleTitle(tmp);
	#else
		printf("%c]0;%s%c", '\033', tmp, '\007');
	#endif
}

ZoneAuthRequest::ZoneAuthRequest(int32 account_id, char* name, int32 access_key) {
accountid = account_id;
character_name = string(name);
accesskey = access_key;
timestamp = Timer::GetUnixTimeStamp();
firstlogin = false;
}

ZoneAuthRequest::~ZoneAuthRequest ( )
{
}

void ZoneAuth::AddAuth(ZoneAuthRequest *zar) {
	LogWrite(NET__DEBUG, 0, "Net", "AddAuth: %u Key: %u", zar->GetAccountID(), zar->GetAccessKey());
	list.Insert(zar);
}

ZoneAuthRequest* ZoneAuth::GetAuth(int32 account_id, int32 access_key) {
	LinkedListIterator<ZoneAuthRequest*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetAccountID() == account_id && iterator.GetData()->GetAccessKey() == access_key) {
			ZoneAuthRequest* tmp = iterator.GetData();
			return tmp;
		}
		iterator.Advance();
	}
	return 0;

}

void ZoneAuth::PurgeInactiveAuth() {
	LinkedListIterator<ZoneAuthRequest*> iterator(list);

	iterator.Reset();
	int32 current_time = Timer::GetUnixTimeStamp();
	while(iterator.MoreElements()) {
		if ((iterator.GetData()->GetTimeStamp()+60) < current_time) {
			iterator.RemoveCurrent();
		}
		iterator.Advance();
	}
}

void ZoneAuth::RemoveAuth(ZoneAuthRequest *zar) {
	LinkedListIterator<ZoneAuthRequest*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData() == zar) {
			iterator.RemoveCurrent();
			break;
		}
		iterator.Advance();
	}
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
	printf("\n\nCopyright (C) 2007-2022 EQ2Emulator. https://www.eq2emu.com \n\n");
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

#ifdef DISCORD
ThreadReturnType StartDiscord(void* tmp)
{
#ifndef DISCORD
	THREAD_RETURN(NULL);
#endif
	if (tmp == 0) {
		ThrowError("StartDiscord: tmp = 0!");
		THREAD_RETURN(NULL);
	}

#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#endif

	bool enablediscord = rule_manager.GetGlobalRule(R_Discord, DiscordEnabled)->GetBool();
	
	if(enablediscord == false) {
		LogWrite(INIT__INFO, 0,"Discord","Bot Disabled By Rule...");
		THREAD_RETURN(NULL);
	}

	LogWrite(INIT__INFO, 0, "Discord", "Starting Discord Bridge...");
	const char* bottoken = rule_manager.GetGlobalRule(R_Discord, DiscordBotToken)->GetString();

	if(strlen(bottoken)== 0) {
		LogWrite(INIT__INFO, 0,"Discord","Bot Token Was Empty...");
		THREAD_RETURN(NULL);
	}

	dpp::cluster bot(bottoken, dpp::i_default_intents | dpp::i_message_content);
	
	//if we have debug on, go ahead and show DPP logs.
	#ifdef DEBUG
		bot.on_log([&bot](const dpp::log_t & event) {
		std::cout << "[" << dpp::utility::loglevel(event.severity) << "] " << event.message << "\n";
		});
	#endif
	
	bot.on_message_create([&bot](const dpp::message_create_t& event) {
		if (event.msg.author.is_bot() == false) {
			std::string chanid  = event.msg.channel_id.str();
			std::string listenchan = rule_manager.GetGlobalRule(R_Discord, DiscordListenChan)->GetString();

			if(chanid.compare(listenchan) != 0 || !chanid.size() || !listenchan.size()) {
				return;
			}
			 chat.TellChannel(NULL, listenchan.c_str(), event.msg.content.c_str(), event.msg.author.username.c_str());
		}
	});

	while(true) {
		bot.start(dpp::st_wait);
		//wait 30s for reconnect. prevents hammering discord and a potential ban.
		std::this_thread::sleep_for(std::chrono::milliseconds(30000));
	}
	
	THREAD_RETURN(NULL);
}
#endif