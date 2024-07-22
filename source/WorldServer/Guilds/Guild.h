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

#ifndef GUILD_H_
#define GUILD_H_

#include <vector>
#include <deque>
#include <map>
#include "../../common/Mutex.h"
#include "../MutexMap.h"
using namespace std;

class ZoneServer;
class Client;
class Player;

#define GUILD_RANK_LEADER			0
#define GUILD_RANK_SENIOR_OFFICER	1
#define GUILD_RANK_OFFICER			2
#define GUILD_RANK_SENIOR_MEMBER	3
#define GUILD_RANK_MEMBER			4
#define GUILD_RANK_JUNIOR_MEMBER	5
#define GUILD_RANK_INITIATE			6
#define GUILD_RANK_RECRUIT			7

#define GUILD_PERMISSIONS_INVITE						0
#define GUILD_PERMISSIONS_RMEOVE_MEMBER					1
#define GUILD_PERMISSIONS_PROMOTE_MEMBER				2
#define GUILD_PERMISSIONS_DEMOTE_MEMBER					3
#define GUILD_PERMISSIONS_CHANGE_MOTD					6
#define GUILD_PERMISSIONS_CHANGE_PERMISSIONS			7
#define GUILD_PERMISSIONS_CHANGE_RANK_NAMES				8
#define GUILD_PERMISSIONS_SEE_OFFICER_NOTES				9
#define GUILD_PERMISSIONS_EDIT_OFFICER_NOTES			10
#define GUILD_PERMISSIONS_SEE_OFFICER_CHAT				11
#define GUILD_PERMISSIONS_SPEAK_IN_OFFICER_CHAT			12
#define GUILD_PERMISSIONS_SEE_GUILD_CHAT				13
#define GUILD_PERMISSIONS_SPEAK_IN_GUILD_CHAT			14
#define GUILD_PERMISSIONS_EDIT_PERSONAL_NOTES			15
#define GUILD_PERMISSIONS_EDIT_PERSONAL_NOTES_OTHERS	16
#define GUILD_PERMISSIONS_EDIT_EVENT_FILTERS			17
#define GUILD_PERMISSIONS_EDIT_EVENTS					18
#define GUILD_PERMISSIONS_PURCHASE_STATUS_ITEMS			19
#define GUILD_PERMISSIONS_DISPLAY_GUILD_NAME			20
#define GUILD_PERMISSIONS_SEND_EMAIL_TO_GUILD			21
#define GUILD_PERMISSIONS_BANK1_SEE_CONTENTS			22
#define GUILD_PERMISSIONS_BANK2_SEE_CONTENTS			23
#define GUILD_PERMISSIONS_BANK3_SEE_CONTENTS			24
#define GUILD_PERMISSIONS_BANK4_SEE_CONTENTS			25
#define GUILD_PERMISSIONS_BANK1_DEPOSIT					26
#define GUILD_PERMISSIONS_BANK2_DEPOSIT					27
#define GUILD_PERMISSIONS_BANK3_DEPOSIT					28
#define GUILD_PERMISSIONS_BANK4_DEPOSIT					29
#define GUILD_PERMISSIONS_BANK1_WITHDRAWL				30
#define GUILD_PERMISSIONS_BANK2_WITHDRAWL				31
#define GUILD_PERMISSIONS_BANK3_WITHDRAWL				32
#define GUILD_PERMISSIONS_BANK4_WITHDRAWL				33
#define GUILD_PERMISSIONS_EDIT_RECRUITING_SETTINGS		35
#define GUILD_PERMISSIONS_MAKE_OTHERS_RECRUITERS		36
#define GUILD_PERMISSIONS_SEE_RECRUITING_SETTINGS		37
#define GUILD_PERMISSIONS_ASSIGN_POINTS					43
#define GUILD_PERMISSIONS_RECEIVE_POINTS				44

#define GUILD_EVENT_FILTER_CATEGORY_RETAIN_HISTORY	0
#define GUILD_EVENT_FILTER_CATEGORY_BROADCAST		1

