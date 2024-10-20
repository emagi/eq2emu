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
#ifndef CLIENT_H
#define CLIENT_H

#include <list>
#include <atomic>
#include <mutex>
#include <shared_mutex>

#include "../common/EQStream.h"
#include "../common/timer.h"
#include "Items/Items.h"
#include "zoneserver.h"
#include "Player.h"
#include "Quests.h"

using namespace std;
#define CLIENT_TIMEOUT 60000
struct TransportDestination;
struct ConversationOption;
struct VoiceOverStruct;

#define MAIL_SEND_RESULT_SUCCESS				0
#define MAIL_SEND_RESULT_UNKNOWN_PLAYER			1
#define MAIL_SEND_RESULT_CANNOT_SEND_TO_PLAYER	2
#define MAIL_SEND_RESULT_GIFT_WRONG_SERVER		3	/* Cannot send gifts across worlds */
#define MAIL_SEND_RESULT_CANNOT_SEND_TO_SELF	4
#define MAIL_SEND_RESULT_MAILBOX_FULL			5
#define MAIL_SEND_RESULT_NOT_ENOUGH_COIN		6
#define MAIL_SEND_RESULT_ITEM_IN_BAG			7	/* Cannot send non-empty bags as gifts */
#define MAIL_SEND_RESULT_NOT_IN_GUILD			8
#define MAIL_SEND_RESULT_GUILD_ACCESS_DENIED	9
#define MAIL_SEND_RESULT_GIFTS_TO_GUILD			10	/* Cannot send gifts to entire guild */
#define MAIL_SEND_RESULT_EMPTY_TO_LIST			11	/* Empty recipient list */
#define MAIL_SEND_RESULT_TRIAL_PLAYERS			12	/* Cannot send mail to trial players */
#define MAIL_SEND_RESULT_MAIL_WRONG_SERVER		13	/* Cannot send mail across worlds */
#define MAIL_SEND_RESULT_UNKNOWN_ERROR			14

#define MAIL_TYPE_REGULAR	0
#define MAIL_TYPE_SPAM		1
#define MAIL_TYPE_GM		2

struct QueuedQuest{
	int32 quest_id;
	int32 step;
	bool display_quest_helper;
};

struct BuyBackItem{
	int32	item_id;
	int32	unique_id;
	int16	quantity;
	int32	price;
	bool	save_needed;
};

struct MacroData{
	string	name;
	string	text;
	int16	icon;
};

struct Mail {
	int32	mail_id;
	int32	player_to_id;
	string	player_from;
	string	subject;
	string	mail_body;
	int8	already_read;
	int8	mail_type;
	int32	coin_copper;
	int32	coin_silver;
	int32	coin_gold;
	int32	coin_plat;
	int16	stack;
	int32	postage_cost;
	int32	attachment_cost;
	int32	char_item_id;
	int32	time_sent;
	int32	expire_time;
	int8	save_needed;
};

struct MailWindow {
	int32	coin_copper;
	int32	coin_silver;
	int32	coin_gold;
	int32	coin_plat;
	Item*	item;
	int32	char_item_id;
	int32	stack;
};

struct PendingGuildInvite {
	Guild* guild;
	Player* invited_by;
};

struct PendingResurrection {
	Spawn* caster;
	Timer* expire_timer;
	string spell_name;
	string heal_name;
	bool active;
	float hp_perc;
	float mp_perc;
	float range;
	int8 crit_mod;
	bool no_calcs;
	int32 subspell;
	bool crit;
	bool should_delete;
	int32 spell_visual;
};

#define PAPERDOLL_TYPE_FULL 0
#define PAPERDOLL_TYPE_HEAD 1

struct IncomingPaperdollImage {
	uchar* image_bytes;
	int32 current_size_bytes;
	int8 image_num_packets;
	int8 last_received_packet_index;
	int8 image_type;
};
struct WaypointInfo {
	int32 id;
	int8 type;
};


class Client {
public:
	Client(EQStream* ieqs);
    ~Client();
	
