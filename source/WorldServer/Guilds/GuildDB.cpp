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
#ifdef WIN32
	#include <WinSock2.h>
	#include <windows.h>
#endif
#include <math.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ios>
#include <mysql.h>
#include <assert.h>
#include "../../common/Log.h"
#include "../WorldDatabase.h"
#include "Guild.h"

extern GuildList guild_list;
extern RuleManager rule_manager;

void WorldDatabase::LoadGuilds() {
	int32 num_guilds = 0;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `id`, `name`, `motd`, `level`, `xp`, `xp_needed`, `formed_on` FROM `guilds`");
	while (result && (row = mysql_fetch_row(result))) {
		LogWrite(GUILD__DEBUG, 1, "Guilds", "%u. %s", atoul(row[0]), row[1]);
		Guild* guild = new Guild;
		guild->SetID(atoul(row[0]));
		guild->SetName(row[1]);
		if (row[2])
			guild->SetMOTD(row[2], false);
		guild->SetLevel(atoi(row[3]), false);
		guild->SetEXPCurrent(atoul(row[4]), false);
		guild->SetEXPToNextLevel(atoul(row[5]), false);
		guild->SetFormedDate(atoul(row[6]));

		LogWrite(GUILD__DEBUG, 3, "Guilds", "\tLoaded %i guild members.", LoadGuildMembers(guild));
		LogWrite(GUILD__DEBUG, 3, "Guilds", "\tLoading Guild Ranks...");
		LoadGuildRanks(guild);
		LogWrite(GUILD__DEBUG, 3, "Guilds", "\tLoading Guild Event Filters...");
		LoadGuildEventFilters(guild);
		LogWrite(GUILD__DEBUG, 3, "Guilds", "\tLoading Guild Events...");
		LoadGuildEvents(guild);
		LogWrite(GUILD__DEBUG, 3, "Guilds", "\tLoading Guild Recruiting...");
		LoadGuildRecruiting(guild);
		guild_list.AddGuild(guild);
		num_guilds++;
	}
	LogWrite(GUILD__INFO, 0, "Guilds", "\tLoaded %u Guild(s)", num_guilds);
}

