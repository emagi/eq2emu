/*  
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2005 - 2026  EQ2EMulator Development Team (http://www.eq2emu.com formerly http://www.eq2emulator.net)

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

#ifndef ZONESERVER_H
#define ZONESERVER_H

#include <mutex>
#include <shared_mutex>

#include "../common/linked_list.h"
#include "../common/timer.h"
#include "../common/queue.h"
#include "../common/servertalk.h"
#include "../common/TCPConnection.h"
#include "WorldTCPConnection.h"
#include "../common/Mutex.h"
#include "../common/DataBuffer.h"
#include "net.h"
#include "Player.h"
#include "Combat.h"
#include <list>
#include <map>
#include <set>
#include "MutexList.h"
#include "MutexMap.h"
#include "MutexVector.h"
#include "NPC.h"
#include "Widget.h"
#include "Object.h"
#include "GroundSpawn.h"
#include "Sign.h"
#include "Zone/map.h"
#include "Zone/pathfinder_interface.h"
#include "Zone/mob_movement_manager.h"
#include "Zone/region_map.h"

extern NetConnection net;		// needs to be here or compile errors in commands.cpp
class SpellProcess;
class TradeskillMgr;
class Bot;

#define EXPANSION_UNKNOWN	1
#define EXPANSION_UNKNOWN2	64
#define EXPANSION_UNKNOWN3	128
#define EXPANSION_UNKNOWN4	256
#define EXPANSION_UNKNOWN5	512
#define EXPANSION_DOF		1024
#define EXPANSION_KOS		2048
#define EXPANSION_EOF		4096
#define EXPANSION_ROK		8192
#define EXPANSION_TSO		16384
#define EXPANSION_DOV		65536 // This enables DoV and CoE AA tree's lower values disable both trees
// Can't verify these 3 values
// 32768 - SF
// 131072 - AoD

#define SPAWN_SCRIPT_SPAWN				0
#define SPAWN_SCRIPT_RESPAWN			1
#define SPAWN_SCRIPT_ATTACKED			2
#define SPAWN_SCRIPT_TARGETED			3
#define SPAWN_SCRIPT_HAILED				4
#define SPAWN_SCRIPT_DEATH				5
#define SPAWN_SCRIPT_KILLED				6
#define SPAWN_SCRIPT_AGGRO				7
#define SPAWN_SCRIPT_HEALTHCHANGED		8
#define SPAWN_SCRIPT_RANDOMCHAT			9
#define SPAWN_SCRIPT_CONVERSATION		10
#define SPAWN_SCRIPT_TIMER				11
#define SPAWN_SCRIPT_CUSTOM				12
#define SPAWN_SCRIPT_HAILED_BUSY		13
#define SPAWN_SCRIPT_CASTED_ON			14
#define SPAWN_SCRIPT_AUTO_ATTACK_TICK	15
#define SPAWN_SCRIPT_COMBAT_RESET		16
#define SPAWN_SCRIPT_GROUP_DEAD			17
#define SPAWN_SCRIPT_HEAR_SAY			18
#define SPAWN_SCRIPT_PRESPAWN			19
#define SPAWN_SCRIPT_USEDOOR			20
#define SPAWN_SCRIPT_BOARD				21
#define SPAWN_SCRIPT_DEBOARD			22

#define SPAWN_CONDITIONAL_NONE			0
#define SPAWN_CONDITIONAL_DAY			1
#define SPAWN_CONDITIONAL_NIGHT			2
#define SPAWN_CONDITIONAL_NOT_RAINING	4
#define SPAWN_CONDITIONAL_RAINING		8

#define MAX_REVIVEPOINT_DISTANCE 1000

/* JA: TODO Turn into R_World Rules */
#define SEND_SPAWN_DISTANCE 250		/* when spawns appear visually to the client */
#define HEAR_SPAWN_DISTANCE	30		/* max distance a client can be from a spawn to 'hear' it */
#define MAX_CHASE_DISTANCE 80
#define REMOVE_SPAWN_DISTANCE 300 // increased distance between send/remove is ideal, this makes sure there is no overlap if a 'fast' client (AKA GM warp speed)

#define TRACKING_STOP				0
#define TRACKING_START				1
#define TRACKING_UPDATE				2
#define TRACKING_CLOSE_WINDOW		3

#define TRACKING_TYPE_ENTITIES		1
#define TRACKING_TYPE_HARVESTABLES	2

#define TRACKING_SPAWN_TYPE_PC		0
#define TRACKING_SPAWN_TYPE_NPC		1

#define WAYPOINT_CATEGORY_GROUP			0
#define WAYPOINT_CATEGORY_QUESTS		1
#define WAYPOINT_CATEGORY_PEOPLE		2
#define WAYPOINT_CATEGORY_PLACES		3
#define WAYPOINT_CATEGORY_USER			4
#define WAYPOINT_CATEGORY_DIRECTIONS	5
#define WAYPOINT_CATEGORY_TRACKING		6
#define WAYPOINT_CATEGORY_HOUSES		7
#define WAYPOINT_CATEGORY_MAP			8

struct PlayerProximity{
	float				distance;
	string				in_range_lua_function;
	string				leaving_range_lua_function;
	map<Client*, bool>	clients_in_proximity;
};

struct LocationProximity {
	float				x;
	float				y;
	float				z;
	float				max_variation;
	string				in_range_lua_function;
	string				leaving_range_lua_function;
	map<Client*, bool>	clients_in_proximity;
};

struct LocationGrid {
	int32					id;
	int32					grid_id;
	string					name;
	bool					include_y;
	bool					discovery;
	MutexList<Location*>	locations;
	MutexMap<Player*, bool>	players;
};

struct GridMap {
	int32					grid_id;
	std::map<int32, Spawn*> spawns;
	mutable std::shared_mutex MSpawns;
};

struct TrackedSpawn {
	Spawn* spawn;
	float distance;
};

struct HouseItem {
	int32 spawn_id;
	int32 item_id;
	int64 unique_id;
	Item* item;
};

class Widget;
class Client;
class Sign;
class Object;
class GroundSpawn;
struct GroundSpawnEntry;
struct GroundSpawnEntryItem;
struct LootTable;
struct LootDrop;
struct GlobalLoot;
struct TransportDestination;
struct LocationTransportDestination;

#ifdef WIN32
	void ZoneLoop(void *tmp);
	void SpawnLoop(void *tmp);
	void SendInitialSpawns(void *tmp);
	void SendLevelChangedSpawns(void*tmp);
#else
	void *ZoneLoop(void *tmp);
	void *SpawnLoop(void *tmp);
	void *SendInitialSpawns(void *tmp);
	void *SendLevelChangedSpawns(void *tmp);
#endif
using namespace std;
struct RevivePoint{
	int32	id;
	int32	zone_id; //usually this zone, but not always
	string	location_name;
	float	x;
	float	y;
	float	z;
	float	heading;
	bool	always_included;
};

struct SpawnScriptTimer {
	int32	timer;
	int32	spawn;
	int32	player;
	string	function;
	int32	current_count;
	int32	max_count;
};

