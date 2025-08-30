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

#ifndef EQ2_WORLD_H
#define EQ2_WORLD_H

#include <string>
#include <vector>
#include <map>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <mysql.h>
#include "SpawnLists.h"
#include "zoneserver.h"
#include "NPC.h"
#include "Widget.h"
#include "Object.h"
#include "GroundSpawn.h"
#include "Sign.h"
#include "Variables.h"
#include "MutexList.h"

#include "PlayerGroups.h"
#include "../common/Web/WebServer.h"

#include "./Zone/region_map.h"
#include "./Zone/map.h"

#include "./Web/PeerManager.h"

using namespace std;
struct MerchantInfo{
	vector<int32> inventory_ids;
	/*int32	faction_id;
	sint32	faction_min;
	sint32	faction_max;
	float	low_buy_multiplier;
	float	high_buy_multiplier;
	float	low_sell_multiplier;
	float	high_sell_multiplier;*/
};

struct MerchantItemInfo{
	int32	item_id;
	int16	quantity;
	int32	price_item_id;
	int32	price_item2_id;
	int16	price_item_qty;
	int16	price_item2_qty;
	int32	price_status;
	int64	price_coins;
	int32	price_stationcash;
};

struct LootTable{
	string	name;
	int32	mincoin;
	int32	maxcoin;
	int16	maxlootitems;
	float	lootdrop_probability;
	float	coin_probability;
};

struct LootDrop{
	int32	item_id;
	int16	item_charges;
	bool	equip_item;
	float	probability;
	int32	no_drop_quest_completed_id;
};

struct GroundSpawnEntry {
	int16	min_skill_level;
	int16	min_adventure_level;
	int8	bonus_table;
	float	harvest1;
	float	harvest3;
	float	harvest5;
	float	harvest_imbue;
	float	harvest_rare;
	float	harvest10;
	int32	harvest_coin;
};

struct GroundSpawnEntryItem {
	int32	item_id;
	int8	is_rare;
	int32	grid_id;
};

struct TransportDestination{
	int32	unique_id;
	int8	type;
	string	display_name;
	string	message;
	int32	destination_zone_id;
	float	destination_x;
	float	destination_y;
	float	destination_z;
	float	destination_heading;
	int32	cost;
	int8	min_level;
	int8	max_level;
	int32	req_quest;
	int16	req_quest_step;
	int32	req_quest_complete;

	int32	map_x;
	int32	map_y;
	int32	faction_id;
	int32	faction_value;

	int32	expansion_flag;
	int32	holiday_flag;

	int32	min_client_version;
	int32	max_client_version;

	int32	flight_path_id;

	int16	mount_id;
	int8	mount_red_color;
	int8	mount_green_color;
	int8	mount_blue_color;
};

struct LocationTransportDestination{
	int32	unique_id;
	string	message;
	int32	destination_zone_id;
	float	destination_x;
	float	destination_y;
	float	destination_z;
	float	destination_heading;
	float	trigger_x;
	float	trigger_y;
	float	trigger_z;
	float	trigger_radius;
	int32	cost;
	int32	faction_id;
	int32	faction_value;
	bool	force_zone;
};

struct LottoPlayer {
	int32 end_time;
	int8 num_matches;
	bool set;
};

struct HouseZone {
	int32		id;
	string		name;
	int64		cost_coin;
	int32		cost_status;
	int64		upkeep_coin;
	int32		upkeep_status;
	int8		vault_slots;
	int8		alignment;
	int8		guild_level;
	int32		zone_id;
	int32		exit_zone_id;
	float		exit_x;
	float		exit_y;
	float		exit_z;
	float		exit_heading;
};

struct Deposit {
	int32 timestamp;
	int64 amount;
	int64 last_amount;
	int32 status;
	int32 last_status;
	string name;
};

struct HouseHistory {
	HouseHistory()
	{
		timestamp = 0;
		amount = 0;
		name = string("");
		reason = string("");
		status = 0;
		pos_flag = 0;
	}
	HouseHistory(int32 in_timestamp, int64 in_amount, string in_name, string in_reason, int32 in_status, int8 in_pos_flag)
	{
		timestamp = in_timestamp;
		amount = in_amount;
		name = in_name;
		reason = in_reason;
		status = in_status;
		pos_flag = in_pos_flag;
	}
	int32 timestamp;
	int64 amount;
	string name;
	string reason;
	int32 status;
	int8 pos_flag;
};