int32 WorldDatabase::LoadGuildMembers(Guild* guild) {
	int32 num_members = 0;
	Query query;
	MYSQL_ROW row;
	char *name;
	int32 char_id;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `char_id`, `recruiter_id`, `guild_status`, `points`, `rank_id`, `member_flags`, `join_date`, `note`, `officer_note`, `recruiting_message`, `recruiter_picture_data` FROM `guild_members` WHERE `guild_id`=%u", guild->GetID());

	while (result && (row = mysql_fetch_row(result))) {
		char_id = atoul(row[0]);
		if (!(name = GetCharacterName(char_id))) {
			LogWrite(GUILD__ERROR, 0, "Guilds", "WorldDatabase::LoadGuildMembers Cannot find guild member with character id %u.", char_id);
			continue;
		}

		GuildMember* gm = new GuildMember;
		gm->character_id = char_id;
		gm->recruiter_id = atoul(row[1]);
		gm->guild_status = atoul(row[2]);
		gm->points = atof(row[3]);
		gm->rank = atoi(row[4]);
		gm->member_flags = atoi(row[5]);
		gm->join_date = atoul(row[6]);
		if (row[7])
			gm->note = string(row[7]);
		if (row[8])
			gm->officer_note = string(row[8]);
		if (row[9])
			gm->recruiter_description = string(row[9]);
		int16 recruiter_picture_data_size = 0;
		if (row[10] && (recruiter_picture_data_size = strlen(row[10])) > 0) {
			gm->recruiter_picture_data_size = recruiter_picture_data_size / 2;
			gm->recruiter_picture_data = new unsigned char[gm->recruiter_picture_data_size];
			unsigned char* cpy = gm->recruiter_picture_data;
			const char* str = row[10];
			char high, low;
			for (const char* ptr = str; *ptr; ptr += 2) {
				high = tolower(*ptr);
				low = tolower(*(ptr+1));
				if (isdigit(high))
					high = high - '0';
				else if (high >= 'a' && high <= 'f')
					high = (high - 'a') + 10;
				else {
					LogWrite(GUILD__ERROR, 0, "Guilds", "Guild mate with id %u has corrupt picture data. Data not loading.", gm->character_id);
					safe_delete_array(gm->recruiter_picture_data);
					gm->recruiter_picture_data_size = 0;
					break;
				}
				if (isdigit(low))
					low = low - '0';
				else if (low >= 'a' && low <= 'f')
					low = (low - 'a') + 10;
				else {
					LogWrite(GUILD__ERROR, 0, "Guilds", "Guild mate with id %u has corrupt picture data. Data not loading.", gm->character_id);
					safe_delete_array(gm->recruiter_picture_data);
					gm->recruiter_picture_data_size = 0;
					break;
				}
				*cpy++ = low | (high << 4);
			}
			/*for (int16 i = 0; i < gm->recruiter_picture_data_size; i++)
				if (i<10)
					printf("int:%u hex:%x\n", gm->recruiter_picture_data[i], gm->recruiter_picture_data[i]);*/
		}
		else {
			gm->recruiter_picture_data_size = 0;
			gm->recruiter_picture_data = 0;
		}
		strncpy(gm->name, name, sizeof(gm->name));
		gm->account_id = GetCharacterAccountID(char_id);
		gm->adventure_class = GetCharacterClass(char_id);
		gm->adventure_level = GetCharacterLevel(char_id);
		gm->tradeskill_class = 0;
		gm->tradeskill_level = 0;
		gm->last_login_date = GetCharacterTimeStamp(char_id);
		gm->zone = GetZoneDescription(GetCharacterCurrentZoneID(char_id));
		gm->recruiting_show_adventure_class = 1;
		LoadGuildPointsHistory(guild, gm);
		guild->AddGuildMember(gm);
		safe_delete_array(name);
		num_members++;
	}
	return num_members;
}

void WorldDatabase::LoadGuildEvents(Guild* guild) {
	if (guild) {
		Query query;
		MYSQL_ROW row;
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `event_id`, `event_date`, `event_type`, `description`, `locked` FROM `guild_events` WHERE `guild_id`=%u AND `display`=1 AND `archived`=0 ORDER BY `event_date` DESC LIMIT 0, %u", guild->GetID(), GUILD_MAX_EVENTS);
		while (result && (row = mysql_fetch_row(result)))
			guild->AddGuildEvent(atoi64(row[0]), atoul(row[2]), row[3], atoul(row[1]), atoi(row[4]));
	}
}

void WorldDatabase::LoadGuildRanks(Guild* guild) {

	if (guild) {
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Loading Ranks for guild id: %u", guild->GetID());
		Query query;
		MYSQL_ROW row;
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `rank_id`, `rank_name`, `permission1`, `permission2` FROM `guild_ranks` WHERE `guild_id`=%u", guild->GetID());
		while (result && (row = mysql_fetch_row(result))) {
			int8 rank_id = atoi(row[0]);
			int32 permission1 = atoul(row[2]);
			int32 permission2 = atoul(row[3]);
			guild->SetRankName(rank_id, row[1], false);
			LogWrite(GUILD__DEBUG, 5, "Guilds", "\tLoading rank_id: %i", rank_id);
			LogWrite(GUILD__DEBUG, 5, "Guilds", "\tPermission1: %ul, Permission2: %ul", permission1, permission2);
			for (int32 i = 0; i <= 44; i++) {
				int32 bitwise_val;
				if (i < 32) {
					bitwise_val = (int32)pow(2.0, (double)(i));
					guild->SetPermission(rank_id, i, permission1 & bitwise_val ? 1 : 0, false);
					LogWrite(GUILD__DEBUG, 5, "Guilds", "\tSetting Permission %u to %u", i, permission1 & bitwise_val ? 1 : 0);
				}
				else {
					bitwise_val = (int32)pow(2.0, (double)(i - 32));
					guild->SetPermission(rank_id, i, permission2 & bitwise_val ? 1 : 0, false);
					LogWrite(GUILD__DEBUG, 5, "Guilds", "\tSetting Permission %u to %u", i, permission2 & bitwise_val ? 1 : 0);
				}
			}
		}
	}
}

