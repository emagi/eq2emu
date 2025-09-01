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

#ifndef __EQ2_PLAYER__
#define __EQ2_PLAYER__

#include "Entity.h"
#include "Items/Items.h"
#include "Factions.h"
#include "Skills.h"
#include "Quests.h"
#include "MutexMap.h"
#include "Guilds/Guild.h"
#include "Collections/Collections.h"
#include "Recipes/Recipe.h"
#include "Titles.h"
#include "Languages.h"
#include "Achievements/Achievements.h"
#include "Traits/Traits.h"
#include <algorithm>
#include <set>

#define CF_COMBAT_EXPERIENCE_ENABLED  0
#define CF_ENABLE_CHANGE_LASTNAME  1
#define CF_FOOD_AUTO_CONSUME  2
#define CF_DRINK_AUTO_CONSUME  3
#define CF_AUTO_ATTACK  4
#define CF_RANGED_AUTO_ATTACK  5
#define CF_QUEST_EXPERIENCE_ENABLED  6
#define CF_CHASE_CAMERA_MAYBE  7
#define CF_100  8
#define CF_200  9
#define CF_IS_SITTING  10 /*CAN'T CAST OR ATTACK*/
#define CF_800  11
#define CF_ANONYMOUS  12
#define CF_ROLEPLAYING  13
#define CF_AFK  14
#define CF_LFG  15
#define CF_LFW  16
#define CF_HIDE_HOOD  17
#define CF_HIDE_HELM  18
#define CF_SHOW_ILLUSION  19
#define CF_ALLOW_DUEL_INVITES  20
#define CF_ALLOW_TRADE_INVITES  21
#define CF_ALLOW_GROUP_INVITES  22
#define CF_ALLOW_RAID_INVITES  23
#define CF_ALLOW_GUILD_INVITES  24
#define CF_2000000  25
#define CF_4000000  26
#define CF_DEFENSE_SKILLS_AT_MAX_QUESTIONABLE  27
#define CF_SHOW_GUILD_HERALDRY  28
#define CF_SHOW_CLOAK  29
#define CF_IN_PVP  30
#define CF_IS_HATED  31
#define CF2_1  32
#define CF2_2  33
#define CF2_4  34
#define CF2_ALLOW_LON_INVITES  35
#define CF2_SHOW_RANGED 36
#define CF2_ALLOW_VOICE_INVITES  37
#define CF2_CHARACTER_BONUS_EXPERIENCE_ENABLED  38
#define CF2_80  39
#define CF2_100  40 /* hide achievments*/
#define CF2_200  41
#define CF2_400  42
#define CF2_800  43 /* enable facebook updates*/
#define CF2_1000  44 /* enable twitter updates*/
#define CF2_2000  45 /* enable eq2 player updates */
#define CF2_4000  46 /*eq2 players, link to alt chars */
#define CF2_8000  47
#define CF2_10000  48
#define CF2_20000  49
#define CF2_40000  50
#define CF2_80000  51
#define CF2_100000  52
#define CF2_200000  53
#define CF2_400000  54
#define CF2_800000  55
#define CF2_1000000  56
#define CF2_2000000  57
#define CF2_4000000  58
#define CF2_8000000  59
#define CF2_10000000  60
#define CF2_20000000  61
#define CF2_40000000  62
#define CF2_80000000  63
#define CF_MAXIMUM_FLAG  63
#define CF_HIDE_STATUS  49 /* !!FORTESTING ONLY!! */
#define CF_GM_HIDDEN  50 /* !!FOR TESTING ONLY!! */

#define UPDATE_ACTIVITY_FALLING			0
#define UPDATE_ACTIVITY_RUNNING			128
#define UPDATE_ACTIVITY_RIDING_BOAT     256
#define UPDATE_ACTIVITY_JUMPING			1024
#define UPDATE_ACTIVITY_IN_WATER_ABOVE	6144
#define UPDATE_ACTIVITY_IN_WATER_BELOW	6272
#define UPDATE_ACTIVITY_SITING			6336
#define UPDATE_ACTIVITY_DROWNING		14464
#define UPDATE_ACTIVITY_DROWNING2		14336



#define UPDATE_ACTIVITY_FALLING_AOM			16384
#define UPDATE_ACTIVITY_RIDING_BOAT_AOM     256
#define UPDATE_ACTIVITY_RUNNING_AOM			16512
#define UPDATE_ACTIVITY_JUMPING_AOM			17408
#define UPDATE_ACTIVITY_MOVE_WATER_BELOW_AOM	22528
#define UPDATE_ACTIVITY_MOVE_WATER_ABOVE_AOM	22656
#define UPDATE_ACTIVITY_SITTING_AOM			22720
#define UPDATE_ACTIVITY_DROWNING_AOM		30720
#define UPDATE_ACTIVITY_DROWNING2_AOM		30848

#define NUM_MAINTAINED_EFFECTS	30
#define NUM_SPELL_EFFECTS		45

/* Character History Type Defines */
#define HISTORY_TYPE_NONE		0
#define HISTORY_TYPE_DEATH		1
#define HISTORY_TYPE_DISCOVERY	2
#define HISTORY_TYPE_XP			3

/* Spell Status */
#define SPELL_STATUS_QUEUE		4
#define SPELL_STATUS_LOCK		66

/* Character History Sub Type Defines */
#define HISTORY_SUBTYPE_NONE	0
#define HISTORY_SUBTYPE_ADVENTURE	1
#define HISTORY_SUBTYPE_TRADESKILL	2
#define HISTORY_SUBTYPE_QUEST		3
#define HISTORY_SUBTYPE_AA			4
#define HISTORY_SUBTYPE_ITEM		5
#define HISTORY_SUBTYPE_LOCATION	6

/// <summary>Character history data, should match the `character_history` table in the DB</summary>
struct HistoryData {
	int32		Value;
	int32		Value2;
	char		Location[200];
	int32		EventID;
	int32		EventDate;
	bool		needs_save;
};

/// <summary>History set through the LUA system</summary>
struct LUAHistory {
	int32 Value;
	int32 Value2;
	bool SaveNeeded;
};

struct SpellBookEntry{
	int32	spell_id;
	int8	tier;
	int32	type;
	sint32	slot;
	int32	recast_available;
	int8	status;
	int16	recast;
	int32	timer;
	bool	save_needed;
	bool	in_use;
	bool	in_remiss;
	Player* player;
	bool visible;
};

struct GMTagFilter {
	int32	filter_type;
	int32	filter_value;
	char	filter_search_criteria[256];
	int16	visual_tag;
};

enum GMTagFilterType {
	GMFILTERTYPE_NONE=0,
	GMFILTERTYPE_FACTION=1,
	GMFILTERTYPE_SPAWNGROUP=2,
	GMFILTERTYPE_RACE=3,
	GMFILTERTYPE_GROUNDSPAWN=4
};
enum SpawnState{
	SPAWN_STATE_NONE=0,
	SPAWN_STATE_SENDING=1,
	SPAWN_STATE_SENT_WAIT=2,
	SPAWN_STATE_SENT=3,
	SPAWN_STATE_REMOVING=4,
	SPAWN_STATE_REMOVING_SLEEP=5,
	SPAWN_STATE_REMOVED=6
};
#define QUICKBAR_NORMAL		1
#define QUICKBAR_INV_SLOT	2
#define QUICKBAR_MACRO		3
#define QUICKBAR_TEXT_CMD	4
#define QUICKBAR_ITEM		6