struct PlayerHouse {
	int32 house_id;
	int64 unique_id;
	int32 instance_id;
	int32 upkeep_due;
	int64 escrow_coins;
	int32 escrow_status;
	string player_name;
	int32 character_id;
	list<Deposit> deposits;
	map<string, Deposit> depositsMap;
	list<HouseHistory> history;
};

// Constants for STATs counters

// Server Utilization
#define STAT_SERVER_OS_TYPE						1		// what OS this server is running on
#define STAT_SERVER_CPU_TYPE					2		// cpu type/speed (ie., Intel P4 3.0GHz)
#define STAT_SERVER_CPU_CURRENT					3		// current CPU usage by EQ2World.exe process
#define STAT_SERVER_CPU_PEAK					4		// highest CPU usage by EQ2World.exe this session
#define STAT_SERVER_PHYSICAL_RAM_TOTAL			5		// total RAM in server
#define STAT_SERVER_PHYSICAL_RAM_CURRENT		6		// current RAM usage by EQ2World.exe
#define STAT_SERVER_PHYSICAL_RAM_PEAK			7		// highest RAM usage by EQ2World.exe this session
#define STAT_SERVER_VIRTUAL_RAM_TOTAL			8		// total vRAM in server
#define STAT_SERVER_VIRTUAL_RAM_CURRENT			9		// current vRAM usage by EQ2World.exe
#define STAT_SERVER_VIRTUAL_RAM_PEAK			10		// highest vRAM usage by EQ2World.exe this session
#define STAT_SERVER_DISK_USAGE					11		// size of /eq2emulator folder and contents
#define STAT_SERVER_THREAD_COUNT				12		// thread count of EQ2World.exe process
#define STAT_SERVER_AVG_LATENCY					13		// network latency between world and loginserver

// Server Stats
#define STAT_SERVER_CREATED						100		// unix_timestamp of date server first came online
#define STAT_SERVER_START_TIME					101		// unix_timestamp of date/time server booted up
#define STAT_SERVER_ACCEPTED_CONNECTION			102		// successful connections since server startup
#define STAT_SERVER_MOST_CONNECTIONS			103		// most players online in history of server
#define STAT_SERVER_NUM_ACCOUNTS				104		// total number of unique accounts
#define STAT_SERVER_NUM_CHARACTERS				105		// total number of player characters
#define STAT_SERVER_AVG_CHAR_LEVEL				106		// average level of player characters
#define STAT_SERVER_NUM_ACTIVE_ZONES			107		// number of currently running/loaded zones
#define STAT_SERVER_NUM_ACTIVE_INSTANCES		108		// number of active zones that are "instances"

// Player PvE counters
#define STAT_PLAYER_TOTAL_NPC_KILLS				1000	// total NPC kills by player
#define STAT_PLAYER_TOTAL_DEATHS				1001	// total non-PvP deaths of player
#define STAT_PLAYER_KVD_RATIO					1002	// kill-versus-death ratio of player
#define STAT_PLAYER_HIGHEST_MELEE_HIT			1003	// players highest melee hit to date
#define STAT_PLAYER_HIGHEST_MAGIC_HIT			1004	// players highest magic hit to date
#define STAT_PLAYER_HIGHEST_HO_HIT				1005	// players highest heroic opportunity hit
#define STAT_PLAYER_TOTAL_STATUS				1006	// player total status
#define STAT_PLAYER_TOTAL_WEALTH				1007	// player total wealth
#define STAT_PLAYER_QUESTS_COMPLETED			1008	// total quests completed
#define STAT_PLAYER_RECIPES_KNOWN				1009	// total recipes player knows
#define STAT_PLAYER_TOTAL_CRAFTED_ITEMS			1010	// total items crafted by player
#define STAT_PLAYER_ITEMS_DISCOVERED			1011	// total items discovered by player
#define STAT_PLAYER_RARES_HARVESTED				1012	// total rare harvests by player
#define STAT_PLAYER_ITEMS_HARVESTED				1013	// total rare harvests by player
#define STAT_PLAYER_MASTER_ABILITIES			1014	// total master abilities player has
#define STAT_PLAYER_HIGHEST_FALLING_HIT			1015	// player's highest damage amount taken from falling

