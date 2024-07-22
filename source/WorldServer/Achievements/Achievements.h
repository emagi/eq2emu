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

#ifndef ACHIEVEMENTS_H_
#define ACHIEVEMENTS_H_

#include "../../common/types.h"
#include "../../common/Mutex.h"
#include "../Items/Items.h"
#include <map>
#include <vector>

using namespace std;

struct AchievementRewards
{
	int32	achievement_id;
	string	reward;
};

struct AchievementRequirements
{
	int32	achievement_id;
	string	name;
	int32	qty_req;
};

struct AchievementUpdateItems
{
	int32	achievement_id;
	int32	item_update;
};

class Achievement {
public:
	Achievement();
	Achievement(Achievement *in);
	virtual ~Achievement();

	void SetID(int32 id) {this->id = id;}
	void SetTitle(const char *title) {strncpy(this->title, title, sizeof(this->title));}
	void SetUncompletedText(const char *uncompleted_text) {strncpy(this->uncompleted_text, uncompleted_text, sizeof(this->uncompleted_text));}
	void SetCompletedText(const char *completed_text) {strncpy(this->completed_text, completed_text, sizeof(this->completed_text));}
	void SetCategory(const char *category) {strncpy(this->category, category, sizeof(this->category));}
	void SetExpansion(const char *expansion) {strncpy(this->expansion, expansion, sizeof(this->expansion));}
	void SetIcon(int16 icon) {this->icon = icon;}
	void SetPointValue(int32 point_value) {this->point_value = point_value;}
	void SetQtyReq(int32 qty_req) {this->qty_req = qty_req;}
	void SetHide(bool hide) {this->hide = hide;}
	void SetUnknown3a(int32 unknown3a) {this->unknown3a = unknown3a;}
	void SetUnknown3b(int32 unknown3b) {this->unknown3b = unknown3b;}

	void AddAchievementRequirement(struct AchievementRequirements *requirements);
	void AddAchievementReward(struct AchievementRewards *reward);

	int32 GetID() {return id;}
	const char * GetTitle() {return title;}
	const char * GetUncompletedText() {return uncompleted_text;}
	const char * GetCompletedText() {return completed_text;}
	const char * GetCategory() {return category;}
	const char * GetExpansion() {return expansion;}
	int16 GetIcon() {return icon;}
	int32 GetPointValue() {return point_value;}
	int32 GetQtyReq() {return qty_req;}
	bool GetHide() {return hide;}
	int32 GetUnknown3a() {return unknown3a;}
	int32 GetUnknown3b() {return unknown3b;}
	vector<struct AchievementRequirements *> * GetRequirements() {return &requirements;}
	vector<struct AchievementRewards *> * GetRewards() {return &rewards;}

private:
	int32	id;
	char	title[512];
	char	uncompleted_text[512];
	char	completed_text[512];
	char	category[32];
	char	expansion[32];
	int16	icon;
	int32	point_value;
	int32	qty_req;
	bool	hide;
	int32	unknown3a;
	int32	unknown3b;
	vector<struct AchievementRequirements *> requirements;
	vector<struct AchievementRewards *> rewards;
};

class AchievementUpdate {
public:
	AchievementUpdate();
	AchievementUpdate(AchievementUpdate *in);
	virtual ~AchievementUpdate();

	void SetID(int32 id) {this->id = id;}
	void SetCompletedDate(int32 completed_date) {this->completed_date = completed_date;}

	void AddAchievementUpdateItems(struct AchievementUpdateItems *update_items);

	int32 GetID() {return id;}
	int32 GetCompletedDate() {return completed_date;}

	vector<struct AchievementUpdateItems *> * GetUpdateItems() {return &update_items;}

private:
	int32	id;
	int32	completed_date;
	vector<struct AchievementUpdateItems *> update_items;
};

class MasterAchievementList {
public:
	MasterAchievementList();
	virtual ~MasterAchievementList();

	bool AddAchievement(Achievement *achievement);
	Achievement * GetAchievement(int32 achievement_id);
	void ClearAchievements();
	int32 Size();
	void CreateMasterAchievementListPacket();
	EQ2Packet * GetAchievementPacket() { return m_packetsCreated ? masterPacket : 0;}
	EQ2Packet *masterPacket;
private:
	Mutex mutex_achievements;
	map<int32, Achievement *> achievements;
	
	bool m_packetsCreated;
};

class PlayerAchievementList {
public:
	PlayerAchievementList();
	virtual ~PlayerAchievementList();

	bool AddAchievement(Achievement *achievement);
	Achievement * GetAchievement(int32 achievement_id);
	void ClearAchievements();
	int32 Size();

	map<int32, Achievement *> * GetAchievements() {return &achievements;}

private:
	map<int32, Achievement *> achievements;
};

class PlayerAchievementUpdateList {
public:
	PlayerAchievementUpdateList();
	virtual ~PlayerAchievementUpdateList();

	bool AddAchievementUpdate(AchievementUpdate *achievement_update);
	void ClearAchievementUpdates();
	int32 Size();

	map<int32, AchievementUpdate *> * GetAchievementUpdates() {return &achievement_updates;}

private:
	map<int32, AchievementUpdate *> achievement_updates;
};
#endif