	void	RemoveClientFromZone();
	bool	Process(bool zone_process = false);
	void	Disconnect(bool send_disconnect = true);
	void	SetConnected(bool val){ connected = val; }
	bool	IsConnected(){ return connected; }
	bool	IsReadyForSpawns(){ return ready_for_spawns; }
	bool	IsReadyForUpdates() { return ready_for_updates; }
	bool	IsZoning(){ return client_zoning; }
	void	SetReadyForUpdates();
	void	SetReadyForSpawns(bool val);
	void	QueuePacket(EQ2Packet* app, bool attemptedCombine=false);
	void	SendLoginInfo();
	int8	GetMessageChannelColor(int8 channel_type);
	void	HandleTellMessage(Client* from, const char* message, const char* to, int32 current_language_id);
	void	SimpleMessage(int8 color, const char* message);
	void	Message(int8 type, const char* message, ...);
	void	SendSpellUpdate(Spell* spell, bool add_silently = false, bool add_to_hotbar = true);
	void	Zone(ZoneServer* new_zone, bool set_coords = true, bool is_spell = false);
	void	Zone(const char* new_zone, bool set_coords = true, bool is_spell = false);
	void	Zone(int32 instanceid, bool set_coords = true, bool byInstanceID=false, bool is_spell = false);
	void	SendZoneInfo();
	void	SendZoneSpawns();
	void	HandleVerbRequest(EQApplicationPacket* app);
	void	SendControlGhost(int32 send_id=0xFFFFFFFF, int8 unknown2=0);
	void	SendCharInfo();
	void	SendLoginDeniedBadVersion();
	void	SendCharPOVGhost();
	void	SendPlayerDeathWindow();
	float	DistanceFrom(Client* client);
	void	SendDefaultGroupOptions();
	bool	HandleLootItemByID(Spawn* entity, int32 item_id, Spawn* target);
	bool	HandleLootItem(Spawn* entity, Item* item, Spawn* target=nullptr, bool overrideLootRestrictions = false);
	void	HandleLootItemRequestPacket(EQApplicationPacket* app);
	void	HandleSkillInfoRequest(EQApplicationPacket* app);
	void	HandleExamineInfoRequest(EQApplicationPacket* app);
	void	HandleQuickbarUpdateRequest(EQApplicationPacket* app);
	void	SendPopupMessage(int8 unknown, const char* text, const char* type, float size, int8 red, int8 green, int8 blue);	
	void	PopulateSkillMap();
	void	ChangeLevel(int16 old_level, int16 new_level);
	void	ChangeTSLevel(int16 old_level, int16 new_level);
	bool	Summon(const char* search_name);
	std::string	IdentifyInstanceLockout(int32 zoneID, bool displayClient = true);
	ZoneServer*	IdentifyInstance(int32 zoneID);
	bool	TryZoneInstance(int32 zoneID, bool zone_coords_valid=false);
	bool	GotoSpawn(const char* search_name, bool forceTarget=false);
	void	DisplayDeadWindow();
	void	HandlePlayerRevive(int32 point_id);
	void	Bank(Spawn* banker, bool cancel = false);
	void	BankWithdrawal(int64 amount);
	bool    BankWithdrawalNoBanker(int64 amount);
	bool    BankHasCoin(int64 amount);
	void	BankDeposit(int64 amount);
	Spawn*	GetBanker();
	void	SetBanker(Spawn* in_banker);
	bool	AddItem(int32 item_id, int16 quantity = 0, AddItemType type = AddItemType::NOT_SET);
	bool	AddItem(Item* item, bool* item_deleted = 0, AddItemType type = AddItemType::NOT_SET);
	bool	AddItemToBank(int32 item_id, int16 quantity = 0);
	bool	AddItemToBank(Item* item);
	void	UnequipItem(int16 index, sint32 bag_id = -999, int8 to_slot = 255, int8 appearance_equip = 0);
	bool	RemoveItem(Item *item, int16 quantity, bool force_override_no_delete = false);
	void	ProcessTeleport(Spawn* spawn, vector<TransportDestination*>* destinations, int32 transport_id = 0, bool is_spell = false);
	void	ProcessTeleportLocation(EQApplicationPacket* app); 

	void	UpdateCharacterInstances();
	void	SetLastSavedTimeStamp(int32 unixts) { last_saved_timestamp = unixts; }
	int32	GetLastSavedTimeStamp() { return last_saved_timestamp; }

	bool	CheckZoneAccess(const char* zoneName);
	