// Player PvP counters
#define STAT_PLAYER_TOTAL_PVP_KILLS				1100	// total PVP kills by player
#define STAT_PLAYER_PVP_KILL_STREAK				1101	// longest PVP kill streak of player
#define STAT_PLAYER_TOTAL_PVP_DEATHS			1102	// total PVP deaths of player
#define STAT_PLAYER_PVP_DEATH_STREAK			1103	// longest PVP death streak of player
#define STAT_PLAYER_PVP_KVD_RATIO				1104	// PVP kill-versus-death ratio of player
#define STAT_PLAYER_TOTAL_ARENA_KILLS			1105	// total Arena kills of player

// MOST stats for players
#define STAT_PLAYER_MOST_NPC_KILLS				1200	// IPvP: Player with most NPC kills
#define STAT_PLAYER_MOST_NPC_DEATHS				1201	// IPvP: Player with most non-PVP deaths
#define STAT_PLAYER_MOST_PVP_KILLS				1202	// IPvP: Player with most PvP kills
#define STAT_PLAYER_MOST_PVP_DEATHS				1203	// IPvP: Player with most PvP deaths
#define STAT_PLAYER_MOST_ARENA_KILLS			1204	// IPvP: Player with most Arena kills
#define STAT_PLAYER_MOST_STATUS					1205	// IPvP: Player with most Status
#define STAT_PLAYER_MOST_WEALTH					1206	// IPvP: Player with most Wealth

// HIGHEST stats for players
#define STAT_PLAYER_HIGHEST_NPC_KVD_RATIO		1300	// IPvP: Player with highest NPC kill-versus-death ratio
#define STAT_PLAYER_HIGHEST_PVP_KILL_STREAK		1301	// IPvP: Player with longest PvP kill streak
#define STAT_PLAYER_HIGHEST_PVP_DEATH_STREAK	1302	// IPvP: Player with longest PvP death streak
#define STAT_PLAYER_HIGHEST_PVP_KVD_RATIO		1303	// IPvP: Player with highest PvP kill-versus-death ratio
#define STAT_PLAYER_HIGHEST_HP					1304	// IPvP: Player with highest HP on server
#define STAT_PLAYER_HIGHEST_POWER				1305	// IPvP: Player with highest Power on server
#define STAT_PLAYER_HIGHEST_RESISTS				1306	// IPvP: Player with highest Resists on server


struct Statistic {
	int32 stat_id;
	sint32 stat_value;
	int32 stat_date;
	bool  save_needed;
};


// Player EVENT defines
// Some EVENTs are single occurrance (S), while others are cummulative throughout the life of the player (C)
#define PLAYER_EVENT_NEW_ADV_LEVEL				2000	// (C) player achieves a new adventure level
#define PLAYER_EVENT_NEW_TS_LEVEL				2001	// (C) player achieves a new tradeskill level
#define PLAYER_EVENT_NEW_AA						2002	// (C) player earns AA pt
#define PLAYER_EVENT_NEW_ACHIEVEMENT			2003	// (C) player new achievement
#define PLAYER_EVENT_LAST_DEATH					2004	// (S) player was last killed
#define PLAYER_EVENT_LAST_KILL					2005	// (S) player last killed spawn_id
#define PLAYER_EVENT_DISCOVER_POI				2006	// (C) player discovers location_id

// These maybe should be World stat events, since it is about 1 player discovering a new item?
#define PLAYER_EVENT_DISCOVER_ITEM				2007	// (C) player discovers item_id
#define PLAYER_EVENT_DISCOVER_RECIPE			2008	// (C) player discovers recipe_id


struct PlayerHistory {
	int32	history_zone;
	int32	history_id;
	sint32	history_value;
	int32	history_date;
	bool	save_needed;
};

struct GlobalLoot {
	int8	minLevel;
	int8	maxLevel;
	int32	table_id;
	int32	loot_tier;
};

