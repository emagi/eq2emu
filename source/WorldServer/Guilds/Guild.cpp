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

#include <string.h>
#include <math.h>
#include <assert.h>
#include "Guild.h"
#include "../Player.h"
#include "../client.h"
#include "../World.h"
#include "../zoneserver.h"
#include "../WorldDatabase.h"
#include "../../common/Log.h"
#include "../Rules/Rules.h"
#include "../Web/PeerManager.h"

extern ConfigReader configReader;
extern ZoneList zone_list;
extern WorldDatabase database;
extern World world;
extern RuleManager rule_manager;
extern PeerManager peer_manager;

/***************************************************************************************************************************************************
 *																							GUILD
 ***************************************************************************************************************************************************/

Guild::Guild() {
	id = 0;
	memset(name, 0, sizeof(name));
	level = 1;
	formed_date = 0;
	memset(motd, 0, sizeof(motd));
	exp_current = 111;
	exp_to_next_level = 2521;
	recruiting_min_level = 1;
	recruiting_play_style = GUILD_RECRUITING_PLAYSTYLE_NONE;
	recruiting_flags.Put(GUILD_RECRUITING_FLAG_TRAINING, 0);
	recruiting_flags.Put(GUILD_RECRUITING_FLAG_FIGHTERS, 0);
	recruiting_flags.Put(GUILD_RECRUITING_FLAG_PRIESTS, 0);
	recruiting_flags.Put(GUILD_RECRUITING_FLAG_SCOUTS, 0);
	recruiting_flags.Put(GUILD_RECRUITING_FLAG_MAGES, 0);
	recruiting_flags.Put(GUILD_RECRUITING_FLAG_TRADESKILLERS, 0);
	recruiting_desc_tags.Put(0, GUILD_RECRUITING_DESC_TAG_NONE);
	recruiting_desc_tags.Put(1, GUILD_RECRUITING_DESC_TAG_NONE);
	recruiting_desc_tags.Put(2, GUILD_RECRUITING_DESC_TAG_NONE);
	recruiting_desc_tags.Put(3, GUILD_RECRUITING_DESC_TAG_NONE);
	banks[0].name = "Bank 1";
	banks[1].name = "Bank 2";
	banks[2].name = "Bank 3";
	banks[3].name = "Bank 4";
	save_needed = false;
	member_save_needed = false;
	events_save_needed = false;
	ranks_save_needed = false;
	event_filters_save_needed = false;
	points_history_save_needed = false;
	recruiting_save_needed = false;
	mMembers.SetName("Guild::members");
}

Guild::~Guild() {
	map<int32, GuildMember*>::iterator guild_member_itr;

	mMembers.writelock(__FUNCTION__, __LINE__);
	for (guild_member_itr = members.begin(); guild_member_itr != members.end(); guild_member_itr++) {
		deque<PointHistory*>::iterator point_history_itr;
		for (point_history_itr = guild_member_itr->second->point_history.begin(); point_history_itr != guild_member_itr->second->point_history.end(); point_history_itr++)
			safe_delete(*point_history_itr);
		safe_delete_array(guild_member_itr->second->recruiter_picture_data);
		safe_delete(guild_member_itr->second);
	}
	mMembers.releasewritelock(__FUNCTION__, __LINE__);

	deque<GuildEvent*>::iterator guild_events_itr;
	for (guild_events_itr = guild_events.begin(); guild_events_itr != guild_events.end(); guild_events_itr++)
		safe_delete(*guild_events_itr);
	MutexMap<int8, MutexMap<int8, int8>*>::iterator permissions_itr = permissions.begin();
	while (permissions_itr.Next())
		safe_delete(permissions_itr.second);
	MutexMap<int8, MutexMap<int8, int8>*>::iterator filter_itr = event_filters.begin();
	while (filter_itr.Next())
		safe_delete(filter_itr.second);
	for (int32 i = 0; i < 4; i++) {
		deque<GuildBankEvent*>::iterator bank_event_itr;
		for (bank_event_itr = banks[i].events.begin(); bank_event_itr != banks[i].events.end(); bank_event_itr++)
			safe_delete(*bank_event_itr);
	}
}

void Guild::SetName(const char* name, bool send_packet) {

	assert(name);

	strncpy(this->name, name, sizeof(this->name));
	if (send_packet) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Setting Guild Name to '%s'...", name);
		SendGuildUpdate();
		save_needed = true;
	}
}

void Guild::SetLevel(int8 level, bool send_packet) {

	this->level = level;
	if (send_packet) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Setting Guild Level to %i...", level);
		SendGuildUpdate();
		save_needed = true;
	}
}

void Guild::SetMOTD(const char *motd, bool send_packet) {

	assert(motd);

	strncpy(this->motd, motd, sizeof(this->motd));
	if (send_packet) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Setting Guild MOTD text:\n'%s'", motd);
		SendGuildUpdate();
		save_needed = true;
	}
}

void Guild::SetEXPCurrent(int64 exp, bool send_packet) {

	exp_current = exp;
	if (send_packet) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Setting Guild Current XP to %u", exp);
		SendGuildUpdate();
		save_needed = true;
	}
}

void Guild::AddEXPCurrent(sint64 exp, bool send_packet) {

	bool ret = false;
	char message[128];
	char adjective[16];

	int8 guild_max_level = rule_manager.GetGlobalRule(R_Guild, MaxLevel)->GetInt8();
	if (exp > 0 && level < guild_max_level) {
		exp_current += exp;
		if (exp_current >= exp_to_next_level) {
			LogWrite(GUILD__DEBUG, 0, "Guilds", "Guild %s Level UP! New Level: %i (current XP: %ul)", name, level, exp_current);
			int64 left_over = exp_current - exp_to_next_level;
			level++;
			exp_to_next_level *= 2;
			exp_current = left_over;
			AddNewGuildEvent(GUILD_EVENT_GUILD_LEVEL_UP, "The guild gained a level and is now level %u.", Timer::GetUnixTimeStamp(), true, level);
			SendMessageToGuild(GUILD_EVENT_GUILD_LEVEL_UP, "The guild gained a level and is now level %u.", level);
			
			if (level % 10 == 0) {
				if (level == 10)
					strncpy(adjective, "bold", sizeof(adjective));
				else if (level == 20)
					strncpy(adjective, "daring", sizeof(adjective));
				else if (level == 30)
					strncpy(adjective, "gallant", sizeof(adjective));
				else if (level == 40)
					strncpy(adjective, "noble", sizeof(adjective));
				else if (level == 50)
					strncpy(adjective, "heroic", sizeof(adjective));
				else if (level == 60)
					strncpy(adjective, "lordly", sizeof(adjective));
				else if (level == 70)
					strncpy(adjective, "legendary", sizeof(adjective));
				else if (level == 80)
					strncpy(adjective, "epic", sizeof(adjective));
				else
					strncpy(adjective, "too uber for cheerios", sizeof(adjective) - 1);
				sprintf(message, "The %s guild <%s> has attained level %u", adjective, name, level);
				zone_list.TransmitGlobalAnnouncement(message);
			}
		}
		save_needed = true;
		ret = true;
	}
	else if (exp < 0) {
	}
	if (ret && send_packet)
		SendGuildUpdate();
}

void Guild::SetEXPToNextLevel(int64 exp, bool send_packet) {

	exp_to_next_level = exp;
	if (send_packet) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Guild XP to next level: %ul", exp_to_next_level);
		SendGuildUpdate();
		save_needed = true;
	}
}

void Guild::SetRecruitingShortDesc(const char* new_desc, bool send_packet) {

	assert(new_desc);

	recruiting_short_desc = string(new_desc);
	if (send_packet) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Set Guild Recruiting short desc:\n%s", recruiting_short_desc.c_str());
		SendGuildUpdate();
		recruiting_save_needed = true;
	}
}

void Guild::SetRecruitingFullDesc(const char* new_desc, bool send_packet) {

	assert(new_desc);

	recruiting_full_desc = string(new_desc);
	if (send_packet) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Set Guild Recruiting full desc:\n%s", recruiting_full_desc.c_str());
		SendGuildUpdate();
		recruiting_save_needed = true;
	}
}

void Guild::SetRecruitingMinLevel(int8 new_level, bool send_packet) {

	recruiting_min_level = new_level;
	if (send_packet) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Set Guild Recruiting min_level: %i", recruiting_min_level);
		SendGuildUpdate();
		recruiting_save_needed = true;
	}
}

void Guild::SetRecruitingPlayStyle(int8 new_play_style, bool send_packet) {

	recruiting_play_style = new_play_style;
	if (send_packet) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Set Guild Recruiting play style: %i", recruiting_play_style);
		SendGuildUpdate();
		recruiting_save_needed = true;
	}
}

bool Guild::SetRecruitingDescTag(int8 index, int8 tag, bool send_packet) {

	bool ret = false;
	if (index <= 3) {
		recruiting_desc_tags.Put(index, tag);
		ret = true;
	}
	if (ret && send_packet) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Set Guild Recruiting descriptive tag index: %i, tag: %i", index, tag);
		SendGuildUpdate();
		recruiting_save_needed = true;
	}
	return ret;
}

int8 Guild::GetRecruitingDescTag(int8 index) {

	int8 ret = 0;
	if (index <= 3)
		ret = recruiting_desc_tags.Get(index);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Get Guild Recruiting descriptive tag index: %i, value: %i", index, ret);
	return ret;
}

bool Guild::SetPermission(int8 rank, int8 permission, int8 value, bool send_packet, bool save_needed) {

	bool ret = false;
	if (value == 0 || value == 1) {
		if (permissions.count(rank) == 0)
			permissions.Put(rank, new MutexMap<int8, int8>);
		permissions.Get(rank)->Put(permission, value);
		ret = true;
	}
	if (ret && send_packet) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Set Guild Permissions - Rank: %i, Permission: %i, Value: %i", rank, permission, value);
		SendGuildUpdate();
		if(save_needed)
			ranks_save_needed = true;
	}
	return ret;
}

int8 Guild::GetPermission(int8 rank, int8 permission) {

	int8 ret = 0;
	if (permissions.count(rank) > 0 && permissions.Get(rank)->count(permission) > 0)
		ret = permissions.Get(rank)->Get(permission);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Get Guild Permissions - Rank: %i, Permission: %i, Value: %i", rank, permission, ret);
	return ret;
}

bool Guild::SetEventFilter(int8 event_id, int8 category, int8 value, bool send_packet, bool save_needed) {

	bool ret = false;
	if ((category == GUILD_EVENT_FILTER_CATEGORY_RETAIN_HISTORY || category == GUILD_EVENT_FILTER_CATEGORY_BROADCAST) && (value == 0 || value == 1)) {
		if (event_filters.count(event_id) == 0)
			event_filters.Put(event_id, new MutexMap<int8, int8>);
		event_filters.Get(event_id)->Put(category, value);
		ret = true;
	}
	if (ret && send_packet) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Set Guild Event Filter - EventID: %i, Category: %i, Value: %i", event_id, category, value);
		SendGuildUpdate();
		if(save_needed)
			event_filters_save_needed = true;
	}
	return ret;
}

int8 Guild::GetEventFilter(int8 event_id, int8 category) {

	int8 ret = 0;
	if (event_filters.count(event_id) > 0 && event_filters.Get(event_id)->count(category) > 0) {
		ret = event_filters.Get(event_id)->Get(category);
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Get Guild Event Filter - EventID: %i, Category: %i, Value: %i", event_id, category, ret);
	}
	return ret;
}