enum Instance_Type {
	NONE,
	GROUP_LOCKOUT_INSTANCE,
	GROUP_PERSIST_INSTANCE,
	RAID_LOCKOUT_INSTANCE,
	RAID_PERSIST_INSTANCE,
	SOLO_LOCKOUT_INSTANCE,
	SOLO_PERSIST_INSTANCE,
	TRADESKILL_INSTANCE,		// allows anyone to enter, server searches for the first instance that is available
	PUBLIC_INSTANCE,			// same as tradeskill, except dead spawns are tracked
	PERSONAL_HOUSE_INSTANCE,
	GUILD_HOUSE_INSTANCE,
	QUEST_INSTANCE
};

struct FlightPathInfo {
	float	speed;
	bool	flying;
	bool	dismount;
};

struct FlightPathLocation {
	float	X;
	float	Y;
	float	Z;
};

struct ZoneInfoSlideStructInfo {
	float unknown1[2];
	int32 unknown2[2];
	int32 unknown3;
	int32 unknown4;
	char slide[128];
	char voiceover[128];
	int32 key1;
	int32 key2;
};
struct ZoneInfoSlideStructTransitionInfo {
	int32 transition_x;
	int32 transition_y;
	float transition_zoom;
	float transition_time;
};
struct ZoneInfoSlideStruct {
	ZoneInfoSlideStructInfo* info;
	vector<ZoneInfoSlideStructTransitionInfo*> slide_transition_info;
};

enum SUBSPAWN_TYPES {
	COLLECTOR = 0,
	HOUSE_ITEM_SPAWN = 1,
	MAX_SUBSPAWN_TYPE = 20
};

// need to attempt to clean this up and add xml comments, remove unused code, find a logical way to sort the functions maybe by get/set/process/add etc...
class ZoneServer {
public:
	ZoneServer(const char* file);
    ~ZoneServer();
	
	void		IncrementIncomingClients();
	void		DecrementIncomingClients();
	void		Init();
	bool		Process();
	bool		SpawnProcess();

	ZoneInfoSlideStruct* GenerateSlideStruct(float unknown1a, float unknown1b, int32 unknown2a, int32 unknown2b, int32 unknown3, int32 unknown4, const char* slide, const char* voiceover, int32 key1, int32 key2);
	void AddZoneInfoSlideStructTransitionInfo(ZoneInfoSlideStruct* info, int32 x, int32 y, float zoom, float transition_time);	
	vector<ZoneInfoSlideStruct*>* GenerateTutorialSlides();
	
	void	LoadRevivePoints(vector<RevivePoint*>* revive_points);
	vector<RevivePoint*>* GetRevivePoints(Client* client);
	RevivePoint* GetRevivePoint(int32 id);

	void	AddClient(Client* client);
	
	void	SimpleMessage(int8 type, const char* message, Spawn* from, float distance, bool send_to_sender = true);
	void	HandleChatMessage(Spawn* from, const char* to, int16 channel, const char* message, float distance = 0, const char* channel_name = 0, bool show_bubble = true, int32 language = 0);
	void	HandleChatMessage(Client* client, Spawn* from, const char* to, int16 channel, const char* message, float distance = 0, const char* channel_name = 0, bool show_bubble = true, int32 language = 0);
	void	HandleChatMessage(Client* client, std::string fromName, const char* to, int16 channel, const char* message, float distance = 0, const char* channel_name = 0, int32 language = 0);
	void	HandleChatMessage(std::string fromName, const char* to, int16 channel, const char* message, float distance, const char* channel_name, int32 language);
	
	void	HandleBroadcast(const char* message);
	void	HandleAnnouncement(const char* message);
	
	int16	SetSpawnTargetable(Spawn* spawn, float distance);
	int16	SetSpawnTargetable(int32 spawn_id);
	void	ApplySetSpawnCommand(Client* client, Spawn* target, int8 type, const char* value);
	void	SetSpawnCommand(Spawn* spawn, int8 type, char* value, Client* client = 0);
	void	SetSpawnCommand(int32 spawn_id, int8 type, char* value, Client* client = 0);
	void	AddLoot(NPC* npc, Spawn* killer = nullptr, GroupLootMethod loot_method = GroupLootMethod::METHOD_FFA, int8 item_rarity = 0, int32 group_id = 0);
	
	NPC*	AddNPCSpawn(SpawnLocation* spawnlocation, SpawnEntry* spawnentry);
	Object*	AddObjectSpawn(SpawnLocation* spawnlocation, SpawnEntry* spawnentry);
	GroundSpawn*	AddGroundSpawn(SpawnLocation* spawnlocation, SpawnEntry* spawnentry);
	Widget*	AddWidgetSpawn(SpawnLocation* spawnlocation, SpawnEntry* spawnentry);
	Sign*	AddSignSpawn(SpawnLocation* spawnlocation, SpawnEntry* spawnentry);
	void	AddSpawn(Spawn* spawn);
	void	RemoveDeadEnemyList(Spawn* spawn);
	void	RemoveDeadSpawn(Spawn* spawn);
	
	void	AddSpawnGroupLocation(int32 group_id, int32 location_id, int32 spawn_location_id);
	void	AddSpawnGroupAssociation(int32 group_id1, int32 group_id2);
	
	void	AddSpawnGroupChance(int32 group_id, float percent);
	
	void	RemoveSpawn(Spawn* spawn, bool delete_spawn = true, bool respawn = true, bool lock = true, bool erase_from_spawn_list = true, bool lock_spell_process = false);
	void	ProcessSpawnLocations();
	void	SendQuestUpdates(Client* client, Spawn* spawn = 0);
	
	EQ2Packet* GetZoneInfoPacket(Client* client);
	Spawn*	FindSpawn(Player* searcher, const char* name);
	bool	CallSpawnScript(Spawn* npc, int8 type, Spawn* spawn = 0, const char* message = 0, bool is_door_open = false, sint32 input_value = 0, sint32* return_value = 0);
	void	SendSpawnVisualState(Spawn* spawn, int16 type);
	void	SendSpellFailedPacket(Client* client, int16 error);
	void	SendInterruptPacket(Spawn* interrupted, LuaSpell* spell, bool fizzle=false);
	void	HandleEmote(Spawn* originator, string name, Spawn* opt_target = nullptr, bool no_target = false);
	Spawn*	GetSpawnByDatabaseID(int32 id);
	Spawn*	GetSpawnByID(int32 id, bool spawnListLocked=false);
	
	void	PlaySoundFile(Client* client, const char* name, float origin_x, float origin_y, float origin_z);
	void	SendZoneSpawns(Client* client);
	void	StartZoneInitialSpawnThread(Client* client);
	void	SendSpawnChanges();
	void	SendSpawnChanges(Spawn* spawn);
	void	SendSpawnChanges(Spawn* spawn, Client* client, bool override_changes = false, bool override_vis_changes = false);
	void	SendSpawnChangesByDBID(int32 spawn_id, Client* client, bool override_changes = false, bool override_vis_changes = false);
	void	SendPlayerPositionChanges(Player* player);
	
	void	UpdateVitality(float amount);
	
	vector<Entity*> GetPlayers();
	