#define TRANSPORT_TYPE_LOCATION		0
#define TRANSPORT_TYPE_ZONE			1
#define TRANSPORT_TYPE_GENERIC		2
#define TRANSPORT_TYPE_FLIGHT		3
#define TRANSPORT_TYPE_FORCEZONE	4


// structs MUST start with class_id and race_id, in that order as int8's
struct StartingStructHeader
{
	int8 class_id;
	int8 race_id;
};

struct StartingSkill
{
	StartingStructHeader header;
	int32 skill_id;
	int16 current_val;
	int16 max_val;
	int32 progress; // what is this for..?
};

struct StartingSpell
{
	StartingStructHeader header;
	int32 spell_id;
	int8 tier;
	int32 knowledge_slot;
};

#define MAX_VOICEOVER_TYPE 2
struct VoiceOverStruct{
	string				mp3_string;
	string				text_string;
	string				emote_string;
	int32				key1;
	int32				key2;
	bool				is_garbled;
	int8				garble_link_id;
};

struct WhoAllPeerPlayer {
	std::string name;
	std::string subtitle;
	std::string zoneName;
	int8 adventureClass;
	int8 tradeskillClass;
	int8 deity;
	int8 race;
	sint16 status;
	int flags;
	int flags2;
	int16 level;
	
	WhoAllPeerPlayer(std::string inName, std::string inSubtitle, std::string inZoneName, int8 inAdvClass, int8 inTradeskillClass, int8 inDeity, int8 inRace, 
					 sint16 inStatus, int inFlags, int inFlags2, int16 inLevel)
					 {
						 name = inName;
						 subtitle = inSubtitle;
						 zoneName = inZoneName;
						 adventureClass = inAdvClass;
						 tradeskillClass = inTradeskillClass;
						 deity = inDeity;
						 race = inRace;
						 status = inStatus;
						 flags = inFlags;
						 flags2 = inFlags2;
						 level = inLevel;
					 }
};

class ZoneInfoMemory {
public:
	int32 zoneID;
	std::string zoneName;
	std::string zoneFile;
	std::string zoneDescription;
	std::string zoneMotd;
	std::string zoneSkyFile;
	std::string zoneLuaScript;
	float underworld;
	float safeX, safeY, safeZ, safeHeading;
	int16 minimumLevel, maximumLevel, minimumVersion;
	int32 defReenterTime, defResetTime, defLockoutTime;
	int8 groupZoneOption;
	float xpModifier;
	bool cityZone, canBind, canGate, canEvac, weatherAllowed;
	int32 rulesetID;
	Instance_Type instanceType;
	int32 shutdownTime;
	// Add other fields as necessary

	// Constructor
	ZoneInfoMemory() : zoneID(0), underworld(0.0f), safeX(0.0f), safeY(0.0f), safeZ(0.0f),
		safeHeading(0.0f), minimumLevel(0), maximumLevel(0), minimumVersion(0),
		defReenterTime(0), defResetTime(0), defLockoutTime(0), groupZoneOption(0),
		xpModifier(1.0f), cityZone(false), canBind(false), canGate(false),
		canEvac(false), weatherAllowed(true), rulesetID(0) {}

	void LoadFromDatabaseRow(MYSQL_ROW row);
};

class ZoneList {
	public:
	ZoneList();
	~ZoneList();
	
	void Add(ZoneServer* zone);
	void Remove(ZoneServer* zone);
	bool GetZone(ZoneChangeDetails* zone_details, int32 opt_zone_id, std::string opt_zone_name = "", bool loadZone = true, bool skip_existing_zones = false, bool increment_zone = true, bool check_peers = true, bool check_instances = false, bool only_always_loaded = false, bool skip_self = false, bool duplicated_zone = false, int32 minLevel = 0, int32 maxLevel = 0, int32 avgLevel = 0, int32 firstLevel = 0);
	bool GetZoneByInstance(ZoneChangeDetails* zone_details, int32 instance_id, int32 zone_id = 0, bool loadZone = true, bool skip_existing_zones = false, bool increment_zone = true, bool check_peers = true, int32 minLevel = 0, int32 maxLevel = 0, int32 avgLevel = 0, int32 firstLevel = 0);
	