void WorldDatabase::LoadGuildEventFilters(Guild* guild) {
	if (guild) {
		Query query;
		MYSQL_ROW row;
		bool event_filter_added = false;
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `event_id`, `retain`, `broadcast` FROM `guild_event_filters` WHERE `guild_id`=%u", guild->GetID());
		while (result && (row = mysql_fetch_row(result))) {
			guild->SetEventFilter(atoi(row[0]), GUILD_EVENT_FILTER_CATEGORY_RETAIN_HISTORY, atoi(row[1]), false);
			guild->SetEventFilter(atoi(row[0]), GUILD_EVENT_FILTER_CATEGORY_BROADCAST, atoi(row[2]), false);
			if (!event_filter_added)
				event_filter_added = true;
		}

		if (!event_filter_added)
			LoadGuildDefaultEventFilters(guild);
	}
}

void WorldDatabase::LoadGuildPointsHistory(Guild* guild, GuildMember* guild_member) {
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result;

	assert(guild);
	assert(guild_member);
		
	result = query.RunQuery2(Q_SELECT, "SELECT `points_date`, `modified_by`, `comment`, `points` FROM `guild_points_history` WHERE `guild_id`=%u AND `char_id`=%u ORDER BY `points_date` DESC", guild->GetID(), guild_member->character_id);
	while (result && (row = mysql_fetch_row(result)))
		guild->AddPointHistory(guild_member, atoul(row[0]), row[1], atof(row[3]), row[2], false);
}

void WorldDatabase::LoadGuildRecruiting(Guild* guild) {
	if (guild) {
		Query query;
		MYSQL_ROW row;
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `short_desc`, `full_desc`, `min_level`, `play_style`, `looking_for`, `descriptive_tag1`, `descriptive_tag2`, `descriptive_tag3`, `descriptive_tag4` FROM `guild_recruiting` WHERE `guild_id`=%u", guild->GetID());
		while (result && (row = mysql_fetch_row(result))) {
			if (row[0])
				guild->SetRecruitingShortDesc(row[0], false);
			if (row[1])
				guild->SetRecruitingFullDesc(row[1], false);
			guild->SetRecruitingMinLevel(atoi(row[2]), false);
			guild->SetRecruitingPlayStyle(atoi(row[3]), false);
			for (int32 i = 0; i <= 5; i++) {
				int32 bitwise_val = (int32)pow(2.0, (double)i);
				guild->SetRecruitingFlag(i, atoi(row[4]) & bitwise_val ? 1 : 0, false);
			}
			guild->SetRecruitingDescTag(0, atoi(row[5]), false);
			guild->SetRecruitingDescTag(1, atoi(row[6]), false);
			guild->SetRecruitingDescTag(2, atoi(row[7]), false);
			guild->SetRecruitingDescTag(3, atoi(row[8]), false);
		}
	}
}