	ZoneServer* GetCurrentZone();
	int32 GetCurrentZoneID();
	void	SetCurrentZoneByInstanceID(int32 id, int32 zoneid);
	//void	SetCurrentZoneByInstanceID(instanceid, zoneid);
	void	SetCurrentZone(int32 id);
	void	SetCurrentZone(ZoneServer* zone);
	void	SetZoningDestination(ZoneServer* zone) {
		zoning_destination = zone;
	}
	ZoneServer* GetZoningDestination() { return zoning_destination; }
	Player*	GetPlayer(){ return player; }
	EQStream*	getConnection(){ return eqs; }
	void	setConnection(EQStream* ieqs){ eqs = ieqs; }

	inline int32		GetIP()				{ return ip; }
	inline int16		GetPort()			{ return port; }
	inline int32		WaitingForBootup()	{ return pwaitingforbootup; }
	inline int32		GetCharacterID()		{ return character_id; }
	inline int32		GetAccountID()			{ return account_id; }
	inline const char*	GetAccountName()		{ return account_name; }
	inline sint16		GetAdminStatus()		{ return admin_status; }
	inline int16		GetVersion()			{ return version; }
	void SetNameCRC(int32 val){ name_crc = val; }
	int32 GetNameCRC(){ return name_crc; }


	void				SetVersion(int16 new_version){ version = new_version; }
	void				SetAccountID(int32 in_accountid) { account_id = in_accountid; }
	void				SetCharacterID(int32 in_characterid) { character_id = in_characterid; }
	void				SetAdminStatus(sint16 in_status) { admin_status = in_status; }
	

	void	DetermineCharacterUpdates ( );

	void	UpdateTimeStampFlag ( int8 flagType )
	{
		if(! (timestamp_flag & flagType ) )
		timestamp_flag |= flagType;
	}