	bool IsClientConnectedPeer(int32 account_id, int32 char_id);
	//ZoneServer*	Get(int32 id, bool loadZone = true, bool skip_existing_zones = false, bool increment_zone = true);
	//ZoneServer* Get(const char* zone_name, bool loadZone=true, bool skip_existing_zones = false, bool increment_zone = true);
	//ZoneServer* GetByInstanceID(int32 id, int32 zone_id=0, bool skip_existing_zones = false, bool increment_zone = true);

	/// <summary>Get the instance for the given zone id with the lowest population</summary>
	/// <param name='zone_id'>The id of the zone to look up</param>
	/// <returns>ZoneServer* of an active zone with the given id</returns>
	ZoneServer* GetByLowestPopulation(int32 zone_id);
	
	int32 GetZonesPlayersCount();

	void AddClientToMap(string name, Client* client){
		name = ToLower(name);
		MClientList.lock();
		client_map[name] = client;
		MClientList.unlock();
	}
	void CheckFriendList(Client* client);
	void CheckFriendZoned(Client* client);

	// move to Chat/Chat.h?
	void TransmitBroadcast(const char* message);
	void TransmitGlobalAnnouncement(const char* message);
	
	// these are handled internally by the transmit and peering
	bool HandleGlobalChatMessage(Client* from, char* to, int16 channel, const char* message, const char* channel_name = 0, int32 current_language_id = 0);
	void HandleGlobalBroadcast(const char* message);
	void HandleGlobalAnnouncement(const char* message);
	//

	int32 Count();
	Client*	GetClientByCharName(string name){ 
		name = ToLower(name);
		Client* ret = 0;
		MClientList.lock();
		if(client_map.count(name) > 0)
			ret = client_map[name]; 
		MClientList.unlock();
		return ret;
	}
	Client* GetClientByCharID(int32 id) {
		Client* ret = 0;
		MClientList.lock();
		map<string, Client*>::iterator itr;
		for (itr = client_map.begin(); itr != client_map.end(); itr++) {
			if (itr->second->GetCharacterID() == id) {
				ret = itr->second;
				break;
			}
		}
		MClientList.unlock();
		return ret;
	}
	Client* GetClientByEQStream(EQStream* eqs) {
		Client* ret = 0;
		if (eqs) {
			MClientList.lock();
			map<string, Client*>::iterator itr;
			for (itr = client_map.begin(); itr != client_map.end(); itr++) {
				if (itr->second->getConnection() == eqs) {
					ret = itr->second;
					break;
				}
			}
			MClientList.unlock();
		}
		return ret;
	}
	void UpdateVitality(float amount);
	void RemoveClientFromMap(string name, Client* client){
		name = ToLower(name);
		MClientList.lock();
		if(client_map.count(name) > 0 && client_map[name] == client)
			client_map.erase(name);
		MClientList.unlock();
	}
	bool ClientConnected(int32 account_id, int32 char_id);
	void RemoveClientZoneReference(ZoneServer* zone);
	void ReloadClientQuests();
	bool DepopFinished();
	void Depop();
	void Repop();	
	void DeleteSpellProcess();
	void SendZoneWideChannelMessage(std::string fromName, const char* to, int16 channel, const char* message, float distance, const char* channel_name, int32 language);
	void LoadSpellProcess();
	void ProcessWhoQuery(const char* query, Client* client);
	void ProcessWhoQuery(vector<string>* queries, ZoneServer* zone, vector<Entity*>* players, bool isGM);
	void SendZoneList(Client* client);
	void WritePlayerStatistics();
	void ShutDownZones();
	void ReloadMail();
	void ReloadSpawns();
	
	int32 GetHighestDuplicateID(const std::string& inc_zone_name, int32 inc_zone_id, bool increment_new_value = true);
	bool GetDuplicateZoneDetails(ZoneChangeDetails* zone_details, const std::string& inc_zone_name, int32 inc_zone_id, int32 matchDuplicateId);
	
	void WatchdogHeartbeat();

	void SendTimeUpdate();
	