#define GUILD_EVENT_GUILD_LEVEL_UP				0
#define GUILD_EVENT_GUILD_LEVEL_DOWN			1
#define GUILD_EVENT_DISCOVERS_ITEM				2
#define GUILD_EVENT_GAINS_ADV_LEVEL_1_10		3
#define GUILD_EVENT_GAINS_ADV_LEVEL_11_20		4
#define GUILD_EVENT_GAINS_ADV_LEVEL_21_30		5
#define GUILD_EVENT_GAINS_ADV_LEVEL_31_40		6
#define GUILD_EVENT_GAINS_ADV_LEVEL_41_50		7
#define GUILD_EVENT_GAINS_TS_LEVEL_1_10			8
#define GUILD_EVENT_GAINS_TS_LEVEL_11_20		9
#define GUILD_EVENT_GAINS_TS_LEVEL_21_30		10
#define GUILD_EVENT_GAINS_TS_LEVEL_31_40		11
#define GUILD_EVENT_GAINS_TS_LEVEL_41_50		12
#define GUILD_EVENT_MEMBER_JOINS				13
#define GUILD_EVENT_MEMBER_LEAVES				14
#define GUILD_EVENT_MEMBER_PROMOTED				15
#define GUILD_EVENT_MEMBER_DEMOTED				16
#define GUILD_EVENT_COMPLETES_HERITAGE_QUEST	19
#define GUILD_EVENT_KILLS_EPIC_MONSTER			20
#define GUILD_EVENT_LOOTS_ARTIFACT				21
#define GUILD_EVENT_LOOTS_FABELED_ITEM			22
#define GUILD_EVENT_LOOTS_LEGENDARY_ITEM		23
#define GUILD_EVENT_COMPLETES_WRIT				24
#define GUILD_EVENT_LOOTS_MYTHICAL_ITEM			25
#define GUILD_EVENT_GAINS_ADV_LEVEL_10			26
#define GUILD_EVENT_GAINS_ADV_LEVEL_20			27
#define GUILD_EVENT_GAINS_ADV_LEVEL_30			28
#define GUILD_EVENT_GAINS_ADV_LEVEL_40			29
#define GUILD_EVENT_GAINS_ADV_LEVEL_50			30
#define GUILD_EVENT_GAINS_TS_LEVEL_10			31
#define GUILD_EVENT_GAINS_TS_LEVEL_20			32
#define GUILD_EVENT_GAINS_TS_LEVEL_30			33
#define GUILD_EVENT_GAINS_TS_LEVEL_40			34
#define GUILD_EVENT_GAINS_TS_LEVEL_50			35
#define GUILD_EVENT_GAINS_ADV_LEVEL_51_60		37
#define GUILD_EVENT_GAINS_TS_LEVEL_51_60		38
#define GUILD_EVENT_GAINS_ADV_LEVEL_60			39
#define GUILD_EVENT_GAINS_TS_LEVEL_60			40
#define GUILD_EVENT_GAINS_ADV_LEVEL_61_70		41
#define GUILD_EVENT_GAINS_TS_LEVEL_61_70		42
#define GUILD_EVENT_GAINS_ADV_LEVEL_70			43
#define GUILD_EVENT_GAINS_TS_LEVEL_70			44
#define GUILD_EVENT_GAINS_AA_10					45
#define GUILD_EVENT_GAINS_AA_20					46
#define GUILD_EVENT_GAINS_AA_30					47
#define GUILD_EVENT_GAINS_AA_40					48
#define GUILD_EVENT_GAINS_AA_50					49
#define GUILD_EVENT_GAINS_AA_1_10				50
#define GUILD_EVENT_GAINS_AA_11_20				51
#define GUILD_EVENT_GAINS_AA_21_30				52
#define GUILD_EVENT_GAINS_AA_31_40				53
#define GUILD_EVENT_GAINS_AA_41_50				54
#define GUILD_EVENT_BECOMES_RECRUITER			55
#define GUILD_EVENT_NO_LONGER_RECRUITER			56
#define GUILD_EVENT_HERALDY_CHANGE				57
#define GUILD_EVENT_GAINS_AA_60					58
#define GUILD_EVENT_GAINS_AA_70					59
#define GUILD_EVENT_GAINS_AA_80					60
#define GUILD_EVENT_GAINS_AA_90					61
#define GUILD_EVENT_GAINS_AA_100				62
#define GUILD_EVENT_GAINS_AA_51_60				63
#define GUILD_EVENT_GAINS_AA_61_70				64
#define GUILD_EVENT_GAINS_AA_71_80				65
#define GUILD_EVENT_GAINS_AA_81_90				66
#define GUILD_EVENT_GAINS_AA_91_100				67
#define GUILD_EVENT_GAINS_ADV_LEVEL_80			68
#define GUILD_EVENT_GAINS_TS_LEVEL_80			69
#define GUILD_EVENT_GAINS_ADV_LEVEL_71_80		70
#define GUILD_EVENT_GAINS_TS_LEVEL_71_80		71
#define GUILD_EVENT_GAINS_AA_110				72
#define GUILD_EVENT_GAINS_AA_120				73
#define GUILD_EVENT_GAINS_AA_130				74
#define GUILD_EVENT_GAINS_AA_140				75
#define GUILD_EVENT_GAINS_AA_101_110			76
#define GUILD_EVENT_GAINS_AA_111_120			77
#define GUILD_EVENT_GAINS_AA_121_130			78
#define GUILD_EVENT_GAINS_AA_131_140			79
#define GUILD_EVENT_GAINS_AA_150				80
#define GUILD_EVENT_GAINS_AA_141_150			81
#define GUILD_EVENT_GAINS_AA_160				82
#define GUILD_EVENT_GAINS_AA_170				83
#define GUILD_EVENT_GAINS_AA_180				84
#define GUILD_EVENT_GAINS_AA_190				85
#define GUILD_EVENT_GAINS_AA_200				86
#define GUILD_EVENT_GAINS_AA_151_160			87
#define GUILD_EVENT_GAINS_AA_161_170			88
#define GUILD_EVENT_GAINS_AA_171_180			89
#define GUILD_EVENT_GAINS_AA_181_190			90
#define GUILD_EVENT_GAINS_AA_191_200			91
#define GUILD_EVENT_EARNS_ACHIEVEMENT			92