void WorldDatabase::SaveGuild(Guild* guild, bool new_guild) {
	Query query;

	assert(guild);
	
	if (new_guild) {
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Saving NEW Guild '%s' (%u) data...", guild->GetName(), guild->GetID());
		query.RunQuery2(Q_INSERT,	"INSERT INTO `guilds` (`name`, `motd`, `level`, `xp`, `xp_needed`, `formed_on`) "
									"VALUES ('%s', '%s', %i, %llu, %llu, %u)", 
									getSafeEscapeString(guild->GetName()).c_str(), getSafeEscapeString(guild->GetMOTD()).c_str(), guild->GetLevel(), guild->GetEXPCurrent(), guild->GetEXPToNextLevel(), guild->GetFormedDate());
		guild->SetID(query.GetLastInsertedID());
	}
	else {
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Saving Guild '%s' (%u) data...", guild->GetName(), guild->GetID());
		query.RunQuery2(Q_UPDATE,	"UPDATE `guilds` "
									"SET `name`='%s', `motd`='%s', `level`=%i, `xp`=%llu, `xp_needed`=%llu, `formed_on`=%u WHERE `id`=%u",
									getSafeEscapeString(guild->GetName()).c_str(), getSafeEscapeString(guild->GetMOTD()).c_str(), guild->GetLevel(), guild->GetEXPCurrent(), guild->GetEXPToNextLevel(), guild->GetFormedDate(), guild->GetID());
	}
	guild->SetSaveNeeded(false);
}

void WorldDatabase::SaveGuildMembers(Guild* guild) {
	map<int32, GuildMember *>* members;
	map<int32, GuildMember *>::iterator itr;
	Mutex *mMembers;
	GuildMember *gm;
	Query query, query2;

	assert(guild);

	members = guild->GetGuildMembers();
	mMembers = guild->GetGuildMembersMutex();

	mMembers->readlock(__FUNCTION__, __LINE__);
	for (itr = members->begin(); itr != members->end(); itr++) {
		gm = itr->second;
		LogWrite(GUILD__DEBUG, 5, "Guilds", "Saving Guild Member '%s' (%u) data...", gm->name, gm->character_id);
		query.RunQuery2(Q_INSERT, "INSERT INTO `guild_members` (`guild_id`, `char_id`, `recruiter_id`, `guild_status`, `points`, `rank_id`, `member_flags`, `join_date`, `note`, `officer_note`, `recruiting_message`, `recruiter_picture_data`) VALUES (%u, %u, %u, %u, %f, %u, %u, %u, '%s', '%s', '%s', NULL) ON DUPLICATE KEY UPDATE `guild_id`=%u, `recruiter_id`=%u, `guild_status`=%u, `points`=%f, `rank_id`=%u, `member_flags`=%u, `join_date`=%u, `note`='%s', `officer_note`='%s', `recruiting_message`='%s', `recruiter_picture_data`=NULL", guild->GetID(), gm->character_id, gm->recruiter_id, gm->guild_status, gm->points, gm->rank, gm->member_flags, gm->join_date, getSafeEscapeString(gm->note.c_str()).c_str(), getSafeEscapeString(gm->officer_note.c_str()).c_str(), getSafeEscapeString(gm->recruiter_description.c_str()).c_str(), guild->GetID(), gm->recruiter_id, gm->guild_status, gm->points, gm->rank, gm->member_flags, gm->join_date, getSafeEscapeString(gm->note.c_str()).c_str(), getSafeEscapeString(gm->officer_note.c_str()).c_str(), getSafeEscapeString(gm->recruiter_description.c_str()).c_str());
		if (gm && gm->recruiter_picture_data_size > 0 && gm->recruiter_picture_data) {
			stringstream ss_hex;
			stringstream ss_query;
			ss_hex.flags(ios::hex);
			for (int16 i = 0; i < gm->recruiter_picture_data_size; i++)
				ss_hex << setfill('0') << setw(2) << (int)gm->recruiter_picture_data[i];
			ss_query << "UPDATE `guild_members` SET `recruiter_picture_data`='" << ss_hex.str() << "' WHERE `char_id`=" << gm->character_id;
			query2.RunQuery2(ss_query.str(), Q_UPDATE);
		}
	}
	guild->SetMemberSaveNeeded(false);
	mMembers->releasereadlock(__FUNCTION__, __LINE__);
}