#define EXP_DISABLED_STATE	0
#define EXP_ENABLED_STATE	1
#define MELEE_COMBAT_STATE	16
#define RANGE_COMBAT_STATE	32

struct QuickBarItem{
	bool				deleted;
	int32				hotbar;
	int32 				slot;
	int32 				type;
	int16 				icon;
	int16				icon_type;
	int32				id;
	int8				tier;
	int64				unique_id;
	EQ2_16BitString		text;
};

struct LoginAppearances {
	bool	deleted;
	int16	equip_type;
	int8	red;
	int8	green;
	int8	blue;
	int8	h_red;
	int8	h_green;
	int8	h_blue;
	bool	update_needed;
};

struct SpawnQueueState {
	Timer spawn_state_timer;
	int16 index_id;
};

class PlayerLoginAppearance {
public:
	PlayerLoginAppearance() { appearanceList = new map<int8, LoginAppearances>; }
	~PlayerLoginAppearance() { }

	void AddEquipmentToUpdate(int8 slot_id, LoginAppearances* equip)
	{
		//LoginAppearances data;
		//data.equip_type = equip->equip_type;
		//appearanceList[slot_id] = data;
	}

	void DeleteEquipmentFromUpdate(int8 slot_id, LoginAppearances* equip)
	{
		//LoginAppearances data;
		//data.deleted		= equip->deleted;
		//data.update_needed	= true;
		//appearanceList[slot_id] = data;
	}

	void RemoveEquipmentUpdates()
	{
	 	appearanceList->clear();
		safe_delete(appearanceList);
	}

private:
	map<int8, LoginAppearances>* appearanceList;
};


struct InstanceData{
	int32	db_id;
	int32	instance_id;
	int32	zone_id;
	int8	zone_instance_type;
	string	zone_name;
	int32	last_success_timestamp;
	int32	last_failure_timestamp;
	int32	success_lockout_time;
	int32	failure_lockout_time;
};

class CharacterInstances {
public:
	CharacterInstances();
	~CharacterInstances();

	///<summary>Adds an instance data to the player with the given data</summary>
	///<param name='db_id'>The unique id for this record in the database</param>
	///<param name='instance_id'>The id of the instance</param>
	///<param name='last_success_timestamp'>The success timestamp</param>
	///<param name='last_failure_timestamp'>The failure timestamp</param>
	///<param name='success_lockout_time'>The lockout time, in secs, for completing the instance</param>
	///<param name='failure_lockout_time'>The lockout time, in secs, for failing the instance</param>
	///<param name='zone_id'>The id of the zone</param>
	///<param name='zone_instancetype'>The type of instance of the zone</param>
	///<param name='zone_name'>The name of the zone</param>
	void AddInstance(int32 db_id, int32 instance_id, int32 last_success_timestamp, int32 last_failure_timestamp, int32 success_lockout_time, int32 failure_lockout_time, int32 zone_id, int8 zone_instancetype, string zone_name);

	///<summary>Clears all instance data</summary>
	void RemoveInstances();
	
	///<summary>Removes the instace with the given zone id</summary>
	///<param name='zone_id'>The zone id of the instance to remove</param>
	///<returns>True if the instance was found and removed</returns>
	bool RemoveInstanceByZoneID(int32 zone_id);
	
	///<summary>Removes the instance with the given instance id</summary>
	///<param name='instance_id'>the instance id of the instance to remove</param>
	///<returns>True if instance was found and removed</returns>
	bool RemoveInstanceByInstanceID(int32 instance_id);

	///<summary>Gets the instance with the given zone id</summary>
	///<param name='zone_id'>The zone id of the instance to get</param>
	///<returns>InstanceData* of the instance record for the given zone id</returns>
	InstanceData* FindInstanceByZoneID(int32 zone_id);

	///<summary>Gets the instance with the given database id</summary>
	///<param name='db_id'>The database id of the instance to get</param>
	///<returns>InstanceData* of the instance record for the given database id</returns>
	InstanceData* FindInstanceByDBID(int32 db_id);
	
	///<summary>Gets the instance with the given instance id</summary>
	///<param name='instance_id'>The instance id of the instance to get</param>
	///<returns>InstanceData* of the instance record for the given instance id</returns>
	InstanceData* FindInstanceByInstanceID(int32 instance_id);

	///<summary>Gets a list of all the lockout instances</summary>
	vector<InstanceData> GetLockoutInstances();

	///<summary>Gets a list of all the persistent instances</summary>
	vector<InstanceData> GetPersistentInstances();
	
	///<summary>Check the timers for the instances</summary>
	///<param name='player'>player we are checking the timers for</param>
	void ProcessInstanceTimers(Player* player);

	///<summary>Gets the total number of instances</summary>
	int32 GetInstanceCount();
private:
	vector<InstanceData> instanceList;
	Mutex m_instanceList;
};

class Player;
struct PlayerGroup;
struct GroupMemberInfo;
struct Statistic;
struct Mail;
class PlayerInfo {
public:
	~PlayerInfo();
	PlayerInfo(Player* in_player);

	EQ2Packet* serialize(int16 version, int16 modifyPos = 0, int32 modifyValue = 0);
	PacketStruct* serialize2(int16 version);
	EQ2Packet* serialize3(PacketStruct* packet, int16 version);
	EQ2Packet* serializePet(int16 version);
	void CalculateXPPercentages();
	void CalculateTSXPPercentages();
	void SetHouseZone(int32 id);
	void SetBindZone(int32 id);
	void SetBindX(float x);
	void SetBindY(float y);
	void SetBindZ(float z);
	void SetBindHeading(float heading);
	void SetAccountAge(int32 days);
	int32 GetHouseZoneID();
	int32 GetBindZoneID();
	float GetBindZoneX();
	float GetBindZoneY();
	float GetBindZoneZ();
	float GetBindZoneHeading();
	float GetBoatX() { return boat_x_offset; }
	float GetBoatY() { return boat_y_offset; }
	float GetBoatZ() { return boat_z_offset; }
	int32 GetBoatSpawn();
	void SetBoatX(float x) { boat_x_offset = x; }
	void SetBoatY(float y) { boat_y_offset = y; }
	void SetBoatZ(float z) { boat_z_offset = z; }
	void SetBoatSpawn(Spawn* boat);
	void RemoveOldPackets();

private:
	int32			house_zone_id;
	int32			bind_zone_id;
	float			bind_x;
	float			bind_y;
	float			bind_z;
	float			bind_heading;
	uchar*			changes;
	uchar*			orig_packet;
	uchar*			pet_changes;
	uchar*			pet_orig_packet;
	InfoStruct*		info_struct;
	Player*			player;
	float           boat_x_offset;
	float           boat_y_offset;
	float           boat_z_offset;
	int32           boat_spawn;
};