	void	KillSpawn(bool spawnListLocked, Spawn* dead, Spawn* killer, bool send_packet = true, int8 type = 0, int8 damage_type = 0, int16 kill_blow_type = 0);
	
	void	SendDamagePacket(Spawn* attacker, Spawn* victim, int8 type1, int8 type2, int8 damage_type, int16 damage, const char* spell_name);
	void    SendHealPacket(Spawn* caster, Spawn* target, int16 type, int32 heal_amt, const char* spell_name);
	
	void	SendCastSpellPacket(LuaSpell* spell, Entity* caster, int32 spell_visual_override = 0, int16 casttime_override = 0xFFFF);
	void	SendCastSpellPacket(int32 spell_visual, Spawn* target, Spawn* caster = 0);
	void	SendCastEntityCommandPacket(EntityCommand* entity_command, int32 spawn_id, int32 target_id);
	void	TriggerCharSheetTimer();
	
	/// <summary>Sends the game time packet to all connected clients</summary>
	void	SendTimeUpdateToAllClients();
	void	AddWidgetTimer(Spawn* widget, float time);
	bool	HasWidgetTimer(Spawn* widget);
	
	void	Despawn(Spawn* spawn, int32 timer);
	
	void	RepopSpawns(Client* client, Spawn* spawn);
	bool	AddCloseSpawnsToSpawnGroup(Spawn* spawn, float radius);
	void	Depop(bool respawns = false, bool repop = false);

	Spawn*	GetSpawnGroup(int32 id);
	bool	IsSpawnGroupAlive(int32 id);
	void	AddEnemyList(NPC* npc);
	
	void	ReloadClientQuests();
	void	SendAllSpawnsForLevelChange(Client* client);
	void	SendAllSpawnsForSeeInvisChange(Client* client);
	void	SendAllSpawnsForVisChange(Client* client, bool limitToEntities=true);
	
	void	AddLocationGrid(LocationGrid* grid);
	void	RemoveLocationGrids();

	void	DeleteTransporters();
	
	void	CheckTransporters(Client* client);
	
	void	WritePlayerStatistics();

	bool	SendRadiusSpawnInfo(Client* client, float radius);
	void	FindSpawn(Client* client, char* regSearchStr);

	volatile bool	spawnthread_active;
	volatile bool	combatthread_active;
	volatile int8	initial_spawn_threads_active;
	volatile bool	client_thread_active;
	void	AddChangedSpawn(Spawn* spawn);
	
	void	AddDamagedSpawn(Spawn* spawn);
	
	void	AddDrowningVictim(Player* player);
	void	RemoveDrowningVictim(Player* player);
	Client* GetDrowningVictim(Player* player);
	
	void	DeleteSpellProcess();
	void	LoadSpellProcess();
	void	LockAllSpells(Player* player);
	void	UnlockAllSpells(Player* player);
	void	RemoveSpellTimersFromSpawn(Spawn* spawn, bool remove_all, bool delete_recast = true, bool call_expire_function = true, bool lock_spell_process = false);
	void	Interrupted(Entity* caster, Spawn* interruptor, int16 error_code, bool cancel = false, bool from_movement = false);
	Spell*	GetSpell(Entity* caster);
	void	ProcessSpell(Spell* spell, Entity* caster, Spawn* target = 0, bool lock = true, bool harvest_spell = false, LuaSpell* customSpell = 0, int16 custom_cast_time = 0, bool in_heroic_opp = false);
	void	ProcessEntityCommand(EntityCommand* entity_command, Entity* caster, Spawn* target, bool lock = true);
	void	AddPlayerTracking(Player* player);
	void	RemovePlayerTracking(Player* player, int8 mode);
	
	void	SendUpdateTitles(Client *client, Title *suffix = 0, Title *prefix = 0);
	void	SendUpdateTitles(Spawn *spawn, Title *suffix = 0, Title *prefix = 0);
	
	void    RemoveTargetFromSpell(LuaSpell* spell, Spawn* target, bool remove_caster = false);

	/// <summary>Set the rain levl in the zone</summary>
	/// <param name='val'>Level of rain in the zone 0.0 - 1.1 (rain starts at 0.76)</param>
	void	SetRain(float val);

	/// <summary>Sets the wind direction</summary>
	/// <param name='val'>Direction in degrees to set the wind</param>
	void	SetWind(float val);

	/// <summary>Handles zone-wide weather changes</summary>
	void	ProcessWeather();
	
	Spawn*  GetClosestTransportSpawn(float x, float y, float z);
	Spawn*  GetTransportByRailID(sint64 rail_id);
	
	void    ResurrectSpawn(Spawn* spawn, Client* client);

	void	HidePrivateSpawn(Spawn* spawn);
	Client*	GetClientByName(char* name);
	Client*	GetClientByCharID(int32 charid);
	
	bool SetPlayerTargetByName(Client* originator, char* targetName, float distance);
	std::vector<int32> GetGridsByLocation(Spawn* originator, glm::vec3 loc, float distance);
	/// <summary>Gets spawns for a true AoE spell</summary>
	std::vector<std::pair<int32, float>> GetAttackableSpawnsByDistance(Spawn* spawn, float distance);
	std::vector<std::pair<int32, float>> GetSpawnsByDistance(Spawn* spawn, float max_distance, bool include_players);

	// Comparator function to sort by the value (second element of the pair)
	static bool compareByValue(const std::pair<int32, float>& a, const std::pair<int32, float>& b) {
		return a.second < b.second;
	}

	void StartZoneSpawnsForLevelThread(Client* client);

    void SendDispellPacket(Entity* caster, Spawn* target, string dispell_name, string spell_name, int8 dispell_type);

	void SetupInstance(int32 createdInstanceID=0);
	void SendUpdateDefaultCommand(Spawn* spawn, const char* command, float distance, Spawn* toplayer = NULL);

	map<int32, int32>* GetSpawnLocationsByGroup(int32 group_id);

	IPathfinder* pathing;
	MobMovementManager* movementMgr;

	/****************************************************
	Following functions are only used for LUA commands
	****************************************************/

	int32			GetClosestLocation(Spawn* spawn);
	Spawn*			GetClosestSpawn(Spawn* spawn, int32 spawn_id);
	SpawnLocation*	GetSpawnLocation(int32 id);
	void			PlayFlavor(Client* client, Spawn* spawn, const char* mp3, const char* text, const char* emote, int32 key1, int32 key2, int8 language);
	void			PlayVoice(Client* client, Spawn* spawn, const char* mp3, int32 key1, int32 key2);
	void			PlayFlavor(Spawn* spawn, const char* mp3, const char* text, const char* emote, int32 key1, int32 key2, int8 language);
	void			PlayFlavorID(Spawn* spawn, int8 type, int32 id, int16 index, int8 language);
	void			PlayVoice(Spawn* spawn, const char* mp3, int32 key1, int32 key2);
	void			SendThreatPacket(Spawn* caster, Spawn* target, int32 threat_amt, const char* spell_name);
	void			SendYellPacket(Spawn* yeller, float max_distance=50.0f);
	void			KillSpawnByDistance(Spawn* spawn, float max_distance, bool include_players = false, bool send_packet = false);
	void			SpawnSetByDistance(Spawn* spawn, float max_distance, string field, string value);
	void			AddSpawnScriptTimer(SpawnScriptTimer* timer);
	Spawn*			GetSpawnByLocationID(int32 location_id);
	void			AddMovementNPC(Spawn* spawn);
	void			AddPlayerProximity(Spawn* spawn, float distance, string in_range_function, string leaving_range_function);
	void			AddLocationProximity(float x, float y, float z, float max_variation, string in_range_function, string leaving_range_function);
	void			PlayAnimation(Spawn* spawn, int32 visual_state, Spawn* spawn2 = 0, int8 type = 1);
	void			AddTransportSpawn(Spawn* spawn);
	vector<Spawn*>	GetSpawnsByID(int32 id);
	vector<Spawn*>	GetSpawnsByRailID(sint64 rail_id);
	void			RemovePlayerPassenger(int32 char_id);
	bool			IsDusk() { return isDusk; }										// never used, probably meant for lua though