	void	PopulateClientList(boost::property_tree::ptree& pt);
	void	PopulateZoneList(boost::property_tree::ptree& pt);
private:
	Mutex				MClientList;
	Mutex				MZoneList;
	map<ZoneServer*, int32> removed_zoneservers;
	map<string,Client*> client_map;
	list<ZoneServer*> zlist;
};
class World {
public:
	World();
	~World();
	int8				GetClassID(const char* name);
	void				Process();
	void				init(std::string web_ipaddr, int16 web_port, std::string cert_file, std::string key_file, std::string key_password, std::string hardcode_user, std::string hardcode_password);
	PacketStruct*		GetWorldTime(int16 version);
	void				WorldTimeTick();
	float				GetXPRate();
	float				GetTSXPRate();
	void				LoadVitalityInformation();
	void				UpdateVitality();
	WorldTime*			GetWorldTimeStruct(){
		return &world_time;
	}
	ulong GetCurrentThreadID();
	int64 GetThreadUsageCPUTime();
	
	// These 2 functions are never used.  What was their purpose? Should they be removed?
	void				AddNPCAppearance(int32 id, AppearanceData* appearance){ npc_appearance_list[id] = appearance; }
	AppearanceData*		GetNPCAppearance(int32 id) { return npc_appearance_list[id]; }
	
	void ReloadGuilds();
	bool ReportBug(string data, char* player_name, int32 account_id, const char* spawn_name, int32 spawn_id, int32 zone_id);
	void AddSpawnScript(int32 id, const char* name);
	void AddSpawnEntryScript(int32 id, const char* name);
	void AddSpawnLocationScript(int32 id, const char* name);
	void AddZoneScript(int32 id, const char* name);
	void LoadPlayerScripts();
	void AddPlayerScript(int32 zone_id, const char* zone_name);
	const char* GetSpawnScript(int32 id);
	const char* GetSpawnEntryScript(int32 id);
	const char* GetSpawnLocationScript(int32 id);
	const char* GetZoneScript(int32 id);
	const char* GetPlayerScript(int32 zone_id);
	void ResetSpawnScripts();
	void ResetZoneScripts();
	void ResetPlayerScripts();
	int16 GetMerchantItemQuantity(int32 merchant_id, int32 item_id);
	void DecreaseMerchantQuantity(int32 merchant_id, int32 item_id, int16 amount);
	int32 GetInventoryID(int32 merchant_id, int32 item_id);
	void AddMerchantItem(int32 inventory_id, MerchantItemInfo ItemInfo);
	void RemoveMerchantItem(int32 inventory_id, int32 item_id);
	vector<MerchantItemInfo>* GetMerchantList(int32 merchant_id);
	vector<MerchantItemInfo>* GetMerchantItemList(int32 merchant_id, int8 merchant_type, Player* player);
	MerchantInfo* GetMerchantInfo(int32 merchant_id);
	map<int32, MerchantInfo*>* GetMerchantInfo();
	void AddMerchantInfo(int32 merchant_id, MerchantInfo* multiplier);
	void DeleteMerchantsInfo();
	void DeleteMerchantItems();
	void DeleteSpawns();
	vector<Variable*>* GetClientVariables();
	void WritePlayerStatistics();
	void WriteServerStatistics();
	void AddServerStatistic(int32 stat_id, sint32 stat_value, int32 stat_date);
	void UpdateServerStatistic(int32 stat_id, sint32 stat_value, bool overwrite = false);
	sint32 GetServerStatisticValue(int32 stat_id);
	void RemoveServerStatistics();
	
	bool RejoinGroup(Client* client, int32 group_id);
	
	void AddBonuses(Item* item, ItemStatsValues* values, int16 type, float value, Entity* entity);
	int32 CreateGuild(const char* guild_name, Client* leader = 0, int32 group_id = 0);
	void SaveGuilds();
	void PickRandomLottoDigits(int32* digits);
	void AddLottoPlayer(int32 character_id, int32 end_time);
	void RemoveLottoPlayer(int32 character_id);
	void SetLottoPlayerNumMatches(int32 character_id, int8 num_matches);
	void CheckLottoPlayers();
	void PopulateTOVStatMap();
	int32 LoadItemBlueStats();
	sint16 GetItemStatAOMValue(sint16 subtype);
	sint16 GetItemStatTOVValue(sint16 subtype);
	sint16 GetItemStatDOVValue(sint16 subtype);
	sint16 GetItemStatCOEValue(sint16 subtype);
	sint16 GetItemStatKAValue(sint16 subtype);
	sint16 GetItemStatTESTValue(sint16 subtype);
	int8 TranslateSlotSubTypeToClient(Client* client, int8 stat_type, sint16 sub_type);