	int8	GetTimeStampFlag ( ) { return timestamp_flag; }
	bool	UpdateQuickbarNeeded();
	void	Save();
	bool	remove_from_list;
	void	CloseLoot(int32 spawn_id);
	void	SendLootResponsePacket(int32 total_coins, vector<Item*>* items, Spawn* entity, bool ignore_loot_tier = false);
	void	LootSpawnRequest(Spawn* entity, bool attemptDisarm=true);
	bool	LootSpawnByMethod(Spawn* entity);
	void	OpenChest(Spawn* entity, bool attemptDisarm=true);
	void	CastGroupOrSelf(Entity* source, uint32 spellID, uint32 spellTier=1, float restrictiveRadius=0.0f);
	void	CheckPlayerQuestsKillUpdate(Spawn* spawn);
	void	CheckPlayerQuestsChatUpdate(Spawn* spawn);
	void	CheckPlayerQuestsItemUpdate(Item* item);
	void	CheckPlayerQuestsSpellUpdate(Spell* spell);
	void	CheckPlayerQuestsLocationUpdate();
	void	AddPendingQuest(Quest* quest, bool forced = false);
	void	AcceptQuest(int32 quest_id);
	void	RemovePendingQuest(int32 quest_id);
	void	SetPlayerQuest(Quest* quest, map<int32, int32>* progress);
	void	AddPlayerQuest(Quest* quest, bool call_accepted = true, bool send_packets = true);
	void	RemovePlayerQuest(int32 id, bool send_update = true, bool delete_quest = true);
	void	SendQuestJournal(bool all_quests = false, Client* client = 0, bool updated = true);
	void	SendQuestUpdate(Quest* quest);
	void	SendQuestFailure(Quest* quest);
	void	SendQuestUpdateStep(Quest* quest, int32 step, bool display_quest_helper = true);
	void	SendQuestUpdateStepImmediately(Quest* quest, int32 step, bool display_quest_helper = true);
	void	DisplayQuestRewards(Quest* quest, int64 coin, vector<Item*>* rewards=0, vector<Item*>* selectable_rewards=0, map<int32, sint32>* factions=0, const char* header="Quest Reward!", int32 status_points=0, const char* text=0, bool was_displayed = false);
	void	PopulateQuestRewardItems(vector <Item*>* items, PacketStruct* packet, std::string num_rewards_str = "num_rewards", std::string reward_id_str = "reward_id" , std::string item_str = "item");
	void	DisplayQuestComplete(Quest* quest, bool tempReward = false, std::string customDescription = string(""), bool was_displayed = false);
	void	DisplayRandomizeFeatures(int32 features);
	void	AcceptQuestReward(Quest* quest, int32 item_id);
	Quest*	GetPendingQuestAcceptance(int32 item_id);
	void	DisplayConversation(int32 conversation_id, int32 spawn_id, vector<ConversationOption>* conversations, const char* text, const char* mp3, int32 key1, int32 key2, int8 language = 0, int8 can_close = 1);
	void	DisplayConversation(Item* item, vector<ConversationOption>* conversations, const char* text, int8 type, const char* mp3 = 0, int32 key1 = 0, int32 key2 = 0, int8 language = 0, int8 can_close = 1);
	void	DisplayConversation(Spawn* src, int8 type, vector<ConversationOption>* conversations, const char* text, const char* mp3 = 0, int32 key1 = 0, int32 key2 = 0, int8 language = 0, int8 can_close = 1);
	void	CloseDialog(int32 conversation_id);
	int32	GetConversationID(Spawn* spawn, Item* item);
	void	CombineSpawns(float radius, Spawn* spawn);
	void	AddCombineSpawn(Spawn* spawn);
	void	RemoveCombineSpawn(Spawn* spawn);
	void	SaveCombineSpawns(const char* name = 0);
	Spawn*	GetCombineSpawn();
	bool	ShouldTarget();
	void	TargetSpawn(Spawn* spawn);
	void	ReloadQuests();
	int32	GetCurrentQuestID(){ return current_quest_id; }
	void	SetLuaDebugClient(bool val);
	void	SetMerchantTransaction(Spawn* spawn);
	Spawn*	GetMerchantTransaction();
	void	SetMailTransaction(Spawn* spawn);
	Spawn*	GetMailTransaction();
	void	PlaySound(const char* name);
	void	SendBuyMerchantList(bool sell = false);
	void	SendSellMerchantList(bool sell = false);
	void	SendBuyBackList(bool sell = false);
	void	SendRepairList();
	void	ShowLottoWindow();
	void	PlayLotto(int32 price, int32 ticket_item_id);
	void	SendGuildCreateWindow();
	float	CalculateBuyMultiplier(int32 merchant_id);
	float	CalculateSellMultiplier(int32 merchant_id);
	void	BuyItem(int32 item_id, int16 quantity);
	void	SellItem(int32 item_id, int16 quantity, int32 unique_id = 0);
	void	BuyBack(int32 item_id, int16 quantity);
	void	RepairItem(int32 item_id);
	void	RepairAllItems();
	void	AddBuyBack(int32 unique_id, int32 item_id, int16 quantity, int32 price, bool save_needed = true);
	deque<BuyBackItem*>*	GetBuyBacks();
	vector<Item*>* GetRepairableItems();
	vector<Item*>* GetItemsByEffectType(ItemEffectType type, ItemEffectType secondary_effect = NO_EFFECT_TYPE);
	void	SendMailList();
	void	DisplayMailMessage(int32 mail_id);
	void	HandleSentMail(EQApplicationPacket* app);
	void	DeleteMail(int32 mail_id, bool from_database = false);
	bool	AddMailItem(Item* item);
	bool	AddMailCoin(int32 copper, int32 silver = 0, int32 gold = 0, int32 plat = 0);
	bool	RemoveMailCoin(int32 copper, int32 silver = 0, int32 gold = 0, int32 plat = 0);
	void	TakeMailAttachments(int32 mail_id);
	void	ResetSendMail(bool cancel = true, bool needslock = true);
	bool	GateAllowed();
	bool	BindAllowed();
	bool	Bind();
	bool	Gate(bool is_spell = false);
	void	SendChatRelationship(int8 type, const char* name);
	void	SendFriendList();
	void	SendIgnoreList();
	void	SendNewAdventureSpells();
	void	SendNewTradeskillSpells();
	string	GetCoinMessage(int32 total_coins);
	void	SetItemSearch(vector<Item*>* items);
	vector<Item*>* GetSearchItems();
	void	SearchStore(int32 page);
	void	SetPlayer(Player* new_player);