int32 Guild::GetNumUniqueAccounts() {

	map<int32, GuildMember *>::iterator itr;
	map<int32, bool> accounts;

	mMembers.readlock(__FUNCTION__, __LINE__);
	if (members.size() > 0) {
		for (itr = members.begin(); itr != members.end(); itr++) {
			if (accounts.count(itr->second->account_id) == 0)
				accounts[itr->second->account_id] = true;
		}
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Found %i Unique Account(s) in Guild", accounts.size());
	return accounts.size();
}

int32 Guild::GetNumRecruiters() {

	map<int32, GuildMember*>::iterator itr;
	int32 ret = 0;

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if (itr->second->recruiter_id > 0 && (itr->second->member_flags & GUILD_MEMBER_FLAGS_RECRUITING_FOR_GUILD))
			ret++;
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Found %i Recruiter(s) in Guild", ret);
	return ret;
}

int32 Guild::GetNextRecruiterID() {

	map<int32, GuildMember*>::iterator itr;
	map<int32, bool> tmp;
	int32 i, ret = 0;

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if (itr->second->recruiter_id > 0)
			tmp[itr->second->recruiter_id] = true;
	}
	for (i = 1; i < 0xFFFFFFFF; i++) {
		if (tmp.count(i) == 0) {
			ret = i;
			break;
		}
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Next Guild Recruiter ID: %i", ret);
	return ret;
}

int64 Guild::GetNextEventID() {

	GuildEvent *ge;
	int64 ret = 1;

	if (guild_events.size() > 0) {
		ge = guild_events.front();
		ret = ge->event_id + 1;
	}
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Next Guild Event ID: %i", ret);
	return ret;
}

GuildMember * Guild::GetGuildMemberOnline(Client *client) {

	map<int32, GuildMember*>::iterator itr;
	GuildMember *ret = 0;

	assert(client);

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if (itr->second->character_id == client->GetCharacterID()) {
			ret = itr->second;
			break;
		}
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 3, "Guilds", "Guild Member Online: %s", ret->name);
	return ret;
}

GuildMember * Guild::GetGuildMember(Player *player) {

	assert(player);
	return GetGuildMember(player->GetCharacterID());
}

GuildMember * Guild::GetGuildMember(int32 character_id) {

	GuildMember *ret = 0;

	mMembers.readlock(__FUNCTION__, __LINE__);
	if (members.count(character_id) > 0)
		ret = members[character_id];
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "%s: %i", __FUNCTION__, character_id);
	return ret;
}

GuildMember * Guild::GetGuildMember(const char *player_name)  {

	map<int32, GuildMember*>::iterator itr;
	GuildMember *ret = 0;

	assert(player_name);

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if (!strncmp(player_name, itr->second->name, sizeof(itr->second->name))) {
			ret = itr->second;
			break;
		}
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "%s: %s", __FUNCTION__, player_name);
	return ret;
}

vector<GuildMember *> * Guild::GetGuildRecruiters() {

	map<int32, GuildMember*>::iterator itr;
	vector<GuildMember *> *ret = 0;

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if (itr->second->recruiter_id > 0 && (itr->second->member_flags & GUILD_MEMBER_FLAGS_RECRUITING_FOR_GUILD)) {
			if (!ret)
				ret = new vector<GuildMember *>;
			ret->push_back(itr->second);
			LogWrite(GUILD__DEBUG, 1, "Guilds", "Get Guild Recruiter '%s' (%i)", itr->second->name, itr->second->recruiter_id);
		}
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	
	return ret;
}

GuildEvent * Guild::GetGuildEvent(int64 event_id) {

	deque<GuildEvent*>::iterator itr;
	GuildEvent* ret = 0;

	for (itr = guild_events.begin(); itr != guild_events.end(); itr++) {
		if ((*itr)->event_id == event_id) {
			ret = *itr;
			break;
		}
	}
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Get Guild Event: %s (%lli)", ret->description.c_str(), ret->event_id);
	return ret;
}

bool Guild::SetRankName(int8 rank, const char *name, bool send_packet) {

	assert(name);

	ranks.Put(rank, string(name));

	if (send_packet) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Set Guild Rank Name: %s (%i)", name, rank);
		SendGuildUpdate();
		ranks_save_needed = true;
	}
	return true;
}

const char * Guild::GetRankName(int8 rank) {

	if (ranks.count(rank) > 0) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Get Guild Rank Name: %s (%i)", ranks.Get(rank).c_str(), rank);
		return ranks.Get(rank).c_str();
	}
	return 0;
}

bool Guild::SetRecruitingFlag(int8 flag, int8 value, bool send_packet) {

	bool ret = false;

	if (recruiting_flags.count(flag) > 0 && (value == 0 || value == 1)) {
		recruiting_flags.Put(flag, value);
		ret = true;
	}
	if (ret && send_packet) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Set Guild Recruiting Flag: %i, Value: %i", flag, value);
		SendGuildUpdate();
		recruiting_save_needed = true;
	}
	return ret;
}

int8 Guild::GetRecruitingFlag(int8 flag) {

	int8 value = 0;
	if (recruiting_flags.count(flag) > 0)
		value = recruiting_flags.Get(flag);
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Get Guild Recruiting Flag: %i, Value: %i", flag, value);
	return value;
}

bool Guild::SetGuildRecruiter(Client* client, const char* name, bool value, bool send_packet) {

	GuildMember *gm;
	const char *awarder_name;

	assert(client);
	assert(name);

	if (!(gm = GetGuildMember(name)))
		return false;

	awarder_name = client->GetPlayer()->GetName();
	if (value) {
		gm->recruiter_id = GetNextRecruiterID();
		AddNewGuildEvent(GUILD_EVENT_BECOMES_RECRUITER, "%s awarded %s Guild Recruiting Permission.", Timer::GetUnixTimeStamp(), true, awarder_name, name);
		SendMessageToGuild(GUILD_EVENT_BECOMES_RECRUITER, "%s awarded %s Guild Recruiting Permission.", awarder_name, name);
		LogWrite(GUILD__DEBUG, 0, "Guilds", "%s makes %s a guild recruiter.", awarder_name, name);
	}
	else {
		gm->recruiter_id = 0;
		AddNewGuildEvent(GUILD_EVENT_NO_LONGER_RECRUITER, "%s revoked %s's Guild Recruiting Permission.", Timer::GetUnixTimeStamp(), true, awarder_name, name);
		SendMessageToGuild(GUILD_EVENT_NO_LONGER_RECRUITER, "%s revoked %s's Guild Recruiting Permission.", awarder_name, name);
		LogWrite(GUILD__DEBUG, 0, "Guilds", "%s removes %s from guild recruiters.", awarder_name, name);
	}

	if (send_packet) {
		SendGuildMember(gm);
		member_save_needed = true;
	}

	return true;
}

bool Guild::SetGuildRecruiterDescription(Client *client, const char *description, bool send_packet) {

	GuildMember *gm;

	assert(client);
	assert(description);

	if (!(gm = GetGuildMember(client->GetPlayer())))
		return false;

	gm->recruiter_description = string(description);

	if (send_packet) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Set guild recruiter description:\n%s", gm->recruiter_description.c_str());
		SendGuildRecruiterInfo(client, client->GetPlayer());
		member_save_needed = true;
	}

	return true;
}

bool Guild::ToggleGuildRecruiterAdventureClass(Client *client, bool send_packet) {

	GuildMember *gm;

	assert(client);
		
	if (!(gm = GetGuildMember(client->GetPlayer())))
		return false;

	gm->recruiting_show_adventure_class = (gm->recruiting_show_adventure_class == 0 ? 1 : 0);

	if (send_packet) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Toggle guild recruiter adventure class = %i", gm->recruiting_show_adventure_class);
		SendGuildRecruiterInfo(client, client->GetPlayer());
	}

	return true;
}

bool Guild::SetGuildMemberNote(const char *name, const char *note, bool send_packet) {

	GuildMember *gm;

	assert(name);
	assert(note);	

	if (!(gm = GetGuildMember(name)))
		return false;

	gm->note = string(note);

	if (send_packet) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Set guild member note:\n%s", gm->note.c_str());
		SendGuildMember(gm);
		member_save_needed = true;
	}

	return true;
}

bool Guild::SetGuildOfficerNote(const char *name, const char *note, bool send_packet) {

	GuildMember *gm;

	assert(name);
	assert(note);	

	if (!(gm = GetGuildMember(name)))
		return false;

	gm->officer_note = string(note);

	if (send_packet) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Set guild officer note:\n%s", gm->officer_note.c_str());
		SendGuildMember(gm);
		member_save_needed = true;
	}

	return true;
}

bool Guild::AddNewGuildMember(Client *client, const char *invited_by, int8 rank) {

	Player *player;
	GuildMember *gm;

	assert(client);

	player = client->GetPlayer();
	assert(player);

	if (members.count(player->GetCharacterID()) == 0 && !player->GetGuild() && ((Player*)player)->GetClient()) {
		gm = new GuildMember;

		gm->account_id = ((Player*)player)->GetClient()->GetAccountID();
		gm->character_id = player->GetCharacterID();
		strncpy(gm->name, player->GetName(), sizeof(gm->name));
		gm->guild_status = 0;
		gm->points = 0.0;
		gm->adventure_class = player->GetAdventureClass();
		gm->adventure_level = player->GetLevel();
		gm->tradeskill_class = player->GetTradeskillClass();
		gm->tradeskill_level = player->GetTSLevel();
		gm->rank = rank;
		gm->zone = string(player->GetZone()->GetZoneDescription());
		gm->join_date = Timer::GetUnixTimeStamp();
		gm->last_login_date = gm->join_date;
		gm->recruiter_id = 0;
		gm->member_flags = GUILD_MEMBER_FLAGS_NOTIFY_LOGINS;
		gm->recruiting_show_adventure_class = 1;
		gm->recruiter_picture_data_size = 0;
		gm->recruiter_picture_data = 0;
		mMembers.writelock(__FUNCTION__, __LINE__);
		members[player->GetCharacterID()] = gm;
		mMembers.releasewritelock(__FUNCTION__, __LINE__);
		player->SetGuild(this);
		string subtitle;
		subtitle.append("<").append(GetName()).append(">");
		player->SetSubTitle(subtitle.c_str());

		if (invited_by)
			client->SimpleMessage(CHANNEL_NARRATIVE, "You accept the invite into the guild.");
		else {
			client->SimpleMessage(CHANNEL_NARRATIVE, "You have formed the guild.");
			LogWrite(GUILD__DEBUG, 0, "Guilds", "New Guild formed: %s", GetName());
		}

		client->PlaySound("ui_joined");
		SendGuildUpdate(client);
		SendGuildMember(gm);
		SendGuildMOTD(client);
		SendGuildEventList(client);
		SendGuildBankEventList(client);
		SendAllGuildEvents(client);
		SendGuildMemberList(client);
		if(client->GetVersion() > 561)
			client->GetCurrentZone()->SendUpdateTitles(client->GetPlayer());

		if (invited_by) {
			AddNewGuildEvent(GUILD_EVENT_MEMBER_JOINS, "%s has accepted %s's invitation to join %s.", Timer::GetUnixTimeStamp(), true, player->GetName(), invited_by, GetName());
			SendMessageToGuild(GUILD_EVENT_MEMBER_JOINS, "%s has accepted %s's invitation to join %s.", player->GetName(), invited_by, GetName());
			LogWrite(GUILD__DEBUG, 0, "Guilds", "%s invited %s to join guild: %s", invited_by, player->GetName(), GetName());
		}

		member_save_needed = true;
		
		peer_manager.sendPeersAddGuildMember(gm->character_id, GetID(), (invited_by != nullptr) ? std::string(invited_by) : "", gm->join_date, rank);
	}

	return true;
}

