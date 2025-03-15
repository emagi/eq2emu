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
#ifndef QUESTS_H
#define QUESTS_H

#include <map>
#include <vector>
#include "Items/Items.h"

#define	QUEST_STEP_TYPE_KILL			1
#define	QUEST_STEP_TYPE_CHAT			2
#define	QUEST_STEP_TYPE_OBTAIN_ITEM		3
#define	QUEST_STEP_TYPE_LOCATION		4
#define	QUEST_STEP_TYPE_SPELL			5
#define	QUEST_STEP_TYPE_NORMAL			6
#define QUEST_STEP_TYPE_CRAFT           7
#define QUEST_STEP_TYPE_HARVEST         8
#define	QUEST_STEP_TYPE_KILL_RACE_REQ	9 // kill using race type requirement instead of npc db id

#define QUEST_DISPLAY_STATUS_HIDDEN			0
#define QUEST_DISPLAY_STATUS_NO_CHECK		1
#define QUEST_DISPLAY_STATUS_YELLOW			2
#define QUEST_DISPLAY_STATUS_COMPLETED		4
#define QUEST_DISPLAY_STATUS_REPEATABLE		8
#define QUEST_DISPLAY_STATUS_CAN_SHARE		16
#define QUEST_DISPLAY_STATUS_COMPLETE_FLAG	32
#define	QUEST_DISPLAY_STATUS_SHOW			64
#define QUEST_DISPLAY_STATUS_CHECK			128


#define QUEST_SHAREABLE_NONE				0
#define QUEST_SHAREABLE_ACTIVE				1
#define QUEST_SHAREABLE_DURING				2
#define QUEST_SHAREABLE_COMPLETED			4

struct QuestFactionPrereq{
	int32	faction_id;
	sint32	min;
	sint32	max;
};

struct Location {
	int32 id;
	float x;
	float y;
	float z;
	int32 zone_id;
};

class QuestStep{
public:
	QuestStep(int32 in_id, int8 in_type, string in_description, vector<int32>* in_ids, int32 in_quantity, const char* in_task_group, vector<Location>* in_locations, float in_max_variation, float in_percentage, int32 in_usableitemid);
	QuestStep(QuestStep* old_step);
	~QuestStep();
	bool			CheckStepKillRaceReqUpdate(Spawn* spawn);
	bool			CheckStepReferencedID(int32 id);
	bool			CheckStepLocationUpdate(float char_x, float char_y, float char_z, int32 zone_id);
	int32			AddStepProgress(int32 val);
	void			SetStepProgress(int32 val);
	int32			GetStepProgress();
	int8			GetStepType();
	bool			Complete();
	void			SetComplete();
	void			ResetTaskGroup();
	const char*		GetTaskGroup();
	const char*		GetDescription();
	void			SetDescription(string desc);
	int16			GetQuestCurrentQuantity();
	int16			GetQuestNeededQuantity();
	map<int32, bool>*  GetUpdateIDs() { return ids; }
	int16			GetIcon();
	void			SetIcon(int16 in_icon);
	const char*		GetUpdateTargetName();
	void			SetUpdateTargetName(const char* name);
	const char*		GetUpdateName();
	void			SetUpdateName(const char* name);
	int32			GetStepID();
	int32			GetItemID();
	bool			WasUpdated();
	void			WasUpdated(bool val);
	float			GetPercentage();

	void			SetTaskGroup(string val) { task_group = val; }

private:
	bool				updated;
	int32				id;
	string				update_name;
	string				update_target_name;
	int16				icon;
	int8				type;
	string				description;
	std::map<int32, bool>*	ids;
	int32				quantity;
	string				task_group;
	vector<Location>*	locations;
	float				max_variation;
	float				percentage;
	int32				usableitemid;
	int32				step_progress;
};
class Player;
class Spell;

class Quest{
public:
	Quest(int32 in_id);
	Quest(Quest* old_quest);
	~Quest();
	EQ2Packet*			OfferQuest(int16 version, Player* player);
	EQ2Packet*			QuestJournalReply(int16 version, int32 player_crc, Player* player, QuestStep* updateStep = 0, int8 update_count = 1, bool old_completed_quest=false, bool quest_failure = false, bool display_quest_helper = true);

	void				RegisterQuest(string in_name, string in_type, string in_zone, int8 in_level, string in_desc);