class PlayerControlFlags{
public:
	PlayerControlFlags();
	~PlayerControlFlags();

	void SetPlayerControlFlag(int8 param, int8 param_value, bool is_active);
	bool ControlFlagsChanged();
	void SendControlFlagUpdates(Client* client);
private:
	bool flags_changed;
	map<int8, map<int8, int8> > flag_changes;
	map<int8, map<int8, bool> > current_flags;
	Mutex MControlFlags;
	Mutex MFlagChanges;
};

class Player : public Entity{
public:
	Player();
	virtual ~Player();
	EQ2Packet* serialize(Player* player, int16 version);
	//int8	GetMaxArtLevel(){ return info->GetInfo()->max_art_level; }
	//int8	GetArtLevel(){ return info->GetInfo()->art_level; }

	Client* GetClient() { return client; }
	void SetClient(Client* client) { this->client = client; }
	PlayerInfo* GetPlayerInfo();
	void SetCharSheetChanged(bool val);
	bool GetCharSheetChanged();
	void SetRaidSheetChanged(bool val);
	bool GetRaidSheetChanged();
	void AddFriend(const char* name, bool save);
	bool IsFriend(const char* name);
	void RemoveFriend(const char* name);
	map<string, int8>* GetFriends();
	void AddIgnore(const char* name, bool save);
	bool IsIgnored(const char* name);
	void RemoveIgnore(const char* name);
	map<string, int8>* GetIgnoredPlayers();

	// JA: POI Discoveries
	map<int32, vector<int32> >* GetPlayerDiscoveredPOIs();
	void AddPlayerDiscoveredPOI(int32 location_id);
	//

	EQ2Packet* Move(float x, float y, float z, int16 version, float heading = -1.0f);

	/*void	SetMaxArtLevel(int8 new_max){
		max_art_level = new_max;
		}
		void	SetArtLevel(int8 new_lvl){
		art_level = new_lvl;
		}*/
	bool WasSentSpawn(int32 spawn_id);
	bool IsSendingSpawn(int32 spawn_id);
	bool IsRemovingSpawn(int32 spawn_id);
	bool SetSpawnSentState(Spawn* spawn, SpawnState state);
	void CheckSpawnStateQueue();
	void SetSideSpeed(float side_speed, bool updateFlags = true) {
		SetPos(&appearance.pos.SideSpeed, side_speed, updateFlags);
	}
	float GetSideSpeed() {
		return appearance.pos.SideSpeed;
	}
	void SetVertSpeed(float vert_speed, bool updateFlags = true) {
		SetPos(&appearance.pos.VertSpeed, vert_speed, updateFlags);
	}
	float GetVertSpeed() {
		return appearance.pos.VertSpeed;
	}
	
	void SetClientHeading1(float heading, bool updateFlags = true) {
		SetPos(&appearance.pos.ClientHeading1, heading, updateFlags);
	}
	float GetClientHeading1() {
		return appearance.pos.ClientHeading1;
	}
	
	void SetClientHeading2(float heading, bool updateFlags = true) {
		SetPos(&appearance.pos.ClientHeading2, heading, updateFlags);
	}
	float GetClientHeading2() {
		return appearance.pos.ClientHeading2;
	}
	
	void SetClientPitch(float pitch, bool updateFlags = true) {
		SetPos(&appearance.pos.ClientPitch, pitch, updateFlags);
	}
	float GetClientPitch() {
		return appearance.pos.ClientPitch;
	}
	