void WorldDatabase::SaveGuildEvents(Guild* guild) {
	if (guild) {
		deque<GuildEvent*>* guild_events = guild->GetGuildEvents();
		deque<GuildEvent*>::iterator itr;
		for (itr = guild_events->begin(); itr != guild_events->end(); itr++) {
			GuildEvent* ge = *itr;
			if (!ge->save_needed)
				continue;

			LogWrite(GUILD__DEBUG, 5, "Guilds", "Saving Guild Events for guild '%s' (%u)...", guild->GetName(), guild->GetID());

			Query query;
			query.RunQuery2(Q_INSERT,	"INSERT INTO `guild_events` (`guild_id`, `event_id`, `event_date`, `event_type`, `description`, `display`, `locked`, `archived`) "
										"VALUES (%u, %llu, %u, %u, '%s', 1, %u, 0) "
										"ON DUPLICATE KEY UPDATE `locked`=%i", 
										guild->GetID(), ge->event_id, ge->date, ge->type, getSafeEscapeString(ge->description.c_str()).c_str(), ge->locked, ge->locked);
			ge->save_needed = false;
		}
		guild->SetEventsSaveNeeded(false);
	}
}

void WorldDatabase::SaveGuildRanks(Guild* guild) {
	if (guild) {
		MutexMap<int8, MutexMap<int8, int8>*>* permissions = guild->GetPermissions();
		MutexMap<int8, string>* ranks = guild->GetGuildRanks();
		MutexMap<int8, string>::iterator ranks_itr = ranks->begin();
		while (ranks_itr.Next()) {
			int32 permission1 = 0;
			int32 permission2 = 0;
			for (int32 i = 0; i <= 44; i++) {
				if (permissions->count(ranks_itr.first) > 0 && permissions->Get(ranks_itr.first)->count(i) > 0 && permissions->Get(ranks_itr.first)->Get(i)) {
					if (i < 32)
						permission1 += (int32)pow(2.0, (double)i);
					else
						permission2 += (int32)pow(2.0, (double)(i - 32));
				}
			}
			LogWrite(GUILD__DEBUG, 5, "Guilds", "Saving Guild Ranks for guild '%s' (%u)...", guild->GetName(), guild->GetID());
			Query query;
			query.RunQuery2(Q_INSERT,	"INSERT INTO `guild_ranks` (`guild_id`, `rank_id`, `rank_name`, `permission1`, `permission2`) "
										"VALUES (%u, %u, '%s', %u, %u) "
										"ON DUPLICATE KEY UPDATE `rank_name`='%s', `permission1`=%u, permission2=%u", 
										guild->GetID(), ranks_itr.first, getSafeEscapeString(ranks_itr.second.c_str()).c_str(), permission1, permission2, getSafeEscapeString(ranks_itr.second.c_str()).c_str(), permission1, permission2);
		}
		guild->SetRanksSaveNeeded(false);
	}
}

void WorldDatabase::SaveGuildEventFilters(Guild* guild) {
	int32 i;
	assert(guild);
	
	for (i = 0; i < 93; i++) {
		LogWrite(GUILD__DEBUG, 5, "Guilds", "Saving Guild EventFilters for guild '%s' (%u)...", guild->GetName(), guild->GetID());
		Query query;
		query.RunQuery2(Q_INSERT,	"INSERT INTO `guild_event_filters` (`guild_id`, `event_id`, `retain`, `broadcast`) "
									"VALUES (%u, %u, %u, %u) "
									"ON DUPLICATE KEY UPDATE `retain`=%u, `broadcast`=%u",
									guild->GetID(), i, guild->GetEventFilter(i, GUILD_EVENT_FILTER_CATEGORY_RETAIN_HISTORY), guild->GetEventFilter(i, GUILD_EVENT_FILTER_CATEGORY_BROADCAST), guild->GetEventFilter(i, GUILD_EVENT_FILTER_CATEGORY_RETAIN_HISTORY), guild->GetEventFilter(i, GUILD_EVENT_FILTER_CATEGORY_BROADCAST));
	}

	guild->SetEventFiltersSaveNeeded(false);
}