#define GUILD_RECRUITING_FLAG_TRAINING		0
#define GUILD_RECRUITING_FLAG_FIGHTERS		1
#define GUILD_RECRUITING_FLAG_PRIESTS		2
#define GUILD_RECRUITING_FLAG_SCOUTS		3
#define GUILD_RECRUITING_FLAG_MAGES			4
#define GUILD_RECRUITING_FLAG_TRADESKILLERS	5

#define GUILD_RECRUITING_PLAYSTYLE_NONE		0
#define GUILD_RECRUITING_PLAYSTYLE_CASUAL	1
#define GUILD_RECRUITING_PLAYSTYLE_HARDCORE	2

#define GUILD_RECRUITING_DESC_TAG_NONE				0
#define GUILD_RECRUITING_DESC_TAG_GOOD				1
#define GUILD_RECRUITING_DESC_TAG_EVIL				2
#define GUILD_RECRUITING_DESC_TAG_CHATTY			3
#define GUILD_RECRUITING_DESC_TAG_ORGANIZED			4
#define GUILD_RECRUITING_DESC_TAG_ROLEPLAY			5
#define GUILD_RECRUITING_DESC_TAG_ENJOY_QUESTS		6
#define GUILD_RECRUITING_DESC_TAG_ENJOY_RAIDS		7
#define GUILD_RECRUITING_DESC_TAG_ODD_HOURS			8
#define GUILD_RECRUITING_DESC_TAG_CRAFTER_ORIENTED	9
#define GUILD_RECRUITING_DESC_TAG_FAMILY_FRIENDLY	10
#define GUILD_RECRUITING_DESC_TAG_MATURE_HUMOR		11
#define GUILD_RECRUITING_DESC_TAG_INMATES_RUN		12
#define GUILD_RECRUITING_DESC_TAG_VERY_FUNNY		13
#define GUILD_RECRUITING_DESC_TAG_HUMOR_CAUES_PAIN	14
#define GUILD_RECRUITING_DESC_TAG_SERIOUS			15