	int8 GetTutorialStep() {
		return tutorial_step;
	}
	void SetTutorialStep(int8 val) {
		tutorial_step = val;
	}
	void	AddMaintainedSpell(LuaSpell* spell);
	void	AddSpellEffect(LuaSpell* spell, int32 override_expire_time = 0);
	void	RemoveMaintainedSpell(LuaSpell* spell);
	void	RemoveSpellEffect(LuaSpell* spell);
	void	AddQuickbarItem(int32 bar, int32 slot, int32 type, int16 icon, int16 icon_type, int32 id, int8 tier, int32 unique_id, const char* text, bool update = true);
	void	RemoveQuickbarItem(int32 bar, int32 slot, bool update = true);
	void	MoveQuickbarItem(int32 id, int32 new_slot);
	void	ClearQuickbarItems();
	PlayerItemList*	GetPlayerItemList();
	PlayerItemList    item_list;
	PlayerSkillList	  skill_list;
	Skill*	GetSkillByName(const char* name, bool check_update = false);
	Skill*	GetSkillByID(int32 skill_id, bool check_update = false);
	PlayerSkillList* GetSkills();
	bool DamageEquippedItems(int8 amount = 10, Client* client = 0);
	vector<EQ2Packet*>	EquipItem(int16 index, int16 version, int8 appearance_type, int8 slot_id = 255);
	bool CanEquipItem(Item* item, int8 slot);
	void SetEquippedItemAppearances();
	vector<EQ2Packet*>	UnequipItem(int16 index, sint32 bag_id, int8 slot, int16 version, int8 appearance_type = 0, bool send_item_updates = true);
	int16 ConvertSlotToClient(int8 slot, int16 version);
	int16 ConvertSlotFromClient(int8 slot, int16 version);
	int16 GetNumSlotsEquip(int16 version);
	int8 GetMaxBagSlots(int16 version);
	EQ2Packet* SwapEquippedItems(int8 slot1, int8 slot2, int16 version, int16 equiptype);
	EQ2Packet*	RemoveInventoryItem(int8 bag_slot, int8 slot);
	EQ2Packet*	SendInventoryUpdate(int16 version);
	EQ2Packet*	SendBagUpdate(int32 bag_unique_id, int16 version);
	void        SendQuestRequiredSpawns(int32 quest_id);
	void		SendHistoryRequiredSpawns(int32 event_id);
	map<int32, Item*>* GetItemList();
	map<int32, Item*>* GetBankItemList();
	vector<Item*>* GetEquippedItemList();
	vector<Item*>* GetAppearanceEquippedItemList();
	Quest* SetStepComplete(int32 quest_id, int32 step);
	Quest* AddStepProgress(int32 quest_id, int32 step, int32 progress);
	int32  GetStepProgress(int32 quest_id, int32 step_id);
	Quest* GetQuestByPositionID(int32 list_position_id);
	bool AddItem(Item* item, AddItemType type = AddItemType::NOT_SET);
	bool AddItemToBank(Item* item);
	int16 GetSpellSlotMappingCount();
	int16 GetSpellPacketCount();
	Quest* GetQuest(int32 quest_id);
	bool GetQuestStepComplete(int32 quest_id, int32 step_id);
	int16 GetQuestStep(int32 quest_id);
	int16 GetTaskGroupStep(int32 quest_id);
	int8 GetSpellTier(int32 id);
	void SetSpellStatus(Spell* spell, int8 status);
	void RemoveSpellStatus(Spell* spell, int8 status);
	EQ2Packet* GetSpellSlotMappingPacket(int16 version);
	EQ2Packet* GetSpellBookUpdatePacket(int16 version);
	EQ2Packet* GetRaidUpdatePacket(int16 version);
	int32 GetCharacterID();
	void SetCharacterID(int32 new_id);
	EQ2Packet* GetQuickbarPacket(int16 version);
	vector<QuickBarItem*>* GetQuickbar();
	bool UpdateQuickbarNeeded();
	void ResetQuickbarNeeded();
	void set_character_flag(int flag);
	void reset_character_flag(int flag);
	void toggle_character_flag(int flag);
	bool get_character_flag(int flag);
	void AddCoins(int64 val);
	bool RemoveCoins(int64 val);
	/// <summary>Checks to see if the player has the given amount of coins</summary>
	/// <param name="val">Amount of coins to check</param>
	/// <returns>True if the player has enough coins</returns>
	bool HasCoins(int64 val);
	void AddSkill(int32 skill_id, int16 current_val, int16 max_val, bool save_needed = false);
	void RemovePlayerSkill(int32 skill_id, bool save = false);
	void RemoveSkillFromDB(Skill* skill, bool save = false);
	void AddSpellBookEntry(int32 spell_id, int8 tier, sint32 slot, int32 type, int32 timer, bool save_needed = false);
	SpellBookEntry* GetSpellBookSpell(int32 spell_id);
	vector<SpellBookEntry*>* GetSpellsSaveNeeded();
	sint32 GetFreeSpellBookSlot(int32 type);
	/// <summary>Get a vector of spell ids for all spells in the spell book for the given skill</summary>
	/// <param name='skill_id'>The id of the skill to check</param>
	/// <returns>A vector of int32's of the spell id's</returns>
	vector<int32> GetSpellBookSpellIDBySkill(int32 skill_id);
	void UpdateInventory(int32 bag_id);
	EQ2Packet* MoveInventoryItem(sint32 to_bag_id, int16 from_index, int8 new_slot, int8 charges, int8 appearance_type, bool* item_deleted, int16 version = 1);
	bool IsPlayer(){ return true; }
	MaintainedEffects* GetFreeMaintainedSpellSlot();
	MaintainedEffects* GetMaintainedSpell(int32 id, bool on_char_load = false);
	MaintainedEffects* GetMaintainedSpellBySlot(int8 slot);
	MaintainedEffects* GetMaintainedSpells();
	SpellEffects* GetFreeSpellEffectSlot();
	SpellEffects* GetSpellEffects();
	int32	GetCoinsCopper();
	int32	GetCoinsSilver();
	int32	GetCoinsGold();
	int32	GetCoinsPlat();
	int32	GetBankCoinsCopper();
	int32	GetBankCoinsSilver();
	int32	GetBankCoinsGold();
	int32	GetBankCoinsPlat();
	int32	GetStatusPoints();
	float	GetXPVitality();
	float	GetTSXPVitality();
	bool	AdventureXPEnabled();
	bool	TradeskillXPEnabled();
	void	SetNeededXP(int32 val);
	void	SetNeededXP();
	static int32 GetNeededXPByLevel(int8 level);
	void	SetXP(int32 val);
	void	SetNeededTSXP(int32 val);
	void	SetNeededTSXP();
	void	SetTSXP(int32 val);
	int32	GetNeededXP();
	float	GetXPDebt();
	int32	GetXP();
	int32	GetNeededTSXP();
	int32	GetTSXP();
	bool	AddXP(int32 xp_amount);
	bool	AddTSXP(int32 xp_amount);
	bool	DoubleXPEnabled();
	float	CalculateXP(Spawn* victim);
	float	CalculateTSXP(int8 level);
	void	CalculateOfflineDebtRecovery(int32 unix_timestamp);
	void	InCombat(bool val, bool range = false);
	void	PrepareIncomingMovementPacket(int32 len, uchar* data, int16 version, bool dead_window_sent = false);
	uchar*	GetMovementPacketData(){
		return movement_packet;
	}
	void	AddSpawnInfoPacketForXOR(int32 spawn_id, uchar* packet, int16 packet_size);
	uchar*	GetSpawnInfoPacketForXOR(int32 spawn_id);
	void	AddSpawnVisPacketForXOR(int32 spawn_id, uchar* packet, int16 packet_size);
	uchar*	GetSpawnVisPacketForXOR(int32 spawn_id);
	void	AddSpawnPosPacketForXOR(int32 spawn_id, uchar* packet, int16 packet_size);
	uchar*	GetSpawnPosPacketForXOR(int32 spawn_id);
	uchar*	GetTempInfoPacketForXOR();
	uchar*	GetTempVisPacketForXOR();
	uchar*	GetTempPosPacketForXOR();
	uchar*	SetTempInfoPacketForXOR(int16 size);
	uchar*	SetTempVisPacketForXOR(int16 size);
	uchar*	SetTempPosPacketForXOR(int16 size);
	int32	GetTempInfoXorSize() { return info_xor_size; }
	int32	GetTempVisXorSize() { return vis_xor_size; }
	int32	GetTempPosXorSize() { return pos_xor_size; }
	bool	CheckPlayerInfo();
	void	CalculateLocation();
	void	SetSpawnDeleteTime(int32 id, int32 time);
	int32	GetSpawnDeleteTime(int32 id);
	void	ClearRemovalTimers();
	void	ClearEverything();
	bool	IsResurrecting();
	void	SetResurrecting(bool val);
	int8    GetTSArrowColor(int8 level);
	Spawn*	GetSpawnByIndex(int16 index);
	int16	GetIndexForSpawn(Spawn* spawn);
	bool	WasSpawnRemoved(Spawn* spawn);
	void	ResetSpawnPackets(int32 id);
	void	RemoveSpawn(Spawn* spawn, bool delete_spawn = true);
	bool	ShouldSendSpawn(Spawn* spawn);
	Client* client = 0;
	void SetLevel(int16 level, bool setUpdateFlags = true);

	Spawn* GetSpawnWithPlayerID(int32 id){
		Spawn* spawn = 0;

		index_mutex.readlock(__FUNCTION__, __LINE__);
		if (player_spawn_id_map.count(id) > 0)
			spawn = player_spawn_id_map[id];
		index_mutex.releasereadlock(__FUNCTION__, __LINE__);
		return spawn;
	}
	int32 GetIDWithPlayerSpawn(Spawn* spawn){
		int32 id = 0;
	
		index_mutex.readlock(__FUNCTION__, __LINE__);
		if (player_spawn_reverse_id_map.count(spawn) > 0)
			id = player_spawn_reverse_id_map[spawn];
		index_mutex.releasereadlock(__FUNCTION__, __LINE__);

		return id;
	}

	int16 GetNextSpawnIndex(Spawn* spawn, bool set_lock = true);
	bool SetSpawnMap(Spawn* spawn);