void WorldDatabase::SaveGuildPointsHistory(Guild* guild) {
	map<int32, GuildMember *> *members;
	map<int32, GuildMember *>::iterator itr;
	Mutex *mMembers;
	deque<PointHistory *> *ph_list;
	deque<PointHistory*>::iterator ph_itr;
	PointHistory*  ph;

	assert (guild);

	members = guild->GetGuildMembers();
	mMembers = guild->GetGuildMembersMutex();

	mMembers->readlock(__FUNCTION__, __LINE__);
	for (itr = members->begin(); itr != members->end(); itr++) {
		ph_list = &itr->second->point_history;
			
		for (ph_itr = ph_list->begin(); ph_itr != ph_list->end(); ph_itr++) {
			ph = *ph_itr;
			if (!ph->saved_needed)
				continue;

			LogWrite(GUILD__DEBUG, 5, "Guilds", "Saving Guild Point History for guild '%s' (%u)...", guild->GetName(), guild->GetID());
			Query query;
			query.RunQuery2(Q_INSERT,	"INSERT INTO `guild_points_history` (`guild_id`, `char_id`, `points_date`, `modified_by`, `comment`, `points`) "
										"VALUES (%u, %u, %u, '%s', '%s', %f)", 
										guild->GetID(), itr->first, ph->date, getSafeEscapeString(ph->modified_by.c_str()).c_str(), getSafeEscapeString(ph->comment.c_str()).c_str(), ph->points);
			ph->saved_needed = false;
		}
	}
	guild->SetPointsHistorySaveNeeded(false);
	mMembers->releasereadlock(__FUNCTION__, __LINE__);
}

void WorldDatabase::SaveGuildRecruiting(Guild* guild) {
	if (guild) {
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Saving Recruiting info for guild '%s' (%u)...", guild->GetName(), guild->GetID());
		Query query;
		query.RunQuery2(Q_INSERT, "INSERT INTO `guild_recruiting` (`guild_id`, `short_desc`, `full_desc`, `min_level`, `play_style`, `looking_for`, `descriptive_tag1`, `descriptive_tag2`, `descriptive_tag3`, `descriptive_tag4`) VALUES (%u, '%s', '%s', %u, %u, %u, %u, %u, %u, %u) ON DUPLICATE KEY UPDATE `short_desc`='%s', `full_desc`='%s', `min_level`=%u, `play_style`=%u, `looking_for`=%u, `descriptive_tag1`=%u, `descriptive_tag2`=%u, `descriptive_tag3`=%u, `descriptive_tag4`=%u", guild->GetID(), getSafeEscapeString(guild->GetRecruitingShortDesc().c_str()).c_str(), getSafeEscapeString(guild->GetRecruitingFullDesc().c_str()).c_str(), guild->GetRecruitingMinLevel(), guild->GetRecruitingPlayStyle(), guild->GetRecruitingLookingForPacketValue(), guild->GetRecruitingDescTag(0), guild->GetRecruitingDescTag(1), guild->GetRecruitingDescTag(2), guild->GetRecruitingDescTag(3), getSafeEscapeString(guild->GetRecruitingShortDesc().c_str()).c_str(), getSafeEscapeString(guild->GetRecruitingFullDesc().c_str()).c_str(), guild->GetRecruitingMinLevel(), guild->GetRecruitingPlayStyle(), guild->GetRecruitingLookingForPacketValue(), guild->GetRecruitingDescTag(0), guild->GetRecruitingDescTag(1), guild->GetRecruitingDescTag(2), guild->GetRecruitingDescTag(3));
		guild->SetRecruitingSaveNeeded(false);
	}
}

void WorldDatabase::DeleteGuild(Guild* guild) {
	if (guild) {
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Deleting Guild '%s' (%u)...", guild->GetName(), guild->GetID());
		Query query;
		query.RunQuery2(Q_DELETE, "DELETE FROM `guilds` WHERE `id`=%u", guild->GetID());
	}
}