#define GUILD_MEMBER_FLAGS_RECRUITING_FOR_GUILD	1
#define GUILD_MEMBER_FLAGS_NOTIFY_LOGINS		2
#define GUILD_MEMBER_FLAGS_DONT_GENERATE_EVENTS	4

#define GUILD_EVENT_ACTION_LOCK		0
#define GUILD_EVENT_ACTION_UNLOCK	1
#define GUILD_EVENT_ACTION_DELETE	2

#define GUILD_MAX_LEVEL			80
#define GUILD_MAX_POINT_HISTORY	50
#define GUILD_MAX_EVENTS		500
#define GUILD_MAX_LOCKED_EVENTS	200

struct PointHistory {
	int32 date;
	string modified_by;
	string comment;
	float points;
	bool saved_needed;
};

struct GuildMember {
	int32 character_id;
	int32 account_id;
	int32 recruiter_id;	//00 00 00 00 if not a guild recruiter
	char name[64];
	int32 guild_status;
	float points;
	int8 adventure_class;
	int8 adventure_level;
	int8 tradeskill_class;
	int8 tradeskill_level;
	int8 rank;
	int8 member_flags;
	string zone;
	int32 join_date;
	int32 last_login_date;
	string note;
	string officer_note;
	string recruiter_description;
	unsigned char* recruiter_picture_data;
	int16 recruiter_picture_data_size;
	int8 recruiting_show_adventure_class;
	deque<PointHistory*> point_history;
};

struct GuildEvent {
	int64 event_id;
	int32 date;
	int32 type;
	string description;
	int8 locked;
	bool save_needed;
};

struct GuildBankEvent {
	int64 event_id;
	int32 date;
	int32 type;
	string description;
};

struct Bank {
	string name;
	deque<GuildBankEvent*> events;
};