	/****************************************************
	Following functions are all contained in the header
	****************************************************/

	inline const char*	GetZoneName()	{ return zone_name; }
	void	SetZoneName(char* new_zone) { 
		if( strlen(new_zone) >= sizeof zone_name )
			return;
		strcpy(zone_name, new_zone); 
	}
	inline const char* GetZoneFile() { return zone_file; }
	void	SetZoneFile(char* zone) {
		if (strlen(zone) >= sizeof zone_file)
			return;
		strcpy(zone_file, zone);
	}
	inline const char* GetZoneSkyFile() { return zonesky_file; }
	void	SetZoneSkyFile(char* zone) {
		if (strlen(zone) >= sizeof zonesky_file)
			return;
		strcpy(zonesky_file, zone);
	}
	inline const char*	GetZoneDescription() { return zone_description; }
	void	SetZoneDescription(char* desc) { 
		if( strlen(desc) >= sizeof zone_description )
			return;
		strncpy(zone_description, desc, 255); 
	}
	
	void			SetUnderWorld(float under){ underworld = under; }
	float			GetUnderWorld(){ return underworld; }
	
	inline int32	GetZoneID()		{ return zoneID; }
	void			SetZoneID(int32 new_id){ zoneID = new_id; }
	
	inline bool		IsCityZone()	{ return cityzone; }
	inline bool		AlwaysLoaded()	{ return always_loaded; }
	inline bool		DuplicatedZone()	{ return duplicated_zone; }
	inline int32	DuplicatedID()	{ return duplicated_id; }
	void			SetCityZone(bool val) { cityzone = val; }
	void			SetAlwaysLoaded(bool val) { always_loaded = val; }
	void			SetDuplicatedZone(bool val) { duplicated_zone = val; }
	void			SetDuplicatedID(int32 id) { duplicated_id = id; }
	int32			NumPlayers()	{ return pNumPlayers; }
	void			SetMinimumStatus(sint16 minStatus) { minimumStatus = minStatus; }
	sint16			GetMinimumStatus() { return minimumStatus; }
	void			SetMinimumLevel(int16 minLevel) { minimumLevel = minLevel; }
	void			SetMaximumLevel(int16 maxLevel) { maximumLevel = maxLevel; }
	void			SetMinimumVersion(int16 minVersion) { minimumVersion = minVersion; }
	int16			GetMinimumLevel() { return minimumLevel; }
	int16			GetMaximumLevel() { return maximumLevel; }
	int16			GetMinimumVersion() { return minimumVersion; }
	inline bool		GetZoneLockState() { return locked; }						// JA: /zone lock|unlock
	void			SetZoneLockState(bool lock_state) { locked = lock_state; }	// JA: /zone lock|unlock
	int32			GetInstanceID() { return instanceID; }
	bool			IsInstanceZone() { return isInstance; }
	void			SetInstanceID(int32 newInstanceID) { instanceID = newInstanceID; }
	void SetShutdownTimer(int val){ 
		shutdownTimer.SetTimer(val*1000);
	}
	bool			IsShutdownTimerEnabled() { return shutdownTimer.Enabled(); }
	int32			GetShutdownRemainingTime() { return shutdownTimer.GetRemainingTime(); }
	
	void AddSpawnLocation(int32 id, SpawnLocation* spawnlocation) {
		MSpawnLocationList.writelock(__FUNCTION__, __LINE__);
		if (spawn_location_list.count(id) > 0)
			safe_delete(spawn_location_list[id]);
		spawn_location_list[id] = spawnlocation;
		MSpawnLocationList.releasewritelock(__FUNCTION__, __LINE__);
	}
	
	void			SetInstanceType(int16 type) { InstanceType = (Instance_Type)type; if(type>0)isInstance=true; else isInstance=false; }
	Instance_Type	GetInstanceType() { return InstanceType; }
	float			GetSafeX(){ return safe_x; }
	float			GetSafeY(){ return safe_y; }
	float			GetSafeZ(){ return safe_z; }
	float			GetSafeHeading() { return safe_heading; }
	void			SetSafeX(float val){ safe_x = val; }
	void			SetSafeY(float val){ safe_y = val; }
	void			SetSafeZ(float val){ safe_z = val; }
	void			SetSafeHeading(float val) { safe_heading = val; }
	float			GetXPModifier() { return xp_mod; }
	void			SetXPModifier(float val) { xp_mod = val; }
	void			SetZoneMOTD(string z_motd) { zone_motd = z_motd; }
	string			GetZoneMOTD() { return zone_motd; }
	bool			isZoneShuttingDown ( ) { return zoneShuttingDown; }
	void			Shutdown(){ zoneShuttingDown = true; }
	int32			GetClientCount(){ return clients.size(); }
	int32			GetDefaultLockoutTime() { return def_lockout_time; }
	int32			GetDefaultReenterTime() { return def_reenter_time; }
	int32			GetDefaultResetTime() { return def_reset_time; }
	int8			GetForceGroupZoneOption() { return group_zone_option; }
	void			SetDefaultLockoutTime(int32 val) { def_lockout_time = val; }
	void			SetDefaultReenterTime(int32 val) { def_reenter_time = val; }
	void			SetDefaultResetTime(int32 val) { def_reset_time = val; }
	void			SetForceGroupZoneOption(int8 val) { group_zone_option = val; }
	SpellProcess*	GetSpellProcess() {return spellProcess;}
	bool			FinishedDepop(){ return finished_depop; }

	/// <summary>Returns the Tradeskill Manager for this zone</summary>
	TradeskillMgr*	GetTradeskillMgr() { return tradeskillMgr; }
	