	void	AddPendingQuestAcceptReward(Quest* quest);
	void	AddPendingQuestReward(Quest* quest, bool update=true, bool is_temporary = false, std::string description = std::string(""));
	bool	HasQuestRewardQueued(int32 quest_id, bool is_temporary, bool is_collection);
	void	QueueQuestReward(int32 quest_id, bool is_temporary, bool is_collection, bool has_displayed, int64 tmp_coin, int32 tmp_status, std::string description, bool db_saved=false, int32 index=0);
	void	RemoveQueuedQuestReward();
	void	AddPendingQuestUpdate(int32 quest_id, int32 step_id, int32 progress = 0xFFFFFFFF);
	void	ProcessQuestUpdates();	
	void	AddWaypoint(const char* waypoint_name, int8 waypoint_category, int32 spawn_id);
	void	BeginWaypoint(const char* waypoint_name, float x, float y, float z);
	void	InspectPlayer(Player* player_to_inspect);
	void	SetPendingGuildInvite(Guild* guild, Player* invited_by = 0);
	PendingGuildInvite*	GetPendingGuildInvite() {return &pending_guild_invite;}
	void	ShowClaimWindow();
	void	ShowGuildSearchWindow();
	void	CheckQuestQueue();
	void	ShowDressingRoom(Item *item, sint32 crc);
	void	SendCollectionList();
	bool	SendCollectionsForItem(Item *item);
	void	HandleCollectionAddItem(int32 collection_id, Item *item);
	void	DisplayCollectionComplete(Collection *collection);
	void	HandInCollections();
	void	AcceptCollectionRewards(Collection *collection, int32 selectable_item_id = 0);
	void	SendRecipeList();
	void	PopulateRecipeData(Recipe* recipe, PacketStruct* packet, int i=0);
	int32	GetRecipeCRC(Recipe* recipe);
	void	SendRecipeDetails(vector<int32>* recipes);
	void	SendTitleUpdate();
	void	SendUpdateTitles(sint32 prefix, sint32 suffix);
	void	SendLanguagesUpdate(int32 id, bool setlang = 1);
	void	SendAchievementsList();
	void	SendAchievementUpdate(bool first_login = false);

	///<summary>Send the pet options window to the client</summary>
	///<param name="type">Type of pet, 1 = combat 0 = non combat</param>
	void	SendPetOptionsWindow(const char* pet_name, int8 type = 1);
	void	SendBiography();

	bool IsCrafting();

	void SetRecipeListSent(bool val) {m_recipeListSent = val; }
	bool GetRecipeListSent() { return m_recipeListSent; }
	void ShowRecipeBook();
	PendingResurrection* GetCurrentRez();
	void SendResurrectionWindow();
	void AcceptResurrection();
	Mutex m_resurrect;
	Mutex* GetResurrectMutex();
	void SetPendingLastName(string last_name);
	void RemovePendingLastName();
	string* GetPendingLastName();
	void SendLastNameConfirmation();

	void SetInitialSpawnsSent(bool val) { initial_spawns_sent = val; }

	bool GetInitialSpawnsSent() { return initial_spawns_sent; }

	void SendQuestJournalUpdate(Quest* quest, bool updated=true);

	void AddQuestTimer(int32 quest_id);

	void RemoveQuestTimer(int32 quest_id);

	void SetPendingFlightPath(int32 val) { pending_flight_path = val; }
	int32 GetPendingFlightPath() { return pending_flight_path; }

	void EndAutoMount();
	bool GetOnAutoMount() { return on_auto_mount; }
	
	bool IsCurrentTransmuteID(int32 trans_id);
	void SetTransmuteID(int32 trans_id);
	int32 GetTransmuteID();

	enum ServerSpawnPlacementMode { DEFAULT, OPEN_HEADING, CLOSE_HEADING };
	void SetSpawnPlacementMode(ServerSpawnPlacementMode mode) { spawnPlacementMode = mode; }
	ServerSpawnPlacementMode GetSpawnPlacementMode() { return spawnPlacementMode; }

	bool HandleNewLogin(int32 account_id, int32 access_code);
	void SendSpawnChanges(set<Spawn*>& spawns);
	void MakeSpawnChangePacket(map<int32, SpawnData> info_changes, map<int32, SpawnData> pos_changes, map<int32, SpawnData> vis_changes, int32 info_size, int32 pos_size, int32 vis_size);

	bool IsZonedIn() { return connected_to_zone; }

	void SendHailCommand(Spawn* target);
	void SendDefaultCommand(Spawn* spawn, const char* command, float distance);

	void SetTempPlacementSpawn(Spawn* tmp);
	
	Spawn* GetTempPlacementSpawn() { return tempPlacementSpawn; }