bool Guild::AddNewGuildMember(int32 characterID, const char *invited_by, int32 join_timestamp, int8 rank) {
	GuildMember *gm;

	if (members.count(characterID) == 0) {
		gm = new GuildMember;
		bool foundMember = peer_manager.GetClientGuildDetails(characterID, gm);
		if(!foundMember) {
			LogWrite(GUILD__ERROR, 0, "Guilds", "FAILED TO FIND MEMBER: %s invited %s to join guild: %s", invited_by, gm->name, GetName());
			safe_delete(gm);
			return false;
		}
		gm->rank = rank;
		gm->join_date = join_timestamp;
		gm->last_login_date = gm->join_date;
		mMembers.writelock(__FUNCTION__, __LINE__);
		members[characterID] = gm;
		mMembers.releasewritelock(__FUNCTION__, __LINE__);
		
		if (invited_by) {
			AddNewGuildEvent(GUILD_EVENT_MEMBER_JOINS, "%s has accepted %s's invitation to join %s.", Timer::GetUnixTimeStamp(), true, gm->name, invited_by, GetName());
			SendMessageToGuild(GUILD_EVENT_MEMBER_JOINS, "%s has accepted %s's invitation to join %s.", gm->name, invited_by, GetName());
			LogWrite(GUILD__DEBUG, 0, "Guilds", "%s invited %s to join guild: %s", invited_by, gm->name, GetName());
		}
	}

	return true;
}

bool Guild::AddGuildMember(GuildMember *guild_member) {

	assert(guild_member);

	mMembers.writelock(__FUNCTION__, __LINE__);
	assert(members.count(guild_member->character_id) == 0);
	members[guild_member->character_id] = guild_member;
	mMembers.releasewritelock(__FUNCTION__, __LINE__);

	// spammy
	LogWrite(GUILD__DEBUG, 5, "Guilds", "Added Player: %s (%i) to Guild: %s", guild_member->name, guild_member->character_id, GetName());

	return true;
}

void Guild::RemoveGuildMember(int32 character_id, bool send_packet) {
	GuildMember *gm = 0;
	Client *client;

	mMembers.writelock(__FUNCTION__, __LINE__);
	if (members.count(character_id) > 0) {
		gm = members[character_id];
		members.erase(gm->character_id);
	}
	mMembers.releasewritelock(__FUNCTION__, __LINE__);

	if (gm) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Remove Player: %s (%i) from Guild: %s", members[character_id]->name, character_id, GetName());

		if ((client = zone_list.GetClientByCharID(character_id))) {
			client->GetPlayer()->SetGuild(0);
			client->GetPlayer()->SetSubTitle("");
			client->GetCurrentZone()->SendUpdateTitles(client->GetPlayer());
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You were removed from the guild.");
		}

		database.DeleteGuildMember(this, gm->character_id);

		if (send_packet) {
			SendGuildMemberLeave(gm->character_id);
			SendGuildUpdate();
			SendGuildMemberList();
		}

		AddNewGuildEvent(GUILD_EVENT_MEMBER_LEAVES, "%s left the guild.", Timer::GetUnixTimeStamp(), true, gm->name);
		SendMessageToGuild(GUILD_EVENT_MEMBER_LEAVES, "%s left the guild.", gm->name);

		safe_delete_array(gm->recruiter_picture_data);
		safe_delete(gm);
	}
}

void Guild::RemoveAllGuildMembers() {
	map<int32, GuildMember *>::iterator itr;
	Client *client;

	mMembers.writelock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if ((client = zone_list.GetClientByCharID(itr->second->character_id))) {
			client->GetPlayer()->SetGuild(0);
			client->GetPlayer()->SetSubTitle("");
		}
		safe_delete(itr->second);
	}
	members.clear();
	mMembers.releasewritelock(__FUNCTION__, __LINE__);

	LogWrite(GUILD__DEBUG, 0, "Guilds", "Removed ALL members from Guild: %s", GetName());

}

bool Guild::DemoteGuildMember(Client *client, const char *name, bool send_packet) {

	GuildMember *gm;
	const char *demoter_name;
	bool ret = false;

	assert(client);
	assert(name);

	mMembers.readlock(__FUNCTION__, __LINE__);
	if ((gm = GetGuildMember(name)) && gm->rank != GUILD_RANK_RECRUIT) {
		demoter_name = client->GetPlayer()->GetName();
		gm->rank++;

		AddNewGuildEvent(GUILD_EVENT_MEMBER_DEMOTED, "%s has demoted %s to %s.", Timer::GetUnixTimeStamp(), true, demoter_name, name, ranks.Get(gm->rank).c_str());
		SendMessageToGuild(GUILD_EVENT_MEMBER_DEMOTED, "%s has demoted %s to %s.", demoter_name, name, ranks.Get(gm->rank).c_str());
		LogWrite(GUILD__DEBUG, 0, "Guilds", "%s demoted %s to %s in Guild: %s", demoter_name, name, ranks.Get(gm->rank).c_str(), GetName());

		ret = true;

		if (send_packet) {
			SendGuildMember(gm);
			SendGuildMemberList();
			member_save_needed = true;
		}
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

bool Guild::PromoteGuildMember(Client *client, const char *name, bool send_packet) {

	GuildMember *gm;
	const char *promoter_name;
	bool ret = false;

	assert(client);
	assert(name);

	mMembers.readlock(__FUNCTION__, __LINE__);
	if ((gm = GetGuildMember(name)) && gm->rank != GUILD_RANK_LEADER) {
		promoter_name = client->GetPlayer()->GetName();
		gm->rank--;

		AddNewGuildEvent(GUILD_EVENT_MEMBER_PROMOTED, "%s has promoted %s to %s.", Timer::GetUnixTimeStamp(), true, promoter_name, name, ranks.Get(gm->rank).c_str());
		SendMessageToGuild(GUILD_EVENT_MEMBER_PROMOTED, "%s has promoted %s to %s.", promoter_name, name, ranks.Get(gm->rank).c_str());
		LogWrite(GUILD__DEBUG, 0, "Guilds", "%s promoted %s to %s in Guild: %s", promoter_name, name, ranks.Get(gm->rank).c_str(), GetName());

		if (send_packet) {
			SendGuildMember(gm);
			SendGuildMemberList();
			member_save_needed = true;
		}
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

int32 Guild::KickGuildMember(Client *client, const char *name, bool send_packet) {

	GuildMember *gm;
	Client *kicked_client;
	const char *kicker_name;
	int32 character_id = 0;
	assert(client);
	assert(name);

	if (!(gm = GetGuildMember(name)))
		return 0;

	kicker_name = client->GetPlayer()->GetName();
	character_id = gm->character_id;
	
	if (!strncmp(kicker_name, gm->name, sizeof(gm->name))) {
		AddNewGuildEvent(GUILD_EVENT_MEMBER_LEAVES, "%s left the guild.", Timer::GetUnixTimeStamp(), true, gm->name);
		SendMessageToGuild(GUILD_EVENT_MEMBER_LEAVES, "%s left the guild.", gm->name);
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Player: %s has left guild: %s", gm->name, GetName());
	}
	else {
		AddNewGuildEvent(GUILD_EVENT_MEMBER_LEAVES, "%s kicked %s from the guild.", Timer::GetUnixTimeStamp(), true, kicker_name, gm->name);
		SendMessageToGuild(GUILD_EVENT_MEMBER_LEAVES, "%s kicked %s from the guild.", kicker_name, gm->name);
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Player: %s was kicked from guild: %s by %s.", gm->name, GetName(), kicker_name);
	}

	if ((kicked_client = zone_list.GetClientByCharID(gm->character_id))) {
		kicked_client->GetPlayer()->SetGuild(0);
		kicked_client->GetPlayer()->SetSubTitle("");
		if (!strncmp(client->GetPlayer()->GetName(), gm->name, sizeof(gm->name)))
			kicked_client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You left the guild.");
		else
			kicked_client->Message(CHANNEL_COLOR_YELLOW, "You were kicked from the guild by %s.", kicker_name);
	}

	mMembers.writelock(__FUNCTION__, __LINE__);
	members.erase(gm->character_id);
	mMembers.releasewritelock(__FUNCTION__, __LINE__);

	database.DeleteGuildMember(this, gm->character_id);

	if (send_packet) {
		SendGuildMemberLeave(gm->character_id);
		SendGuildUpdate();
		SendGuildMemberList();
	}

	safe_delete_array(gm->recruiter_picture_data);
	safe_delete(gm);

	return character_id;
}

bool Guild::InvitePlayer(Client *client, const char *name, bool send_packet) {

	Client *client_invite;
	Player *player_invite;
	PacketStruct *packet;

	assert(client);
	assert(name);

	if (!(client_invite = zone_list.GetClientByCharName(string(name)))) {
		client->Message(CHANNEL_NARRATIVE, "%s couldn't be found.", name);
		LogWrite(GUILD__WARNING, 0, "Guilds", "Attempted to invite %s to guild %s: Player Not Found", name, GetName());
		return false;
	}

	player_invite = client_invite->GetPlayer();

	if (player_invite->GetGuild()) {
		client->Message(CHANNEL_NARRATIVE, "%s is already in a guild.", player_invite->GetName());
		LogWrite(GUILD__WARNING, 0, "Guilds", "Attempted to invite %s to guild %s: Already in a guild", player_invite->GetName(), GetName());
		return false;
	}

	if (client_invite->GetPendingGuildInvite()->guild) {
		client->Message(CHANNEL_NARRATIVE, "%s is already considering joining a guild.", player_invite->GetName());
		LogWrite(GUILD__WARNING, 0, "Guilds", "Attempted to invite %s to guild %s: Pending Invite elsewhere", player_invite->GetName(), GetName());
		return false;
	}

	if (!(packet = configReader.getStruct("WS_ReceiveOffer", client_invite->GetVersion())))
		return false;

	packet->setDataByName("type", 2);
	packet->setMediumStringByName("name", client->GetPlayer()->GetName());
	packet->setDataByName("unknown2", 1);
	packet->setMediumStringByName("guild_name", GetName());
	client_invite->QueuePacket(packet->serialize());

	client->Message(CHANNEL_NARRATIVE, "You have invited %s to join %s.", player_invite->GetName(), GetName());
	LogWrite(GUILD__DEBUG, 0, "Guilds", "%s invited %s to guild %s", client->GetPlayer()->GetName(), player_invite->GetName(), GetName());

	client_invite->SetPendingGuildInvite(this, client->GetPlayer());

	safe_delete(packet);
	return true;
}

bool Guild::AddPointsToAll(Client *client, float points, const char *comment, bool send_packet) {

	map<int32, GuildMember *>::iterator itr;
	vector<int32> character_ids;
	GuildMember *gm;
	Client *client_to;

	assert(client);

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		gm = itr->second;
	
		if (!permissions.Get(gm->rank)->Get(GUILD_PERMISSIONS_RECEIVE_POINTS)) {
			LogWrite(GUILD__DEBUG, 0, "Guilds", "PlayerID: %i not allowed to receive points! Skipping...", gm->character_id);
			continue;
		}

		gm->points += points;
		character_ids.push_back(gm->character_id);

		AddPointHistory(gm, Timer::GetUnixTimeStamp(), client->GetPlayer()->GetName(), points, comment);
		if ((client_to = zone_list.GetClientByCharID(gm->character_id)))
		{
			client_to->Message(CHANNEL_GUILD_CHAT, "%s increased your guild member points by %.1f.", client->GetPlayer()->GetName(), points);
			LogWrite(GUILD__DEBUG, 0, "Guilds", "Guild: %s", GetName());
			LogWrite(GUILD__DEBUG, 0, "Guilds", "\tAwarded By: %s +%.1f pts to Player: %s", client->GetPlayer()->GetName(), points, gm->name);
		}

		SendGuildMember(gm); //tmp
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);

	client->Message(CHANNEL_GUILD_CHAT, "Points modified for %u guild members.", character_ids.size());
	if (send_packet) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "%s modified points for %u members. Reason: %s", client->GetPlayer()->GetName(), character_ids.size(), points, comment);
		SendGuildModification(points, &character_ids);
		member_save_needed = true;
		points_history_save_needed = true;
	}

	return true;
}

bool Guild::AddPointsToAllOnline(Client *client, float points, const char *comment, bool send_packet) {

	map<int32, GuildMember *>::iterator itr;
	vector<int32> character_ids;
	GuildMember *gm;
	Client *client_to;

	assert(client);

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		gm = itr->second;

		if (!(client_to = zone_list.GetClientByCharID(gm->character_id))) {
			LogWrite(GUILD__DEBUG, 0, "Guilds", "PlayerID: %i not online to receive points! Skipping...", gm->character_id);
			continue;
		}

		if (!permissions.Get(gm->rank)->Get(GUILD_PERMISSIONS_RECEIVE_POINTS)) {
			LogWrite(GUILD__DEBUG, 0, "Guilds", "PlayerID: %i not allowed to receive points! Skipping...", gm->character_id);
			continue;
		}

		gm->points += points;
		character_ids.push_back(gm->character_id);

		AddPointHistory(gm, Timer::GetUnixTimeStamp(), client->GetPlayer()->GetName(), points, comment);
		client_to->Message(CHANNEL_GUILD_CHAT, "%s increased your guild member points by %.1f.", client->GetPlayer()->GetName(), points);
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Guild: %s", GetName());
		LogWrite(GUILD__DEBUG, 0, "Guilds", "\tAwarded By: %s +%.1f pts to Player: %s", client->GetPlayer()->GetName(), points, gm->name);

		SendGuildMember(gm); //tmp

	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);

	client->Message(CHANNEL_GUILD_CHAT, "Points modified for %u guild members.", character_ids.size());
	if (send_packet) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "%s modified points for %u members. Reason: %s", client->GetPlayer()->GetName(), character_ids.size(), points, comment);
		SendGuildModification(points, &character_ids);
		member_save_needed = true;
		points_history_save_needed = true;
	}

	return true;
}