	// had to add these to access weather from Commands
	bool	isWeatherEnabled() { return weather_enabled; }
	void	SetWeatherEnabled(bool val) { weather_enabled = val; }
	bool	isWeatherAllowed() { return weather_allowed; }
	void	SetWeatherAllowed(bool val) { weather_allowed = val; }
	int8	GetWeatherType() { return weather_type; }
	void	SetWeatherType(int8 val) { weather_type = val; }
	int32	GetWeatherFrequency() { return weather_frequency; }
	void	SetWeatherFrequency(int32 val) { weather_frequency = val; }
	float	GetWeatherMinSeverity() { return weather_min_severity; }
	void	SetWeatherMinSeverity(float val) { weather_min_severity = val; }
	float	GetWeatherMaxSeverity() { return weather_max_severity; }
	void	SetWeatherMaxSeverity(float val) { weather_max_severity = val; }
	float	GetWeatherChangeAmount() { return weather_change_amount; }
	void	SetWeatherChangeAmount(float val) { weather_change_amount = val; }
	float	GetWeatherDynamicOffset() { return weather_dynamic_offset; }
	void	SetWeatherDynamicOffset(float val) { weather_dynamic_offset = val; }
	int8	GetWeatherChance() { return weather_change_chance; }
	void	SetWeatherChance(int8 val) { weather_change_chance = val; }
	float	GetCurrentWeather() { return weather_current_severity; }
	void	SetCurrentWeather(float val) { weather_current_severity = val; }
	int8	GetWeatherPattern() { return weather_pattern; }
	void	SetWeatherPattern(int8 val) { weather_pattern = val; }
	void	SetWeatherLastChangedTime(int32 val) { weather_last_changed_time = val; }

	int32	GetExpansionFlag() { return expansion_flag; }
	void	SetExpansionFlag(int32 val) { expansion_flag = val; }

	int32	GetHolidayFlag() { return holiday_flag; }
	void	SetHolidayFlag(int32 val) { holiday_flag = val; }

	int32	GetCanBind() { return can_bind; }
	void	SetCanBind(int32 val) { can_bind = val; }

	bool	GetCanGate() { return can_gate; }
	void	SetCanGate(int32 val) { can_gate = val; }

	bool	GetCanEvac() { return can_evac; }
	void	SetCanEvac(int32 val) { can_evac = val; }

	void	RemoveClientImmediately(Client* client);

	void	ClearHate(Entity* entity);



	/****************************************************
	Following functions are pending deletion, left in for
	now just to make sure one won't be of future use.
	****************************************************/
	//void	RemoveFromRangeMap(Client* client);																	// never used?
	//void	AddSpawnAssociatedGroup(vector<int32>* ret, int32 group_id);										// never used, not even any code for it
	//inline const char*	GetCAddress()	{ return clientaddress; }		// never used?
	//inline int16		GetCPort()		{ return clientport; }			// never used?
	//inline bool			IsBootingUp()	{ return BootingUp; }			// never used?
	//int32	GetShutdownTimer() {return shutdownTimer.GetTimerTime();}	// never used

	// Following were private

	//char	clientaddress[250];		// never used
	//int16	clientport;				// never used
	//bool	BootingUp;				// never used
	//bool	authenticated;			// never used?
	//int16	next_index;			// never used

















	void AddFlightPath(int32 id, FlightPathInfo* info);
	void AddFlightPathLocation(int32 id, FlightPathLocation* location);
	void DeleteFlightPaths();
	void SendFlightPathsPackets(Client* client, int32 index = 0xFFFFFFFF);
	int32 GetFlightPathIndex(int32 id);
	float GetFlightPathSpeed(int32 id);


	void	SendSpawn(Spawn* spawn, Client* client);														// moved from private to public for bots

	void ProcessSpawnConditional(int8 condition);

	void SetSpawnStructs(Client* client);

	void AddSpawnProximities(Spawn* spawn);
	void RemoveSpawnProximities(Spawn* spawn);
	void SetSpawnScript(SpawnEntry* entry, Spawn* spawn);
	bool IsLoading() {
		return LoadingData;
	}

	vector<HouseItem> GetHouseItems(Client* client);
	Spawn* GetSpawnFromUniqueItemID(int32 unique_id);
	void SendHouseItems(Client* client);

	int32 GetWatchdogTime() { return watchdogTimestamp; }
	void SetWatchdogTime(int32 time) { watchdogTimestamp = time; }
	void CancelThreads();

	bool	SendRemoveSpawn(Client* client, Spawn* spawn, PacketStruct* packet = 0, bool delete_spawn = false);

	void	AddSpawnToGroup(Spawn* spawn, int32 group_id);

	void	QueueStateCommandToClients(int32 spawn_id, int32 state);
	void	QueueDefaultCommand(int32 spawn_id, std::string command, float distance);
	void	ProcessQueuedStateCommands();
	void	RemoveClientsFromZone(ZoneServer* zone);

	void	WorldTimeUpdateTrigger() { sync_game_time_timer.Trigger(); }
	void	StopSpawnScriptTimer(Spawn* spawn, std::string functionName);

	Client*	RemoveZoneServerFromClient(ZoneServer* zone);
	
	void	SendSubSpawnUpdates(SUBSPAWN_TYPES subtype);
	bool	HouseItemSpawnExists(int32 item_id);
	void	ProcessPendingSpawns();
	void 	AddSpawnToGrid(Spawn* spawn, int32 grid_id);
	void	RemoveSpawnFromGrid(Spawn* spawn, int32 grid_id);
	int32	GetSpawnCountInGrid(int32 grid_id);
	void	SendClientSpawnListInGrid(Client* client, int32 grid_id);
	
	void 	AddIgnoredWidget(int32 id);	
	
	void	AddRespawn(Spawn* spawn);
	void	AddRespawn(int32 locationID, int32 respawnTime);
	
	void	SendRespawnTimerList(Client* client);
private:
#ifndef WIN32
	pthread_t ZoneThread;
	pthread_t SpawnThread;
#endif

	/* Private Functions */
	void	AddTransporter(LocationTransportDestination* loc);
	void	CheckDeadSpawnRemoval();
	void	DeleteData(bool boot_clients = true);
	void	DeleteFactionLists();
	void	ProcessDepop(bool respawns_allowed = false, bool repop = false);

