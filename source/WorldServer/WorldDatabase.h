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
#ifndef EQ2WORLD_EMU_DATABASE_H
#define EQ2WORLD_EMU_DATABASE_H

#ifdef WIN32
	#include <WinSock2.h>
	#include <windows.h>
#endif
#include <mysql.h>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>

#include "../common/database.h"
#include "../common/types.h"
#include "../common/MiscFunctions.h"
#include "../common/Mutex.h"
#include "../common/DatabaseNew.h"
#include "client.h"
#include "Object.h"
#include "Widget.h"
#include "Sign.h"
#include "NPC.h"
#include "zoneserver.h"
#include "Collections/Collections.h"
#include "Achievements/Achievements.h"
#include "Recipes/Recipe.h"
#include "../common/PacketStruct.h"
#include "Spells.h"
#include "Titles.h"
#include "Rules/Rules.h"
#include "Languages.h"
#include "World.h"

using namespace std;

#define APPEARANCE_SOGA_HFHC	0
#define APPEARANCE_SOGA_HTHC	1
#define APPEARANCE_SOGA_HFC		2
#define APPEARANCE_SOGA_HTC		3
#define APPEARANCE_SOGA_HH		4
#define APPEARANCE_SOGA_HC1		5
#define APPEARANCE_SOGA_HC2		6
#define APPEARANCE_SOGA_SC		7
#define APPEARANCE_SOGA_EC		8
#define APPEARANCE_HTHC			9
#define APPEARANCE_HFHC			10
#define APPEARANCE_HTC			11
#define APPEARANCE_HFC			12
#define APPEARANCE_HH			13
#define APPEARANCE_HC1			14
#define APPEARANCE_HC2			15
#define APPEARANCE_WC1			16
#define APPEARANCE_WC2			17
#define APPEARANCE_SC			18
#define APPEARANCE_EC			19
#define APPEARANCE_SHIRT		20
#define APPEARANCE_UCC			21
#define APPEARANCE_PANTS		22
#define APPEARANCE_ULC			23
#define APPEARANCE_U9			24
#define APPEARANCE_BODY_SIZE	25
#define APPEARANCE_SOGA_WC1		26
#define APPEARANCE_SOGA_WC2		27
#define APPEARANCE_SOGA_SHIRT	28
#define APPEARANCE_SOGA_UCC		29
#define APPEARANCE_SOGA_PANTS	30
#define APPEARANCE_SOGA_ULC		31
#define APPEARANCE_SOGA_U13		32
#define APPEARANCE_SOGA_EBT		33
#define APPEARANCE_SOGA_CHEEKT	34
#define APPEARANCE_SOGA_NT		35
#define APPEARANCE_SOGA_CHINT	36
#define APPEARANCE_SOGA_LT		37
#define APPEARANCE_SOGA_EART	38
#define APPEARANCE_SOGA_EYET	39
#define APPEARANCE_EBT			40
#define APPEARANCE_CHEEKT		41
#define APPEARANCE_NT			42
#define APPEARANCE_CHINT		43
#define APPEARANCE_EART			44
#define APPEARANCE_EYET			45
#define APPEARANCE_LT			46
#define APPEARANCE_BODY_AGE		47
#define APPEARANCE_MC			48
#define APPEARANCE_SMC			49
#define APPEARANCE_SBS			50
#define APPEARANCE_SBA			51

#define CHAR_PROPERTY_SPEED					"modify_speed"
#define CHAR_PROPERTY_FLYMODE				"modify_flymode"
#define CHAR_PROPERTY_INVUL					"modify_invul"
#define CHAR_PROPERTY_REGIONDEBUG			"modify_regiondebug"
#define CHAR_PROPERTY_GMVISION				"modify_gmvision"
#define CHAR_PROPERTY_LUADEBUG				"modify_luadebug"

#define CHAR_PROPERTY_GROUPLOOTMETHOD	 	"group_loot_method"
#define CHAR_PROPERTY_GROUPLOOTITEMRARITY 	"group_loot_item_rarity"
#define CHAR_PROPERTY_GROUPAUTOSPLIT 		"group_auto_split"
#define CHAR_PROPERTY_GROUPDEFAULTYELL 		"group_default_yell"
#define CHAR_PROPERTY_GROUPAUTOLOCK 		"group_autolock"
#define CHAR_PROPERTY_GROUPLOCKMETHOD 		"group_lock_method"
#define CHAR_PROPERTY_GROUPSOLOAUTOLOCK 	"group_solo_autolock"
#define CHAR_PROPERTY_AUTOLOOTMETHOD	 	"group_auto_loot_method"

