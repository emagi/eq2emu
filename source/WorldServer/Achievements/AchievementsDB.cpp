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
#include <mysql.h>
#include <assert.h>
#include "../../common/Log.h"
#include "../WorldDatabase.h"
#include "Achievements.h"

extern MasterAchievementList master_achievement_list;

void WorldDatabase::LoadAchievements() 
{
	Achievement *achievement;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int32 aReqs_total = 0;
	int32 aRewards_total = 0;

	res = query.RunQuery2(Q_SELECT,	"SELECT `achievement_id`,`title`,`uncompleted_text`,`completed_text`,`category`,`expansion`,`icon`,`point_value`,`qty_req`,`hide_achievement`,`unknown3a`,`unknown3b`\n"
									"FROM `achievements`");

	if (res) 
	{
		while ((row = mysql_fetch_row(res))) 
		{
			achievement = new Achievement();
			achievement->SetID(atoul(row[0]));
			achievement->SetTitle(row[1]);
			achievement->SetUncompletedText(row[2]);
			achievement->SetCompletedText(row[3]);
			achievement->SetCategory(row[4]);
			achievement->SetExpansion(row[5]);
			achievement->SetIcon(atoi(row[6]));
			achievement->SetPointValue(atoul(row[7]));
			achievement->SetQtyReq(atoul(row[8]));
			achievement->SetHide( atoi(row[9]) == 0 ? false : true );
			achievement->SetUnknown3a(atoul(row[10]));
			achievement->SetUnknown3b(atoul(row[11]));

			LogWrite(ACHIEVEMENT__DEBUG, 5, "Achievements", "\tLoading Achievement: '%s' (%u)", achievement->GetTitle(), achievement->GetID());

			if (!master_achievement_list.AddAchievement(achievement)) 
			{
				LogWrite(ACHIEVEMENT__ERROR, 0, "Achievements", "Error adding achievement '%s' - duplicate ID: %u", achievement->GetTitle(), achievement->GetID()); 
				safe_delete(achievement);
				continue;
			}

			aReqs_total += LoadAchievementRequirements(achievement);
			aRewards_total += LoadAchievementRewards(achievement);
		}
	}
	LogWrite(ACHIEVEMENT__DEBUG, 0, "Achievements", "\tLoaded %u achievements", master_achievement_list.Size());
	LogWrite(ACHIEVEMENT__DEBUG, 0, "Achievements", "\tLoaded %u achievement requirements", aReqs_total);
	LogWrite(ACHIEVEMENT__DEBUG, 0, "Achievements", "\tLoaded %u achievement rewards", aRewards_total);

}

int32 WorldDatabase::LoadAchievementRequirements(Achievement *achievement) 
{
	AchievementRequirements *requirements;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int16 total = 0;

	assert(achievement);

	res = query.RunQuery2(Q_SELECT, "SELECT `achievement_id`, `name`, `qty_req` FROM `achievements_requirements` WHERE `achievement_id` = %u", achievement->GetID());
	if (res) {
		while ((row = mysql_fetch_row(res))) {
			requirements = new AchievementRequirements();
			requirements->achievement_id = atoul(row[0]);
			requirements->name = row[1];
			requirements->qty_req = atoul(row[2]);
			achievement->AddAchievementRequirement(requirements);
			LogWrite(ACHIEVEMENT__DEBUG, 5, "Achievements", "Loading Achievements Requirement '%s'", requirements->name.c_str());
			total++;
		}
	}
	LogWrite(ACHIEVEMENT__DEBUG, 5, "Achievements", "Loaded %u requirements for achievement '%s' ID: %u", total, achievement->GetTitle(), achievement->GetID());
	return total;
}

int32 WorldDatabase::LoadAchievementRewards(Achievement *achievement) 
{
	AchievementRewards *rewards;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int16 total = 0;

	assert(achievement);

	res = query.RunQuery2(Q_SELECT, "SELECT `achievement_id`, `reward` FROM `achievements_rewards` WHERE `achievement_id` = %u", achievement->GetID());
	if (res) {
		while ((row = mysql_fetch_row(res))) {
			rewards = new AchievementRewards();
			rewards->achievement_id = atoul(row[0]);
			rewards->reward = row[1];
			achievement->AddAchievementReward(rewards);
			LogWrite(ACHIEVEMENT__DEBUG, 5, "Achievements", "Loading Achievements Reward '%s'", rewards->reward.c_str());
			total++;
		}
	}
	LogWrite(ACHIEVEMENT__DEBUG, 5, "Achievements", "Loaded %u rewards for achievement '%s' ID: %u", total, achievement->GetTitle(), achievement->GetID());
	return total;
}