	void SetSpawnMapIndex(Spawn* spawn, int32 index)
	{
		index_mutex.writelock(__FUNCTION__, __LINE__);
		if (player_spawn_id_map.count(index))
			player_spawn_id_map[index] = spawn;
		else
			player_spawn_id_map[index] = spawn;
		index_mutex.releasewritelock(__FUNCTION__, __LINE__);
	}

	int16 SetSpawnMapAndIndex(Spawn* spawn);

	PacketStruct*	GetQuestJournalPacket(bool all_quests, int16 version, int32 crc, int32 current_quest_id, bool updated = true);
	void			RemoveQuest(int32 id, bool delete_quest);
	vector<Quest*>* CheckQuestsChatUpdate(Spawn* spawn);
	vector<Quest*>* CheckQuestsItemUpdate(Item* item);
	vector<Quest*>* CheckQuestsLocationUpdate();
	vector<Quest*>* CheckQuestsKillUpdate(Spawn* spawn,bool update = true, bool killer_in_encounter = true);
	bool HasQuestUpdateRequirement(Spawn* spawn);
	vector<Quest*>* CheckQuestsSpellUpdate(Spell* spell);
	void CheckQuestsCraftUpdate(Item* item, int32 qty);
	void CheckQuestsHarvestUpdate(Item* item, int32 qty);
	vector<Quest*>* CheckQuestsFailures();
	bool CheckQuestRemoveFlag(Spawn* spawn);
	int8 CheckQuestFlag(Spawn* spawn);
	bool UpdateQuestReward(int32 quest_id, QuestRewardData* qrd);
	Quest* PendingQuestAcceptance(int32 quest_id, int32 item_id, bool* quest_exists);
	bool AcceptQuestReward(int32 item_id, int32 selectable_item_id);
	
	bool SendQuestStepUpdate(int32 quest_id, int32 quest_step_id, bool display_quest_helper);
	void SendQuest(int32 quest_id);
	void UpdateQuestCompleteCount(int32 quest_id);
	void GetQuestTemporaryRewards(int32 quest_id, std::vector<Item*>* items);
	void AddQuestTemporaryReward(int32 quest_id, int32 item_id, int16 item_count);
	
	bool CheckQuestRequired(Spawn* spawn);
	void AddQuestRequiredSpawn(Spawn* spawn, int32 quest_id);
	void AddHistoryRequiredSpawn(Spawn* spawn, int32 event_id);
	int16				spawn_index;
	int32				spawn_id;
	int8 tutorial_step;
	map<int32, vector<int32>*>  player_spawn_quests_required;
	map<int32, vector<int32>*>   player_spawn_history_required;
	Mutex				m_playerSpawnQuestsRequired;
	Mutex				m_playerSpawnHistoryRequired;
	bool	HasQuestBeenCompleted(int32 quest_id);
	int32	GetQuestCompletedCount(int32 quest_id);
	void	AddCompletedQuest(Quest* quest);
	bool	HasActiveQuest(int32 quest_id);
	bool	HasAnyQuest(int32 quest_id);
	map<int32, Quest*>	pending_quests;
	map<int32, Quest*>	player_quests;
	map<int32, Quest*>*	GetPlayerQuests();
	map<int32, Quest*>*	GetCompletedPlayerQuests();
	void				SetFactionValue(int32 faction_id, sint32 value){
		factions.SetFactionValue(faction_id, value);
	}
	PlayerFaction*		GetFactions(){
		return &factions;
	}
	vector<int32> GetQuestIDs();
	map<int32, int16> macro_icons;

	bool				HasPendingLootItems(int32 id);
	bool				HasPendingLootItem(int32 id, int32 item_id);
	vector<Item*>*		GetPendingLootItems(int32 id);
	void				RemovePendingLootItem(int32 id, int32 item_id);
	void				RemovePendingLootItems(int32 id);
	void				AddPendingLootItems(int32 id, vector<Item*>* items);
	int16				GetTierUp(int16 tier);
	bool				HasSpell(int32 spell_id, int8 tier = 255, bool include_higher_tiers = false, bool include_possible_scribe = false);
	bool				HasRecipeBook(int32 recipe_id);
	void				AddPlayerStatistic(int32 stat_id, sint32 stat_value, int32 stat_date);
	void				UpdatePlayerStatistic(int32 stat_id, sint32 stat_value, bool overwrite = false);
	sint64				GetPlayerStatisticValue(int32 stat_id);
	void				WritePlayerStatistics();



	//PlayerGroup*		GetGroup();
	void				SetGroup(PlayerGroup* group);
	bool				IsGroupMember(Entity* player);
	void				SetGroupInformation(PacketStruct* packet);


	void				ResetSavedSpawns();
	bool				IsReturningFromLD();
	void				SetReturningFromLD(bool val);
	bool				CheckLevelStatus(int16 new_level);
	int16				GetLastMovementActivity();
	void				DestroyQuests();
	string				GetAwayMessage() const { return away_message; }
	void				SetAwayMessage(string val) { away_message = val; }
	void				SetRangeAttack(bool val);
	bool				GetRangeAttack();
	bool				AddMail(Mail* mail);
	MutexMap<int32, Mail*>*	GetMail();
	Mail*				GetMail(int32 mail_id);
	void				DeleteMail(bool from_database = false);
	void				DeleteMail(int32 mail_id, bool from_database = false);
	CharacterInstances*	GetCharacterInstances() { return &character_instances; }
	void				SetIsTracking(bool val) { is_tracking = val; }
	bool				GetIsTracking() const { return is_tracking; }
	void				SetBiography(string new_biography) { biography = new_biography; }
	string				GetBiography() const { return biography; }
	void				SetPlayerAdventureClass(int8 new_class, bool set_by_gm_command = false);
	void				SetGuild(Guild* new_guild) { guild = new_guild; }
	Guild*				GetGuild() { return guild; }
	void				AddSkillBonus(int32 spell_id, int32 skill_id, float value);
	SkillBonus*			GetSkillBonus(int32 spell_id);
	virtual void		RemoveSkillBonus(int32 spell_id);

	virtual bool		CanSeeInvis(Entity* target);
	bool				CheckChangeInvisHistory(Entity* target);
	void				UpdateTargetInvisHistory(int32 targetID, bool canSeeStatus);
	void				RemoveTargetInvisHistory(int32 targetID);