#define CHAR_PROPERTY_ASSISTAUTOATTACK	 	"assist_auto_attack"

#define CHAR_PROPERTY_SETACTIVEFOOD 		"set_active_food"
#define CHAR_PROPERTY_SETACTIVEDRINK 		"set_active_drink"

#define DB_TYPE_SPELLEFFECTS		1
#define DB_TYPE_MAINTAINEDEFFECTS	2

struct StartingItem{
	string	type;
	int32	item_id;
	string	creator;
	int8	condition;
	int8	attuned;
	int16	count;
};

struct ClaimItems {
	int32 char_id;
	int32 item_id;
	int8  max_claim;
	int8  curr_claim;
	int8  one_per_char;
	int32 vet_reward_time;
};

class Bot;

class WorldDatabase : public Database {
public:
	WorldDatabase();
	~WorldDatabase();

	bool ConnectNewDatabase();

	void PingNewDB();

	string	GetZoneName(int32 id);
	string	GetZoneDescription(int32 id);
	int32	LoadCharacterSkills(int32 char_id, Player* player);
	void	DeleteCharacterSkill(int32 char_id, Skill* skill);
	void	DeleteCharacterSpell(int32 character_id, int32 spell_id);
	int32	LoadCharacterSpells(int32 char_id, Player* player);
	int32	LoadItemBlueStats();
	void	SaveQuickBar(int32 char_id, vector<QuickBarItem*>* quickbar_items);
	void	SavePlayerSpells(Client* client);
	int32	LoadSkills();
	void	LoadCommandList();
	map<int8, vector<MacroData*> >*	LoadCharacterMacros(int32 char_id);
	void	UpdateCharacterMacro(int32 char_id, int8 number, const char* name, int16 icon, vector<string>* updates);
	void	SaveWorldTime(WorldTime* time);