bool Guild::AddPointsToGroup(Client *client, float points, const char *comment, bool send_packet) {

	deque<GroupMemberInfo*>::iterator itr;
	deque<GroupMemberInfo*>* group_members;
	vector<int32> character_ids;
	GroupMemberInfo *gmi;
	GuildMember *gm;
	
	assert(client);

	if (!client->GetPlayer()->GetGroupMemberInfo()) {
		client->SimpleMessage(CHANNEL_GUILD_CHAT, "Cannot assign points because you aren't in a group.");
		LogWrite(GUILD__DEBUG, 0, "Guilds", "%s tried to assign points for group and failed: Not in a group", client->GetPlayer()->GetName());
		return false;
	}

	mMembers.readlock(__FUNCTION__, __LINE__);
	world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);

	PlayerGroup* group = world.GetGroupManager()->GetGroup(client->GetPlayer()->GetGroupMemberInfo()->group_id);
	if (group)
	{
		group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
		deque<GroupMemberInfo*>* group_members = group->GetMembers();
		for (itr = group_members->begin(); itr != group_members->end(); itr++) {
			gmi = *itr;

			if (!gmi->client)
				continue;

			if (gmi->client->GetPlayer()->GetGuild() != this) {
				LogWrite(GUILD__DEBUG, 0, "Guilds", "PlayerID: %i not in guild to receive group points! Skipping...", gmi->client->GetPlayer()->GetCharacterID());
				continue;
			}

			if (!(gm = members[gmi->client->GetCharacterID()]) || !permissions.Get(gm->rank)->Get(GUILD_PERMISSIONS_RECEIVE_POINTS)) {
				LogWrite(GUILD__DEBUG, 0, "Guilds", "PlayerID: %i not allowed to receive points! Skipping...", gm->character_id);
				continue;
			}

			gm->points += points;
			character_ids.push_back(gm->character_id);

			AddPointHistory(gm, Timer::GetUnixTimeStamp(), client->GetPlayer()->GetName(), points, comment);
			gmi->client->Message(CHANNEL_GUILD_CHAT, "%s increased your guild member points by %.1f.", client->GetPlayer()->GetName(), points);
			LogWrite(GUILD__DEBUG, 0, "Guilds", "Guild: %s", GetName());
			LogWrite(GUILD__DEBUG, 0, "Guilds", "\tAwarded By: %s +%.1f pts to Player: %s", client->GetPlayer()->GetName(), points, gm->name);

			SendGuildMember(gm); //tmp

		}
		group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
	}

	world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
	mMembers.releasereadlock(__FUNCTION__, __LINE__);

	client->Message(CHANNEL_GUILD_CHAT, "Points modified for %u guild members.", character_ids.size());
	if (send_packet) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "%s modified points for %u members. Reason: %s", client->GetPlayer()->GetName(), character_ids.size(), points, comment);
		SendGuildModification(points, &character_ids);
		member_save_needed = true;
		points_history_save_needed = true;
	}

	return true;
}

bool Guild::AddPointsToRaid(Client *client, float points, const char *comment, bool send_packet) {

	assert(client);	
	LogWrite(MISC__TODO, 1, "TODO", "Implement Raiding\n%s, %s, %i", __FILE__, __FUNCTION__, __LINE__);
	client->SimpleMessage(CHANNEL_GUILD_CHAT, "Cannot assign points because you aren't in a raid.");
	return false;
}

bool Guild::AddPointsToGuildMember(Client *client, float points, const char *name, const char *comment, bool send_packet) {

	vector<int32> character_ids;
	GuildMember *gm;
	Client *client_to;

	assert(client);
	assert(name);

	if (!(gm = GetGuildMember(name)))
		return false;

	mMembers.readlock(__FUNCTION__, __LINE__);
	if (!permissions.Get(gm->rank)->Get(GUILD_PERMISSIONS_RECEIVE_POINTS)) {
		mMembers.releasereadlock(__FUNCTION__, __LINE__);
		LogWrite(GUILD__DEBUG, 0, "Guilds", "PlayerID: %i not allowed to receive points! Skipping...", gm->character_id);
		client->Message(CHANNEL_GUILD_CHAT, "%s does not have permission to receive guild points.", gm->name);
		return false;
	}

	gm->points += points;
	character_ids.push_back(gm->character_id);

	if ((client_to = zone_list.GetClientByCharID(gm->character_id))) {
		client_to->Message(CHANNEL_GUILD_CHAT, "%s increased your guild member points by %.1f.", client->GetPlayer()->GetName(), points);
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Guild: %s", GetName());
		LogWrite(GUILD__DEBUG, 0, "Guilds", "\tAwarded By: %s +%.1f pts to Player: %s", client->GetPlayer()->GetName(), points, gm->name);
	}

	LogWrite(GUILD__DEBUG, 0, "Guilds", "%s modified points for 1 guild member. Reason: %s", client->GetPlayer()->GetName(), comment);
	client->SimpleMessage(CHANNEL_GUILD_CHAT, "Points modified for 1 guild member.");
	AddPointHistory(gm, Timer::GetUnixTimeStamp(), client->GetPlayer()->GetName(), points, comment);

	if (send_packet) {
		SendGuildMember(gm); //tmp

		SendGuildModification(points, &character_ids);
		member_save_needed = true;
		points_history_save_needed = true;
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);

	return true;
}

bool Guild::AddPointHistory(GuildMember *guild_member, int32 date, const char *modified_by, float points, const char *comment, bool new_point_history) {
	PointHistory *ph, *ph_delete;
	deque<PointHistory *> *ph_list;

	assert(guild_member);
	assert(modified_by);

	ph = new PointHistory;
	ph->date = date;
	ph->modified_by = string(modified_by);
	ph->points = points;
	if (comment)
		ph->comment = string(comment);
	ph->saved_needed = new_point_history;

	mMembers.readlock(__FUNCTION__, __LINE__);
	ph_list = &guild_member->point_history;
	if (ph_list->size() == GUILD_MAX_POINT_HISTORY) {
		ph_delete = ph_list->back();
		database.DeleteGuildPointHistory(this, guild_member->character_id, ph_delete);
		safe_delete(ph_delete);
		ph_list->pop_back();
	}

	ph_list->push_front(ph);
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	
	return true;
}