	void				SetPrereqLevel(int8 lvl);
	void				SetPrereqTSLevel(int8 lvl);
	void                SetPrereqMaxLevel(int8 lvl) {prereq_max_level = lvl;}
	void                SetPrereqMaxTSLevel(int8 lvl) {prereq_max_tslevel = lvl;}
	void				AddPrereqClass(int8 class_id);
	void				AddPrereqTradeskillClass(int8 class_id);
	void				AddPrereqModelType(int16 model_type);
	void				AddPrereqRace(int8 race);
	void				AddPrereqQuest(int32 quest_id);
	void				AddPrereqFaction(int32 faction_id, sint32 min, sint32 max = 0);
	void				AddPrereqItem(Item* item);

	void				AddRewardItem(Item* item);
	void				AddTmpRewardItem(Item* item);
	void				GetTmpRewardItemsByID(std::vector<Item*>* items);
	void				AddRewardItemVec(vector<Item*>* items, Item* item, bool combine_items = true);
	void				AddSelectableRewardItem(Item* item);
	void				AddRewardCoins(int32 copper, int32 silver, int32 gold, int32 plat);
	void                AddRewardCoinsMax(int64 coins);
	void				AddRewardFaction(int32 faction_id, sint32 amount);
	void				SetRewardStatus(int32 amount);
	void				SetRewardComment(string comment);
	void				SetRewardXP(int32 xp);
	void                SetRewardTSXP(int32 xp) { reward_tsexp = xp; }

	bool				AddQuestStep(QuestStep* step);
	QuestStep*			AddQuestStep(int32 id, int8 in_type, string in_description, vector<int32>* in_ids, int32 in_quantity, const char* in_task_group = 0, vector<Location>* in_locations = 0, float in_max_variation = 0, float in_percentage = 0,int32 in_usableitemid = 0);
	bool				SetStepComplete(int32 step);
	bool				AddStepProgress(int32 step_id, int32 progress);
	int16				GetQuestStep();
	int32				GetStepProgress(int32 step_id);
	int16				GetTaskGroupStep();
	bool				QuestStepIsActive(int16 quest_step_id);
	bool				CheckQuestReferencedSpawns(Spawn* spawn);
	bool				CheckQuestKillUpdate(Spawn* spawn, bool update = true);
	bool				CheckQuestChatUpdate(int32 id, bool update = true);
	bool				CheckQuestItemUpdate(int32 id, int8 quantity = 1);
	bool				CheckQuestLocationUpdate(float char_x, float char_y, float char_z, int32 zone_id);
	bool				CheckQuestSpellUpdate(Spell* spell);
	bool                CheckQuestRefIDUpdate(int32 id, int32 quantity = 1);

	int8				GetQuestLevel();
	int8				GetVisible();
	int32				GetQuestID();
	void				SetQuestID(int32 in_id);
	int8				GetPrereqLevel();
	int8				GetPrereqTSLevel();
	int8                GetPrereqMaxLevel() {return prereq_max_level;}
	int8                GetPrereqMaxTSLevel() {return prereq_max_tslevel;}
	vector<QuestFactionPrereq>*	GetPrereqFactions();
	vector<int8>*		GetPrereqRaces();
	vector<int16>*		GetPrereqModelTypes();
	vector<int8>*		GetPrereqClasses();
	vector<int8>*		GetPrereqTradeskillClasses();
	vector<int32>*		GetPrereqQuests();
	vector<Item*>*		GetPrereqItems();
	vector<Item*>*		GetRewardItems();
	vector<Item*>*		GetTmpRewardItems();
	vector<Item*>*		GetSelectableRewardItems();
	map<int32, sint32>*	GetRewardFactions();
	void				GiveQuestReward(Player* player);
	void				AddCompleteAction(int32 step, string action);
	void                AddProgressAction(int32 step, string action);
	void				SetName(string in_name);
	void				SetType(string in_type);
	void				SetLevel(int8 in_level);
	void                SetEncounterLevel(int8 level) {enc_level = level;}
	void				SetDescription(string desc);
	void				SetStepDescription(int32 step, string desc);
	void				SetTaskGroupDescription(int32 step, string desc, bool display_bullets);

	void				SetStatusTmpReward(int32 status) { tmp_reward_status = status; }
	int64				GetStatusTmpReward() { return tmp_reward_status; }