	bool	SaveSpawnInfo(Spawn* spawn);
	int32	GetNextSpawnIDInZone(int32 zone_id);
	bool	SaveSpawnEntry(Spawn* spawn, const char* spawn_location_name, int8 percent, float x_offset, float y_offset, float z_offset, bool save_zonespawn = true, bool create_spawnlocation = true);
	float	GetSpawnLocationPlacementOffsetX(int32 location_id);
	float	GetSpawnLocationPlacementOffsetY(int32 location_id);
	float	GetSpawnLocationPlacementOffsetZ(int32 location_id);
	int32	GetNextSpawnLocation();
	bool	CreateNewSpawnLocation(int32 id, const char* name);
	bool	RemoveSpawnFromSpawnLocation(Spawn* spawn);
	int32	GetSpawnLocationCount(int32 location, Spawn* spawn = 0);
	vector<string>* GetSpawnNameList(const char* in_name);
	void	LoadSubCommandList();
	void	LoadGlobalVariables();
	void	UpdateVitality(int32 timestamp, float amount);
	void	SaveVariable(const char* name, const char* value, const char* comment);
	void	LoadVisualStates();
	void	LoadAppearanceMasterList();
	void	Save(Client* client);
	void	SaveItems(Client* client);
	void	SaveItem(int32 account_id, int32 char_id, Item* item, const char* type);
	void	DeleteBuyBack(int32 char_id, int32 item_id, int16 quantity, int32 price);
	void	LoadBuyBacks(Client* client);
	void	SaveBuyBacks(Client* client);
	void	SaveBuyBack(int32 char_id, int32 item_id, int16 quantity, int32 price);
	void	DeleteItem(int32 char_id, Item* item, const char* type);
	void	SaveCharacterColors(int32 char_id, const char* type, EQ2_Color color);
	void	SaveCharacterFloats(int32 char_id, const char* type, float float1, float float2, float float3, float multiplier = 100.0f);
	int16	GetAppearanceID(string name);
	vector<int16>* GetAppearanceIDsLikeName(string name, bool filtered = true);
	string	GetAppearanceName(int16 appearance_id);
	void	UpdateRandomize(int32 spawn_id, sint32 value);
	int32	SaveCharacter(PacketStruct* create, int32 loginID);
	int32	LoadNPCAppearanceEquipmentData(ZoneServer* zone);
	void	SaveNPCAppearanceEquipment(int32 spawn_id, int8 slot_id, int16 type, int8 red=0, int8 green=0, int8 blue=0, int8 hred=0, int8 hgreen=0, int8 hblue=0);
	void	LoadSpecialZones();
	void	SaveCharacterSkills(Client* client);
	void	SaveCharacterQuests(Client* client);
	void	SaveCharacterQuestProgress(Client* client, Quest* quest);
	void	DeleteCharacterQuest(int32 quest_id, int32 char_id, bool repeated_quest = false);
	void	LoadCharacterQuests(Client* client);
	void	LoadPlayerAA(Player *player);
	void	LoadCharacterQuestProgress(Client* client);
	void	LoadCharacterFriendsIgnoreList(Player* player);
	void	LoadZoneInfo(ZoneServer* zone, int32 minLevel=0, int32 maxLevel=0, int32 avgLevel=0, int32 firstLevel=0);
	void	LoadZonePlayerLevels(ZoneServer* zone);
	void	LoadZoneInfo(ZoneInfo* zone_info);
	int32	GetZoneID(const char* name);
	void	SaveZoneInfo(int32 zone_id, const char* field, sint32 value);
	void	SaveZoneInfo(int32 zone_id, const char* field, float value);
	void	SaveZoneInfo(int32 zone_id, const char* field, const char* value);
	bool	GetZoneRequirements(const char* zoneName,sint16* minStatus, int16* minLevel, int16* maxLevel, int16* minVersion);
	int16	GetMinimumClientVersion(int8 expansion_id);
	string	GetExpansionIDByVersion(int16 version);
	int32	CheckTableVersions(char* tablename);
	bool	RunDatabaseQueries(TableQuery* queries, bool output_result = true, bool data = false);
	void	UpdateTableVersion(char* name, int32 version);
	void	UpdateDataTableVersion(char* name, int32 version);
	void	UpdateStartingFactions(int32 char_id, int8 choice);
	string	GetStartingZoneName(int8 choice);
	void	UpdateStartingZone(int32 char_id, int8 class_id, int8 race_id, PacketStruct* create);
	void	UpdateStartingItems(int32 char_id, int8 class_id, int8 race_id, bool base_class = false);
	void	UpdateStartingSkills(int32 char_id, int8 class_id, int8 race_id);
	void	UpdateStartingSpells(int32 char_id, int8 class_id, int8 race_id);
	void	UpdateStartingSkillbar(int32 char_id, int8 class_id, int8 race_id);
	void	UpdateStartingTitles(int32 char_id, int8 class_id, int8 race_id, int8 gender_id);
	bool	UpdateSpawnLocationSpawns(Spawn* spawn);
	bool	UpdateSpawnWidget(int32 widget_id, char* query);
	bool	CheckVersionTable();
	void	LoadFactionAlliances();
	void	LoadFactionList();
	bool	LoadPlayerFactions(Client* client);
	void	SavePlayerFactions(Client* client);
	bool    VerifyFactionID(int32 char_id, int32 faction_id);
	void	LoadSpawnScriptData();
	void	LoadZoneScriptData();
	int32	LoadSpellScriptData();
	bool	UpdateSpawnScriptData(int32 spawn_id, int32 spawn_location_id, int32 spawnentry_id, const char* name);
	map<int32, string>*	GetZoneList(const char* name, bool is_admin = false);
	bool	VerifyZone(const char* name);
	int8	GetInstanceTypeByZoneID(int32 zoneID);
	/*void	loadNPCAppearance(int32 appearance_id);
	void	LoadNPCAppearances();*/
	void	ResetDatabase();
	void	EnableConstraints();
	void	DisableConstraints();
	int32	SaveCombinedSpawnLocation(ZoneServer* zone, Spawn* spawn, const char* name);
	int32	ProcessSpawnLocations(ZoneServer* zone, const char* sql_query, int8 type);
	int32	LoadSpawnLocationGroupAssociations(ZoneServer* zone);
	int32	LoadSpawnLocationGroups(ZoneServer* zone);
	int32	LoadSpawnGroupChances(ZoneServer* zone);
	bool	SpawnGroupAddAssociation(int32 group1, int32 group2);
	bool	SpawnGroupRemoveAssociation(int32 group1, int32 group2);
	bool	SpawnGroupAddSpawn(Spawn* spawn, int32 group_id);
	bool	SpawnGroupRemoveSpawn(Spawn* spawn, int32 group_id);
	int32	CreateSpawnGroup(Spawn* spawn, string name);
	void	DeleteSpawnGroup(int32 id);
	bool	SetGroupSpawnChance(int32 id, float chance);	
	void	LoadGroundSpawnEntries(ZoneServer* zone);
	void	LoadGroundSpawnItems(ZoneServer* zone);
	void	LoadSpawns(ZoneServer* zone);
	int8	GetAppearanceType(string type);
	void	LoadNPCs(ZoneServer* zone);
	void	LoadSpiritShards(ZoneServer* zone);
	int32	LoadAppearances(ZoneServer* zone, Client* client = 0);
	int32	LoadNPCSpells();
	int32	LoadNPCSkills(ZoneServer* zone);
	int32	LoadNPCEquipment(ZoneServer* zone);
	void	LoadObjects(ZoneServer* zone);
	void	LoadGroundSpawns(ZoneServer* zone);
	void	LoadWidgets(ZoneServer* zone);
	void	LoadSigns(ZoneServer* zone);
	void	ReloadItemList(int32 item_id = 0);
	void	LoadItemList(int32 item_id = 0);
	int32	LoadItemStats(int32 item_id = 0);
	int32	LoadItemModStrings(int32 item_id = 0);
	int32	LoadItemAppearances(int32 item_id = 0);
	int32	LoadItemLevelOverride(int32 item_id = 0);
	int32	LoadItemEffects(int32 item_id = 0);
	int32	LoadBookPages(int32 item_id = 0);
	int32	LoadNextUniqueItemID();
	int32	LoadSkillItems(int32 item_id = 0);
	int32	LoadRangeWeapons(int32 item_id = 0);
	int32	LoadThrownWeapons(int32 item_id = 0);
	int32	LoadBaubles(int32 item_id = 0);
	int32	LoadBooks(int32 item_id = 0);
	int32	LoadItemsets(int32 item_id = 0);
	int32	LoadHouseItem(int32 item_id = 0);
	int32	LoadRecipeBookItems(int32 item_id = 0);
	int32	LoadArmor(int32 item_id = 0);
	int32	LoadAdornments(int32 item_id = 0);
	int32	LoadClassifications();
	int32	LoadShields(int32 item_id = 0);
	int32	LoadBags(int32 item_id = 0);
	int32	LoadFoods(int32 item_id = 0);
	int32	LoadWeapons(int32 item_id = 0);
	int32	LoadRanged();
	int32   LoadHouseContainers(int32 item_id = 0);
	void	LoadBrokerItemStats();