void Guild::ViewGuildMemberPoints(Client *client, const char * name) {

	deque<PointHistory *> *ph_list;
	deque<PointHistory *>::iterator itr;
	PointHistory *ph;
	GuildMember *gm;
	PacketStruct *packet;
	int32 i;

	assert(client);
	assert(name);
		
	if (!(gm = GetGuildMember(name)))
		return;

	if (!(packet = configReader.getStruct("WS_RequestGuildEventDetails", client->GetVersion())))
		return;

	mMembers.readlock(__FUNCTION__, __LINE__);
	ph_list = &gm->point_history;
	i = 0;

	packet->setDataByName("account_id", client->GetAccountID());
	packet->setDataByName("character_id", client->GetCharacterID());
	packet->setDataByName("guild_id", id);
	packet->setArrayLengthByName("num_events", ph_list->size());

	// this log entry may be excessive... test it out.
	LogWrite(GUILD__DEBUG, 0, "Guilds", "account_id: %i, character_id: %i, guild_id: %i, num_events: %i", client->GetAccountID(), client->GetCharacterID(), id, ph_list->size());

	for (itr = ph_list->begin(); itr != ph_list->end(); itr++) {
		ph = *itr;
		packet->setArrayDataByName("date", ph->date, i);
		packet->setArrayDataByName("modified_by", ph->modified_by.c_str(), i);
		packet->setArrayDataByName("comment", ph->comment.c_str(), i);
		packet->setArrayDataByName("points", ph->points, i);
		// this log entry may be excessive... test it out.
		LogWrite(GUILD__DEBUG, 0, "Guilds", "date: %i, modified_by: %i, comment: %i, points: %.1f", ph->date, ph->modified_by.c_str(), ph->comment.c_str(), ph->points);
		i++;
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	//DumpPacket(packet->serialize());

	client->QueuePacket(packet->serialize());
	safe_delete(packet);
}

bool Guild::ChangeMemberFlag(Client *client, int8 member_flag, int8 value, bool send_packet) {

	GuildMember *gm;
	bool ret = false;

	assert (client);

	if (!(gm = GetGuildMemberOnline(client)))
		return false;

	mMembers.readlock(__FUNCTION__, __LINE__);
	switch (member_flag) {
		case GUILD_MEMBER_FLAGS_RECRUITING_FOR_GUILD: {
			if (value > 0 && !(gm->member_flags & GUILD_MEMBER_FLAGS_RECRUITING_FOR_GUILD)) {
				gm->member_flags += GUILD_MEMBER_FLAGS_RECRUITING_FOR_GUILD;
				ret = true;
			}		
			else if (value == 0 && gm->member_flags & GUILD_MEMBER_FLAGS_RECRUITING_FOR_GUILD) {
				gm->member_flags -= GUILD_MEMBER_FLAGS_RECRUITING_FOR_GUILD;
				ret = true;
			}
			break;
		}
		case GUILD_MEMBER_FLAGS_NOTIFY_LOGINS: {
			if (value > 0 && !(gm->member_flags & GUILD_MEMBER_FLAGS_NOTIFY_LOGINS)) {
				gm->member_flags += GUILD_MEMBER_FLAGS_NOTIFY_LOGINS;
				client->SimpleMessage(CHANNEL_GUILD_CHAT, "Guild online notifications are now enabled.");
				ret = true;
			}
			else if (value == 0 && gm->member_flags & GUILD_MEMBER_FLAGS_NOTIFY_LOGINS) {
				gm->member_flags -= GUILD_MEMBER_FLAGS_NOTIFY_LOGINS;
				client->SimpleMessage(CHANNEL_GUILD_CHAT, "Guild online notifications are now disabled.");
				ret = true;
			}
			break;
		}
		case GUILD_MEMBER_FLAGS_DONT_GENERATE_EVENTS: {
			if (value > 1 && !(gm->member_flags & GUILD_MEMBER_FLAGS_DONT_GENERATE_EVENTS)) {
				gm->member_flags += GUILD_MEMBER_FLAGS_DONT_GENERATE_EVENTS;
				client->SimpleMessage(CHANNEL_GUILD_CHAT, "Guild events are now disabled for this character.");
				ret = true;
			}
			else if (value == 0 && gm->member_flags & GUILD_MEMBER_FLAGS_DONT_GENERATE_EVENTS) {
				gm->member_flags -= GUILD_MEMBER_FLAGS_DONT_GENERATE_EVENTS;
				client->SimpleMessage(CHANNEL_GUILD_CHAT, "Guild events are now enabled for this character.");
				ret = true;
			}
			break;
		}
		default:
			break;
	}

	if (ret && send_packet) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Guild Member Flag '%i' changed to %i", member_flag, value);
		member_save_needed = true;
		SendGuildMember(client, gm);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}
bool Guild::UpdateGuildStatus(Player *player ,int32 Status) {
	GuildMember *gm;
	assert(player);
	assert(members.count(player->GetCharacterID()) > 0);
	mMembers.readlock(__FUNCTION__, __LINE__);
	gm = members[player->GetCharacterID()];
	gm->guild_status += Status;
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	member_save_needed = true;
	return true;
}
bool Guild::UpdateGuildMemberInfo(Player *player) {

	GuildMember *gm;

	assert(player);
	assert(members.count(player->GetCharacterID()) > 0);

	LogWrite(GUILD__DEBUG, 0, "Guilds", "Updating Guild Member Info for Player: %i", player->GetCharacterID());

	mMembers.readlock(__FUNCTION__, __LINE__);
	gm = members[player->GetCharacterID()];
	gm->adventure_class = player->GetAdventureClass();
	gm->adventure_level = player->GetLevel();
	gm->tradeskill_class = player->GetTradeskillClass();
	gm->tradeskill_level = player->GetTSLevel();
	gm->zone = string(player->GetZone()->GetZoneDescription());
	gm->last_login_date = database.GetCharacterTimeStamp(player->GetCharacterID());
	mMembers.releasereadlock(__FUNCTION__, __LINE__);

	return true;
}

void Guild::AddGuildEvent(int64 event_id, int32 type, const char *description, int32 date, int8 locked) {

	LogWrite(GUILD__DEBUG, 3, "Guilds", "Guild: %s", GetName());
	LogWrite(GUILD__DEBUG, 3, "Guilds", "Add Guild Event: %lli, %i, %s", event_id, type, string(description).c_str());

	GuildEvent *ge;

	assert(description);
//	assert(event_filters.Get(type)->Get(GUILD_EVENT_FILTER_CATEGORY_RETAIN_HISTORY));
	assert(guild_events.size() < GUILD_MAX_EVENTS);

	ge = new GuildEvent;
	ge->event_id = event_id;
	ge->type = type;
	ge->description = string(description);
	ge->date = date;
	ge->locked = locked;
	ge->save_needed = false;
	guild_events.push_back(ge);
}

void Guild::AddNewGuildEvent(int32 type, const char *description, int32 date, bool send_packet, ...) {

	deque<GuildEvent *>::reverse_iterator itr;
	GuildEvent *ge, *current_ge;
	char buffer[4096];
	va_list argptr;

	assert(description);

	va_start(argptr, send_packet);
	vsnprintf(buffer, sizeof(buffer), description, argptr);
	va_end(argptr);

	ge = new GuildEvent;
	ge->event_id = GetNextEventID();
	ge->type = type;
	ge->description = string(buffer);
	ge->date = date;
	ge->locked = 0;

	if (!event_filters.Get(type)->Get(GUILD_EVENT_FILTER_CATEGORY_RETAIN_HISTORY)) {
		database.SaveHiddenGuildEvent(this, ge);
		return;
	}

	if (guild_events.size() == GUILD_MAX_EVENTS) {
		for (itr = guild_events.rbegin(); itr != guild_events.rend(); itr++) {
			current_ge = *itr;

			if (current_ge->locked == 0) {
				database.ArchiveGuildEvent(this, current_ge);
				safe_delete(current_ge);
				guild_events.erase(--itr.base());
				guild_events.push_front(ge);
				break;
			}
		}
	}
	else
		guild_events.push_front(ge);

	if (send_packet) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Some Add New Guild Event thing happened, not sure what...");
		SendNewGuildEvent(ge);
		ge->save_needed = true;
		events_save_needed = true;
	}
}

bool Guild::LockGuildEvent(int64 event_id, bool lock, bool send_packet) {

	bool ret = false;
	GuildEvent* ge = GetGuildEvent(event_id);
	if (ge) {
		if (lock) {
			ge->locked = 1;
			if (send_packet)
				SendGuildEventAction(GUILD_EVENT_ACTION_LOCK, ge);
		}
		else {
			ge->locked = 0;
			if (send_packet)
				SendGuildEventAction(GUILD_EVENT_ACTION_UNLOCK, ge);
		}
		ret = true;
	}
	if (ret && send_packet) {
		LogWrite(GUILD__DEBUG, 0, "Guilds", "Toggle guild event lock, EventID: %lli, value: %i", event_id, lock);
		ge->save_needed = true;
		events_save_needed = true;
	}
	return ret;
}

bool Guild::DeleteGuildEvent(int64 event_id, bool send_packet) {

	bool ret = false;
	deque<GuildEvent*>::iterator itr;
	for (itr = guild_events.begin(); itr != guild_events.end(); itr++) {
		GuildEvent* ge = *itr;
		if (ge->event_id == event_id) {
			if (send_packet)
				SendGuildEventAction(GUILD_EVENT_ACTION_DELETE, ge);
			database.DeleteGuildEvent(this, ge->event_id);
			safe_delete(ge);
			guild_events.erase(itr);
			ret = true;
			break;
		}
	}
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Delete guild event, EventID: %lli", event_id);
	return ret;
}

int32 Guild::GetPermissionsPacketValue(int8 rank, int32 start, int32 end) {

	int32 ret = 0;
	for (int32 i = start; i <= end; i++) {
		if (permissions.count(rank) > 0 && permissions.Get(rank)->count(i) > 0 && permissions.Get(rank)->Get(i)) {
			if (i >= 0 && i <= 31)
				ret += (int32)pow(2.0, (double)i);
			else if (i >= 32 && i <= 63)
				ret += (int32)pow(2.0, (double)(i - 32));
		}
	}
	return ret;
}

int32 Guild::GetEventFilterPacketValue(int8 category, int32 start, int32 end) {

	int32 ret = 0;
	for (int32 i = start; i <= end; i++) {
		if (event_filters.count(i) > 0 && event_filters.Get(i)->count(category) > 0 && event_filters.Get(i)->Get(category)) {
			if (i >= 0 && i <= 31)
				ret += (int32)pow(2.0, (double)i);
			else if (i >= 32 && i <= 63)
				ret += (int32)pow(2.0, (double)(i - 32));
			else if (i >= 64 && i <= 95)
				ret += (int32)pow(2.0, (double)(i - 64));
		}
	}
	return ret;
}

int8 Guild::GetRecruitingLookingForPacketValue() {

	int8 ret = 0;
	MutexMap<int8, int8>::iterator itr = recruiting_flags.begin();
	while (itr.Next()) {
		if (itr.second)
			ret += (int8)pow(2.0, (double)itr.first);
	}
	return ret;
}

void Guild::SendGuildMOTD(Client* client) {

	if (client && strlen(motd) > 0)
		client->Message(CHANNEL_GUILD_MOTD, "Guild MOTD: %s", motd);

	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild MOTD.\n'%s'", motd);
}

void Guild::SendGuildEventList() {

	map<int32, GuildMember*>::iterator itr;
	Client *client;

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if ((client = zone_list.GetClientByCharID(itr->second->character_id)))
			SendGuildEventList(client);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);

	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild Event List (%s).", __FUNCTION__);
}

void Guild::SendGuildEventList(Client* client) {

	if (client) {
		PacketStruct* packet = configReader.getStruct("WS_GuildEventList", client->GetVersion());
		if (packet) {
			packet->setDataByName("account_id", client->GetAccountID());
			packet->setArrayLengthByName("num_events", guild_events.size());
			deque<GuildEvent*>::iterator itr;
			int32 i = 0;
			for (itr = guild_events.begin(); itr != guild_events.end(); itr++) {
				packet->setArrayDataByName("event_id", (*itr)->event_id, i);
				packet->setArrayDataByName("locked", (*itr)->locked, i);
				i++;
			}
			client->QueuePacket(packet->serialize());
			//DumpPacket(packet->serialize());
			safe_delete(packet);
		}
	}
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild Event List (%s).", __FUNCTION__);
}

void Guild::SendGuildEventDetails() {

	map<int32, GuildMember*>::iterator itr;
	Client *client;

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if ((client = zone_list.GetClientByCharID(itr->second->character_id)))
			SendGuildEventDetails(client);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild Event Details (%s).", __FUNCTION__);
}