	void				SetCoinTmpReward(int64 coins) { tmp_reward_coins = coins; }
	int64				GetCoinTmpReward() { return tmp_reward_coins; }
	int64				GetCoinsReward();
	int64               GetCoinsRewardMax();
	int64               GetGeneratedCoin();
    void                SetGeneratedCoin(int64 coin);
	int8				GetLevel();
	int8                GetEncounterLevel() { return enc_level; }
	const char*			GetName();
	const char*			GetDescription();
	const char*			GetType();
	void				SetZone(string in_zone);
	const char*			GetZone();
	int8				GetDay();
	int8				GetMonth();
	int8				GetYear();
	int32				GetStatusPoints();
	void				SetDay(int8 value);
	void				SetMonth(int8 value);
	void				SetYear(int8 value);
	vector<QuestStep*>*	GetQuestUpdates();
	vector<QuestStep*>*	GetQuestFailures();
	vector<QuestStep*>*	GetQuestSteps();
	bool				GetQuestStepCompleted(int32 step_id);
	QuestStep*			GetQuestStep(int32 step_id);
	void				SetCompleteAction(string action);
	const char*			GetCompleteAction();
	const char*			GetCompleteAction(int32 step);
	void				SetQuestGiver(int32 id);
	int32				GetQuestGiver();
	void				SetQuestReturnNPC(int32 id);
	int32				GetQuestReturnNPC();
	Player*				GetPlayer();
	void				SetPlayer(Player* in_player);
	bool				GetCompleted();
	bool				HasSentLastUpdate() { return has_sent_last_update; }
	void				SetSentLastUpdate(bool val) { has_sent_last_update = val; }
	void				SetCompletedDescription(string desc);
	const char*			GetCompletedDescription();
	int32				GetExpReward();
	int32               GetTSExpReward() { return reward_tsexp; }
	bool				GetDeleted();
	void				SetDeleted(bool val);
	bool				GetUpdateRequired();
	void				SetUpdateRequired(bool val);
	void				SetTurnedIn(bool val);
	bool				GetTurnedIn();
	bool				GetSaveNeeded(){ return needs_save; }
	void				SetSaveNeeded(bool val){ needs_save = val; }

	void				SetFeatherColor(int8 val) { m_featherColor = val; }
	int8				GetFeatherColor() { return m_featherColor; }

	void				SetRepeatable(bool val) { m_repeatable = val; }
	bool				IsRepeatable() { return m_repeatable; }
	
	void				SetTracked(bool val) { m_tracked = val; }
	bool				GetTracked() { return m_tracked; }
	bool				IsTracked() { return m_tracked && !m_hidden; }
	void                SetCompletedFlag(bool val);
	bool                GetCompletedFlag() {return completed_flag;}
	bool                GetYellowName() {return yellow_name;}
	void                SetYellowName(bool val) {yellow_name = val;}
	bool                CheckCategoryYellow();

	///<summary>Sets the custom flags for use in lua</summary>
	///<param name='flags'>Value to set the flags to</param>
	void				SetQuestFlags(int32 flags) { m_questFlags = flags; SetSaveNeeded(true); }

	///<summary>Gets the custom lua flags</summary>
	///<returns>The current flags (int32)</returns>
	int32				GetQuestFlags() { return m_questFlags; }

	///<summary>Checks to see if the quest is hidden</summary>
	///<returns>True if the quest is hidden</returns>
	bool				IsHidden() { return m_hidden; }

	///<summary>Sets the quest hidden flag</summary>
	///<param name='val'>Value to set the hidden flag to</param>
	void				SetHidden(bool val) { m_hidden = val; SetSaveNeeded(true); }
	
	///<summary>Sets the quest status earned</summary>
	///<param name='val'>Value to set the quest status earned</param>
	void				SetStatusEarned(int32 status_) { m_status = status_; SetSaveNeeded(true); }

	///<summary>Gets the quest status earned</summary>
	int32				GetStatusEarned() { return m_status; }

	///<summary>Gets the step timer</summary>
	///<returns>Unix timestamp (int32)</returns>
	int32				GetStepTimer() { return m_timestamp; }

	///<summary>Sets the step timer</summary>
	///<param name='duration'>How long to set the timer for, in seconds</param>
	void				SetStepTimer(int32 duration);

	///<summary>Sets the step that the timer is for</summary>
	///<param name='step'>Step to set the timer for</param>
	void				SetTimerStep(int32 step) { m_timerStep = step; }

	///<summary>Gets the step that the timer is for</summary>
	int32				GetTimerStep() { return m_timerStep; }

	///<summary>Adds a lua call back function for when the step fails</summary>
	///<param name='step'>The step to add the call back for</param>
	///<param name='action'>The lua callback function</param>
	void				AddFailedAction(int32 step, string action);

	///<summary>Fail the given step</summary>
	///<param name='step'>The step to fail</param>
	void				StepFailed(int32 step);

	///<summary>Removes the given step from the quest</summary>
	///<param name='step'>The step id to remove</param>
	///<param name='client'>The client who has this quest</param>
	///<returns>True if able to remove the quest</returns>
	bool				RemoveQuestStep(int32 step, Client* client);