	bool				HasFreeBankSlot();
	int8				FindFreeBankSlot();
	PlayerCollectionList * GetCollectionList() { return &collection_list; }
	PlayerRecipeList *	GetRecipeList() { return &recipe_list; }
	PlayerRecipeBookList * GetRecipeBookList() { return &recipebook_list; }
	PlayerAchievementList * GetAchievementList() { return &achievement_list; }
	PlayerAchievementUpdateList * GetAchievementUpdateList() { return &achievement_update_list; }
	void				SetPendingCollectionReward(Collection *collection) { pending_collection_reward = collection; }
	Collection *		GetPendingCollectionReward() { return pending_collection_reward; }
	void AddPendingSelectableItemReward(int32 source_id, Item* item) {
		if (pending_selectable_item_rewards.count(source_id) == 0)
			pending_selectable_item_rewards[source_id] = vector<Item*>();
		pending_selectable_item_rewards[source_id].push_back(item);
	}
	void AddPendingItemReward(Item* item) { 
		pending_item_rewards.push_back(item);
	}
	bool HasPendingItemRewards() { return (pending_item_rewards.size() > 0 || pending_selectable_item_rewards.size() > 0); }
	vector<Item*> GetPendingItemRewards() { return pending_item_rewards; }
	map<int32, Item*> GetPendingSelectableItemReward(int32 item_id) { //since the client sends the selected item id, we need to have the associated source and remove all of them.  Yes, there is an edge case if multiple sources have the same Item in them, but limited on what the client sends (just a single item id)
		map<int32, Item*> ret;
		if (pending_selectable_item_rewards.size() > 0) {
			map<int32, vector<Item*>>::iterator map_itr;
			for (map_itr = pending_selectable_item_rewards.begin(); map_itr != pending_selectable_item_rewards.end(); map_itr++) {
				vector<Item*>::iterator itr;
				for (itr = map_itr->second.begin(); itr != map_itr->second.end(); itr++) {
					if ((*itr)->details.item_id == item_id) {
						ret[map_itr->first] = *itr;
						break;
					}
				}
				if (ret.size() > 0)
					break;
			}			
		}
		return map<int32, Item*>();
	}
	void ClearPendingSelectableItemRewards(int32 source_id, bool all = false) { 
		if (pending_selectable_item_rewards.size() > 0) {
			map<int32, vector<Item*>>::iterator map_itr;
			if (all) {
				for (map_itr = pending_selectable_item_rewards.begin(); map_itr != pending_selectable_item_rewards.end(); map_itr++) {
					vector<Item*>::iterator itr;
					for (itr = map_itr->second.begin(); itr != map_itr->second.end(); itr++) {
						safe_delete(*itr);
					}
				}
				pending_selectable_item_rewards.clear();
			}
			else {
				if (pending_selectable_item_rewards.count(source_id) > 0) {
					vector<Item*>::iterator itr;
					for (itr = pending_selectable_item_rewards[source_id].begin(); itr != pending_selectable_item_rewards[source_id].end(); itr++) {
						safe_delete(*itr);
					}
					pending_selectable_item_rewards.erase(source_id);
				}
			}			
		}
	}	
	void ClearPendingItemRewards() { //the client doesn't send any reference to where the pending rewards came from, so if they collect one, we should just them all of them at once
		if (pending_item_rewards.size() > 0) {
			vector<Item*>::iterator itr;
			for (itr = pending_item_rewards.begin(); itr != pending_item_rewards.end(); itr++) {
				safe_delete(*itr);
			}
			pending_item_rewards.clear();
		}
	}
	
	enum DELETE_BOOK_TYPE {
		DELETE_TRADESKILLS = 1,
		DELETE_SPELLS = 2,
		DELETE_COMBAT_ART = 4,
		DELETE_ABILITY = 8,
		DELETE_NOT_SHOWN = 16
	};
	void DeleteSpellBook(int8 type_selection = 0);
	void RemoveSpellBookEntry(int32 spell_id, bool remove_passives_from_list = true);
	void ResortSpellBook(int32 sort_by, int32 order, int32 pattern, int32 maxlvl_only, int32 book_type);
	void GetSpellBookSlotSort(int32 pattern, int32* i, int8* page_book_count, int32* last_start_point);
	static bool SortSpellEntryByName(SpellBookEntry* s1, SpellBookEntry* s2);
	static bool SortSpellEntryByCategory(SpellBookEntry* s1, SpellBookEntry* s2);
	static bool SortSpellEntryByLevel(SpellBookEntry* s1, SpellBookEntry* s2);
	static bool SortSpellEntryByNameReverse(SpellBookEntry* s1, SpellBookEntry* s2);
	static bool SortSpellEntryByCategoryReverse(SpellBookEntry* s1, SpellBookEntry* s2);
	static bool SortSpellEntryByLevelReverse(SpellBookEntry* s1, SpellBookEntry* s2);

	int8 GetSpellSlot(int32 spell_id);
	void				AddTitle(sint32 title_id, const char *name, int8 prefix, bool save_needed = false);
	void				AddAAEntry(int16 template_id, int8 tab_id, int32 aa_id, int16 order, int8 treeid);
	PlayerTitlesList*	GetPlayerTitles() { return &player_titles_list; }
	void				AddLanguage(int32 id, const char *name, bool save_needed = false);
	PlayerLanguagesList* GetPlayerLanguages() { return &player_languages_list; }
	bool				HasLanguage(int32 id);
	bool				HasLanguage(const char* name);
	bool                CanReceiveQuest(int32 quest_id, int8* ret = 0);
	float               GetBoatX() { if (info) return info->GetBoatX(); return 0; }
	float               GetBoatY() { if (info) return info->GetBoatY(); return 0; }
	float               GetBoatZ() { if (info) return info->GetBoatZ(); return 0; }
	int32               GetBoatSpawn() { if (info) return info->GetBoatSpawn(); return 0; }
	void                SetBoatX(float x) { if (info) info->SetBoatX(x); }
	void                SetBoatY(float y) { if (info) info->SetBoatY(y); }
	void                SetBoatZ(float z) { if (info) info->SetBoatZ(z); }
	void                SetBoatSpawn(Spawn* boat) { if (info) info->SetBoatSpawn(boat); }
	Mutex*              GetGroupBuffMutex();
	void                SetPendingDeletion(bool val) { pending_deletion = val; }
	bool                GetPendingDeletion() { return pending_deletion; }
	float               GetPosPacketSpeed() { return pos_packet_speed; }
	bool                ControlFlagsChanged();
	void                SetPlayerControlFlag(int8 param, int8 param_value, bool is_active);
	void                SendControlFlagUpdates(Client* client);

	/// <summary>Casts all the passive spells for the player, only call after zoning is complete.</summary>
	void ApplyPassiveSpells();

	/// <summary>Removes all passive spell effects from the player and clears the passive list</summary>
	void RemoveAllPassives();

	/// <summary>Gets the current recipie ID</summary>
	int32 GetCurrentRecipe() { return current_recipe; }

	/// <summary>Sets the current recipie ID</summary>
	/// <param name="val">Id of the new recipe</param>
	void SetCurrentRecipe(int32 val) { current_recipe = val; }

	/// <summary>Reset the pet window info</summary>
	void ResetPetInfo();

	void ProcessCombat();

	/* Character history stuff */

	/// <summary>Adds a new history event to the player</summary>
	/// <param name="type">The history type</param>
	/// <param name="subtype">The history sub type</param>
	/// <param name="value">The first history value</param>
	/// <param name="value2">The second history value</param>
	void UpdatePlayerHistory(int8 type, int8 subtype, int32 value, int32 value2 = 0);

	/// <summary>Checks to see if the player has discovered the location</summary>
	/// <param name="locationID">The ID of the location to check</param>
	/// <returns>True if the player has discovered the location</returns>
	bool DiscoveredLocation(int32 locationID);