void WorldDatabase::LoadPlayerAchievements(Player *player) {
	Achievement *achievement;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int32 aReqs_total = 0;
	int32 aRewards_total = 0;
	int32 total = 0;

	assert(player);

	res = query.RunQuery2(Q_SELECT,	"SELECT `achievement_id`,`title`,`uncompleted_text`,`completed_text`,`category`,`expansion`,`icon`,`point_value`,`qty_req`,`hide_achievement`,`unknown3a`,`unknown3b`\n"
									"FROM `achievements`");

	if (res) 
	{
		while ((row = mysql_fetch_row(res))) 
		{
			achievement = new Achievement();
			achievement->SetID(atoul(row[0]));
			achievement->SetTitle(row[1]);
			achievement->SetUncompletedText(row[2]);
			achievement->SetCompletedText(row[3]);
			achievement->SetCategory(row[4]);
			achievement->SetExpansion(row[5]);
			achievement->SetIcon(atoi(row[6]));
			achievement->SetPointValue(atoul(row[7]));
			achievement->SetQtyReq(atoul(row[8]));
			achievement->SetHide( atoi(row[9]) == 0 ? false : true );
			achievement->SetUnknown3a(atoul(row[10]));
			achievement->SetUnknown3b(atoul(row[11]));

			LogWrite(ACHIEVEMENT__DEBUG, 5, "Achievements", "\tLoading Achievement: '%s' (%u)", achievement->GetTitle(), achievement->GetID());

			if (!player->GetAchievementList()->AddAchievement(achievement)) 
			{
				LogWrite(ACHIEVEMENT__ERROR, 0, "Achievements", "Error adding achievement '%s' - duplicate ID: %u", achievement->GetTitle(), achievement->GetID()); 
				safe_delete(achievement);
				continue;
			}
			total++;
			aReqs_total += LoadAchievementRequirements(achievement);
			aRewards_total += LoadAchievementRewards(achievement);
		}
	}
	LogWrite(ACHIEVEMENT__DEBUG, 0, "Achievements", "\tLoaded %u achievements for '%s'", total, player->GetName());
	LogWrite(ACHIEVEMENT__DEBUG, 0, "Achievements", "\tLoaded %u achievement requirements for '%s'", aReqs_total, player->GetName());
	LogWrite(ACHIEVEMENT__DEBUG, 0, "Achievements", "\tLoaded %u achievement rewards for '%s'", aRewards_total, player->GetName());
}

int32 WorldDatabase::LoadPlayerAchievementsUpdates(Player *player) {
	AchievementUpdate *update;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int32 total = 0;
	int32 items_total = 0;

	assert(player);

	res = query.RunQuery2(Q_SELECT, "SELECT `char_id`, `achievement_id`, `completed_date` FROM character_achievements WHERE char_id = %u ", player->GetCharacterID());
	if (res) {
		while ((row = mysql_fetch_row(res))) {
			update = new AchievementUpdate();
			update->SetID(atoul(row[1]));
			update->SetCompletedDate(atoul(row[2]));
			
			LogWrite(ACHIEVEMENT__DEBUG, 5, "Achievements", "Loading Player Achievement Update for Achievement ID: %u ", update->GetID());

			if (!player->GetAchievementUpdateList()->AddAchievementUpdate(update))
			{
				LogWrite(ACHIEVEMENT__ERROR, 0, "Achievements", "Error adding achievement update %u - diplicate ID", update->GetID());
				safe_delete(update);
				continue;
			}
			total++;
			items_total += LoadPlayerAchievementsUpdateItems(update, player->GetCharacterID());
		}
	}

	LogWrite(ACHIEVEMENT__DEBUG, 0, "Achievements", "Loaded %u player achievement updates", total);
	
	return total;
}

int32 WorldDatabase::LoadPlayerAchievementsUpdateItems(AchievementUpdate *update, int32 player_id) {
	AchievementUpdateItems *update_items;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int32 total = 0;

	assert(update);

	res = query.RunQuery2(Q_SELECT, "SELECT `achievement_id`, `items` FROM character_achievements_items WHERE char_id = %u AND achievement_id = %u;", player_id, update->GetID());
	if (res) {
		while ((row = mysql_fetch_row(res))) {
			update_items = new AchievementUpdateItems();
			update_items->achievement_id = atoul(row[0]);
			update_items->item_update = atoul(row[1]);
			update->AddAchievementUpdateItems(update_items);

			LogWrite(ACHIEVEMENT__DEBUG, 5, "Achievements", "Loading Player Achievement Update Items for Achievement ID: %u ", update_items->achievement_id);
			total++;
		}
	}
	LogWrite(ACHIEVEMENT__DEBUG, 0, "Achievements", "Loaded %u player achievement update items", total);
	return total;
}