void Guild::SendGuildEventDetails(Client* client) {

	if (client) {
		PacketStruct* packet = configReader.getStruct("WS_GuildEventDetails", client->GetVersion());
		if (packet) {
			deque<GuildEvent*>::iterator itr;
			int32 i = 0;
			for (itr = guild_events.begin(); itr != guild_events.end(); itr++) {
				packet->setArrayDataByName("event_id", (*itr)->event_id, i);
				i++;
			}
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild Event Details (%s).", __FUNCTION__);
}

void Guild::SendAllGuildEvents() {

	map<int32, GuildMember*>::iterator itr;
	Client *client;

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if ((client = zone_list.GetClientByCharID(itr->second->character_id)))
			SendAllGuildEvents(client);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Sent ALL guild Events (%s).", __FUNCTION__);
}

void Guild::SendAllGuildEvents(Client* client) {

	if (client) {
		deque<GuildEvent*>::iterator itr;
		for (itr = guild_events.begin(); itr != guild_events.end(); itr++)
			SendOldGuildEvent(client, *itr);
	}
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Sent ALL guild Events (%s).", __FUNCTION__);
}

void Guild::SendOldGuildEvent(Client* client, GuildEvent* guild_event) {

	if (client && guild_event) {
		PacketStruct* packet = configReader.getStruct("WS_RequestGuildInfo", client->GetVersion());
		if (packet) {
			packet->setDataByName("account_id", client->GetAccountID());
			packet->setDataByName("event_id", guild_event->event_id);
			packet->setDataByName("date", guild_event->date);
			packet->setDataByName("type", guild_event->type);
			packet->setMediumStringByName("description", guild_event->description.c_str());
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
	LogWrite(GUILD__DEBUG, 3, "Guilds", "Sent OLD guild Events.");
}

void Guild::SendNewGuildEvent(GuildEvent* guild_event) {

	map<int32, GuildMember *>::iterator itr;
	Client *client;

	assert (guild_event);
	
	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if ((client = zone_list.GetClientByCharID(itr->second->character_id)))
			SendNewGuildEvent(client, guild_event);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Sent NEW guild Events. (%s)", __FUNCTION__);
}

void Guild::SendNewGuildEvent(Client* client, GuildEvent* guild_event) {

	if (client && guild_event) {
		PacketStruct* packet = configReader.getStruct("WS_GuildEventAdd", client->GetVersion());
		if (packet) {
			packet->setDataByName("account_id", client->GetAccountID());
			packet->setDataByName("event_id", guild_event->event_id);
			packet->setDataByName("type", guild_event->type);
			packet->setDataByName("date", guild_event->date);
			packet->setDataByName("description", guild_event->description.c_str());
			//DumpPacket(packet->serialize());
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Sent NEW guild Events. (%s)", __FUNCTION__);
}

void Guild::SendGuildEventAction(int8 action, GuildEvent* guild_event) {

	map<int32, GuildMember *>::iterator itr;
	Client *client;

	assert(guild_event);

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if ((client = zone_list.GetClientByCharID(itr->second->character_id)))
			SendGuildEventAction(client, action, guild_event);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);

	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild events Action. (%s)", __FUNCTION__);
}

void Guild::SendGuildEventAction(Client* client, int8 action, GuildEvent* guild_event) {

	if (guild_event) {
		PacketStruct* packet = configReader.getStruct("WS_GuildEventAction", client->GetVersion());
		if (packet) {
			packet->setDataByName("account_id", client->GetAccountID());
			packet->setDataByName("event_id", guild_event->event_id);
			packet->setDataByName("action", action);
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}

	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild events Action. (%s)", __FUNCTION__);
}

void Guild::SendGuildBankEventList() {

	map<int32, GuildMember*>::iterator itr;
	Client *client;

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if ((client = zone_list.GetClientByCharID(itr->second->character_id)))
			SendGuildBankEventList(client);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild bank events list. (%s)", __FUNCTION__);
}

void Guild::SendGuildBankEventList(Client* client) {

	if (client) {
		for (int32 i = 0; i < 4; i++) {
			PacketStruct* packet = configReader.getStruct("WS_GuildBankEventList", client->GetVersion());
			if (packet) {
				packet->setDataByName("account_id", client->GetAccountID());
				packet->setDataByName("bank_number", i);
				packet->setArrayLengthByName("num_events", banks[i].events.size());
				deque<GuildBankEvent*>::iterator itr;
				for (itr = banks[i].events.begin(); itr != banks[i].events.end(); itr++)
					packet->setArrayDataByName("event_id", (*itr)->event_id, i);
				//DumpPacket(packet->serialize());
				client->QueuePacket(packet->serialize());
				safe_delete(packet);
			}
		}
	}
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild bank events list. (%s)", __FUNCTION__);
}

void Guild::SendGuildUpdate() {

	map<int32, GuildMember*>::iterator itr;
	Client *client;

	LogWrite(GUILD__DEBUG, 1, "Guilds", "SendGuildUpdate to all guild member clients online... (%s)", __FUNCTION__);
	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if ((client = zone_list.GetClientByCharID(itr->second->character_id)))
			SendGuildUpdate(client);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
}

void Guild::SendGuildUpdate(Client* client) {

	if (client) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "SendGuildUpdate to client online... (%s)", __FUNCTION__);

		PacketStruct* packet = configReader.getStruct("WS_GuildUpdate", client->GetVersion());
		if (packet) {
			packet->setMediumStringByName("guild_name", GetName());
			packet->setMediumStringByName("guild_motd", motd);
			packet->setDataByName("guild_id", id);
			packet->setDataByName("guild_level", level);
			packet->setDataByName("unknown", 1);
			packet->setDataByName("formed_date", formed_date);
			packet->setDataByName("unique_accounts", GetNumUniqueAccounts());
			packet->setDataByName("num_members", members.size());
			packet->setDataByName("exp_current", exp_current);
			packet->setDataByName("exp_to_next_level", exp_to_next_level);
			packet->setDataByName("event_filter_retain1", GetEventFilterPacketValue(GUILD_EVENT_FILTER_CATEGORY_RETAIN_HISTORY, 0, 31));
			packet->setDataByName("event_filter_retain2", GetEventFilterPacketValue(GUILD_EVENT_FILTER_CATEGORY_RETAIN_HISTORY, 32, 63));
			packet->setDataByName("event_filter_retain3", GetEventFilterPacketValue(GUILD_EVENT_FILTER_CATEGORY_RETAIN_HISTORY, 63, 92));
			packet->setDataByName("event_filter_retain4", 0);
			packet->setDataByName("event_filter_broadcast1", GetEventFilterPacketValue(GUILD_EVENT_FILTER_CATEGORY_BROADCAST, 0, 31));
			packet->setDataByName("event_filter_broadcast2", GetEventFilterPacketValue(GUILD_EVENT_FILTER_CATEGORY_BROADCAST, 32, 63));
			packet->setDataByName("event_filter_broadcast3", GetEventFilterPacketValue(GUILD_EVENT_FILTER_CATEGORY_BROADCAST, 64, 92));
			packet->setDataByName("event_filter_broadcast4", 0);
			packet->setDataByName("recruiting_looking_for", GetRecruitingLookingForPacketValue());
			packet->setDataByName("recruiting_desc_tag1", GetRecruitingDescTag(0));
			packet->setDataByName("recruiting_desc_tag2", GetRecruitingDescTag(1));
			packet->setDataByName("recruiting_desc_tag3", GetRecruitingDescTag(2));
			packet->setDataByName("recruiting_desc_tag4", GetRecruitingDescTag(3));
			packet->setDataByName("recruiting_playstyle", recruiting_play_style);
			packet->setDataByName("recruiting_min_level", recruiting_min_level);
			packet->setMediumStringByName("recuiting_short_description", recruiting_short_desc.c_str());
			packet->setMediumStringByName("recruiting_full_description", recruiting_full_desc.c_str());
			packet->setMediumStringByName("rank0_name", ranks.Get(GUILD_RANK_LEADER).c_str());
			packet->setDataByName("rank0_permissions1", GetPermissionsPacketValue(GUILD_RANK_LEADER, 0, 31));
			packet->setDataByName("rank0_permissions2", GetPermissionsPacketValue(GUILD_RANK_LEADER, 32, 44));
			packet->setDataByName("rank0_permissions_unused", 0);
			packet->setMediumStringByName("rank1_name", ranks.Get(GUILD_RANK_SENIOR_OFFICER).c_str());
			packet->setDataByName("rank1_permissions1", GetPermissionsPacketValue(GUILD_RANK_SENIOR_OFFICER, 0, 31));
			packet->setDataByName("rank1_permissions2", GetPermissionsPacketValue(GUILD_RANK_SENIOR_OFFICER, 32, 44));
			packet->setDataByName("rank1_permissions_unused", 0);
			packet->setMediumStringByName("rank2_name", ranks.Get(GUILD_RANK_OFFICER).c_str());
			packet->setDataByName("rank2_permissions1", GetPermissionsPacketValue(GUILD_RANK_OFFICER, 0, 31));
			packet->setDataByName("rank2_permissions2", GetPermissionsPacketValue(GUILD_RANK_OFFICER, 32, 44));
			packet->setDataByName("rank2_permissions_unused", 0);
			packet->setMediumStringByName("rank3_name", ranks.Get(GUILD_RANK_SENIOR_MEMBER).c_str());
			packet->setDataByName("rank3_permissions1", GetPermissionsPacketValue(GUILD_RANK_SENIOR_MEMBER, 0, 31));
			packet->setDataByName("rank3_permissions2", GetPermissionsPacketValue(GUILD_RANK_SENIOR_MEMBER, 32, 44));
			packet->setDataByName("rank3_permissions_unused", 0);
			packet->setMediumStringByName("rank4_name", ranks.Get(GUILD_RANK_MEMBER).c_str());
			packet->setDataByName("rank4_permissions1", GetPermissionsPacketValue(GUILD_RANK_MEMBER, 0, 31));
			packet->setDataByName("rank4_permissions2", GetPermissionsPacketValue(GUILD_RANK_MEMBER, 32, 44));
			packet->setDataByName("rank4_permissions_unused", 0);
			packet->setMediumStringByName("rank5_name", ranks.Get(GUILD_RANK_JUNIOR_MEMBER).c_str());
			packet->setDataByName("rank5_permissions1", GetPermissionsPacketValue(GUILD_RANK_JUNIOR_MEMBER, 0, 31));
			packet->setDataByName("rank5_permissions2", GetPermissionsPacketValue(GUILD_RANK_JUNIOR_MEMBER, 32, 44));
			packet->setDataByName("rank5_permissions_unused", 0);
			packet->setMediumStringByName("rank6_name", ranks.Get(GUILD_RANK_INITIATE).c_str());
			packet->setDataByName("rank6_permissions1", GetPermissionsPacketValue(GUILD_RANK_INITIATE, 0, 31));
			packet->setDataByName("rank6_permissions2", GetPermissionsPacketValue(GUILD_RANK_INITIATE, 32, 44));
			packet->setDataByName("rank6_permissions_unused", 0);
			packet->setMediumStringByName("rank7_name", ranks.Get(GUILD_RANK_RECRUIT).c_str());
			packet->setDataByName("rank7_permissions1", GetPermissionsPacketValue(GUILD_RANK_RECRUIT, 0, 31));
			packet->setDataByName("rank7_permissions2", GetPermissionsPacketValue(GUILD_RANK_RECRUIT, 32, 44));
			packet->setDataByName("rank7_permissions_unused", 0);
			packet->setMediumStringByName("bank1_name", banks[0].name.c_str());
			packet->setMediumStringByName("bank2_name", banks[1].name.c_str());
			packet->setMediumStringByName("bank3_name", banks[2].name.c_str());
			packet->setMediumStringByName("bank4_name", banks[3].name.c_str());
			EQ2Packet* pack = packet->serialize();
			//DumpPacket(pack);
			client->QueuePacket(pack);
			safe_delete(packet);
		}
	}
}

void Guild::SendGuildMemberList() {

	map<int32, GuildMember*>::iterator itr;
	Client *client;

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if ((client = zone_list.GetClientByCharID(itr->second->character_id)))
			SendGuildMemberList(client);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild member list to all clients.");
}

void Guild::SendGuildMemberList(Client* client) {

	map<int32, GuildMember *>::iterator itr;
	GuildMember *gm;

	if (client) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild member list to a client.");
		PacketStruct* packet = configReader.getStruct("WS_GuildMembershipResponse", client->GetVersion());
		if (packet) {
			packet->setDataByName("guild_id", id);
			packet->setDataByName("character_id_to", client->GetCharacterID());

			mMembers.readlock(__FUNCTION__, __LINE__);
			packet->setArrayLengthByName("num_members", members.size());
			int32 i = 0;
			for (itr = members.begin(); itr != members.end(); itr++) {
				gm = itr->second;
				packet->setArrayDataByName("account_id", gm->account_id, i);
				packet->setArrayDataByName("character_id", gm->character_id, i);
				packet->setArrayDataByName("name", gm->name, i);
				packet->setArrayDataByName("unknown2", 0, i);
				packet->setArrayDataByName("unknown3", 1, i);
				packet->setArrayDataByName("adventure_class", gm->adventure_class, i);
				packet->setArrayDataByName("adventure_level", gm->adventure_level, i);
				packet->setArrayDataByName("tradeskill_class", gm->tradeskill_class, i);
				packet->setArrayDataByName("tradeskill_level", gm->tradeskill_level, i);
				packet->setArrayDataByName("rank", gm->rank, i);
				packet->setArrayDataByName("member_flags", gm->member_flags, i);
				packet->setArrayDataByName("join_date", gm->join_date, i);
				packet->setArrayDataByName("guild_status", gm->guild_status, i);
				packet->setArrayDataByName("last_login", gm->last_login_date, i);
				packet->setArrayDataByName("recruiter_id", gm->recruiter_id, i);
				packet->setArrayDataByName("points", gm->points, i);
				if (zone_list.GetClientByCharID(gm->character_id))
					packet->setArrayDataByName("zone", gm->zone.c_str(), i);
				packet->setArrayDataByName("note", gm->note.c_str(), i);
				packet->setArrayDataByName("officer_note", gm->officer_note.c_str(), i);
				i++;
			}
			mMembers.releasereadlock(__FUNCTION__, __LINE__);
			//DumpPacket(packet->serialize());
			//packet->PrintPacket();
			EQ2Packet* pack = packet->serialize();
			//DumpPacket(pack);
			client->QueuePacket(pack);
			safe_delete(packet);
		}
	}
}

void Guild::SendGuildMember(Player* player, bool include_zone) {

	map<int32, GuildMember *>::iterator itr;
	Client *client;
	GuildMember *gm;

	assert(player);

	mMembers.readlock(__FUNCTION__, __LINE__);
	if (members.count(player->GetCharacterID()) > 0) {
		gm = members[player->GetCharacterID()];
		
		for (itr = members.begin(); itr != members.end(); itr++) {
			if ((client = zone_list.GetClientByCharID(itr->second->character_id)))
				SendGuildMember(client, gm, include_zone);
		}
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild member.");
}

void Guild::SendGuildMember(GuildMember* gm, bool include_zone) {

	map<int32, GuildMember *>::iterator itr;
	Client *client;

	assert(gm);

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if ((client = zone_list.GetClientByCharID(itr->second->character_id)))
			SendGuildMember(client, gm, include_zone);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild member.");
}

void Guild::SendGuildMember(Client* client, GuildMember* gm, bool include_zone) {

	if (client && gm) {
		PacketStruct* packet = configReader.getStruct("WS_JoinGuildNotify", client->GetVersion());
		if (packet) {
			packet->setDataByName("guild_id", id);
			packet->setDataByName("character_id", gm->character_id);
			packet->setDataByName("account_id", gm->account_id);
			packet->setMediumStringByName("name", gm->name);
			packet->setDataByName("unknown2", 0);
			packet->setDataByName("unknown3", 1);
			packet->setDataByName("adventure_class", gm->adventure_class);
			packet->setDataByName("adventure_level", gm->adventure_level);
			packet->setDataByName("tradeskill_class", gm->tradeskill_class);
			packet->setDataByName("tradeskill_level", gm->tradeskill_level);
			packet->setDataByName("rank", gm->rank);
			packet->setDataByName("member_flags", gm->member_flags);
			packet->setDataByName("join_date", gm->join_date);
			packet->setDataByName("guild_status", gm->guild_status);
			packet->setDataByName("last_login", gm->last_login_date);
			packet->setDataByName("recruiter_id", gm->recruiter_id);
			packet->setDataByName("points", gm->points);
			packet->setMediumStringByName("note", gm->note.c_str());
			packet->setMediumStringByName("officer_note", gm->officer_note.c_str());
			if (include_zone && zone_list.GetClientByCharID(gm->character_id)) {
				packet->setMediumStringByName("zone", gm->zone.c_str());
			//DumpPacket(packet->serialize());
			}
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild member to a client.");
}

void Guild::SendGuildModification(float points, vector<int32>* character_ids) {

	map<int32, GuildMember *>::iterator itr;
	Client *client;

	if (character_ids) {
		mMembers.readlock(__FUNCTION__, __LINE__);
		for (itr = members.begin(); itr != members.end(); itr++) {
			if ((client = zone_list.GetClientByCharID(itr->second->character_id)))
				SendGuildModification(client, points, character_ids);
		}
		mMembers.releasereadlock(__FUNCTION__, __LINE__);
	}
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild modification to all clients.");
}

void Guild::SendGuildModification(Client* client, float points, vector<int32>* character_ids) {

	if (client && character_ids) {
		PacketStruct* packet = configReader.getStruct("WS_ModifyGuild", client->GetVersion());
		if (packet) {
			packet->setDataByName("guild_id", id);
			packet->setDataByName("unknown2", 0xFFFFFFFF);
			packet->setDataByName("points", points);
			packet->setArrayLengthByName("num_character_ids", character_ids->size());
			for (int32 i = 0; i < character_ids->size(); i++)
				packet->setArrayDataByName("character_id", character_ids->at(i), i);
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild modification to a client.");
}

void Guild::GuildMemberLogin(Client *client, bool first_login) {

	map<int32, GuildMember*>::iterator itr;
	Client *client_to;
	char buf[128];

	assert(client);


	UpdateGuildMemberInfo(client->GetPlayer());
	if (first_login)
		SendGuildMOTD(client);
	if(client->GetVersion() > 561) {
		if (first_login)
			SendGuildMember(client->GetPlayer(), false);
	}
	SendGuildRecruiterInfo(client, client->GetPlayer());
	SendGuildEventList(client);
	SendGuildBankEventList(client);
	SendGuildMember(client->GetPlayer());
	SendGuildEventDetails(client);
	SendGuildUpdate(client);
		
	uchar blah5[] = {/*0xFF,0x09,0x01,*/0x01,0x00,0x00,0x00,0x00,0x00,0x00};
	uchar blah6[] = {/*0xFF,0x09,0x01,*/0x01,0x00,0x00,0x00,0x01,0x00,0x00};
	uchar blah7[] = {/*0xFF,0x09,0x01,*/0x01,0x00,0x00,0x00,0x02,0x00,0x00};
	uchar blah8[] = {/*0xFF,0x09,0x01,*/0x01,0x00,0x00,0x00,0x03,0x00,0x00};

	//DumpPacket(blah5, sizeof(blah5));
	//DumpPacket(blah6, sizeof(blah6));
	//DumpPacket(blah7, sizeof(blah7));
	//DumpPacket(blah8, sizeof(blah8));

	if(client->GetVersion() > 561) {
		client->QueuePacket(new EQ2Packet(OP_RequestGuildBankEventDetailsMs, blah5, sizeof(blah5)));
		client->QueuePacket(new EQ2Packet(OP_RequestGuildBankEventDetailsMs, blah6, sizeof(blah6)));
		client->QueuePacket(new EQ2Packet(OP_RequestGuildBankEventDetailsMs, blah7, sizeof(blah7)));
		client->QueuePacket(new EQ2Packet(OP_RequestGuildBankEventDetailsMs, blah8, sizeof(blah8)));
	}
	
	if (first_login)
		SendAllGuildEvents(client);
	if(client->GetVersion() > 561) {
		SendGuildMemberList(client);
	}
	
	if (first_login) {
		snprintf(buf, sizeof(buf), "Guildmate: %s has logged in", client->GetPlayer()->GetName());
		
		mMembers.readlock(__FUNCTION__, __LINE__);
		for (itr = members.begin(); itr != members.end(); itr++) {
			if ((client_to = zone_list.GetClientByCharID(itr->second->character_id)))
				client_to->SimpleMessage(CHANNEL_GUILD_MEMBER_ONLINE, buf);
		}
		mMembers.releasereadlock(__FUNCTION__, __LINE__);
	}

	if (first_login){
		uchar blah1[] = {/*0xFF,0x09,0x01,*/0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		uchar blah2[] = {/*0xFF,0x09,0x01,*/0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00};
		uchar blah3[] = {/*0xFF,0x09,0x01,*/0x01,0x00,0x00,0x00,0x02,0x00,0x00,0x00};
		uchar blah4[] = {/*0xFF,0x09,0x01,*/0x01,0x00,0x00,0x00,0x03,0x00,0x00,0x00};

		//DumpPacket(blah1, sizeof(blah1));
		//DumpPacket(blah2, sizeof(blah2));
		//DumpPacket(blah3, sizeof(blah3));
		//DumpPacket(blah4, sizeof(blah4));

		client->QueuePacket(new EQ2Packet(OP_GuildBankUpdateMsg, blah1, sizeof(blah1)));
		client->QueuePacket(new EQ2Packet(OP_GuildBankUpdateMsg, blah2, sizeof(blah2)));
		client->QueuePacket(new EQ2Packet(OP_GuildBankUpdateMsg, blah3, sizeof(blah3)));
		client->QueuePacket(new EQ2Packet(OP_GuildBankUpdateMsg, blah4, sizeof(blah4)));
	}
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Guild Member logged in.");
}

void Guild::GuildMemberLogoff(Player *player) {

	map<int32, GuildMember*>::iterator itr;
	GuildMember *gm;
	Client *client;
	char buf[128];

	assert(player);

	mMembers.readlock(__FUNCTION__, __LINE__);
	if ((gm = GetGuildMember(player))) {
		snprintf(buf, sizeof(buf), "Guildmate: %s has logged out", player->GetName());
		gm->zone.clear();
		
		for (itr = members.begin(); itr != members.end(); itr++) {
			if ((client = zone_list.GetClientByCharID(itr->second->character_id))) {
				SendGuildMember(client, gm, false);
				client->SimpleMessage(CHANNEL_GUILD_MEMBER_ONLINE, buf);
			}
		}
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Guild Member logged out.");
}

void Guild::SendGuildMemberLeave(int32 character_id) {

	map<int32, GuildMember*>::iterator itr;
	Client *client;

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if ((client = zone_list.GetClientByCharID(itr->second->character_id)))
			SendGuildMemberLeave(client, character_id);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild member left the guild to all clients.");
}

void Guild::SendGuildMemberLeave(Client* client, int32 character_id) {

	PacketStruct* packet = configReader.getStruct("WS_LeaveGuildNotify", client->GetVersion());
	if (packet) {
		packet->setDataByName("guild_id", id);
		packet->setDataByName("character_id", character_id);
		client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
	LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild member left the guild to a client.");
}

void Guild::SendGuildRecruitingDetails(Client* client) {

	if (client) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild recruiting details to a client.");
		PacketStruct* packet = configReader.getStruct("WS_GuildRecruitingDetails", client->GetVersion());
		if (packet) {
			vector<GuildMember*>* recruiters = GetGuildRecruiters();
			vector<GuildMember*>::iterator itr;
			packet->setDataByName("guild_id", id);
			packet->setDataByName("recruiting_full_description", recruiting_full_desc.c_str());
			if (recruiters) {
				int32 i = 0;
				packet->setArrayLengthByName("num_recruiters", recruiters->size());
				for (itr = recruiters->begin(); itr != recruiters->end(); itr++) {
					GuildMember* gm = *itr;
					packet->setArrayDataByName("adventure_class", gm->adventure_class, i);
					packet->setArrayDataByName("adventure_level", gm->adventure_level, i);
					packet->setArrayDataByName("tradeskill_class", gm->tradeskill_class, i);
					packet->setArrayDataByName("tradeskill_level", gm->tradeskill_level, i);
					packet->setArrayDataByName("show_adventure_class", gm->recruiting_show_adventure_class, i);
					packet->setArrayDataByName("unknown2", 4, i);
					packet->setArrayDataByName("unknown3", 2, i);
					packet->setSubArrayLengthByName("num_bytes", gm->recruiter_picture_data_size, i);
					if (gm->recruiter_picture_data_size > 0) {
						for (int16 j = 0; j < gm->recruiter_picture_data_size; j++)
							packet->setSubArrayDataByName("picture_byte", gm->recruiter_picture_data[j], i, j);
					}
					packet->setArrayDataByName("char_name", gm->name, i);
					packet->setArrayDataByName("recruiter_description", gm->recruiter_description.c_str(), i);
					i++;
				}
				safe_delete(recruiters);
			}
			//DumpPacket(packet->serialize());
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
}

void Guild::SendGuildRecruitingImages(Client* client) {
	if (client) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild recruiting images to a client.");
		PacketStruct* packet = configReader.getStruct("WS_GuildRecruitingImage", client->GetVersion());
		if (packet) {
			vector<GuildMember*>* recruiters = GetGuildRecruiters();
			packet->setDataByName("guild_id", id);
			if (recruiters && recruiters->size() > 0) {
				GuildMember* gm = recruiters->at(0);
				packet->setArrayLengthByName("num_bytes", gm->recruiter_picture_data_size);
				if (gm->recruiter_picture_data_size > 0) {
					for (int16 i = 0; i < gm->recruiter_picture_data_size; i++)
						packet->setArrayDataByName("picture_byte", gm->recruiter_picture_data[i], i);
				}
				safe_delete(recruiters);
			}
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
}

void Guild::SendGuildRecruiterInfo(Client* client, Player* player) {

	if (client && player) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "Sent guild recruiter info to a client.");
		GuildMember* gm = GetGuildMember(player);
		if (gm) {
			PacketStruct* packet = configReader.getStruct("WS_GuildRecruitingMemberInfo", client->GetVersion());
			if (packet) {
				packet->setDataByName("character_id", gm->character_id);
				packet->setDataByName("unknown", 1);
				packet->setDataByName("adventure_class", gm->adventure_class);
				packet->setDataByName("adventure_level", gm->adventure_level);
				packet->setDataByName("tradeskill_class", gm->tradeskill_class);
				packet->setDataByName("tradeskill_level", gm->tradeskill_level);
				packet->setDataByName("show_adventure_class", gm->recruiting_show_adventure_class);
				packet->setDataByName("unknown3", 0);

				// hack!
				gm->recruiter_picture_data_size = 0;
				packet->setArrayLengthByName("num_bytes", gm->recruiter_picture_data_size);
				if (gm->recruiter_picture_data_size > 0) {
					for (int16 i = 0; i < gm->recruiter_picture_data_size; i++)
						packet->setArrayDataByName("picture_byte", gm->recruiter_picture_data[i], i);
				}
				
				packet->setMediumStringByName("recruiter_name", gm->name);
				packet->setMediumStringByName("recruiter_description", gm->recruiter_description.c_str());
				//DumpPacket(packet->serialize());
				client->QueuePacket(packet->serialize());
				safe_delete(packet);
			}
		}
	}
}

bool Guild::HandleGuildSay(Client* sender, const char* message) {

	map<int32, GuildMember *>::iterator itr;
	GuildMember *gm;
	Client *client;

	assert(sender);
	assert(message);

	if (!(gm = GetGuildMemberOnline(sender)))
		return false;

	if (!permissions.Get(gm->rank)->Get(GUILD_PERMISSIONS_SPEAK_IN_GUILD_CHAT)) {
		sender->SimpleMessage(CHANNEL_NARRATIVE, "You do not have permission to speak in guild chat.");
		return false;
	}

	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if (!(client = zone_list.GetClientByCharID(itr->second->character_id)))
			continue;
			
		if (permissions.Get(itr->second->rank)->Get(GUILD_PERMISSIONS_SEE_GUILD_CHAT))
			client->GetCurrentZone()->HandleChatMessage(client, sender->GetPlayer(), client->GetPlayer()->GetName(), CHANNEL_GUILD_SAY, message, 0, 0, false, sender->GetPlayer()->GetCurrentLanguage());
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Guild Say");
	
	return true;
}

void Guild::HandleGuildSay(std::string senderName, const char* message, int8 language) {

	map<int32, GuildMember *>::iterator itr;
	GuildMember *gm;

	assert(message);
	Client* client = 0;
	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if (!(client = zone_list.GetClientByCharID(itr->second->character_id)))
			continue;
			
		if (permissions.Get(itr->second->rank)->Get(GUILD_PERMISSIONS_SEE_GUILD_CHAT))
			client->GetCurrentZone()->HandleChatMessage(senderName, client->GetPlayer()->GetName(), CHANNEL_GUILD_SAY, message, 0, 0, language);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Guild Say");
}

bool Guild::HandleOfficerSay(Client* sender, const char* message) {

	map<int32, GuildMember *>::iterator itr;
	GuildMember *gm;
	Client *client;

	assert(sender);
	assert(message);

	if (!(gm = GetGuildMemberOnline(sender)))
		return false;

	if (!permissions.Get(gm->rank)->Get(GUILD_PERMISSIONS_SPEAK_IN_OFFICER_CHAT)) {
		sender->SimpleMessage(CHANNEL_NARRATIVE, "You do not have permission to speak in officer chat.");
		return false;
	}
				
	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if (!(client = zone_list.GetClientByCharID(itr->second->character_id)))
			continue;

		if (permissions.Get(itr->second->rank)->Get(GUILD_PERMISSIONS_SEE_OFFICER_CHAT))
			client->GetCurrentZone()->HandleChatMessage(client, sender->GetPlayer(), client->GetPlayer()->GetName(), CHANNEL_OFFICER_SAY, message, 0, 0, false, sender->GetPlayer()->GetCurrentLanguage());
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Officer Say");
	return true;
}

void Guild::HandleOfficerSay(std::string senderName, const char* message, int8 language) {

	map<int32, GuildMember *>::iterator itr;
	Client *client;
			
	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if (!(client = zone_list.GetClientByCharID(itr->second->character_id)))
			continue;

		if (permissions.Get(itr->second->rank)->Get(GUILD_PERMISSIONS_SEE_OFFICER_CHAT))
			client->GetCurrentZone()->HandleChatMessage(senderName, client->GetPlayer()->GetName(), CHANNEL_OFFICER_SAY, message, 0, 0, language);

	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Officer Say");
}

void Guild::SendMessageToGuild(int8 event_type, const char* message, ...) {

	map<int32, GuildMember *>::iterator itr;
	Client *client;
	va_list argptr;
	char buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);
	
	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if (!(client = zone_list.GetClientByCharID(itr->second->character_id)))
			continue;

		if (event_filters.Get(itr->second->rank)->Get(GUILD_EVENT_FILTER_CATEGORY_BROADCAST))
			client->SimpleMessage(CHANNEL_GUILD_EVENT, buffer);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Sent message to entire guild.");
}

void Guild::SendGuildChatMessage(const char* message, ...) {

	map<int32, GuildMember *>::iterator itr;
	Client *client;
	va_list argptr;
	char buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);
	
	mMembers.readlock(__FUNCTION__, __LINE__);
	for (itr = members.begin(); itr != members.end(); itr++) {
		if (!(client = zone_list.GetClientByCharID(itr->second->character_id)))
			continue;

		if (event_filters.Get(itr->second->rank)->Get(GUILD_EVENT_FILTER_CATEGORY_BROADCAST))
			client->SimpleMessage(CHANNEL_GUILD_CHAT, buffer);
	}
	mMembers.releasereadlock(__FUNCTION__, __LINE__);
	LogWrite(GUILD__DEBUG, 0, "Guilds", "Sent message to entire guild.");
}

string Guild::GetEpicMobDeathMessage(const char* player_name, const char* mob_name) {

	char message[256];
	int8 choice;
	
	assert(player_name);
	assert(mob_name);

	choice = (rand() % 5) + 1;
	if (choice == 1)
		snprintf(message, sizeof(message), "%s was slain by %s in a thunderous engagement!", mob_name, player_name);
	else if (choice == 2)
		snprintf(message, sizeof(message), "%s was slain by %s in a titanic struggle!", mob_name, player_name);
	else if (choice == 3)
		snprintf(message, sizeof(message), "%s was slain by %s's heroic might!", mob_name, player_name);
	else if (choice == 4)
		snprintf(message, sizeof(message), "%s slew %s in an earth shaking battle!", player_name, mob_name);
	else 
		snprintf(message, sizeof(message), "%s slew %s in a heroic clash!", player_name, mob_name);

	LogWrite(GUILD__DEBUG, 0, "Guilds", "Guild Epic Mob Death message sent.");
	return string(message);
}

/***************************************************************************************************************************************************************
 *																												GUILDLIST
 ***************************************************************************************************************************************************************/

GuildList::GuildList() {
}

GuildList::~GuildList() {

	MutexMap<int32, Guild*>::iterator itr = guild_list.begin();
	while (itr.Next())
		safe_delete(itr.second);
}

bool GuildList::AddGuild(Guild* guild) {

	bool ret = false;
	if (guild && guild_list.count(guild->GetID()) == 0) {
		guild_list.Put(guild->GetID(), guild);
		ret = true;
	}
	return ret;
}

Guild* GuildList::GetGuild(int32 guild_id) {

	Guild* ret = 0;
	if (guild_list.count(guild_id) > 0)
		ret = guild_list.Get(guild_id);
	return ret;
}

Guild* GuildList::GetGuild(const char* guild_name) {

	Guild* ret = 0;
	MutexMap<int32, Guild*>::iterator itr = guild_list.begin();
	while (itr.Next()) {
		if (strncasecmp(itr.second->GetName(), guild_name, strlen(guild_name)) == 0) {
			ret = itr.second;
			break;
		}
	}
	return ret;
}

bool GuildList::RemoveGuild(Guild* guild, bool delete_data) {

	bool ret = false;
	if (guild && guild_list.count(guild->GetID()) > 0) {
		guild_list.erase(guild->GetID(), false, delete_data);
		ret = true;
	}
	return ret;
}

bool GuildList::RemoveGuild(int32 guild_id, bool delete_data) {

	bool ret = false;
	if (guild_list.count(guild_id) > 0) {
		guild_list.erase(guild_id, false, delete_data);
		ret = true;
	}
	return ret;
}
