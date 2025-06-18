/*  
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2005 - 2025  EQ2EMulator Development Team (http://www.eq2emu.com formerly http://www.eq2emulator.net)

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
#include <iostream>
using namespace std;
#include <string.h>
#include "../common/misc.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <regex>
#include <unordered_set>
#include "Commands/Commands.h"
#include "Zone/pathfinder_interface.h"
#include "NPC_AI.h"

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib,"imagehlp.lib")
#else
#include <sys/socket.h>
#include <sys/stat.h>
#ifdef FREEBSD //Timothy Whitman - January 7, 2003
#include <sys/types.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>

#include "../common/unix.h"

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

extern int errno;
#endif

#include "../common/servertalk.h"
#include "../common/packet_dump.h"
#include "WorldDatabase.h"
#include "races.h"
#include "classes.h"
#include "../common/seperator.h"
#include "../common/EQStream.h"
#include "../common/EQStreamFactory.h"
#include "../common/opcodemgr.h"
#include "zoneserver.h"
#include "client.h"
#include "LoginServer.h"
#include "World.h"
#include <string>
#include <assert.h>
#include "LuaInterface.h"
#include "Factions.h"
#include "VisualStates.h"
#include "ClientPacketFunctions.h"
#include "SpellProcess.h"
#include "../common/Log.h"
#include "Rules/Rules.h"
#include "Chat/Chat.h"
#include "Tradeskills/Tradeskills.h"
#include "RaceTypes/RaceTypes.h"
#include <algorithm>
#include <random>

#include "Bots/Bot.h"

#ifdef WIN32
#define strncasecmp	_strnicmp
#define strcasecmp  _stricmp
#endif

// int32 numplayers = 0;												// never used?
// extern bool GetZoneLongName(char* short_name, char** long_name);		// never used?
// extern bool holdzones;												// never used?
// extern volatile bool RunLoops;										// never used in the zone server?
// extern Classes classes;												// never used in the zone server?

#define NO_CATCH 1

extern WorldDatabase	database;
extern sint32			numzones;
extern ClientList		client_list;
extern LoginServer loginserver;
extern ZoneList zone_list;
extern World world;
extern ConfigReader configReader;
extern Commands commands;
extern LuaInterface* lua_interface;
extern MasterFactionList master_faction_list;
extern VisualStates visual_states;
extern RuleManager rule_manager;
extern Chat chat;
extern MasterRaceTypeList race_types_list;
extern MasterSpellList master_spell_list;		// temp - remove later
extern MasterSkillList master_skill_list;


int32 MinInstanceID = 1000;

// JA: Moved most default values to Rules and risky initializers to ZoneServer::Init() - 2012.12.07
ZoneServer::ZoneServer(const char* name) {
	incoming_clients = 0;
	default_zone_map = nullptr;
	
	MIncomingClients.SetName("ZoneServer::MIncomingClients");

	depop_zone = false;
	repop_zone = false;
	respawns_allowed = true;
	instanceID = 0;
	strcpy(zone_name, name);
	zoneID = 0;
	rain = 0;
	cityzone = false;
	always_loaded = false;
	locked = false;	// JA: implementing /zone lock|unlock commands
	pNumPlayers = 0;
	LoadingData = true;
	zoneShuttingDown = false;
	++numzones;
	revive_points = 0;
	zone_motd = "";
	finished_depop = true;
	initial_spawn_threads_active = 0;
	minimumStatus = 0;
	minimumLevel = 0;
	maximumLevel = 0;
	minimumVersion = 0;
	weather_current_severity = 0;
	weather_signaled = false;
	xp_mod = 0;
	isDusk = false;
	dusk_hour = 0;
	dusk_minute = 0;
	dawn_hour = 0;
	dawn_minute = 0;
	reloading_spellprocess = false;
	expansion_flag = 0;
	holiday_flag = 0;
	can_bind = 1;
	can_gate = 1;
	MMasterZoneLock = new CriticalSection(MUTEX_ATTRIBUTE_RECURSIVE);
	
	pathing = nullptr;
	strcpy(zonesky_file,"");
	
	reloading = true;
	spawnthread_active = false;
	movementMgr = nullptr;
	spellProcess = nullptr;
	tradeskillMgr = nullptr;
	watchdogTimestamp = Timer::GetCurrentTime2();

	MPendingSpawnRemoval.SetName("ZoneServer::MPendingSpawnRemoval");

	lifetime_client_count = 0;
	
	groupraidMinLevel = 0;
	groupraidMaxLevel = 0;
	groupraidAvgLevel = 0;
	groupraidFirstLevel = 0;
	
	is_initialized = false;
	isInstance = false;
	duplicated_zone = false;
	duplicated_id = 0;
}

typedef map <int32, bool> ChangedSpawnMapType;
ZoneServer::~ZoneServer() {
	zoneShuttingDown = true;  //ensure other threads shut down too
	//allow other threads to properly shut down
	if(is_initialized) {
		LogWrite(ZONE__INFO, 0, "Zone", "Initiating zone shutdown of '%s'", zone_name);
	}
	int32 disp_count = 0;
	int32 next_disp_count = 100;
	while (spawnthread_active || initial_spawn_threads_active > 0){
		bool disp = false;
		if ( disp_count == 0 ) {
			disp = true;
		}
		else if(disp_count >= next_disp_count) {
			disp_count = 0;
			disp = true;
		}
		
		disp_count++;
		if (spawnthread_active && disp)
			LogWrite(ZONE__DEBUG, 7, "Zone", "Zone shutdown waiting on spawn thread");
		if (initial_spawn_threads_active > 0 && disp)
			LogWrite(ZONE__DEBUG, 7, "Zone", "Zone shutdown waiting on initial spawn thread");
		Sleep(10);
	}
	
	MChangedSpawns.lock();
	changed_spawns.clear();
	MChangedSpawns.unlock();
	
	transport_spawns.clear();
	safe_delete(tradeskillMgr);
	MMasterZoneLock->lock();
	MMasterSpawnLock.writelock(__FUNCTION__, __LINE__);
	DeleteData(true);
	RemoveLocationProximities();
	RemoveLocationGrids();
	DeleteSpawns(true);
	
	DeleteGlobalSpawns();

	if(spellProcess)
		spellProcess->RemoveAllSpells();
	
	DeleteFlightPaths();

	MMasterSpawnLock.releasewritelock(__FUNCTION__, __LINE__);
	MMasterZoneLock->unlock();
	world.UpdateServerStatistic(STAT_SERVER_NUM_ACTIVE_ZONES, -1);

	// If lockout, public, tradeskill, or quest instance delete from db when zone shuts down
	if (InstanceType == SOLO_LOCKOUT_INSTANCE || InstanceType == GROUP_LOCKOUT_INSTANCE || InstanceType == RAID_LOCKOUT_INSTANCE ||
		InstanceType == PUBLIC_INSTANCE || InstanceType == TRADESKILL_INSTANCE || InstanceType == QUEST_INSTANCE) {
		LogWrite(INSTANCE__DEBUG, 0, "Instance",  "Non persistent instance shutting down, deleting instance");
		database.DeleteInstance(instanceID);
	}

	if (pathing != nullptr)
		delete pathing;

	if (movementMgr != nullptr)
		delete movementMgr;

	// moved to the bottom as we want spawns deleted first, this used to be above Spawn deletion which is a big no no
	safe_delete(spellProcess);


    MGridMaps.lock();
	std::map<int32, GridMap*>::iterator grids;
	for(grids = grid_maps.begin(); grids != grid_maps.end(); grids++) {
		GridMap* gm = grids->second;
		safe_delete(gm);
	}
	grid_maps.clear();
    MGridMaps.unlock();
	
	if(is_initialized) {
		LogWrite(ZONE__INFO, 0, "Zone", "Completed zone shutdown of '%s'", zone_name);
	}
	--numzones;
	UpdateWindowTitle(0);
	zone_list.Remove(this);
	zone_list.RemoveClientZoneReference(this);
	safe_delete(MMasterZoneLock);
}

void ZoneServer::IncrementIncomingClients() { 
	MIncomingClients.writelock(__FUNCTION__, __LINE__);
	incoming_clients++;
	LogWrite(ZONE__INFO, 0, "Zone", "Increment incoming clients of '%s' zoneid %u (instance id: %u).  Current incoming client count: %u", zone_name, zoneID, instanceID, incoming_clients);
	MIncomingClients.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::DecrementIncomingClients() { 
	MIncomingClients.writelock(__FUNCTION__, __LINE__);
	bool zeroed = false;
	if(incoming_clients)
		incoming_clients--;
	else
		zeroed = true;
	LogWrite(ZONE__INFO, 0, "Zone", "Decrement incoming clients of '%s' zoneid %u (instance id: %u).  Current incoming client count: %u (was client count previously zero: %u)", zone_name, zoneID, instanceID, incoming_clients, zeroed);
	MIncomingClients.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::Init()
{
	LogWrite(ZONE__INFO, 0, "Zone", "Loading new Zone '%s'", zone_name);
	zone_list.Add(this);
	
	is_initialized = true;

	spellProcess = new SpellProcess();
	tradeskillMgr = new TradeskillMgr();

	/* Dynamic Timers */
	regenTimer.Start(rule_manager.GetGlobalRule(R_Zone, RegenTimer)->GetInt32());
	client_save.Start(rule_manager.GetGlobalRule(R_Zone, ClientSaveTimer)->GetInt32());
	shutdownTimer.Disable();
	spawn_range.Start(rule_manager.GetGlobalRule(R_Zone, CheckAttackPlayer)->GetInt32());
	aggro_timer.Start(rule_manager.GetGlobalRule(R_Zone, CheckAttackNPC)->GetInt32());
	/* Weather stuff */
	InitWeather();

	/* Static Timers */
	// JA - haven't decided yet if these should remain hard-coded. Changing them could break EQ2Emu functionality
	spawn_check_add.Start(1000);
	spawn_check_remove.Start(30000);
	spawn_expire_timer.Start(10000);
	respawn_timer.Start(10000);
	// there was never a starter for these?
	widget_timer.Start(5000);

	tracking_timer.Start(5000);

	movement_timer.Start(100);
	location_prox_timer.Start(1000);
	location_grid_timer.Start(1000);

	charsheet_changes.Start(500);

	// Send game time packet every in game hour (180 sec)
	sync_game_time_timer.Start(180000);

	// Get the dusk and dawn time from the rule manager and store it in the correct variables
	sscanf (rule_manager.GetGlobalRule(R_World, DuskTime)->GetString(), "%d:%d", &dusk_hour, &dusk_minute);
	sscanf (rule_manager.GetGlobalRule(R_World, DawnTime)->GetString(), "%d:%d", &dawn_hour, &dawn_minute);

	spawn_update.Start(rule_manager.GetGlobalRule(R_Zone, SpawnUpdateTimer)->GetInt16());
	LogWrite(ZONE__DEBUG, 0, "Zone", "SpawnUpdateTimer: %ims", rule_manager.GetGlobalRule(R_Zone, SpawnUpdateTimer)->GetInt16());

	queue_updates.Start(rule_manager.GetGlobalRule(R_Zone, SpawnUpdateTimer)->GetInt16());
	LogWrite(ZONE__DEBUG, 0, "Zone", "QueueUpdateTimer(inherits SpawnUpdateTimer): %ims", rule_manager.GetGlobalRule(R_Zone, SpawnUpdateTimer)->GetInt16());

	spawn_delete_timer = rule_manager.GetGlobalRule(R_Zone, SpawnDeleteTimer)->GetInt32();
	LogWrite(ZONE__DEBUG, 0, "Zone", "SpawnDeleteTimer: %ums", spawn_delete_timer);

	LogWrite(ZONE__DEBUG, 0, "Zone", "Loading zone flight paths");
	database.LoadZoneFlightPaths(this);

	world.UpdateServerStatistic(STAT_SERVER_NUM_ACTIVE_ZONES, 1);
	UpdateWindowTitle(0);

	string zoneName(GetZoneFile());
			
	world.LoadRegionMaps(zoneName);
	
	world.LoadMaps(zoneName);
	
	pathing = IPathfinder::Load(zoneName);
	movementMgr = new MobMovementManager();

	if(GetInstanceID()) {
		PlayerHouse* ph = world.GetPlayerHouseByInstanceID(GetInstanceID());
		if(ph) {
			HouseZone* hz = world.GetHouseZone(ph->house_id);
			if(hz) {
				std::string desc = ph->player_name + "'s " + hz->name;
				SetZoneDescription((char*)desc.c_str());
			}
		}
	}
	
	MMasterSpawnLock.SetName("ZoneServer::MMasterSpawnLock");
	m_npc_faction_list.SetName("ZoneServer::npc_faction_list");
	m_enemy_faction_list.SetName("ZoneServer::enemy_faction_list");
	m_reverse_enemy_faction_list.SetName("ZoneServer::reverse_enemy_faction_list");
	MDeadSpawns.SetName("ZoneServer::dead_spawns");
	MTransportSpawns.SetName("ZoneServer::transport_spawns");
	MSpawnList.SetName("ZoneServer::spawn_list");
	MTransporters.SetName("ZoneServer::m_transportMaps");
	MSpawnGroupAssociation.SetName("ZoneServer::spawn_group_associations");
	MSpawnGroupLocations.SetName("ZoneServer::spawn_group_locations");
	MSpawnLocationGroups.SetName("ZoneServer::spawn_location_groups");
	MSpawnGroupChances.SetName("ZoneServer::spawn_group_chances");
	MTransportLocations.SetName("ZoneServer::transporter_locations");
	MSpawnLocationList.SetName("ZoneServer::spawn_location_list");
	MSpawnDeleteList.SetName("ZoneServer::spawn_delete_list");
	MSpawnScriptTimers.SetName("ZoneServer::spawn_script_timers");
	MRemoveSpawnScriptTimersList.SetName("ZoneServer::remove_spawn_script_timers_list");
	MClientList.SetName("ZoneServer::clients");
	MWidgetTimers.SetName("ZoneServer::widget_timers");
#ifdef WIN32
	_beginthread(ZoneLoop, 0, this);
	_beginthread(SpawnLoop, 0, this);
#else
	pthread_create(&ZoneThread, NULL, ZoneLoop, this);
	pthread_detach(ZoneThread);
	pthread_create(&SpawnThread, NULL, SpawnLoop, this);
	pthread_detach(SpawnThread);
#endif
}

void ZoneServer::CancelThreads() {
#ifdef WIN32
	LogWrite(WORLD__ERROR, 1, "World", "Zone %s is hung, however CancelThreads is unsupported for WIN32.", GetZoneName());
#else
	pthread_cancel(ZoneThread);
	pthread_cancel(SpawnThread);
#endif
}

void ZoneServer::InitWeather()
{
	weather_enabled = rule_manager.GetZoneRule(GetZoneID(), R_Zone, WeatherEnabled)->GetBool();
	if( weather_enabled && isWeatherAllowed())
	{
		string tmp;
		// set up weather system when zone starts up
		weather_type				= rule_manager.GetZoneRule(GetZoneID(), R_Zone, WeatherType)->GetInt8();
		switch(weather_type)
		{
		case 3: tmp = "Chaotic"; break;
		case 2: tmp = "Random"; break;
		case 1: tmp = "Dynamic"; break;
		default: tmp = "Normal"; break;
		}
		LogWrite(ZONE__DEBUG, 0, "Zone", "%s: Setting up '%s' weather", zone_name, tmp.c_str());

		weather_frequency			= rule_manager.GetZoneRule(GetZoneID(), R_Zone, WeatherChangeFrequency)->GetInt32();
		LogWrite(ZONE__DEBUG, 1, "Zone", "%s: Change weather every %u seconds", zone_name, weather_frequency);

		weather_change_chance		= rule_manager.GetZoneRule(GetZoneID(), R_Zone, WeatherChangeChance)->GetInt8();
		LogWrite(ZONE__DEBUG, 1, "Zone", "%s: Chance of weather change: %i%%", zone_name, weather_change_chance);

		weather_min_severity		= rule_manager.GetZoneRule(GetZoneID(), R_Zone, MinWeatherSeverity)->GetFloat();
		weather_max_severity		= rule_manager.GetZoneRule(GetZoneID(), R_Zone, MaxWeatherSeverity)->GetFloat();
		LogWrite(ZONE__DEBUG, 1, "Zone", "%s: Weather Severity min/max is %.2f - %.2f", zone_name, weather_min_severity, weather_max_severity);
		// Allow a random roll to determine if weather should start out severe or calm
		if( MakeRandomInt(1, 100) > 50)
		{
			weather_pattern				= 1; // default weather to increase in severity initially
			weather_current_severity	= weather_min_severity;
		}
		else
		{
			weather_pattern				= 0; // default weather to decrease in severity initially
			weather_current_severity	= weather_max_severity;
		}
		LogWrite(ZONE__DEBUG, 1, "Zone", "%s: Weather Severity set to %.2f, pattern: %i", zone_name, weather_current_severity, weather_pattern);

		weather_change_amount		= rule_manager.GetZoneRule(GetZoneID(), R_Zone, WeatherChangePerInterval)->GetFloat();
		LogWrite(ZONE__DEBUG, 1, "Zone", "%s: Weather change by %.2f each interval", zone_name, weather_change_amount);

		if( weather_type > 0 )
		{
			weather_dynamic_offset		= rule_manager.GetZoneRule(GetZoneID(), R_Zone, WeatherDynamicMaxOffset)->GetFloat();
			LogWrite(ZONE__DEBUG, 1, "Zone", "%s: Weather Max Offset changes no more than %.2f each interval", zone_name, weather_dynamic_offset);
		}
		else
			weather_dynamic_offset = 0;

		SetRain(weather_current_severity);
		weather_last_changed_time = Timer::GetUnixTimeStamp();
		weatherTimer.Start(rule_manager.GetZoneRule(GetZoneID(), R_Zone, WeatherTimer)->GetInt32());
	}
}
void ZoneServer::DeleteSpellProcess(){
	//Just get a lock to make sure we aren't already looping the spawnprocess or clientprocess if this is different than the calling thread
	MMasterSpawnLock.writelock(__FUNCTION__, __LINE__);
	MMasterZoneLock->lock();
	reloading_spellprocess = true;
	// Remove spells from NPC's
	Spawn* spawn = 0;
	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if(spawn && spawn->IsNPC())
			((NPC*)spawn)->SetSpells(0);
		
		if(spawn->IsEntity()) {
			((Entity*)spawn)->RemoveSpellBonus(nullptr, true);
			((Entity*)spawn)->DeleteSpellEffects(true);
		}
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	MMasterZoneLock->unlock();
	MMasterSpawnLock.releasewritelock(__FUNCTION__, __LINE__);
	
	DismissAllPets();
	spellProcess->RemoveAllSpells(true);
	safe_delete(spellProcess);
}

void ZoneServer::LoadSpellProcess(){
	spellProcess = new SpellProcess();
	reloading_spellprocess = false;

	// Reload NPC's spells
	Spawn* spawn = 0;
	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if(spawn && spawn->IsNPC())
			((NPC*)spawn)->SetSpells(world.GetNPCSpells(((NPC*)spawn)->GetPrimarySpellList(), ((NPC*)spawn)->GetSecondarySpellList()));
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::LockAllSpells(Player* player) {
	if (player && spellProcess) {
		Client* client = ((Player*)player)->GetClient();
		if (client)
			spellProcess->LockAllSpells(client);
	}
}
	
void ZoneServer::UnlockAllSpells(Player* player) {
	if (player && spellProcess) {
		Client* client = ((Player*)player)->GetClient();
		if (client)
			spellProcess->UnlockAllSpells(client);
	}
}

void ZoneServer::DeleteFactionLists() {
	map<int32, vector<int32> *>::iterator faction_itr;
	map<int32, vector<int32> *>::iterator spawn_itr;

	m_enemy_faction_list.writelock(__FUNCTION__, __LINE__);
	for (faction_itr = enemy_faction_list.begin(); faction_itr != enemy_faction_list.end(); faction_itr++)
		safe_delete(faction_itr->second);
	enemy_faction_list.clear();
	m_enemy_faction_list.releasewritelock(__FUNCTION__, __LINE__);

	m_reverse_enemy_faction_list.writelock(__FUNCTION__, __LINE__);
	for (faction_itr = reverse_enemy_faction_list.begin(); faction_itr != reverse_enemy_faction_list.end(); faction_itr++)
		safe_delete(faction_itr->second);
	reverse_enemy_faction_list.clear();
	m_reverse_enemy_faction_list.releasewritelock(__FUNCTION__, __LINE__);

	m_npc_faction_list.writelock(__FUNCTION__, __LINE__);
	for (spawn_itr = npc_faction_list.begin(); spawn_itr != npc_faction_list.end(); spawn_itr++)
		safe_delete(spawn_itr->second);
	npc_faction_list.clear();
	m_npc_faction_list.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::DeleteData(bool boot_clients){
	Spawn* spawn = 0;
	vector<Spawn*> tmp_player_list; // changed to a vector from a MutexList as this is a local variable and don't need mutex stuff for the list

	// Clear spawn groups
	spawn_group_map.clear();

	// Loop through the spawn list and set the spawn for deletion
	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if(spawn){
			if(!boot_clients && (spawn->IsPlayer() || spawn->IsBot()))
				tmp_player_list.push_back(spawn);
			else if(spawn->IsPlayer()){
				Client* client = ((Player*)spawn)->GetClient();
				if(client)
					client->Disconnect();
			}
			else{
				RemoveSpawnSupportFunctions(spawn, boot_clients, true);
				RemoveSpawnFromGrid(spawn, spawn->GetLocation());
				AddPendingDelete(spawn);
			}
		}
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);

	// Quick hack to prevent a deadlock, RemoveSpawnSupportFunctions() will cancel spells and result in zone->GetSpawnByID()
	// being called which read locks the spawn list and caused a dead lock as the above mutex's were write locked
	MSpawnList.writelock(__FUNCTION__, __LINE__);
	// Clear the spawn list, this was in the mutex above, moved it down so the above mutex could be a read lock
	spawn_list.clear();

	// Moved this up so we only read lock the list once in this list
	vector<Spawn*>::iterator spawn_iter2;
	for (spawn_iter2 = tmp_player_list.begin(); spawn_iter2 != tmp_player_list.end(); spawn_iter2++) {
		spawn_list[(*spawn_iter2)->GetID()] = (*spawn_iter2);
	}
	MSpawnList.releasewritelock(__FUNCTION__, __LINE__);

	// Clear player proximities
	RemovePlayerProximity(0, true);

	spawn_range_map.clear(true);
	if(boot_clients) {
		// Refactor
		vector<Client*>::iterator itr;

		MClientList.writelock(__FUNCTION__, __LINE__);
		for (itr = clients.begin(); itr != clients.end(); itr++)
			safe_delete(*itr);

		clients.clear();
		MClientList.releasewritelock(__FUNCTION__, __LINE__);
	}

	// Clear and delete spawn locations
	MSpawnLocationList.writelock(__FUNCTION__, __LINE__);
	map<int32, SpawnLocation*>::iterator spawn_location_iter;
	for (spawn_location_iter = spawn_location_list.begin(); spawn_location_iter != spawn_location_list.end(); spawn_location_iter++)
		safe_delete(spawn_location_iter->second);

	spawn_location_list.clear();
	MSpawnLocationList.releasewritelock(__FUNCTION__, __LINE__);

	// If we allow clients to stay in the zone we need to preserve the revive_points, otherwise if the player dies they will crash
	if(revive_points && boot_clients){
		vector<RevivePoint*>::iterator revive_iter;
		for(revive_iter=revive_points->begin(); revive_iter != revive_points->end(); revive_iter++){
			safe_delete(*revive_iter);
		}
		safe_delete(revive_points);
	}

	MSpawnGroupAssociation.writelock(__FUNCTION__, __LINE__);
	map<int32, set<int32>*>::iterator assoc_itr;
	for (assoc_itr = spawn_group_associations.begin(); assoc_itr != spawn_group_associations.end(); assoc_itr++)
		safe_delete(assoc_itr->second);

	spawn_group_associations.clear();
	MSpawnGroupAssociation.releasewritelock(__FUNCTION__, __LINE__);

	MSpawnGroupLocations.writelock(__FUNCTION__, __LINE__);
	map<int32, map<int32, int32>*>::iterator loc_itr;
	for (loc_itr = spawn_group_locations.begin(); loc_itr != spawn_group_locations.end(); loc_itr++)
		safe_delete(loc_itr->second);

	spawn_group_locations.clear();
	MSpawnGroupLocations.releasewritelock(__FUNCTION__, __LINE__);

	MSpawnLocationGroups.writelock(__FUNCTION__, __LINE__);
	map<int32, list<int32>*>::iterator group_itr;
	for (group_itr = spawn_location_groups.begin(); group_itr != spawn_location_groups.end(); group_itr++)
		safe_delete(group_itr->second);

	spawn_location_groups.clear();
	MSpawnLocationGroups.releasewritelock(__FUNCTION__, __LINE__);
	
	// Clear lists that need more then just a Clear()
	DeleteFactionLists();
	DeleteSpawnScriptTimers(0, true);
	DeleteSpawnScriptTimers();
	ClearDeadSpawns();

	// Clear lists
	movement_spawns.clear();
	respawn_timers.clear();
	transport_spawns.clear();
	quick_database_id_lookup.clear();

	MWidgetTimers.writelock(__FUNCTION__, __LINE__);
	widget_timers.clear();
	MWidgetTimers.releasewritelock(__FUNCTION__, __LINE__);

	map<int16, PacketStruct*>::iterator struct_itr;
	for (struct_itr = versioned_info_structs.begin(); struct_itr != versioned_info_structs.end(); struct_itr++)
		safe_delete(struct_itr->second);
	versioned_info_structs.clear();

	for (struct_itr = versioned_pos_structs.begin(); struct_itr != versioned_pos_structs.end(); struct_itr++)
		safe_delete(struct_itr->second);
	versioned_pos_structs.clear();

	for (struct_itr = versioned_vis_structs.begin(); struct_itr != versioned_vis_structs.end(); struct_itr++)
		safe_delete(struct_itr->second);
	versioned_vis_structs.clear();
}

void ZoneServer::RemoveLocationProximities() {
	MutexList<LocationProximity*>::iterator itr = location_proximities.begin();
	while(itr.Next()){
		safe_delete(itr->value);
	}
	location_proximities.clear();
}

RevivePoint* ZoneServer::GetRevivePoint(int32 id){
	vector<RevivePoint*>::iterator revive_iter;
	for(revive_iter=revive_points->begin(); revive_iter != revive_points->end(); revive_iter++){
		if((*revive_iter)->id == id)
			return *revive_iter;
	}
	return 0;
}

vector<RevivePoint*>* ZoneServer::GetRevivePoints(Client* client)
{
	vector<RevivePoint*>* points = new vector<RevivePoint*>;
	RevivePoint* closest_point = 0;

	// we should not check for revive points if this is null
	if ( revive_points != NULL )
	{
		LogWrite(ZONE__DEBUG, 0, "Zone", "Got revive point in %s!", __FUNCTION__);

		float closest = 100000;
		float test_closest = 0;
		RevivePoint* test_point = 0;
		vector<RevivePoint*>::iterator revive_iter;
		for(revive_iter=revive_points->begin(); revive_iter != revive_points->end(); revive_iter++)
		{
			test_point = *revive_iter;
			if(test_point)
			{
				test_closest = client->GetPlayer()->GetDistance(test_point->x, test_point->y, test_point->z);

				// should this be changed to list all revive points within max distance or just the closest
				if(test_closest < closest)
				{
					LogWrite(ZONE__DEBUG, 0, "Zone", "test_closest: %.2f, closest: %.2f", test_closest, closest);
					closest = test_closest;
					closest_point = test_point;
				}
				if(test_point->always_included ) {
					points->push_back(test_point);
					if(closest_point == test_point) {
						closest_point = nullptr;
						closest = 100000;
					}
				}
			}
		}
		if(closest_point) {
			points->push_back(closest_point);
		}
	}

	if(closest_point && points->size() == 0 && closest_point->zone_id == GetZoneID())
	{
		LogWrite(ZONE__WARNING, 0, "Zone", "Nearest Revive Point too far away. Add another!");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "The closest revive point is quite far away, you might want to ask the server admin for a closer one.");
		points->push_back(closest_point);
	}
	else if(points->size() == 0)
	{
		LogWrite(ZONE__WARNING, 0, "Zone", "No Revive Points set for zoneID %u. Add some!", GetZoneID());
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "There are no revive points for this zone, you might want to ask the server admin for one.");
		closest_point = new RevivePoint;
		closest_point->heading = GetSafeHeading();
		closest_point->id = 0xFFFFFFFF;
		closest_point->location_name = "Zone Safe Point";
		closest_point->zone_id = GetZoneID();
		closest_point->x = GetSafeX();
		closest_point->y = GetSafeY();
		closest_point->z = GetSafeZ();
		closest_point->always_included = true;
		points->push_back(closest_point);
	}
	return points;
}

void ZoneServer::TriggerCharSheetTimer(){
	charsheet_changes.Trigger();
}

void ZoneServer::RegenUpdate(){
	if(damaged_spawns.size(true) == 0)
		return;

	Spawn* spawn = 0;
	MutexList<int32>::iterator spawn_iter = damaged_spawns.begin();
	while(spawn_iter.Next()){	
		spawn = GetSpawnByID(spawn_iter->value);
		if(spawn && (((spawn->GetHP() < spawn->GetTotalHP()) && spawn->GetHP()>0) ||  (spawn->GetPower() < spawn->GetTotalPower()))){
			if(spawn->IsEntity())
				((Entity*)spawn)->DoRegenUpdate();
			if(spawn->IsPlayer()){
				Client* client = ((Player*)spawn)->GetClient();
				if(client && client->IsReadyForUpdates())
					client->QueuePacket(client->GetPlayer()->GetPlayerInfo()->serialize(client->GetVersion()));
			}
		}
		else
			RemoveDamagedSpawn(spawn);
		//Spawn no longer valid, remove it from the list
		if (!spawn)
			damaged_spawns.Remove(spawn_iter->value);
	}
}

void ZoneServer::ClearDeadSpawns(){
	MDeadSpawns.writelock(__FUNCTION__, __LINE__);
	dead_spawns.clear();
	MDeadSpawns.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::ProcessDepop(bool respawns_allowed, bool repop) {
	vector<Client*>::iterator client_itr;
	Client* client = 0;
	Spawn* spawn = 0;
	PacketStruct* packet = 0;
	int16 packet_version = 0;
	spawn_expire_timers.clear();

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(!client)
			continue;
		client->GetPlayer()->SetTarget(0);
		client->SetMailTransaction(0);
		if(repop)
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Zone Repop in progress...");
		else{
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Zone Depop in progress...");
			if(respawns_allowed)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Spawns will respawn according to their respawn timers.");
		}
		if(!packet || packet_version != client->GetVersion()){
			safe_delete(packet);
			packet_version = client->GetVersion();
			packet = configReader.getStruct("WS_DestroyGhostCmd", packet_version);
		}
		map<int32, Spawn*>::iterator itr;
		MSpawnList.readlock(__FUNCTION__, __LINE__);
		for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
			spawn = itr->second;
			if(spawn && !spawn->IsPlayer() && !spawn->IsBot()){
				bool dispatched = false;
				if(spawn->IsPet())
				{
					Entity* owner = ((Entity*)spawn)->GetOwner();
					if(owner)
					{
						owner->DismissPet((Entity*)spawn);
						dispatched = true;
					}
				}
				
				spawn->SetDeletedSpawn(true);
				
				if(!dispatched)
					SendRemoveSpawn(client, spawn, packet);
			}
		}
		MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);

	DeleteTransporters();
	safe_delete(packet);	
	if(!repop && respawns_allowed){
		spawn_range_map.clear(true);
		MutexList<Spawn*> tmp_player_list; // Local variable, never be on another thread so probably don't need the extra mutex code that comes with a MutexList
		ClearDeadSpawns();

		map<int32, Spawn*>::iterator itr;
		MSpawnList.writelock(__FUNCTION__, __LINE__);
		for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
			spawn = itr->second;
			if (spawn) {
				if(spawn->GetRespawnTime() > 0 && spawn->GetSpawnLocationID() > 0)
					respawn_timers.Put(spawn->GetSpawnLocationID(), Timer::GetCurrentTime2() + spawn->GetRespawnTime()*1000);
				if(spawn->IsPlayer() || spawn->IsBot())
					tmp_player_list.Add(spawn);
				else {
				RemoveSpawnSupportFunctions(spawn, true);
				RemoveSpawnFromGrid(spawn, spawn->GetLocation());
					AddPendingDelete(spawn);
				}
			}
		}
		spawn_list.clear();
		//add back just the clients
		MutexList<Spawn*>::iterator spawn_iter2 = tmp_player_list.begin();
		while(spawn_iter2.Next()) {
			spawn_list[spawn_iter2->value->GetID()] = spawn_iter2->value;
		}
		MSpawnList.releasewritelock(__FUNCTION__, __LINE__);
	}
	else {
		DeleteData(false);
	}

	if(repop)
	{
		// reload spirit shards for the current zone
		database.LoadSpiritShards(this);

		LoadingData = true;
	}
}

void ZoneServer::Depop(bool respawns, bool repop) {
	respawns_allowed = respawns;
	repop_zone = repop;
	finished_depop = false;
	depop_zone = true;
}

bool ZoneServer::AddCloseSpawnsToSpawnGroup(Spawn* spawn, float radius){
	if(!spawn)
		return false;

	Spawn* close_spawn = 0;
	bool ret = true;
	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		close_spawn = itr->second;
		if(close_spawn && close_spawn != spawn && !close_spawn->IsPlayer() && close_spawn->GetDistance(spawn) <= radius){
			if((spawn->IsNPC() && close_spawn->IsNPC()) || (spawn->IsGroundSpawn() && close_spawn->IsGroundSpawn()) || (spawn->IsObject() && close_spawn->IsObject()) || (spawn->IsWidget() && close_spawn->IsWidget()) || (spawn->IsSign() && close_spawn->IsSign())){
				if(close_spawn->GetSpawnGroupID() == 0){
					spawn->AddSpawnToGroup(close_spawn);
					close_spawn->AddSpawnToGroup(spawn);
				}
				else
					ret = false;
			}
		} 
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

void ZoneServer::RepopSpawns(Client* client, Spawn* in_spawn){
	vector<Spawn*>* spawns = in_spawn->GetSpawnGroup();
	PacketStruct* packet = configReader.getStruct("WS_DestroyGhostCmd", client->GetVersion());
	if(spawns){
		if(!packet)
			return;

		Spawn* spawn = 0;
		vector<Spawn*>::iterator itr;
		for(itr = spawns->begin(); itr != spawns->end(); itr++){
			spawn = *itr;
			spawn->SetDeletedSpawn(true);
			SendRemoveSpawn(client, spawn, packet);
		}
	}
	safe_delete(spawns);
	if(in_spawn)
		in_spawn->SetDeletedSpawn(true);

	SendRemoveSpawn(client, in_spawn, packet);
	spawn_check_add.Trigger();
	safe_delete(packet);
}

bool ZoneServer::AggroVictim(NPC* npc, Spawn* victim, Client* client)
{
	bool isEntity = victim->IsEntity();
	if(isEntity && !npc->AttackAllowed((Entity*)victim))
		return false;
	
	victim->CheckEncounterState((Entity*)npc, true);
	
	if (npc->HasSpawnGroup()) {
		vector<Spawn*>* groupVec = npc->GetSpawnGroup();
		for (int32 i = 0; i < groupVec->size(); i++) {
			Spawn* group_member = groupVec->at(i);
			if (group_member && !group_member->EngagedInCombat() && group_member->Alive()) {
				CallSpawnScript(group_member, SPAWN_SCRIPT_AGGRO, victim);
				if (isEntity)
					((NPC*)group_member)->AddHate((Entity*)victim, 50);
				else
					((NPC*)group_member)->InCombat(true);
			}
		}
		safe_delete(groupVec);
	}
	else
	{
		if (isEntity)
		{
			CallSpawnScript(victim, SPAWN_SCRIPT_AGGRO, victim);
			npc->AddHate((Entity*)victim, 50);
		}
		else
			npc->InCombat(true);
	}
	return true;
}

bool ZoneServer::CheckNPCAttacks(NPC* npc, Spawn* victim, Client* client){
	if(!npc || !victim)
		return true;

	if (client) {
		int8 arrow = 0;
		if (client->IsReadyForUpdates() && npc->CanSeeInvis(client->GetPlayer()) && client->GetPlayer()->GetFactions()->ShouldAttack(npc->GetFactionID()) && npc->AttackAllowed((Entity*)victim, false)) {
			if (!npc->EngagedInCombat()) {
				if(client->GetPlayer()->GetArrowColor(npc->GetLevel()) != ARROW_COLOR_GRAY) {
					AggroVictim(npc, victim, client);
				}
				else if(npc->IsScaredByStrongPlayers() &&
						!client->GetPlayer()->IsSpawnInRangeList(npc->GetID())) {
					SendSpawnChanges(npc, client, true, true);
					client->GetPlayer()->SetSpawnInRangeList(npc->GetID(), true);
				}
			}
		}
	}
	else{
		AggroVictim(npc, victim, client);
	}
	return true;
}

bool ZoneServer::CheckEnemyList(NPC* npc) {
	vector<int32> *factions;
	vector<int32>::iterator faction_itr;
	vector<int32> *spawns;
	vector<int32>::iterator spawn_itr;
	map<float, Spawn*> attack_spawns;
	map<float, Spawn*> reverse_attack_spawns;	
	map<float, Spawn*>::iterator itr;
	int32 faction_id = npc->GetFactionID();
	float distance;

	if (faction_id == 0)
		return true;

	m_enemy_faction_list.readlock(__FUNCTION__, __LINE__);
	if (enemy_faction_list.count(faction_id) > 0) {
		factions = enemy_faction_list[faction_id];

		for (faction_itr = factions->begin(); faction_itr != factions->end(); faction_itr++) {
			m_npc_faction_list.readlock(__FUNCTION__, __LINE__);
			if (npc_faction_list.count(*faction_itr) > 0) {
				spawns = npc_faction_list[*faction_itr];
				spawn_itr = spawns->begin();

				for (spawn_itr = spawns->begin(); spawn_itr != spawns->end(); spawn_itr++) {
					Spawn* spawn = GetSpawnByID(*spawn_itr);
					if (spawn) {
						if ((!npc->IsPrivateSpawn() || npc->AllowedAccess(spawn)) && (distance = spawn->GetDistance(npc)) <= npc->GetAggroRadius() && npc->CheckLoS(spawn))
							attack_spawns[distance] = spawn;
					}
				}
			}
			m_npc_faction_list.releasereadlock(__FUNCTION__, __LINE__);
		}
	}
	m_enemy_faction_list.releasereadlock(__FUNCTION__, __LINE__);

	m_reverse_enemy_faction_list.readlock(__FUNCTION__, __LINE__);
	if (reverse_enemy_faction_list.count(faction_id) > 0) {
		factions = reverse_enemy_faction_list[faction_id];

		for (faction_itr = factions->begin(); faction_itr != factions->end(); faction_itr++) {
			m_npc_faction_list.readlock(__FUNCTION__, __LINE__);
			if (npc_faction_list.count(*faction_itr) > 0) {
				spawns = npc_faction_list[*faction_itr];
				spawn_itr = spawns->begin();

				for (spawn_itr = spawns->begin(); spawn_itr != spawns->end(); spawn_itr++) {
					Spawn* spawn = GetSpawnByID(*spawn_itr);
					if (spawn) {
						if ((!npc->IsPrivateSpawn() || npc->AllowedAccess(spawn)) && (distance = spawn->GetDistance(npc)) <= npc->GetAggroRadius() && npc->CheckLoS(spawn))
							reverse_attack_spawns[distance] = spawn;
					}
				}
			}
			m_npc_faction_list.releasereadlock(__FUNCTION__, __LINE__);
		}
	}
	m_reverse_enemy_faction_list.releasereadlock(__FUNCTION__, __LINE__);

	if (attack_spawns.size() > 0) {
		for (itr = attack_spawns.begin(); itr != attack_spawns.end(); itr++)
			CheckNPCAttacks(npc, itr->second);
	}
	if (reverse_attack_spawns.size() > 0) {
		for (itr = reverse_attack_spawns.begin(); itr != reverse_attack_spawns.end(); itr++)
			CheckNPCAttacks((NPC*)itr->second, npc);
	}

	return attack_spawns.size() == 0;
}

void ZoneServer::RemoveDeadEnemyList(Spawn *spawn) 
{
	int32 faction_id = spawn->GetFactionID();
	vector<int32> *spawns;
	vector<int32>::iterator itr;

	m_npc_faction_list.writelock(__FUNCTION__, __LINE__);
	if (npc_faction_list.count(faction_id) > 0) {
		spawns = npc_faction_list[faction_id];

		for (itr = spawns->begin(); itr != spawns->end(); itr++) {
			if (*itr == spawn->GetID()) {
				spawns->erase(itr);
				break;
			}
		}
	}
	m_npc_faction_list.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::AddEnemyList(NPC* npc){
	int32 faction_id = npc->GetFactionID();
	vector<int32> *hostile_factions;
	vector<int32>::iterator itr;

	if(faction_id <= 9)
		return;

	if(!rule_manager.GetZoneRule(GetZoneID(), R_Faction, AllowFactionBasedCombat)->GetBool()) {
		LogWrite(FACTION__WARNING, 0, "Faction", "Faction Combat is DISABLED via R_Faction::AllowFactionBasedCombat rule!");
		return;
	}
	
	m_npc_faction_list.readlock(__FUNCTION__, __LINE__);
	if (npc_faction_list.count(faction_id) == 0) {
		if(faction_id > 10) {
			if ((hostile_factions = master_faction_list.GetHostileFactions(faction_id)) != NULL) {
				itr = hostile_factions->begin();

				for (itr = hostile_factions->begin(); itr != hostile_factions->end(); itr++) {
					m_enemy_faction_list.writelock(__FUNCTION__, __LINE__);
					if (enemy_faction_list.count(faction_id) == 0)
						enemy_faction_list[faction_id] = new vector<int32>;
					enemy_faction_list[faction_id]->push_back(*itr);
					m_enemy_faction_list.releasewritelock(__FUNCTION__, __LINE__);

					m_reverse_enemy_faction_list.writelock(__FUNCTION__, __LINE__);
					if(reverse_enemy_faction_list.count(*itr) == 0)
						reverse_enemy_faction_list[*itr] = new vector<int32>;
					reverse_enemy_faction_list[*itr]->push_back(faction_id);
					m_reverse_enemy_faction_list.releasewritelock(__FUNCTION__, __LINE__);
				}
			}
		}

		/*m_enemy_faction_list.writelock(__FUNCTION__, __LINE__);
		if(enemy_faction_list.count(1) == 0)
			enemy_faction_list[1] = new vector<int32>;
		enemy_faction_list[1]->push_back(faction_id);
		m_enemy_faction_list.releasewritelock(__FUNCTION__, __LINE__);*/
	}
	m_npc_faction_list.releasereadlock(__FUNCTION__, __LINE__);

	m_npc_faction_list.writelock(__FUNCTION__, __LINE__);
	if(npc_faction_list.count(faction_id) == 0)
		npc_faction_list[faction_id] = new vector<int32>;
	npc_faction_list[faction_id]->push_back(npc->GetID());
	m_npc_faction_list.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::CheckSpawnRange(Client* client, Spawn* spawn, bool initial_login){
	if(client && spawn && (initial_login || client->IsConnected())) {
		if(spawn != client->GetPlayer()) {
			if(spawn_range_map.count(client) == 0)
				spawn_range_map.Put(client, new MutexMap<int32, float >());
			float curDist = spawn->GetDistance(client->GetPlayer());

			int32 ghost_spawn_id = client->GetPlayerPOVGhostSpawnID();
			Spawn* otherSpawn = GetSpawnByID(ghost_spawn_id);
			
			if (!client->GetPlayer()->WasSentSpawn(spawn->GetID()) 
				&& (!otherSpawn || otherSpawn->GetDistance(spawn) > SEND_SPAWN_DISTANCE) && curDist > SEND_SPAWN_DISTANCE)
			{
				return;
			}

			spawn_range_map.Get(client)->Put(spawn->GetID(), curDist);

			if(!initial_login && client && spawn->IsNPC() && (!spawn->IsPrivateSpawn() || spawn->AllowedAccess(client->GetPlayer())) 
					&& curDist <= ((NPC*)spawn)->GetAggroRadius() && !client->GetPlayer()->GetInvulnerable())
				CheckNPCAttacks((NPC*)spawn, client->GetPlayer(), client);
		} 

		if(!initial_login)
			CheckPlayerProximity(spawn, client);
	}
}

void ZoneServer::CheckSpawnRange(Spawn* spawn){
	vector<Client*>::iterator client_itr;
	Client* client = 0;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(client && client->IsReadyForSpawns())
			CheckSpawnRange(client, spawn);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

bool ZoneServer::PrepareSpawnID(Player* player, Spawn* spawn){
	return player->SetSpawnMap(spawn);
}

void ZoneServer::CheckSendSpawnToClient(Client* client, bool initial_login) {
	if (!client) {
		LogWrite(ZONE__ERROR, 0, "Zone", "CheckSendSpawnToClient called with an invalid client");
		return;
	}

	if (!initial_login && !client->GetInitialSpawnsSent() || (!initial_login && !client->IsReadyForSpawns()))
		return;

	Spawn* spawn = 0;
	map<float, vector<Spawn*>* > closest_spawns;
	if (spawn_range_map.count(client) > 0) {
		if (initial_login || client->IsConnected()) {
			MutexMap<int32, float >::iterator spawn_iter = spawn_range_map.Get(client)->begin();
			while (spawn_iter.Next()) {
				spawn = GetSpawnByID(spawn_iter->first, true);
				if (spawn && spawn->GetPrivateQuestSpawn()) {
					if (!spawn->IsPrivateSpawn())
						spawn->AddAllowAccessSpawn(spawn);
					if (spawn->MeetsSpawnAccessRequirements(client->GetPlayer())) {
						if (spawn->IsPrivateSpawn() && !spawn->AllowedAccess(client->GetPlayer()))
							spawn->AddAllowAccessSpawn(client->GetPlayer());
					}
					else if (spawn->AllowedAccess(client->GetPlayer()))
						spawn->RemoveSpawnAccess(client->GetPlayer());
				}
				if (spawn && spawn != client->GetPlayer() && client->GetPlayer()->ShouldSendSpawn(spawn)) {
					if ((!initial_login && spawn_iter->second <= SEND_SPAWN_DISTANCE) || (initial_login && (spawn_iter->second <= (SEND_SPAWN_DISTANCE / 2) || spawn->IsWidget()))) {
						if(PrepareSpawnID(client->GetPlayer(), spawn)) {
							if (closest_spawns.count(spawn_iter->second) == 0)
								closest_spawns[spawn_iter->second] = new vector<Spawn*>();
							closest_spawns[spawn_iter->second]->push_back(spawn);
						}
					}
				}
			}
		}
		vector<Spawn*>::iterator spawn_iter2;
		map<float, vector<Spawn*>* >::iterator itr;
		for (itr = closest_spawns.begin(); itr != closest_spawns.end(); ) {
			for (spawn_iter2 = itr->second->begin(); spawn_iter2 != itr->second->end(); spawn_iter2++) {
				spawn = *spawn_iter2;

				if(!client->IsReloadingZone() || (client->IsReloadingZone() && spawn != client->GetPlayer()))
					SendSpawn(spawn, client);
				
				if (client->ShouldTarget() && client->GetCombineSpawn() == spawn)
					client->TargetSpawn(spawn);
			}
			vector<Spawn*>* vect = itr->second;
			map<float, vector<Spawn*>* >::iterator tmpitr = itr;
			itr++;
			closest_spawns.erase(tmpitr);
			safe_delete(vect);
		}
	}

	if (initial_login)
		client->SetInitialSpawnsSent(true);
}

void ZoneServer::CheckSendSpawnToClient(){
	vector<Client*>::iterator itr;
	Client* client = 0;

	MClientList.readlock(__FUNCTION__, __LINE__);
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		client = *itr;
		if(client->IsReadyForSpawns())
			CheckSendSpawnToClient(client);
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::CheckRemoveSpawnFromClient(Spawn* spawn) {
	vector<Client*>::iterator itr;
	Client* client = 0;
	PacketStruct* packet = 0;
	int16 packet_version = 0;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		client = *itr;
		if(client){
			int32 ghost_spawn_id = client->GetPlayerPOVGhostSpawnID();
			Spawn* otherSpawn = GetSpawnByID(ghost_spawn_id);
			if(!packet || packet_version != client->GetVersion()){
				safe_delete(packet);
				packet_version = client->GetVersion();
				packet = configReader.getStruct("WS_DestroyGhostCmd", packet_version);
			}

			if(spawn && spawn != client->GetPlayer() && 
				client->GetPlayer()->WasSentSpawn(spawn->GetID()) && 
				!client->GetPlayer()->IsRemovingSpawn(spawn->GetID()) && 
				client->GetPlayer()->WasSpawnRemoved(spawn) == false && 
				(ghost_spawn_id == 0 || (ghost_spawn_id != spawn->GetID() && otherSpawn && otherSpawn->GetDistance(spawn) > REMOVE_SPAWN_DISTANCE)) &&
				(spawn_range_map.Get(client)->Get(spawn->GetID()) > REMOVE_SPAWN_DISTANCE &&
					!spawn->IsSign() && !spawn->IsObject() && !spawn->IsWidget() && !spawn->IsTransportSpawn())){
				SendRemoveSpawn(client, spawn, packet);
				spawn_range_map.Get(client)->erase(spawn->GetID());
			}

		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
	safe_delete(packet);
}

bool ZoneServer::CombatProcess(Spawn* spawn) {
	bool ret = true;

	if (spawn && spawn->IsEntity())
		((Entity*)spawn)->ProcessCombat();
	if (spawn && !spawn->Alive() && !spawn->IsLootDispensed()) {
		LootProcess(spawn);
	}

	return ret;
}

void ZoneServer::LootProcess(Spawn* spawn) {
	if (spawn->GetLootMethod() == GroupLootMethod::METHOD_ROUND_ROBIN) {
		spawn->LockLoot();
		if (spawn->CheckLootTimer() || spawn->IsLootWindowComplete()) {
			LogWrite(LOOT__INFO, 0, "Loot", "%s: Dispensing loot, loot window was completed? %s.", spawn->GetName(), spawn->IsLootWindowComplete() ? "YES" : "NO");
			spawn->DisableLootTimer();
			spawn->SetLootDispensed();
			Spawn* looter = nullptr;
			if (spawn->GetLootGroupID() < 1 && spawn->GetLootWindowList()->size() > 0) {
				std::map<int32, bool>::iterator itr;

				for (itr = spawn->GetLootWindowList()->begin(); itr != spawn->GetLootWindowList()->end(); itr++) {
					Spawn* entry = GetSpawnByID(itr->first, true);
					if (entry->IsPlayer()) {
						looter = entry;
						break;
					}
				}

				int32 item_id = 0;
				std::vector<int32> item_list;
				spawn->GetLootItemsList(&item_list);
				spawn->UnlockLoot();

				std::vector<int32>::iterator item_itr;

				for (item_itr = item_list.begin(); item_itr != item_list.end(); item_itr++) {
					int32 item_id = *item_itr;
					Item* tmpItem = master_item_list.GetItem(item_id);

					bool skipItem = spawn->IsItemInLootTier(tmpItem);

					if (skipItem)
						continue;

					if (looter) {
						if (looter->IsPlayer()) {

							Item* item = spawn->LootItem(item_id);
							bool success = false;
							success = ((Player*)looter)->GetClient()->HandleLootItem(spawn, item, ((Player*)looter));

							if (!success)
								spawn->AddLootItem(item);
						}
						else {
							Item* item = spawn->LootItem(item_id);
							safe_delete(item);
						}
					}
				}
			}
			else if (spawn->GetLootGroupID() > 0) {
				int32 item_id = 0;
				std::vector<int32> item_list;
				spawn->GetLootItemsList(&item_list);
				spawn->UnlockLoot();
				spawn->DistributeGroupLoot_RoundRobin(&item_list);
			}
			
			if (!spawn->HasLoot()) {
				if (spawn->IsNPC())
					RemoveDeadSpawn(spawn);
			}
			else {
				spawn->LockLoot();
				spawn->SetLootMethod(GroupLootMethod::METHOD_FFA, 0, 0);
				spawn->SetLooterSpawnID(0);
				spawn->UnlockLoot();
			}
		}
		else {
			spawn->UnlockLoot();
		}
	}
	else if ((spawn->GetLootMethod() == GroupLootMethod::METHOD_LOTTO || spawn->GetLootMethod() == GroupLootMethod::METHOD_NEED_BEFORE_GREED) && spawn->IsLootTimerRunning()) {
		spawn->LockLoot();
		if (spawn->CheckLootTimer() || spawn->IsLootWindowComplete()) {
			LogWrite(LOOT__INFO, 0, "Loot", "%s: Dispensing loot, loot window was completed? %s.", spawn->GetName(), spawn->IsLootWindowComplete() ? "YES" : "NO");
			spawn->DisableLootTimer();
			spawn->SetLootDispensed();

			// identify any clients that still have the loot window open, close it out
			CloseSpawnLootWindow(spawn);

			// lotto items while we have loot items in the list
			int32 item_id = 0;
			std::vector<int32> item_list;
			spawn->GetLootItemsList(&item_list);
			spawn->UnlockLoot();

			std::vector<int32>::iterator item_itr;

			for (item_itr = item_list.begin(); item_itr != item_list.end(); item_itr++) {
				int32 item_id = *item_itr;
				Item* tmpItem = master_item_list.GetItem(item_id);

				bool skipItem = spawn->IsItemInLootTier(tmpItem);

				if (skipItem)
					continue;

				std::map<int32, int32> out_entries;
				std::map<int32, int32>::iterator out_itr;
				bool itemNeed = true;
				switch (spawn->GetLootMethod()) {
				case GroupLootMethod::METHOD_LOTTO: {
					spawn->GetSpawnLottoEntries(item_id, &out_entries);
					break;
				}
				case GroupLootMethod::METHOD_NEED_BEFORE_GREED: {
					spawn->GetSpawnNeedGreedEntries(item_id, true, &out_entries);
					if (out_entries.size() < 1) {
						spawn->GetSpawnNeedGreedEntries(item_id, false, &out_entries);
						itemNeed = false;
					}
					break;
				}
				}
				if (out_entries.size() < 1) {
					LogWrite(LOOT__INFO, 0, "Loot", "%s: No spawns matched for loot attempt of %s (%u), skip item.", spawn->GetName(), tmpItem ? tmpItem->name.c_str() : "Unknown", item_id);
					continue;
				}
				Spawn* looter = nullptr;
				int32 curWinValue = 0;
				for (out_itr = out_entries.begin(); out_itr != out_entries.end(); out_itr++) {
					Spawn* entry = GetSpawnByID(out_itr->first, true);
					if ((out_itr->second > curWinValue) || looter == nullptr) {
						curWinValue = out_itr->second;
						looter = entry;
					}
					if (spawn->GetLootMethod() == GroupLootMethod::METHOD_LOTTO) {
						world.GetGroupManager()->SendGroupChatMessage(spawn->GetLootGroupID(), CHANNEL_LOOT_ROLLS, "%s rolled %u on %s.", entry->GetName(), out_itr->second, tmpItem ? tmpItem->name.c_str() : "Unknown");
					}
					else {
						world.GetGroupManager()->SendGroupChatMessage(spawn->GetLootGroupID(), CHANNEL_LOOT_ROLLS, "%s rolled %s (%u) on %s.", entry->GetName(), itemNeed ? "NEED" : "GREED", out_itr->second, tmpItem ? tmpItem->name.c_str() : "Unknown");
					}
				}

				if (looter) {
					if (looter->IsPlayer()) {
						Item* item = spawn->LootItem(item_id);
						bool success = false;
						success = ((Player*)looter)->GetClient()->HandleLootItem(spawn, item, ((Player*)looter));

						if (!success)
							spawn->AddLootItem(item);
					}
					else {
						Item* item = spawn->LootItem(item_id);
						safe_delete(item);
					}
				}
			}

			if (!spawn->HasLoot()) {
				if (spawn->IsNPC())
					RemoveDeadSpawn(spawn);
			}
			else {
				spawn->LockLoot();
				spawn->SetLootMethod(GroupLootMethod::METHOD_FFA, 0, 0);
				spawn->SetLooterSpawnID(0);
				spawn->UnlockLoot();
			}
		}
		else {
			spawn->UnlockLoot();
		}
	}
}

void ZoneServer::CloseSpawnLootWindow(Spawn* spawn) {
	if (spawn->GetLootWindowList()->size() > 0) {
		std::map<int32, bool>::iterator itr;
		for (itr = spawn->GetLootWindowList()->begin(); itr != spawn->GetLootWindowList()->end(); itr++) {
			if (itr->second)
				continue;

			itr->second = true;
			Spawn* looter = GetSpawnByID(itr->first, true);
			if (looter && looter->IsPlayer() && ((Player*)looter)->GetClient()) {
				LogWrite(LOOT__DEBUG, 0, "Loot", "%s: Close loot for player %s.", spawn->GetName(), looter->GetName());
				((Player*)looter)->GetClient()->CloseLoot(spawn->GetID());
			}
		}
	}
}
void ZoneServer::AddPendingDelete(Spawn* spawn) {
	MSpawnDeleteList.writelock(__FUNCTION__, __LINE__);
	spawn->SetDeletedSpawn(true);
	if (spawn_delete_list.count(spawn) == 0)
		spawn_delete_list.insert(make_pair(spawn, Timer::GetCurrentTime2() + spawn_delete_timer)); //give other threads up to 30 seconds to stop using this spawn reference
	MSpawnDeleteList.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::DeleteSpawns(bool delete_all) {
	MSpawnDeleteList.writelock(__FUNCTION__, __LINE__);
	MPendingSpawnRemoval.readlock(__FUNCTION__, __LINE__);
	if(spawn_delete_list.size() > 0){
		map<Spawn*, int32>::iterator itr;
		map<Spawn*, int32>::iterator erase_itr;
		int32 current_time = Timer::GetCurrentTime2();
		Spawn* spawn = 0;
		for (itr = spawn_delete_list.begin(); itr != spawn_delete_list.end(); ) {
			if (delete_all || current_time >= itr->second){
				// we haven't removed it from the spawn list yet..
				if(!delete_all && m_pendingSpawnRemove.count(itr->first->GetID()))
					continue;
				
				spawn = itr->first;
				
				lua_interface->SetLuaUserDataStale(spawn);
				
				if (spellProcess) {
					spellProcess->RemoveCaster(spawn, true);
				}

				if(movementMgr != nullptr) {
					movementMgr->RemoveMob((Entity*)spawn);
				}

				// delete brain if it has one
				if(spawn->IsNPC()) {
					NPC* tmpNPC = (NPC*)spawn;
					if(tmpNPC->Brain())
						tmpNPC->SetBrain(nullptr);
				}

				erase_itr = itr++;
				spawn_delete_list.erase(erase_itr);
				
				MSpawnList.writelock(__FUNCTION__, __LINE__);
				std::map<int32, Spawn*>::iterator sitr = spawn_list.find(spawn->GetID());
				if(sitr != spawn_list.end()) {
					spawn_list.erase(sitr);
				}
				
				if(spawn->IsCollector()) {
					std::map<int32, Spawn*>::iterator subitr = subspawn_list[SUBSPAWN_TYPES::COLLECTOR].find(spawn->GetID());
					if(subitr != subspawn_list[SUBSPAWN_TYPES::COLLECTOR].end()) {
						subspawn_list[SUBSPAWN_TYPES::COLLECTOR].erase(subitr);
					}
				}
				
				if(spawn->GetPickupItemID()) {
					std::map<int32, Spawn*>::iterator subitr = subspawn_list[SUBSPAWN_TYPES::HOUSE_ITEM_SPAWN].find(spawn->GetPickupItemID());
					if(subitr != subspawn_list[SUBSPAWN_TYPES::HOUSE_ITEM_SPAWN].end() && subitr->second == spawn) {
						subspawn_list[SUBSPAWN_TYPES::HOUSE_ITEM_SPAWN].erase(subitr);
					}
					housing_spawn_map.erase(spawn->GetID());
				}
				MSpawnList.releasewritelock(__FUNCTION__, __LINE__);
				safe_delete(spawn);
			}
			else
				itr++;
		}
	}
	MPendingSpawnRemoval.releasereadlock(__FUNCTION__, __LINE__);
	MSpawnDeleteList.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::AddDamagedSpawn(Spawn* spawn){
	if (spawn)
		damaged_spawns.Add(spawn->GetID());
}

void ZoneServer::RemoveDamagedSpawn(Spawn* spawn){
	if (spawn)
		damaged_spawns.Remove(spawn->GetID());
}

bool ZoneServer::Process()
{
	MMasterZoneLock->lock(); //Changing this back to a recursive lock to fix a possible /reload spells crash with multiple zones running - Foof
	SetWatchdogTime(Timer::GetCurrentTime2());
#ifndef NO_CATCH
	try
	{
#endif
			while (zoneID == 0) { //this is loaded by world
				SetWatchdogTime(Timer::GetCurrentTime2());
				Sleep(10);
			}

			if (LoadingData) {
				if (lua_interface) {
					string tmpScript("ZoneScripts/");
					tmpScript.append(GetZoneName());
					tmpScript.append(".lua");
					struct stat buffer;
					bool fileExists = (stat(tmpScript.c_str(), &buffer) == 0);
					if (fileExists)
						lua_interface->RunZoneScript(tmpScript.c_str(), "preinit_zone_script", this);
				}

			if (reloading) {
				LogWrite(COMMAND__DEBUG, 0, "Command", "-Loading Entity Commands...");
				database.LoadEntityCommands(this);
				LogWrite(NPC__INFO, 0, "NPC", "-Loading Spirit Shard data...");
				database.LoadSpiritShards(this);
				LogWrite(NPC__INFO, 0, "NPC", "-Load Spirit Shard data complete!");

				LogWrite(NPC__INFO, 0, "NPC", "-Loading NPC data...");
				database.LoadNPCs(this);
				LogWrite(NPC__INFO, 0, "NPC", "-Load NPC data complete!");

				LogWrite(OBJECT__INFO, 0, "Object", "-Loading Object data...");
				database.LoadObjects(this);
				LogWrite(OBJECT__INFO, 0, "Object", "-Load Object data complete!");

				LogWrite(SIGN__INFO, 0, "Sign", "-Loading Sign data...");
				database.LoadSigns(this);
				LogWrite(SIGN__INFO, 0, "Sign", "-Load Sign data complete!");

				LogWrite(WIDGET__INFO, 0, "Widget", "-Loading Widget data...");
				database.LoadWidgets(this);
				LogWrite(WIDGET__INFO, 0, "Widget", "-Load Widget data complete!");

				LogWrite(GROUNDSPAWN__INFO, 0, "GSpawn", "-Loading Groundspawn data...");
				database.LoadGroundSpawns(this);
				database.LoadGroundSpawnEntries(this);
				LogWrite(GROUNDSPAWN__INFO, 0, "GSpawn", "-Load Groundspawn data complete!");

				LogWrite(PET__INFO, 0, "Pet", "-Loading Pet data...");
				database.GetPetNames(this);
				LogWrite(PET__INFO, 0, "Pet", "-Load Pet data complete!");

				LogWrite(LOOT__INFO, 0, "Loot", "-Loading Spawn loot data...");
				database.LoadLoot(this);
				LogWrite(LOOT__INFO, 0, "Loot", "-Loading Spawn loot data complete!");

				LogWrite(TRANSPORT__INFO, 0, "Transport", "-Loading Transporters...");
				database.LoadTransporters(this);
				LogWrite(TRANSPORT__INFO, 0, "Transport", "-Loading Transporters complete!");
				reloading = false;
				world.RemoveReloadingSubSystem("Spawns");
			}

			MSpawnGroupAssociation.writelock(__FUNCTION__, __LINE__);
			spawn_group_associations.clear();
			MSpawnGroupAssociation.releasewritelock(__FUNCTION__, __LINE__);

			MSpawnGroupLocations.writelock(__FUNCTION__, __LINE__);
			spawn_group_locations.clear();
			MSpawnGroupLocations.releasewritelock(__FUNCTION__, __LINE__);

			MSpawnLocationGroups.writelock(__FUNCTION__, __LINE__);
			spawn_location_groups.clear();
			MSpawnLocationGroups.releasewritelock(__FUNCTION__, __LINE__);

			MSpawnGroupChances.writelock(__FUNCTION__, __LINE__);
			spawn_group_chances.clear();
			MSpawnGroupChances.releasewritelock(__FUNCTION__, __LINE__);
			Map* zonemap = world.GetMap(std::string(GetZoneFile()),0);
			while (zonemap != nullptr && zonemap->IsMapLoading())
			{
				SetWatchdogTime(Timer::GetCurrentTime2());
				// Client loop
				ClientProcess(true);
				Sleep(10);
			}
			
			default_zone_map = world.GetMap(std::string(GetZoneFile()),0);

			DeleteTransporters();
			ReloadTransporters();

			database.LoadSpawns(this);
			ProcessSpawnLocations();

			if (!revive_points)
				revive_points = new vector<RevivePoint*>;
			else {
				while (!revive_points->empty()) {
					safe_delete(revive_points->back());
					revive_points->pop_back();
				}
			}
			database.LoadRevivePoints(revive_points, GetZoneID());

			RemoveLocationGrids();
			database.LoadLocationGrids(this);


			MMasterZoneLock->unlock();
			
			while(true) {
				ProcessPendingSpawns();
				Sleep(20);
				MPendingSpawnListAdd.readlock(__FUNCTION__, __LINE__);
				int32 count = pending_spawn_list_add.size();
				MPendingSpawnListAdd.releasereadlock(__FUNCTION__, __LINE__);
				if(count < 1)
					break;
			}
	
			startupDelayTimer.Start(60000); // this is hard coded for 60 seconds after the zone is loaded to allow a client to at least add itself to the list before we start zone shutdown timer
			
			MMasterZoneLock->lock();
			
			LoadingData = false;

			const char* zone_script = world.GetZoneScript(this->GetZoneID());
			if (lua_interface && zone_script) {
				RemoveLocationProximities();
				lua_interface->RunZoneScript(zone_script, "init_zone_script", this);
			}

			spawn_range.Trigger();
			spawn_check_add.Trigger();
		}

		if (reloading_spellprocess){
			MMasterZoneLock->unlock();
			return !zoneShuttingDown;
		}
		
		if(shutdownTimer.Enabled() && shutdownTimer.Check() && connected_clients.size(true) == 0) {
			//if(lifetime_client_count)
				zoneShuttingDown = true;
			/*else { // allow up to 120 seconds then timeout
				LogWrite(ZONE__WARNING, 0, "Zone", "No clients have connected to zone '%s' and the shutdown timer has counted down -- will delay shutdown for 120 seconds.", GetZoneName());
				shutdownTimer.Start(120000, true);
				lifetime_client_count = 1;
			}*/
			MMasterZoneLock->unlock();
			return false;
		}

		// client loop
		if(charsheet_changes.Check()) {
			SendCharSheetChanges();
		}
		else {
			SendRaidSheetChanges();
		}

		// Client loop
		ClientProcess(startupDelayTimer.Enabled());
		if(startupDelayTimer.Check())
			startupDelayTimer.Disable();

		if(!reloading && spellProcess)
			spellProcess->Process();
		if (tradeskillMgr)
			tradeskillMgr->Process();
		
		// Client loop
		if(client_save.Check())
			SaveClients();

		// Possibility to do a client loop
		if(weather_enabled && weatherTimer.Check())
			ProcessWeather();

		// client related loop, move to main thread?
		if(!zoneShuttingDown)
			ProcessDrowning();

		// client than location_proximities loop, move to main thread
		if (location_prox_timer.Check() && !zoneShuttingDown)
			CheckLocationProximity();

		// client than location_grid loop, move to main thread
		if (location_grid_timer.Check() && !zoneShuttingDown)
			CheckLocationGrids();

		if (sync_game_time_timer.Check() && !zoneShuttingDown)
			SendTimeUpdateToAllClients();

		world.MWorldTime.readlock(__FUNCTION__, __LINE__);
		int hour = world.GetWorldTimeStruct()->hour;
		int minute = world.GetWorldTimeStruct()->minute;
		world.MWorldTime.releasereadlock(__FUNCTION__, __LINE__);

		if (!isDusk && (hour >= 19 || hour < 8)) {//((hour > dusk_hour || hour < dawn_hour) || ((dusk_hour == hour && minute >= dusk_minute) || (hour == dawn_hour && minute < dawn_minute)))) {
			isDusk = true;
			const char* zone_script = world.GetZoneScript(GetZoneID());
			if (lua_interface && zone_script)
				lua_interface->RunZoneScript(zone_script, "dusk", this);

			ProcessSpawnConditional(SPAWN_CONDITIONAL_NIGHT);
		}
		else if (isDusk && hour >= 8 && hour < 19) {//((hour > dawn_hour && hour < dusk_hour) || ((hour == dawn_hour && minute >= dawn_minute) || (hour == dusk_hour && minute < dusk_minute)))) {
			isDusk = false;
			const char* zone_script = world.GetZoneScript(GetZoneID());
			if (lua_interface && zone_script)
				lua_interface->RunZoneScript(zone_script, "dawn", this);

			ProcessSpawnConditional(SPAWN_CONDITIONAL_DAY);
		}

		// damaged spawns loop, spawn related, move to spawn thread?
		if(regenTimer.Check())
			RegenUpdate();

		// respawn_timers loop
		if(respawn_timer.Check() && !zoneShuttingDown)
			CheckRespawns();

		// spawn_expire_timers loop
		if (spawn_expire_timer.Check() && !zoneShuttingDown)
			CheckSpawnExpireTimers();

		// widget_timers loop
		if(widget_timer.Check() && !zoneShuttingDown)
			CheckWidgetTimers();

		// spawn_script_timers loop
		if(!reloading && !zoneShuttingDown)
			CheckSpawnScriptTimers();

		// Check to see if a dead spawn needs to be removed
		CheckDeadSpawnRemoval();
#ifndef NO_CATCH
	}
	catch(...)
	{
		LogWrite(ZONE__ERROR, 0, "Zone", "Exception while running '%s'", GetZoneName());
		zoneShuttingDown = true;
		MMasterZoneLock->unlock();
		return false;
	}
#endif
	MMasterZoneLock->unlock();
	return (zoneShuttingDown == false);
}

bool ZoneServer::SpawnProcess(){
	if(depop_zone) {
		depop_zone = false;
		ProcessDepop(respawns_allowed, repop_zone);
		finished_depop = true;
	}

	MMasterSpawnLock.writelock(__FUNCTION__, __LINE__);
	// If the zone is loading data or shutting down don't do anything
	if(!LoadingData && !zoneShuttingDown && !reloading_spellprocess) {
		// Set some bool's for timers
		bool movement = movement_timer.Check();
		bool spawnRange = spawn_range.Check();
		bool checkRemove = spawn_check_remove.Check();
		bool aggroCheck = aggro_timer.Check();
		vector<int32> pending_spawn_list_remove;

		// Check to see if there are any spawn id's that need to be removed from the spawn list, if so remove them all
		ProcessSpawnRemovals();
		
		map<int32, Spawn*>::iterator itr;
		if (spawnRange || checkRemove)
		{
			// Loop through the spawn list
			MSpawnList.readlock(__FUNCTION__, __LINE__);
			// Loop throught the list to set up spawns to be sent to clients
			for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
				// if zone is shutting down kill the loop
				if (zoneShuttingDown)
					break;

				Spawn* spawn = itr->second;
				if (spawn) {
					// Checks the range to all clients in the zone
					if (spawnRange)
						CheckSpawnRange(spawn);

					// Checks to see if the spawn needs to be removed from a client
					if (checkRemove)
						CheckRemoveSpawnFromClient(spawn);
				}
			}
			MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
		}

		// Broke the spawn loop into 2 so spawns are sent to the client faster, send the spawns to clients now then resume the spawn loop

		// client loop, move to main thread?
		// moved this back to the spawn thread as on the main thread during a depop, done on the spawn thread, spawns would start to pop again
		// might be an issue with other functions moved from the spawn thread to the main thread?
		if(spawn_check_add.Check() && !zoneShuttingDown)
			CheckSendSpawnToClient();


		// send spawn changes, changed_spawns loop
		if (spawn_update.Check() && !zoneShuttingDown) { //check for changed spawns every {Rule:SpawnUpdateTimer} milliseconds (default: 200ms)
			SendSpawnChanges();
		}

		if (movement || aggroCheck)
		{
			MSpawnList.readlock(__FUNCTION__, __LINE__);
			for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
				// Break the loop if the zone is shutting down
				if (zoneShuttingDown)
					break;

				Spawn* spawn = itr->second;
				if (spawn) {
					// Process spawn movement
					if (movement) {
						spawn->ProcessMovement(true);
						// update last_movement_update for all spawns (used for time_step)
						spawn->last_movement_update = Timer::GetCurrentTime2();
						if (!aggroCheck)
							CombatProcess(spawn);
					}

					// Makes NPC's KOS to other NPC's or players
					if (aggroCheck)
					{
						ProcessAggroChecks(spawn);
						CombatProcess(spawn);
					}
				}
				else {
					// unable to get a valid spawn, lets add the id to another list to remove from the spawn list after this loop is finished
					pending_spawn_list_remove.push_back(itr->first);
				}

			}
			MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
		}

		// Check to see if there are any spawn id's that need to be removed from the spawn list, if so remove them all
		if (pending_spawn_list_remove.size() > 0) {
			MSpawnList.writelock(__FUNCTION__, __LINE__);
			vector<int32>::iterator itr2;
			for (itr2 = pending_spawn_list_remove.begin(); itr2 != pending_spawn_list_remove.end(); itr2++) {
				spawn_list.erase(*itr2);
				subspawn_list[SUBSPAWN_TYPES::COLLECTOR].erase(*itr2);
				
				std::map<int32,int32>::iterator hsmitr = housing_spawn_map.find(*itr2);
				if(hsmitr != housing_spawn_map.end()) {
					subspawn_list[SUBSPAWN_TYPES::HOUSE_ITEM_SPAWN].erase(hsmitr->second);
					housing_spawn_map.erase(hsmitr);
				}
			}

			pending_spawn_list_remove.clear();
			MSpawnList.releasewritelock(__FUNCTION__, __LINE__);
		}
		
		// Double Check to see if there are any spawn id's that need to be removed from the spawn list, if so remove them before we replace with pending spawns
		// and also potentially further down when we delete the Spawn* in DeleteSpawns(false)
		ProcessSpawnRemovals();

		// Check to see if there are spawns waiting to be added to the spawn list, if so add them all
		if (pending_spawn_list_add.size() > 0) {
			ProcessPendingSpawns();
		}

		MSpawnList.readlock(__FUNCTION__, __LINE__);
		if (movementMgr != nullptr)
			movementMgr->Process();
		MSpawnList.releasereadlock(__FUNCTION__, __LINE__);

		if(queue_updates.Check())
			ProcessQueuedStateCommands();
		// Do other loops for spawns
		// tracking, client loop with spawn loop for each client that is tracking, change to a spawn_range_map loop instead of using the main spawn list?
		//if (tracking_timer.Check())
			//ProcessTracking(); // probably doesn't work as spawn loop iterator is never set


		// Delete unused spawns, do this last
		if(!zoneShuttingDown)
			DeleteSpawns(false);

		// Nothing should come after this


		//LogWrite(PLAYER__ERROR, 0, "Debug", "Spawn loop time %u", Timer::GetCurrentTime2() - time);
	}

	MMasterSpawnLock.releasewritelock(__FUNCTION__, __LINE__);
	
	return (zoneShuttingDown == false);
}

void ZoneServer::CheckDeadSpawnRemoval() {
	MDeadSpawns.writelock(__FUNCTION__, __LINE__);
		if(dead_spawns.size() > 0){
			vector<Spawn*> tmp_dead_list;			
			int32 current_time = Timer::GetCurrentTime2();
			Spawn* spawn = 0;
			map<int32, int32>::iterator itr = dead_spawns.begin();
			map<int32, int32>::iterator itr_delete;
			while (itr != dead_spawns.end()) {
				spawn = GetSpawnByID(itr->first);
				if (spawn) {
					if(current_time >= itr->second)
						tmp_dead_list.push_back(spawn);
					itr++;
				}
				else {
					itr_delete = itr++;
					dead_spawns.erase(itr_delete);
				}
			}
			for(int i=tmp_dead_list.size()-1;i>=0;i--){
				spawn = tmp_dead_list[i];
				if (!spawn->IsPlayer())
				{
					dead_spawns.erase(spawn->GetID());
					MDeadSpawns.releasewritelock(__FUNCTION__, __LINE__);
					RemoveSpawn(spawn, true, true, true, true, true);
					MDeadSpawns.writelock(__FUNCTION__, __LINE__);
				}
			}
		}
		MDeadSpawns.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::CheckRespawns(){	
	vector<int32> tmp_respawn_list;
	MutexMap<int32, int32>::iterator itr = respawn_timers.begin();
	while(itr.Next()){
		if(Timer::GetCurrentTime2() >= itr->second)
			tmp_respawn_list.push_back(itr->first);
	}
	for(int i=tmp_respawn_list.size()-1;i>=0;i--){
		if ( IsInstanceZone() )
		{
			database.DeleteInstanceSpawnRemoved(GetInstanceID(),tmp_respawn_list[i]);
		}
		else {
			database.DeletePersistedRespawn(GetZoneID(),tmp_respawn_list[i]);
		}

		ProcessSpawnLocation(tmp_respawn_list[i], nullptr, nullptr, nullptr, nullptr, nullptr, true);
		respawn_timers.erase(tmp_respawn_list[i]);
	}
}

void ZoneServer::SendRespawnTimerList(Client* client){	
	MutexMap<int32, int32>::iterator itr = respawn_timers.begin();
		client->Message(CHANNEL_FACTION, "Respawn Timers:");
		client->Message(CHANNEL_FACTION, "Location ID : Time Remaining");
	while(itr.Next()){
		client->Message(CHANNEL_FACTION, "%u: %i seconds.", itr->first, (itr->second - Timer::GetCurrentTime2())/1000);
	}
}

void ZoneServer::CheckSpawnExpireTimers() {
	MutexMap<int32, int32>::iterator itr = spawn_expire_timers.begin();
	while (itr.Next()) {
		Spawn* spawn = GetSpawnByID(itr->first);
		if (spawn) {
			if (Timer::GetCurrentTime2() >= itr.second) {
				spawn_expire_timers.erase(itr.first);
				Despawn(spawn, spawn->GetRespawnTime());
			}
		}
		else
			spawn_expire_timers.erase(itr->first);
	}
}

void ZoneServer::AddSpawnExpireTimer(Spawn* spawn, int32 expire_time, int32 expire_offset) {
	if (spawn) {
		int32 actual_expire_time = expire_time;
		if (expire_offset > 0) {
			int32 low = expire_time;
			int32 high = expire_time + expire_offset;
			if (expire_offset < expire_time)
				low = expire_time - expire_offset;
			int32 range = (high - low) + 1;
			actual_expire_time = (low + (int32)((range * rand()) / (RAND_MAX + 1.0)));
		}
		actual_expire_time *= 1000;
		spawn_expire_timers.Put(spawn->GetID(), Timer::GetCurrentTime2() + actual_expire_time);
	}
}

void ZoneServer::SaveClient(Client* client){
	client->Save();
}

void ZoneServer::SaveClients(){
	vector<Client*>::iterator itr;
	Client* client = 0;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		client = *itr;
		if(client->IsConnected() && client->IsReadyForUpdates()){
			SaveClient(client);	
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::SendSpawnVisualState(Spawn* spawn, int16 type){
	if(!spawn)
		return;

	vector<Client*>::iterator itr;
	spawn->SetTempVisualState(type);
	Client* client = 0;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		client = *itr;
		if(client && client->GetPlayer() != spawn)
			AddChangedSpawn(spawn);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::SendSpawnChangesByDBID(int32 db_id, Client* client, bool override_changes, bool override_vis_changes){
	Spawn* spawn = GetSpawnByDatabaseID(db_id);
	if(spawn && (spawn->changed || override_changes || override_vis_changes))
		SendSpawnChanges(spawn, client, override_changes, override_vis_changes);
}

void ZoneServer::SendSpawnChanges(Spawn* spawn, Client* client, bool override_changes, bool override_vis_changes){
	if(client && client->IsConnected() && client->IsReadyForUpdates() && client->GetPlayer()->WasSentSpawn(spawn->GetID()) && (spawn->IsTransportSpawn() || client->GetPlayer()->GetDistance(spawn) < SEND_SPAWN_DISTANCE)){
		EQ2Packet* outapp = spawn->spawn_update_packet(client->GetPlayer(), client->GetVersion(), override_changes, override_vis_changes);
		if(outapp)
			client->QueuePacket(outapp);
	}
}

void ZoneServer::SendSpawnChanges(Spawn* spawn){
	MClientList.readlock(__FUNCTION__, __LINE__);
	MSpawnList.readlock();
	if(spawn && spawn->changed){
		if(!spawn->IsPlayer() || (spawn->IsPlayer() && (spawn->info_changed || spawn->vis_changed))){
			vector<Client*>::iterator itr;
			Client* client = 0;

			// MClientList locked at a higher level
			for (itr = clients.begin(); itr != clients.end(); itr++) {
				client = *itr;
				SendSpawnChanges(spawn, client);
			}
		}
		spawn->changed = false;
		spawn->info_changed = false;
		if(spawn->IsPlayer() == false)
			spawn->position_changed = false;
		spawn->vis_changed = false;
	}
	MSpawnList.releasereadlock();
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

Spawn* ZoneServer::FindSpawn(Player* searcher, const char* name){
	if(!searcher || !name)
		return 0;

	Spawn* spawn = 0;
	vector<Spawn*> find_spawn_list;
	vector<Spawn*>::iterator fspawn_iter;
	int8 name_size = strlen(name);
	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if(spawn && !strncasecmp(spawn->GetName(), name, name_size))
			find_spawn_list.push_back(spawn);
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	Spawn* closest = 0;
	float distance = 0;
	float test_distance = 0;
	for(fspawn_iter=find_spawn_list.begin(); fspawn_iter!=find_spawn_list.end(); fspawn_iter++){
		spawn = *fspawn_iter;
		if(spawn && ((((test_distance = searcher->GetDistance(spawn)) < distance)) || !closest)){
			distance = test_distance;
			closest = spawn;
		}
	}
	return closest;
}

void ZoneServer::AddChangedSpawn(Spawn* spawn) {
	if (!spawn || (spawn->IsPlayer() && !spawn->info_changed && !spawn->vis_changed) || (spawn->IsPlayer() && ((Player*)spawn)->GetClient() && ((Player*)spawn)->GetClient()->IsReadyForUpdates() == false))
		return;
	
    MChangedSpawns.lock_shared();
	ChangedSpawnMapType::iterator it = changed_spawns.find(spawn->GetID());
	if (it != changed_spawns.end()) {
		it->second = true;
		MChangedSpawns.unlock_shared();
	}
	else {
		MChangedSpawns.unlock_shared();
		MChangedSpawns.lock();
		changed_spawns.insert(make_pair(spawn->GetID(),true));
		MChangedSpawns.unlock();
	}
}

void ZoneServer::RemoveChangedSpawn(Spawn* spawn){
	if(!spawn)
		return;
	
    MChangedSpawns.lock();
	ChangedSpawnMapType::iterator it = changed_spawns.find(spawn->GetID());
	if (it != changed_spawns.end()) {
		it->second = false;
	}
	MChangedSpawns.unlock();
}

void ZoneServer::AddDrowningVictim(Player* player){
	Client* client = ((Player*)player)->GetClient();
	if(client && drowning_victims.count(client) == 0)
		drowning_victims.Put(client, Timer::GetCurrentTime2());
}

void ZoneServer::RemoveDrowningVictim(Player* player){
	Client* client = ((Player*)player)->GetClient();
	if(client)
		drowning_victims.erase(client);
}

Client* ZoneServer::GetDrowningVictim(Player* player){
	Client* client = ((Player*)player)->GetClient();
	if(client && drowning_victims.count(client) > 0)
		return(client);
	return 0;
}

void ZoneServer::ProcessDrowning(){
	vector<Client*> dead_list;
	if(drowning_victims.size(true) > 0){
		sint32 damage = 0;
		int32 current_time = Timer::GetCurrentTime2();
		MutexMap<Client*, int32>::iterator itr = drowning_victims.begin();
		while(itr.Next()){
			if(current_time >= itr->second) {
				Client* client = itr->first;
				Player* player = client->GetPlayer();
				drowning_victims.Get(client) = Timer::GetCurrentTime2() + 2000;
				damage = player->GetTotalHP()/20 + player->GetInfoStruct()->get_hp_regen();
				player->TakeDamage(damage);
				if(!player->Alive())
					dead_list.push_back(client);
				player->SetCharSheetChanged(true);
				SendCharSheetChanges(client);
				SendDamagePacket(0, player, DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE, DAMAGE_PACKET_RESULT_SUCCESSFUL, DAMAGE_PACKET_DAMAGE_TYPE_DROWN, damage, 0);
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are drowning!");
			}
		}
	}
	if(dead_list.size() > 0){
		vector<Client*>::iterator itr;
		for(itr = dead_list.begin(); itr != dead_list.end(); itr++){
			RemoveDrowningVictim((*itr)->GetPlayer());
			KillSpawn(false, (*itr)->GetPlayer(), nullptr, true, 0, 0, 10); // kill blow type 10 means death by WATER! (glug glug!)
		}
	}
}

void ZoneServer::SendSpawnChanges(){
    std::shared_lock lock(MChangedSpawns);
	if (changed_spawns.size() < 1)
		return;

	set<Spawn*> spawns_to_send;
	Spawn* spawn = 0;

	int count = 0;
	
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	
	for( ChangedSpawnMapType::iterator it = changed_spawns.begin(); it != changed_spawns.end(); ++it ) {
		if(!it->second)
			continue;
		
		spawn = GetSpawnByID(it->first);
		if(spawn){
			spawns_to_send.insert(spawn);
			count++;
		}
	}
	
	vector<Client*>::iterator client_itr;
	Client* client = 0;
	
	MClientList.readlock(__FUNCTION__, __LINE__);
	if(clients.size())
	{
		for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
			client = *client_itr;
			if(client)
				client->SendSpawnChanges(spawns_to_send);
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);

	for (const auto& spawn : spawns_to_send) {
		spawn->changed = false;
		spawn->position_changed = false;
		spawn->vis_changed = false;
		spawn->info_changed = false;
		spawn->size_changed = false;
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::SendPlayerPositionChanges(Player* player){
	if(player){
		player->position_changed = false;
		Client* client = 0;
		vector<Client*>::iterator client_itr;

		MClientList.readlock(__FUNCTION__, __LINE__);
		for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
			client = *client_itr;
			if(player != client->GetPlayer() && client->GetPlayer()->WasSentSpawn(player->GetID())){
				if(client->GetVersion() > 373) {
					EQ2Packet* outapp = player->player_position_update_packet(client->GetPlayer(), client->GetVersion(), true);
					if(outapp)
						client->QueuePacket(outapp);
				}
			}
		}
		MClientList.releasereadlock(__FUNCTION__, __LINE__);
	}
}

void ZoneServer::SendRaidSheetChanges(){
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		Client* client = (Client*)(*client_itr);
		if(client && client->IsConnected() && client->GetPlayer()->GetRaidSheetChanged()) {
			client->GetPlayer()->SetRaidSheetChanged(false);
			EQ2Packet* packet = client->GetPlayer()->GetRaidUpdatePacket(client->GetVersion());
			if(packet) {
				client->QueuePacket(packet);
			}
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::SendCharSheetChanges(){
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++)
		SendCharSheetChanges(*client_itr);
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::SendCharSheetChanges(Client* client){
	if(client && client->IsConnected()){
		if(client->GetPlayer()->GetCharSheetChanged()) {
			client->GetPlayer()->SetCharSheetChanged(false);
			ClientPacketFunctions::SendCharacterSheet(client);
		}
	}
}

int32 ZoneServer::CalculateSpawnGroup(SpawnLocation* spawnlocation, bool respawn)
{
	int32 group = 0;
	list<int32>* groups_at_location = GetSpawnGroupsByLocation(spawnlocation->placement_id);

	LogWrite(SPAWN__TRACE, 0, "Spawn", "Enter %s", __FUNCTION__);

	if(groups_at_location){
		list<int32>::iterator group_location_itr;
		float chance = 0;
		float total_chance = 0;
		map<int32, float> tmp_chances;		
		set<int32>* associated_groups = 0;
		for (group_location_itr = groups_at_location->begin(); group_location_itr != groups_at_location->end(); group_location_itr++) {
			if(tmp_chances.count(*group_location_itr) > 0)
				continue;
			associated_groups = GetAssociatedGroups(*group_location_itr);
			if(associated_groups){
				set<int32>::iterator group_itr;
				for (group_itr = associated_groups->begin(); group_itr != associated_groups->end(); group_itr++) {
					chance = GetSpawnGroupChance(*group_itr);
					if(chance > 0){
						total_chance += chance;
						tmp_chances[*group_itr] = chance;
					}
					else
						tmp_chances[*group_itr] = 0;
				}
			}
			else{ //single group, no associations
				chance = GetSpawnGroupChance(*group_location_itr);
				total_chance += chance;
				tmp_chances[*group_location_itr] = chance;				
			}
		}
		if(tmp_chances.size() > 1){
			//set the default for any chances not set
			map<int32, float>::iterator itr2;
			for(itr2 = tmp_chances.begin(); itr2 != tmp_chances.end(); itr2++){
				if(itr2->second == 0){
					total_chance += 100/tmp_chances.size();
					tmp_chances[itr2->first] = (float)(100 / tmp_chances.size());
				}
			}
		}
		if(tmp_chances.size() > 1){
			float roll = (float)(rand()%((int32)total_chance));
			map<int32, float>::iterator itr3;
			for (itr3 = tmp_chances.begin(); itr3 != tmp_chances.end(); itr3++){
				if(itr3->second >= roll){
					group = itr3->first;
					break;
				}
				else
					roll -= itr3->second;
			}
		}
		else if(tmp_chances.size() == 1)
			group = tmp_chances.begin()->first;
	}
	if(group > 0){
		map<int32, int32>* locations = GetSpawnLocationsByGroup(group);
		if(locations){
			map<int32, int32>::iterator itr;
			Spawn* spawn = 0;
			Spawn* leader = 0;

			MSpawnLocationList.readlock(__FUNCTION__, __LINE__);
			for (itr = locations->begin(); itr != locations->end(); itr++) {
				if(spawn_location_list.count(itr->second) > 0){
					spawn = ProcessSpawnLocation(spawn_location_list[itr->second], nullptr, nullptr, nullptr, nullptr, nullptr, respawn);
					if(!leader && spawn)
						leader = spawn;
					if(leader)
						leader->AddSpawnToGroup(spawn);
					if(spawn){						
						//if(spawn_group_map.count(group) == 0)
						//	spawn_group_map.Put(group, new MutexList<Spawn*>());
						MutexList<int32>* groupList = &spawn_group_map.Get(group);
						groupList->Add(spawn->GetID());
						spawn->SetSpawnGroupID(group);
					}
				}
			}
			MSpawnLocationList.releasereadlock(__FUNCTION__, __LINE__);
		}
	}

	LogWrite(SPAWN__TRACE, 0, "Spawn", "Exit %s", __FUNCTION__);

	return group;
}

void ZoneServer::ProcessSpawnLocation(int32 location_id, map<int32,int32>* instNPCs, map<int32,int32>* instGroundSpawns, map<int32,int32>* instObjSpawns, map<int32,int32>* instWidgetSpawns, map<int32,int32>* instSignSpawns, bool respawn)
{
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Enter %s", __FUNCTION__);

	MSpawnLocationList.readlock(__FUNCTION__, __LINE__);

	if(spawn_location_list.count(location_id) > 0)
	{
		if(respawn) //see if there are any spawns still in game associated with this spawn's group, if so, dont spawn this
		{ 
			list<int32>* groups = GetSpawnGroupsByLocation(spawn_location_list[location_id]->placement_id);

			if(groups)
			{
				set<int32>* associated_groups = 0;
				bool should_spawn = true;
				list<int32>::iterator itr;
				for (itr = groups->begin(); itr != groups->end(); itr++) {
					associated_groups = GetAssociatedGroups(*itr);

					if(associated_groups)
					{
						set<int32>::iterator assoc_itr;
						for (assoc_itr = associated_groups->begin(); assoc_itr != associated_groups->end(); assoc_itr++) {
							if(spawn_group_map.count(*assoc_itr) > 0 && spawn_group_map.Get(*assoc_itr).size() > 0)
								should_spawn = false;
						}
					}
				}

				if(should_spawn)
					CalculateSpawnGroup(spawn_location_list[location_id]);

				LogWrite(SPAWN__TRACE, 0, "Spawn", "Exit %s", __FUNCTION__);
				// need to unlock the list before we exit the function
				MSpawnLocationList.releasereadlock(__FUNCTION__, __LINE__);
				return;
			}
		}

		LogWrite(SPAWN__TRACE, 0, "Spawn", "Exit %s", __FUNCTION__);

		ProcessSpawnLocation(spawn_location_list[location_id], instNPCs, instGroundSpawns, instObjSpawns, instWidgetSpawns, instSignSpawns, respawn);
	}
	MSpawnLocationList.releasereadlock(__FUNCTION__, __LINE__);
}

Spawn* ZoneServer::ProcessSpawnLocation(SpawnLocation* spawnlocation, map<int32,int32>* instNPCs, map<int32,int32>* instGroundSpawns, map<int32,int32>* instObjSpawns, map<int32,int32>* instWidgetSpawns, map<int32,int32>* instSignSpawns, bool respawn)
{
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Enter %s", __FUNCTION__);

	if(!spawnlocation)
		return 0;

	Spawn* spawn = 0;
	float rand_number = MakeRandomFloat(0, spawnlocation->total_percentage);

	for(int32 i=0;i<spawnlocation->entities.size();i++)
	{
		if(spawnlocation->entities[i]->spawn_percentage == 0)
			continue;
		if(DuplicatedZone() && !spawnlocation->entities[i]->duplicated_spawn) {
			return nullptr; // dupe public/shared zone, we have turned off duplicating spawns for this location
		}
		
		int32 spawnTime = 1;
		
		if(spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_NPC)
			spawnTime = database.CheckSpawnRemoveInfo(instNPCs,spawnlocation->entities[i]->spawn_location_id);
		else if(spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_OBJECT)
			spawnTime = database.CheckSpawnRemoveInfo(instObjSpawns,spawnlocation->entities[i]->spawn_location_id);
		else if(spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_WIDGET)
			spawnTime = database.CheckSpawnRemoveInfo(instWidgetSpawns,spawnlocation->entities[i]->spawn_location_id);
		else if(spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_SIGN)
			spawnTime = database.CheckSpawnRemoveInfo(instSignSpawns,spawnlocation->entities[i]->spawn_location_id);
		else if(spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_GROUNDSPAWN)
			spawnTime = database.CheckSpawnRemoveInfo(instGroundSpawns,spawnlocation->entities[i]->spawn_location_id);
		
		if(spawnTime == 0) { // don't respawn
			return nullptr;
		}
		else if(spawnTime > 1) { // if not 1, respawn after time
			AddRespawn(spawnlocation->entities[i]->spawn_location_id, spawnTime);
			return nullptr;
		}
		
		if (spawnlocation->conditional > 0) {
			if ((spawnlocation->conditional & SPAWN_CONDITIONAL_DAY) == SPAWN_CONDITIONAL_DAY && isDusk)
				continue;

			if ((spawnlocation->conditional & SPAWN_CONDITIONAL_NIGHT) == SPAWN_CONDITIONAL_NIGHT && !isDusk)
				continue;

			if ((spawnlocation->conditional & SPAWN_CONDITIONAL_DAY) == SPAWN_CONDITIONAL_NOT_RAINING && rain >= 0.75f)
				continue;

			if ((spawnlocation->conditional & SPAWN_CONDITIONAL_DAY) == SPAWN_CONDITIONAL_RAINING && rain < 0.75f)
				continue;
		}

		if (spawnlocation->entities[i]->spawn_percentage >= rand_number) {
			if (spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_NPC)
				spawn = AddNPCSpawn(spawnlocation, spawnlocation->entities[i]);
			else if (spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_GROUNDSPAWN)
				spawn = AddGroundSpawn(spawnlocation, spawnlocation->entities[i]);
			else if (spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_OBJECT)
				spawn = AddObjectSpawn(spawnlocation, spawnlocation->entities[i]);
			else if (spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_WIDGET)
				spawn = AddWidgetSpawn(spawnlocation, spawnlocation->entities[i]);
			else if (spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_SIGN)
				spawn = AddSignSpawn(spawnlocation, spawnlocation->entities[i]);

			if (GetInstanceType() == PERSONAL_HOUSE_INSTANCE)
				database.GetHouseSpawnInstanceData(this, spawn);

			if(spawn && spawn->IsOmittedByDBFlag())
			{
				LogWrite(SPAWN__WARNING, 0, "Spawn", "Spawn (%u) in spawn location id %u was skipped due to a missing expansion / holiday flag being met (ZoneServer::ProcessSpawnLocation)", spawnlocation->entities[i]->spawn_id, spawnlocation->entities[i]->spawn_location_id);
				safe_delete(spawn);
				spawn = 0;
				continue;
			}
			else if (!spawn)
			{
				LogWrite(ZONE__ERROR, 0, "Zone", "Error adding spawn by spawn location to zone %s with location id %u, spawn id %u, spawn type %u.", GetZoneName(), spawnlocation->entities[i]->spawn_location_id, spawnlocation->entities[i]->spawn_id, spawnlocation->entities[i]->spawn_type);
				continue;
			}

			if (spawn) 
			{
				if(respawn)
					CallSpawnScript(spawn, SPAWN_SCRIPT_RESPAWN);
				else	
					CallSpawnScript(spawn, SPAWN_SCRIPT_SPAWN);
			}
			break;
		}
		else
			rand_number -= spawnlocation->entities[i]->spawn_percentage;
	}

	LogWrite(SPAWN__TRACE, 0, "Spawn", "Exit %s", __FUNCTION__);

	return spawn;
}


Spawn* ZoneServer::ProcessInstanceSpawnLocation(SpawnLocation* spawnlocation, map<int32,int32>* instNPCs, map<int32,int32>* instGroundSpawns, map<int32,int32>* instObjSpawns, map<int32,int32>* instWidgetSpawns, map<int32,int32>* instSignSpawns, bool respawn)
{
	if(!spawnlocation)
		return 0;

	LogWrite(SPAWN__TRACE, 0, "Spawn", "Enter %s", __FUNCTION__);

	Spawn* spawn = 0;
	float rand_number = MakeRandomFloat(0, spawnlocation->total_percentage);

	for(int32 i=0;i<spawnlocation->entities.size();i++)
	{
		if(spawnlocation->entities[i]->spawn_percentage == 0)
			continue;

		int32 spawnTime = 0;

		if(spawnlocation->entities[i]->spawn_percentage >= rand_number)
		{
			if(spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_NPC && 
				(spawnTime = database.CheckSpawnRemoveInfo(instNPCs,spawnlocation->entities[i]->spawn_location_id)) > 0)
				spawn = AddNPCSpawn(spawnlocation, spawnlocation->entities[i]);
			else if(spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_GROUNDSPAWN && 
				(spawnTime = database.CheckSpawnRemoveInfo(instGroundSpawns,spawnlocation->entities[i]->spawn_location_id)) > 0)
				spawn = AddGroundSpawn(spawnlocation, spawnlocation->entities[i]);
			else if(spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_OBJECT && 
				(spawnTime = database.CheckSpawnRemoveInfo(instObjSpawns,spawnlocation->entities[i]->spawn_location_id)) > 0)
				spawn = AddObjectSpawn(spawnlocation, spawnlocation->entities[i]);
			else if(spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_WIDGET && 
				(spawnTime = database.CheckSpawnRemoveInfo(instWidgetSpawns,spawnlocation->entities[i]->spawn_location_id)) > 0)
				spawn = AddWidgetSpawn(spawnlocation, spawnlocation->entities[i]);
			else if(spawnlocation->entities[i]->spawn_type == SPAWN_ENTRY_TYPE_SIGN && 
				(spawnTime = database.CheckSpawnRemoveInfo(instSignSpawns,spawnlocation->entities[i]->spawn_location_id)) > 0)
				spawn = AddSignSpawn(spawnlocation, spawnlocation->entities[i]);

			if(spawn && spawn->IsOmittedByDBFlag())
			{
				LogWrite(SPAWN__WARNING, 0, "Spawn", "Spawn (%u) in spawn location id %u was skipped due to a missing expansion / holiday flag being met (ZoneServer::ProcessInstanceSpawnLocation)", spawnlocation->entities[i]->spawn_id, spawnlocation->entities[i]->spawn_location_id);
				safe_delete(spawn);
				spawn = 0;
				continue;
			}

			if (GetInstanceType() == PERSONAL_HOUSE_INSTANCE)
				database.GetHouseSpawnInstanceData(this, spawn);

			const char* script = 0;

			for(int x=0;x<3;x++)
			{
				switch(x)
				{
					case 0:
						script = world.GetSpawnEntryScript(spawnlocation->entities[i]->spawn_entry_id);
						break;
					case 1:
						script = world.GetSpawnLocationScript(spawnlocation->entities[i]->spawn_location_id);
						break;
					case 2:
						script = world.GetSpawnScript(spawnlocation->entities[i]->spawn_id);
						break;
				}

				if(spawn && script && lua_interface->GetSpawnScript(script) != 0)
				{
					spawn->SetSpawnScript(string(script));
					break;
				}
			}

			if(spawn)
			{
				if (respawn)
					CallSpawnScript(spawn, SPAWN_SCRIPT_RESPAWN);
				else
					CallSpawnScript(spawn, SPAWN_SCRIPT_SPAWN);

				if ( spawnTime > 1 )
				{
					spawn->SetRespawnTime(spawnTime);
				}
			}
			break;
		}
		else
			rand_number -= spawnlocation->entities[i]->spawn_percentage;
	}

	LogWrite(SPAWN__TRACE, 0, "Spawn", "Exit %s", __FUNCTION__);

	return spawn;
}

void ZoneServer::ProcessSpawnLocations()
{
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Enter %s", __FUNCTION__);

	map<int32,int32>* instNPCs = NULL;
	map<int32,int32>* instGroundSpawns = NULL;
	map<int32,int32>* instObjSpawns = NULL;
	map<int32,int32>* instWidgetSpawns = NULL;
	map<int32,int32>* instSignSpawns = NULL;

	if ( this->IsInstanceZone() )
	{
		LogWrite(SPAWN__DEBUG, 0, "Spawn", "Processing Instance Removed Spawns...");
		instNPCs = database.GetInstanceRemovedSpawns(this->GetInstanceID() , SPAWN_ENTRY_TYPE_NPC );
		instGroundSpawns = database.GetInstanceRemovedSpawns(this->GetInstanceID() , SPAWN_ENTRY_TYPE_GROUNDSPAWN );
		instObjSpawns = database.GetInstanceRemovedSpawns(this->GetInstanceID() , SPAWN_ENTRY_TYPE_OBJECT );
		instWidgetSpawns = database.GetInstanceRemovedSpawns(this->GetInstanceID() , SPAWN_ENTRY_TYPE_WIDGET );
		instSignSpawns = database.GetInstanceRemovedSpawns(this->GetInstanceID() , SPAWN_ENTRY_TYPE_SIGN );
	}
	else {
		instNPCs = database.GetPersistedSpawns(this->GetZoneID() , SPAWN_ENTRY_TYPE_NPC );
		instGroundSpawns = database.GetPersistedSpawns(this->GetZoneID() , SPAWN_ENTRY_TYPE_GROUNDSPAWN );
		instObjSpawns = database.GetPersistedSpawns(this->GetZoneID() , SPAWN_ENTRY_TYPE_OBJECT );
		instWidgetSpawns = database.GetPersistedSpawns(this->GetZoneID() , SPAWN_ENTRY_TYPE_WIDGET );
		instSignSpawns = database.GetPersistedSpawns(this->GetZoneID() , SPAWN_ENTRY_TYPE_SIGN );
	}

	map<int32, bool> processed_spawn_locations;
	map<int32, SpawnLocation*>::iterator itr;
	MSpawnLocationList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_location_list.begin(); itr != spawn_location_list.end(); itr++) {
		LogWrite(SPAWN__TRACE, 0, "Spawn", "while spawn_location_list itr (#%u)", spawn_location_list.size());

		if(itr->second && processed_spawn_locations.count(itr->second->placement_id) > 0) //if we processed one spawn in a spawn group, we processed them all for that group
			continue;

		if(itr->second && spawn_location_groups.count(itr->second->placement_id) > 0)
		{
			int32 group_id = CalculateSpawnGroup(itr->second);

			if(group_id)
			{
				LogWrite(SPAWN__TRACE, 0, "Spawn", "is group_id");
				set<int32>* associated_groups = GetAssociatedGroups(group_id);

				if(associated_groups)
				{
					LogWrite(SPAWN__TRACE, 0, "Spawn", "is associated_groups");
					vector<int32>* associated_locations = GetAssociatedLocations(associated_groups);

					if(associated_locations)
					{
						LogWrite(SPAWN__TRACE, 0, "Spawn", "is associated_locations");
						for(int32 i=0;i<associated_locations->size();i++)
						{
							LogWrite(SPAWN__DEBUG, 5, "Spawn", "Loading processed_spawn_locations...");
							processed_spawn_locations[associated_locations->at(i)] = true;
						}

						safe_delete(associated_locations);
					}
				}
			}
		}
		else
		{
			if ( this->IsInstanceZone() )
			{
				//LogWrite(SPAWN__DEBUG, 5, "Spawn", "ProcessInstanceSpawnLocation (%u)...", itr->second->placement_id);
				ProcessInstanceSpawnLocation(itr->second,instNPCs,instGroundSpawns,instObjSpawns,instWidgetSpawns,instSignSpawns);
			}
			else
			{
				//LogWrite(SPAWN__DEBUG, 5, "Spawn", "ProcessSpawnLocation (%u)...", itr->second->placement_id);
				ProcessSpawnLocation(itr->second,instNPCs,instGroundSpawns,instObjSpawns,instWidgetSpawns,instSignSpawns);
			}
		}
	}
	MSpawnLocationList.releasereadlock(__FUNCTION__, __LINE__);

	safe_delete(instNPCs);
	safe_delete(instGroundSpawns);
	safe_delete(instObjSpawns);
	safe_delete(instWidgetSpawns);
	safe_delete(instSignSpawns);

	LogWrite(SPAWN__TRACE, 0, "Spawn", "Exit %s", __FUNCTION__);
}

void ZoneServer::AddLoot(NPC* npc, Spawn* killer, GroupLootMethod loot_method, int8 item_rarity, int32 group_id){
	// this function is ran twice, first on spawn of mob, then at death of mob (gray mob check and no_drop_quest_completed_id check)

	// first we see if the skipping of gray mobs loot is enabled, then we move all non body drops
	if(killer)
	{
		npc->SetLootMethod(loot_method, item_rarity, group_id);
		int8 skip_loot_gray_mob_flag = rule_manager.GetZoneRule(GetZoneID(), R_Loot, SkipLootGrayMob)->GetInt8();
		if(skip_loot_gray_mob_flag) {
			Player* player = 0;
			if(killer->IsPlayer())
				player = (Player*)killer;
			else if(killer->IsPet()) {
				Spawn* owner = ((Entity*)killer)->GetOwner();
				if(owner->IsPlayer())
					player = (Player*)owner;
			}
			if(player) {
				int8 difficulty = player->GetArrowColor(npc->GetLevel());
				if(difficulty == ARROW_COLOR_GRAY) {
					npc->ClearNonBodyLoot();
				}
			}
		}
	}

	// check for starting loot of Spawn and death of Spawn loot (no_drop_quest_completed_id)
	vector<int32> loot_tables = GetSpawnLootList(npc->GetDatabaseID(), GetZoneID(), npc->GetLevel(), race_types_list.GetRaceType(npc->GetModelType()), npc);
	if(loot_tables.size() > 0){
		vector<LootDrop*>* loot_drops = 0;
		vector<LootDrop*>::iterator loot_drop_itr;
		LootTable* table = 0;
		vector<int32>::iterator loot_list_itr;
		float chancecoin = 0;
		float chancetable = 0;
		float chancedrop = 0;
		float chancetally = 0;
		float droptally = 0;
		// the following loop,loops through each table
		for(loot_list_itr = loot_tables.begin(); loot_list_itr != loot_tables.end(); loot_list_itr++){
			table = GetLootTable(*loot_list_itr);
			// if killer is assigned this is on-death, we already assigned coin
			if(!killer && table && table->maxcoin > 0){
				chancecoin = rand()%100;
				if(table->coin_probability >= chancecoin){
					if(table->maxcoin > table->mincoin)
						npc->AddLootCoins(table->mincoin + rand()%(table->maxcoin - table->mincoin));
				}
			}
			int numberchances = 1;

			//if (table->lootdrop_probability == 100){			}
		//else
			//chancetally += table->lootdrop_probability;
			int maxchance = 0;
			if (table) {
				maxchance = table->maxlootitems;
				for (numberchances; numberchances <= maxchance; numberchances++) {
					chancetable = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 100));
					//LogWrite(PLAYER__DEBUG, 0, "Player", "Table Chance: '%f'", chancetable);
					float droppercenttotal = 0;
					//--------------------------------------------------------------------------------------------------------------------------------------------------------------
					if (table->lootdrop_probability == 100 || table->lootdrop_probability >= chancetable) {

						//LogWrite(PLAYER__DEBUG, 0, "Player", "Probability:%f  Table Chance: '%f'", table->lootdrop_probability, chancetable);
						loot_drops = GetLootDrops(*loot_list_itr);
						if (loot_drops && loot_drops->size() > 0) {
							LootDrop* drop = 0;
							int16 count = 0;

							std::shuffle(loot_drops->begin(), loot_drops->end(), std::default_random_engine(Timer::GetCurrentTime2()));

							int16 IC = 0;
							for (loot_drop_itr = loot_drops->begin(); loot_drop_itr != loot_drops->end(); loot_drop_itr++) {
								drop = *loot_drop_itr;
								droppercenttotal += drop->probability;
							}

							
							int droplistsize = loot_drops->size();
							float chancedroptally = 0;
							bool breakIterMaxLoot = false;
							chancedrop = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 100));
							for (loot_drop_itr = loot_drops->begin(); loot_drop_itr != loot_drops->end(); loot_drop_itr++) {
								drop = *loot_drop_itr;

								// if no killer is provided, then we are instantiating the spawn loot, quest related loot should be added on death of the spawn to check against spawn/group members
								if(drop->no_drop_quest_completed_id && killer == nullptr)
									continue;
								else if(!drop->no_drop_quest_completed_id && killer) // skip since this doesn't have a quest id attached and we are doing after-math of death loot additions
									continue;
								else if(killer && drop->no_drop_quest_completed_id) // check if the player already completed quest related to item
								{
									Player* player = nullptr;
									if(killer->IsPlayer())
									{
										player = (Player*)killer;
										// player has already completed the quest
										if(player->HasQuestBeenCompleted(drop->no_drop_quest_completed_id) && !player->GetGroupMemberInfo())
										{
											LogWrite(PLAYER__DEBUG, 0, "Player", "%s: Player has completed quest %u, skipping loot item %u", npc->GetName(), drop->no_drop_quest_completed_id, drop->item_id);
											continue;
										}
										else if(player->GetGroupMemberInfo() && world.GetGroupManager()->HasGroupCompletedQuest(player->GetGroupMemberInfo()->group_id, drop->no_drop_quest_completed_id))
										{
											LogWrite(PLAYER__DEBUG, 0, "Player", "%s: Group %u has completed quest %u, skipping loot item %u", npc->GetName(), player->GetGroupMemberInfo()->group_id, drop->no_drop_quest_completed_id, drop->item_id);
											continue;
										}
									}
									else
									{
										LogWrite(PLAYER__DEBUG, 0, "Player", "%s: Killer is not a player, skipping loot item %u", npc->GetName(), drop->item_id);
										continue;
									}
								}

								if (npc->HasLootItemID(drop->item_id))
									continue;

								if (droppercenttotal >= 100)
									droppercenttotal = 100;
								chancedroptally += 100 / droppercenttotal * drop->probability;
								//chancedrop = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 100));
								//LogWrite(PLAYER__DEBUG, 0, "Player", "Loot drop: '%i'     Chance: %f    Prob tally:  %f  min: %f", drop, chancedrop, chancedroptally, chancedroptally - drop->probability);
								if ((chancedroptally == 100) || ((chancedroptally >= chancedrop) && (chancedroptally - (100 / droppercenttotal * drop->probability)) <= chancedrop)) {

									//LogWrite(PLAYER__DEBUG, 0, "Player", "Loot drop: '%i'     Chance: %f    Prob:  %f  We have a loot drop winner", drop, chancedrop, chancedroptally);
									count++;
									npc->AddLootItem(drop->item_id, drop->item_charges);
									//LogWrite(PLAYER__DEBUG, 0, "Player", "loot Count: '%i'",count);
									//LogWrite(MISC__TODO, 1, "TODO", "Auto-Equip new looted items\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);
									//if(drop->equip_item) 

								}
								// so many items on this table break out and cap it!
								if (table->maxlootitems > 0 && count >= table->maxlootitems) {
									breakIterMaxLoot = true;
									break;
								}
							}
							// hit our max item drop for this table already, break out of numberchances
							if(breakIterMaxLoot) {
								break;
							}
						}
					}
				}
			}
		}
	}
}

void ZoneServer::DeterminePosition(SpawnLocation* spawnlocation, Spawn* spawn){
	if(!spawn || !spawnlocation)
		return;

	int offset = 0;
	if(spawnlocation->x_offset > 0){
		//since rand() deals with integers only, we are going to divide by 1000 later so that we can use fractions of integers
		offset = (int)((spawnlocation->x_offset*1000)+1); 
		spawn->SetX(spawnlocation->x + ((float)(rand()%offset - rand()%offset))/1000);
	}
	else
		spawn->SetX(spawnlocation->x);
	if(spawnlocation->y_offset > 0){
		//since rand() deals with integers only, we are going to divide by 1000 later so that we can use fractions of integers
		offset = (int)((spawnlocation->y_offset*1000)+1); 
		spawn->SetY(spawnlocation->y + ((float)(rand()%offset - rand()%offset))/1000);
	}
	else
		spawn->SetY(spawnlocation->y, true, true);
	if(spawnlocation->z_offset > 0){
		//since rand() deals with integers only, we are going to divide by 1000 later so that we can use fractions of integers
		offset = (int)((spawnlocation->z_offset*1000)+1); 
		spawn->SetZ(spawnlocation->z + ((float)(rand()%offset - rand()%offset))/1000);
	}
	else
		spawn->SetZ(spawnlocation->z);
	spawn->SetHeading(spawnlocation->heading);
	spawn->SetPitch(spawnlocation->pitch);
	spawn->SetRoll(spawnlocation->roll);
	spawn->SetSpawnOrigX(spawn->GetX());
	spawn->SetSpawnOrigY(spawn->GetY());
	spawn->SetSpawnOrigZ(spawn->GetZ());
	spawn->SetSpawnOrigHeading(spawn->GetHeading());
	spawn->SetSpawnOrigPitch(spawnlocation->pitch);
	spawn->SetSpawnOrigRoll(spawnlocation->roll);
	spawn->SetLocation(spawnlocation->grid_id);
	spawn->SetSpawnLocationPlacementID(spawnlocation->placement_id);
}

NPC* ZoneServer::AddNPCSpawn(SpawnLocation* spawnlocation, SpawnEntry* spawnentry){
	LogWrite(SPAWN__TRACE, 1, "Spawn", "Enter %s", __FUNCTION__);
	NPC* npc = GetNewNPC(spawnentry->spawn_id);
	if(npc && !npc->IsOmittedByDBFlag()){
		InfoStruct* info = npc->GetInfoStruct();
		DeterminePosition(spawnlocation, npc);
		npc->SetDatabaseID(spawnentry->spawn_id);
		npc->SetSpawnLocationID(spawnentry->spawn_location_id);
		npc->SetSpawnEntryID(spawnentry->spawn_entry_id);
		npc->SetRespawnTime(spawnentry->respawn);
		npc->SetRespawnOffsetLow(spawnentry->respawn_offset_low);
		npc->SetRespawnOffsetHigh(spawnentry->respawn_offset_high);
		npc->SetDuplicateSpawn(spawnentry->duplicated_spawn);
		npc->SetExpireTime(spawnentry->expire_time);

		//devn00b add overrides for some spawns
		if(spawnentry->hp_override > 0){
			npc->SetHP(spawnentry->hp_override);
		}
		if(spawnentry->lvl_override > 0){
			npc->SetLevel(spawnentry->lvl_override);
		}
		if(spawnentry->mp_override > 0){
			npc->SetPower(spawnentry->mp_override); 
		}
		if(spawnentry->str_override > 0){
			info->set_str_base(spawnentry->str_override);
			info->set_str(spawnentry->str_override);
		}
		if(spawnentry->sta_override > 0){
			info->set_sta_base(spawnentry->sta_override);
			info->set_sta(spawnentry->sta_override);
		}
		if(spawnentry->wis_override > 0){
			info->set_wis_base(spawnentry->wis_override);
			info->set_wis(spawnentry->wis_override);
		}
		if(spawnentry->int_override > 0){
			info->set_intel_base(spawnentry->int_override);
			info->set_intel(spawnentry->int_override);
		}		
		if(spawnentry->agi_override > 0){
			info->set_agi_base(spawnentry->agi_override);
			info->set_agi(spawnentry->agi_override);
		}				
		if(spawnentry->heat_override > 0){
			info->set_heat_base(spawnentry->heat_override);
			info->set_heat(spawnentry->heat_override);
		}	
		if(spawnentry->cold_override > 0){
			info->set_cold_base(spawnentry->cold_override);
			info->set_cold(spawnentry->cold_override);
		}		
		if(spawnentry->magic_override > 0){
			info->set_magic_base(spawnentry->magic_override);
			info->set_magic(spawnentry->magic_override);
		}
		if(spawnentry->mental_override > 0){
			info->set_mental_base(spawnentry->mental_override);
			info->set_mental(spawnentry->mental_override);
		}
		if(spawnentry->divine_override > 0){
			info->set_divine_base(spawnentry->divine_override);
			info->set_divine(spawnentry->divine_override);
		}
		if(spawnentry->disease_override > 0){
			info->set_disease_base(spawnentry->disease_override);
			info->set_disease(spawnentry->disease_override);
		}
		if(spawnentry->poison_override > 0){
			info->set_poison_base(spawnentry->poison_override);
			info->set_poison(spawnentry->poison_override);
		}
		if(spawnentry->difficulty_override > 0){
			npc->SetDifficulty(spawnentry->difficulty_override, 1);
		}
		if (spawnentry->expire_time > 0)
			AddSpawnExpireTimer(npc, spawnentry->expire_time, spawnentry->expire_offset);
		AddLoot(npc);

		SetSpawnScript(spawnentry, npc);

		CallSpawnScript(npc, SPAWN_SCRIPT_PRESPAWN);

		AddSpawn(npc);
	}
	LogWrite(SPAWN__TRACE, 1, "Spawn", "Exit %s", __FUNCTION__);
	return npc;
}

vector<int32>* ZoneServer::GetAssociatedLocations(set<int32>* groups){
	vector<int32>* ret = 0;
	LogWrite(SPAWN__TRACE, 1, "Spawn", "Enter %s", __FUNCTION__);
	if(groups){
		int32 group_id = 0;		
		set<int32>::iterator group_itr;
		for (group_itr = groups->begin(); group_itr != groups->end(); group_itr++) {
			if(!ret)
				ret = new vector<int32>();
			group_id = *group_itr;

			MSpawnGroupLocations.readlock(__FUNCTION__, __LINE__);
			if(spawn_group_locations.count(group_id) > 0){
				map<int32, int32>::iterator itr;
				for (itr = spawn_group_locations[group_id]->begin(); itr != spawn_group_locations[group_id]->end(); itr++) {
					ret->push_back(itr->first);
				}
			}
			MSpawnGroupLocations.releasereadlock(__FUNCTION__, __LINE__);
		}
	}
	LogWrite(SPAWN__TRACE, 1, "Spawn", "Exit %s", __FUNCTION__);
	return ret;
}

set<int32>* ZoneServer::GetAssociatedGroups(int32 group_id) {
	set<int32>* ret = 0;
	MSpawnGroupAssociation.readlock(__FUNCTION__, __LINE__);
	LogWrite(SPAWN__TRACE, 1, "Spawn", "Enter %s", __FUNCTION__);
	if(spawn_group_associations.count(group_id) > 0)
		ret = spawn_group_associations[group_id];
	LogWrite(SPAWN__TRACE, 1, "Spawn", "Exit %s", __FUNCTION__);
	MSpawnGroupAssociation.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

map<int32, int32>* ZoneServer::GetSpawnLocationsByGroup(int32 group_id) {
	map<int32, int32>* ret = 0;

	MSpawnGroupLocations.readlock(__FUNCTION__, __LINE__);
	if(spawn_group_locations.count(group_id) > 0)
		ret = spawn_group_locations[group_id];
	MSpawnGroupLocations.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

list<int32>* ZoneServer::GetSpawnGroupsByLocation(int32 location_id){
	list<int32>* ret = 0;

	MSpawnLocationGroups.readlock(__FUNCTION__, __LINE__);
	if(spawn_location_groups.count(location_id) > 0)
		ret = spawn_location_groups[location_id];
	MSpawnLocationGroups.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

float ZoneServer::GetSpawnGroupChance(int32 group_id){
	float ret = -1;

	MSpawnGroupChances.readlock(__FUNCTION__, __LINE__);
	if(spawn_group_chances.count(group_id) > 0)
		ret = spawn_group_chances[group_id];
	MSpawnGroupChances.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

void ZoneServer::AddSpawnGroupChance(int32 group_id, float percent){
	MSpawnGroupChances.writelock(__FUNCTION__, __LINE__);
	spawn_group_chances[group_id] = percent;
	MSpawnGroupChances.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::AddSpawnGroupAssociation(int32 group_id1, int32 group_id2) {
	MSpawnGroupAssociation.writelock(__FUNCTION__, __LINE__);
	//Check if we already have containers for these group ids, if not create them
	if (spawn_group_associations.count(group_id1) == 0)
		spawn_group_associations[group_id1] = new set<int32>;
	if (spawn_group_associations.count(group_id2) == 0)
		spawn_group_associations[group_id2] = new set<int32>;

	//Associate groups 1 and 2 now
	set<int32>* group_1 = spawn_group_associations.find(group_id1)->second;
	set<int32>* group_2 = spawn_group_associations.find(group_id2)->second;
	group_1->insert(group_id2);
	group_2->insert(group_id1);
	
	//Associate the remaining groups together
	set<int32>::iterator itr;
	for (itr = group_1->begin(); itr != group_1->end(); itr++){
		group_2->insert(*itr);
		map<int32, set<int32>*>::iterator assoc_itr = spawn_group_associations.find(*itr);
		if (assoc_itr != spawn_group_associations.end())
			assoc_itr->second->insert(group_id2);
		else {
			set<int32>* new_set = new set<int32>;
			spawn_group_associations[*itr] = new_set;
			new_set->insert(group_id2);
		}
	}
	for (itr = group_2->begin(); itr != group_2->end(); itr++){
		group_1->insert(*itr);
		map<int32, set<int32>*>::iterator assoc_itr = spawn_group_associations.find(*itr);
		if (assoc_itr != spawn_group_associations.end())
			assoc_itr->second->insert(group_id1);
		else {
			set<int32>* new_set = new set<int32>;
			spawn_group_associations[*itr] = new_set;
			new_set->insert(group_id1);
		}
	}
	MSpawnGroupAssociation.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::AddSpawnGroupLocation(int32 group_id, int32 location_id, int32 spawn_location_id) {
	MSpawnGroupLocations.writelock(__FUNCTION__, __LINE__);
	if(spawn_group_locations.count(group_id) == 0)
		spawn_group_locations[group_id] =  new map<int32, int32>();
	(*spawn_group_locations[group_id])[location_id] = spawn_location_id;
	MSpawnGroupLocations.releasewritelock(__FUNCTION__, __LINE__);

	MSpawnLocationGroups.writelock(__FUNCTION__, __LINE__);
	if(spawn_location_groups.count(location_id) == 0)
		spawn_location_groups[location_id] = new list<int32>();
	spawn_location_groups[location_id]->push_back(group_id);
	MSpawnLocationGroups.releasewritelock(__FUNCTION__, __LINE__);

	MSpawnGroupAssociation.writelock(__FUNCTION__, __LINE__);
	if(spawn_group_associations.count(group_id) == 0)
		spawn_group_associations[group_id] = new set<int32>();
	spawn_group_associations[group_id]->insert(group_id);
	MSpawnGroupAssociation.releasewritelock(__FUNCTION__, __LINE__);
}

bool ZoneServer::CallSpawnScript(Spawn* npc, int8 type, Spawn* spawn, const char* message, bool is_door_open, sint32 input_value, sint32* return_value){

	LogWrite(SPAWN__TRACE, 0, "Spawn", "Enter %s", __FUNCTION__);
	if(!npc)
		return false;

	const char* script = npc->GetSpawnScript();
	if ( script == nullptr || strlen(script) < 1 )
	{
		if (!npc->IsPet() && npc->GetZone() != nullptr)
		{
			string tmpScript;
			tmpScript.append("SpawnScripts/");
			tmpScript.append(npc->GetZone()->GetZoneName());
			tmpScript.append("/");
			int count = 0;
			for (int s = 0; s < strlen(npc->GetName()); s++)
			{
				if (isalnum((unsigned char)npc->GetName()[s]))
				{
					tmpScript += npc->GetName()[s];
					count++;
				}
			}

			tmpScript.append(".lua");

			if (count < 1)
			{
				LogWrite(SPAWN__TRACE, 0, "Spawn", "Could not form script name %s..", __FUNCTION__);
			}
			else
			{
				struct stat buffer;
				bool fileExists = (stat(tmpScript.c_str(), &buffer) == 0);
				if (fileExists)
				{
					LogWrite(SPAWN__WARNING, 0, "Spawn", "No script file described in the database, overriding with SpawnScript at %s", (char*)tmpScript.c_str());
					npc->SetSpawnScript(tmpScript);
					script = npc->GetSpawnScript();
				}
			}
		}
	}

	bool result = false;
	if(lua_interface && script){
		result = true; // default to true, if we don't match a switch case, return false in default case
		switch(type){
			case SPAWN_SCRIPT_SPAWN:{
				lua_interface->RunSpawnScript(script, "spawn", npc);
				break;
									}
			case SPAWN_SCRIPT_RESPAWN:{
				lua_interface->RunSpawnScript(script, "respawn", npc);
				break;
									  }
			case SPAWN_SCRIPT_ATTACKED:{
				lua_interface->RunSpawnScript(script, "attacked", npc, spawn);
				break;
									   }
			case SPAWN_SCRIPT_TARGETED:{
				lua_interface->RunSpawnScript(script, "targeted", npc, spawn);
				break;
									   }
			case SPAWN_SCRIPT_HAILED:{
				result = lua_interface->RunSpawnScript(script, "hailed", npc, spawn);
				break;
									 }
			case SPAWN_SCRIPT_HAILED_BUSY:{
				lua_interface->RunSpawnScript(script, "hailed_busy", npc, spawn);
				break;
										  }
			case SPAWN_SCRIPT_DEATH:{
				lua_interface->RunSpawnScript(script, "death", npc, spawn);
				break;
									}
			case SPAWN_SCRIPT_KILLED:{
				lua_interface->RunSpawnScript(script, "killed", npc, spawn);
				break;
									 }
			case SPAWN_SCRIPT_AGGRO:{
				lua_interface->RunSpawnScript(script, "aggro", npc, spawn);
				break;
									}
			case SPAWN_SCRIPT_HEALTHCHANGED:{
				result = lua_interface->RunSpawnScript(script, "healthchanged", npc, spawn, 0, false, input_value, return_value);
				break;
											}
			case SPAWN_SCRIPT_RANDOMCHAT:{
				lua_interface->RunSpawnScript(script, "randomchat", npc, 0, message);
				break;
										 }
			case SPAWN_SCRIPT_CUSTOM:
			case SPAWN_SCRIPT_TIMER:
			case SPAWN_SCRIPT_CONVERSATION:{
				lua_interface->RunSpawnScript(script, message, npc, spawn);
				break;
										   }
			case SPAWN_SCRIPT_CASTED_ON: {
				lua_interface->RunSpawnScript(script, "casted_on", npc, spawn, message);
				break;
										 }
			case SPAWN_SCRIPT_AUTO_ATTACK_TICK: {
				lua_interface->RunSpawnScript(script, "auto_attack_tick", npc, spawn);
				break;
												 }
			case SPAWN_SCRIPT_COMBAT_RESET: {
				lua_interface->RunSpawnScript(script, "CombatReset", npc);
				break;
											}
			case SPAWN_SCRIPT_GROUP_DEAD: {
				lua_interface->RunSpawnScript(script, "group_dead", npc, spawn);
				break;
										  }
			case SPAWN_SCRIPT_HEAR_SAY: {
				lua_interface->RunSpawnScript(script, "hear_say", npc, spawn, message);
				break;
			}
			case SPAWN_SCRIPT_PRESPAWN: {
				lua_interface->RunSpawnScript(script, "prespawn", npc);
				break;
			}
			case SPAWN_SCRIPT_USEDOOR: {
				result = lua_interface->RunSpawnScript(script, "usedoor", npc, spawn, "", is_door_open);
				break;
			}
			case SPAWN_SCRIPT_BOARD: {
				result = lua_interface->RunSpawnScript(script, "board", npc, spawn);
				break;
			}
			case SPAWN_SCRIPT_DEBOARD: {
				result = lua_interface->RunSpawnScript(script, "deboard", npc, spawn);
				break;
			}
			default:
			{
				result = false;
				break;
			}
		}
	}
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Exit %s", __FUNCTION__);

	return result;
}

void ZoneServer::DeleteTransporters() {
	MTransportLocations.writelock(__FUNCTION__, __LINE__);
	transporter_locations.clear(); //world takes care of actually deleting the data
	MTransportLocations.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::ReloadTransporters(){
	MutexList<LocationTransportDestination*>* locations = GetLocationTransporters(GetZoneID());
	if(locations){
		MutexList<LocationTransportDestination*>::iterator itr = locations->begin();
		while(itr.Next())
			AddTransporter(itr->value);
	}
}

void ZoneServer::CheckTransporters(Client* client) {
	MTransportLocations.readlock(__FUNCTION__, __LINE__);
	if(transporter_locations.size() > 0){
		LocationTransportDestination* loc = 0;
		list<LocationTransportDestination*>::iterator itr;
		for (itr = transporter_locations.begin(); itr != transporter_locations.end(); itr++) {
			loc = *itr;
			if(client->GetPlayer()->GetDistance(loc->trigger_x, loc->trigger_y, loc->trigger_z) <= loc->trigger_radius){
				if(loc->destination_zone_id == 0 || loc->destination_zone_id == GetZoneID()){
					EQ2Packet* packet = client->GetPlayer()->Move(loc->destination_x, loc->destination_y, loc->destination_z, client->GetVersion());
					if(packet)
						client->QueuePacket(packet);
				}
				else{
					ZoneChangeDetails zone_details;
					bool foundZone = zone_list.GetZone(&zone_details, loc->destination_zone_id);
					if(foundZone){
						client->GetPlayer()->SetX(loc->destination_x);
						client->GetPlayer()->SetY(loc->destination_y);
						client->GetPlayer()->SetZ(loc->destination_z);
						client->GetPlayer()->SetHeading(loc->destination_heading);
						client->Zone(&zone_details, (ZoneServer*)zone_details.zonePtr);
					}
				}
				break;
			}
		}
	}
	MTransportLocations.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::AddTransporter(LocationTransportDestination* loc) {
	MTransportLocations.writelock(__FUNCTION__, __LINE__);
	transporter_locations.push_back(loc);
	MTransportLocations.releasewritelock(__FUNCTION__, __LINE__);
}

Sign* ZoneServer::AddSignSpawn(SpawnLocation* spawnlocation, SpawnEntry* spawnentry){
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Enter %s", __FUNCTION__);
	Sign* sign = GetNewSign(spawnentry->spawn_id);
	if(sign && !sign->IsOmittedByDBFlag()){
		DeterminePosition(spawnlocation, sign);
		sign->SetDatabaseID(spawnentry->spawn_id);
		sign->SetSpawnLocationID(spawnentry->spawn_location_id);
		sign->SetSpawnEntryID(spawnentry->spawn_entry_id);
		sign->SetRespawnTime(spawnentry->respawn);
		sign->SetRespawnOffsetLow(spawnentry->respawn_offset_low);
		sign->SetRespawnOffsetHigh(spawnentry->respawn_offset_high);
		sign->SetDuplicateSpawn(spawnentry->duplicated_spawn);
		sign->SetExpireTime(spawnentry->expire_time);
		if (spawnentry->expire_time > 0)
			AddSpawnExpireTimer(sign, spawnentry->expire_time, spawnentry->expire_offset);

		SetSpawnScript(spawnentry, sign);

		CallSpawnScript(sign, SPAWN_SCRIPT_PRESPAWN);

		AddSpawn(sign);
	}
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Exit %s", __FUNCTION__);
	return sign;
}

Widget* ZoneServer::AddWidgetSpawn(SpawnLocation* spawnlocation, SpawnEntry* spawnentry){
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Enter %s", __FUNCTION__);
	Widget* widget = GetNewWidget(spawnentry->spawn_id);
	if(widget && !widget->IsOmittedByDBFlag()){
		DeterminePosition(spawnlocation, widget);
		widget->SetDatabaseID(spawnentry->spawn_id);
		widget->SetSpawnLocationID(spawnentry->spawn_location_id);
		widget->SetSpawnEntryID(spawnentry->spawn_entry_id);
		if(!widget->GetIncludeLocation()){
			widget->SetX(widget->GetWidgetX());
			if(widget->GetCloseY() != 0)
				widget->SetY(widget->GetCloseY());
			widget->SetZ(widget->GetWidgetZ());
		}
		widget->SetRespawnTime(spawnentry->respawn);
		widget->SetRespawnOffsetLow(spawnentry->respawn_offset_low);
		widget->SetRespawnOffsetHigh(spawnentry->respawn_offset_high);
		widget->SetDuplicateSpawn(spawnentry->duplicated_spawn);
		widget->SetExpireTime(spawnentry->expire_time);
		widget->SetSpawnOrigHeading(widget->GetHeading());
		if (spawnentry->expire_time > 0)
			AddSpawnExpireTimer(widget, spawnentry->expire_time, spawnentry->expire_offset);

		SetSpawnScript(spawnentry, widget);

		CallSpawnScript(widget, SPAWN_SCRIPT_PRESPAWN);

		AddSpawn(widget);
	}
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Exit %s", __FUNCTION__);
	return widget;
}

Object* ZoneServer::AddObjectSpawn(SpawnLocation* spawnlocation, SpawnEntry* spawnentry){
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Enter %s", __FUNCTION__);
	Object* object = GetNewObject(spawnentry->spawn_id);
	if(object && !object->IsOmittedByDBFlag()){
		DeterminePosition(spawnlocation, object);
		object->SetDatabaseID(spawnentry->spawn_id);
		object->SetSpawnLocationID(spawnentry->spawn_location_id);
		object->SetSpawnEntryID(spawnentry->spawn_entry_id);
		object->SetRespawnTime(spawnentry->respawn);
		object->SetRespawnOffsetLow(spawnentry->respawn_offset_low);
		object->SetRespawnOffsetHigh(spawnentry->respawn_offset_high);
		object->SetDuplicateSpawn(spawnentry->duplicated_spawn);
		object->SetExpireTime(spawnentry->expire_time);
		if (spawnentry->expire_time > 0)
			AddSpawnExpireTimer(object, spawnentry->expire_time, spawnentry->expire_offset);

		SetSpawnScript(spawnentry, object);

		CallSpawnScript(object, SPAWN_SCRIPT_PRESPAWN);

		AddSpawn(object);
	}
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Exit %s", __FUNCTION__);
	return object;
}

GroundSpawn* ZoneServer::AddGroundSpawn(SpawnLocation* spawnlocation, SpawnEntry* spawnentry){
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Enter %s", __FUNCTION__);
	GroundSpawn* spawn = GetNewGroundSpawn(spawnentry->spawn_id);
	if(spawn && !spawn->IsOmittedByDBFlag()){
		DeterminePosition(spawnlocation, spawn);
		spawn->SetDatabaseID(spawnentry->spawn_id);
		spawn->SetSpawnLocationID(spawnentry->spawn_location_id);
		spawn->SetSpawnEntryID(spawnentry->spawn_entry_id);
		spawn->SetRespawnTime(spawnentry->respawn);
		spawn->SetRespawnOffsetLow(spawnentry->respawn_offset_low);
		spawn->SetRespawnOffsetHigh(spawnentry->respawn_offset_high);
		spawn->SetDuplicateSpawn(spawnentry->duplicated_spawn);
		spawn->SetExpireTime(spawnentry->expire_time);
		
		if(spawn->GetRandomizeHeading()) {
			float rand_heading = MakeRandomFloat(0.0f, 360.0f);
			spawn->SetHeading(rand_heading);
		}
		else {
			spawn->SetHeading(spawnlocation->heading);
		}
		
		if (spawnentry->expire_time > 0)
			AddSpawnExpireTimer(spawn, spawnentry->expire_time, spawnentry->expire_offset);

		SetSpawnScript(spawnentry, spawn);

		CallSpawnScript(spawn, SPAWN_SCRIPT_PRESPAWN);

		AddSpawn(spawn);
	}
	LogWrite(SPAWN__TRACE, 0, "Spawn", "Exit %s", __FUNCTION__);
	return spawn;
}

void ZoneServer::AddSpawn(Spawn* spawn) {
	if(!spawn->IsPlayer()) {
		spawn->SetZone(this); // we already set it on loadCharacter
	}
	else {
		pNumPlayers++;
	}
	MIgnoredWidgets.lock_shared();
	std::map<int32, bool>::iterator itr;
	for(itr = ignored_widgets.begin(); itr != ignored_widgets.end(); itr++) {
		spawn->AddIgnoredWidget(itr->first);
	}
	MIgnoredWidgets.unlock_shared();
	spawn->position_changed = false;
	spawn->info_changed = false;
	spawn->vis_changed = false;
	spawn->changed = false;
	
	// Write locking the spawn list here will cause deadlocks, so instead add it to a temp list that the
	// main spawn thread will put into the spawn_list when ever it has a chance.
	MPendingSpawnListAdd.writelock(__FUNCTION__, __LINE__);
	pending_spawn_list_add.push_back(spawn);
	MPendingSpawnListAdd.releasewritelock(__FUNCTION__, __LINE__);
	
	spawn_range.Trigger();

	if (GetInstanceType() == PERSONAL_HOUSE_INSTANCE && spawn->IsObject())
	{
		spawn->AddSecondaryEntityCommand("Examine", 20, "house_spawn_examine", "", 0, 0);
		spawn->AddSecondaryEntityCommand("Move", 20, "move_item", "", 0, 0);
		spawn->AddSecondaryEntityCommand("Pack in Moving Crate", 20, "house_spawn_pack_in_moving_crate", "", 0, 0);
		spawn->AddSecondaryEntityCommand("Pick Up", 20, "pickup", "", 0, 0);
		spawn->SetShowCommandIcon(1);
	}

	if(spawn->IsNPC())
		AddEnemyList((NPC*)spawn);
	if(spawn->IsPlayer() && ((Player*)spawn)->GetGroupMemberInfo())
		spawn->SendGroupUpdate();
	if (spawn->IsPlayer()) {
		((Player*)spawn)->GetInfoStruct()->set_rain(rain);
		((Player*)spawn)->SetCharSheetChanged(true);
	}
	
	if (movementMgr != nullptr && spawn->IsEntity()) {
		movementMgr->AddMob((Entity*)spawn);
	}

	AddSpawnProximities(spawn);

	AddSpawnToGrid(spawn, spawn->GetLocation());
	spawn->SetAddedToWorldTimestamp(Timer::GetCurrentTime2());
}

void ZoneServer::AddClient(Client* client){
	MClientList.writelock(__FUNCTION__, __LINE__);
	lifetime_client_count++;
	DecrementIncomingClients();
	clients.push_back(client);
	MClientList.releasewritelock(__FUNCTION__, __LINE__);

	connected_clients.Add(client);
}

void ZoneServer::RemoveClient(Client* client)
{
	Guild *guild;

	bool dismissPets = false;
	if(client)
	{			
		if (client->GetPlayer()) 
			client_list.RemovePlayerFromInvisHistory(client->GetPlayer()->GetID());

		LogWrite(ZONE__DEBUG, 0, "Zone", "Sending login equipment appearance updates...");
		loginserver.SendImmediateEquipmentUpdatesForChar(client->GetPlayer()->GetCharacterID());

		if (!client->IsZoning()) 
		{			
			client->SaveSpells();
			
			client->GetPlayer()->DeleteSpellEffects(true);
			
			if ((guild = client->GetPlayer()->GetGuild()) != NULL)
				guild->GuildMemberLogoff(client->GetPlayer());

			chat.LeaveAllChannels(client);
		}

		if(!zoneShuttingDown && !client->IsZoning())
		{
			world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);
			GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
			int32 group_id = 0;
			if (gmi) {
				group_id = gmi->group_id;
			}
			world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
			if (group_id) {
				int32 size = world.GetGroupManager()->GetGroupSize(group_id);
				if (size > 1) {
					bool send_left_message = size > 2;
					// removegroupmember can delete the gmi, so make sure we still have a group_id after
					world.GetGroupManager()->RemoveGroupMember(group_id, client->GetPlayer());
					if (send_left_message)
						world.GetGroupManager()->GroupMessage(group_id, "%s has left the group.", client->GetPlayer()->GetName());
				}
			}
			if( (client->GetPlayer()->GetActivityStatus() & ACTIVITY_STATUS_LINKDEAD) > 0)
			{
				LogWrite(ZONE__DEBUG, 0, "Zone", "Removing client '%s' (%u) due to LD/Exit...", client->GetPlayer()->GetName(), client->GetPlayer()->GetCharacterID());
			}
			else
			{
				LogWrite(ZONE__DEBUG, 0, "Zone", "Removing client '%s' (%u) due to Camp/Quit...", client->GetPlayer()->GetName(), client->GetPlayer()->GetCharacterID());
			}
				
				dismissPets = true;
			//}
		}
		else
		{
			LogWrite(ZONE__DEBUG, 0, "Zone", "Removing client '%s' (%u) due to some client zoning...", client->GetPlayer()->GetName(), client->GetPlayer()->GetCharacterID());
		}

		map<int32, int32>::iterator itr;
		for (itr = client->GetPlayer()->SpawnedBots.begin(); itr != client->GetPlayer()->SpawnedBots.end(); itr++) {
			Spawn* spawn = GetSpawnByID(itr->second);
			if (spawn && spawn->IsBot()) {
				((Entity*)spawn)->SetOwner(nullptr);
				((Bot*)spawn)->Camp();
			}
		}
		
		if(dismissPets) {
				((Entity*)client->GetPlayer())->DismissAllPets();
		}
		
		MClientList.writelock(__FUNCTION__, __LINE__);
		LogWrite(ZONE__DEBUG, 0, "Zone", "Calling clients.Remove(client)...");
		
		std::vector<Client*>::iterator itr2 = find(clients.begin(), clients.end(), client);
		if (itr2 != clients.end())
			clients.erase(itr2);
		MClientList.releasewritelock(__FUNCTION__, __LINE__);

		LogWrite(ZONE__INFO, 0, "Zone", "Scheduling client '%s' for removal.", client->GetPlayer()->GetName());
		database.ToggleCharacterOnline(client, 0);
		
		RemoveSpawn(client->GetPlayer(), false, true, true, true, true);
		
		int32 DisconnectClientTimer = rule_manager.GetGlobalRule(R_World, RemoveDisconnectedClientsTimer)->GetInt32();
		
		if(client->GetPlayer()->GetClient() == client)
			client->GetPlayer()->SetClient(nullptr);
			
		connected_clients.Remove(client, true, DisconnectClientTimer); // changed from a hardcoded 30000 (30 sec) to the DisconnectClientTimer rule
	}
}

void ZoneServer::RemoveClientImmediately(Client* client) {
	Guild *guild;

	if(client) 
	{
		if(client->GetPlayer()) {
			if((client->GetPlayer()->GetActivityStatus() & ACTIVITY_STATUS_LINKDEAD) > 0) {
				client->GetPlayer()->SetActivityStatus(client->GetPlayer()->GetActivityStatus() - ACTIVITY_STATUS_LINKDEAD);
			}
			if ((client->GetPlayer()->GetActivityStatus() & ACTIVITY_STATUS_CAMPING) == 0) {
				client->GetPlayer()->SetActivityStatus(client->GetPlayer()->GetActivityStatus() + ACTIVITY_STATUS_CAMPING);
			}
			client->Disconnect();
		}
		MClientList.writelock(__FUNCTION__, __LINE__);
		std::vector<Client*>::iterator itr = find(clients.begin(), clients.end(), client);
		if (itr != clients.end())
			clients.erase(itr);
		MClientList.releasewritelock(__FUNCTION__, __LINE__);
			//clients.Remove(client, true);
	}
}

void ZoneServer::ClientProcess(bool ignore_shutdown_timer)
{
	if(!ignore_shutdown_timer && connected_clients.size(true) == 0)
	{
		MIncomingClients.readlock(__FUNCTION__, __LINE__);
		bool shutdownDelayCheck = shutdownDelayTimer.Check();
		if((!AlwaysLoaded() && !shutdownTimer.Enabled()) || (!AlwaysLoaded() && shutdownDelayCheck))
		{
			if(incoming_clients && !shutdownDelayTimer.Enabled()) {
				LogWrite(ZONE__INFO, 0, "Zone", "Incoming clients (%u) expected for %s, delaying shutdown timer...", incoming_clients, GetZoneName());
				int32 timerDelay = rule_manager.GetGlobalRule(R_Zone, ShutdownDelayTimer)->GetInt32();

				if(timerDelay < 10) {
					LogWrite(ZONE__INFO, 0, "Zone", "Overriding %s shutdown delay timer as other clients are incoming, value %u too short, setting to 10...", GetZoneName(), timerDelay);
					timerDelay = 10;
				}
				shutdownDelayTimer.Start(timerDelay, true);
			}
			else if(!incoming_clients || shutdownDelayCheck) {
				if(!shutdownTimer.Enabled()) {
					LogWrite(ZONE__INFO, 0, "Zone", "Starting zone shutdown timer for %s...", GetZoneName());
					shutdownTimer.Start();
				}
				else {
					LogWrite(ZONE__INFO, 0, "Zone", "zone shutdown timer for %s has %u remaining...", GetZoneName(), shutdownTimer.GetRemainingTime());
				}
			}
		}
		else if(AlwaysLoaded() && shutdownTimer.Enabled()) {
			shutdownTimer.Disable();
		}
		MIncomingClients.releasereadlock(__FUNCTION__, __LINE__);
		return;
	}

	shutdownTimer.Disable();
	shutdownDelayTimer.Disable();	
	Client* client = 0;		
	MutexList<Client*>::iterator iterator = connected_clients.begin();

	while(iterator.Next())
	{
		client = iterator->value;
#ifndef NO_CATCH
		try
		{
#endif
			if(zoneShuttingDown || !client->Process(true))
			{
				if(!zoneShuttingDown && !client->IsZoning())
				{
					// avoid spam of messages while we await linkdead to complete
					if(!client->IsLinkdeadTimerEnabled()) {
						bool camping = (client->GetPlayer()->GetActivityStatus() & ACTIVITY_STATUS_CAMPING);
						LogWrite(ZONE__DEBUG, 0, "Zone", "Client is disconnecting in %s (camping = %s)", __FUNCTION__, camping ? "true" : "false");
						if(!camping) {
							client->setConnection(nullptr);
						}
					}
					
					if((client->GetPlayer()->GetActivityStatus() & ACTIVITY_STATUS_LINKDEAD) > 0) {
						client->StartLinkdeadTimer();
					}
					else if( (client->GetPlayer()->GetActivityStatus() & ACTIVITY_STATUS_CAMPING) == 0 )
					{
						 //only set LD flag if we're disconnecting but not camping/quitting
						client->GetPlayer()->SetActivityStatus(client->GetPlayer()->GetActivityStatus() + ACTIVITY_STATUS_LINKDEAD);
						client->StartLinkdeadTimer();
					}
					else {
						// camp timer completed, remove client
						RemoveClient(client);
						client->Disconnect();
					}
				}
				else {
					// force boot all players or clients zoning
					RemoveClient(client);
					client->Disconnect();
				}
			}
#ifndef NO_CATCH
		}
		catch(...)
		{
			LogWrite(ZONE__ERROR, 0, "Zone", "Exception caught when in ZoneServer::ClientProcess() for zone '%s'!\n%s, %i", GetZoneName(), __FUNCTION__, __LINE__);
			try{
				bool isLinkdead = false;
				if(!client->IsZoning())
				{
					if( (client->GetPlayer()->GetActivityStatus() & ACTIVITY_STATUS_CAMPING) == 0 )
					{
						client->GetPlayer()->SetActivityStatus(client->GetPlayer()->GetActivityStatus() + ACTIVITY_STATUS_LINKDEAD);
						client->StartLinkdeadTimer();
						isLinkdead = true;
						if(client->GetPlayer()->GetGroupMemberInfo())
							world.GetGroupManager()->GroupMessage(client->GetPlayer()->GetGroupMemberInfo()->group_id, "%s has gone Linkdead.", client->GetPlayer()->GetName());
					}
				}
				
				if(!isLinkdead) {
					RemoveClient(client);
					client->Disconnect();
				}
			}
			catch(...){
				LogWrite(ZONE__ERROR, 0, "Zone", "Exception caught when in ZoneServer::ClientProcess(), second try\n%s, %i", __FUNCTION__, __LINE__);
			}
		}
#endif
	}
}

void ZoneServer::SimpleMessage(int8 type, const char* message, Spawn* from, float distance, bool send_to_sender){
	Client* client = 0;
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(from && client && client->IsConnected() && (send_to_sender || from != client->GetPlayer()) && from->GetDistance(client->GetPlayer()) <= distance){
			client->SimpleMessage(type, message);
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::HandleChatMessage(Client* client, Spawn* from, const char* to, int16 channel, const char* message, float distance, const char* channel_name, bool show_bubble, int32 language) {	
	if ((!distance || from->GetDistance(client->GetPlayer()) <= distance) && (!from || !client->GetPlayer()->IsIgnored(from->GetName()))) {
		PacketStruct* packet = configReader.getStruct("WS_HearChat", client->GetVersion());
		if (packet) {
			if (from)
				packet->setMediumStringByName("from", from->GetName());
			if (client->GetPlayer() != from)
				packet->setMediumStringByName("to", client->GetPlayer()->GetName());
			int8 clientchannel = client->GetMessageChannelColor(channel);
			packet->setDataByName("channel", client->GetMessageChannelColor(channel));
			if (from && ((from == client->GetPlayer()) || (client->GetPlayer()->WasSentSpawn(from->GetID()))))
				packet->setDataByName("from_spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(from));
			else
				packet->setDataByName("from_spawn_id", 0xFFFFFFFF);
			packet->setDataByName("to_spawn_id", 0xFFFFFFFF);
			packet->setMediumStringByName("message", message);
			packet->setDataByName("language", language);

			bool hasLanguage = client->GetPlayer()->HasLanguage(language);
			if (language > 0 && !hasLanguage)
				packet->setDataByName("understood", 0);
			else
				packet->setDataByName("understood", 1);

			show_bubble == true ? packet->setDataByName("show_bubble", 1) : packet->setDataByName("show_bubble", 0);
			if (channel_name)
				packet->setMediumStringByName("channel_name", channel_name);
			EQ2Packet* outapp = packet->serialize();
			//DumpPacket(outapp);
			client->QueuePacket(outapp);
			safe_delete(packet);
		}
	}
}

void ZoneServer::HandleChatMessage(Client* client, std::string fromName, const char* to, int16 channel, const char* message, float distance, const char* channel_name, int32 language) {	
	if (!client->GetPlayer()->IsIgnored(fromName.c_str())) {
		PacketStruct* packet = configReader.getStruct("WS_HearChat", client->GetVersion());
		if (packet) {
			packet->setMediumStringByName("from", fromName.c_str());
			
			int8 clientchannel = client->GetMessageChannelColor(channel);
			packet->setDataByName("channel", client->GetMessageChannelColor(channel));
			packet->setDataByName("from_spawn_id", 0xFFFFFFFF);
			packet->setDataByName("to_spawn_id", 0xFFFFFFFF);
			packet->setMediumStringByName("message", message);
			packet->setDataByName("language", language);

			bool hasLanguage = client->GetPlayer()->HasLanguage(language);
			if (language > 0 && !hasLanguage)
				packet->setDataByName("understood", 0);
			else
				packet->setDataByName("understood", 1);

			packet->setDataByName("show_bubble", 0);
			if (channel_name)
				packet->setMediumStringByName("channel_name", channel_name);
			EQ2Packet* outapp = packet->serialize();
			//DumpPacket(outapp);
			client->QueuePacket(outapp);
			safe_delete(packet);
		}
	}
}

void ZoneServer::HandleChatMessage(Spawn* from, const char* to, int16 channel, const char* message, float distance, const char* channel_name, bool show_bubble, int32 language){
	vector<Client*>::iterator client_itr;
	Client* client = 0;
	std::string tokenedMsg = std::string(message);
	SpellProcess::ReplaceEffectTokens(tokenedMsg, from, from->GetTarget());

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(client && client->IsConnected())
			HandleChatMessage(client, from, to, channel, tokenedMsg.c_str(), distance, channel_name, show_bubble, language);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::HandleChatMessage(std::string fromName, const char* to, int16 channel, const char* message, float distance, const char* channel_name, int32 language){
	vector<Client*>::iterator client_itr;
	Client* client = 0;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(client && client->IsConnected())
			HandleChatMessage(client, fromName, to, channel, message, distance, channel_name, language);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::HandleBroadcast(const char* message) {
	vector<Client*>::iterator client_itr;
	Client* client = 0;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(client && client->IsConnected())
			client->SimpleMessage(CHANNEL_BROADCAST, message);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::HandleAnnouncement(const char* message) {
	vector<Client*>::iterator client_itr;
	Client* client = 0;
	int32 words = ::CountWordsInString(message);
	if (words < 5)
		words = 5;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(client && client->IsConnected()) {
			client->SimpleMessage(CHANNEL_BROADCAST, message);
			client->SendPopupMessage(10, message, "ui_harvest_normal", words, 0xFF, 0xFF, 0x00);
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::SendTimeUpdate(Client* client){
	if(client){
		PacketStruct* packet = world.GetWorldTime(client->GetVersion());
		if(packet){
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		} 
	}
}

void ZoneServer::SendTimeUpdateToAllClients(){
	Client* client = 0;
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(client && client->IsConnected())
			SendTimeUpdate(client);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::UpdateVitality(float amount){
	Client* client = 0;
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(client && client->GetPlayer()->GetInfoStruct()->get_xp_vitality() < 100){
			if((client->GetPlayer()->GetInfoStruct()->get_xp_vitality() + amount) > 100)
				client->GetPlayer()->GetInfoStruct()->set_xp_vitality(100);
			else
				client->GetPlayer()->GetInfoStruct()->add_xp_vitality(amount);
			client->GetPlayer()->SetCharSheetChanged(true);
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::SendSpawn(Spawn* spawn, Client* client){
	EQ2Packet* outapp = spawn->serialize(client->GetPlayer(), client->GetVersion());

	if(!client->GetPlayer()->IsSendingSpawn(spawn->GetID())) {
		safe_delete(outapp);
	}
	else {
		LogWrite(ZONE__DEBUG, 7, "Zone", "%s: Processing SendSpawn for spawn index %u (%s)...", client->GetPlayer()->GetName(), client->GetPlayer()->GetIndexForSpawn(spawn), spawn->GetName());
		if(outapp)
			client->QueuePacket(outapp, true);

		client->GetPlayer()->SetSpawnSentState(spawn, SpawnState::SPAWN_STATE_SENT_WAIT);
	}

	/*
	vis flags:
	2 = show icon
	4 = targetable
	16 = show name
	32 = show level/border
	activity_status:
	4 - linkdead
	8 - camping
	16 - LFG
	32 - LFW
	2048 - mentoring
	4096 - displays shield
	8192 - immunity gained
	16384 - immunity remaining
	attackable_status
	1 - no_hp_bar
	4 - not attackable
	npc_con
	-4 = scowls
	-3 = threatening
	-2 = dubiously
	-1 = apprehensively
	0 = indifferent
	1 = amiably
	2 = kindly
	3 = warmly
	4 = ally
	quest_flag
	1 = new quest
	2 = update and new quest
	3 = update
	*/
	if(spawn->IsEntity() && spawn->HasTrapTriggered())
		client->QueueStateCommand(client->GetPlayer()->GetIDWithPlayerSpawn(spawn), spawn->GetTrapState());
}

Client*	ZoneServer::GetClientByName(char* name) {
	Client* ret = 0;
	vector<Client*>::iterator itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		if ((*itr)->GetPlayer()) {
			if (strncmp((*itr)->GetPlayer()->GetName(), name, strlen(name)) == 0) {
				ret = *itr;
				break;
			}
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

Client*	ZoneServer::GetClientByCharID(int32 charid) {
	Client* ret = 0;
	vector<Client*>::iterator itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		if ((*itr)->GetCharacterID() == charid) {
			ret = *itr;
			break;
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

void ZoneServer::AddMovementNPC(Spawn* spawn){
	if (spawn)
		movement_spawns.Put(spawn->GetID(), 1);
}

void ZoneServer::RemoveMovementNPC(Spawn* spawn){
	if (spawn)
		remove_movement_spawns.Add(spawn->GetID());
}

void ZoneServer::PlayFlavor(Client* client, Spawn* spawn, const char* mp3, const char* text, const char* emote, int32 key1, int32 key2, int8 language){
	if(!client || !spawn)
		return;

	PacketStruct* packet = configReader.getStruct("WS_PlayFlavor", client->GetVersion());
	if(packet){
		packet->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(spawn));
		packet->setDataByName("unknown1", 0xFFFFFFFF);
		packet->setDataByName("unknown5", 1, 1);
		packet->setDataByName("unknown5", 1, 6);
		if(mp3){
			packet->setMediumStringByName("mp3", mp3);
			packet->setDataByName("key", key1);
			packet->setDataByName("key", key2, 1);
		}
		packet->setMediumStringByName("name", spawn->GetName());
		if(text)
			packet->setMediumStringByName("text", text);
		if(emote) {
			if(client->GetVersion() > 561) {
				packet->setMediumStringByName("emote", emote);
			}
			else {
				HandleEmote(spawn, std::string(emote));
			}
		}
		if (language != 0)
			packet->setDataByName("language", language);

		//We should probably add Common = language id 0 or 0xFF so admins can customize more..
		if (language == 0 || client->GetPlayer()->HasLanguage(language))
			packet->setDataByName("understood", 1);

		EQ2Packet* app = packet->serialize();
		//DumpPacket(app);
		client->QueuePacket(app);
		safe_delete(packet);
	}
}

void ZoneServer::PlayVoice(Client* client, Spawn* spawn, const char* mp3, int32 key1, int32 key2){
	if(!client || !spawn)
		return;

	PacketStruct* packet = configReader.getStruct("WS_PlayVoice", client->GetVersion());
	if(packet){
		packet->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(spawn));
		packet->setMediumStringByName("mp3", mp3);
		packet->setDataByName("key", key1);
		packet->setDataByName("key", key2, 1);
		client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
}

void ZoneServer::PlayFlavor(Spawn* spawn, const char* mp3, const char* text, const char* emote, int32 key1, int32 key2, int8 language){
	if(!spawn)
		return;

	Client* client = 0;
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(!client || !client->IsReadyForUpdates() || !client->GetPlayer()->WasSentSpawn(spawn->GetID()) || client->GetPlayer()->GetDistance(spawn) > 30)
			continue;
		PlayFlavor(client, spawn, mp3, text, emote, key1, key2, language);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::PlayFlavorID(Spawn* spawn, int8 type, int32 id, int16 index, int8 language){
	if(!spawn)
		return;

	Client* client = 0;
	vector<Client*>::iterator client_itr;

	VoiceOverStruct non_garble, garble;
	bool garble_success = false;
	bool success = world.FindVoiceOver(type, id, index, &non_garble, &garble_success, &garble);
	
	VoiceOverStruct* resStruct = nullptr;
	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(!client || !client->IsReadyForUpdates() || !client->GetPlayer()->WasSentSpawn(spawn->GetID()) || client->GetPlayer()->GetDistance(spawn) > 30)
			continue;
		
		client->SendPlayFlavor(spawn, language, &non_garble, &garble, success, garble_success);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::PlayVoice(Spawn* spawn, const char* mp3, int32 key1, int32 key2){
	if(!spawn || !mp3)
		return;

	Client* client = 0;
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(!client || !client->IsReadyForUpdates() || !client->GetPlayer()->WasSentSpawn(spawn->GetID()))
			continue;
		PlayVoice(client, spawn, mp3, key1, key2);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::PlaySoundFile(Client* client, const char* name, float origin_x, float origin_y, float origin_z){
	if(!name)
		return;

	PacketStruct* packet = 0;
	if(client){
		packet = configReader.getStruct("WS_Play3DSound", client->GetVersion());
		if(packet){
			packet->setMediumStringByName("name", name);
			packet->setDataByName("x", origin_x);
			packet->setDataByName("y", origin_y);
			packet->setDataByName("z", origin_z);
			packet->setDataByName("unknown1", 1);
			packet->setDataByName("unknown2", 2.5);
			packet->setDataByName("unknown3", 15);
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
	else{
		EQ2Packet* outapp = 0;
		int16 packet_version = 0;
		vector<Client*>::iterator client_itr;

		MClientList.readlock(__FUNCTION__, __LINE__);
		for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
			client = *client_itr;
			if(client && (!packet || packet_version != client->GetVersion())){
				safe_delete(packet);
				safe_delete(outapp);
				packet_version = client->GetVersion();
				packet = configReader.getStruct("WS_Play3DSound", packet_version);
				if(packet){
					packet->setMediumStringByName("name", name);
					packet->setDataByName("x", origin_x);
					packet->setDataByName("y", origin_y);
					packet->setDataByName("z", origin_z);
					packet->setDataByName("unknown1", 1);
					packet->setDataByName("unknown2", 2.5);
					packet->setDataByName("unknown3", 15);
					outapp = packet->serialize();
				}
			}
			if(outapp && client && client->IsReadyForUpdates())
				client->QueuePacket(outapp->Copy());
		}
		MClientList.releasereadlock(__FUNCTION__, __LINE__);
		safe_delete(packet);
		safe_delete(outapp);
	}
}

bool ZoneServer::HasWidgetTimer(Spawn* widget){	
	bool ret = false;
	if (widget) {
		int32 id = widget->GetID();
		map<int32, int32>::iterator itr;
		MWidgetTimers.readlock(__FUNCTION__, __LINE__);
		for (itr = widget_timers.begin(); itr != widget_timers.end(); itr++) {
			if(itr->first == id){
				ret = true;
				break;
			}
		}
		MWidgetTimers.releasereadlock(__FUNCTION__, __LINE__);
	}
	return ret;
}

void ZoneServer::CheckWidgetTimers(){
	vector<int32> remove_list;
	map<int32, int32>::iterator itr;

	MWidgetTimers.readlock(__FUNCTION__, __LINE__);
	for (itr = widget_timers.begin(); itr != widget_timers.end(); itr++) {
		if(Timer::GetCurrentTime2() >= itr->second){
			/*Spawn* widget = GetSpawnByID(itr->first);
			if (widget && widget->IsWidget())
				((Widget*)widget)->HandleTimerUpdate();*/

			remove_list.push_back(itr->first);
		}
	}
	MWidgetTimers.releasereadlock(__FUNCTION__, __LINE__);

	for (int32 i = 0; i < remove_list.size(); i++) {
		Spawn* widget = GetSpawnByID(remove_list[i]);
		if (widget && widget->IsWidget())
			((Widget*)widget)->HandleTimerUpdate();
	}

	MWidgetTimers.writelock(__FUNCTION__, __LINE__);
	for(int32 i=0;i<remove_list.size(); i++)
		widget_timers.erase(remove_list[i]);
	MWidgetTimers.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::AddWidgetTimer(Spawn* widget, float time) {
	if (widget && widget->IsWidget()) {
		MWidgetTimers.writelock(__FUNCTION__, __LINE__);
		widget_timers[widget->GetID()] = ((int32)(time * 1000)) + Timer::GetCurrentTime2();
		MWidgetTimers.releasewritelock(__FUNCTION__, __LINE__);
	}
}

Spawn*	ZoneServer::GetSpawnGroup(int32 id){
	Spawn* ret = 0;
	Spawn* spawn = 0;
	
	if(id < 1)
		return 0;
	
	bool lookup = false;
	if(quick_group_id_lookup.count(id) > 0) {
		ret = GetSpawnByID(quick_group_id_lookup.Get(id));
		lookup = true;
	}
	if(ret == NULL) {
		if(lookup)
			quick_group_id_lookup.erase(id);
		map<int32, Spawn*>::iterator itr;
		MSpawnList.readlock(__FUNCTION__, __LINE__);
		for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
			spawn = itr->second;
			if(spawn){
				if(spawn->Alive() && spawn->GetSpawnGroupID() == id){
					ret = spawn;
					quick_group_id_lookup.Put(id, spawn->GetID());
					break;
				}
			}
		}
		MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	}
	return ret;
}

bool ZoneServer::IsSpawnGroupAlive(int32 id){
	bool ret = false;
	if(id < 1)
		return ret;
	
	Spawn* spawn = GetSpawnGroup(id);
	if(spawn) {
		vector<Spawn*> groupMembers;
		if (!spawn->IsPlayer() && spawn->HasSpawnGroup()) {
			groupMembers = *spawn->GetSpawnGroup();
		}
		
		Spawn* group_spawn = 0;
		vector<Spawn*>::iterator itr;
		for(itr = groupMembers.begin(); itr != groupMembers.end(); itr++){
			group_spawn = *itr;
			if(group_spawn->Alive()) {
				 ret = true;
				 break;
			}
		}
	}
	return ret;
}

Spawn* ZoneServer::GetSpawnByLocationID(int32 location_id) {
	Spawn* ret = 0;
	Spawn* current_spawn = 0;
	
	if(location_id < 1)
		return 0;
	
	bool lookup = false;
	if(quick_location_id_lookup.count(location_id) > 0) {
		ret = GetSpawnByID(quick_location_id_lookup.Get(location_id));
		lookup = true;
	}
	if(ret == NULL) {
		if(lookup)
			quick_location_id_lookup.erase(location_id);
		map<int32, Spawn*>::iterator itr;
		MSpawnList.readlock(__FUNCTION__, __LINE__);
		for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
			current_spawn = itr->second;
			if (current_spawn && current_spawn->GetSpawnLocationID() == location_id) {
				ret = current_spawn;
				quick_location_id_lookup.Put(location_id, ret->GetID());
				break;
			}
		}
		MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	}
	return ret;
}

Spawn*	ZoneServer::GetSpawnByDatabaseID(int32 id){
	Spawn* ret = 0;
	
	if(id < 1)
		return 0;
	
	bool lookup = false;
	
	if(quick_database_id_lookup.count(id) > 0) {
		ret = GetSpawnByID(quick_database_id_lookup.Get(id));
		lookup = true;
	}
	if(ret == NULL){
		if(lookup)
			quick_database_id_lookup.erase(id);
		Spawn* spawn = 0;
		map<int32, Spawn*>::iterator itr;
		MSpawnList.readlock(__FUNCTION__, __LINE__);
		for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++){
			spawn = itr->second;
			if(spawn){
				if(spawn->GetDatabaseID() == id){
					quick_database_id_lookup.Put(id, spawn->GetID());
					ret = spawn;
					break;
				}
			}
		}
		MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	}
	return ret;
}

Spawn* ZoneServer::GetSpawnByID(int32 id, bool spawnListLocked) {
	Spawn* ret = 0;
	if (!spawnListLocked )
		MSpawnList.readlock(__FUNCTION__, __LINE__);

	if (spawn_list.count(id) > 0)
		ret = spawn_list[id];
	
	if (!spawnListLocked)
		MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

bool ZoneServer::SendRemoveSpawn(Client* client, Spawn* spawn, PacketStruct* packet, bool delete_spawn)
{
	if(!client || !spawn || (client && client->GetPlayer() == spawn))
		return false;

	if(client->GetPlayerPOVGhostSpawnID() == spawn->GetID()) {
			client->SetPlayerPOVGhost(nullptr);
	}
	
	spawn->RemoveSpawnFromPlayer(client->GetPlayer());
	return true;
}

void ZoneServer::SetSpawnCommand(Spawn* spawn, int8 type, char* value, Client* client){
	//commands
	LogWrite(MISC__TODO, 1, "TODO", "%s does nothing!\n%s, %i", __FUNCTION__, __FILE__, __LINE__);
}

void ZoneServer::SetSpawnCommand(int32 spawn_id, int8 type, char* value, Client* client){
	LogWrite(MISC__TODO, 1, "TODO", "%s does nothing!\n%s, %i", __FUNCTION__, __FILE__, __LINE__);
}

void ZoneServer::ApplySetSpawnCommand(Client* client, Spawn* target, int8 type, const char* value){
	// This will apply the /spawn set command to all the spawns in the zone with the same DB ID, we do not want to set
	// location values (x, y, z, heading, grid) for all spawns in the zone with the same DB ID, only the targeted spawn
	if(type == SPAWN_SET_VALUE_SPAWNENTRY_SCRIPT || type == SPAWN_SET_VALUE_SPAWNLOCATION_SCRIPT || (type >= SPAWN_SET_VALUE_X && type <= SPAWN_SET_VALUE_LOCATION) ||
		type == SPAWN_SET_VALUE_PITCH || type == SPAWN_SET_VALUE_ROLL)
		return;

	Spawn* tmp = 0;
	if(target->IsNPC())
		tmp = GetNPC(target->GetDatabaseID());
	else if(target->IsObject())
		tmp = GetObject(target->GetDatabaseID());
	else if(target->IsGroundSpawn())
		tmp = GetGroundSpawn(target->GetDatabaseID());
	else if(target->IsSign())
		tmp = GetSign(target->GetDatabaseID());
	else if(target->IsWidget())
		tmp = GetWidget(target->GetDatabaseID());
	if(tmp && type == SPAWN_SET_VALUE_SPAWN_SCRIPT)
		tmp->SetSpawnScript(value);
	else if(tmp)
		commands.SetSpawnCommand(client, tmp, type, value); // set the master spawn
	Spawn* spawn = 0;

	// this check needs to be here otherwise every spawn with 0 will be set
	if ( target->GetDatabaseID ( ) > 0 )
	{
		map<int32, Spawn*>::iterator itr;
		MSpawnList.readlock(__FUNCTION__, __LINE__);
		for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
			spawn = itr->second;
			if(spawn && spawn->GetDatabaseID() == target->GetDatabaseID()){
				if(type == SPAWN_SET_VALUE_SPAWN_SCRIPT)
					spawn->SetSpawnScript(value);
				else
					commands.SetSpawnCommand(client, spawn, type, value);
			}
		}
		MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	}
}

void ZoneServer::StopSpawnScriptTimer(Spawn* spawn, std::string functionName){
	MSpawnScriptTimers.writelock(__FUNCTION__, __LINE__);
	MRemoveSpawnScriptTimersList.writelock(__FUNCTION__, __LINE__);
	if(spawn_script_timers.size() > 0){
		set<SpawnScriptTimer*>::iterator itr;
		SpawnScriptTimer* timer = 0;
		for (itr = spawn_script_timers.begin(); itr != spawn_script_timers.end(); ) {
			timer = *itr;
			if(timer->spawn == spawn->GetID() && (functionName == "" || timer->function == functionName) && remove_spawn_script_timers_list.count(timer) == 0) {
				itr = spawn_script_timers.erase(itr);
				safe_delete(timer);
			}
			else
				itr++;
		}
	}
	MRemoveSpawnScriptTimersList.releasewritelock(__FUNCTION__, __LINE__);
	MSpawnScriptTimers.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::DeleteSpawnScriptTimers(Spawn* spawn, bool all){
	MSpawnScriptTimers.writelock(__FUNCTION__, __LINE__);
	MRemoveSpawnScriptTimersList.writelock(__FUNCTION__, __LINE__);
	if(spawn_script_timers.size() > 0){
		set<SpawnScriptTimer*>::iterator itr;
		SpawnScriptTimer* timer = 0;
		for (itr = spawn_script_timers.begin(); itr != spawn_script_timers.end(); itr++) {
			timer = *itr;
			if((all || timer->spawn == spawn->GetID()) && remove_spawn_script_timers_list.count(timer) == 0)
				remove_spawn_script_timers_list.insert(timer);
		}

		if(all)
			spawn_script_timers.clear();
	}
	MRemoveSpawnScriptTimersList.releasewritelock(__FUNCTION__, __LINE__);
	MSpawnScriptTimers.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::DeleteSpawnScriptTimers() {
	MSpawnScriptTimers.writelock(__FUNCTION__, __LINE__);
	MRemoveSpawnScriptTimersList.writelock(__FUNCTION__, __LINE__);
	if(remove_spawn_script_timers_list.size() > 0){
		set<SpawnScriptTimer*>::iterator itr;
		SpawnScriptTimer* timer = 0;
		
		for (itr = remove_spawn_script_timers_list.begin(); itr != remove_spawn_script_timers_list.end(); itr++) {
			timer = *itr;
			set<SpawnScriptTimer*>::iterator itr2;

			itr2 = spawn_script_timers.find(timer);

			if(itr2 != spawn_script_timers.end())
				spawn_script_timers.erase(itr2);

			safe_delete(timer);	
		}
		remove_spawn_script_timers_list.clear();
	}
	MRemoveSpawnScriptTimersList.releasewritelock(__FUNCTION__, __LINE__);
	MSpawnScriptTimers.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::CheckSpawnScriptTimers(){
	DeleteSpawnScriptTimers();
	SpawnScriptTimer* timer = 0;
	vector<SpawnScriptTimer> call_timers;
	MSpawnScriptTimers.readlock(__FUNCTION__, __LINE__);
	MRemoveSpawnScriptTimersList.writelock(__FUNCTION__, __LINE__);
	if(spawn_script_timers.size() > 0){
		int32 current_time = Timer::GetCurrentTime2();
		set<SpawnScriptTimer*>::iterator itr;
		for (itr = spawn_script_timers.begin(); itr != spawn_script_timers.end(); itr++) {
			timer = *itr;
			if(remove_spawn_script_timers_list.count(timer) == 0 && 
				timer->current_count < timer->max_count && current_time >= timer->timer){
				timer->current_count++;	
				SpawnScriptTimer tmpTimer;
				tmpTimer.current_count = timer->current_count;
				tmpTimer.function = timer->function;
				tmpTimer.player = timer->player;
				tmpTimer.spawn = timer->spawn;
				tmpTimer.max_count = timer->max_count;
				call_timers.push_back(tmpTimer);
			}
			if(timer->current_count >= timer->max_count && remove_spawn_script_timers_list.count(timer) == 0)
				remove_spawn_script_timers_list.insert(timer);
		}
	}
	MRemoveSpawnScriptTimersList.releasewritelock(__FUNCTION__, __LINE__);
	MSpawnScriptTimers.releasereadlock(__FUNCTION__, __LINE__);
	if(call_timers.size() > 0){
		vector<SpawnScriptTimer>::iterator itr;
		for(itr = call_timers.begin(); itr != call_timers.end(); itr++){
			SpawnScriptTimer tmpTimer = (SpawnScriptTimer)*itr;
			Spawn* callSpawn = GetSpawnByID(tmpTimer.spawn);
			Spawn* target = nullptr;
			if(tmpTimer.player) {
				target = GetSpawnByID(tmpTimer.player);
			}
			if(callSpawn) {
				if(!callSpawn->IsPlayer()) {
					CallSpawnScript(GetSpawnByID(tmpTimer.spawn), SPAWN_SCRIPT_TIMER, target, tmpTimer.function.c_str());
				}
				else {
					const char* playerScript = world.GetPlayerScript(0); // 0 = global script
					const char* playerZoneScript = world.GetPlayerScript(GetZoneID()); // zone script
					if(playerScript || playerZoneScript) {
						std::vector<LuaArg> args = {
							LuaArg(this), 
							LuaArg(callSpawn),
							LuaArg(target)
						};
						if(playerScript) {
							lua_interface->RunPlayerScriptWithReturn(playerScript, tmpTimer.function.c_str(), args);
						}
						if(playerZoneScript) {
							lua_interface->RunPlayerScriptWithReturn(playerZoneScript, tmpTimer.function.c_str(), args);
						}
					}
				}
			}
		}
	}
}

void ZoneServer::KillSpawnByDistance(Spawn* spawn, float max_distance, bool include_players, bool send_packet){
	if(!spawn)
		return;
	
	auto loc = glm::vec3(spawn->GetX(), spawn->GetZ(), spawn->GetY());
	std::vector<int32> grids_by_radius;
	if(spawn->GetMap()) {
		grids_by_radius = GetGridsByLocation(spawn, loc, max_distance);
	}
	else {
		grids_by_radius.push_back(spawn->GetLocation());
	}
	
	Spawn* test_spawn = 0;
    MGridMaps.lock_shared();
	std::vector<int32>::iterator grid_radius_itr;
	for(grid_radius_itr = grids_by_radius.begin(); grid_radius_itr != grids_by_radius.end(); grid_radius_itr++) {
		std::map<int32, GridMap*>::iterator grids = grid_maps.find((*grid_radius_itr));
		if(grids != grid_maps.end()) {
			grids->second->MSpawns.lock_shared();
			typedef map <int32, Spawn*> SpawnMapType;
			for( SpawnMapType::iterator it = grids->second->spawns.begin(); it != grids->second->spawns.end(); ++it ) {
				test_spawn = it->second;
				if(test_spawn && test_spawn->Alive() && test_spawn->GetID() > 0 && test_spawn->GetID() != spawn->GetID() && test_spawn->IsEntity() &&
				  (!test_spawn->IsPlayer() || include_players)){
					if(test_spawn->GetDistance(spawn) < max_distance)
						KillSpawn(true, test_spawn, spawn, send_packet);
				}
			}
			grids->second->MSpawns.unlock_shared();
		}
	}
    MGridMaps.unlock_shared();
}

void ZoneServer::SpawnSetByDistance(Spawn* spawn, float max_distance, string field, string value){
	if(!spawn)
		return;
	
	Spawn* test_spawn = 0;
	int32 type = commands.GetSpawnSetType(field);
	if(type == 0xFFFFFFFF)
		return;

	auto loc = glm::vec3(spawn->GetX(), spawn->GetZ(), spawn->GetY());
	std::vector<int32> grids_by_radius;
	if(spawn->GetMap()) {
		grids_by_radius = GetGridsByLocation(spawn, loc, max_distance);
	}
	else {
		grids_by_radius.push_back(spawn->GetLocation());
	}
	
    MGridMaps.lock_shared();
	std::vector<int32>::iterator grid_radius_itr;
	for(grid_radius_itr = grids_by_radius.begin(); grid_radius_itr != grids_by_radius.end(); grid_radius_itr++) {
		std::map<int32, GridMap*>::iterator grids = grid_maps.find((*grid_radius_itr));
		grids->second->MSpawns.lock_shared();
		typedef map <int32, Spawn*> SpawnMapType;
		for( SpawnMapType::iterator it = grids->second->spawns.begin(); it != grids->second->spawns.end(); ++it ) {
				test_spawn = it->second;
			if(test_spawn && test_spawn->GetID() > 0 && test_spawn->GetID() != spawn->GetID() && !test_spawn->IsPlayer()){
				if(test_spawn->GetDistance(spawn) < max_distance){
					commands.SetSpawnCommand(0, test_spawn, type, value.c_str());
				}
			}
		}
		grids->second->MSpawns.unlock_shared();
	}
    MGridMaps.unlock_shared();
}

void ZoneServer::AddSpawnScriptTimer(SpawnScriptTimer* timer){
	MSpawnScriptTimers.writelock(__FUNCTION__, __LINE__);
	spawn_script_timers.insert(timer);
	MSpawnScriptTimers.releasewritelock(__FUNCTION__, __LINE__);
}

/*
void ZoneServer::RemoveFromRangeMap(Client* client){
	spawn_range_map.erase(client);
}
*/

void ZoneServer::RemoveSpawn(Spawn* spawn, bool delete_spawn, bool respawn, bool lock, bool erase_from_spawn_list, bool lock_spell_process) 
{
	if(!spawn->IsDeletedSpawn() && spawn->IsPlayer()) {
		if(pNumPlayers > 0)
			pNumPlayers--;
	}
	LogWrite(ZONE__DEBUG, 3, "Zone", "Processing RemoveSpawn function for %s (%i)...", spawn->GetName(),spawn->GetID());

	PacketStruct* packet = 0;
	int16 packet_version = 0;
	Client* client = 0;

	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;

		if (client && (!client->IsZoning() || client->GetPlayer() != spawn)) {
			if (client->GetPlayer()->HasTarget() && client->GetPlayer()->GetTarget() == spawn)
				client->GetPlayer()->SetTarget(0);
			if(client->GetMailTransaction() == spawn)
				client->SetMailTransaction(0);
			if(client->GetBanker() == spawn->GetID())
				client->SetBanker(0);
			if(client->GetTransportSpawnID() == spawn->GetID())
				client->SetTransportSpawnID(0);
			if(client->GetTempPlacementSpawn() == spawn)
				client->SetTempPlacementSpawn(nullptr);
			if(client->GetCombineSpawn() == spawn)
				client->SetCombineSpawn(nullptr);

			//don't send destroy ghost of 283 client when zoning
			if((client->GetVersion() > 373 || !client->IsZoning()) && (client->GetPlayer()->WasSentSpawn(spawn->GetID()) || client->GetPlayer()->IsSendingSpawn(spawn->GetID())))
				SendRemoveSpawn(client, spawn, packet, delete_spawn);
			
			if (spawn_range_map.count(client) > 0)
				spawn_range_map.Get(client)->erase(spawn->GetID());
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);

	safe_delete(packet);
	
	spawn->RemoveSpawnProximities();
	RemoveSpawnProximities(spawn);

	if (movementMgr != nullptr && spawn->IsEntity()) {
		movementMgr->RemoveMob((Entity*)spawn);
	}

	RemoveSpawnSupportFunctions(spawn, lock_spell_process);
	if (reloading)
		RemoveDeadEnemyList(spawn);

	if (lock)
		MDeadSpawns.writelock(__FUNCTION__, __LINE__);

	if (dead_spawns.count(spawn->GetID()) > 0)
		dead_spawns.erase(spawn->GetID());
	if (lock)
		MDeadSpawns.releasewritelock(__FUNCTION__, __LINE__);

	if (spawn_expire_timers.count(spawn->GetID()) > 0)
		spawn_expire_timers.erase(spawn->GetID());
	
	spawn->SetDeletedSpawn(true);

	// we will remove the spawn ptr and entry in the spawn_list later.. it is not safe right now (lua? client process? spawn process? etc? too many factors)
	if(erase_from_spawn_list)
		AddPendingSpawnRemove(spawn->GetID());

	if(respawn && !spawn->IsPlayer() && spawn->GetSpawnLocationID() > 0) {
		LogWrite(ZONE__DEBUG, 3, "Zone", "Handle NPC Respawn for '%s'.", spawn->GetName());
		AddRespawn(spawn);
	}
	
	RemoveSpawnFromGrid(spawn, spawn->GetLocation());

	// Do we really need the mutex locks and check to dead_spawns as we remove it from dead spawns at the start of this function
	if (lock && !respawn)
		MDeadSpawns.readlock(__FUNCTION__, __LINE__);
	if(delete_spawn && dead_spawns.count(spawn->GetID()) == 0)
		AddPendingDelete(spawn);
	if (lock && !respawn)
		MDeadSpawns.releasereadlock(__FUNCTION__, __LINE__);

	LogWrite(ZONE__DEBUG, 3, "Zone", "Done processing RemoveSpawn function...");
}

Spawn* ZoneServer::GetClosestSpawn(Spawn* spawn, int32 spawn_id){
	Spawn* closest_spawn = 0;
	Spawn* test_spawn = 0;
	float closest_distance = 1000000;
	float test_distance = 0;

	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		test_spawn = itr->second;
		if(test_spawn && test_spawn != spawn && test_spawn->GetDatabaseID() == spawn_id){
			test_distance = test_spawn->GetDistance(spawn);
			if(test_distance < closest_distance){
				closest_distance = test_distance;
				closest_spawn = test_spawn;
			}
		}
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);

	return closest_spawn;
}

int32 ZoneServer::GetClosestLocation(Spawn* spawn){
	Spawn* closest_spawn = 0;
	Spawn* test_spawn = 0;
	float closest_distance = 1000000;
	float test_distance = 0;

	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		test_spawn = itr->second;
		if(test_spawn){
			test_distance = test_spawn->GetDistance(spawn);
			if(test_distance < closest_distance){
				closest_distance = test_distance;
				closest_spawn = test_spawn;
				if(closest_distance < 10)
					break;
			}
		}
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);

	if(closest_spawn)
		return closest_spawn->GetLocation();
	return 0;
}

void ZoneServer::SendQuestUpdates(Client* client, Spawn* spawn){
	if(!client)
		return;

	if(spawn){
		if(client->GetPlayer()->WasSentSpawn(spawn->GetID()))
			SendSpawnChanges(spawn, client, false, true);
	}
	else{
		client->GetCurrentZone()->SendAllSpawnsForVisChange(client);
	}
}

void ZoneServer::SendAllSpawnsForLevelChange(Client* client) {
	Spawn* spawn = 0;
	if (spawn_range_map.count(client) > 0) {
		MutexMap<int32, float >::iterator itr = spawn_range_map.Get(client)->begin();
		while (itr.Next()) {
			spawn = GetSpawnByID(itr->first);
			if (spawn && client->GetPlayer()->WasSentSpawn(spawn->GetID())) {
				SendSpawnChanges(spawn, client, false, true);
				// Attempt to slow down the packet spam sent to the client
				// who the bloody fuck put a Sleep here
				//Sleep(5);
			}
		}
	}
}


void ZoneServer::SendAllSpawnsForSeeInvisChange(Client* client) {
	Spawn* spawn = 0;
	if (spawn_range_map.count(client) > 0) {
		MutexMap<int32, float >::iterator itr = spawn_range_map.Get(client)->begin();
		while (itr.Next()) {
			spawn = GetSpawnByID(itr->first);
			if (spawn && spawn->IsEntity() && (((Entity*)spawn)->IsInvis() || ((Entity*)spawn)->IsStealthed()) && client->GetPlayer()->WasSentSpawn(spawn->GetID())) {
				SendSpawnChanges(spawn, client, true, true);
			}
		}
	}
}


void ZoneServer::SendAllSpawnsForVisChange(Client* client, bool limitToEntities) {
	Spawn* spawn = 0;
	if (spawn_range_map.count(client) > 0) {
		MutexMap<int32, float >::iterator itr = spawn_range_map.Get(client)->begin();
		while (itr.Next()) {
			spawn = GetSpawnByID(itr->first);
			if (spawn && (!limitToEntities || (limitToEntities && spawn->IsEntity())) && client->GetPlayer()->WasSentSpawn(spawn->GetID())) {
				SendSpawnChanges(spawn, client, false, true);
			}
		}
	}
}

void ZoneServer::StartZoneSpawnsForLevelThread(Client* client){
	if(zoneShuttingDown)
		return;

#ifdef WIN32
	_beginthread(SendLevelChangedSpawns, 0, client);
#else
	pthread_t thread;
	pthread_create(&thread, NULL, SendLevelChangedSpawns, client);
	pthread_detach(thread);
#endif
}

void ZoneServer::ReloadClientQuests(){
	Client* client = 0;
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(client)
			client->ReloadQuests();
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::SendCalculatedXP(Player* player, Spawn* victim){
	if (player && victim) {
		if (player->GetGroupMemberInfo()) {
			world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);

			PlayerGroup* group = world.GetGroupManager()->GetGroup(player->GetGroupMemberInfo()->group_id);
			if (group)
			{
				group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
				deque<GroupMemberInfo*>* members = group->GetMembers();
				deque<GroupMemberInfo*>::iterator itr;
				bool skipGrayMob = false;
				
				for (itr = members->begin(); itr != members->end(); itr++) {
					GroupMemberInfo* gmi = *itr;
					if (gmi->client) {
						Player* group_member = gmi->client->GetPlayer();
						if(group_member && group_member->GetArrowColor(victim->GetLevel()) == ARROW_COLOR_GRAY) {
							skipGrayMob = true;
							break;
						}
					}
				}

				for (itr = members->begin(); !skipGrayMob && itr != members->end(); itr++) {
					GroupMemberInfo* gmi = *itr;
					if (gmi->client) {
						Player* group_member = gmi->client->GetPlayer();
						if(group_member) {
							float xp = group_member->CalculateXP(victim) / members->size();
							if (xp > 0) {
								group_member->AddXP((int32)xp);
							}
						}
					}
				}
				group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
			}

			world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
		}
		else {
			float xp = player->CalculateXP(victim);
			if (xp > 0) {
				Client* client = ((Player*)player)->GetClient();
				if(!client)
					return;
				player->AddXP((int32)xp);
			}
		}
	}
}

void ZoneServer::ProcessFaction(Spawn* spawn, Client* client)
{
	if(client && !spawn->IsPlayer() && spawn->GetFactionID() > 10)
	{
		bool update_result = false;
		Faction* faction = 0;
		vector<int32>* factions = 0;
		Player* player = client->GetPlayer();

		bool hasfaction = database.VerifyFactionID(player->GetCharacterID(), spawn->GetFactionID());
		//if 0 they dont have an entry in the db for this faction. if one they do and we can skip it.
		if(hasfaction == 0) {
			//Find out the default for this faction
			sint32 defaultfaction = master_faction_list.GetDefaultFactionValue(spawn->GetFactionID());
			//add the default faction for the player.
			player->SetFactionValue(spawn->GetFactionID(), defaultfaction);
			//save the character so the new default gets written to the db. 
			client->Save();
		}

		if(player->GetFactions()->ShouldDecrease(spawn->GetFactionID()))
		{

			update_result = player->GetFactions()->DecreaseFaction(spawn->GetFactionID());
			faction = master_faction_list.GetFaction(spawn->GetFactionID());

			if(faction && update_result)
				client->Message(CHANNEL_FACTION, "Your faction standing with %s got worse.", faction->name.c_str());
			else if(faction)
				client->Message(CHANNEL_FACTION, "Your faction standing with %s could not possibly get any worse.", faction->name.c_str());

			factions = master_faction_list.GetHostileFactions(spawn->GetFactionID());

			if(factions)
			{
				vector<int32>::iterator itr;

				for(itr = factions->begin(); itr != factions->end(); itr++)
				{
					if(player->GetFactions()->ShouldIncrease(*itr))
					{

						update_result = player->GetFactions()->IncreaseFaction(*itr);
						faction = master_faction_list.GetFaction(*itr);

						if(faction && update_result)
							client->Message(CHANNEL_FACTION, "Your faction standing with %s got better.", faction->name.c_str());
						else if(faction)
							client->Message(CHANNEL_FACTION, "Your faction standing with %s could not possibly get any better.", faction->name.c_str());
					}
				}
			}
		}

		factions = master_faction_list.GetFriendlyFactions(spawn->GetFactionID());

		if(factions)
		{
			vector<int32>::iterator itr;

			for(itr = factions->begin(); itr != factions->end(); itr++)
			{
				if(player->GetFactions()->ShouldDecrease(*itr))
				{
					bool hasfaction = database.VerifyFactionID(player->GetCharacterID(),spawn->GetFactionID());
					if(hasfaction == 0) {
						//they do not have the faction. Lets get the default value and feed it in.
						sint32 defaultfaction = master_faction_list.GetDefaultFactionValue(spawn->GetFactionID());
						//add the default faction for the player.
						player->SetFactionValue(spawn->GetFactionID(), defaultfaction);
					}

					update_result = player->GetFactions()->DecreaseFaction(*itr);
					faction = master_faction_list.GetFaction(*itr);

					if(faction && update_result)
						client->Message(CHANNEL_FACTION, "Your faction standing with %s got worse.", faction->name.c_str());
					else if(faction)
						client->Message(CHANNEL_FACTION, "Your faction standing with %s could not possibly get any worse.", faction->name.c_str());
				}
			}
		}

		EQ2Packet* outapp = client->GetPlayer()->GetFactions()->FactionUpdate(client->GetVersion());

		if(outapp)
			client->QueuePacket(outapp);
	}
}

void ZoneServer::Despawn(Spawn* spawn, int32 timer){
	if (spawn && movementMgr != nullptr) {
		movementMgr->RemoveMob((Entity*)spawn);
	}
	if(!spawn || spawn->IsPlayer())
		return;
	if(spawn->IsEntity())
		((Entity*)spawn)->InCombat(false);
	if(timer == 0)
		timer = 1;
	AddDeadSpawn(spawn, timer);
}

void ZoneServer::KillSpawn(bool spawnListLocked, Spawn* dead, Spawn* killer, bool send_packet, int8 type, int8 damage_type, int16 kill_blow_type)
{
	bool isSpell = (type == DAMAGE_PACKET_TYPE_SIPHON_SPELL || type == DAMAGE_PACKET_TYPE_SIPHON_SPELL2 ||
					type == DAMAGE_PACKET_TYPE_SPELL_DAMAGE || type == DAMAGE_PACKET_TYPE_SPELL_CRIT_DMG || 
					type == DAMAGE_PACKET_TYPE_SPELL_DAMAGE2 || type == DAMAGE_PACKET_TYPE_SPELL_DAMAGE3);
	
	MDeadSpawns.readlock(__FUNCTION__, __LINE__);
	if(!dead || this->dead_spawns.count(dead->GetID()) > 0) {
		MDeadSpawns.releasereadlock(__FUNCTION__, __LINE__);
		return;
	}
	MDeadSpawns.releasereadlock(__FUNCTION__, __LINE__);

	PacketStruct* packet = 0;
	Client* client = 0;
	vector<int32>* encounter = 0;
	int32 encounter_player_bot_count = 1;
	bool killer_in_encounter = false;
	int8 loot_state = dead->GetLockedNoLoot();
		
	if(dead->IsEntity())
	{
		// add any special quest related loot (no_drop_quest_completed)
		if(dead->IsNPC() && ((NPC*)dead)->Brain()) {
			if(!((NPC*)dead)->Brain()->PlayerInEncounter() || (loot_state != ENCOUNTER_STATE_LOCKED && loot_state != ENCOUNTER_STATE_OVERMATCHED)) {
				LogWrite(LOOT__DEBUG, 0, "Loot", "NPC %s bypassed loot drop due to no player in encounter, or encounter state not locked.", ((NPC*)dead)->GetName());
			}
			else {
				Entity* hated = ((NPC*)dead)->Brain()->GetMostHated();
				if(hated) {
					GroupLootMethod loot_method = GroupLootMethod::METHOD_FFA;
					int8 item_rarity = 0;
					if(hated->GetGroupMemberInfo()) {
						world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);
						PlayerGroup* group = world.GetGroupManager()->GetGroup(hated->GetGroupMemberInfo()->group_id);
						if (group) {
							loot_method = (GroupLootMethod)group->GetGroupOptions()->loot_method;
							item_rarity = group->GetGroupOptions()->loot_items_rarity;
							LogWrite(LOOT__DEBUG, 0, "Loot", "%s: Loot method set to %u.", dead->GetName(), loot_method);
						}
						world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
					}
					AddLoot((NPC*)dead, hated, loot_method, item_rarity, hated->GetGroupMemberInfo() ? hated->GetGroupMemberInfo()->group_id : 0);
				}
			}
		}
		((Entity*)dead)->InCombat(false);
		dead->SetInitialState(16512, false); // This will make aerial npc's fall after death
		dead->SetHP(0);
		dead->SetSpawnType(3);
		dead->appearance.attackable = 0;


		// Remove hate towards dead from all npc's in the zone
		ClearHate((Entity*)dead);
		
		// Check kill and death procs
		if (killer && dead != killer){
			if (dead->IsEntity())
				((Entity*)dead)->CheckProcs(PROC_TYPE_DEATH, killer);
			if (killer->IsEntity())
				((Entity*)killer)->CheckProcs(PROC_TYPE_KILL, dead);
		}

		//Check if caster is alive after death proc called, incase of deathsave
		if (dead->Alive())
			return;

		RemoveSpellTimersFromSpawn(dead, true, !dead->IsPlayer(), true, !isSpell);
		((Entity*)dead)->IsCasting(false);
		
		if(dead->IsPlayer()) 
		{
			((Player*)dead)->UpdatePlayerStatistic(STAT_PLAYER_TOTAL_DEATHS, 1);
			client = ((Player*)dead)->GetClient();

			((Entity*)dead)->HandleDeathExperienceDebt(killer);

			if(client) {

				if(client->GetPlayer()->DamageEquippedItems(10, client))
					client->QueuePacket(client->GetPlayer()->GetEquipmentList()->serialize(client->GetVersion(), client->GetPlayer()));

				client->DisplayDeadWindow();
			}
		}
		else if (dead->IsNPC()) {
			encounter = ((NPC*)dead)->Brain()->GetEncounter();
			encounter_player_bot_count = ((NPC*)dead)->Brain()->CountPlayerBotInEncounter();
			if(encounter_player_bot_count < 1)
				encounter_player_bot_count = 1;
		}

	}

	dead->SetActionState(0);
	dead->SetTempActionState(0);

	// Needs npc to have access to the encounter list for who is allowed to loot
	NPC* chest = 0;
	
	if (dead->IsNPC() && !((NPC*)dead)->Brain()->PlayerInEncounter()) {
		dead->SetLootCoins(0);
		dead->ClearLoot();
	}

	Spawn* groupMemberAlive = nullptr;
	// If dead has loot attempt to drop a chest
	if (dead->HasLoot()) {
		if(!(groupMemberAlive = dead->IsSpawnGroupMembersAlive(dead))) {
			chest = ((Entity*)dead)->DropChest();
		}
		else {
			switch(dead->GetLootDropType()) {
				case 0: 
					// default drop all chest type as a group
					dead->TransferLoot(groupMemberAlive);
				break;
				case 1:
					// this is a primary mob it drops its own loot
					chest = ((Entity*)dead)->DropChest();
				break;
			}
		}
	}
	// If dead is an npc get the encounter and loop through it giving out the rewards, no rewards for pets
	if (dead->IsNPC() && !dead->IsPet() && !dead->IsBot()) {
		Spawn* spawn = 0;
		int8 size = encounter->size();

		for (int8 i = 0; i < encounter->size(); i++) {
			spawn = GetSpawnByID(encounter->at(i), spawnListLocked);
			// set a flag to let us know if the killer is in the encounter
			if (!killer_in_encounter && spawn == killer)
				killer_in_encounter = true;

			if (spawn && spawn->IsPlayer()) {
				// Update players total kill count
				((Player*)spawn)->UpdatePlayerStatistic(STAT_PLAYER_TOTAL_NPC_KILLS, 1);

				// If this was an epic mob kill send the announcement for this player
				if (dead->GetDifficulty() >= 10)
					SendEpicMobDeathToGuild((Player*)spawn, dead);

				// Clear hostile spells from the players spell queue
				spellProcess->RemoveSpellFromQueue((Player*)spawn, true);

				// Get the client of the player
				client = ((Player*)spawn)->GetClient();
				// valid client?
				if (client) {
					// Check for quest kill updates
					if(!dead->IsNPC() || loot_state != ENCOUNTER_STATE_BROKEN) {
						client->CheckPlayerQuestsKillUpdate(dead);
					}
					// If the dead mob is not a player and if it had a faction with an ID greater or equal to 10 the send faction changes
					if (!dead->IsPlayer() && dead->GetFactionID() > 10)
						ProcessFaction(dead, client);

					// Send xp...this is currently wrong fix it
					if (spawn != dead && ((Player*)spawn)->GetArrowColor(dead->GetLevel()) >= ARROW_COLOR_GREEN) {
						//SendCalculatedXP((Player*)spawn, dead);

						float xp = ((Player*)spawn)->CalculateXP(dead) / encounter_player_bot_count;
						if (xp > 0) {
							((Player*)spawn)->AddXP((int32)xp);
						}
					}
				}
			}

			// If a chest is being dropped add this spawn to the chest's encounter so they can loot it
			if (chest && spawn && spawn->IsEntity())
				chest->Brain()->AddToEncounter((Entity*)spawn);
		}
	}
	
	// If a chest is being dropped add it to the world and set the timer to remove it.
	if (chest) {
		AddSpawn(chest);
		AddDeadSpawn(chest, 0xFFFFFFFF);
		LogWrite(LOOT__DEBUG, 0, "Loot", "Adding a chest to the world...");
	}
		
	// Reset client pointer
	client = 0;

	// Killer was not in the encounter, give them the faction hit but no xp
	if (!killer_in_encounter) {
		// make sure the killer is a player and the dead spawn had a faction and wasn't a player
		if (killer && killer->IsPlayer()) {
			if (!dead->IsPlayer() && dead->GetFactionID() > 10) {
			client = ((Player*)killer)->GetClient();
			if (client)
				ProcessFaction(dead, client);
			}

			// Clear hostile spells from the killers spell queue
			spellProcess->RemoveSpellFromQueue((Player*)killer, true);
		}
	}

	// Reset client pointer
	client = 0;	


	vector<Spawn*>* group = dead->GetSpawnGroup();
	if (group && group->size() == 1)
		CallSpawnScript(dead, SPAWN_SCRIPT_GROUP_DEAD, killer);
	safe_delete(group);


	// Remove the support functions for the dead spawn
	RemoveSpawnSupportFunctions(dead, !isSpell);

	// Erase the expire timer if it has one
	if (spawn_expire_timers.count(dead->GetID()) > 0)
		spawn_expire_timers.erase(dead->GetID());

	// If dead is an npc or object call the spawn scrip and handle instance stuff
	if(dead->IsNPC() || dead->IsObject())
	{
		// handle instance spawn db info
		// we don't care if a NPC or a client kills the spawn, we could have events that cause NPCs to kill NPCs.
		if(dead->GetZone()->GetInstanceID() > 0 && dead->GetSpawnLocationID() > 0)
		{
			// use respawn time to either insert/update entry (likely insert in this situation)
			if(dead->IsNPC())
				database.CreateInstanceSpawnRemoved(dead->GetSpawnLocationID(),SPAWN_ENTRY_TYPE_NPC, dead->GetRespawnTime(),dead->GetZone()->GetInstanceID());
			else if ( dead->IsObject ( ) )
				database.CreateInstanceSpawnRemoved(dead->GetSpawnLocationID(),SPAWN_ENTRY_TYPE_OBJECT, dead->GetRespawnTime(),dead->GetZone()->GetInstanceID());
		}
		else if(!groupMemberAlive && dead->GetSpawnLocationID() > 0 && dead->GetRespawnTime() && !DuplicatedZone()) {
			if(dead->IsNPC())
				database.CreatePersistedRespawn(dead->GetSpawnLocationID(),SPAWN_ENTRY_TYPE_NPC,dead->GetRespawnTime(),GetZoneID());
			else if(dead->IsObject())
				database.CreatePersistedRespawn(dead->GetSpawnLocationID(),SPAWN_ENTRY_TYPE_OBJECT,dead->GetRespawnTime(),GetZoneID());
		}

		// Call the spawn scripts death() function
		CallSpawnScript(dead, SPAWN_SCRIPT_DEATH, killer);
		const char* zone_script = world.GetZoneScript(this->GetZoneID());
		if (zone_script && lua_interface)
			lua_interface->RunZoneScript(zone_script, "spawn_killed", this, dead, 0, 0, killer);
	}
	
	int32 victim_id = dead->GetID();
	int32 attacker_id = 0xFFFFFFFF;

	if(killer)
		attacker_id = killer->GetID();

	if(send_packet)
	{
		vector<Client*>::iterator client_itr;
		MClientList.readlock(__FUNCTION__, __LINE__);
		for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
			client = *client_itr;
			if(!client->GetPlayer()->WasSentSpawn(victim_id) || (attacker_id != 0xFFFFFFFF && !client->GetPlayer()->WasSentSpawn(attacker_id)) )
				continue;
			else if(killer && killer->GetDistance(client->GetPlayer()) > HEAR_SPAWN_DISTANCE)
				continue;

			packet = configReader.getStruct("WS_HearDeath", client->GetVersion());
			if(packet)
			{
				if(killer)
					packet->setDataByName("attacker", client->GetPlayer()->GetIDWithPlayerSpawn(killer));
				else
					packet->setDataByName("attacker", 0xFFFFFFFF);

				packet->setDataByName("defender", client->GetPlayer()->GetIDWithPlayerSpawn(dead));
				packet->setDataByName("damage_type", damage_type);
				packet->setDataByName("blow_type", kill_blow_type);

				client->QueuePacket(packet->serialize());
				LogWrite(COMBAT__DEBUG, 0, "Combat", "Zone Killing of '%s' by '%s' damage type %u, blow type %u", dead->GetName(), killer ? killer->GetName() : "", damage_type, kill_blow_type);
				safe_delete(packet);
			}
		}
		MClientList.releasereadlock(__FUNCTION__, __LINE__);
	}


	int32 pop_timer = 0xFFFFFFFF;
	if(killer && killer->IsNPC())
	{
		// Call the spawn scripts killed() function
		CallSpawnScript(killer, SPAWN_SCRIPT_KILLED, dead);		

		if(!dead->IsPlayer())
		{
			LogWrite(MISC__TODO, 1, "TODO", "Whenever pets are added, check for pet kills\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);
			// Set the time for the corpse to linger to 5 sec
			//pop_timer = 5000;
			// commented out the timer so in the event the killer is not a player (pet, guard, something else i haven't thought of)
			// the corpse doesn't vanish sooner then it should if it had loot for the players.  AddDeadSpawn() will set timers based on if
			// the corpse has loot or not if the timer value (pop_timer) is 0xFFFFFFFF
		}
	}
	
	// If the dead spawns was not a player add it to the dead spawn list
	if (!dead->IsPlayer() && !dead->IsBot())
		AddDeadSpawn(dead, pop_timer);

	// if dead was a player clear hostile spells from its spell queue
	if (dead->IsPlayer())
		spellProcess->RemoveSpellFromQueue((Player*)dead, true);

	if (dead->IsNPC())
		((NPC*)dead)->Brain()->ClearHate();

	safe_delete(encounter);
	
	const char* functionToCall = "on_player_death";
	bool isPlayerDead = true;
	if(dead->IsPlayer() || (killer && killer->IsPlayer() && (isPlayerDead = false))) {
		if(!isPlayerDead) {
			functionToCall = "on_player_kill";
		}
		const char* playerScript = world.GetPlayerScript(0); // 0 = global script
		const char* playerZoneScript = world.GetPlayerScript(GetZoneID()); // zone script
		if(playerScript || playerZoneScript) {
			std::vector<LuaArg> args = {
				LuaArg(this),
				LuaArg(dead), 
				LuaArg(killer)
			};
			if(playerScript) {
				lua_interface->RunPlayerScriptWithReturn(playerScript, functionToCall, args);
			}
			if(playerZoneScript) {
				lua_interface->RunPlayerScriptWithReturn(playerZoneScript, functionToCall, args);
			}
		}
	}
}

void ZoneServer::SendDamagePacket(Spawn* attacker, Spawn* victim, int8 type1, int8 type2, int8 damage_type, int16 damage, const char* spell_name) {
	//Scat: was set but never being used anywhere. i see references to 0xFFFFFFFF below so could be old code not used anymore
	//int32 attacker_id = 0xFFFFFFFF;
	//if(attacker)
	//	attacker_id = attacker->GetID();
	PacketStruct* packet = 0;
	Client* client = 0;
	if (attacker && victim && victim->IsPlayer() && victim->GetTarget() == 0) {
		client = ((Player*)victim)->GetClient();
		if (client)
			client->TargetSpawn(attacker);
	}

	if(damage_type == DAMAGE_PACKET_DAMAGE_TYPE_FOCUS) {
		damage_type = 0;
		type2 = DAMAGE_PACKET_RESULT_FOCUS;
	}
	
	vector<Client*>::iterator client_itr;
	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if (!client || (client->GetPlayer() != attacker && client->GetPlayer() != victim && ((attacker && client->GetPlayer()->WasSentSpawn(attacker->GetID()) == false) || (victim && client->GetPlayer()->WasSentSpawn(victim->GetID()) == false))))
			continue;
		if (attacker && attacker->GetDistance(client->GetPlayer()) > 50)
			continue;
		if (victim && victim->GetDistance(client->GetPlayer()) > 50)
			continue;
		
		int8 mod_type1 = type1;
		if(client->GetVersion() <= 561) {
			mod_type1 = DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE;
		}
		
		switch (mod_type1) {
		case DAMAGE_PACKET_TYPE_SIPHON_SPELL:
		case DAMAGE_PACKET_TYPE_SIPHON_SPELL2:
			packet = configReader.getStruct("WS_HearSiphonSpellDamage", client->GetVersion());
			break;
		case DAMAGE_PACKET_TYPE_MULTIPLE_DAMAGE:
			if (client->GetVersion() > 561)
				packet = configReader.getStruct("WS_HearMultipleDamage", client->GetVersion());
			else
				packet = configReader.getStruct("WS_HearSimpleDamage", client->GetVersion());
			break;
		case DAMAGE_PACKET_TYPE_SIMPLE_CRIT_DMG:
		case DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE:
			packet = configReader.getStruct("WS_HearSimpleDamage", client->GetVersion());
			break;
		case DAMAGE_PACKET_TYPE_SPELL_DAMAGE2:
		case DAMAGE_PACKET_TYPE_SPELL_DAMAGE3:
		case DAMAGE_PACKET_TYPE_SPELL_CRIT_DMG:
		case DAMAGE_PACKET_TYPE_SPELL_DAMAGE:
			if (client->GetVersion() > 561)
				packet = configReader.getStruct("WS_HearSpellDamage", client->GetVersion());
			else
				packet = configReader.getStruct("WS_HearSimpleDamage", client->GetVersion());
			if (packet)
				packet->setSubstructDataByName("header", "unknown", 5);
			break;
		case DAMAGE_PACKET_TYPE_RANGE_DAMAGE:
			packet = configReader.getStruct("WS_HearRangeDamage", client->GetVersion());
			break;
		case DAMAGE_PACKET_TYPE_RANGE_SPELL_DMG:
		case DAMAGE_PACKET_TYPE_RANGE_SPELL_DMG2:
			packet = configReader.getStruct("WS_HearRangeDamage", client->GetVersion());
			break;
		default:
			LogWrite(ZONE__ERROR, 0, "Zone", "Unknown Damage Packet type: %i in ZoneServer::SendDamagePacket.", type1);
			MClientList.releasereadlock(__FUNCTION__, __LINE__);
			return;
		}
		
		if (packet) {
			if (client->GetVersion() > 561) {
				packet->setSubstructDataByName("header", "packet_type", type1);
				packet->setSubstructDataByName("header", "result_type", type2);
				packet->setDataByName("damage_type", damage_type);
				packet->setDataByName("damage", damage);
			}
			else {
				switch (type2) {
				case DAMAGE_PACKET_RESULT_MISS:
					packet->setSubstructDataByName("header", "result_type", 1);
					break;
				case DAMAGE_PACKET_RESULT_DODGE:
					packet->setSubstructDataByName("header", "result_type", 2);
					break;
				case DAMAGE_PACKET_RESULT_PARRY:
					packet->setSubstructDataByName("header", "result_type", 3);
					break;
				case DAMAGE_PACKET_RESULT_RIPOSTE:
					packet->setSubstructDataByName("header", "result_type", 4);
					break;
				case DAMAGE_PACKET_RESULT_BLOCK:
					packet->setSubstructDataByName("header", "result_type", 5);
					break;
				case DAMAGE_PACKET_RESULT_INVULNERABLE:
					packet->setSubstructDataByName("header", "result_type", 7);
					break;
				case DAMAGE_PACKET_RESULT_RESIST:
					packet->setSubstructDataByName("header", "result_type", 9);
					break;
				case DAMAGE_PACKET_RESULT_REFLECT:
					packet->setSubstructDataByName("header", "result_type", 10);
					break;
				case DAMAGE_PACKET_RESULT_IMMUNE:
					packet->setSubstructDataByName("header", "result_type", 11);
					break;
				}
				packet->setArrayLengthByName("num_dmg", 1);
				packet->setSubstructDataByName("header", "defender_proxy", client->GetPlayer()->GetIDWithPlayerSpawn(victim));
				packet->setArrayDataByName("damage_type", damage_type);
				packet->setArrayDataByName("damage", damage);
			}

			if (!attacker)
				packet->setSubstructDataByName("header", "attacker", 0xFFFFFFFF);
			else
				packet->setSubstructDataByName("header", "attacker", client->GetPlayer()->GetIDWithPlayerSpawn(attacker));
			packet->setSubstructDataByName("header", "defender", client->GetPlayer()->GetIDWithPlayerSpawn(victim));			
			if (spell_name) {
				packet->setDataByName("spell", 1);
				packet->setDataByName("spell_name", spell_name);
			}
			EQ2Packet* app = packet->serialize();
			//DumpPacket(app);
			client->QueuePacket(app);
			safe_delete(packet);
			packet = 0;
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::SendHealPacket(Spawn* caster, Spawn* target, int16 heal_type, int32 heal_amt, const char* spell_name){
	Client* client = 0;
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(!client || (client->GetPlayer() != caster && ((caster && client->GetPlayer()->WasSentSpawn(caster->GetID()) == false) || (target && client->GetPlayer()->WasSentSpawn(target->GetID()) == false))))
			continue;
		if(caster && caster->GetDistance(client->GetPlayer()) > 50)
			continue;
		if(target && target->GetDistance(client->GetPlayer()) > 50)
			continue;
		

		PacketStruct* packet = configReader.getStruct("WS_HearHeal", client->GetVersion());
		if (packet) {
			packet->setDataByName("caster", client->GetPlayer()->GetIDWithPlayerSpawn(caster));
			packet->setDataByName("target", client->GetPlayer()->GetIDWithPlayerSpawn(target));
			packet->setDataByName("heal_amt", heal_amt);
			packet->setDataByName("spellname", spell_name);
			packet->setDataByName("type", heal_type);
			packet->setDataByName("unknown2", 1);
			EQ2Packet* app = packet->serialize();
			client->QueuePacket(app);
			safe_delete(packet);
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::SendThreatPacket(Spawn* caster, Spawn* target, int32 threat_amt, const char* spell_name) {
	Client* client = 0;

	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(!client || (client->GetPlayer() != caster && ((caster && client->GetPlayer()->WasSentSpawn(caster->GetID()) == false) || (target && client->GetPlayer()->WasSentSpawn(target->GetID()) == false))))
			continue;
		if(caster && caster->GetDistance(client->GetPlayer()) > 50)
			continue;
		if(target && target->GetDistance(client->GetPlayer()) > 50)
			continue;
		
		if(client->GetVersion() <= 561) {
			int8 channel = 46;
			
			if(client->GetPlayer() == caster || client->GetPlayer() == target)
				channel = 42;
			
			client->Message(channel, "%s increases %s hate with %s for %u threat.", spell_name, (client->GetPlayer() == caster) ? "YOUR" : caster->GetName(), (client->GetPlayer() == target) ? "YOU" : target->GetName(), threat_amt);
		}
		else {
			PacketStruct* packet = configReader.getStruct("WS_HearThreatCmd", client->GetVersion());
			if (packet) {
				packet->setDataByName("spell_name", spell_name);
				packet->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(caster));
				packet->setDataByName("target", client->GetPlayer()->GetIDWithPlayerSpawn(target));
				packet->setDataByName("threat_amount", threat_amt);
				
				client->QueuePacket(packet->serialize());
			}
			safe_delete(packet);
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}


void ZoneServer::SendYellPacket(Spawn* yeller, float max_distance) {
	Client* client = 0;

					
	string yellMsg = std::string(yeller->GetName()) + " yelled for help!";
	vector<Client*>::iterator client_itr;
	PacketStruct* packet = nullptr;
	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(!client || !client->GetPlayer() || client->GetPlayer()->WasSentSpawn(yeller->GetID()) == false)
			continue;
		if(client->GetPlayer()->GetDistance(yeller) > max_distance)
			continue;
		
		if(packet && packet->GetVersion() == client->GetVersion()) {
			client->QueuePacket(packet->serialize());
		}
		else {
			safe_delete(packet);
			packet = configReader.getStruct("WS_EncounterBroken", client->GetVersion());
			if (packet) {
				packet->setDataByName("message", yellMsg.c_str());
				/* none of the other data seems necessary, keeping for reference for future disassembly
				packet2->setDataByName("unknown2", 0x40);
				packet2->setDataByName("unknown3", 0x40);
				packet2->setDataByName("unknown4", 0xFF);
				packet2->setDataByName("unknown5", 0xFF);
				packet2->setDataByName("unknown6", 0xFF);*/
				client->QueuePacket(packet->serialize());
			}
		}
	}
	safe_delete(packet);
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::SendSpellFailedPacket(Client* client, int16 error){
	if(!client)
		return;

	PacketStruct* packet = configReader.getStruct("WS_DisplaySpellFailed", client->GetVersion());
	if(packet){
		/*		Temp solution, need to modify the error code before this function and while we still have access to the spell/combat art		*/
		error = master_spell_list.GetSpellErrorValue(client->GetVersion(), error);

		if(client->GetVersion() > 373 && client->GetVersion() <= 561 && error) {
			error += 1;
		}
		
		packet->setDataByName("error_code", error);
		//packet->PrintPacket();
		client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
}

void ZoneServer::SendInterruptPacket(Spawn* interrupted, LuaSpell* spell, bool fizzle){
	if(!interrupted || !spell)
		return;
	
	EQ2Packet* outapp = 0;
	PacketStruct* packet = 0;
	Client* client = 0;
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(!client || !client->GetPlayer()->WasSentSpawn(interrupted->GetID()))
			continue;
		packet = configReader.getStruct(fizzle ? "WS_SpellFizzle" : "WS_Interrupt", client->GetVersion());
		if(packet){
			packet->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(interrupted));
			packet->setArrayLengthByName("num_targets", spell->targets.size());
			for (int32 i = 0; i < spell->targets.size(); i++)
				packet->setArrayDataByName("target_id", client->GetPlayer()->GetIDWithPlayerSpawn(client->GetPlayer()->GetZone()->GetSpawnByID(spell->targets[i])), i);
			packet->setDataByName("spell_id", spell->spell->GetSpellID());
			outapp = packet->serialize();
			client->QueuePacket(outapp);
			safe_delete(packet);
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
	safe_delete(packet);
}

void ZoneServer::SendCastSpellPacket(LuaSpell* spell, Entity* caster, int32 spell_visual_override, int16 casttime_override){
	EQ2Packet* outapp = 0;
	PacketStruct* packet = 0;
	Client* client = 0;
	if(!caster || !spell || !spell->spell || spell->interrupted)
		return;

	if(spell->is_damage_spell && (!spell->has_damaged || spell->resisted)) {
		// we did not successfully hit target, so we should not send the visual
		return;
	}
	
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(!client)
			continue;
		
		packet = configReader.getStruct("WS_HearCastSpell", client->GetVersion());
		if(packet){
			int32 caster_id = client->GetPlayer()->GetIDWithPlayerSpawn(caster);
			
			if(!caster_id) {
				safe_delete(packet);
				continue;
			}
			
			packet->setDataByName("spawn_id", caster_id);
			packet->setArrayLengthByName("num_targets", spell->targets.size());
			for (int32 i = 0; i < spell->targets.size(); i++) {
				int32 target_id = client->GetPlayer()->GetIDWithPlayerSpawn(spell->caster->GetZone()->GetSpawnByID(spell->targets[i]));
				if(target_id) {
					packet->setArrayDataByName("target", target_id, i);
				}
				else {
					packet->setArrayDataByName("target", 0xFFFFFFFF, i);
				}
			}
			
			int32 visual_to_use = spell_visual_override > 0 ? spell_visual_override : spell->spell->GetSpellData()->spell_visual;
			int32 visual = client->GetSpellVisualOverride(visual_to_use);
			
			packet->setDataByName("spell_visual", visual); //result
			if(casttime_override != 0xFFFF) {
				packet->setDataByName("cast_time", casttime_override*.01f); //delay
			}
			else {
				packet->setDataByName("cast_time", spell->spell->GetSpellData()->cast_time*.01f); //delay
			}
			packet->setDataByName("spell_id", spell->spell->GetSpellID());
			packet->setDataByName("spell_level", 1);
			packet->setDataByName("spell_tier", spell->spell->GetSpellData()->tier);
			outapp = packet->serialize();
			client->QueuePacket(outapp);
			safe_delete(packet);
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
	safe_delete(packet);
}

void ZoneServer::SendCastSpellPacket(int32 spell_visual, Spawn* target, Spawn* caster) {
	if (target) {
		vector<Client*>::iterator client_itr;

		MClientList.readlock(__FUNCTION__, __LINE__);
		for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
			Client* client = *client_itr;
			if (!client)
				continue;
			PacketStruct* packet = configReader.getStruct("WS_HearCastSpell", client->GetVersion());
			if (packet) {
				
				int32 target_id = client->GetPlayer()->GetIDWithPlayerSpawn(target);
				if(!target_id) { // client is not aware of spawn
					safe_delete(packet);
					continue;
				}
				
				if (!caster) {
					packet->setDataByName("spawn_id", 0xFFFFFFFF);
				}
				else {
					int32 caster_id = client->GetPlayer()->GetIDWithPlayerSpawn(caster);
					
					if(!caster_id) { // client is not aware of spawn
						safe_delete(packet);
						continue;
					}
					packet->setDataByName("spawn_id", caster_id);
				}
				packet->setArrayLengthByName("num_targets", 1);
				packet->setArrayDataByName("target", target_id);

				int32 visual = client->GetSpellVisualOverride(spell_visual);
				
				packet->setDataByName("spell_visual", visual);
				packet->setDataByName("cast_time", 0);
				packet->setDataByName("spell_id", 0);
				packet->setDataByName("spell_level", 0);
				packet->setDataByName("spell_tier", 1);
				client->QueuePacket(packet->serialize());
				safe_delete(packet);
			}
		}
		MClientList.releasereadlock(__FUNCTION__, __LINE__);
	}
}

void ZoneServer::SendCastEntityCommandPacket(EntityCommand* entity_command, int32 spawn_id, int32 target_id) {
	if (entity_command) {
		Spawn* spawn = GetSpawnByID(spawn_id);
		Spawn* target = GetSpawnByID(target_id);
		if (!spawn || !target)
			return;

		Client* client = 0;
		vector<Client*>::iterator client_itr;

		MClientList.readlock(__FUNCTION__, __LINE__);
		for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
			client = *client_itr;
			if (!client || !client->GetPlayer()->WasSentSpawn(spawn_id) || !client->GetPlayer()->WasSentSpawn(target_id))
				continue;
			PacketStruct* packet = configReader.getStruct("WS_HearCastSpell", client->GetVersion());
			if (packet) {
				int32 caster_id = client->GetPlayer()->GetIDWithPlayerSpawn(spawn);
				int32 target_id = client->GetPlayer()->GetIDWithPlayerSpawn(target);
				
				if(!caster_id || !target_id)
					continue;
				
				packet->setDataByName("spawn_id", caster_id);
				packet->setArrayLengthByName("num_targets", 1);
				packet->setArrayDataByName("target", target_id);
				packet->setDataByName("num_targets", 1);
				packet->setDataByName("spell_visual", entity_command->spell_visual); //result
				packet->setDataByName("cast_time", entity_command->cast_time * 0.01); //delay
				packet->setDataByName("spell_id", 1);
				packet->setDataByName("spell_level", 1);
				packet->setDataByName("spell_tier", 1);
				EQ2Packet* outapp = packet->serialize();
				client->QueuePacket(outapp);
				safe_delete(packet);
			}
		}
		MClientList.releasereadlock(__FUNCTION__, __LINE__);
	}
}

void ZoneServer::StartZoneInitialSpawnThread(Client* client){
	if(zoneShuttingDown)
		return;

#ifdef WIN32
	_beginthread(SendInitialSpawns, 0, client);
#else
	pthread_t thread;
	pthread_create(&thread, NULL, SendInitialSpawns, client);
	pthread_detach(thread);
#endif
}

void ZoneServer::SendZoneSpawns(Client* client){
	int8 count = 0;
	while (LoadingData && count <= 6000) { //sleep for max of 60 seconds (60000ms) while the maps are loading
		count++;
		Sleep(10);
	}
	count = 0;
	int16 size = 0;
	//give the spawn thread a tad bit of time to add the pending_spawns to spawn_list (up to 10 seconds)
	while (count < 1000) {		
		MPendingSpawnListAdd.readlock(__FUNCTION__, __LINE__);
		size = pending_spawn_list_add.size();
		MPendingSpawnListAdd.releasereadlock(__FUNCTION__, __LINE__);
		if (size == 0)
			break;
		Sleep(10);
		count++;
	}
	initial_spawn_threads_active++;

	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		Spawn* spawn = itr->second;
		if (spawn) {
			if(spawn == client->GetPlayer() && (client->IsReloadingZone() || client->GetPlayer()->IsReturningFromLD()))
			{
				if(!client->GetPlayer()->SetSpawnMap(spawn))
					continue;
			}
			
			CheckSpawnRange(client, spawn, true);
		}
	}

	CheckSendSpawnToClient(client, true);
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	client->SetConnected(true);
	ClientPacketFunctions::SendFinishedEntitiesList(client);
	initial_spawn_threads_active--;
}

vector<Entity*> ZoneServer::GetPlayers(){
	vector<Entity*> ret;
	Client* client = 0;
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		ret.push_back(client->GetPlayer());
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

int16 ZoneServer::SetSpawnTargetable(Spawn* spawn, float distance){
	Spawn* test_spawn = 0;
	int16 ret_val = 0;
	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		test_spawn = itr->second;
		if(test_spawn){
			if(test_spawn->GetDistance(spawn) <= distance){
				test_spawn->SetTargetable(1);
				ret_val++;
			}
		}
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	return ret_val;
}

int16 ZoneServer::SetSpawnTargetable(int32 spawn_id){
	Spawn* spawn = 0;
	int16 ret_val = 0;
	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if(spawn){
			if(spawn->GetDatabaseID() == spawn_id){
				spawn->SetTargetable(1);
				ret_val++;
			}
		}
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	return ret_val;
}

ZoneInfoSlideStruct* ZoneServer::GenerateSlideStruct(float unknown1a, float unknown1b, int32 unknown2a, int32 unknown2b, int32 unknown3, int32 unknown4, const char* slide, const char* voiceover, int32 key1, int32 key2) {
	ZoneInfoSlideStructInfo* info = new ZoneInfoSlideStructInfo();
	memset(info, 0, sizeof(ZoneInfoSlideStructInfo));
	info->unknown1[0] = unknown1a;
	info->unknown1[1] = unknown1b;
	info->unknown2[0] = unknown2a;
	info->unknown2[1] = unknown2b;
	info->unknown3 = unknown3;
	info->unknown4 = unknown4;
	int8 length = strlen(slide);
	if (length >= 128)
		length = 127;
	strncpy(info->slide, slide, length);
	length = strlen(voiceover);
	if (length >= 128)
		length = 127;
	strncpy(info->voiceover, voiceover, length);
	info->key1 = key1;
	info->key2 = key2;
	ZoneInfoSlideStruct* ret = new ZoneInfoSlideStruct();
	ret->info = info;
	return ret;
}

void ZoneServer::AddZoneInfoSlideStructTransitionInfo(ZoneInfoSlideStruct* info, int32 x, int32 y, float zoom, float transition_time) {
	ZoneInfoSlideStructTransitionInfo* transition_info = new ZoneInfoSlideStructTransitionInfo();
	transition_info->transition_x = x;
	transition_info->transition_y = y;
	transition_info->transition_zoom = zoom;
	transition_info->transition_time = transition_time;
	info->slide_transition_info.push_back(transition_info);
}

vector<ZoneInfoSlideStruct*>* ZoneServer::GenerateTutorialSlides() {
	vector<ZoneInfoSlideStruct*>* slides = new vector<ZoneInfoSlideStruct*>();
	ZoneInfoSlideStruct* slide = GenerateSlideStruct(0.5, 0.5, 15, 15, 1842, 1012, "images/slideshows/boat_06p_tutorial02/lore_chapt01_final001.dds", "voiceover/english/antonia_intro/antonia_intro_001_64.mp3", 2519553957, 1010319376);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 495, 1, 0);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 495, 1.5, 16);
	slides->push_back(slide);

	slide = GenerateSlideStruct(0.5, 0.5, 15, 15, 1842, 1012, "images/slideshows/boat_06p_tutorial02/lore_chapt02_final001.dds", "voiceover/english/antonia_intro/antonia_intro_002_64.mp3", 567178266, 3055063399);
	AddZoneInfoSlideStructTransitionInfo(slide, 600, 365, 1.60000002384186, 0);
	AddZoneInfoSlideStructTransitionInfo(slide, 800, 370, 1.39999997615814, 9);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 420, 1.20000004768372, 8);
	slides->push_back(slide);

	slide = GenerateSlideStruct(0.5, 0.5, 15, 15, 1842, 1012, "images/slideshows/boat_06p_tutorial02/lore_chapt03_final001.dds", "voiceover/english/antonia_intro/antonia_intro_003_64.mp3", 3171561318, 593374281);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 420, 1.20000004768372, 0);
	AddZoneInfoSlideStructTransitionInfo(slide, 750, 320, 1.60000002384186, 10);
	AddZoneInfoSlideStructTransitionInfo(slide, 575, 265, 2.29999995231628, 10);
	slides->push_back(slide);

	slide = GenerateSlideStruct(0.5, 0.5, 15, 15, 1842, 1012, "images/slideshows/boat_06p_tutorial02/lore_chapt04_final001.dds", "voiceover/english/antonia_intro/antonia_intro_004_64.mp3", 1959944485, 4285605574);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 420, 2.5, 0);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 420, 1.70000004768372, 8);
	AddZoneInfoSlideStructTransitionInfo(slide, 675, 390, 2.20000004768372, 11);
	slides->push_back(slide);

	slide = GenerateSlideStruct(0.5, 0.5, 15, 15, 1842, 1012, "images/slideshows/boat_06p_tutorial02/lore_chapt05_final001.dds", "voiceover/english/antonia_intro/antonia_intro_005_64.mp3", 609693392, 260295215);
	AddZoneInfoSlideStructTransitionInfo(slide, 750, 500, 2.79999995231628, 0);
	AddZoneInfoSlideStructTransitionInfo(slide, 720, 300, 2.5, 9);
	AddZoneInfoSlideStructTransitionInfo(slide, 975, 270, 2.20000004768372, 9);
	slides->push_back(slide);

	slide = GenerateSlideStruct(0.5, 0.5, 15, 15, 1842, 1012, "images/slideshows/boat_06p_tutorial02/lore_chapt06_final001.dds", "voiceover/english/antonia_intro/antonia_intro_006_64.mp3", 3056613203, 775201556);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 495, 1.89999997615814, 0);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 475, 1, 24);
	slides->push_back(slide);

	slide = GenerateSlideStruct(0.5, 0.5, 15, 15, 1842, 1012, "images/slideshows/boat_06p_tutorial02/lore_chapt07_final001.dds", "voiceover/english/antonia_intro/antonia_intro_007_64.mp3", 3113327662, 1299367895);
	AddZoneInfoSlideStructTransitionInfo(slide, 1400, 420, 2.40000009536743, 0);
	AddZoneInfoSlideStructTransitionInfo(slide, 1200, 375, 1.70000004768372, 7);
	AddZoneInfoSlideStructTransitionInfo(slide, 800, 225, 2.29999995231628, 7);
	slides->push_back(slide);

	slide = GenerateSlideStruct(0.5, 0.5, 15, 15, 1842, 1012, "images/slideshows/boat_06p_tutorial02/lore_chapt08_final001.dds", "voiceover/english/antonia_intro/antonia_intro_008_64.mp3", 2558791235, 2674773065);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 495, 1, 0);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 495, 1.5, 27);
	slides->push_back(slide);

	slide = GenerateSlideStruct(0.5, 0.5, 15, 15, 1842, 1012, "images/slideshows/boat_06p_tutorial02/lore_chapt09_final001.dds", "voiceover/english/antonia_intro/antonia_intro_009_64.mp3", 4029296401, 1369011033);
	AddZoneInfoSlideStructTransitionInfo(slide, 715, 305, 2.40000009536743, 0);
	AddZoneInfoSlideStructTransitionInfo(slide, 730, 325, 1.79999995231628, 6);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 395, 1.5, 5);
	AddZoneInfoSlideStructTransitionInfo(slide, 1360, 330, 1.79999995231628, 9);
	slides->push_back(slide);

	slide = GenerateSlideStruct(0.5, 0.5, 15, 15, 1842, 1012, "images/slideshows/boat_06p_tutorial02/lore_chapt10_final001.dds", "voiceover/english/antonia_intro/antonia_intro_010_64.mp3", 3055524517, 3787058332);
	AddZoneInfoSlideStructTransitionInfo(slide, 670, 675, 2.20000004768372, 0);
	AddZoneInfoSlideStructTransitionInfo(slide, 710, 390, 1.79999995231628, 7);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 415, 1.60000002384186, 5.5);
	AddZoneInfoSlideStructTransitionInfo(slide, 1250, 675, 1.79999995231628, 8);
	slides->push_back(slide);

	slide = GenerateSlideStruct(0.5, 0.5, 15, 15, 1842, 1012, "images/slideshows/boat_06p_tutorial02/lore_chapt11_final001.dds", "voiceover/english/antonia_intro/antonia_intro_011_64.mp3", 3525586740, 812068950);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 495, 1, 0);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 495, 2, 19);
	slides->push_back(slide);

	slide = GenerateSlideStruct(0.5, 0.5, 15, 15, 1842, 1012, "images/slideshows/boat_06p_tutorial02/lore_chapt12_final001.dds", "voiceover/english/antonia_intro/antonia_intro_012_64.mp3", 3493874350, 2037661816);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 495, 2, 0);
	AddZoneInfoSlideStructTransitionInfo(slide, 920, 495, 1, 43);
	slides->push_back(slide);

	return slides;
}

EQ2Packet* ZoneServer::GetZoneInfoPacket(Client* client){
	PacketStruct* packet = configReader.getStruct("WS_ZoneInfo", client->GetVersion());
	packet->setSmallStringByName("server1",net.GetWorldName());
	packet->setSmallStringByName("server2",net.GetWorldName());
	packet->setDataByName("unknown1", 1, 1);//1, 1
	int32 expansions = EXPANSION_UNKNOWN + EXPANSION_DOF + EXPANSION_KOS + EXPANSION_EOF + EXPANSION_ROK + EXPANSION_TSO + EXPANSION_DOV;
	//packet->setDataByName("expansions_enabled", 82313211);//expansions 63181
	//packet->setDataByName("expansions_enabled", 552075103);//expansions 63182 
	packet->setDataByName("expansions_enabled", 4294967295);//expansions 1096 //612499455 works
	if (client->GetVersion() >= 1193) {
		packet->setDataByName("unknown3", 4294967295, 0); // DOV and down
		packet->setDataByName("unknown3", 4294967295, 1); //COE and up
		packet->setDataByName("unknown3", 4294967295, 2);
	}
	else
		packet->setDataByName("unknown3", 4294967295, 0); // DOV and down

	packet->setSmallStringByName("auction_website", "eq2emulator.net");
	packet->setDataByName("auction_port", 80);
	packet->setSmallStringByName("upload_page", "test_upload.m");
	packet->setSmallStringByName("upload_key", "dsya987yda9");
	packet->setSmallStringByName("zone", GetZoneFile());
	//packet->setSmallStringByName("zone2", GetZoneName());

	//if ( strlen(GetZoneSkyFile()) > 0 )
	//	packet->setSmallStringByName("zone_unknown2", GetZoneSkyFile()); // used for the sky map

	packet->setSmallStringByName("zone_desc", GetZoneDescription());
	packet->setSmallStringByName("char_name", client->GetPlayer()->GetName());
		
	packet->setDataByName("x", client->GetPlayer()->GetX());
	packet->setDataByName("y", client->GetPlayer()->GetY());
	packet->setDataByName("z", client->GetPlayer()->GetZ());

	if ((GetZoneFile() && strcmp("boat_06p_tutorial02", GetZoneFile()) == 0) && client->GetPlayer()->GetX() == this->GetSafeX() && client->GetPlayer()->GetY() == this->GetSafeY() && client->GetPlayer()->GetZ() == this->GetSafeZ()) { //basically the only time the player will see this is if their zone in coords are the exact same as the safe coords (they haven't moved)		
		vector<ZoneInfoSlideStruct*>* slides = GenerateTutorialSlides();
		if (slides) {
			packet->setArrayLengthByName("num_slides", slides->size());
			ZoneInfoSlideStruct* slide = 0;
			for (int8 i = 0; i < slides->size(); i++) {
				slide = slides->at(i);
				packet->setArrayDataByName("unknown1", slide->info->unknown1[0], i, 0);
				packet->setArrayDataByName("unknown1", slide->info->unknown1[1], i, 1);
				packet->setArrayDataByName("unknown2", slide->info->unknown2[0], i, 0);
				packet->setArrayDataByName("unknown2", slide->info->unknown2[1], i, 1);
				packet->setArrayDataByName("unknown3", slide->info->unknown3, i);
				packet->setArrayDataByName("unknown4", slide->info->unknown4, i);
				packet->setArrayDataByName("slide", slide->info->slide, i);
				packet->setArrayDataByName("voiceover", slide->info->voiceover, i);
				packet->setArrayDataByName("key1", slide->info->key1, i);
				packet->setArrayDataByName("key2", slide->info->key2, i);
				packet->setSubArrayLengthByName("num_transitions", slide->slide_transition_info.size(), i);
				for (int8 x = 0; x < slide->slide_transition_info.size(); x++) {
					packet->setSubArrayDataByName("transition_x", slide->slide_transition_info[x]->transition_x, i, x);
					packet->setSubArrayDataByName("transition_y", slide->slide_transition_info[x]->transition_y, i, x);
					packet->setSubArrayDataByName("transition_zoom", slide->slide_transition_info[x]->transition_zoom, i, x);
					packet->setSubArrayDataByName("transition_time", slide->slide_transition_info[x]->transition_time, i, x);
					safe_delete(slide->slide_transition_info[x]);
				}
				safe_delete(slide->info);
				safe_delete(slide);
			}
		}
		safe_delete(slides);
	}

	if(rule_manager.GetZoneRule(GetZoneID(), R_Zone, UseMapUnderworldCoords)->GetBool() && client->GetPlayer()->GetMap()) {
		packet->setDataByName("underworld", client->GetPlayer()->GetMap()->GetMinY() + rule_manager.GetZoneRule(GetZoneID(), R_Zone, MapUnderworldCoordOffset)->GetFloat());
	}
	else {
		packet->setDataByName("underworld", underworld);
	}
	
	// unknown3 can prevent screen shots from being taken if
	//packet->setDataByName("unknown3", 2094661567, 1);			// Screenshots allowed with this value
	//packet->setDataByName("unknown3", 3815767999, 1);			// Screenshots disabled with this value
	//packet->setDataByName("unknown3", 1, 2);

	/*if (client->GetVersion() >= 63587) {
		packet->setArrayLengthByName("num_exp_feature_bytes", 9);
		packet->setArrayDataByName("exp_feature_bytes", 95, 0);//kos and dof
		packet->setArrayDataByName("exp_feature_bytes", 255, 1);//eof rok tso sf dov coe tov
		packet->setArrayDataByName("exp_feature_bytes", 247, 2);//aom tot ka exp14
		packet->setArrayDataByName("exp_feature_bytes", 32, 3);//rum cellar
		packet->setArrayDataByName("exp_feature_bytes", 140, 4);
		packet->setArrayDataByName("exp_feature_bytes", 62, 5);
		packet->setArrayDataByName("exp_feature_bytes", 0, 6);
		packet->setArrayDataByName("exp_feature_bytes", 45, 7);
		packet->setArrayDataByName("exp_feature_bytes", 128, 8);

		packet->setArrayLengthByName("num_unknown3b_bytes", 9);
		packet->setArrayDataByName("unknown3b_bytes", 95, 0);
		packet->setArrayDataByName("unknown3b_bytes", 255, 1);
		packet->setArrayDataByName("unknown3b_bytes", 247, 2);
		packet->setArrayDataByName("unknown3b_bytes", 237, 3);
		packet->setArrayDataByName("unknown3b_bytes", 143, 4);
		packet->setArrayDataByName("unknown3b_bytes", 255, 5);
		packet->setArrayDataByName("unknown3b_bytes", 255, 6);
		packet->setArrayDataByName("unknown3b_bytes", 255, 7);
		packet->setArrayDataByName("unknown3b_bytes", 128, 8);
	}
	else if (client->GetVersion() >= 63214) {
		packet->setArrayLengthByName("num_exp_feature_bytes", 9);
		packet->setArrayDataByName("exp_feature_bytes", 95, 0);//kos and dof
		packet->setArrayDataByName("exp_feature_bytes", 255, 1);//eof rok tso sf dov coe tov
		packet->setArrayDataByName("exp_feature_bytes", 247, 2);//aom tot ka exp14
		packet->setArrayDataByName("exp_feature_bytes", 32, 3);//rum cellar
		packet->setArrayDataByName("exp_feature_bytes", 140, 4);
		packet->setArrayDataByName("exp_feature_bytes", 62, 5);
		packet->setArrayDataByName("exp_feature_bytes", 0, 6);
		packet->setArrayDataByName("exp_feature_bytes", 45, 7);
		packet->setArrayDataByName("exp_feature_bytes", 128, 8);

		packet->setArrayLengthByName("num_unknown3b_bytes", 9);
		packet->setArrayDataByName("unknown3b_bytes", 95, 0);
		packet->setArrayDataByName("unknown3b_bytes", 255, 1);
		packet->setArrayDataByName("unknown3b_bytes", 247, 2);
		packet->setArrayDataByName("unknown3b_bytes", 237, 3);
		packet->setArrayDataByName("unknown3b_bytes", 143, 4);
		packet->setArrayDataByName("unknown3b_bytes", 255, 5);
		packet->setArrayDataByName("unknown3b_bytes", 255, 6);
		packet->setArrayDataByName("unknown3b_bytes", 255, 7);
		packet->setArrayDataByName("unknown3b_bytes", 128, 8);
	}*/
	if (client->GetVersion() >= 64644) {
		packet->setDataByName("unknown3a", 12598924);
		packet->setDataByName("unknown3b", 3992452959);
		packet->setDataByName("unknown3c", 4294967183);
		packet->setDataByName("unknown2a", 9);
		packet->setDataByName("unknown2b", 9);
	}
	else if (client->GetVersion() >= 63181) {
		packet->setDataByName("unknown3a", 750796556);//63182 73821356
		packet->setDataByName("unknown3b", 3991404383);// 63182 3991404383
		packet->setDataByName("unknown3c", 4278189967);// 63182 4278189967
		packet->setDataByName("unknown2a", 8);// 63182
		packet->setDataByName("unknown2b", 8);// 63182
		
	}
	else{
		//packet->setDataByName("unknown3", 872447025,0);//63181 
		//packet->setDataByName("unknown3", 3085434875,1);// 63181 
		//packet->setDataByName("unknown3", 2147483633,2);// 63181 
	}
	
	packet->setDataByName("year", world.GetWorldTimeStruct()->year);
	packet->setDataByName("month", world.GetWorldTimeStruct()->month);
	packet->setDataByName("day", world.GetWorldTimeStruct()->day);
	packet->setDataByName("hour", world.GetWorldTimeStruct()->hour);
	packet->setDataByName("minute", world.GetWorldTimeStruct()->minute);
	packet->setDataByName("unknown", 0);
	packet->setDataByName("unknown7", 1);
	packet->setDataByName("unknown7", 1, 1);
	
	packet->setDataByName("unknown9", 13);
	//packet->setDataByName("unknown10", 25188959);4294967295
	//packet->setDataByName("unknown10", 25190239);
	packet->setDataByName("unknown10", 25191524);//25191524
	packet->setDataByName("unknown10b", 1);
	packet->setDataByName("permission_level",3);// added on 63182  for now till we figur it out 0=none,1=visitor,2=friend,3=trustee,4=owner
	packet->setDataByName("num_adv", 9);

	packet->setArrayDataByName("adv_name", "adv02_dun_drowned_caverns", 0);
	packet->setArrayDataByName("adv_id", 6, 0);
	packet->setArrayDataByName("adv_name", "adv02_dun_sundered_splitpaw_hub", 1);
	packet->setArrayDataByName("adv_id", 5, 1);
	packet->setArrayDataByName("adv_name", "exp03_rgn_butcherblock", 2);
	packet->setArrayDataByName("adv_id", 8, 2);
	packet->setArrayDataByName("adv_name", "exp03_rgn_greater_faydark", 3);
	packet->setArrayDataByName("adv_id", 7, 3);
	packet->setArrayDataByName("adv_name", "mod01_dun_crypt_of_thaen", 4);
	packet->setArrayDataByName("adv_id", 3, 4);
	packet->setArrayDataByName("adv_name", "mod01_dun_tombs_of_night", 5);
	packet->setArrayDataByName("adv_id", 4, 5);
	packet->setArrayDataByName("adv_name", "nektulos_mini01", 6);
	packet->setArrayDataByName("adv_id", 0, 6);
	packet->setArrayDataByName("adv_name", "nektulos_mini02", 7);
	packet->setArrayDataByName("adv_id", 1, 7);
	packet->setArrayDataByName("adv_name", "nektulos_mini03", 8);
	packet->setArrayDataByName("adv_id", 2, 8);	




	LogWrite(MISC__TODO, 0, "TODO", "Put cl_ client commands back in variables (not Rules) so they can be dynamically maintained");
	vector<Variable*>* variables = world.GetClientVariables();
	packet->setArrayLengthByName("num_client_setup", variables->size());
	for(int i=variables->size()-1;i>=0;i--)
		packet->setArrayDataByName("client_cmds", variables->at(i)->GetNameValuePair().c_str(), i);

	// For AoM clients so item link work
	if (client->GetVersion() >= 60114)
		packet->setArrayDataByName("client_cmds", "chat_linkbrackets_item 1", variables->size());

	safe_delete(variables);
	//packet->setDataByName("unknown8", ); story?
	// AA Tabs for 1193+ clients
	if (client->GetVersion() >= 1193) {
		packet->setArrayLengthByName("tab_count", 48);
		int8 i = 0;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":es24a58bd8fcaac8c2:All", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c727bd47a6:Racial Innate", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c75a96e23c:Tradeskill Advancement", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c744f1fd99:Focus Effects", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c71edd2a66:Heroic", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c76ee6239f:Shadows", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7e678b977:Prestige", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c77ee422d7:Animalist", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7f165af77:Bard", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7421b9375:Brawler", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7a03ae7d1:Cleric", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7c9605e9f:Crusader", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7f9424168:Druid", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c79cb9556c:Enchanter", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c70c8b6aa4:Predator", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c73a43b6dd:Rogue", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c759fe7d15:Sorcerer", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7ad610aca:Summoner", i);
		
		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c71e056728:Warrior", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7ba864c0b:Assassin", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7b8116aad:Beastlord", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7f53feb7b:Berserker", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c73d8a70e2:Brigand", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c770c766d6:Bruiser", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c79226984b:Coercer", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c70c58bb30:Conjurer", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c73dfe68d0:Defiler", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c792919a6b:Dirge", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7062e5f55:Fury", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c762c1fdfc:Guardian", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c78addfbf4:Illusionist", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7ece054a7:Inquisitor", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7d550d2e7:Monk", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c743cfeaa2:Mystic", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7f63c9c8c:Necromancer", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c70c5de0ae:Paladin", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c79bc97b3a:Ranger", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c78fbd2256:Shadowknight", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7781cc625:Shaman", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c77eecdcdb:Swashbuckler", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7648d181e:Templar", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c78df47d77:Troubador", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7c78ce0b8:Warden", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c76290dcfa:Warlock", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7d1d52cf5:Wizard", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c71c8f6f4d:Shaper", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c72f6e354d:Channeler", i);

		i++;
		packet->setArrayDataByName("tab_index", i, i);
		packet->setArrayDataByName("tab_name", ":410385c7df8bd37d:Dragon", i);
		}
		packet->setDataByName("unknown_mj", 1);//int8
		packet->setDataByName("unknown_mj1", 335544320);//int32
		packet->setDataByName("unknown_mj2", 4);//int32
		packet->setDataByName("unknown_mj3", 3962504088);//int32
		packet->setDataByName("unknown_mj4", 3985947216);//int32
		packet->setDataByName("unknown_mj5", 1);//int32
		packet->setDataByName("unknown_mj6", 386);//int32
		packet->setDataByName("unknown_mj7", 4294967295);//int32
		packet->setDataByName("unknown_mj8", 2716312211);//int32
		packet->setDataByName("unknown_mj9", 1774338333);//int32
		packet->setDataByName("unknown_mj10", 1);//int32
		packet->setDataByName("unknown_mj11", 391);//int32
		packet->setDataByName("unknown_mj12", 4294967295);//int32
		packet->setDataByName("unknown_mj13", 3168965163);//int32
		packet->setDataByName("unknown_mj14", 4117025286);//int32
		packet->setDataByName("unknown_mj15", 1);//int32
		packet->setDataByName("unknown_mj16", 394);//int32
		packet->setDataByName("unknown_mj17", 4294967295);//int32
		packet->setDataByName("unknown_mj18", 1790669110);//int32
		packet->setDataByName("unknown_mj19", 107158108);//int32
		packet->setDataByName("unknown_mj20", 1);//int32
		packet->setDataByName("unknown_mj21", 393);//int32
		packet->setDataByName("unknown_mj22", 4294967295);//int32

	EQ2Packet* outapp = packet->serialize();
	//packet->PrintPacket();
	//DumpPacket(outapp);
	safe_delete(packet);
	return outapp;
}

void ZoneServer::SendUpdateDefaultCommand(Spawn* spawn, const char* command, float distance, Spawn* toPlayer){
	if (spawn == nullptr || command == nullptr)
		return;

	if (toPlayer)
	{
		if (!toPlayer->IsPlayer())
			return;

		Client* client = ((Player*)toPlayer)->GetClient();
		if (client)
		{
			client->SendDefaultCommand(spawn, command, distance);
		}
		// we don't override the primary command cause that would change ALL clients
		return;
	}

	QueueDefaultCommand(spawn->GetID(), std::string(command), distance);

	if (strlen(command)>0)
		spawn->SetPrimaryCommand(command, command, distance);
}

void ZoneServer::CheckPlayerProximity(Spawn* spawn, Client* client){
	if (player_proximities.size() < 1)
		return;

	if(player_proximities.count(spawn->GetID()) > 0){
		PlayerProximity* prox = player_proximities.Get(spawn->GetID());
		if(prox->clients_in_proximity.count(client) == 0 && spawn_range_map.count(client) > 0 && spawn_range_map.Get(client)->count(spawn->GetID()) > 0 && spawn_range_map.Get(client)->Get(spawn->GetID()) < prox->distance){
			prox->clients_in_proximity[client] = true;
			CallSpawnScript(spawn, SPAWN_SCRIPT_CUSTOM, client->GetPlayer(), prox->in_range_lua_function.c_str());
		}
		else if(prox->clients_in_proximity.count(client) > 0 && spawn_range_map.count(client) > 0 && spawn_range_map.Get(client)->count(spawn->GetID()) > 0 && spawn_range_map.Get(client)->Get(spawn->GetID()) > prox->distance){
			if(prox->leaving_range_lua_function.length() > 0)
				CallSpawnScript(spawn, SPAWN_SCRIPT_CUSTOM, client->GetPlayer(), prox->leaving_range_lua_function.c_str());
			prox->clients_in_proximity.erase(client);
		}
	}
}

void ZoneServer::AddPlayerProximity(Spawn* spawn, float distance, string in_range_function, string leaving_range_function){
	RemovePlayerProximity(spawn);
	PlayerProximity* prox = new PlayerProximity;
	prox->distance = distance;
	prox->in_range_lua_function = in_range_function;
	prox->leaving_range_lua_function = leaving_range_function;
	player_proximities.Put(spawn->GetID(), prox);
}

void ZoneServer::RemovePlayerProximity(Client* client){	
	PlayerProximity* prox = 0;
	MutexMap<int32, PlayerProximity*>::iterator itr = player_proximities.begin();
	while(itr.Next()){
		prox = itr->second;
		if(prox->clients_in_proximity.count(client) > 0)
			prox->clients_in_proximity.erase(client);
	}
}

void ZoneServer::RemovePlayerProximity(Spawn* spawn, bool all){
	if(all){
		MutexMap<int32, PlayerProximity*>::iterator itr = player_proximities.begin();
		while(itr.Next()){
			player_proximities.erase(itr->first, false, true, 10000);
		}
	}
	else if(player_proximities.count(spawn->GetID()) > 0){
		player_proximities.erase(spawn->GetID(), false, true, 10000);
	}
}

void ZoneServer::AddLocationProximity(float x, float y, float z, float max_variation, string in_range_function, string leaving_range_function) {
	LocationProximity* prox = new LocationProximity;
	prox->x = x;
	prox->y = y;
	prox->z = z;
	prox->max_variation = max_variation;
	prox->in_range_lua_function = in_range_function;
	prox->leaving_range_lua_function = leaving_range_function;
	location_proximities.Add(prox);
}

void ZoneServer::CheckLocationProximity() {
	const char* zone_script = world.GetZoneScript(this->GetZoneID());
	if (!zone_script)
		return;

	if (location_proximities.size() > 0 && connected_clients.size() > 0) {
		Client* client = 0;
		MutexList<Client*>::iterator iterator = connected_clients.begin();
		while(iterator.Next()){
			client = iterator->value;
			if (client->IsConnected() && client->IsReadyForUpdates() && !client->IsZoning()) {
				try {
					MutexList<LocationProximity*>::iterator itr = location_proximities.begin();
					LocationProximity* prox = 0;
					while(itr.Next()){
						prox = itr->value;
						bool in_range = false;
						float char_x = client->GetPlayer()->GetX();
						float char_y = client->GetPlayer()->GetY();
						float char_z = client->GetPlayer()->GetZ();
						float x = prox->x;
						float y = prox->y;
						float z = prox->z;
						float max_variation = prox->max_variation;
						float total_diff = 0;
						float diff = x - char_x; //Check X
						if(diff < 0)
							diff *= -1;
						if(diff <=  max_variation) {
							total_diff += diff;
							diff = z - char_z; //Check Z (we check Z first because it is far more likely to be a much greater variation than y)
							if(diff < 0)
								diff *= -1;
							if(diff <=  max_variation) {
								total_diff += diff;
								if(total_diff <=  max_variation) { //Check Total
									diff = y - char_y; //Check Y
									if(diff < 0)
										diff *= -1;
									if(diff <=  max_variation) {
										total_diff += diff;
										if(total_diff <= max_variation) {
											in_range = true;
											if(lua_interface && prox->in_range_lua_function.length() > 0 && prox->clients_in_proximity.count(client) == 0) { //Check Total
												prox->clients_in_proximity[client] = true;
												lua_interface->RunZoneScript(zone_script, prox->in_range_lua_function.c_str(), this, client->GetPlayer());
											}
										}
									}
								}
							}
						}
						if (!in_range) {
							if(lua_interface && prox->leaving_range_lua_function.length() > 0 && prox->clients_in_proximity.count(client) > 0) {
								lua_interface->RunZoneScript(zone_script, prox->leaving_range_lua_function.c_str(), this, client->GetPlayer());
								prox->clients_in_proximity.erase(client);
							}
						}
					}
				}
				catch (...) {
					LogWrite(ZONE__ERROR, 0, "Zone", "Except caught in ZoneServer::CheckLocationProximity");
					return;
				}
			}
		}
	}
}

void ZoneServer::CheckLocationGrids() {
	if (connected_clients.size() > 0 && location_grids.size() > 0) {
		MutexList<Client*>::iterator client_itr = connected_clients.begin();
		while (client_itr.Next()) {
			Client* client = client_itr.value;
			if (!client)
				continue;
			Player* player = client->GetPlayer();
			float x = player->GetX();
			float y = player->GetY();
			float z = player->GetZ();
			int32 grid_id = player->GetLocation();
			MutexList<LocationGrid*>::iterator location_grid_itr = location_grids.begin();
			while (location_grid_itr.Next()) {
				LocationGrid* grid = location_grid_itr.value;
				bool playerInGrid = false;
				if (grid->locations.size() > 0 || (playerInGrid = (grid->grid_id > 0 && grid->grid_id == grid_id)) || grid->players.count(player) > 0) {
					float x_small = 0;
					float x_large = 0;
					float y_small = 0;
					float y_large = 0;
					float z_small = 0;
					float z_large = 0;
					bool first = true;
					bool in_grid = false;
					if(grid->locations.size() == 0 && playerInGrid) { // no locations, we presume player is in grid
						in_grid = true;
					}
					else {
						MutexList<Location*>::iterator location_itr = grid->locations.begin();
						while (location_itr.Next()) {
							Location* location = location_itr.value;
							if (first) {
								x_small = location->x;
								x_large = location->x;
								if (grid->include_y) {
									y_small = location->y;
									y_large = location->y;
								}
								z_small = location->z;
								z_large = location->z;
								first = false;
							}
							else {
								if (location->x < x_small)
									x_small = location->x;
								else if (location->x > x_large)
									x_large = location->x;
								if (grid->include_y) {
									if (location->y < y_small)
										y_small = location->y;
									else if (location->y > y_large)
										y_large = location->y;
								}
								if (location->z < z_small)
									z_small = location->z;
								else if (location->z > z_large)
									z_large = location->z;
							}
						}
						if (grid->include_y && (x >= x_small && x <= x_large && y >= y_small && y <= y_large && z >= z_small && z <= z_large))
							in_grid = true;
						else if (x >= x_small && x <= x_large && z >= z_small && z <= z_large)
							in_grid = true;
					}
					if (in_grid && grid->players.count(player) == 0) {
						grid->players.Put(player, true);

						bool show_enter_location_popup = true;
						bool discovery_enabled = rule_manager.GetZoneRule(GetZoneID(), R_World, EnablePOIDiscovery)->GetBool();

						if( grid->discovery && discovery_enabled && !player->DiscoveredLocation(grid->id) )
						{
							// check if player has already discovered this location

							// if not, process new discovery
							char tmp[200] = {0};
							sprintf(tmp, "\\#FFE400You have discovered\12\\#FFF283%s", grid->name.c_str());
							client->SendPopupMessage(11, tmp, "ui_discovery", 2.25, 0xFF, 0xFF, 0xFF);
							LogWrite(ZONE__DEBUG, 0, "Zone", "Player '%s' discovered location '%s' (%u)", player->GetName(), grid->name.c_str(), grid->id);

							player->UpdatePlayerHistory(HISTORY_TYPE_DISCOVERY, HISTORY_SUBTYPE_LOCATION, grid->id);
							show_enter_location_popup = false;

							// else, print standard location entry
						}

						if( show_enter_location_popup )
						{
							LogWrite(ZONE__DEBUG, 0, "Zone", "Player '%s' entering location '%s' (%u)", player->GetName(), grid->name.c_str(), grid->id);
							client->SendPopupMessage(10, grid->name.c_str(), 0, 2.5, 255, 255, 0);
						}
					}
					else if (!in_grid && grid->players.count(player) > 0) {
						LogWrite(ZONE__DEBUG, 0, "Zone", "Player '%s' leaving location '%s' (%u)", player->GetName(), grid->name.c_str(), grid->id);
						grid->players.erase(player);
					}
				}
			}
		}
	}
}

// Called from a command (client, main zone thread) and the main zone thread
// so no need for a mutex container
void ZoneServer::AddLocationGrid(LocationGrid* grid) {
	if (grid)
		location_grids.Add(grid);
}

void ZoneServer::RemoveLocationGrids() {
	MutexList<LocationGrid*>::iterator itr = location_grids.begin();
	while (itr.Next())
		itr.value->locations.clear(true);
	location_grids.clear(true);
}

void ZoneServer::RemoveSpellTimersFromSpawn(Spawn* spawn, bool remove_all, bool delete_recast, bool call_expire_function, bool lock_spell_process){
	if(spellProcess)
		spellProcess->RemoveSpellTimersFromSpawn(spawn, remove_all, delete_recast, call_expire_function, lock_spell_process);
}

void ZoneServer::Interrupted(Entity* caster, Spawn* interruptor, int16 error_code, bool cancel, bool from_movement){
	if(spellProcess)
		spellProcess->Interrupted(caster, interruptor, error_code, cancel, from_movement);
}

Spell* ZoneServer::GetSpell(Entity* caster){
	Spell* spell = 0;
	if(spellProcess)
		spell = spellProcess->GetSpell(caster);
	return spell;
}

void ZoneServer::ProcessSpell(Spell* spell, Entity* caster, Spawn* target, bool lock, bool harvest_spell, LuaSpell* customSpell, int16 custom_cast_time, bool in_heroic_opp){
	if(spellProcess)
		spellProcess->ProcessSpell(this, spell, caster, target, lock, harvest_spell, customSpell, custom_cast_time, in_heroic_opp);
}

void ZoneServer::ProcessEntityCommand(EntityCommand* entity_command, Entity* caster, Spawn* target, bool lock) {
	if (target && target->GetSpawnScript()) {
		Player* player = 0;
		if (caster && caster->IsPlayer())
			player = (Player*)caster;
		CallSpawnScript(target, SPAWN_SCRIPT_CUSTOM, player, entity_command->command.c_str());
	}
	if (spellProcess)
		spellProcess->ProcessEntityCommand(this, entity_command, caster, target, lock);
}

void ZoneServer::RemoveSpawnSupportFunctions(Spawn* spawn, bool lock_spell_process, bool shutdown) {
	if(!spawn)
		return;	
	
	// remove entity* in trade class
	if(spawn->IsEntity()) {
		((Entity*)spawn)->TerminateTrade();
	}
	
	if(spawn->IsPlayer() && spawn->GetZone()) {
		spawn->GetZone()->RemovePlayerPassenger(((Player*)spawn)->GetCharacterID());
		if(((Player*)spawn)->GetClient()) {
			GetTradeskillMgr()->StopCrafting(((Player*)spawn)->GetClient());
		}
	}
	if(spawn->IsEntity())
		RemoveSpellTimersFromSpawn((Entity*)spawn, true, true, true, lock_spell_process);
	
	if(!shutdown) { // in case of shutdown, DeleteData(true) handles the cleanup later via DeleteSpawnScriptTimers
		StopSpawnScriptTimer(spawn, "");
	}
	
	if(spawn->IsEntity()) {
		ClearHate((Entity*)spawn);
	}

	RemoveDamagedSpawn(spawn);
	spawn->SendSpawnChanges(false);
	RemoveChangedSpawn(spawn);
	
	// Everything inside this if will be nuked during a reload in other spots, no need to do it twice
	if (!reloading) {
		RemoveDeadEnemyList(spawn);

		spawn->changed = true;
		spawn->info_changed = true;
		spawn->vis_changed = true;
		spawn->position_changed = true;
		SendSpawnChanges(spawn);

		if (spawn->GetSpawnGroupID() > 0) {
			int32 group_id = spawn->GetSpawnGroupID();
			spawn->RemoveSpawnFromGroup(false, (spawn->IsEntity() && !spawn->Alive()) ? true : false);
			if (spawn_group_map.count(group_id) > 0)
				spawn_group_map.Get(group_id).Remove(spawn->GetID());
		}

		if (!spawn->IsPlayer()) {
			if(quick_database_id_lookup.count(spawn->GetDatabaseID()) > 0)
				quick_database_id_lookup.erase(spawn->GetDatabaseID());
			
			if(spawn->GetSpawnLocationID() > 0 && quick_location_id_lookup.count(spawn->GetSpawnLocationID()) > 0 && quick_location_id_lookup.Get(spawn->GetSpawnLocationID()) == spawn->GetID())
				quick_location_id_lookup.erase(spawn->GetSpawnLocationID());
			
			if(spawn->GetSpawnGroupID() > 0 && quick_group_id_lookup.count(spawn->GetSpawnGroupID()) > 0 && quick_group_id_lookup.Get(spawn->GetSpawnGroupID()) == spawn->GetID())
				quick_group_id_lookup.erase(spawn->GetSpawnGroupID());
		}

		DeleteSpawnScriptTimers(spawn);
		RemovePlayerProximity(spawn);
	}

	// We don't use RemoveMovementNPC() here as it caused a hell of a delay during reloads
	// instead we remove it from the list directly
	if (spawn->IsNPC())
		movement_spawns.erase(spawn->GetID());
}

void ZoneServer::HandleEmote(Spawn* originator, string name, Spawn* opt_target, bool no_target) {
	if (!originator) {
		LogWrite(ZONE__ERROR, 0, "Zone", "HandleEmote called with an invalid client");
		return;
	}

	Spawn* target = originator->GetTarget();
	if(opt_target)
		target = opt_target;
	
	if(no_target) // override having a target
		target = nullptr;
		
	Client* orig_client = (originator->IsPlayer() && ((Player*)originator)->GetClient()) ? ((Player*)originator)->GetClient() : nullptr;
	Client* client = 0;
	int32 cur_client_version = orig_client ? orig_client->GetVersion() : 546;
	Emote* origEmote = visual_states.FindEmote(name, cur_client_version);
	if(!origEmote){
		if(orig_client) {
			orig_client->Message(CHANNEL_COLOR_YELLOW, "Unable to find emote '%s'.  If this should be a valid emote be sure to submit a /bug report.", name.c_str());
		}
		return;
	}
	Emote* emote = origEmote;

	PacketStruct* packet = 0;
	char* emoteResponse = 0;
	vector<Client*>::iterator client_itr;
	
	map<int32, Emote*> emote_version_range;
	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(!client || (client && originator->IsPlayer() && client->GetPlayer()->IsIgnored(originator->GetName())))
			continue;

		// establish appropriate emote for the version used by the client
		if (client->GetVersion() != cur_client_version)
		{
			map<int32, Emote*>::iterator rangeitr = emote_version_range.find(client->GetVersion());
			if (rangeitr == emote_version_range.end())
			{
				Emote* tmp_new_emote = visual_states.FindEmote(name, client->GetVersion());
				if (tmp_new_emote)
				{
					emote_version_range.insert(make_pair(client->GetVersion(), tmp_new_emote));
					emote = tmp_new_emote;
				} // else its missing just use the current clients default
			}
			else // we have an existing emote already cached
				emote = rangeitr->second;
		}
		else // since the client and originator client match use the original emote
			emote = origEmote;

		packet = configReader.getStruct("WS_CannedEmote", client->GetVersion());
		if(packet){
			packet->setDataByName("spawn_id" , client->GetPlayer()->GetIDWithPlayerSpawn(originator));
			if(!emoteResponse){
				string message;
				if(target && target->GetID() != originator->GetID()){
					message = emote->GetTargetedMessageString();
					if(message.find("%t") < 0xFFFFFFFF)
						message.replace(message.find("%t"), 2, target->GetName());
				}
				if(message.length() == 0)
					message = emote->GetMessageString();
				if(message.find("%g1") < 0xFFFFFFFF){
					if(originator->GetGender() == 1)
						message.replace(message.find("%g1"), 3, "his");
					else
						message.replace(message.find("%g1"), 3, "her");
				}
				if(message.find("%g2") < 0xFFFFFFFF){
					if(originator->GetGender() == 1)
						message.replace(message.find("%g2"), 3, "him");
					else
						message.replace(message.find("%g2"), 3, "her");
				}
				if(message.find("%g3") < 0xFFFFFFFF){
					if(originator->GetGender() == 1)
						message.replace(message.find("%g3"), 3, "he");
					else
						message.replace(message.find("%g3"), 3, "she");
				}
				if(message.length() > 0){
					emoteResponse = new char[message.length() + strlen(originator->GetName()) + 10];
					sprintf(emoteResponse,"%s %s", originator->GetName(), message.c_str());
				}
			}
			if(originator->IsPlayer()) {
				packet->setMediumStringByName("emote_msg", emoteResponse ? emoteResponse : "");
			}
			packet->setDataByName("anim_type", emote->GetVisualState());
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
			safe_delete_array(emoteResponse);
		}	
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}


void ZoneServer::SetupInstance(int32 createdInstanceID) {
	if ( createdInstanceID == 0 ) // if this happens that isn't good!
		instanceID = ++MinInstanceID;
	else // db should pass the good ID
		instanceID = createdInstanceID;
}

void ZoneServer::RemoveDeadSpawn(Spawn* spawn){
	AddDeadSpawn(spawn, 0);
}

void ZoneServer::AddDeadSpawn(Spawn* spawn, int32 timer){
	MDeadSpawns.writelock(__FUNCTION__, __LINE__);
	if (dead_spawns.count(spawn->GetID()) > 0)
		dead_spawns[spawn->GetID()] = Timer::GetCurrentTime2() + timer;
	else if(timer != 0xFFFFFFFF)
		dead_spawns.insert(make_pair(spawn->GetID(), Timer::GetCurrentTime2() + timer));
	else{
		if(spawn->IsEntity() && spawn->HasLoot()){
			dead_spawns.insert(make_pair(spawn->GetID(), Timer::GetCurrentTime2() + (15000 * spawn->GetLevel() + 240000)));
			SendUpdateDefaultCommand(spawn, "loot", 10);
		}
		else
			dead_spawns.insert(make_pair(spawn->GetID(), Timer::GetCurrentTime2() + 10000));
	}
	MDeadSpawns.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::WritePlayerStatistics() {
	MutexList<Client*>::iterator client_itr = connected_clients.begin();
	while(client_itr.Next())
		client_itr->value->GetPlayer()->WritePlayerStatistics();
}

bool ZoneServer::SendRadiusSpawnInfo(Client* client, float radius) {
	if (!client)
		return false;

	Spawn* spawn = 0;
	bool ret = false;
	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if (spawn && spawn != client->GetPlayer() && !spawn->IsPlayer() && spawn->GetDistance(client->GetPlayer()) <= radius) {
			const char* type = "NPC";
			const char* specialTypeID = "N/A";
			int32 specialID = 0, spawnEntryID = spawn->GetSpawnEntryID();
			if (spawn->IsObject())
			{
				Object* obj = (Object*)spawn;
				specialID = obj->GetID();
				specialTypeID = "GetID";
				type = "Object";
			}
			else if (spawn->IsSign())
			{
				Sign* sign = (Sign*)spawn;
				specialID = sign->GetWidgetID();
				specialTypeID = "WidgetID";
				type = "Sign";
			}
			else if (spawn->IsWidget())
			{
				Widget* widget = (Widget*)spawn;
				specialID = widget->GetWidgetID();
				specialTypeID = "WidgetID";
				if ( specialID == 0xFFFFFFFF )
					specialTypeID = "WidgetID(spawn_widgets entry missing)";

				type = "Widget";
			}
			else if (spawn->IsGroundSpawn())
			{
				GroundSpawn* gs = (GroundSpawn*)spawn;
				specialID = gs->GetGroundSpawnEntryID();
				specialTypeID = "GroundSpawnEntryID";
				type = "GroundSpawn";
			}
			client->Message(CHANNEL_COLOR_RED, "Name: %s (%s), Spawn Table ID: %u, %s: %u", spawn->GetName(), type, spawn->GetDatabaseID(), specialTypeID, specialID);
			client->Message(CHANNEL_COLOR_RED, "Spawn Location ID: %u, Spawn Group ID: %u, SpawnEntryID: %u, Grid ID: %u", spawn->GetSpawnLocationID(), spawn->GetSpawnGroupID(), spawnEntryID, spawn->GetLocation());
			client->Message(CHANNEL_COLOR_RED, "Respawn Time: %u (sec), X: %f, Y: %f, Z: %f Heading: %f", spawn->GetRespawnTime(), spawn->GetX(), spawn->GetY(), spawn->GetZ(), spawn->GetHeading());
			client->Message(CHANNEL_COLOR_YELLOW, "=============================");
			ret = true;
		}
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

void ZoneServer::FindSpawn(Client* client, char* regSearchStr)
{
	if (!regSearchStr || strlen(regSearchStr) < 1)
	{
		client->SimpleMessage(CHANNEL_COLOR_RED, "Bad ZoneServer::FindSpawn(Client*, char* regSearchStr) attempt, regSearchStr is NULL or empty.");
		return;
	}

	string resString = string(regSearchStr);
	try
	{
		std::regex pre_re_check("^[a-zA-Z0-9_ ]+$");
		bool output = std::regex_match(resString, pre_re_check);
		if (output)
		{
			string newStr(".*");
			newStr.append(regSearchStr);
			newStr.append(".*");
			resString = newStr;
		}
	}
	catch (...)
	{
		client->SimpleMessage(CHANNEL_COLOR_RED, "Try/Catch ZoneServer::FindSpawn(Client*, char* regSearchStr) failure.");
		return;
	}
	std::regex re;
	try {
		re = std::regex(resString, std::regex_constants::icase);
	}
	catch(...) {
		client->SimpleMessage(CHANNEL_COLOR_RED, "Invalid regex for FindSpawn.");
		return;
	}
	
	client->Message(CHANNEL_NARRATIVE, "RegEx Search Spawn List: %s", regSearchStr);
	client->Message(CHANNEL_NARRATIVE, "Database ID | Spawn Name | X , Y , Z");
	client->Message(CHANNEL_NARRATIVE, "========================");
	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	int32 spawnsFound = 0;
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		Spawn* spawn = itr->second;
		if (!spawn || !spawn->GetName())
			continue;
		bool output = false;
		try {
			output = std::regex_match(string(spawn->GetName()), re);
		}
		catch (...)
		{
			continue;
		}

		if (output)
		{
			client->Message(CHANNEL_NARRATIVE, "%i | %s | %f , %f , %f", spawn->GetDatabaseID(), spawn->GetName(), spawn->GetX(), spawn->GetY(), spawn->GetZ());
			spawnsFound++;
		}
	}
	client->Message(CHANNEL_NARRATIVE, "========================", spawnsFound);
	client->Message(CHANNEL_NARRATIVE, "%u Results Found.", spawnsFound);
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
}


void ZoneServer::AddPlayerTracking(Player* player) {
	if (player && !player->GetIsTracking() && players_tracking.count(player->GetDatabaseID()) == 0) {
		Client* client = ((Player*)player)->GetClient();
		if (client) {
			PacketStruct* packet = configReader.getStruct("WS_TrackingUpdate", client->GetVersion());
			if (packet) {
				player->SetIsTracking(true);
				players_tracking.Put(client->GetCharacterID(), player);
				packet->setDataByName("mode", TRACKING_START);
				packet->setDataByName("type", TRACKING_TYPE_ENTITIES);
				client->QueuePacket(packet->serialize());
				safe_delete(packet);
			}
		}
	}
}

void ZoneServer::RemovePlayerTracking(Player* player, int8 mode) {
	if (player && player->GetIsTracking()) {
		Client* client = ((Player*)player)->GetClient();
		if (client) {
			PacketStruct* packet = configReader.getStruct("WS_TrackingUpdate", client->GetVersion());
			if (packet) {
				player->SetIsTracking(false);
				players_tracking.erase(client->GetCharacterID());
				packet->setDataByName("mode", mode);
				packet->setDataByName("type", TRACKING_TYPE_ENTITIES);
				client->QueuePacket(packet->serialize());
				safe_delete(packet);
			}
		}
	}
}

void ZoneServer::ProcessTracking() {
	MutexMap<int32, Player*>::iterator itr = players_tracking.begin();
	while (itr.Next()) {
		Player* player = itr->second;
		if(player->GetClient()) {
			ProcessTracking(player->GetClient());
		}
	}
}

void ZoneServer::ProcessTracking(Client* client) {
	if (!client)
		return;

	Player* player = client->GetPlayer();
	if (player && player->GetIsTracking()) {
		MutexMap<int32, Spawn*>::iterator spawn_itr;
		PacketStruct* packet = configReader.getStruct("WS_TrackingUpdate", client->GetVersion());
		if (packet) {
			packet->setDataByName("mode", TRACKING_UPDATE);
			packet->setDataByName("type", TRACKING_TYPE_ENTITIES);
			vector<TrackedSpawn*> spawns_tracked;
			while (spawn_itr.Next()) {
				Spawn* spawn = spawn_itr->second;
				float distance = player->GetDistance(spawn);
				if (spawn->IsEntity() && distance <= 80 && spawn != player) {
					TrackedSpawn* ts = new TrackedSpawn;
					ts->spawn = spawn;
					ts->distance = distance;

					/* Add spawns in ascending order from closest to furthest */
					if (spawns_tracked.empty())
						spawns_tracked.push_back(ts);
					else {
						vector<TrackedSpawn*>::iterator tracked_itr;
						bool added = false;
						for (tracked_itr = spawns_tracked.begin(); tracked_itr != spawns_tracked.end(); tracked_itr++) {
							TrackedSpawn* cur_ts = *tracked_itr;
							if (ts->distance <= cur_ts->distance) {
								spawns_tracked.insert(tracked_itr, ts);
								added = true;
								break;
							}
						}
						if (!added)
							spawns_tracked.push_back(ts);
					}
				}
			}
			packet->setArrayLengthByName("num_spawns", spawns_tracked.size());
			for (int32 i = 0; i < spawns_tracked.size(); i++) {
				TrackedSpawn* ts = spawns_tracked[i];

				LogWrite(ZONE__DEBUG, 0, "Zone", "%s (%f)", ts->spawn->GetName(), ts->distance);

				packet->setArrayDataByName("spawn_id", player->GetIDWithPlayerSpawn(ts->spawn), i);
				packet->setArrayDataByName("spawn_name", ts->spawn->GetName(), i);
				if (ts->spawn->IsPlayer())
					packet->setArrayDataByName("spawn_type", TRACKING_SPAWN_TYPE_PC, i);
				else
					packet->setArrayDataByName("spawn_type", TRACKING_SPAWN_TYPE_NPC, i);
				packet->setArrayDataByName("spawn_con_color", player->GetArrowColor(ts->spawn->GetLevel()), i);
			}
			packet->setArrayLengthByName("num_array1", 0);
			//for (int32 i = 0; i < spawns_tracked.size(); i++) {
			//}
			packet->setArrayLengthByName("num_spawns2", spawns_tracked.size());
			for (int32 i = 0; i < spawns_tracked.size(); i++) {
				TrackedSpawn* ts = spawns_tracked[i];
				packet->setArrayDataByName("list_spawn_id", player->GetIDWithPlayerSpawn(ts->spawn), i);
				packet->setArrayDataByName("list_number", i, i);
			}
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
			for (int32 i = 0; i < spawns_tracked.size(); i++)
				safe_delete(spawns_tracked[i]);
		}
	}
}

void ZoneServer::SendEpicMobDeathToGuild(Player* killer, Spawn* victim) {
	if (killer && victim) {

		LogWrite(MISC__TODO, 1, "TODO" , "Check if player is in raid\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);

		if (killer->GetGroupMemberInfo()) {
			world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);

			deque<GroupMemberInfo*>::iterator itr;


			PlayerGroup* group = world.GetGroupManager()->GetGroup(killer->GetGroupMemberInfo()->group_id);
			if (group)
			{
				group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
				deque<GroupMemberInfo*>* members = group->GetMembers();
				for (itr = members->begin(); itr != members->end(); itr++) {
					GroupMemberInfo* gmi = *itr;
					if (gmi->client) {
						Player* group_member = gmi->client->GetPlayer();
						if (group_member && group_member->GetGuild()) {
							Guild* guild = group_member->GetGuild();
							string message = Guild::GetEpicMobDeathMessage(group_member->GetName(), victim->GetName());
							guild->AddNewGuildEvent(GUILD_EVENT_KILLS_EPIC_MONSTER, message.c_str(), Timer::GetUnixTimeStamp());
							guild->SendMessageToGuild(GUILD_EVENT_KILLS_EPIC_MONSTER, message.c_str());
						}
					}
				}
				group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
			}

			world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
		}
		else if (killer->GetGuild()) {
			Guild* guild = killer->GetGuild();
			string message = Guild::GetEpicMobDeathMessage(killer->GetName(), victim->GetName());
			guild->AddNewGuildEvent(GUILD_EVENT_KILLS_EPIC_MONSTER, message.c_str(), Timer::GetUnixTimeStamp());
			guild->SendMessageToGuild(GUILD_EVENT_KILLS_EPIC_MONSTER, message.c_str());
		}
	}
}

void ZoneServer::ProcessAggroChecks(Spawn* spawn) {
	if (spawn->GetFactionID() < 1 || spawn->EngagedInCombat())
		return;
	// If faction based combat is not allowed then no need to run the loops so just return out
	if(!rule_manager.GetZoneRule(GetZoneID(), R_Faction, AllowFactionBasedCombat)->GetBool())
		return;

	if (spawn && spawn->IsNPC() && spawn->Alive())
		CheckEnemyList((NPC*)spawn);
}

void ZoneServer::SendUpdateTitles(Client* client, Title* suffix, Title* prefix) {
	assert(client);
	if (client->GetVersion() > 561)
		SendUpdateTitles(client->GetPlayer(), suffix, prefix);
}

void ZoneServer::SendUpdateTitles(Spawn *spawn, Title *suffix, Title *prefix) {
	if (!spawn)
		return;

	vector<Client*>::iterator itr;
	PacketStruct *packet;
	Client* current_client;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		current_client = *itr;

		if (current_client->GetVersion() <= 561)
			continue;

		if (!(packet = configReader.getStruct("WS_UpdateTitle", current_client->GetVersion())))
			continue;

		packet->setDataByName("player_id", current_client->GetPlayer()->GetIDWithPlayerSpawn(spawn));
		packet->setDataByName("player_name", spawn->GetName());
		packet->setDataByName("unknown1", 1, 1);
		if(suffix)
			packet->setDataByName("suffix_title", suffix->GetName());
		else
			packet->setDataByName("suffix_title", spawn->GetSuffixTitle());
		if(prefix)
			packet->setDataByName("prefix_title", prefix->GetName());
		else
			packet->setDataByName("prefix_title", spawn->GetPrefixTitle());
		packet->setDataByName("last_name", spawn->GetLastName());
		packet->setDataByName("sub_title", spawn->GetSubTitle());
		current_client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::AddTransportSpawn(Spawn* spawn){
	if(!spawn)
		return;
	MTransportSpawns.writelock(__FUNCTION__, __LINE__);
	transport_spawns.push_back(spawn->GetID());
	spawn->SetTransportSpawn(true);
	MTransportSpawns.releasewritelock(__FUNCTION__, __LINE__);
}

Spawn* ZoneServer::GetClosestTransportSpawn(float x, float y, float z){
	Spawn* spawn = 0;
	Spawn* closest_spawn = 0;
	float closest_distance = 0.0;
	MTransportSpawns.writelock(__FUNCTION__, __LINE__);
	vector<int32>::iterator itr = transport_spawns.begin();
	while(itr != transport_spawns.end()){
		spawn = GetSpawnByID(*itr);
		if(spawn){
			if(closest_distance == 0.0){
				closest_spawn = spawn;
				closest_distance = spawn->GetDistance(x, y, z);
			}
			else if(spawn->GetDistance(x, y, z) < closest_distance){
				closest_spawn = spawn;
				closest_distance = spawn->GetDistance(x, y, z);
			}
			itr++;
		}
		else
			itr = transport_spawns.erase(itr);
	}
	MTransportSpawns.releasewritelock(__FUNCTION__, __LINE__);

	return closest_spawn;
}

Spawn* ZoneServer::GetTransportByRailID(sint64 rail_id){
	Spawn* spawn = 0;
	Spawn* closest_spawn = 0;
	MTransportSpawns.readlock(__FUNCTION__, __LINE__);
	vector<int32>::iterator itr = transport_spawns.begin();
	while(itr != transport_spawns.end()){
		spawn = GetSpawnByID(*itr);
		//printf("Rail id: %i vs %i\n", spawn ? spawn->GetRailID() : 0, rail_id);
		if(spawn && spawn->GetRailID() == rail_id){
			closest_spawn = spawn;
			break;
		}
		itr++;
	}
	MTransportSpawns.releasereadlock(__FUNCTION__, __LINE__);

	return closest_spawn;
}

void ZoneServer::SetRain(float val) {
	rain = val;
	vector<Client*>::iterator itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		Client* client = *itr;
		client->GetPlayer()->GetInfoStruct()->set_rain(val);
		client->GetPlayer()->SetCharSheetChanged(true);
		if( val >= 0.75 && !weather_signaled )
		{
			client->SimpleMessage(CHANNEL_NARRATIVE, "It starts to rain.");
		}
		else if( val < 0.75 && weather_signaled ) 
		{
			client->SimpleMessage(CHANNEL_NARRATIVE, "It stops raining.");
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);

	if (val >= 0.75 && !weather_signaled) {
		weather_signaled = true;
		ProcessSpawnConditional(SPAWN_CONDITIONAL_RAINING);
	}
	else if (val < 0.75 && weather_signaled) {
		weather_signaled = false;
		ProcessSpawnConditional(SPAWN_CONDITIONAL_NOT_RAINING);
	}
}

void ZoneServer::SetWind(float val) {
	vector<Client*>::iterator itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		Client* client = *itr;
		client->GetPlayer()->GetInfoStruct()->set_wind(val);
		client->GetPlayer()->SetCharSheetChanged(true);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::ProcessWeather()
{
	// if the global rule to disable weather is set, or if the `weather_allowed` field in the zone record == 0, do not process weather
	if( !weather_enabled || !isWeatherAllowed() )
		return;

	LogWrite(ZONE__DEBUG, 1, "Zone", "%s: Processing weather changes", zone_name);
	float new_weather = 0;
	float weather_offset = 0;
	bool change_weather = false;

	// check to see if it is time to change the weather according to weather_frequency (time between changes)
	if( weather_last_changed_time <= (Timer::GetUnixTimeStamp() - weather_frequency) )
	{
		LogWrite(ZONE__DEBUG, 2, "Zone", "%s: Checking for weather changes", zone_name);
		// reset last changed time (frequency check)
		weather_last_changed_time = Timer::GetUnixTimeStamp();

		// this is the chance a weather change occurs at all at the expired interval
		int8 weather_random = MakeRandomInt(1, 100);
		LogWrite(ZONE__DEBUG, 2, "Zone", "%s: Chance to change weather: %i%%, rolled: %i%% - Change weather: %s", zone_name, weather_change_chance, weather_random, weather_random <= weather_change_chance ? "True" : "False");

		if( weather_random <= weather_change_chance )
		{
			change_weather = true;
			weather_offset = weather_change_amount;

			if( weather_type == 3 ) // chaotic weather patterns, random weather between min/max
			{
				new_weather = MakeRandomFloat(weather_min_severity, weather_max_severity);
				LogWrite(ZONE__DEBUG, 3, "Zone", "%s: Chaotic weather severity changed to %.2f", zone_name, new_weather);
				weather_pattern = 2;
			}
			else if( weather_type == 2 ) // random weather patterns, combination of normal + dynamic + max_offset
			{
				weather_offset = MakeRandomFloat(weather_change_amount, weather_dynamic_offset);
				LogWrite(ZONE__DEBUG, 3, "Zone", "%s: Random weather severity changed by %.2f", zone_name, weather_offset);

				int8 weather_alter = weather_change_chance / 10;	// the divide is to prevent too many direction changes in a cycle
				weather_random = MakeRandomInt(1, 100);				// chance that the weather changes direction (weather_pattern)

				if( weather_random <= weather_alter )
					weather_pattern = ( weather_pattern == 0 ) ? 1 : 0;
			}
			else if( weather_type == 1 ) // dynamic weather patterns, weather may not reach min/max
			{
				int8 weather_alter = weather_change_chance / 10;	// the divide is to prevent too many direction changes in a cycle
				weather_random = MakeRandomInt(1, 100);				// chance that the weather changes direction (weather_pattern)

				if( weather_random <= weather_alter )
				{
					weather_pattern = ( weather_pattern == 0 ) ? 1 : 0;
					LogWrite(ZONE__DEBUG, 3, "Zone", "%s: Dynamic weather pattern changed to %i", zone_name, weather_pattern);
				}
			}
			else // normal weather patterns, weather starts at min, goes to max, then back down again
			{
				// do nothing (processed below)
				LogWrite(ZONE__DEBUG, 3, "Zone", "%s: Normal weather severity changed by %.2f", zone_name, weather_offset);
			}

			// when all done, change the weather
			if( change_weather )
			{
				if( weather_pattern == 1 )
				{
					// weather is getting worse, til it reaches weather_max_severity
					new_weather = ( weather_current_severity <= weather_max_severity ) ? weather_current_severity + weather_offset : weather_max_severity;
					LogWrite(ZONE__DEBUG, 3, "Zone", "%s: Increased weather severity by %.2f", zone_name, weather_offset);

					if(new_weather > weather_max_severity)
					{
 						new_weather = weather_max_severity - weather_offset;
						weather_pattern = 0;
					}
				}
				else if( weather_pattern == 0 )
				{
					// weather is clearing up, til it reaches weather_min_severity
					new_weather = ( weather_current_severity >= weather_min_severity ) ? weather_current_severity - weather_offset : weather_min_severity;
					LogWrite(ZONE__DEBUG, 3, "Zone", "%s: Decreased weather severity by %.2f", zone_name, weather_offset);

					if(new_weather < weather_min_severity)
					{
						new_weather = weather_min_severity + weather_offset;
						weather_pattern = 1;
					}
				}				

				LogWrite(ZONE__DEBUG, 1, "Zone", "%s: Weather change triggered from %.2f to %.2f", zone_name, weather_current_severity, new_weather);
				this->SetRain(new_weather);
				weather_current_severity = new_weather;
			}
		}
	}
	else
		LogWrite(ZONE__DEBUG, 1, "Zone", "%s: Not time to change weather yet", zone_name);
}

void ZoneServer::HidePrivateSpawn(Spawn* spawn) {
	if (!spawn->IsPrivateSpawn())
		return;

	Client* client = 0;	
	Player* player = 0;
	PacketStruct* packet = 0;
	int32 packet_version = 0;
	MutexList<Client*>::iterator itr = connected_clients.begin();
	while (itr->Next()) {
		client = itr->value;
		player = client->GetPlayer();
		if (player->WasSentSpawn(spawn->GetID()) || client->GetPlayer()->IsSendingSpawn(spawn->GetID())) {
			if (!packet || packet_version != client->GetVersion()) {
				safe_delete(packet);
				packet_version = client->GetVersion();
				packet = configReader.getStruct("WS_DestroyGhostCmd", packet_version);
			}

			SendRemoveSpawn(client, spawn, packet);
			if(spawn_range_map.count(client) > 0)
				spawn_range_map.Get(client)->erase(spawn->GetID());

			if(player->GetTarget() == spawn)
				player->SetTarget(0);
		}
	}

	safe_delete(packet);
}

SpawnLocation* ZoneServer::GetSpawnLocation(int32 id) {
	SpawnLocation* ret = 0;
	MSpawnLocationList.readlock(__FUNCTION__, __LINE__);
	if (spawn_location_list.count(id) > 0)
		ret = spawn_location_list[id];
	MSpawnLocationList.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

void ZoneServer::PlayAnimation(Spawn* spawn, int32 visual_state, Spawn* spawn2, int8 hide_type){
	Client* client = 0;
	PacketStruct* packet = 0;
	Spawn* exclude_spawn = 0;
	if (!spawn)
		return;
	if (spawn2){
		if(hide_type == 1){
			if(spawn2->IsPlayer()) {
				client = ((Player*)spawn2)->GetClient();
				if(client){
					packet = configReader.getStruct("WS_CannedEmote", client->GetVersion());
					packet->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(spawn));
					packet->setDataByName("anim_type", visual_state);
					client->QueuePacket(packet->serialize());
				}
				safe_delete(packet);
				return;
			}
		}
		if(hide_type == 2)
			exclude_spawn = spawn2;
	}

	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++) {
		client = *client_itr;
		if(spawn->GetDistance(client->GetPlayer()) > 50)
			continue;
		if(exclude_spawn == client->GetPlayer())
			continue;
		if(!client->IsReadyForUpdates()) // client is not in world yet so we shouldn't be sending animations of spawns yet
			continue;
		
		if(!packet || packet->GetVersion() != client->GetVersion()) {
			safe_delete(packet);
			packet = configReader.getStruct("WS_CannedEmote", client->GetVersion());
		}
		if (packet) {
			int32 spawn_id = client->GetPlayer()->GetIDWithPlayerSpawn(spawn);
			if(spawn_id) {
				packet->setDataByName("spawn_id", spawn_id);
				packet->setDataByName("anim_type", visual_state);
				client->QueuePacket(packet->serialize());
			}
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
	safe_delete(packet);
}

vector<Spawn*> ZoneServer::GetSpawnsByID(int32 id) {
	vector<Spawn*> tmp_list;
	Spawn* spawn;

	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if (spawn && (spawn->GetDatabaseID() == id))
			tmp_list.push_back(spawn);
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);

	return tmp_list;
}


vector<Spawn*> ZoneServer::GetSpawnsByRailID(sint64 rail_id) {
	vector<Spawn*> tmp_list;
	Spawn* spawn;
	MTransportSpawns.readlock(__FUNCTION__, __LINE__);
	vector<int32>::iterator itr = transport_spawns.begin();
	while(itr != transport_spawns.end()){
		spawn = GetSpawnByID(*itr);
		if(spawn && spawn->GetRailID() == rail_id){
			tmp_list.push_back(spawn);
		}
		itr++;
	}
	MTransportSpawns.releasereadlock(__FUNCTION__, __LINE__);
	return tmp_list;
}

void ZoneServer::RemovePlayerPassenger(int32 char_id) {
	vector<Spawn*> tmp_list;
	Spawn* spawn;
	MTransportSpawns.readlock(__FUNCTION__, __LINE__);
	vector<int32>::iterator itr = transport_spawns.begin();
	while(itr != transport_spawns.end()){
		spawn = GetSpawnByID(*itr);
		if(spawn) {
			spawn->RemoveRailPassenger(char_id);
		}
		itr++;
	}
	MTransportSpawns.releasereadlock(__FUNCTION__, __LINE__);
}

bool ZoneServer::SetPlayerTargetByName(Client* originator, char* targetName, float distance) {
	if(!targetName || !originator->GetPlayer()) {
		return false;
	}
	int32 name_size = strlen(targetName);
	if(name_size < 1 || name_size > 127) {
		return false;
	}
	
	auto loc = glm::vec3(originator->GetPlayer()->GetX(), originator->GetPlayer()->GetZ(), originator->GetPlayer()->GetY());
	std::vector<int32> grids_by_radius;
	if(originator->GetPlayer()->GetMap()) {
		grids_by_radius = GetGridsByLocation(originator->GetPlayer(), loc, distance);
	}
	else {
		grids_by_radius.push_back(originator->GetPlayer()->GetLocation());
	}
	
	bool found_target = false;
	float spawn_dist = 999999.0f;
	Spawn* target_spawn = nullptr;
	float tmp_dist = 0.0f;
    MGridMaps.lock_shared();
	std::vector<int32>::iterator grid_radius_itr;
	for(grid_radius_itr = grids_by_radius.begin(); grid_radius_itr != grids_by_radius.end(); grid_radius_itr++) {
		std::map<int32, GridMap*>::iterator grids = grid_maps.find((*grid_radius_itr));
		if(grids != grid_maps.end()) {
			grids->second->MSpawns.lock_shared();
			typedef map <int32, Spawn*> SpawnMapType;
			for( SpawnMapType::iterator it = grids->second->spawns.begin(); it != grids->second->spawns.end(); ++it ) {
				Spawn* spawn = it->second;
				
				bool inSameGroup = false;
				if(spawn->IsEntity()) {
					GroupMemberInfo* gmi = originator->GetPlayer()->GetGroupMemberInfo();
					inSameGroup = (((Entity*)spawn)->GetGroupMemberInfo() && originator->GetPlayer()->GetGroupMemberInfo() && ((Entity*)spawn)->GetGroupMemberInfo()->group_id == originator->GetPlayer()->GetGroupMemberInfo()->group_id);
				}
				if (spawn && spawn->appearance.targetable > 0 && !strncasecmp(spawn->GetName(), targetName, name_size) && (inSameGroup || (((tmp_dist = spawn->GetDistance(originator->GetPlayer(), true)) <= distance) && tmp_dist < spawn_dist && originator->GetPlayer()->CheckLoS(spawn)))) {
					spawn_dist = tmp_dist;
					target_spawn = spawn;
				}
			}
			grids->second->MSpawns.unlock_shared();
		}
	}
    MGridMaps.unlock_shared();
	
	if(target_spawn != nullptr) {
		originator->TargetSpawn(target_spawn);
	}
	
	return (target_spawn != nullptr);
}

std::vector<int32> ZoneServer::GetGridsByLocation(Spawn* originator, glm::vec3 loc, float distance) {
	std::vector<int32> grids_by_radius;
	if(originator == nullptr)
		return grids_by_radius;
	
	if(originator->GetMap()) {
		grids_by_radius = originator->GetMap()->GetGridsByPoint(loc, distance);
		if(default_zone_map && default_zone_map != originator->GetMap()) {
			std::vector<int32> default_grids = default_zone_map->GetGridsByPoint(loc, distance);
			
			std::unordered_set<int32> elements(grids_by_radius.begin(), grids_by_radius.end());

			for (const auto& elem : default_grids) {
				if (elements.find(elem) == elements.end()) {
					grids_by_radius.push_back(elem);
					elements.insert(elem);
				}
			}
		}
	}
	
	return grids_by_radius;
}

std::vector<std::pair<int32, float>> ZoneServer::GetAttackableSpawnsByDistance(Spawn* caster, float distance) {
	std::vector<std::pair<int32, float>> spawns_by_distance;
	Spawn* spawn = 0;
	auto loc = glm::vec3(caster->GetX(), caster->GetZ(), caster->GetY());
	std::vector<int32> grids_by_radius;
	if(caster->GetMap()) {
		grids_by_radius = GetGridsByLocation(caster, loc, distance);
	}
	else {
		grids_by_radius.push_back(caster->GetLocation());
	}
	
	float tmp_dist = 0.0f;
    MGridMaps.lock_shared();
	std::vector<int32>::iterator grid_radius_itr;
	for(grid_radius_itr = grids_by_radius.begin(); grid_radius_itr != grids_by_radius.end(); grid_radius_itr++) {
		std::map<int32, GridMap*>::iterator grids = grid_maps.find((*grid_radius_itr));
		if(grids != grid_maps.end()) {
			grids->second->MSpawns.lock_shared();
			typedef map <int32, Spawn*> SpawnMapType;
			for( SpawnMapType::iterator it = grids->second->spawns.begin(); it != grids->second->spawns.end(); ++it ) {
				Spawn* spawn = it->second;
				if (spawn && spawn->IsNPC() && spawn->appearance.attackable > 0 && spawn->GetID() > 0 && spawn->GetID() != caster->GetID() &&
					spawn->Alive() && ((tmp_dist = spawn->GetDistance(caster, true)) <= distance)) {
					spawns_by_distance.push_back({spawn->GetID(), tmp_dist});
				}
			}
			grids->second->MSpawns.unlock_shared();
		}
	}
    MGridMaps.unlock_shared();
	std::sort(spawns_by_distance.begin(), spawns_by_distance.end(), compareByValue);
	
	return spawns_by_distance;
}

void ZoneServer::ResurrectSpawn(Spawn* spawn, Client* client) {
	if(!client || !spawn)
		return;
	PendingResurrection* rez = client->GetCurrentRez();
	if(!rez)
		return;

	PacketStruct* packet = 0;
	float power_perc = rez->mp_perc;
	float health_perc = rez->hp_perc;
	Spawn* caster_spawn = GetSpawnByID(rez->caster);
	
	if(!caster_spawn)
		return;
	
	sint32 heal_amt = 0;
	sint32 power_amt = 0;
	bool no_calcs = rez->no_calcs;
	int8 crit_mod = rez->crit_mod;
	Entity* caster = 0;
	InfoStruct* info = 0;
	bool crit = false;
	string heal_spell = rez->heal_name;
	int16 heal_packet_type = 0;
	int16 power_packet_type = 0;

	//Calculations for how much to heal the spawn
	if(health_perc > 0)
		heal_amt = (spawn->GetTotalHP() * (health_perc / 100));
	if(power_perc > 0)
		power_amt = (spawn->GetTotalPower() * (power_perc / 100));

	if(caster_spawn->IsEntity()){
		caster = ((Entity*)caster_spawn);
		info = caster->GetInfoStruct();
	}

	if(!no_calcs && caster){
		heal_amt = caster->CalculateHealAmount(spawn, heal_amt, crit_mod, &crit);
		power_amt = caster->CalculateHealAmount(spawn, power_amt, crit_mod, &crit);
	}

	//Set this rez as a crit to be passed to subspell (not yet used)
	rez->crit = true;

	//Set Heal amt to 1 if 0 now so the player has health
	if(heal_amt == 0)
		heal_amt = 1;

	if(heal_amt > spawn->GetTotalHP())
		heal_amt = spawn->GetTotalHP();
	if(power_amt > spawn->GetTotalPower())
		power_amt = spawn->GetTotalPower();

	spawn->SetAlive(true);
	spawn->SetHP(heal_amt);
	if(power_amt > 0)
		spawn->SetPower(power_amt);

	if(client && caster){
		EQ2Packet* move = ((Player*)spawn)->Move(caster->GetX(), caster->GetY(), caster->GetZ(), client->GetVersion());
		if(move)
			client->QueuePacket(move);
	}

	if(crit){
		power_packet_type = HEAL_PACKET_TYPE_CRIT_MANA;
		heal_packet_type = HEAL_PACKET_TYPE_CRIT_HEAL;
	}
	else {
		power_packet_type = HEAL_PACKET_TYPE_SIMPLE_MANA;
		heal_packet_type = HEAL_PACKET_TYPE_SIMPLE_HEAL;
	}

	SendHealPacket(caster, spawn, heal_packet_type, heal_amt, heal_spell.c_str());
	if(power_amt > 0)
		SendHealPacket(caster, spawn, power_packet_type, power_amt, heal_spell.c_str());

	//The following code sets the spawn as alive
	if(dead_spawns.count(spawn->GetID()) > 0)
		dead_spawns.erase(spawn->GetID());

	if(spawn->IsPlayer()){
		spawn->SetSpawnType(4);
		client = ((Player*)spawn)->GetClient();
		if(client){
			packet = configReader.getStruct("WS_Resurrected", client->GetVersion());
			if(packet){
				client->QueuePacket(packet->serialize());
			}
			safe_delete(packet);
			
			if(client->GetVersion() <= 561) {
				ClientPacketFunctions::SendServerControlFlagsClassic(client, 8, 0);
				ClientPacketFunctions::SendServerControlFlagsClassic(client, 16, 0);
			}
			else {
				ClientPacketFunctions::SendServerControlFlags(client, 1, 8, 0);
				ClientPacketFunctions::SendServerControlFlags(client, 1, 16, 0);
			}
			client->SimpleMessage(CHANNEL_NARRATIVE, "You regain consciousness!");
		}
	}
	spawn->SendSpawnChanges(true);
	spawn->SetTempActionState(-1);
	spawn->appearance.attackable = 1;
	
	if(rez->revive_sickness_spell_id) {
		Spell* spell = master_spell_list.GetSpell(rez->revive_sickness_spell_id, rez->revive_sickness_spell_tier);

		if (spell)
		{
			GetSpellProcess()->CastInstant(spell, caster ? caster : (Entity*)client->GetPlayer(), (Entity*)client->GetPlayer());
		}
	}
}

void ZoneServer::SendDispellPacket(Entity* caster, Spawn* target, string dispell_name, string spell_name, int8 dispell_type){
	if(!caster || !target)
		return;

	Client* client = 0;
	Player* player = 0;
	PacketStruct* packet = 0;
	vector<Client*>::iterator client_itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (client_itr = clients.begin(); client_itr != clients.end(); client_itr++){
		client = *client_itr;
		if(!client || !(player = client->GetPlayer()) || (player != caster && ((caster && player->WasSentSpawn(caster->GetID()) == false) || (target && player->WasSentSpawn(target->GetID()) == false))))
			continue;
		if(caster && caster->GetDistance(player) > 50)
			continue;
		if(target && target->GetDistance(player) > 50)
			continue;

		packet = configReader.getStruct("WS_HearDispell", client->GetVersion());
		if(packet){
			packet->setDataByName("spell_name", spell_name.c_str());
			packet->setDataByName("dispell_name", dispell_name.c_str());
			packet->setDataByName("caster", player->GetIDWithPlayerSpawn(caster));
			packet->setDataByName("target", player->GetIDWithPlayerSpawn(target));
			packet->setDataByName("type", dispell_type);
			client->QueuePacket(packet->serialize());
		}
		safe_delete(packet);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::DismissAllPets() {
	Spawn* spawn = 0;
	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if (spawn && spawn->IsEntity())
			((Entity*)spawn)->DismissAllPets();
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::RemoveTargetFromSpell(LuaSpell* spell, Spawn* target, bool remove_caster){
	if (spellProcess)
		spellProcess->RemoveTargetFromSpell(spell, target, remove_caster);
}

void ZoneServer::ClearHate(Entity* entity) {
	Spawn* spawn = 0;
	map<int32, Spawn*>::iterator itr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if (spawn && spawn->IsNPC() && ((NPC*)spawn)->Brain())
			((NPC*)spawn)->Brain()->ClearHate(entity);
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
}

ThreadReturnType ZoneLoop(void* tmp) {
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#endif
	if (tmp == 0) {
		ThrowError("ZoneLoop(): tmp = 0!");
		THREAD_RETURN(NULL);
	}
	ZoneServer* zs = (ZoneServer*) tmp;
	while (zs->Process()) {
		if(zs->GetClientCount() == 0)
			Sleep(1000);
		else
			Sleep(10);
	}
	// we failed, time to disappear, no more processing period
	safe_delete(zs);
	THREAD_RETURN(NULL);
}

ThreadReturnType SpawnLoop(void* tmp) {
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#endif
	if (tmp == 0) {
		ThrowError("SpawnLoop(): tmp = 0!");
		THREAD_RETURN(NULL);
	}
	ZoneServer* zs = (ZoneServer*) tmp;
#ifndef NO_CATCH
	try {
#endif
		zs->spawnthread_active = true;
		while (zs->SpawnProcess()) {
			if(zs->GetClientCount() == 0)
				Sleep(1000);
			else
				Sleep(20);
		}
		zs->spawnthread_active = false;
#ifndef NO_CATCH
	}
	catch(...) {
		zs->spawnthread_active = false;
		zs->initial_spawn_threads_active = 0;
		LogWrite(ZONE__ERROR, 0, "Zone",  "Error Processing SpawnLoop, shutting down zone '%s'...", zs->GetZoneName());
		try{
			zs->Shutdown();
		}
		catch(...){
			LogWrite(ZONE__ERROR, 0, "Zone",  "Error Processing SpawnLoop while shutting down zone '%s'...", zs->GetZoneName());
			throw;
		}
		throw;
	}
#endif
	THREAD_RETURN(NULL);
}

ThreadReturnType SendInitialSpawns(void* tmp) {
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#endif
	if (tmp == 0) {
		ThrowError("SendInitialSpawns(): tmp = 0!");
		THREAD_RETURN(NULL);
	}
	Client* client = (Client*) tmp;
	client->GetCurrentZone()->SendZoneSpawns(client);
	THREAD_RETURN(NULL);
}

ThreadReturnType SendLevelChangedSpawns(void* tmp) {
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#endif
	if (tmp == 0) {
		ThrowError("SendLevelChangedSpawns(): tmp = 0!");
		THREAD_RETURN(NULL);
	}
	Client* client = (Client*)tmp;
	client->GetCurrentZone()->SendAllSpawnsForLevelChange(client);
	THREAD_RETURN(NULL);
}

void ZoneServer::SetSpawnStructs(Client* client) {
	int16 client_ver = client->GetVersion();
	Player* player = client->GetPlayer();

	//Save a copy of the correct spawn substructs for the client's player, save here a copy if we don't have one
	PacketStruct* pos = configReader.getStruct("Substruct_SpawnPositionStruct", client_ver);
	player->SetSpawnPosStruct(pos);
	if (versioned_pos_structs.count(pos->GetVersion()) == 0)
		versioned_pos_structs[pos->GetVersion()] = new PacketStruct(pos, true);

	PacketStruct* vis = configReader.getStruct("Substruct_SpawnVisualizationInfoStruct", client_ver);
	player->SetSpawnVisStruct(vis);
	if (versioned_vis_structs.count(vis->GetVersion()) == 0)
		versioned_vis_structs[vis->GetVersion()] = new PacketStruct(vis, true);

	PacketStruct* info = configReader.getStruct("Substruct_SpawnInfoStruct", client_ver);
	player->SetSpawnInfoStruct(info);
	if (versioned_info_structs.count(info->GetVersion()) == 0)
		versioned_info_structs[info->GetVersion()] = new PacketStruct(info, true);

	PacketStruct* header = configReader.getStruct("WS_SpawnStruct_Header", client_ver);
	player->SetSpawnHeaderStruct(header);

	PacketStruct* footer = configReader.getStruct("WS_SpawnStruct_Footer", client_ver);
	player->SetSpawnFooterStruct(footer);

	PacketStruct* sfooter = configReader.getStruct("WS_SignWidgetSpawnStruct_Footer", client_ver);
	player->SetSignFooterStruct(sfooter);

	PacketStruct* wfooter = configReader.getStruct("WS_WidgetSpawnStruct_Footer", client_ver);
	player->SetWidgetFooterStruct(wfooter);
}

Spawn* ZoneServer::GetSpawn(int32 id){
	Spawn* ret = 0;

	if(GetNPC(id))
		ret = GetNewNPC(id);
	else if(this->GetObject(id))
		ret = GetNewObject(id);
	else if(GetWidget(id))
		ret = GetNewWidget(id);
	else if(GetSign(id))
		ret = GetNewSign(id);
	else if(GetGroundSpawn(id))
		ret = GetNewGroundSpawn(id);
	// Unable to find the spawn in the list lets attempt to add it if we are not currently reloading
	else if (!reloading && database.LoadNPC(this, id)) {
		if (GetNPC(id))
			ret = GetNewNPC(id);
		else
			LogWrite(NPC__ERROR, 0, "NPC", "Database inserted npc (%u) but was still unable to retrieve it!", id);
	}
	else if (!reloading && database.LoadObject(this, id)) {
		if (this->GetObject(id))
			ret = GetNewObject(id);
		else
			LogWrite(OBJECT__ERROR, 0, "Object", "Database inserted object (%u) but was still unable to retrieve it!", id);
	}
	else if (!reloading && database.LoadWidget(this, id)) {
		if (GetWidget(id))
			ret = GetNewWidget(id);
		else
			LogWrite(WIDGET__ERROR, 0, "Widget", "Database inserted widget (%u) but was still unable to retrieve it!", id);
	}
	else if (!reloading && database.LoadSign(this, id)) {
		if (GetSign(id))
			ret = GetNewSign(id);
		else
			LogWrite(SIGN__ERROR, 0, "Sign", "Database inserted sign (%u) but was still unable to retrieve it!", id);
	}
	else if (!reloading && database.LoadGroundSpawn(this, id)) {
		if (GetGroundSpawn(id))
			ret = GetNewGroundSpawn(id);
		else
			LogWrite(GROUNDSPAWN__ERROR, 0, "GSpawn", "Database inserted ground spawn (%u) but was still unable to retrieve it!", id);
	}

	if(ret && ret->IsOmittedByDBFlag())
	{
		LogWrite(SPAWN__WARNING, 0, "Spawn", "Spawn (%u) was skipped due to a missing expansion / holiday flag being met.", id);
		safe_delete(ret);
		ret = 0;
	}

	if(ret)
		ret->SetID(Spawn::NextID());
	return ret;
}




vector<EntityCommand*>* ZoneServer::GetEntityCommandList(int32 id){
	if(entity_command_list.count(id) > 0)
		return entity_command_list[id];
	else
		return 0;
}

void ZoneServer::SetEntityCommandList(int32 id, EntityCommand* command) {
	if (entity_command_list.count(id) == 0)
		entity_command_list[id] = new vector<EntityCommand*>;

	entity_command_list[id]->push_back(command);
}

EntityCommand* ZoneServer::GetEntityCommand(int32 id, string name) {
	EntityCommand* ret = 0;
	if (entity_command_list.count(id) == 0)
		return ret;

	vector<EntityCommand*>::iterator itr;
	for (itr = entity_command_list[id]->begin(); itr != entity_command_list[id]->end(); itr++) {
		if ((*itr)->name == name) {
			ret = (*itr);
			break;
		}
	}

	return ret;
}

void ZoneServer::ClearEntityCommands() {
	if (entity_command_list.size() > 0) {
		map<int32, vector<EntityCommand*>* >::iterator itr;
		for (itr = entity_command_list.begin(); itr != entity_command_list.end(); itr++) {
			vector<EntityCommand*>* entity_commands = itr->second;
			if (entity_commands && entity_commands->size() > 0) {
				vector<EntityCommand*>::iterator v_itr;
				for (v_itr = entity_commands->begin(); v_itr != entity_commands->end(); v_itr++)
					safe_delete(*v_itr);
				entity_commands->clear();
			}
			safe_delete(entity_commands);
		}
		entity_command_list.clear();
	}
}

void ZoneServer::AddNPCSkill(int32 list_id, int32 skill_id, int16 value){
	npc_skill_list[list_id][skill_id] = value;
}

map<string, Skill*>* ZoneServer::GetNPCSkills(int32 primary_list, int32 secondary_list){
	map<string, Skill*>* ret = 0;
	if(npc_skill_list.count(primary_list) > 0){
		ret = new map<string, Skill*>();
		map<int32, int16>::iterator itr;
		Skill* tmpSkill = 0;
		for(itr = npc_skill_list[primary_list].begin(); itr != npc_skill_list[primary_list].end(); itr++){
			tmpSkill = master_skill_list.GetSkill(itr->first);
			if(tmpSkill){
				tmpSkill = new Skill(tmpSkill);
				tmpSkill->current_val = itr->second;
				tmpSkill->max_val = tmpSkill->current_val+5;
				(*ret)[tmpSkill->name.data] = tmpSkill;
			}
		}
	}
	if(npc_skill_list.count(secondary_list) > 0){
		if(!ret)
			ret = new map<string, Skill*>();
		map<int32, int16>::iterator itr;
		Skill* tmpSkill = 0;
		for(itr = npc_skill_list[secondary_list].begin(); itr != npc_skill_list[secondary_list].end(); itr++){
			tmpSkill = master_skill_list.GetSkill(itr->first);
			if(tmpSkill){
				tmpSkill = new Skill(tmpSkill);
				tmpSkill->current_val = itr->second;
				tmpSkill->max_val = tmpSkill->current_val+5;
				(*ret)[tmpSkill->name.data] = tmpSkill;
			}
		}
	}
	if(ret && ret->size() == 0){
		safe_delete(ret);
		ret = 0;
	}
	return ret;
}

void ZoneServer::AddNPCEquipment(int32 list_id, int32 item_id){
	npc_equipment_list[list_id].push_back(item_id);
}

void ZoneServer::SetNPCEquipment(NPC* npc) {
	if(npc_equipment_list.count(npc->GetEquipmentListID()) > 0){
		Item* tmpItem = 0;
		int8 slot = 0;
		vector<int32>::iterator itr;
		for(itr = npc_equipment_list[npc->GetEquipmentListID()].begin(); itr != npc_equipment_list[npc->GetEquipmentListID()].end(); itr++){
			tmpItem = master_item_list.GetItem(*itr);
			if(tmpItem){
				slot = npc->GetEquipmentList()->GetFreeSlot(tmpItem);
				if(slot < 255){
					tmpItem = new Item(tmpItem);
					npc->GetEquipmentList()->SetItem(slot, tmpItem);
				}
			}
		}
	}
}

void ZoneServer::AddNPC(int32 id, NPC* npc) {
	npc_list[id] = npc;
}

void ZoneServer::AddWidget(int32 id, Widget* widget) {
	widget_list[id] = widget;
}

Widget* ZoneServer::GetWidget(int32 id, bool override_loading) { 
	if((!reloading || override_loading) && widget_list.count(id) > 0)
		return widget_list[id]; 
	else
		return 0;
}

Widget* ZoneServer::GetNewWidget(int32 id) {
	if(!reloading && widget_list.count(id) > 0)
		return widget_list[id]->Copy(); 
	else
		return 0;
}


void ZoneServer::LoadGroundSpawnEntries(){
	MGroundSpawnItems.lock();
	database.LoadGroundSpawnEntries(this);
	MGroundSpawnItems.unlock();
}

void ZoneServer::LoadGroundSpawnItems() {
}

void ZoneServer::AddGroundSpawnEntry(int32 groundspawn_id, int16 min_skill_level, int16 min_adventure_level, int8 bonus_table, float harvest1, float harvest3, float harvest5, float harvest_imbue, float harvest_rare, float harvest10, int32 harvest_coin) {
	GroundSpawnEntry* entry = new GroundSpawnEntry;
	entry->min_skill_level = min_skill_level;
	entry->min_adventure_level = min_adventure_level;
	entry->bonus_table = bonus_table;
	entry->harvest1 = harvest1;
	entry->harvest3 = harvest3;
	entry->harvest5 = harvest5;
	entry->harvest_imbue = harvest_imbue;
	entry->harvest_rare = harvest_rare;
	entry->harvest10 = harvest10;
	entry->harvest_coin = harvest_coin;
	groundspawn_entries[groundspawn_id].push_back(entry);
}

void ZoneServer::AddGroundSpawnItem(int32 groundspawn_id, int32 item_id, int8 is_rare, int32 grid_id) {
	GroundSpawnEntryItem* entry = new GroundSpawnEntryItem;
	entry->item_id = item_id;
	entry->is_rare = is_rare;
	entry->grid_id = grid_id;
	groundspawn_items[groundspawn_id].push_back(entry);

}

vector<GroundSpawnEntry*>* ZoneServer::GetGroundSpawnEntries(int32 id){
	vector<GroundSpawnEntry*>* ret = 0;
	MGroundSpawnItems.lock();
	if(groundspawn_entries.count(id) > 0)
		ret = &groundspawn_entries[id];
	MGroundSpawnItems.unlock();
	return ret;
}

vector<GroundSpawnEntryItem*>* ZoneServer::GetGroundSpawnEntryItems(int32 id){
	vector<GroundSpawnEntryItem*>* ret = 0;
	if(groundspawn_items.count(id) > 0)
		ret = &groundspawn_items[id];
	return ret;
}

// TODO - mis-named, should be DeleteGroundSpawnEntries() but this is ok for now :)
void ZoneServer::DeleteGroundSpawnItems()
{
	MGroundSpawnItems.lock();

	map<int32, vector<GroundSpawnEntry*> >::iterator groundspawnentry_map_itr;
	vector<GroundSpawnEntry*>::iterator groundspawnentry_itr;
	for(groundspawnentry_map_itr = groundspawn_entries.begin(); groundspawnentry_map_itr != groundspawn_entries.end(); groundspawnentry_map_itr++)
	{
		for(groundspawnentry_itr = groundspawnentry_map_itr->second.begin(); groundspawnentry_itr != groundspawnentry_map_itr->second.end(); groundspawnentry_itr++)
		{
			safe_delete(*groundspawnentry_itr);
		}
	}
	groundspawn_entries.clear();

	map<int32, vector<GroundSpawnEntryItem*> >::iterator groundspawnitem_map_itr;
	vector<GroundSpawnEntryItem*>::iterator groundspawnitem_itr;
	for(groundspawnitem_map_itr = groundspawn_items.begin(); groundspawnitem_map_itr != groundspawn_items.end(); groundspawnitem_map_itr++)
	{
		for(groundspawnitem_itr = groundspawnitem_map_itr->second.begin(); groundspawnitem_itr != groundspawnitem_map_itr->second.end(); groundspawnitem_itr++)
		{
			safe_delete(*groundspawnitem_itr);
		}
	}
	groundspawn_items.clear();

	MGroundSpawnItems.unlock();
}

void ZoneServer::AddGroundSpawn(int32 id, GroundSpawn* spawn) {
	groundspawn_list[id] = spawn;
}

GroundSpawn* ZoneServer::GetGroundSpawn(int32 id, bool override_loading) { 
	if((!reloading || override_loading) && groundspawn_list.count(id) > 0)
		return groundspawn_list[id]; 
	else
		return 0;
}

GroundSpawn* ZoneServer::GetNewGroundSpawn(int32 id) {
	if(!reloading && groundspawn_list.count(id) > 0)
		return groundspawn_list[id]->Copy(); 
	else
		return 0;
}

void ZoneServer::AddLootTable(int32 id, LootTable* table){
	loot_tables[id] = table;
}

void ZoneServer::AddLootDrop(int32 id, LootDrop* drop){
	loot_drops[id].push_back(drop);
}

void ZoneServer::AddSpawnLootList(int32 spawn_id, int32 id) {
	spawn_loot_list[spawn_id].push_back(id);
}

void ZoneServer::ClearSpawnLootList(int32 spawn_id) {
	spawn_loot_list[spawn_id].clear();
}

void ZoneServer::AddLevelLootList(GlobalLoot* loot) {
	level_loot_list.push_back(loot);
}

void ZoneServer::AddRacialLootList(int16 racial_id, GlobalLoot* loot) {
	racial_loot_list[racial_id].push_back(loot);
}

void ZoneServer::AddZoneLootList(int32 zone, GlobalLoot* loot) {
	zone_loot_list[zone].push_back(loot);
}

void ZoneServer::ClearLootTables(){
	map<int32,LootTable*>::iterator table_itr;
	for(table_itr = loot_tables.begin(); table_itr != loot_tables.end(); table_itr++){
		safe_delete(table_itr->second);
	}

	map<int32, vector<LootDrop*> >::iterator drop_itr;
	vector<LootDrop*>::iterator drop_itr2;
	for(drop_itr = loot_drops.begin(); drop_itr != loot_drops.end(); drop_itr++){
		for(drop_itr2 = drop_itr->second.begin(); drop_itr2 != drop_itr->second.end(); drop_itr2++){
			safe_delete(*drop_itr2);
		}
	}

	vector<GlobalLoot*>::iterator level_itr;
	for (level_itr = level_loot_list.begin(); level_itr != level_loot_list.end(); level_itr++) {
		safe_delete(*level_itr);
	}


	map<int16, vector<GlobalLoot*> >::iterator race_itr;
	vector<GlobalLoot*>::iterator race_itr2;
	for (race_itr = racial_loot_list.begin(); race_itr != racial_loot_list.end(); race_itr++) {
		for (race_itr2 = race_itr->second.begin(); race_itr2 != race_itr->second.end(); race_itr2++) {
			safe_delete(*race_itr2);
		}
	}

	map<int32, vector<GlobalLoot*> >::iterator zone_itr;
	vector<GlobalLoot*>::iterator zone_itr2;
	for(zone_itr = zone_loot_list.begin(); zone_itr != zone_loot_list.end(); zone_itr++) {
		for (zone_itr2 = zone_itr->second.begin(); zone_itr2 != zone_itr->second.end(); zone_itr2++) {
			safe_delete(*zone_itr2);
		}
	}

	loot_tables.clear();
	loot_drops.clear();
	spawn_loot_list.clear();
	level_loot_list.clear();
	racial_loot_list.clear();
	zone_loot_list.clear();
}

vector<int32> ZoneServer::GetSpawnLootList(int32 spawn_id, int32 zone_id, int8 spawn_level, int16 racial_id, Spawn* spawn) {
	vector<int32> ret;
	int32 returnValue = 0;

	if(reloading)
		return ret;

	if (spawn_loot_list.count(spawn_id) > 0)
		ret.insert(ret.end(), spawn_loot_list[spawn_id].begin(), spawn_loot_list[spawn_id].end());

	if (level_loot_list.size() > 0) {
		vector<GlobalLoot*>::iterator itr;
		for (itr = level_loot_list.begin(); itr != level_loot_list.end(); itr++) {
			GlobalLoot* loot = *itr;
			const char* zone_script = world.GetZoneScript(this->GetZoneID());
			returnValue = 0; // reset since this can override the database setting
			if(zone_script)
			{
				if(lua_interface->RunZoneScriptWithReturn(zone_script, "loot_criteria_level", spawn->GetZone(), spawn, loot->table_id, loot->minLevel, loot->maxLevel, &returnValue) && returnValue == 0)
					continue;
			}
			bool entryAdded = false;
			if (loot->minLevel == 0 && loot->maxLevel == 0 && (!loot->loot_tier || spawn->GetLootTier() >= loot->loot_tier) && (entryAdded = true)) // successful plan to add set entryAdded to true
				ret.push_back(loot->table_id);
			else {
				if (spawn_level >= loot->minLevel && spawn_level <= loot->maxLevel && (!loot->loot_tier || spawn->GetLootTier() >= loot->loot_tier) && (entryAdded = true)) // successful plan to add set entryAdded to true
					ret.push_back(loot->table_id);
			}
			
			if(!entryAdded && returnValue) // DB override via LUA scripting
				ret.push_back(loot->table_id);
		}
	}

	if (racial_loot_list.count(racial_id) > 0) {
		vector<GlobalLoot*>::iterator itr;
		for (itr = racial_loot_list[racial_id].begin(); itr != racial_loot_list[racial_id].end(); itr++) {
			GlobalLoot* loot = *itr;
			const char* zone_script = world.GetZoneScript(this->GetZoneID());
			returnValue = 0; // reset since this can override the database setting
			if(zone_script)
			{
				if(lua_interface->RunZoneScriptWithReturn(zone_script, "loot_criteria_racial", spawn->GetZone(), spawn, loot->table_id, loot->minLevel, loot->maxLevel, &returnValue) && returnValue == 0)
					continue;
			}
			bool entryAdded = false;
			if (loot->minLevel == 0 && loot->maxLevel == 0 && (!loot->loot_tier || spawn->GetLootTier() >= loot->loot_tier) && (entryAdded = true)) // successful plan to add set entryAdded to true
				ret.push_back(loot->table_id);
			else {
				if (spawn_level >= loot->minLevel && spawn_level <= loot->maxLevel && (!loot->loot_tier || spawn->GetLootTier() >= loot->loot_tier) && (entryAdded = true)) // successful plan to add set entryAdded to true
					ret.push_back(loot->table_id);
			}

			if(!entryAdded && returnValue) // DB override via LUA scripting
				ret.push_back(loot->table_id);
		}
	}

	if (zone_loot_list.count(zone_id) > 0) {
		vector<GlobalLoot*>::iterator itr;
		for (itr = zone_loot_list[zone_id].begin(); itr != zone_loot_list[zone_id].end(); itr++) {
			GlobalLoot* loot = *itr;
			const char* zone_script = world.GetZoneScript(this->GetZoneID());
			returnValue = 0; // reset since this can override the database setting
			if(zone_script)
			{
				if(lua_interface->RunZoneScriptWithReturn(zone_script, "loot_criteria_zone", spawn->GetZone(), spawn, loot->table_id, loot->minLevel, loot->maxLevel, &returnValue) && returnValue == 0)
					continue;
			}
			bool entryAdded = false;
			if (loot->minLevel == 0 && loot->maxLevel == 0 && (!loot->loot_tier || spawn->GetLootTier() >= loot->loot_tier) && (entryAdded = true)) // successful plan to add set entryAdded to true
				ret.push_back(loot->table_id);
			else {
				if (spawn_level >= loot->minLevel && spawn_level <= loot->maxLevel && (!loot->loot_tier || spawn->GetLootTier() >= loot->loot_tier) && (entryAdded = true)) // successful plan to add set entryAdded to true
					ret.push_back(loot->table_id);
			}

			if(!entryAdded && returnValue) // DB override via LUA scripting
				ret.push_back(loot->table_id);
		}
	}

	return ret;
}

vector<LootDrop*>* ZoneServer::GetLootDrops(int32 table_id){
	if(!reloading && loot_drops.count(table_id) > 0)
		return &(loot_drops[table_id]);
	else
		return 0;
}

LootTable* ZoneServer::GetLootTable(int32 table_id){
	return loot_tables[table_id];
}

void ZoneServer::AddLocationTransporter(int32 zone_id, string message, float trigger_x, float trigger_y, float trigger_z, float trigger_radius, int32 destination_zone_id, float destination_x, float destination_y, float destination_z, float destination_heading, int32 cost, int32 unique_id){
	LocationTransportDestination* loc = new LocationTransportDestination;
	loc->message = message;
	loc->trigger_x = trigger_x;
	loc->trigger_y = trigger_y;
	loc->trigger_z = trigger_z;
	loc->trigger_radius = trigger_radius;
	loc->destination_zone_id = destination_zone_id;
	loc->destination_x = destination_x;
	loc->destination_y = destination_y;
	loc->destination_z = destination_z;
	loc->destination_heading = destination_heading;
	loc->cost = cost;
	loc->unique_id = unique_id;
	MTransporters.lock();
	if(location_transporters.count(zone_id) == 0)
		location_transporters[zone_id] = new MutexList<LocationTransportDestination*>();
	location_transporters[zone_id]->Add(loc);
	MTransporters.unlock();
}

void ZoneServer::AddTransporter(int32 transport_id, int8 type, string name, string message, int32 destination_zone_id, float destination_x, float destination_y, float destination_z, float destination_heading, 
	int32 cost, int32 unique_id, int8 min_level, int8 max_level, int32 quest_req, int16 quest_step_req, int32 quest_complete, int32 map_x, int32 map_y, int32 expansion_flag, int32 holiday_flag, int32 min_client_version, 
	int32 max_client_version, int32 flight_path_id, int16 mount_id, int8 mount_red_color, int8 mount_green_color, int8 mount_blue_color){
	TransportDestination* transport = new TransportDestination;
	transport->type = type;
	transport->display_name = name;
	transport->message = message;
	transport->destination_zone_id = destination_zone_id;
	transport->destination_x = destination_x;
	transport->destination_y = destination_y;
	transport->destination_z = destination_z;
	transport->destination_heading = destination_heading;
	transport->cost = cost;
	transport->unique_id = unique_id;

	transport->min_level = min_level;
	transport->max_level = max_level;
	transport->req_quest = quest_req;
	transport->req_quest_step = quest_step_req;
	transport->req_quest_complete = quest_complete;

	transport->map_x = map_x;
	transport->map_y = map_y;

	transport->expansion_flag = expansion_flag;
	transport->holiday_flag = holiday_flag;

	transport->min_client_version = min_client_version;
	transport->max_client_version = max_client_version;

	transport->flight_path_id = flight_path_id;

	transport->mount_id = mount_id;
	transport->mount_red_color = mount_red_color;
	transport->mount_green_color = mount_green_color;
	transport->mount_blue_color = mount_blue_color;

	MTransporters.lock();
	transporters[transport_id].push_back(transport);
	MTransporters.unlock();
}

void ZoneServer::GetTransporters(vector<TransportDestination*>* returnList, Client* client, int32 transport_id){
	if (!returnList)
		return;

	MTransporters.lock();
	if (transporters.count(transport_id) > 0)
	{
		vector<TransportDestination*> list;
		for (int i = 0; i < transporters[transport_id].size(); i++)
		{
			if (transporters[transport_id][i]->min_client_version && client->GetVersion() < transporters[transport_id][i]->min_client_version)
				continue;
			else if (transporters[transport_id][i]->max_client_version && client->GetVersion() > transporters[transport_id][i]->max_client_version)
				continue;

			if (database.CheckExpansionFlags(this, transporters[transport_id][i]->expansion_flag) && database.CheckHolidayFlags(this, transporters[transport_id][i]->holiday_flag))
			{
				returnList->push_back(transporters[transport_id][i]);
			}
		}
	}
	MTransporters.unlock();
}

MutexList<LocationTransportDestination*>* ZoneServer::GetLocationTransporters(int32 zone_id){
	MutexList<LocationTransportDestination*>* ret = 0;
	MTransporters.lock();
	if(location_transporters.count(zone_id) > 0)
		ret = location_transporters[zone_id];
	MTransporters.unlock();
	return ret;
}

void ZoneServer::DeleteGlobalTransporters(){
	MTransporters.lock();
	map<int32, vector<TransportDestination*> >::iterator itr;
	vector<TransportDestination*>::iterator transport_vector_itr;
	for(itr = transporters.begin(); itr != transporters.end(); itr++){
		for(transport_vector_itr = itr->second.begin(); transport_vector_itr != itr->second.end(); transport_vector_itr++){
			safe_delete(*transport_vector_itr);
		}
	}
	map<int32, MutexList<LocationTransportDestination*>* >::iterator itr2;
	for(itr2 = location_transporters.begin(); itr2 != location_transporters.end(); itr2++){
		itr2->second->clear(true);
		delete itr2->second;
	}
	transporters.clear();
	location_transporters.clear();
	MTransporters.unlock();
}

void ZoneServer::DeleteGlobalSpawns() {
	ClearLootTables();
	
	map<int32, NPC*>::iterator npc_list_iter;
	for(npc_list_iter=npc_list.begin();npc_list_iter!=npc_list.end();npc_list_iter++) {
		safe_delete(npc_list_iter->second);
	} 
	npc_list.clear();
	map<int32, Object*>::iterator object_list_iter;
	for(object_list_iter=object_list.begin();object_list_iter!=object_list.end();object_list_iter++) {
		safe_delete(object_list_iter->second);
	}
	object_list.clear();
	map<int32, GroundSpawn*>::iterator groundspawn_list_iter;
	for(groundspawn_list_iter=groundspawn_list.begin();groundspawn_list_iter!=groundspawn_list.end();groundspawn_list_iter++) {
		safe_delete(groundspawn_list_iter->second);
	}
	groundspawn_list.clear();
	map<int32, Widget*>::iterator widget_list_iter;
	for(widget_list_iter=widget_list.begin();widget_list_iter!=widget_list.end();widget_list_iter++) {
		safe_delete(widget_list_iter->second);
	}
	widget_list.clear();
	map<int32, Sign*>::iterator sign_list_iter;
	for(sign_list_iter=sign_list.begin();sign_list_iter!=sign_list.end();sign_list_iter++) {
		safe_delete(sign_list_iter->second);
	}
	sign_list.clear();

	/*map<int32, AppearanceData*>::iterator appearance_list_iter;
	for(appearance_list_iter=npc_appearance_list.begin();appearance_list_iter!=npc_appearance_list.end();appearance_list_iter++) {
		safe_delete(appearance_list_iter->second);
	}
	npc_appearance_list.clear();*/

	
	ClearEntityCommands();

	DeleteGroundSpawnItems();
	DeleteTransporters();
	DeleteGlobalTransporters();
	DeleteTransporterMaps();
}

void ZoneServer::AddTransportMap(int32 id, string name) {
	MTransportMaps.writelock(__FUNCTION__, __LINE__);
	m_transportMaps[id] = name;
	MTransportMaps.releasewritelock(__FUNCTION__, __LINE__);
}

bool ZoneServer::TransportHasMap(int32 id) {
	bool ret = false;

	MTransportMaps.readlock(__FUNCTION__, __LINE__);
	ret = m_transportMaps.count(id) > 0;
	MTransportMaps.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

string ZoneServer::GetTransportMap(int32 id) {
	string ret;

	MTransportMaps.readlock(__FUNCTION__, __LINE__);
	if (m_transportMaps.count(id) > 0)
		ret = m_transportMaps[id];
	MTransportMaps.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

void ZoneServer::DeleteTransporterMaps() {
	MTransportMaps.writelock(__FUNCTION__, __LINE__);
	m_transportMaps.clear();
	MTransportMaps.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::ReloadSpawns() {
	if (reloading)
		return;

	reloading = true;
	world.SetReloadingSubsystem("Spawns");
	// Let every one in the zone know what is happening
	HandleBroadcast("Reloading all spawns for this zone.");
	DeleteGlobalSpawns();
	Depop(false, true);
}

void ZoneServer::SendStateCommand(Spawn* spawn, int32 state) {
	vector<Client*>::iterator itr;

	MClientList.readlock(__FUNCTION__, __LINE__);
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		Client* client = *itr;
		if (client && client->GetPlayer()->WasSentSpawn(spawn->GetID()))
			ClientPacketFunctions::SendStateCommand(client, client->GetPlayer()->GetIDWithPlayerSpawn(spawn), state);
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::AddFlightPath(int32 id, FlightPathInfo* info) {
	if (m_flightPaths.count(id) > 0) {
		LogWrite(ZONE__ERROR, 0, "Zone", "Duplicate flight path (%u)", id);
		safe_delete(info);
		return;
	}

	m_flightPaths[id] = info;
}

void ZoneServer::AddFlightPathLocation(int32 id, FlightPathLocation* location) {
	if (m_flightPaths.count(id) == 0) {
		LogWrite(ZONE__ERROR, 0, "Zone", "There is no flight info for this route (%u)", id);
		safe_delete(location);
		return;
	}

	m_flightPathRoutes[id].push_back(location);
}

void ZoneServer::DeleteFlightPaths() {
	map<int32, vector<FlightPathLocation*> >::iterator itr;
	vector<FlightPathLocation*>::iterator itr2;
	map<int32, FlightPathInfo*>::iterator itr3;

	for (itr = m_flightPathRoutes.begin(); itr != m_flightPathRoutes.end(); itr++) {
		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++) {
			safe_delete(*itr2);
		}

		itr->second.clear();
	}
	m_flightPathRoutes.clear();

	for (itr3 = m_flightPaths.begin(); itr3 != m_flightPaths.end(); itr3++) {
		safe_delete(itr3->second);
	}
	m_flightPaths.clear();
}

void ZoneServer::SendFlightPathsPackets(Client* client, int32 index) {
	if(client->GetVersion() <= 561 && index == 0xFFFFFFFF)
		return;
	
	// Only send a packet if there are flight paths
	if (m_flightPathRoutes.size() > 0) {
		PacketStruct* packet = configReader.getStruct("WS_FlightPathsMsg", client->GetVersion());
		if (packet) {
			int32 num_routes = m_flightPaths.size();
			if(index != 0xFFFFFFFF) {
				num_routes = 1;
			}
			packet->setArrayLengthByName("number_of_routes", num_routes);
			packet->setArrayLengthByName("number_of_routes2", num_routes);
			packet->setArrayLengthByName("number_of_routes3", num_routes);
			packet->setArrayLengthByName("number_of_routes4", num_routes);
			map<int32, FlightPathInfo*>::iterator itr;
			int32 i = 0;
			bool breakout = false;
			for (itr = m_flightPaths.begin(); itr != m_flightPaths.end(); itr++, i++) {
				
				if(index != 0xFFFFFFFF) {
					if(i != index) {
						continue;
					}
					else {
						i = 0;
						breakout = true;
					}
				}
				
				packet->setArrayDataByName("route_length", m_flightPathRoutes[itr->first].size(), i);
				packet->setArrayDataByName("ground_mount", itr->second->flying ? 0 : 1, i);
				packet->setArrayDataByName("allow_dismount", itr->second->dismount ? 1 : 0, i);
				packet->setSubArrayLengthByName("route_length2", m_flightPathRoutes[itr->first].size(), i);
				vector<FlightPathLocation*>::iterator itr2;
				int32 j = 0;
				for (itr2 = m_flightPathRoutes[itr->first].begin(); itr2 != m_flightPathRoutes[itr->first].end(); itr2++, j++) {
						packet->setSubArrayDataByName("x", (*itr2)->X, i, j);
						packet->setSubArrayDataByName("y", (*itr2)->Y, i, j);
						packet->setSubArrayDataByName("z", (*itr2)->Z, i, j);
				}
				if(breakout)
					break;
			}
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
}

int32 ZoneServer::GetFlightPathIndex(int32 id) {
	int32 index = 0;
	map<int32, FlightPathInfo*>::iterator itr;
	for (itr = m_flightPaths.begin(); itr != m_flightPaths.end(); itr++, index++) {
		if (itr->first == id)
			return index;
	}

	return -1;
}

float ZoneServer::GetFlightPathSpeed(int32 id) {
	float speed = 1;

	if (m_flightPaths.count(id) > 0)
		speed = m_flightPaths[id]->speed;

	return speed;
}

void ZoneServer::ProcessSpawnConditional(int8 condition) {
	MSpawnLocationList.readlock(__FUNCTION__, __LINE__);
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	map<int32, Spawn*>::iterator itr;
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		if (itr->second != NULL) // null itr->second still coming into ProcessSpawnConditional
		{
			SpawnLocation* loc = spawn_location_list[itr->second->GetSpawnLocationID()];
			if (loc && loc->conditional > 0) {
				if ((loc->conditional & condition) != condition) {
					Despawn(itr->second, 0);
				}
			}
		}
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);

	map<int32, SpawnLocation*>::iterator itr2;
	for (itr2 = spawn_location_list.begin(); itr2 != spawn_location_list.end(); itr2++) {
		SpawnLocation* loc = itr2->second;
		if (loc && loc->conditional > 0 && ((loc->conditional & condition) == condition))
			if (GetSpawnByLocationID(loc->placement_id) == NULL)
				ProcessSpawnLocation(loc, nullptr, nullptr, nullptr, nullptr, nullptr);
	}
	
	MSpawnLocationList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::AddSpawnProximities(Spawn* newSpawn) {
	Spawn* spawn = 0;
	map<int32, Spawn*>::iterator itr;
	MPendingSpawnListAdd.readlock(__FUNCTION__, __LINE__);
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if (spawn && spawn != newSpawn) {
			if (newSpawn->GetDatabaseID())
				spawn->AddSpawnToProximity(newSpawn->GetDatabaseID(), Spawn::SpawnProximityType::SPAWNPROXIMITY_DATABASE_ID);
			if (newSpawn->GetSpawnLocationID())
				spawn->AddSpawnToProximity(newSpawn->GetSpawnLocationID(), Spawn::SpawnProximityType::SPAWNPROXIMITY_LOCATION_ID);

			if (spawn->GetDatabaseID())
				newSpawn->AddSpawnToProximity(spawn->GetDatabaseID(), Spawn::SpawnProximityType::SPAWNPROXIMITY_DATABASE_ID);
			if (spawn->GetSpawnLocationID())
				newSpawn->AddSpawnToProximity(spawn->GetSpawnLocationID(), Spawn::SpawnProximityType::SPAWNPROXIMITY_LOCATION_ID);
		}
	}

	list<Spawn*>::iterator itr2;
	for (itr2 = pending_spawn_list_add.begin(); itr2 != pending_spawn_list_add.end(); itr2++) {
		spawn = *itr2;
		if (spawn && spawn != newSpawn) {
			if (newSpawn->GetDatabaseID())
				spawn->AddSpawnToProximity(newSpawn->GetDatabaseID(), Spawn::SpawnProximityType::SPAWNPROXIMITY_DATABASE_ID);
			if (newSpawn->GetSpawnLocationID())
				spawn->AddSpawnToProximity(newSpawn->GetSpawnLocationID(), Spawn::SpawnProximityType::SPAWNPROXIMITY_LOCATION_ID);

			if (spawn->GetDatabaseID())
				newSpawn->AddSpawnToProximity(spawn->GetDatabaseID(), Spawn::SpawnProximityType::SPAWNPROXIMITY_DATABASE_ID);
			if (spawn->GetSpawnLocationID())
				newSpawn->AddSpawnToProximity(spawn->GetSpawnLocationID(), Spawn::SpawnProximityType::SPAWNPROXIMITY_LOCATION_ID);
		}
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	MPendingSpawnListAdd.releasereadlock(__FUNCTION__, __LINE__);
}

// we only call this inside a write lock
void ZoneServer::RemoveSpawnProximities(Spawn* oldSpawn) {
	Spawn* spawn = 0;
	map<int32, Spawn*>::iterator itr;
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if (spawn && spawn != oldSpawn) {
			if (oldSpawn->GetDatabaseID())
				spawn->RemoveSpawnFromProximity(oldSpawn->GetDatabaseID(), Spawn::SpawnProximityType::SPAWNPROXIMITY_DATABASE_ID);
			if (oldSpawn->GetSpawnLocationID())
				spawn->RemoveSpawnFromProximity(oldSpawn->GetSpawnLocationID(), Spawn::SpawnProximityType::SPAWNPROXIMITY_LOCATION_ID);

			// don't need to remove oldSpawn proximities, we clear them all out
		}
	}
}

void ZoneServer::SetSpawnScript(SpawnEntry* entry, Spawn* spawn)
{
	if (!entry || !spawn)
		return;

	const char* script = 0;

	for (int x = 0; x < 3; x++)
	{
		switch (x)
		{
		case 0:
			script = world.GetSpawnEntryScript(entry->spawn_entry_id);
			break;
		case 1:
			script = world.GetSpawnLocationScript(entry->spawn_location_id);
			break;
		case 2:
			script = world.GetSpawnScript(entry->spawn_id);
			break;
		}

		if (script && lua_interface && lua_interface->GetSpawnScript(script) != 0)
		{
			spawn->SetSpawnScript(string(script));
			break;
		}
	}
}

vector<HouseItem> ZoneServer::GetHouseItems(Client* client)
{
	if (!client->GetCurrentZone()->GetInstanceID() || !client->HasOwnerOrEditAccess())
		return std::vector<HouseItem>();

	PacketStruct* packet = configReader.getStruct("WS_HouseItemsList", client->GetVersion());

	std::vector<HouseItem> items;
	map<int32, Spawn*>::iterator itr;
	Spawn* spawn = 0;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if (spawn && spawn->IsObject() && spawn->GetPickupItemID())
		{
			HouseItem tmpItem;
			tmpItem.item_id = spawn->GetPickupItemID();
			tmpItem.unique_id = spawn->GetPickupUniqueItemID();
			tmpItem.spawn_id = spawn->GetID();
			tmpItem.item = master_item_list.GetItem(spawn->GetPickupItemID());

			if (!tmpItem.item)
				continue;

			items.push_back(tmpItem);
		}
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);

	return items;
}

void ZoneServer::SendHouseItems(Client* client)
{
	if (!client->GetCurrentZone()->GetInstanceID() || !client->HasOwnerOrEditAccess())
		return;

	PacketStruct* packet = configReader.getStruct("WS_HouseItemsList", client->GetVersion());

	if(!packet) {
		return;
	}
	
	std::vector<HouseItem> items = GetHouseItems(client);

	// setting this to 1 puts it on the door widget
	packet->setDataByName("is_widget_door", 1);
	packet->setArrayLengthByName("num_items", items.size());
	for (int i = 0; i < items.size(); i++)
	{
		HouseItem tmpItem = items[i];
		packet->setArrayDataByName("unique_id", tmpItem.unique_id, i); // unique_id is in fact the item_id...
		packet->setArrayDataByName("item_name", tmpItem.item->name.c_str(), i);
		packet->setArrayDataByName("status_reduction", tmpItem.item->houseitem_info->status_rent_reduction, i);

		// location, 0 = floor, 1 = ceiling
		//packet->setArrayDataByName("location", 1, i, 0);

		// item_state int8
		// 0 = normal (cannot pick up item / move item / toggle visibility)
		// 1 = virtual (toggle visibility available, no move item)
		// 2 = hidden (cannot pick up item / move item / toggle visibility)
		// 3 = virtual/hidden/toggle visibility
		// 4 = none (cannot pick up item / move item / toggle visibility)
		// 5 = none, toggle visibility (cannot pick up item / move item)
		// 8 = none (cannot pick up item / move item / toggle visibility)
		//packet->setArrayDataByName("item_state", tmpvalue, i, 0);

		// makes it so we don't have access to move item/retrieve item
		// cannot use in conjunction with ui_tab_flag1/ui_tab_flag2
		//packet->setArrayDataByName("tradeable", 1, i);
		//packet->setArrayDataByName("item_description", "failboat", i);

		// access to move item/retrieve item, do not use in conjunction with tradeable
		packet->setArrayDataByName("ui_tab_flag1", 1, i, 0);
		packet->setArrayDataByName("ui_tab_flag2", 1, i, 0);

		// both of these can serve as description fields (only one should be used they populate the same area below the item name)
		//packet->setArrayDataByName("first_item_description", "test", i);
		//packet->setArrayDataByName("second_item_description", "Description here!", i);

		packet->setArrayDataByName("icon", tmpItem.item->GetIcon(client->GetVersion()), i);
	}

	EQ2Packet* pack = packet->serialize();
	client->QueuePacket(pack);
	safe_delete(packet);
}

Spawn* ZoneServer::GetSpawnFromUniqueItemID(int32 unique_id)
{
	if (!GetInstanceID() || GetInstanceType() != Instance_Type::PERSONAL_HOUSE_INSTANCE)
		return nullptr;

	map<int32, Spawn*>::iterator itr;
	Spawn* spawn = 0;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for (itr = spawn_list.begin(); itr != spawn_list.end(); itr++) {
		spawn = itr->second;
		if (spawn && spawn->IsObject() && spawn->GetPickupUniqueItemID() == unique_id)
		{
			Spawn* tmpSpawn = spawn;
			MSpawnList.releasereadlock();
			return tmpSpawn;
		}
	}
	MSpawnList.releasereadlock();

	return nullptr;
}

void ZoneServer::AddPendingSpawnRemove(int32 id)
{
		MPendingSpawnRemoval.writelock(__FUNCTION__, __LINE__);
		m_pendingSpawnRemove.insert(make_pair(id,true));
		MPendingSpawnRemoval.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::ProcessSpawnRemovals()
{
	MSpawnList.writelock(__FUNCTION__, __LINE__);
	MPendingSpawnRemoval.writelock(__FUNCTION__, __LINE__);
	if (m_pendingSpawnRemove.size() > 0) {
		map<int32,bool>::iterator itr2;
		for (itr2 = m_pendingSpawnRemove.begin(); itr2 != m_pendingSpawnRemove.end(); itr2++) {
			spawn_list.erase(itr2->first);
			subspawn_list[SUBSPAWN_TYPES::COLLECTOR].erase(itr2->first);
			
			std::map<int32,int32>::iterator hsmitr = housing_spawn_map.find(itr2->first);
			if(hsmitr != housing_spawn_map.end()) {
				subspawn_list[SUBSPAWN_TYPES::HOUSE_ITEM_SPAWN].erase(hsmitr->second);
				housing_spawn_map.erase(hsmitr);
			}
		}

		m_pendingSpawnRemove.clear();
	}
	MPendingSpawnRemoval.releasewritelock(__FUNCTION__, __LINE__);
	MSpawnList.releasewritelock(__FUNCTION__, __LINE__);
}

void ZoneServer::AddSpawnToGroup(Spawn* spawn, int32 group_id)
{
	if( spawn->GetSpawnGroupID() > 0 )
		spawn->RemoveSpawnFromGroup();
	MutexList<int32>* groupList = &spawn_group_map.Get(group_id);
	MutexList<int32>::iterator itr2 = groupList->begin();
	
	while(itr2.Next())
	{
		Spawn* groupSpawn = GetSpawnByID(itr2.value);
		if(groupSpawn)
		{
			// found existing group member to add it in
			spawn->AddSpawnToGroup(groupSpawn);
			break;
		}
	}
	groupList->Add(spawn->GetID());
	spawn->SetSpawnGroupID(group_id);
}

void ZoneServer::QueueStateCommandToClients(int32 spawn_id, int32 state)
{
	if(spawn_id < 1)
		return;

	MLuaQueueStateCmd.lock();
	lua_queued_state_commands.insert(make_pair(spawn_id, state));
	MLuaQueueStateCmd.unlock();
}

void ZoneServer::QueueDefaultCommand(int32 spawn_id, std::string command, float distance)
{
	if(spawn_id < 1)
		return;

	MLuaQueueStateCmd.lock();
	lua_spawn_update_command[spawn_id].insert(make_pair(command,distance));
	MLuaQueueStateCmd.unlock();
}

void ZoneServer::ProcessQueuedStateCommands() // in a client list lock only
{
	vector<Client*>::iterator itr;

	MLuaQueueStateCmd.lock();

	if(lua_queued_state_commands.size() > 0)
	{
		std::map<int32, int32>::iterator statecmds;
		for(statecmds = lua_queued_state_commands.begin(); statecmds != lua_queued_state_commands.end(); statecmds++)
		{
			Spawn* spawn = GetSpawnByID(statecmds->first, false);
			if(!spawn)
				continue;
			
			MClientList.readlock(__FUNCTION__, __LINE__);
			for (itr = clients.begin(); itr != clients.end(); itr++) {
				Client* client = *itr;
				if (client && client->GetPlayer()->WasSentSpawn(spawn->GetID()))
					ClientPacketFunctions::SendStateCommand(client, client->GetPlayer()->GetIDWithPlayerSpawn(spawn), statecmds->second);
			}
			MClientList.releasereadlock(__FUNCTION__, __LINE__);
		}
		lua_queued_state_commands.clear();
	}

	if(lua_spawn_update_command.size() > 0)
	{
		std::map<int32, std::map<std::string,float>>::iterator updatecmds;
		for(updatecmds = lua_spawn_update_command.begin(); updatecmds != lua_spawn_update_command.end(); updatecmds++)
		{
			Spawn* spawn = GetSpawnByID(updatecmds->first, false);
			if(!spawn)
				continue;
			
			std::map<std::string,float>::iterator innermap;
			for(innermap = lua_spawn_update_command[updatecmds->first].begin(); innermap != lua_spawn_update_command[updatecmds->first].end(); innermap++)
			{
				MClientList.readlock(__FUNCTION__, __LINE__);
				for (itr = clients.begin(); itr != clients.end(); itr++) {
					Client* client = *itr;
					if (client && client->GetPlayer()->WasSentSpawn(spawn->GetID()))
						client->SendDefaultCommand(spawn, innermap->first.c_str(), innermap->second);
				}
				MClientList.releasereadlock(__FUNCTION__, __LINE__);
			}
			lua_spawn_update_command[updatecmds->first].clear();
		}
		lua_spawn_update_command.clear();
	}
	MLuaQueueStateCmd.unlock();
}

void ZoneServer::RemoveClientsFromZone(ZoneServer* zone) {
	vector<Client*>::iterator itr;
	MClientList.readlock(__FUNCTION__, __LINE__);
	for (itr = clients.begin(); itr != clients.end(); itr++) {
		Client* client = *itr;
		if(client->GetCurrentZone() == zone) {
			client->SetCurrentZone(nullptr);
		}
		if(client->GetZoningDestination() == zone) {
			client->SetZoningDestination(nullptr);
		}
	}
	MClientList.releasereadlock(__FUNCTION__, __LINE__);
}

void ZoneServer::SendSubSpawnUpdates(SUBSPAWN_TYPES subtype) {
	std::map<int32, Spawn*>::iterator subitr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	for(subitr = subspawn_list[subtype].begin(); subitr !=  subspawn_list[subtype].end(); subitr++) {
		subitr->second->changed = true;
		subitr->second->info_changed = true;
		AddChangedSpawn(subitr->second);
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
}

bool ZoneServer::HouseItemSpawnExists(int32 item_id) {
	bool exists = false;
	std::map<int32, Spawn*>::iterator subitr;
	MSpawnList.readlock(__FUNCTION__, __LINE__);
	subitr = subspawn_list[SUBSPAWN_TYPES::HOUSE_ITEM_SPAWN].find(item_id);
	if(subitr != subspawn_list[SUBSPAWN_TYPES::HOUSE_ITEM_SPAWN].end()) {
		exists = true;
	}
	MSpawnList.releasereadlock(__FUNCTION__, __LINE__);
	return exists;
}

void ZoneServer::ProcessPendingSpawns() {
	MPendingSpawnListAdd.writelock(__FUNCTION__, __LINE__);
	list<Spawn*>::iterator itr2;
	for (itr2 = pending_spawn_list_add.begin(); itr2 != pending_spawn_list_add.end(); itr2++) {
		Spawn* spawn = *itr2;
		
		MSpawnList.writelock(__FUNCTION__, __LINE__);
		if (spawn)
			spawn_list[spawn->GetID()] = spawn;
		
		if(spawn->IsCollector()) {
			subspawn_list[SUBSPAWN_TYPES::COLLECTOR].insert(make_pair(spawn->GetID(),spawn));
		}
		if(spawn->GetPickupItemID()) {
			subspawn_list[SUBSPAWN_TYPES::HOUSE_ITEM_SPAWN].insert(make_pair(spawn->GetPickupItemID(),spawn));
			housing_spawn_map.insert(make_pair(spawn->GetID(), spawn->GetPickupItemID()));
		}
		MSpawnList.releasewritelock(__FUNCTION__, __LINE__);
		
		CheckSpawnRange(spawn);
	}

	pending_spawn_list_add.clear();
	MPendingSpawnListAdd.releasewritelock(__FUNCTION__, __LINE__);
	spawn_check_add.Trigger();
}

void ZoneServer::AddSpawnToGrid(Spawn* spawn, int32 grid_id) {
	if(spawn->GetID() == 0 || spawn->IsDeletedSpawn())
		return;
	
    MGridMaps.lock_shared();
	std::map<int32, GridMap*>::iterator grids = grid_maps.find(grid_id);
	if(grids != grid_maps.end()) {
		grids->second->MSpawns.lock();
		grids->second->spawns.insert(make_pair(spawn->GetID(), spawn));
		grids->second->MSpawns.unlock();
	}
	else {
		MGridMaps.unlock_shared();
		MGridMaps.lock();
		GridMap* gm = new GridMap;
		gm->grid_id = grid_id;
		gm->spawns.insert(make_pair(spawn->GetID(), spawn));
		grid_maps.insert(make_pair(grid_id, gm));
		MGridMaps.unlock();
		return;
	}
	
	MGridMaps.unlock_shared();
}

void ZoneServer::RemoveSpawnFromGrid(Spawn* spawn, int32 grid_id) {
    std::shared_lock lock(MGridMaps);
	std::map<int32, GridMap*>::iterator grids = grid_maps.find(grid_id);
	if(grids != grid_maps.end()) {
		grids->second->MSpawns.lock();
		if(grids->second->spawns.count(spawn->GetID()) > 0) {
			grids->second->spawns.erase(spawn->GetID());
		}
		grids->second->MSpawns.unlock();
	}
}

int32 ZoneServer::GetSpawnCountInGrid(int32 grid_id) {
	int32 count = 0;
    std::shared_lock lock(MGridMaps);
	std::map<int32, GridMap*>::iterator grids = grid_maps.find(grid_id);
	if(grids != grid_maps.end()) {
		grids->second->MSpawns.lock_shared();
		count = grids->second->spawns.size();
		grids->second->MSpawns.unlock_shared();
	}
	
	return count;
}

void ZoneServer::SendClientSpawnListInGrid(Client* client, int32 grid_id){
    std::shared_lock lock(MGridMaps);
	
	Spawn* spawn = nullptr;
	client->Message(CHANNEL_COLOR_RED, "Grid ID %u has %u spawns.", grid_id, GetSpawnCountInGrid(grid_id));
	std::map<int32, GridMap*>::iterator grids = grid_maps.find(grid_id);
	if(grids != grid_maps.end()) {
		grids->second->MSpawns.lock_shared();
		typedef map <int32, Spawn*> SpawnMapType;
		for( SpawnMapType::iterator it = grids->second->spawns.begin(); it != grids->second->spawns.end(); ++it ) {
			spawn = it->second;
			client->Message(CHANNEL_COLOR_YELLOW, "Spawn %s (%u), Loc X/Y/Z: %f/%f/%f.", spawn->GetName(), spawn->GetID(), spawn->GetX(), spawn->GetY(), spawn->GetZ());
		}
		grids->second->MSpawns.unlock_shared();
	}
}

void ZoneServer::AddIgnoredWidget(int32 id) {
	std::unique_lock lock(MIgnoredWidgets);
	if(ignored_widgets.find(id) == ignored_widgets.end()) {
		ignored_widgets.insert(make_pair(id,true));
	}
}

void ZoneServer::AddRespawn(Spawn* spawn) {
	int32 respawn_time = spawn->GetRespawnTime();
	if(spawn->GetRespawnOffsetLow() != 0 || spawn->GetRespawnOffsetHigh() != 0) {
		int random_offset = MakeRandomInt(spawn->GetRespawnOffsetLow(), spawn->GetRespawnOffsetHigh());
		int result_time = static_cast<int>(respawn_time) + random_offset;
		respawn_time = static_cast<unsigned>(std::max(result_time, 0));
	}
	AddRespawn(spawn->GetSpawnLocationID(), respawn_time);
}

void ZoneServer::AddRespawn(int32 locationID, int32 respawnTime) {
	if(locationID > 0 && respawnTime > 0) {
		respawn_timers.Put(locationID, Timer::GetCurrentTime2() + respawnTime * 1000);
	}
}