	vector<string> biography;

	volatile bool items_loaded;
	volatile bool spells_loaded;
	volatile bool achievments_loaded;
	
	std::atomic<bool> world_loaded;
	std::atomic<int64> world_uptime;

	void AddHouseZone(int32 id, string name, int64 cost_coins, int32 cost_status, int64 upkeep_coins, int32 upkeep_status, int8 vault_slots, int8 alignment, int8 guild_level, int32 zone_id, int32 exit_zone_id, float exit_x, float exit_y, float exit_z, float exit_heading);
	HouseZone* GetHouseZone(int32 id);

	void AddPlayerHouse(int32 char_id, int32 house_id, int64 unique_id, int32 instance_id, int32 upkeep_due, int64 escrow_coins, int32 escrow_status, string player_name);
	PlayerHouse* GetPlayerHouseByHouseID(int32 char_id, int32 house_id);
	PlayerHouse* GetPlayerHouseByUniqueID(int64 unique_id);
	PlayerHouse* GetPlayerHouseByInstanceID(int32 instance_id);
	vector<PlayerHouse*> GetAllPlayerHouses(int32 char_id);
	vector<PlayerHouse*> GetAllPlayerHousesByHouseID(int32 house_id);
	PlayerHouse* GetPlayerHouse(Client* client, int32 spawn_id, int64 unique_house_id, HouseZone** set_house_zone);
	
	void ReloadHouseData(PlayerHouse* ph);

	PlayerGroupManager* GetGroupManager() { return &m_playerGroupManager; }

	bool CheckTempBugCRC(char* msg);

	void SyncCharacterAbilities(Client* client);

	void LoadStartingLists();
	void PurgeStartingLists();
	multimap<int8, multimap<int8, StartingSkill>*> starting_skills;
	multimap<int8, multimap<int8, StartingSpell>*> starting_spells;
	Mutex MStartingLists;
	void SetReloadingSubsystem(string subsystem);
	void RemoveReloadingSubSystem(string subsystem);

	bool IsReloadingSubsystems();
	int32 GetSuppressedWarningTime() {
		return suppressed_warning;
	}
	void SetSuppressedWarning() { suppressed_warning = Timer::GetCurrentTime2(); }
	map<string, int32> GetOldestReloadingSubsystem();

	void LoadRegionMaps(std::string zoneFile);
	RegionMap* GetRegionMap(std::string zoneFile, int32 client_version);
	
	void LoadMaps(std::string zoneFile);
	void RemoveMaps(std::string zoneFile);
	Map* GetMap(std::string zoneFile, int32 client_version);
	
	void SendTimeUpdate();
	// just in case we roll over a time as to not send bad times to clients (days before hours, hours before minutes as examples)
	Mutex MWorldTime;

	void LoadVoiceOvers();
	void PurgeVoiceOvers();
	typedef std::multimap<int16, VoiceOverStruct>::iterator VOMapIterator;
	bool FindVoiceOver(int8 type, int32 id, int16 index, VoiceOverStruct* struct_ = nullptr, bool* find_garbled = nullptr, VoiceOverStruct* garble_struct_ = nullptr);
	void AddVoiceOver(int8 type, int32 id, int16 index, VoiceOverStruct* struct_);
	void CopyVoiceOver(VoiceOverStruct* struct1, VoiceOverStruct* struct2);
	
	/* NPC Spells */
	void AddNPCSpell(int32 list_id, int32 spell_id, int8 tier, bool spawn_cast, bool aggro_cast, sint8 req_hp_ratio);
	vector<NPCSpell*>* GetNPCSpells(int32 primary_list, int32 secondary_list);
	
	void PurgeNPCSpells();
	