	/// <summary>Load the players history from the database</summary>
	/// <param name="type">The history type</param>
	/// <param name="subtype">The history sub type</param>
	/// <param name="hd">The history data</param>
	void LoadPlayerHistory(int8 type, int8 subtype, HistoryData* hd);

	/// <summary>Save the player's history to the database</summary>
	void SaveHistory();


	/* New functions for spell locking and unlocking*/
	/// <summary>Lock all Spells, Combat arts, and Abilities (not trade skill spells)</summary>
	void LockAllSpells();

	/// <summary>Unlocks all Spells, Combat arts, and Abilities (not trade skill spells)</summary>
	void UnlockAllSpells(bool modify_recast = false, Spell* exception = 0);

	/// <summary>Locks the given spell as well as all spells with a shared timer</summary>
	void LockSpell(Spell* spell, int16 recast);

	/// <summary>Unlocks the given spell as well as all spells with shared timers</summary>
	void UnlockSpell(Spell* spell);
	void UnlockSpell(int32 spell_id, int32 linked_timer_id);

	/// <summary>Locks all ts spells and unlocks all normal spells</summary>
	void LockTSSpells();

	/// <summary>Unlocks all ts spells and locks all normal spells</summary>
	void UnlockTSSpells();

	/// <summary>Queue the given spell</summary>
	void QueueSpell(Spell* spell);

	/// <summary>Unqueue the given spell</summary>
	void UnQueueSpell(Spell* spell);

	///<summary>Get all the spells the player has with the given id</summary>
	vector<Spell*> GetSpellBookSpellsByTimer(Spell* spell, int32 timerID);

	PacketStruct* GetQuestJournalPacket(Quest* quest, int16 version, int32 crc, bool updated = true);

	void SetSpawnInfoStruct(PacketStruct* packet) { safe_delete(spawn_info_struct); spawn_info_struct = packet; }
	void SetSpawnVisStruct(PacketStruct* packet) { safe_delete(spawn_vis_struct); spawn_vis_struct = packet; }
	void SetSpawnPosStruct(PacketStruct* packet) { safe_delete(spawn_pos_struct); spawn_pos_struct = packet; }
	void SetSpawnHeaderStruct(PacketStruct* packet) { safe_delete(spawn_header_struct); spawn_header_struct = packet; }
	void SetSpawnFooterStruct(PacketStruct* packet) { safe_delete(spawn_footer_struct); spawn_footer_struct = packet; }
	void SetSignFooterStruct(PacketStruct* packet) { safe_delete(sign_footer_struct); sign_footer_struct = packet; }
	void SetWidgetFooterStruct(PacketStruct* packet) { safe_delete(widget_footer_struct); widget_footer_struct = packet; }

	PacketStruct* GetSpawnInfoStruct() { return spawn_info_struct; }
	PacketStruct* GetSpawnVisStruct() { return spawn_vis_struct; }
	PacketStruct* GetSpawnPosStruct() { return spawn_pos_struct; }
	PacketStruct* GetSpawnHeaderStruct() { return spawn_header_struct; }
	PacketStruct* GetSpawnFooterStruct() { return spawn_footer_struct; }
	PacketStruct* GetSignFooterStruct() { return sign_footer_struct; }
	PacketStruct* GetWidgetFooterStruct() { return widget_footer_struct; }

	Mutex info_mutex;
	Mutex pos_mutex;
	Mutex vis_mutex;
	Mutex index_mutex;
	Mutex spawn_mutex;
	mutable std::shared_mutex spawn_aggro_range_mutex;

	void SetTempMount(int32 id) { tmp_mount_model = id; }
	int32 GetTempMount() { return tmp_mount_model; }

	void SetTempMountColor(EQ2_Color* color) { tmp_mount_color = *color; }
	EQ2_Color GetTempMountColor() { return tmp_mount_color; }

	void SetTempMountSaddleColor(EQ2_Color* color) { tmp_mount_saddle_color = *color; }
	EQ2_Color GetTempMountSaddleColor() { return tmp_mount_saddle_color; }


	void LoadLUAHistory(int32 event_id, LUAHistory* history);
	void SaveLUAHistory();
	void UpdateLUAHistory(int32 event_id, int32 value, int32 value2);
	LUAHistory* GetLUAHistory(int32 event_id);

	bool HasGMVision() { return gm_vision; }
	void SetGMVision(bool val) { gm_vision = val; }

	void StopCombat(int8 type=0) { 
		switch(type)
		{
			case 2:
				SetRangeAttack(false);
				InCombat(false, true);
			break;
			default:
				InCombat(false);
				InCombat(false, true);
				SetRangeAttack(false);
			break;
		}
	}

	NPC* InstantiateSpiritShard(float origX, float origY, float origZ, float origHeading, int32 origGridID, ZoneServer* origZone);

	void DismissAllPets();

	void SaveSpellEffects();
	void SaveCustomSpellFields(LuaSpell* luaspell);
	void SaveCustomSpellDataIndex(LuaSpell* luaspell);
	void SaveCustomSpellEffectsDisplay(LuaSpell* luaspell);
	
	void SetSaveSpellEffects(bool val) { stop_save_spell_effects = val; }
	AppearanceData SavedApp;
	CharFeatures SavedFeatures;
	bool custNPC;
	Entity* custNPCTarget;
	// bot index, spawn id
	map<int32, int32> SpawnedBots;
	bool StopSaveSpellEffects() { return stop_save_spell_effects; }

	void MentorTarget();
	void SetMentorStats(int32 effective_level, int32 target_char_id = 0, bool update_stats = true);

	bool ResetMentorship() { 
		bool mentorship_status = reset_mentorship;
		if(mentorship_status)
		{
			SetMentorStats(GetLevel());
		}
		reset_mentorship = false;
		return mentorship_status;
	}

	void EnableResetMentorship() {
		reset_mentorship = true;
	}
	
	bool SerializeItemPackets(EquipmentItemList* equipList, vector<EQ2Packet*>* packets, Item* item, int16 version, Item* to_item = 0);

	void AddGMVisualFilter(int32 filter_type, int32 filter_value, char* filter_search_str, int16 visual_tag);
	int16 MatchGMVisualFilter(int32 filter_type, int32 filter_value, char* filter_search_str, bool in_vismutex_lock = false);
	void ClearGMVisualFilters();
	int GetPVPAlignment();
	
	int32	GetCurrentLanguage() { return current_language_id; }
	void	SetCurrentLanguage(int32 language_id) { current_language_id = language_id; }
	
	void	SetActiveReward(bool val) { active_reward = val; }
	bool	IsActiveReward() { return active_reward; }
	
	
	bool	IsSpawnInRangeList(int32 spawn_id);
	void	SetSpawnInRangeList(int32 spawn_id, bool in_range);
	void	ProcessSpawnRangeUpdates();
	void	CalculatePlayerHPPower(int16 new_level = 0);
	bool	IsAllowedCombatEquip(int8 slot = 255, bool send_message = false);
	