	void SetPlacementUniqueItemID(int32 id) { placement_unique_item_id = id; }
	int32 GetPlacementUniqueItemID() { return placement_unique_item_id; }

	void SetHasOwnerOrEditAccess(bool val) { hasOwnerOrEditAccess = val; }
	bool HasOwnerOrEditAccess() { return hasOwnerOrEditAccess; }

	bool HandleHouseEntityCommands(Spawn* spawn, int32 spawnid, string command);
	// find an appropriate spawn to use for the house object, save spawn location/entry data to DB
	bool PopulateHouseSpawn(PacketStruct* place_object);
	
	// finalize the spawn-in of the object in world, remove the item from player inventory, set the spawned in object item id (for future pickup)
	bool PopulateHouseSpawnFinalize();

	void SendMoveObjectMode(Spawn* spawn, uint8 placementMode, float unknown2_3=0.0f);

	void SendFlightAutoMount(int32 path_id, int16 mount_id = 0, int8 mount_red_color = 0xFF, int8 mount_green_color = 0xFF, int8 mount_blue_color=0xFF);

	void SendShowBook(Spawn* sender, string title, int8 language, int8 num_pages, ...);
	void SendShowBook(Spawn* sender, string title, int8 language, vector<Item::BookPage*> pages);

	void SetTemporaryTransportID(int32 id) { temporary_transport_id = id; }
	int32 GetTemporaryTransportID() { return temporary_transport_id; }

	void SetRejoinGroupID(int32 id) { rejoin_group_id = id; }

	void TempRemoveGroup();
	void ReplaceGroupClient(Client* new_client);

	void SendWaypoints();

	void AddWaypoint(string name, int8 type);
	void RemoveWaypoint(string name) {
		if (waypoints.count(name) > 0){
			waypoints.erase(name);
		}
	}
	void SelectWaypoint(int32 id);
	void ClearWaypoint();
	bool ShowPathToTarget(float x, float y, float z, float y_offset);
	bool ShowPathToTarget(Spawn* spawn);

	void SetRegionDebug(bool val) { regionDebugMessaging = val; }

	static void CreateMail(int32 charID, std::string fromName, std::string subjectName, std::string mailBody, 
		int8 mailType, int32 copper, int32 silver, int32 gold, int32 platinum, int32 item_id, int16 stack_size, int32 time_sent, int32 expire_time);
	void CreateAndUpdateMail(std::string fromName, std::string subjectName, std::string mailBody, 
		int8 mailType, int32 copper, int32 silver, int32 gold, int32 platinum, int32 item_id, int16 stack_size, int32 time_sent, int32 expire_time);

	void SendEquipOrInvUpdateBySlot(int8 slot);

	void SetReloadingZone(bool val) { client_reloading_zone = val; }
	bool IsReloadingZone() { return client_reloading_zone; }

	void QueueStateCommand(int32 spawn_player_id, int32 state);
	void ProcessStateCommands();
	void PurgeItem(Item* item);
	void ConsumeFoodDrink(Item* item, int32 slot);
	void AwardCoins(int64 total_coins, std::string reason = string(""));

	void TriggerSpellSave();

	void ClearSentItemDetails() { 
		MItemDetails.writelock(__FUNCTION__, __LINE__);
		sent_item_details.clear();
		MItemDetails.releasewritelock(__FUNCTION__, __LINE__);
	}

	bool IsPlayerLoadingComplete() { return player_loading_complete; }

	int32 GetRejoinGroupID() { return rejoin_group_id; }

	void ClearSentSpellList() { 
		MSpellDetails.writelock(__FUNCTION__, __LINE__);
		sent_spell_details.clear();
		MSpellDetails.releasewritelock(__FUNCTION__, __LINE__);
	}

	void UpdateSentSpellList();

	bool CountSentSpell(int32 id, int32 tier) {
		bool res = false;
		MSpellDetails.readlock(__FUNCTION__, __LINE__);
		std::map<int32, int32>::iterator itr = sent_spell_details.find(id);
		if(itr != sent_spell_details.end() && itr->second == tier)
			res = true;
		MSpellDetails.releasereadlock(__FUNCTION__, __LINE__);
		return res;
	}

	void SetSentSpell(int32 id, int32 tier) {
		MSpellDetails.writelock(__FUNCTION__, __LINE__);
		sent_spell_details[id] = tier;
		MSpellDetails.releasewritelock(__FUNCTION__, __LINE__);
	}