	void	SaveSignMark(int32 char_id, int32 sign_id, char* char_name, Client* client);
	string	GetSignMark(int32 char_id, int32 sign_id, char* char_name); // returns the string containing the character name


	map<int32, vector<LevelArray*> >*	LoadSpellClasses();
	void	LoadTransporters(ZoneServer* zone);
	void	LoadTransportMaps(ZoneServer* zone);
	void	LoadDataFromRow(DatabaseResult *result, Item* item);
	void	LoadCharacterItemList(int32 account_id, int32 char_id, Player* player, int16);
	bool	loadCharacter(const char* name, int32 account_id, Client* client);
	std::string	loadCharacterFromLogin(ZoneChangeDetails* details, int32 char_id, int32 account_id);
	bool	LoadCharacterStats(int32 id, int32 account_id, Client* client);
	void	LoadCharacterQuestRewards(Client* client);
	void	LoadCharacterQuestTemporaryRewards(Client* client, int32 quest_id);
	bool	InsertCharacterStats(int32 character_id, int8 class_id, int8 race_id);
	bool	UpdateCharacterTimeStamp(int32 account_id, int32 character_id, int32 timestamp);
	bool	insertCharacterProperty(Client* client, char* propName, char* propValue);
	bool	loadCharacterProperties(Client* client);
	string	GetPlayerName(char* name);
	int32	GetCharacterTimeStamp(int32 character_id, int32 account_id,bool* char_exists);
	int32	GetCharacterTimeStamp(int32 character_id);
	sint32	GetLatestDataTableVersion(char* name);
	sint16	GetLowestCharacterAdminStatus(int32 account_id);
	sint16	GetHighestCharacterAdminStatus(int32 account_id);
	sint16	GetCharacterAdminStatus(char* character_name);
	sint16	GetCharacterAdminStatus(int32 account_id , int32 char_id);
	bool	UpdateAdminStatus(char* character_name, sint16 flag);
	void	LoadMerchantInformation();
	void	LoadMerchantInventory();
	string	GetMerchantDescription(int32 merchant_id);
	void	LoadPlayerStatistics(Player* player, int32 char_id);
	void	WritePlayerStatistic(Player* player, Statistic* stat);
	void	LoadServerStatistics();
	void	WriteServerStatistic(Statistic* stat);
	void	WriteServerStatistic(int32 stat_id, sint32 stat_value);
	void	WriteServerStatisticsNeededQueries();
	void	SavePlayerMail(Mail* mail);
	void	SavePlayerMail(Client* client);
	void	LoadPlayerMail(Client* client, bool new_only = false);
	void	DeletePlayerMail(Mail* mail);
	vector<int32>* GetAllPlayerIDs();
	void GetPetNames(ZoneServer* zone);
	//void	LoadMerchantMultipliers();