	/*
	Following functions were public but never used outside the zone server so moved them to private
	*/
	void	ClientProcess(bool ignore_shutdown_timer = false);													// never used outside zone server
	void	RemoveClient(Client* client);																		// never used outside zone server
	void	DeterminePosition(SpawnLocation* spawnlocation, Spawn* spawn);										// never used outside zone server
	void	AddDeadSpawn(Spawn* spawn, int32 timer = 0xFFFFFFFF);												// never used outside zone server
	int32	CalculateSpawnGroup(SpawnLocation* spawnlocation, bool respawn = false);							// never used outside zone server
	float	GetSpawnGroupChance(int32 group_id);																// never used outside zone server
	vector<int32>*	GetAssociatedLocations(set<int32>* groups);													// never used outside zone server
	set<int32>* GetAssociatedGroups(int32 group_id);															// never used outside zone server
	list<int32>* GetSpawnGroupsByLocation(int32 location_id);													// never used outside zone server
	void	ProcessSpawnLocation(int32 location_id, map<int32,int32>* instNPCs, map<int32,int32>* instGroundSpawns, map<int32,int32>* instObjSpawns, map<int32,int32>* instWidgetSpawns, map<int32,int32>* instSignSpawns, bool respawn = false);										// never used outside zone server
	Spawn*	ProcessSpawnLocation(SpawnLocation* spawnlocation, map<int32,int32>* instNPCs, map<int32,int32>* instGroundSpawns, map<int32,int32>* instObjSpawns, map<int32,int32>* instWidgetSpawns, map<int32,int32>* instSignSpawns, bool respawn = false);							// never used outside zone server
	Spawn*	ProcessInstanceSpawnLocation(SpawnLocation* spawnlocation, map<int32,int32>* instNPCs, map<int32,int32>* instGroundSpawns, map<int32,int32>* instObjSpawns, map<int32,int32>* instWidgetSpawns, map<int32,int32>* instSignSpawns, bool respawn = false);													// never used outside zone server
	void	SendRaidSheetChanges();																				// never used outside zone server
	void	SendCharSheetChanges();																				// never used outside zone server
	void	SendCharSheetChanges(Client* client);																// never used outside zone server
	void	SaveClients();																						// never used outside zone server
	void	CheckSendSpawnToClient();																			// never used outside zone server
	void	CheckSendSpawnToClient(Client* client, bool initial_login = false);									// never used outside zone server
	void	CheckRemoveSpawnFromClient(Spawn* spawn);															// never used outside zone server
	void	SaveClient(Client* client);																			// never used outside zone server
	void	ProcessFaction(Spawn* spawn, Client* client);														// never used outside zone server
	void	RegenUpdate();																						// never used outside zone server
	void	SendCalculatedXP(Player* player, Spawn* victim);													// never used outside zone server, might not be used at all any more
	void	SendTimeUpdate(Client* client);																		// never used outside zone server
	void	CheckWidgetTimers();																				// never used outside zone server
	void	CheckRespawns();																					// never used outside zone server
	void	CheckSpawnExpireTimers();																			// never used outside zone server
	void	AddSpawnExpireTimer(Spawn* spawn, int32 expire_time, int32 expire_offset = 0);						// never used outside zone server
	void	CheckSpawnRange(Client* client, Spawn* spawn, bool initial_login = false);							// never used outside zone server
	void	CheckSpawnRange(Spawn* spawn);																		// never used outside zone server
	void	DeleteSpawnScriptTimers(Spawn* spawn, bool all = false);											// never used outside zone server
	void	DeleteSpawnScriptTimers();																			// never used outside zone server
	void	CheckSpawnScriptTimers();																			// never used outside zone server
	bool	PrepareSpawnID(Player* player, Spawn* spawn);														// never used outside zone server
	void	RemoveMovementNPC(Spawn* spawn);																	// never used outside zone server
	bool	CheckNPCAttacks(NPC* npc, Spawn* victim, Client* client = 0);										// never used outside zone server
	bool	AggroVictim(NPC* npc, Spawn* victim, Client* client = 0);											// never used outside zone server
	bool	CheckEnemyList(NPC* npc);																			// never used outside zone server
	void	RemovePlayerProximity(Spawn* spawn, bool all = false);												// never used outside zone server
	void	RemovePlayerProximity(Client* client);																// never used outside zone server
	void	CheckPlayerProximity(Spawn* spawn, Client* client);													// never used outside zone server
	void	RemoveLocationProximities();																		// never used outside zone server
	void	CheckLocationProximity();																			// never used outside zone server
	void	CheckLocationGrids();																				// never used outside zone server
	void	RemoveSpawnSupportFunctions(Spawn* spawn, bool lock_spell_process = false, bool shutdown = false);	// never used outside zone server
	void	ReloadTransporters();																				// never used outside zone server
	void	DeleteSpawns(bool delete_all);																		// never used outside zone server
	void	AddPendingDelete(Spawn* spawn);																		// never used outside zone server
	void	ClearDeadSpawns();																					// never used outside zone server
	void	RemoveChangedSpawn(Spawn* spawn);																	// never used outside zone server
	void	ProcessDrowning();																					// never used outside zone server
	void	RemoveDamagedSpawn(Spawn* spawn);																	// never used outside zone server
	void	ProcessTracking();																					// never used outside zone server
	void	ProcessTracking(Client* client);																	// never used outside zone server
	void	SendEpicMobDeathToGuild(Player* killer, Spawn* victim);												// never used outside zone server
	void	ProcessAggroChecks(Spawn* spawn);																	// never used outside zone server
	/// <summary>Checks to see if it is time to remove a spawn and removes it</summary>
	/// <param name='force_delete_all'>Forces all spawns scheduled to be removed regardless of time</param>
	bool CombatProcess(Spawn* spawn);																			// never used outside zone server
	void LootProcess(Spawn* spawn);
	void CloseSpawnLootWindow(Spawn* spawn);
	void	InitWeather();																						// never used outside zone server
	///<summary>Dismiss all pets in the zone, useful when the spell process needs to be reloaded</summary>
	void DismissAllPets();																						// never used outside zone server
	
	/* Mutex Lists */
	std::map<int32, bool> changed_spawns;										// int32 = spawn id
	vector<Client*> clients;
	MutexList<Client*> connected_clients;									// probably remove this list so we are not maintaining 2 client lists
	MutexList<int32> damaged_spawns;										// int32 = spawn id
	MutexList<LocationProximity*> location_proximities;
	MutexList<LocationGrid*> location_grids;
	MutexList<int32> remove_movement_spawns;								// int32 = spawn id
	set<SpawnScriptTimer*> spawn_script_timers;
	Mutex MSpawnScriptTimers;
	set<SpawnScriptTimer*> remove_spawn_script_timers_list;
	Mutex MRemoveSpawnScriptTimersList;
	list<LocationTransportDestination*> transporter_locations;
	
	/* Mutex Maps */
	MutexMap<Client*, int32>						drowning_victims;
	MutexMap<int32, int32>							movement_spawns;								// 1st int32 = spawn id
	MutexMap<int32, PlayerProximity*>				player_proximities;								// 1st int32 = spawn id
	MutexMap<int32, Player*>						players_tracking;
	MutexMap<int32, int32>							quick_database_id_lookup;						// 1st int32 = database id, 2nd int32 = spawn id
	MutexMap<int32, int32>							quick_location_id_lookup;						// 1st int32 = location id, 2nd int32 = spawn id
	MutexMap<int32, int32>							quick_group_id_lookup;							// 1st int32 = group id, 2nd int32 = spawn id
	MutexMap<int32, int32>							respawn_timers;
	map<Spawn*, int32>								spawn_delete_list;
	MutexMap<int32, int32>							spawn_expire_timers;							// 1st int32 = spawn id
	map<int32, set<int32>* >						spawn_group_associations;
	map<int32, float>								spawn_group_chances;
	map<int32, map<int32, int32>* >					spawn_group_locations;
	MutexMap<int32, MutexList<int32> >				spawn_group_map;								// MutexList<int32> is a list of spawn id's
	map<int32, list<int32>* >						spawn_location_groups;
	map<int32, SpawnLocation*>						spawn_location_list;
	MutexMap<Client*, MutexMap<int32, float >* >	spawn_range_map;								// int32 in the MutexMap<int32, float>* = spawn id, float = distance
	Mutex MWidgetTimers;
	map<int32, int32>								widget_timers;									// 1st int32 = spawn id