void WorldDatabase::DeleteGuildMember(Guild* guild, int32 character_id) {
	if (guild) {
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Deleting Character (%u) from guild '%s' (%u)...", character_id, guild->GetName(), guild->GetID());
		Query query;
		query.RunQuery2(Q_DELETE, "DELETE FROM `guild_members` WHERE `guild_id`=%u AND `char_id`=%u", guild->GetID(), character_id);
	}
}

void WorldDatabase::DeleteGuildEvent(Guild* guild, int64 event_id) {
	if (guild) {
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Deleting Event (%u) from guild '%s' (%u)...", event_id, guild->GetName(), guild->GetID());
		Query query;
		query.RunQuery2(Q_DELETE, "DELETE FROM `guild_events` WHERE `guild_id`=%u AND `event_id`=%u", guild->GetID(), event_id);
	}
}

void WorldDatabase::DeleteGuildPointHistory(Guild* guild, int32 character_id, PointHistory* point_history) {
	if (guild && point_history) {
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Deleting PointHistory for Character (%u) from guild '%s' (%u)...", character_id, guild->GetName(), guild->GetID());
		Query query;
		query.RunQuery2(Q_DELETE, "DELETE FROM `guild_points_history` WHERE `guild_id`=%u AND `char_id`=%u AND `points_date`=%u", guild->GetID(), character_id, point_history->date);
	}
}

void WorldDatabase::ArchiveGuildEvent(Guild* guild, GuildEvent* guild_event) {
	if (guild && guild_event) {
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Archiving Event (%u) for guild '%s' (%u)...", guild_event->event_id, guild->GetName(), guild->GetID());
		Query query;
		query.RunQuery2(Q_UPDATE, "UPDATE `guild_events` SET `archived`=1 WHERE `guild_id`=%u AND `event_id`=%u", guild->GetID(), guild_event->event_id);
	}
}

void WorldDatabase::SaveHiddenGuildEvent(Guild* guild, GuildEvent* guild_event) {
	if (guild && guild_event) {
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Saving Hidden Event (%u) for guild '%s' (%u)...", guild_event->event_id, guild->GetName(), guild->GetID());
		Query query;
		query.RunQuery2(Q_INSERT, "INSERT INTO `guild_events` (`guild_id`, `event_id`, `event_date`, `event_type`, `description`, `display`, `locked`, `archived`) VALUES (%u, %u, %u, %u, '%s', 0, %u, 0)", guild->GetID(), guild_event->event_id, guild_event->type, guild_event->date, getSafeEscapeString(guild_event->description.c_str()).c_str(), guild_event->locked);
	}
}

int32 WorldDatabase::GetGuildIDByCharacterID(int32 char_id) {
	if(char_id > 0)
	{
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Look up guild ID for player ID: '%u'...", char_id);
		Query query;
		MYSQL_ROW row;
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT name FROM guilds, guild_members WHERE guilds.id = guild_members.guild_id AND char_id = %u ", char_id);
		while (result && (row = mysql_fetch_row(result))) {
			if( row[0] )
				return atoul(row[0]);
		}
	}
	return 0;
}

void WorldDatabase::LoadGuildDefaultRanks(Guild* guild) {
	if (guild) {
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Load/Set Default Ranks for guild '%s' (%u)...", guild->GetName(), guild->GetID());
		Query query;
		MYSQL_ROW row;
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT DISTINCT `rank_id`, `rank_name`, `permission1`, `permission2` FROM `guild_ranks_defaults`");
		while (result && (row = mysql_fetch_row(result))) {
			int8 rank_id = atoi(row[0]);
			int32 permission1 = atoul(row[2]);
			int32 permission2 = atoul(row[3]);

			LogWrite(GUILD__DEBUG, 3, "Guilds", "\tSetting RankID %i, permission1: %u, permission2: %u", rank_id, permission1, permission2);

			guild->SetRankName(rank_id, row[1], false);
			for (int32 i = 0; i <= 44; i++) {
				int32 bitwise_val;
				if (i < 32) {
					bitwise_val = (int32)pow(2.0, (double)i);
					guild->SetPermission(rank_id, i, permission1 & bitwise_val ? 1 : 0, false);
				}
				else {
					bitwise_val = (int32)pow(2.0, (double)(i - 32));
					guild->SetPermission(rank_id, i, permission2 & bitwise_val ? 1 : 0, false);
				}
			}
		}
	}
}