	char*	GetCharacterName(int32 character_id);
	int8	GetCharacterLevel(int32 character_id);
	int16	GetCharacterModelType(int32 character_id);
	int8	GetCharacterClass(int32 character_id);
	int8	GetCharacterGender(int32 character_id);
	int32	GetCharacterID(const char* name);
	int32	GetCharacterCurrentZoneID(int32 character_id);
	int32	GetCharacterAccountID(int32 character_id);
	void	LoadEntityCommands(ZoneServer* zone);
	void	LoadSpells();
	void	LoadSpellEffects();
	void	LoadSpellLuaData();
	void	LoadTraits();
	int32	LoadPlayerSkillbar(Client* client);
	string	GetColumnNames(char* name);

	string	GetZoneName(char* zone_description);
	bool	GetItemResultsToClient (Client* client, const char* varSearch, int maxResults=20);
	void	LoadRevivePoints(vector<RevivePoint*>* revive_points, int32 zone_id);
	void	SaveBugReport(const char* category, const char* subcategory, const char* causes_crash, const char* reproducible, const char* summary, const char* description, const char* version, const char* player, int32 account_id, const char* spawn_name, int32 spawn_id, int32 zone_id);
	void	FixBugReport();

	int32	LoadQuests();
	void	LoadQuestDetails(Quest* quest);
	
	bool	DeleteCharacter(int32 account_id, int32 character_id);

	int32	GetMaxHotBarID();

	int8	CheckNameFilter(const char* name, int8 min_length = 4, int8 max_length = 15);
	static int32 NextUniqueHotbarID(){ 
		next_id++;
		return next_id; 
	}
	void	LoadFogInit(string zone, PacketStruct* packet);
	static int32		next_id;

	void ToggleCharacterOnline();
	void ToggleCharacterOnline(Client* client, int8 toggle);

	// Zone Instance DB Functions
	map<int32,int32>*	GetInstanceRemovedSpawns(int32 instance_id, int8 type);
	int32				CreateNewInstance(int32 zone_id, int32 playersMinLevel=0, int32 playersMaxLevel=0, int32 playersavgLevel=0, int32 playersfirstLevel=0);
	//int32				AddCharacterInstance(int32 char_id, int32 instance_id, int32 grant_reenter_time_left=0, int32 grant_reset_time_left=0, int32 lockout_time=0);
	int32				AddCharacterInstance(int32 char_id, int32 instance_id, string zone_name, int8 instance_type, int32 last_success, int32 last_failure, int32 success_lockout, int32 failure_lockout);
	bool				UpdateCharacterInstanceTimers(int32 char_id, int32 instance_id, int32 lockout_time=0, int32 reset_time=0, int32 reenter_time=0 );
	bool				UpdateCharacterInstance(int32 char_id, string zone_name, int32 instance_id, int8 type = 0, int32 timestamp = 0);
	bool				VerifyInstanceID(int32 char_id, int32 instance_id);
	bool				CheckVectorForValue(vector<int32>* vector, int32 value);
	int32				CheckSpawnRemoveInfo(map<int32,int32>* inmap, int32 spawn_location_entry_id);
	bool				UpdateInstancedSpawnRemoved(int32 spawn_location_entry_id, int32 spawn_type, int32 respawn_time, int32 instance_id );
	int32				CreateInstanceSpawnRemoved(int32 spawn_location_entry_id, int32 spawn_type, int32 respawn_time, int32 instance_id );
	bool				DeleteInstance(int32 instance_id);
	bool				DeleteInstanceSpawnRemoved(int32 instance_id, int32 spawn_location_entry_id);
	bool				DeleteCharacterFromInstance(int32 char_id, int32 instance_id);
	bool				LoadCharacterInstances(Client* client);
	