	void	DisableSave() { disable_save = true; }
	bool	IsSaveDisabled() { return disable_save; }
	void	ResetZoningCoords() { 
		zoning_x = 0;
		zoning_y = 0;
		zoning_z = 0;
		zoning_h = 0;
	}
	void	SetZoningCoords(float x, float y, float z, float h) { 
		zoning_x = x;
		zoning_y = y;
		zoning_z = z;
		zoning_h = h;
	}
	
	bool	UseItem(Item* item, Spawn* target = nullptr);
	
	void	SendPlayFlavor(Spawn* spawn, int8 language, VoiceOverStruct* non_garble, VoiceOverStruct* garble, bool success = false, bool garble_success = false);
	void	SaveQuestRewardData(bool force_refresh = false);
	void	UpdateCharacterRewardData(QuestRewardData* data);
	void	SetQuestUpdateState(bool val) { quest_updates = val; }
	
	
	bool	SetPlayerPOVGhost(Spawn* spawn);
	
	int32	GetPlayerPOVGhostSpawnID() { return pov_ghost_spawn_id; }
	
	void	HandleDialogSelectMsg(int32 conversation_id, int32 response_index);
	bool	SetPetName(const char* name);
	
	bool	CheckConsumptionAllowed(int16 slot, bool send_message = true);
	
	void	StartLinkdeadTimer();
	bool	IsLinkdeadTimerEnabled();
	
	bool	AddRecipeBookToPlayer(int32 recipe_id, Item* item = nullptr);
	bool	RemoveRecipeFromPlayer(int32 recipe_id);
	
	void	SaveSpells();
	
	void	GiveQuestReward(Quest* quest, bool has_displayed = false);
	
	void	SendReplaceWidget(int32 widget_id, bool delete_widget, float x=0.0f, float y=0.0f, float z=0.0f, int32 grid_id=0);
	void	ProcessZoneIgnoreWidgets();
	
	void	SendHearCast(Spawn* caster, Spawn* target, int32 spell_visual, int16 cast_time);
	int32	GetSpellVisualOverride(int32 spell_visual);
	
	sint16	GetClientItemPacketOffset() { sint16 offset = -1; if(GetVersion() <= 373) { offset = -2; } return offset; }
private:
	void	AddRecipeToPlayerPack(Recipe* recipe, PacketStruct* packet, int16* i);
	void    SavePlayerImages();
	void	SkillChanged(Skill* skill, int16 previous_value, int16 new_value);
	void	SetStepComplete(int32 quest_id, int32 step);
	void	AddStepProgress(int32 quest_id, int32 step, int32 progress);
	
	void	SendNewSpells(int8 class_id);
	void	SendNewTSSpells(int8 class_id);
	void	AddSendNewSpells(vector<Spell*>* spells);
	
	map<int32, map<int32, int32> > quest_pending_updates;
	vector<QueuedQuest*> quest_queue;
	vector<QuestRewardData*> quest_pending_reward;
	volatile bool	quest_updates;
	Mutex	MQuestPendingUpdates;
	Mutex	MQuestQueue;
	Mutex	MDeletePlayer;
	vector<Item*>* search_items;
	int32 waypoint_id = 0;
	map<string, WaypointInfo> waypoints;
	Spawn*	transport_spawn;
	Mutex	MBuyBack;
	deque<BuyBackItem*> buy_back_items;
	Spawn*	merchant_transaction;
	Spawn*	mail_transaction;
	mutable std::shared_mutex MPendingQuestAccept;
	vector<int32> pending_quest_accept;	
	bool	lua_debug;
	bool	should_target;
	Spawn*	combine_spawn;
	int8	num_active_failures;
	int32	next_conversation_id;
	map<int32, int32> conversation_spawns;
	map<int32, Item*> conversation_items;
	mutable std::shared_mutex MConversation;
	map<int32, map<int8, string> > conversation_map;
	int32	current_quest_id;
	Spawn*	banker;
	map<int32, int32> sent_spell_details;
	map<int32, bool> sent_item_details;
	Player*	player;
	int16	version;
	int8	timestamp_flag;
	int32	ip;
	int16	port;
	int32	account_id;
	int32	character_id;
	sint16	admin_status; // -2 Banned, -1 Suspended, 0 User, etc.
	char	account_name[64];
	char	zone_name[64];
	int32	zoneID;
	int32	instanceID;
	Timer*	autobootup_timeout;
	int32	pwaitingforbootup;
	int32	last_update_time;