	std::map<int32, GridMap*> grid_maps;
	
	/* Mutexs */
	mutable std::shared_mutex MGridMaps;
	mutable std::shared_mutex MChangedSpawns;
	
	Mutex	m_enemy_faction_list;
	Mutex	m_npc_faction_list;
	Mutex	m_reverse_enemy_faction_list;
	Mutex	MDeadSpawns;
	CriticalSection* MMasterZoneLock; //This needs to be a recursive lock to fix a possible /reload spells crash with multiple zones loaded - Foof
	Mutex	MMasterSpawnLock;
	Mutex	MPendingSpawnListAdd;
	Mutex	MSpawnList;
	Mutex	MTransportSpawns;
	Mutex	MSpawnGroupAssociation;
	Mutex	MSpawnGroupLocations;
	Mutex	MSpawnLocationGroups;
	Mutex	MSpawnGroupChances;
	Mutex	MTransportLocations;
	Mutex	MSpawnLocationList;
	Mutex	MSpawnDeleteList;
	Mutex	MClientList;
	Mutex	MIncomingClients;
	
	/* Maps */
	map<int32, int32>							dead_spawns;
	map<int32, vector<int32>* >					enemy_faction_list;
	map<int32, vector<int32>* >					npc_faction_list;
	map<int32, vector<int32>* >					reverse_enemy_faction_list;
	map<int32, Spawn*>							spawn_list;
	map<int32, FlightPathInfo*>					m_flightPaths;
	map<int32, vector<FlightPathLocation*> >	m_flightPathRoutes;

	/* Lists */
	list<Spawn*>	pending_spawn_list_add;
	
	/* Specialized Lists to update specific scenarios */
	std::map<int32, Spawn*>	subspawn_list[SUBSPAWN_TYPES::MAX_SUBSPAWN_TYPE];
	std::map<int32, int32>	housing_spawn_map;
	
	/* Vectors */
	vector<RevivePoint*>*	revive_points;
	vector<int32>			transport_spawns;

	/* Classes */
	SpellProcess*	spellProcess;
	TradeskillMgr*	tradeskillMgr;
	
	/* Timers */
	Timer	aggro_timer;
	Timer	charsheet_changes;
	Timer	client_save;
	Timer	location_prox_timer;
	Timer	location_grid_timer;
	Timer	movement_timer;
	Timer	regenTimer;
	Timer	respawn_timer;
	Timer	shutdownTimer;
	Timer	startupDelayTimer;
	Timer	spawn_check_add;
	Timer	spawn_check_remove;
	Timer	spawn_expire_timer;
	Timer	spawn_range;
	Timer	spawn_update;
	Timer	sync_game_time_timer;
	Timer	tracking_timer;
	Timer	weatherTimer;
	Timer	widget_timer;
	Timer	queue_updates;
	Timer	shutdownDelayTimer;
	Timer	delete_timer;
	
	/* Enums */
	Instance_Type InstanceType;

	/* Variables */
	volatile bool	finished_depop;
	volatile bool	depop_zone;
	volatile bool	repop_zone;
	volatile bool	respawns_allowed;
	volatile bool	LoadingData;
	std::atomic<bool> reloading_spellprocess;
	std::atomic<bool> zoneShuttingDown;
	std::atomic<bool> is_initialized;
	bool	cityzone;
	bool	always_loaded;
	bool	duplicated_zone;
	int32	duplicated_id;
	bool	isInstance;	
	
	std::atomic<int32> pNumPlayers;
	sint16	minimumStatus;
	int16	minimumLevel;
	int16	maximumLevel;
	int16	minimumVersion;
	char	zone_name[64];
	char	zonesky_file[64];
	char	zone_file[64];
	char	zone_description[255];
	float	underworld;
	float	safe_x;
	float	safe_y;
	float	safe_z;
	float	safe_heading;
	float	xp_mod;
	volatile int32	zoneID;
	bool	locked;	// JA: implementing /zone lock|unlock commands
	int32	instanceID;
	string	zone_motd;
	int32	def_reenter_time;
	int32	def_reset_time;
	int32	def_lockout_time;
	int8	group_zone_option;
	float	rain;
	bool	isDusk;
	int		dusk_hour;
	int		dawn_hour;
	int		dusk_minute;
	int		dawn_minute;
	int32	spawn_delete_timer;
	int32	expansion_flag;
	int32	holiday_flag;
	//devn00b:test
	int		can_bind;
	bool	can_gate;
	bool	can_evac;

	/* Weather Stuff */
	bool	weather_enabled;			// false = disabled, true = enabled
	int8	weather_type;				// 0 = normal, 1 = dynamic, 2 = random, 3 = chaotic
	int32	weather_frequency;			// how often weather changes
	float	weather_min_severity;		// minimum weather severity in a zone
	float	weather_max_severity;		// maximum weather severity in a zone
	float	weather_change_amount;		// how much does the weather change each interval (normal weather conditions)
	float	weather_dynamic_offset;		// max amount the weather change each interval (dynamic weather conditions)
	int8	weather_change_chance;		// percentage chance the weather will change
	int8	weather_pattern;			// 0 = decreasing severity, 1 = increasing severity, 2 = random severity
	int32	weather_last_changed_time;	// last time weather changed (used with weather_frequency)
	float	weather_current_severity;	// current weather conditions in a zone
	bool	weather_allowed;			// from zones.weather_allowed field in database
	bool	weather_signaled;			// whether or not we told the client "it begins to rain"








	bool reloading;
	map<int32, vector<EntityCommand*>* > entity_command_list;
	map<int32, map<int32, int16> > npc_skill_list;
	map<int32, vector<int32> > npc_equipment_list;
	map<int32, NPC*> npc_list;
	map<int32,Object*> object_list;
	map<int32,Sign*> sign_list;
	map<int32,Widget*> widget_list;
	map<int32, vector<GroundSpawnEntry*> > groundspawn_entries;
	map<int32, vector<GroundSpawnEntryItem*> > groundspawn_items;
	Mutex MGroundSpawnItems;
	map<int32,GroundSpawn*> groundspawn_list;
	map<int32,LootTable*> loot_tables;
	map<int32, vector<LootDrop*> > loot_drops;
	map<int32, vector<int32> > spawn_loot_list;
	vector<GlobalLoot*> level_loot_list;
	map<int16, vector<GlobalLoot*> > racial_loot_list;
	map<int32, vector<GlobalLoot*> > zone_loot_list;
	map<int32, vector<TransportDestination*> > transporters;
	map<int32, MutexList<LocationTransportDestination*>* > location_transporters;
	Mutex MTransporters;
	Mutex MTransportMaps;
	// Map <transport if, map name>
	map<int32, string> m_transportMaps;
	
	int32 watchdogTimestamp;

	std::map<int32, int32> lua_queued_state_commands;
	std::map<int32, std::map<std::string, float>> lua_spawn_update_command;
	std::mutex MLuaQueueStateCmd;
	
	mutable std::shared_mutex MIgnoredWidgets;
	std::map<int32, bool> ignored_widgets;
	Map* default_zone_map; // this is the map that npcs, ground spawns, so on use.  May not be the same as the clients!
	