class Guild {
public:
	Guild();
	virtual ~Guild();
	void SetID(int32 id_in) {id = id_in;}
	void SetName(const char* name, bool send_packet = true);
	void SetLevel(int8 level, bool send_packet = true);
	void SetFormedDate(int32 formed_date_in) {formed_date = formed_date_in;}
	void SetMOTD(const char *motd, bool send_packet = true);
	int32 GetID() const {return id;}
	const char* GetName() const {return name;}
	int8 GetLevel() const {return level;}
	int32 GetFormedDate() const {return formed_date;}
	const char * GetMOTD() const {return motd;}
	void SetEXPCurrent(int64 exp, bool send_packet = true);
	void AddEXPCurrent(sint64 exp, bool send_packet = true);
	int64 GetEXPCurrent() const {return exp_current;}
	void SetEXPToNextLevel(int64 exp, bool send_packet = true);
	int64 GetEXPToNextLevel() const {return exp_to_next_level;}
	void SetRecruitingShortDesc(const char* new_desc, bool send_packet = true);
	string GetRecruitingShortDesc() const {return recruiting_short_desc;}
	void SetRecruitingFullDesc(const char* new_desc, bool send_packet = true);
	string GetRecruitingFullDesc() const {return recruiting_full_desc;}
	void SetRecruitingMinLevel(int8 new_level, bool send_packet = true);
	int8 GetRecruitingMinLevel() const {return recruiting_min_level;}
	void SetRecruitingPlayStyle(int8 new_play_style, bool send_packet = true);
	int8 GetRecruitingPlayStyle() const {return recruiting_play_style;}
	bool SetRecruitingDescTag(int8 index, int8 tag, bool send_packet = true);
	int8 GetRecruitingDescTag(int8 index);
	bool SetPermission(int8 rank, int8 permission, int8 value, bool send_packet = true);
	int8 GetPermission(int8 rank, int8 permission);
	bool SetEventFilter(int8 event_id, int8 category, int8 value, bool send_packet = true);
	int8 GetEventFilter(int8 event_id, int8 category);
	int32 GetNumUniqueAccounts();
	int32 GetNumRecruiters();
	int32 GetNextRecruiterID();
	int64 GetNextEventID();
	GuildMember* GetGuildMemberOnline(Client* client);
	GuildMember* GetGuildMember(Player* player);
	GuildMember* GetGuildMember(int32 character_id);
	GuildMember* GetGuildMember(const char* player_name);
	vector<GuildMember*>* GetGuildRecruiters();
	GuildEvent* GetGuildEvent(int64 event_id);
	bool SetRankName(int8 rank, const char* name, bool send_packet = true);
	const char* GetRankName(int8 rank);
	bool SetRecruitingFlag(int8 flag, int8 value, bool send_packet = true);
	int8 GetRecruitingFlag(int8 flag);
	bool SetGuildRecruiter(Client* client, const char* name, bool value, bool send_packet = true);
	bool SetGuildRecruiterDescription(Client* client, const char* description, bool send_packet = true);
	bool ToggleGuildRecruiterAdventureClass(Client* client, bool send_packet = true);
	bool SetGuildMemberNote(const char* name, const char* note, bool send_packet = true);
	bool SetGuildOfficerNote(const char* name, const char* note, bool send_packet = true);
	bool AddNewGuildMember(Client* client, const char* invited_by = 0, int8 rank = GUILD_RANK_RECRUIT);
	bool AddGuildMember(GuildMember* guild_member);
	void RemoveGuildMember(int32 character_id, bool send_packet = true);
	void RemoveAllGuildMembers();
	bool DemoteGuildMember(Client* client, const char* name, bool send_packet = true);
	bool PromoteGuildMember(Client* client, const char* name, bool send_packet = true);
	bool KickGuildMember(Client* client, const char* name, bool send_packet = true);
	bool InvitePlayer(Client* client, const char* name, bool send_packet = true);
	bool AddPointsToAll(Client* client, float points, const char* comment = 0, bool send_packet = true);
	bool AddPointsToAllOnline(Client* client, float points, const char* comment = 0, bool send_packet = true);
	bool AddPointsToGroup(Client* client, float points, const char* comment = 0, bool send_packet = true);
	bool AddPointsToRaid(Client* client, float points, const char* comment = 0, bool send_packet = true);
	bool AddPointsToGuildMember(Client* client, float points, const char* name, const char* comment = 0, bool send_packet = true);
	bool AddPointHistory(GuildMember* guild_member, int32 date, const char* modified_by, float points, const char* comment = 0, bool new_point_history = true);
	void ViewGuildMemberPoints(Client* client, const char* name);
	bool ChangeMemberFlag(Client* client, int8 member_flag, int8 value, bool send_packet = true);
	bool UpdateGuildMemberInfo(Player* player);
	bool UpdateGuildStatus(Player *player, int32 Status);
	void AddGuildEvent(int64 event_id, int32 type, const char* description, int32 date, int8 locked);
	void AddNewGuildEvent(int32 type, const char* description, int32 date, bool send_packet = true, ...);
	bool LockGuildEvent(int64 event_id, bool lock, bool send_packet = true);
	bool DeleteGuildEvent(int64 event_id, bool send_packet = true);
	void SendGuildMOTD(Client* client);
	void SendGuildEventList();
	void SendGuildEventList(Client* client);
	void SendGuildEventDetails();
	void SendGuildEventDetails(Client* client);
	void SendAllGuildEvents();
	void SendAllGuildEvents(Client* client);
	void SendOldGuildEvent(Client* client, GuildEvent* guild_event);
	void SendNewGuildEvent(GuildEvent* guild_event);
	void SendNewGuildEvent(Client* client, GuildEvent* guild_event);
	void SendGuildEventAction(int8 action, GuildEvent* guild_event);
	void SendGuildEventAction(Client* client, int8 action, GuildEvent* guild_event);
	void SendGuildBankEventList();
	void SendGuildBankEventList(Client* client);
	void SendGuildUpdate();
	void SendGuildUpdate(Client* client);
	void SendGuildMemberList();
	void SendGuildMemberList(Client* client);
	void SendGuildMember(Player* player, bool include_zone = true);
	void SendGuildMember(GuildMember* gm, bool include_zone = true);
	void SendGuildMember(Client* client, GuildMember* gm, bool include_zone = true);
	void SendGuildModification(float points, vector<int32>* character_ids);
	void SendGuildModification(Client* client, float points, vector<int32>* character_ids);
	void GuildMemberLogin(Client *client, bool first_login = false);
	void GuildMemberLogoff(Player *player);
	void SendGuildMemberLeave(int32 character_id);
	void SendGuildMemberLeave(Client* client, int32 character_id);
	void SendGuildRecruitingDetails(Client* client);
	void SendGuildRecruitingImages(Client* client);
	void SendGuildRecruiterInfo(Client* client, Player* player);
	void HandleGuildSay(Client* sender, const char* message);
	void HandleOfficerSay(Client* sender, const char* message);
	void SendMessageToGuild(int8 event_type, const char* message, ...);
	void SetSaveNeeded(bool val) {save_needed = val;}
	bool GetSaveNeeded() {return save_needed;}
	void SetMemberSaveNeeded(bool val) {member_save_needed = val;}
	bool GetMemberSaveNeeded() {return member_save_needed;}
	void SetEventsSaveNeeded(bool val) {events_save_needed = val;}
	bool GetEventsSaveNeeded() {return events_save_needed;}
	void SetRanksSaveNeeded(bool val) {ranks_save_needed = val;}
	bool GetRanksSaveNeeded() {return ranks_save_needed;}
	void SetEventFiltersSaveNeeded(bool val) {event_filters_save_needed = val;}
	bool GetEventFiltersSaveNeeded() {return event_filters_save_needed;}
	void SetPointsHistorySaveNeeded(bool val) {points_history_save_needed = val;}
	bool GetPointsHistorySaveNeeded() {return points_history_save_needed;}
	void SetRecruitingSaveNeeded(bool val) {recruiting_save_needed = val;}
	bool GetRecruitingSaveNeeded() {return recruiting_save_needed;}
	map<int32, GuildMember*>* GetGuildMembers() {return &members;}
	Mutex * GetGuildMembersMutex() {return &mMembers;}
	deque<GuildEvent*>* GetGuildEvents() {return &guild_events;}
	MutexMap<int8, MutexMap<int8, int8>*>* GetPermissions() {return &permissions;}
	MutexMap<int8, string>* GetGuildRanks() {return &ranks;}
	MutexMap<int8, int8>* GetRecruitingFlags() {return &recruiting_flags;}
	MutexMap<int8, int8>* GetRecruitingDescTags() {return &recruiting_desc_tags;}
	int8 GetRecruitingLookingForPacketValue();
	static string GetEpicMobDeathMessage(const char* player_name, const char* mob_name);

private:
	int32 id;
	char name[64];
	int8 level;
	int32 formed_date;
	char motd[256];
	int64 exp_current;
	int64 exp_to_next_level;
	string recruiting_short_desc;
	string recruiting_full_desc;
	int8 recruiting_min_level;
	int8 recruiting_play_style;
	MutexMap<int8, string> ranks;
	map<int32, GuildMember*> members;
	Mutex mMembers;
	deque<GuildEvent*> guild_events;
	MutexMap<int8, MutexMap<int8, int8>*> permissions;
	MutexMap<int8, MutexMap<int8, int8>*> event_filters;
	MutexMap<int8, int8> recruiting_flags;
	MutexMap<int8, int8> recruiting_desc_tags;
	Bank banks[4];
	int32 GetPermissionsPacketValue(int8 rank, int32 start, int32 end);
	int32 GetEventFilterPacketValue(int8 category, int32 start, int32 end);
	bool save_needed;
	bool member_save_needed;
	bool events_save_needed;
	bool event_filters_save_needed;
	bool ranks_save_needed;
	bool points_history_save_needed;
	bool recruiting_save_needed;
};

class GuildList {
public:
	GuildList();
	virtual ~GuildList();
	bool AddGuild(Guild* guild);
	Guild* GetGuild(int32 guild_id);
	Guild* GetGuild(const char* guild_name);
	bool RemoveGuild(Guild* guild, bool delete_data = false);
	bool RemoveGuild(int32 guild_id, bool delete_data = false);
	int32 GetNumGuilds() {return guild_list.size();}
	MutexMap<int32, Guild*>* GetGuilds() {return &guild_list;}

private:
	MutexMap<int32, Guild*> guild_list;
};

#endif