	void	SetActiveFoodUniqueID(int32 unique_id, bool update_db = true);
	void	SetActiveDrinkUniqueID(int32 unique_id, bool update_db = true);
	
	int64	GetActiveFoodUniqueID() { return active_food_unique_id; }
	int64	GetActiveDrinkUniqueID() { return active_drink_unique_id; }
	
	void	SetHouseVaultSlots(int8 allowed_slots) { house_vault_slots = allowed_slots; }
	int8	GetHouseVaultSlots() { return house_vault_slots; }
	
	Mutex MPlayerQuests;
	float   pos_packet_speed;
	
	map <int8, map <int8, vector<TraitData*> > >* SortedTraitList;
	map <int8, vector<TraitData*> >* ClassTraining;
	map <int8, vector<TraitData*> >* RaceTraits;
	map <int8, vector<TraitData*> >* InnateRaceTraits;
	map <int8, vector<TraitData*> >* FocusEffects;
	mutable std::shared_mutex trait_mutex;
	std::atomic<bool> need_trait_update;

	static void InitXPTable();
	static map<int8, int32> m_levelXPReq;

	mutable std::shared_mutex spell_packet_update_mutex;
	mutable std::shared_mutex raid_update_mutex;
private:
	bool reset_mentorship;
	bool range_attack;
	int16 last_movement_activity;
	bool returning_from_ld;
	PlayerGroup* group;
	
	float test_x;
	float test_y;
	float test_z;
	int32 test_time;
	map<int32, map<int32, bool> >	pending_loot_items;
	Mutex				MSpellsBook;
	Mutex				MRecipeBook;
	map<Spawn*, bool>	current_quest_flagged;
	PlayerFaction		factions;
	map<int32, Quest*>	completed_quests;
	std::atomic<bool>	charsheet_changed;
	std::atomic<bool>	raidsheet_changed;
	std::atomic<bool>	hassent_raid;
	map<int32, string>	spawn_vis_packet_list;
	map<int32, string>	spawn_info_packet_list;
	map<int32, string>	spawn_pos_packet_list;
	map<int32, int8> spawn_packet_sent;
	map<int32, SpawnQueueState*> spawn_state_list;
	uchar*				movement_packet;
	uchar*				old_movement_packet;
	uchar*				spell_orig_packet;
	uchar*				spell_xor_packet;
	int16				spell_count;
	
	uchar*				raid_orig_packet;
	uchar*				raid_xor_packet;
	//float				speed;
	int16				target_id;
	Spawn*              combat_target;
	int32				char_id;
	bool				quickbar_updated;
	bool				resurrecting;
	PlayerInfo*			info;
	vector<SpellBookEntry*> spells;
	vector<QuickBarItem*> quickbar_items;
	map<int32, Statistic*> statistics;
	void				RemovePlayerStatistics();
	map<string, int8>	friend_list;
	map<string, int8>	ignore_list;
	bool                pending_deletion;
	PlayerControlFlags  control_flags;

	map<int32, bool>	target_invis_history;

	// JA: POI Discoveries
	map<int32, vector<int32> >	players_poi_list;

	// Jabantiz: Passive spell list, just stores spell id's
	vector<int32>		passive_spells;

	/// <summary>Adds a new passive spell to the list</summary>
	/// <param name='id'>Spell id to add</param>
	/// <param name='tier'>Tier of spell to add</param>
	void AddPassiveSpell(int32 id, int8 tier);

	/// <summary>Removes a passive spell from the list</summary>
	/// <param name='id'>Spell id to remove</param>
	/// <param name='tier'>Tier of spell to remove</param>
	/// <param name='remove_from_list'>Remove the spell from this players passive list, default true</param>
	void RemovePassive(int32 id, int8 tier, bool remove_from_list = true);

	CharacterInstances	character_instances;
	string				away_message;
	string				biography;
	MutexMap<int32, Mail*> mail_list;
	bool				is_tracking;
	Guild*				guild;
	PlayerCollectionList collection_list;
	Collection *		pending_collection_reward;
	vector<Item*>		pending_item_rewards;
	map<int32, vector<Item*>>		pending_selectable_item_rewards;
	PlayerTitlesList	player_titles_list;
	PlayerRecipeList	recipe_list;
	PlayerLanguagesList	player_languages_list;
	PlayerRecipeBookList recipebook_list;
	PlayerAchievementList achievement_list;
	PlayerAchievementUpdateList achievement_update_list;
	// Need to keep track of the recipe the player is crafting as not all crafting packets have this info
	int32				current_recipe;

	void HandleHistoryNone(int8 subtype, int32 value, int32 value2);
	void HandleHistoryDeath(int8 subtype, int32 value, int32 value2);
	void HandleHistoryDiscovery(int8 subtype, int32 value, int32 value2);
	void HandleHistoryXP(int8 subtype, int32 value, int32 value2);

	/// <summary></summary>
	void ModifySpellStatus(SpellBookEntry* spell, sint16 value, bool modify_recast = true, int16 recast = 0);
	void AddSpellStatus(SpellBookEntry* spell, sint16 value, bool modify_recast = true, int16 recast = 0);
	void RemoveSpellStatus(SpellBookEntry* spell, sint16 value, bool modify_recast = true, int16 recast = 0);
	void SetSpellEntryRecast(SpellBookEntry* spell, bool modify_recast, int16 recast);

	//The following variables are for serializing spawn packets
	PacketStruct* spawn_pos_struct;
	PacketStruct* spawn_info_struct;
	PacketStruct* spawn_vis_struct;
	PacketStruct* spawn_header_struct;
	PacketStruct* spawn_footer_struct;
	PacketStruct* sign_footer_struct;
	PacketStruct* widget_footer_struct;
	uchar*		  spawn_tmp_vis_xor_packet;
	uchar*		  spawn_tmp_pos_xor_packet;
	uchar*		  spawn_tmp_info_xor_packet;
	int32 vis_xor_size;
	int32 pos_xor_size;
	int32 info_xor_size;

	// Character history, map<type, map<subtype, vector<data> > >
	map<int8, map<int8, vector<HistoryData*> > > m_characterHistory;

	map<int32, LUAHistory*> m_charLuaHistory;
	Mutex mLUAHistory;

	int32 tmp_mount_model;
	EQ2_Color tmp_mount_color;
	EQ2_Color tmp_mount_saddle_color;

	bool gm_vision;
	bool stop_save_spell_effects;

	map<int32, Spawn*>	player_spawn_id_map;
	map<Spawn*, int32>	player_spawn_reverse_id_map;
	map<int32, bool>	player_aggro_range_spawns;

	bool all_spells_locked;
	Timer lift_cooldown;

	vector<GMTagFilter> gm_visual_filters;
	
	int32 current_language_id;
	
	bool active_reward;
	
	Quest*  GetAnyQuest(int32 quest_id);
	Quest*	GetCompletedQuest(int32 quest_id);
	
	std::atomic<int64> active_food_unique_id;
	std::atomic<int64> active_drink_unique_id;
	
	int8 house_vault_slots;
};
#pragma pack()
#endif