	bool				DeletePersistedRespawn(int32 zone_id, int32 spawn_location_entry_id);
	int32				CreatePersistedRespawn(int32 spawn_location_entry_id, int32 spawn_type, int32 respawn_time, int32 zone_id);
	map<int32,int32>*	GetPersistedSpawns(int32 zone_id, int8 type);
	//

	MutexMap<int32, LoginEquipmentUpdate>* GetEquipmentUpdates();
	MutexMap<int32, LoginEquipmentUpdate>* GetEquipmentUpdates(int32 char_id);
	void				UpdateLoginEquipment();
	MutexMap<int32, LoginZoneUpdate>* GetZoneUpdates();
	void				UpdateLoginZones();
	void				LoadLocationGrids(ZoneServer* zone);
	bool				LoadLocationGridLocations(LocationGrid* grid);
	int32				CreateLocation(int32 zone_id, int32 grid_id, const char* name, bool include_y);
	bool				AddLocationPoint(int32 location_id, float x, float y, float z);
	bool				DeleteLocation(int32 location_id);
	bool				DeleteLocationPoint(int32 location_point_id);
	void				ListLocations(Client* client);
	void				ListLocationPoints(Client* client, int32 location_id);
	bool				LocationExists(int32 location_id);


	bool				GetTableVersions(vector<TableVersion *> *table_versions);
	bool				QueriesFromFile(const char *file);


	/* Achievements */
	void				LoadAchievements();
	int32				LoadAchievementRequirements(Achievement *achievement);
	int32				LoadAchievementRewards(Achievement *achievement);
	void				LoadPlayerAchievements(Player *player);
	int32				LoadPlayerAchievementsUpdates(Player *player);
	int32				LoadPlayerAchievementsUpdateItems(AchievementUpdate *update, int32 player_id);

	/* Alternate Advancement */
	void				LoadAltAdvancements();
	void				LoadTreeNodes();

	/* Collections */
	void				LoadCollections();
	int32				LoadCollectionItems(Collection *collection);
	int32				LoadCollectionRewards(Collection *collection);
	void				LoadPlayerCollections(Player *player);
	void				LoadPlayerCollectionItems(Player *player, Collection *collection);
	void				SavePlayerCollections(Client *client);
	void				SavePlayerCollection(Client *client, Collection *collection);
	void				SavePlayerCollectionItems(Client *client, Collection *collection);
	void				SavePlayerCollectionItem(Client *client, Collection *collection, int32 item_id);
	
	/* Commands */
	map<int32, string>* GetSpawnTemplateListByName(const char* name);
	map<int32, string>* GetSpawnTemplateListByID(int32 location_id);
	int32				SaveSpawnTemplate(int32 placement_id, const char* template_name);
	bool				RemoveSpawnTemplate(int32 template_id);
	int32				CreateSpawnFromTemplateByID(Client* client, int32 template_id);
	int32				CreateSpawnFromTemplateByName(Client* client, const char* template_name);
	bool				SaveZoneSafeCoords(int32 zone_id, float x, float y, float z, float heading);
	bool				SaveSignZoneToCoords(int32 spawn_id, float x, float y, float z, float heading);

	/* Guilds */
	void				LoadGuilds();
	void				LoadGuild(int32 guild_id);
	int32				LoadGuildMembers(Guild* guild);
	void				LoadGuildEvents(Guild* guild);
	void				LoadGuildRanks(Guild* guild);
	void				LoadGuildEventFilters(Guild* guild);
	void				LoadGuildPointsHistory(Guild* guild, GuildMember* guild_member);
	void				LoadGuildRecruiting(Guild* guild);
	void				SaveGuild(Guild* guild, bool new_guild = false);
	void				SaveGuildMembers(Guild* guild);
	void				SaveGuildEvents(Guild* guild);
	void				SaveGuildRanks(Guild* guild);
	void				SaveGuildEventFilters(Guild* guild);
	void				SaveGuildPointsHistory(Guild* guild);
	void				SaveGuildRecruiting(Guild* guild);
	void				DeleteGuild(Guild* guild);
	void				DeleteGuildMember(Guild* guild, int32 character_id);
	void				DeleteGuildEvent(Guild* guild, int64 event_id);
	void				DeleteGuildPointHistory(Guild* guild, int32 character_id, PointHistory* point_history);
	void				ArchiveGuildEvent(Guild* guild, GuildEvent* guild_event);
	void				SaveHiddenGuildEvent(Guild* guild, GuildEvent* guild_event);
	void				LoadGuildDefaultRanks(Guild* guild);
	void				LoadGuildDefaultEventFilters(Guild* guild);
	bool				AddNewPlayerToServerGuild(int32 account_id, int32 char_id);
	int32				GetGuildIDByCharacterID(int32 char_id);