	int32 groupraidMinLevel;
	int32 groupraidMaxLevel;
	int32 groupraidAvgLevel;
	int32 groupraidFirstLevel;
public:
	Spawn*				GetSpawn(int32 id);

	/* Entity Commands */
	map<int32, vector<EntityCommand*>*>* GetEntityCommandListAll() {return &entity_command_list;}
	vector<EntityCommand*>*	GetEntityCommandList(int32 id);
	void				SetEntityCommandList(int32 id, EntityCommand* command);
	void				ClearEntityCommands();
	EntityCommand* GetEntityCommand(int32 id, string name);

	/* NPC's */
	void				AddNPC(int32 id, NPC* npc);
	NPC*				GetNPC(int32 id, bool override_loading = false) {
		if((!reloading || override_loading) && npc_list.count(id) > 0)
			return npc_list[id]; 
		else
			return 0;
	}
	NPC*				GetNewNPC(int32 id) { 
		if(!reloading && npc_list.count(id) > 0)
			return new NPC(npc_list[id]);
		else
			return 0;
	}

	/* NPC Skills */
	void AddNPCSkill(int32 list_id, int32 skill_id, int16 value);
	map<string, Skill*>* GetNPCSkills(int32 primary_list, int32 secondary_list);

	/* NPC Equipment */
	void AddNPCEquipment(int32 list_id, int32 item_id);
	void SetNPCEquipment(NPC* npc);

	/* Objects */
	void				AddObject(int32 id, Object* object){ object_list[id] = object; }
	Object*				GetObject(int32 id, bool override_loading = false) { 
		if((!reloading || override_loading) && object_list.count(id) > 0)
			return object_list[id]; 
		else
			return 0;
	}
	Object*				GetNewObject(int32 id) {
		if(!reloading && object_list.count(id) > 0)
			return object_list[id]->Copy(); 
		else
			return 0;
	}

	/* Signs */
	void				AddSign(int32 id, Sign* sign){ sign_list[id] = sign; }
	Sign*				GetSign(int32 id, bool override_loading = false) { 
		if((!reloading || override_loading) && sign_list.count(id) > 0)
			return sign_list[id]; 
		else
			return 0;
	}
	Sign*				GetNewSign(int32 id) {
		if(!reloading && sign_list.count(id) > 0)
			return sign_list[id]->Copy(); 
		else
			return 0;
	}

	/* Widgets */
	void				AddWidget(int32 id, Widget* widget);
	Widget*				GetWidget(int32 id, bool override_loading = false);
	Widget*				GetNewWidget(int32 id);

	/* Groundspawns */
	// JA: groundspawn revamp
	void AddGroundSpawnEntry(int32 groundspawn_id, int16 min_skill_level, int16 min_adventure_level, int8 bonus_table, float harvest1, float harvest3, float harvest5, float harvest_imbue, float harvest_rare, float harvest10, int32 harvest_coin);
	void AddGroundSpawnItem(int32 groundspawn_id, int32 item_id, int8 is_rare, int32 grid_id);
	vector<GroundSpawnEntry*>* GetGroundSpawnEntries(int32 id);
	vector<GroundSpawnEntryItem*>* GetGroundSpawnEntryItems(int32 id);
	void LoadGroundSpawnEntries();
	void LoadGroundSpawnItems();
	//
	void DeleteGroundSpawnItems();

	void				AddGroundSpawn(int32 id, GroundSpawn* spawn);
	GroundSpawn*				GetGroundSpawn(int32 id, bool override_loading = false);
	GroundSpawn*				GetNewGroundSpawn(int32 id);

	/* Pet names */
	vector<string> pet_names;

	/* Loot */
	void				AddLootTable(int32 id, LootTable* table);
	void				AddLootDrop(int32 id, LootDrop* drop);
	void				AddSpawnLootList(int32 spawn_id, int32 id);
	void				ClearSpawnLootList(int32 spawn_id);
	void				AddLevelLootList(GlobalLoot* loot);
	void				AddRacialLootList(int16 racial_id, GlobalLoot* loot);
	void				AddZoneLootList(int32 zone, GlobalLoot* loot);
	void				ClearLootTables();
	vector<int32>		GetSpawnLootList(int32 spawn_id, int32 zone_id, int8 spawn_level, int16 racial_id, Spawn* spawn = 0);
	vector<LootDrop*>*	GetLootDrops(int32 table_id);
	LootTable*			GetLootTable(int32 table_id);

	/* Transporters */
	void AddLocationTransporter(int32 zone_id, string message, float trigger_x, float trigger_y, float trigger_z, float trigger_radius, int32 destination_zone_id, float destination_x, float destination_y, float destination_z, float destination_heading, int32 cost, int32 unique_id, bool force_zone);
	void AddTransporter(int32 transport_id, int8 type, string name, string message, int32 destination_zone_id, float destination_x, float destination_y, float destination_z, float destination_heading, 
		int32 cost, int32 unique_id, int8 min_level, int8 max_level, int32 quest_req, int16 quest_step_req, int32 quest_complete, int32 map_x, int32 map_y, int32 expansion_flag, int32 holiday_flag, int32 min_client_version,
		int32 max_client_version, int32 flight_path_id, int16 mount_id, int8 mount_red_color, int8 mount_green_color, int8 mount_blue_color);
	void GetTransporters(vector<TransportDestination*>* returnList, Client* client, int32 transport_id);
	MutexList<LocationTransportDestination*>* GetLocationTransporters(int32 zone_id);
	void DeleteGlobalTransporters();
	///<summary></summary>
	///<param name='id'>The transport id</param>
	///<param name='name'>Name of the map</param>
	void AddTransportMap(int32 id, string name);

	///<summary>Checks to see if the transport has a map</summary>
	///<param name='id'>The transport id we want to check</param>
	///<returns>True if the transport id has a map</returns>
	bool TransportHasMap(int32 id);

	///<summary>Gets the map name for the given transport id</summary>
	///<param name='id'>The transport id that we want a map for</param>
	///<returns>Map name</returns>
	string GetTransportMap(int32 id);

	///<summary>Clears the list of transporter maps</summary>
	void DeleteTransporterMaps();

	
	void DeleteGlobalSpawns();

	void ReloadSpawns();

	void SendStateCommand(Spawn* spawn, int32 state);

	int32 getGroupraidMinLevel() const {
		return groupraidMinLevel;
	}

	int32 getGroupraidMaxLevel() const {
		return groupraidMaxLevel;
	}

	int32 getGroupraidAvgLevel() const {
		return groupraidAvgLevel;
	}

	int32 getGroupraidFirstLevel() const {
		return groupraidFirstLevel;
	}
	
	void setGroupRaidLevels(int32 min_level, int32 max_level, int32 avg_level, int32 first_level) {
		groupraidMinLevel = min_level;
		groupraidMaxLevel = max_level;
		groupraidAvgLevel = avg_level;
		groupraidFirstLevel = first_level;
	}

	int32 lifetime_client_count;
	int32 incoming_clients;
};

#endif