	int16				GetCompleteCount() { return m_completeCount; }
	void				SetCompleteCount(int16 val) { m_completeCount = val; }
	void				IncrementCompleteCount() { m_completeCount += 1; }

	void				SetQuestTemporaryState(bool tempState, std::string customDescription = string(""));
	bool				GetQuestTemporaryState() { return quest_state_temporary; }
	std::string			GetQuestTemporaryDescription() { return quest_temporary_description; }
	
	void				SetQuestShareableFlag(int32 flag) { quest_shareable_flag = flag; }
	void				SetCanDeleteQuest(bool newval) { can_delete_quest = newval; }
	void				SetHideReward(bool newval) { hide_reward = newval; }
	
	
	void				SetStatusToEarnMin(int32 value_) { status_to_earn_min = value_; }
	void				SetStatusToEarnMax(int32 value_) { status_to_earn_max = value_; }
	
	int32				GetStatusToEarnMin() { return status_to_earn_min; }
	int32				GetStatusToEarnMax() { return status_to_earn_max; }
	
	int32				GetQuestShareableFlag() { return quest_shareable_flag; }
	bool				CanDeleteQuest() { return can_delete_quest; }
	bool				GetHideReward() { return hide_reward; }
	
	bool				CanShareQuestCriteria(Client* quest_sharer, bool display_client_msg = true);
	Mutex				MQuestSteps;
protected:
	bool				needs_save;
	Mutex				MCompletedActions;
	Mutex               MProgressActions;
	Mutex				MFailedActions;
	bool				turned_in;
	bool				update_needed;
	bool				deleted;
	bool				has_sent_last_update;

	string				completed_description;
	
	map<int32, string>	complete_actions;
	map<int32, string>  progress_actions;
	map<int32, string>	failed_actions;
	int8				day; //only here to make our lives easier
	int8				month;
	int8				year;
	int8				visible;

	int32				id;
	string				name;
	string				type;
	string				zone;
	int8				level;
	int8                enc_level;
	string				description;
	string				complete_action;

	Player*				player;
	vector<QuestFactionPrereq> prereq_factions;
	int8				prereq_level;
	int8				prereq_tslevel;
	int8                prereq_max_level;
	int8                prereq_max_tslevel;
	vector<int16>		prereq_model_types;
	vector<int8>		prereq_races;
	vector<int8>		prereq_classes;
	vector<int8>		prereq_tradeskillclass;
	vector<int32>		prereq_quests;
	vector<Item*>		prereq_items;
	vector<Item*>		reward_items;
	vector<Item*>		selectable_reward_items;
	vector<Item*>		tmp_reward_items;
	int64				reward_coins;
	int64               reward_coins_max;
	int32				tmp_reward_status;
	int64				tmp_reward_coins;
	int64               generated_coin;
	map<int32, sint32>	reward_factions;
	int32				reward_status;
	string				reward_comment;
	int32				reward_exp;
	int32               reward_tsexp;
	vector<QuestStep*>	step_updates;
	vector<QuestStep*>	step_failures;
	map<int32, QuestStep*> quest_step_map;
	map<QuestStep*, int32> quest_step_reverse_map;
	vector<QuestStep*>	quest_steps;
	map<int16, string>  task_group_order;
	int16				task_group_num;
	map<string, vector<QuestStep*> > task_group;
	int32				quest_giver;
	int32				return_id;
	int8				m_featherColor;
	bool				m_repeatable;
	bool				m_tracked;
	bool                completed_flag;
	bool                yellow_name;
	int32				m_questFlags;
	bool				m_hidden;
	int32				m_status;

	int32				m_timestamp;	// timer for a quest step
	int32				m_timerStep;	// used for the fail action when timer expires

	int16				m_completeCount;

	bool				quest_state_temporary;
	std::string			quest_temporary_description;
	int32				quest_shareable_flag;
	bool				can_delete_quest;
	int32				status_to_earn_min;
	int32				status_to_earn_max;
	bool				hide_reward;
};

class MasterQuestList{
public:
	MasterQuestList();
	Quest* GetQuest(int32 id, bool copyQuest = true){
		if(quests.count(id) > 0)
		{
			if(copyQuest)
				return new Quest(quests[id]);
			else
				return quests[id];
		}
		else
			return 0;
	}

	map<int32, Quest*>*	GetQuests(){
		return &quests;
	}

	void AddQuest(int32 id, Quest* quest){
		quests[id] = quest;
	}
	void Reload();
	void LockQuests();
	void UnlockQuests();

private:
	Mutex MQuests;
	map<int32, Quest*> quests;
};

#endif