	/* Chat */
	void LoadChannels();

	/* Recipes */
	void	LoadRecipes();
	void	LoadRecipeBooks();
	void	LoadPlayerRecipes(Player *player);
	int32	LoadPlayerRecipeBooks(int32 char_id, Player *player);
	void	SavePlayerRecipeBook(Player* player, int32 recipebook_id);
	void	LoadRecipeComponents();
	void	UpdatePlayerRecipe(Player* player, int32 recipe_id, int8 highest_rank);
	void	SavePlayerRecipe(Player* player, int32 recipe_id);

	/* Tradeskills */
	void	LoadTradeskillEvents();

	/* Rules */
	void LoadGlobalRuleSet();
	void LoadRuleSets(bool reload=false);
	void LoadRuleSetDetails(RuleSet *rule_set);

	/* Titles */
	sint32				AddMasterTitle(const char* titleName, int8 isPrefix = 0);
	void				LoadTitles();
	sint32				LoadCharacterTitles(int32 char_id, Player *player);
	sint32				GetCharPrefixIndex(int32 char_id, Player *player);
	sint32				GetCharSuffixIndex(int32 char_id, Player *player);
	void				SaveCharPrefixIndex(sint32 index, int32 char_id);
	void				SaveCharSuffixIndex(sint32 index, int32 char_id);
	sint32				AddCharacterTitle(sint32 index, int32 char_id, Spawn* player);

	/* Languages */
	void				LoadLanguages();
	int32				LoadCharacterLanguages(int32 char_id, Player *player);
	int16				GetCharacterCurrentLang(int32 char_id, Player *player);
	void				SaveCharacterCurrentLang(int32 id, int32 char_id, Client *client);
	void 				UpdateStartingLanguage(int32 char_id, uint8 race_id, int32 starting_city=0);

	/// <summary>Saves the given language for the given player</summary>
	/// <param name='char_id'>Character ID to save the language to</param>
	/// <param name='lang_id'>Language ID to save to the character</param>
	void	SaveCharacterLang(int32 char_id, int32 lang_id);

	/* Tradeskills */

	/* Character History */
	void				SaveCharacterHistory(Player* player, int8 type, int8 subtype, int32 value, int32 value2, char* location, int32 event_date);

	/* Housing */
	void				LoadHouseZones();
	int64				AddPlayerHouse(int32 char_id, int32 house_id, int32 instance_id, int32 upkeep_due);
	void				SetHouseUpkeepDue(int32 char_id, int32 house_id, int32 instance_id, int32 upkeep_due);
	void				RemovePlayerHouse(int32 char_id, int32 house_id);
	void				UpdateHouseEscrow(int32 house_id, int32 instance_id, int64 amount_coins, int32 amount_status);
	void				LoadPlayerHouses();
	void				LoadDeposits(PlayerHouse* house);
	void				LoadHistory(PlayerHouse* house);
	void				AddHistory(PlayerHouse* house, char* name, char* reason, int32 timestamp, int64 amount = 0, int32 status = 0, int8 pos_flag = 0);

	/* World */
	bool				CheckBannedIPs(const char* loginIP);

	/* Heroic OP */
	void				LoadHOStarters();
	void				LoadHOWheel();

	/* Claim Items */
	void				LoadClaimItems(int32 char_id);
	int16				CountCharClaimItems(int32 char_id);
	vector<ClaimItems>  LoadCharacterClaimItems(int32 char_id);
	void				ClaimItem(int32 char_id, int32 item_id, Client* client);
	int32				GetAccountAge(int32 account_id);

	/* Race Types */
	void				LoadRaceTypes();

	/* Loot */
	void				LoadLoot(ZoneServer* zone);
	void				LoadGlobalLoot(ZoneServer* zone);
	bool				LoadSpawnLoot(ZoneServer* zone, Spawn* spawn);
	void				AddLootTableToSpawn(Spawn* spawn, int32 loottable_id);
	bool				RemoveSpawnLootTable(Spawn* spawn, int32 loottable_id);

	void				LoadCharacterHistory(int32 char_id, Player *player);
	void				LoadSpellErrors();