	void ClientAuthApproval(int32 success, std::string charName, int32 account_id, std::string zone_name, int32 zoning_id, int32 instance_id, bool first_login);
	
	
	static void Web_worldhandle_status(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_clients(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_setadminstatus(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_reloadrules(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_reloadcommand(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_addpeer(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_zones(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_addcharauth(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_startzone(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_sendglobalmessage(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_newgroup(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_addgroupmember(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_removegroupmember(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_disbandgroup(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_createguild(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_addguildmember(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_removeguildmember(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_setguildpermission(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_setguildeventfilter(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_peerstatus(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_activequery(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_addseller(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_removeseller(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_additemsale(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_removeitemsale(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_addplayerhouse(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_updatehousedeposit(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	static void Web_worldhandle_addchatchannel(const http::request<http::string_body>& req, http::response<http::string_body>& res);
	
	static void Web_populate_status(boost::property_tree::ptree& pt);
	
	void ClearZoneInfoCache();
	std::shared_ptr<ZoneInfoMemory> GetZoneInfoById(int32 zoneID);
	std::shared_ptr<ZoneInfoMemory> GetZoneInfoByName(const std::string& zoneName);
	void AddZoneInfo(int32 zoneID, std::shared_ptr<ZoneInfoMemory> zoneInfo);
	
	Mutex MVoiceOvers;
	
	static sint64 newValue;
	
private:
	multimap<int32, multimap<int16, VoiceOverStruct>*> voiceover_map[3];
	int32 suppressed_warning = 0;
	map<string, int32> reloading_subsystems;
	//void RemovePlayerFromGroup(PlayerGroup* group, GroupMemberInfo* info, bool erase = true);
	//void DeleteGroupMember(GroupMemberInfo* info);
	Mutex MReloadingSubsystems;
	Mutex MMerchantList;
	Mutex MSpawnScripts;
	Mutex MZoneScripts;
	Mutex MPlayerScripts;
	//Mutex MGroups;
	
	mutable std::shared_mutex MNPCSpells;
	
	map<int32, MerchantInfo*> merchant_info;
	map<int32, vector<MerchantItemInfo> > merchant_inventory_items;
	int32 vitality_frequency;
	float vitality_amount;
	float xp_rate;
	float ts_xp_rate; // JA
	WorldTime world_time;
	
	map<int32,AppearanceData*> npc_appearance_list;
	
	map<int32, string> spawn_scripts;
	map<int32, string> spawnentry_scripts;
	map<int32, string> spawnlocation_scripts;
	map<int32, string> zone_scripts;
	map<int32, string> player_scripts;
	//vector<PlayerGroup*> player_groups;
	//map<GroupMemberInfo*, int32> group_removal_pending;
	//map<string, string> pending_groups;
	map<int32, Statistic*> server_statistics;
	MutexMap<int32, LottoPlayer*> lotto_players;
	int32 last_checked_time;
	Timer save_time_timer;
	Timer time_tick_timer;
	Timer vitality_timer;
	Timer player_stats_timer;
	Timer server_stats_timer;
	//Timer remove_grouped_player;
	Timer guilds_timer;
	Timer lotto_players_timer;
	Timer group_buff_updates;

	Timer watchdog_timer;

	map<int32, HouseZone*> m_houseZones;
	// Map <house id, map<char id, player house>>
	map<int32, map<int32, PlayerHouse*> > m_playerHouses;
	Mutex MHouseZones;
	Mutex MPlayerHouses;

	map<int16,int16> tov_itemstat_conversion;
	map<int16,int16> dov_itemstat_conversion;
	map<int16,int16> coe_itemstat_conversion;
	map<int16,int16> ka_itemstat_conversion;

	PlayerGroupManager m_playerGroupManager;

	Mutex				MBugReport;
	map<sint32, bool> bug_report_crc;

	std::map<std::string, RegionMapRange*> region_maps;
	std::map<std::string, MapRange*> maps;
	Mutex				MWorldMaps;
	Mutex				MWorldRegionMaps;
	
	map<int32, map<int32, NPCSpell*> > npc_spell_list;
	
	WebServer*			world_webserver;
	
	std::unordered_map<int32, std::shared_ptr<ZoneInfoMemory>> zoneInfoByID;
	std::unordered_map<std::string, std::shared_ptr<ZoneInfoMemory>> zoneInfoByName;
	mutable std::shared_mutex cacheMutex;
};
#endif