void WorldDatabase::LoadGuildDefaultEventFilters(Guild* guild) {
	if (guild) {
		LogWrite(GUILD__DEBUG, 3, "Guilds", "Load/Set Default Event Filters for guild '%s' (%u)...", guild->GetName(), guild->GetID());
		Query query;
		MYSQL_ROW row;
		MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT DISTINCT `event_id`, `retain`, `broadcast` FROM `guild_event_defaults`");
		while (result && (row = mysql_fetch_row(result))) {

			LogWrite(GUILD__DEBUG, 3, "Guilds", "\tSetting Event Filter %i, retain: %i, broadcast: %i", atoi(row[0]), atoi(row[1]), atoi(row[2]));

			guild->SetEventFilter(atoi(row[0]), GUILD_EVENT_FILTER_CATEGORY_RETAIN_HISTORY, atoi(row[1]), false);
			guild->SetEventFilter(atoi(row[0]), GUILD_EVENT_FILTER_CATEGORY_BROADCAST, atoi(row[2]), false);
		}
	}
}

bool WorldDatabase::AddNewPlayerToServerGuild(int32 account_id, int32 char_id)
{
	// Check if this servers rule allow auto-joining Server guild
	int8 autojoin = rule_manager.GetGlobalRule(R_World, GuildAutoJoin)->GetInt8();
	if( autojoin )
	{
		// if so, what is the guild ID of the default server guild?
		int32 guild_id = rule_manager.GetGlobalRule(R_World, GuildAutoJoinID)->GetInt32();
		Guild* guild = 0;
		guild = guild_list.GetGuild(guild_id);
		if (!guild) 
		{
			// guild was not valid, abort!
			LogWrite(GUILD__ERROR, 1, "Guilds", "Guild ID %u not found! Cannot autojoin members!", guild_id);
			return false;
		}
		else
		{
			// guild was found, so what default Rank to make the players? if not set, use 7 (recruit)
			int8 rank_id = rule_manager.GetGlobalRule(R_World, GuildAutoJoinDefaultRankID)->GetInt8();
			if(!rank_id)
				rank_id = 7;

			// assuming all is good, insert the new guild member here...
			GuildMember *gm = new GuildMember();

			gm->account_id = account_id;
			gm->character_id = char_id;
			char* name = GetCharacterName(gm->character_id);
			strncpy(gm->name, name, sizeof(gm->name));
			gm->guild_status = 0;
			gm->points = 0.0;
			//gm->adventure_class = player->GetAdventureClass();
			//gm->adventure_level = player->GetLevel();
			//gm->tradeskill_class = player->GetTradeskillClass();
			//gm->tradeskill_level = player->GetTSLevel();
			gm->rank = rank_id;
			gm->zone = string("");
			gm->join_date = Timer::GetUnixTimeStamp();
			gm->last_login_date = gm->join_date;
			gm->recruiter_id = 0;
			gm->member_flags = GUILD_MEMBER_FLAGS_NOTIFY_LOGINS;
			gm->recruiting_show_adventure_class = 1;
			gm->recruiter_picture_data_size = 0;
			gm->recruiter_picture_data = 0;

			guild->AddGuildMember(gm);

			Query query;
			query.RunQuery2(Q_INSERT, "INSERT INTO `guild_members` (`guild_id`, `char_id`, `join_date`, `rank_id`) VALUES (%u, %u, %u, %i)", 
				guild_id, char_id, gm->join_date, rank_id);

			LogWrite(GUILD__DEBUG, 3, "Guilds", "Auto-join player (%u) to server guild '%s' (%u) at rank %i...", char_id, guild->GetName(), guild_id, rank_id);

			// success!
			return true;
		}
	}
	// do not auto-join server guild
	return false;
}