	int32	last_saved_timestamp;

	Timer*	CLE_keepalive_timer;
	Timer*	connect;
	Timer*	camp_timer;
	Timer*	linkdead_timer;
	bool	connected;
	std::atomic<bool> ready_for_spawns;
	std::atomic<bool> ready_for_updates;
	
	bool	seencharsel;
	bool	connected_to_zone;
	bool	client_zoning;
	int32	zoning_id;
	int32	zoning_instance_id;
	ZoneServer* zoning_destination;
	float	zoning_x;
	float	zoning_y;
	float	zoning_z;
	float	zoning_h;
	bool	firstlogin;
	
	enum 	NewLoginState { LOGIN_NONE, LOGIN_DELAYED, LOGIN_ALLOWED, LOGIN_INITIAL_LOAD, LOGIN_SEND };
	NewLoginState	new_client_login; // 1 = delayed state, 2 = let client in
	Timer	underworld_cooldown_timer;
	Timer	pos_update;
	Timer	quest_pos_timer;
	Timer	lua_debug_timer;
	Timer	temp_placement_timer;
	Timer	spawn_removal_timer;
	std::atomic<bool> player_pos_changed;
	std::atomic<int8> player_pos_change_count;
	int32 player_pos_timer;
	bool enabled_player_pos_timer;
	bool HandlePacket(EQApplicationPacket *app);
	EQStream* eqs;
	bool quickbar_changed;
	ZoneServer* current_zone;
	int32	name_crc;
	MailWindow	mail_window;
	std::mutex	MMailWindowMutex;
	PendingGuildInvite	pending_guild_invite;
	PendingResurrection current_rez;
	string* pending_last_name;
	IncomingPaperdollImage incoming_paperdoll;
	int32 transmuteID;
	ZoneServer* GetHouseZoneServer(int32 spawn_id, int64 house_id);
	
	std::atomic<bool> m_recipeListSent;
	bool initial_spawns_sent;
	bool should_load_spells;

	// int32 = quest id
	vector<int32> quest_timers;
	Mutex MQuestTimers;

	int32 pending_flight_path;

	ServerSpawnPlacementMode spawnPlacementMode;
	bool on_auto_mount;
	bool EntityCommandPrecheck(Spawn* spawn, const char* command);
	bool delayedLogin;
	int32 delayedAccountID;
	int32 delayedAccessKey;
	Timer delayTimer;
	Spawn* tempPlacementSpawn;
	int32 placement_unique_item_id;
	bool hasOwnerOrEditAccess;
	bool hasSentTempPlacementSpawn;

	int32 temporary_transport_id;

	int32 rejoin_group_id;
	
	int32 lastRegionRemapTime;
	
	bool regionDebugMessaging;

	bool client_reloading_zone;

	map<int32, int32> queued_state_commands;
	Mutex MQueueStateCmds;
	Timer save_spell_state_timer; // will be the 're-trigger' to delay
	int32 save_spell_state_time_bucket; // bucket as we collect over time when timer is reset by new spell effects being casted
	std::mutex MSaveSpellStateMutex;
	bool player_loading_complete;
	Mutex MItemDetails;
	Mutex MSpellDetails;
	bool disable_save;
	vector< string > devices;
	
	std::atomic<int32> pov_ghost_spawn_id;
	Timer delay_msg_timer;
	
	uchar* recipe_orig_packet;
	uchar* recipe_xor_packet;
	int	recipe_packet_count;
	int recipe_orig_packet_size;
};

class ClientList {
public:
	ClientList();
	~ClientList();
	bool	ContainsStream(EQStream* eqs);
	void	Add(Client* client);
	Client*	Get(int32 ip, int16 port);
	Client* FindByAccountID(int32 account_id);
	Client* FindByName(char* charname);
	void	Remove(Client* client, bool delete_data = false);
	void	RemoveConnection(EQStream* eqs);
	void	Process();
	int32	Count();
	void	ReloadQuests();
	void	CheckPlayersInvisStatus(Client* owner);
	void	RemovePlayerFromInvisHistory(int32 spawnID);
private:
	Mutex	MClients;
	list<Client*> client_list;
};
#endif