	/* Load single spawns */
	bool				LoadSign(ZoneServer* zone, int32 spawn_id);
	bool				LoadWidget(ZoneServer* zone, int32 spawn_id);
	bool				LoadObject(ZoneServer* zone, int32 spawn_id);
	bool				LoadGroundSpawn(ZoneServer* zone, int32 spawn_id);
	void				LoadGroundSpawnEntry(ZoneServer* zone, int32 entry_id);
	void				LoadGroundSpawnItems(ZoneServer* zone, int32 entry_id);
	bool				LoadNPC(ZoneServer* zone, int32 spawn_id);
	void				LoadAppearance(ZoneServer* zone, int32 spawn_id);
	void				LoadNPCAppearanceEquipmentData(ZoneServer* zone, int32 spawn_id);

	/* Save character Pictures */
	/// <summary>Saves the pictures that clients send to the server</summary>
	/// <param name='characterID'>The ID of the character</param>
	/// <param name='type'>The type of image this is, 0 = paperdoll, 1 = headshot</param>
	/// <param name='picture'>The raw png data</param>
	void				SaveCharacterPicture(int32 characterID, int8 type, uchar* picture, int32 picture_size);

	/* Quests */
	/// <summary>Updates the given date for the quest in the DB for a repeatable quest</summary>
	/// <param name='client'>Client to save the quest for</param>
	/// <param name='quest_id'>ID of the quest to save</param>
	/// <param name='quest_complete_count'>Number of times the quest has already been completed</param>
	void				SaveCharRepeatableQuest(Client* client, int32 quest_id, int16 quest_complete_count);


	/* Zone Flight Paths */
	void				LoadZoneFlightPaths(ZoneServer* zone);
	void				LoadZoneFlightPathLocations(ZoneServer* zone);

	/* Character LUA History */
	void				SaveCharacterLUAHistory(Player* player, int32 event_id, int32 value, int32 value2);
	void				LoadCharacterLUAHistory(int32 char_id, Player* player);

	/* Bots - BotDB.cpp */
	int32				CreateNewBot(int32 char_id, string name, int8 race, int8 advClass, int8 gender, int16 model_id, int32& index);
	void				SaveBotAppearance(Bot* bot);
	void				SaveBotColors(int32 bot_id, const char* type, EQ2_Color color);
	void				SaveBotFloats(int32 bot_id, const char* type, float float1, float float2, float float3);
	bool				LoadBot(int32 char_id, int32 bot_index, Bot* bot);
	void				LoadBotAppearance(Bot* bot);
	void				SaveBotItem(int32 bot_id, int32 item_id, int8 slot);
	void				LoadBotEquipment(Bot* bot);
	string				GetBotList(int32 char_id);
	void				DeleteBot(int32 char_id, int32 bot_index);
	void				SetBotStartingItems(Bot* bot, int8 class_id, int8 race_id);
	void                LoadTransmuting();

	void				FindSpell(Client* client, char* findString);

	void				LoadChestTraps();

	bool				CheckExpansionFlags(ZoneServer* zone, int32 spawnXpackFlag);
	bool				CheckHolidayFlags(ZoneServer* zone, int32 spawnHolidayFlag);
	void				GetHouseSpawnInstanceData(ZoneServer* zone, Spawn* spawn);
	int32				FindHouseInstanceSpawn(Spawn* spawn);

	/* Starting Character Abilities */

	void				LoadStartingSkills(World* world);
	void				LoadStartingSpells(World* world);
	void				LoadVoiceOvers(World* world);

	int32				CreateSpiritShard(const char* name, int32 level, int8 race, int8 gender, int8 adventure_class, 
									  int16 model_type, int16 soga_model_type, int16 hair_type, int16 hair_face_type, int16 wing_type,
									  int16 chest_type, int16 legs_type, int16 soga_hair_type, int16 soga_hair_face_type, int8 hide_hood, 
									  int16 size, int16 collision_radius, int16 action_state, int16 visual_state, int16 mood_state, int16 emote_state, 
									  int16 pos_state, int16 activity_status, char* sub_title, char* prefix_title, char* suffix_title, char* lastname, 
									  float x, float y, float z, float heading, int32 gridid, int32 charid, int32 zoneid, int32 instanceid);
	bool				DeleteSpiritShard(int32 id);

	void				LoadCharacterSpellEffects(int32 char_id, Client *client, int8 db_spell_type);

	int32				GetMysqlExpCurve(int level);
private:
	DatabaseNew			database_new;
	std::map<int32, string>	zone_names;
	string				skills;
	int32				max_zonename;
	char**				zonename_array;
	std::map<int32, int8>	zone_instance_types;
};
#endif

