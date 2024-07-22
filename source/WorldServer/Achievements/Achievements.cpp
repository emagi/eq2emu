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

#include "Achievements.h"

#include "../../common/Log.h"
#include "../../common/ConfigReader.h"
#include <assert.h>

extern ConfigReader configReader;
extern MasterAchievementList master_achievement_list;

Achievement::Achievement() {
	id = 0;
	memset(title, 0, sizeof(title));
	memset(uncompleted_text, 0, sizeof(uncompleted_text));
	memset(completed_text, 0, sizeof(completed_text));
	memset(category, 0, sizeof(category));
	memset(expansion, 0, sizeof(expansion));
	icon = 0;
	point_value = 0;
	qty_req = 0;
	hide = false;
	unknown3a = 0;
	unknown3b = 0;
}

Achievement::Achievement(Achievement *in) {
	vector<struct AchievementRequirements *> *requirements_in;
	vector<struct AchievementRewards *> *rewards_in;
	vector<struct AchievementRequirements *>::iterator itr;
	vector<struct AchievementRewards *>::iterator itr2;
	struct AchievementRequirements *achievement_requirement;
	struct AchievementRewards *achievement_reward;

	assert(in);

	id = in->GetID();
	strncpy(title, in->GetTitle(), sizeof(title));
	strncpy(uncompleted_text, in->GetUncompletedText(), sizeof(uncompleted_text));
	strncpy(completed_text, in->GetCompletedText(), sizeof(completed_text));
	strncpy(category, in->GetCategory(), sizeof(category));
	strncpy(expansion, in->GetExpansion(), sizeof(expansion));
	icon = in->GetIcon();
	point_value = in->GetPointValue();
	qty_req = in->GetQtyReq();
	hide = in->GetHide();
	unknown3a = in->GetUnknown3a();
	unknown3b = in->GetUnknown3b();

	requirements_in = in->GetRequirements();
	for (itr = requirements_in->begin(); itr != requirements_in->end(); itr++) {
		achievement_requirement = new struct AchievementRequirements;
		achievement_requirement->achievement_id = (*itr)->achievement_id;
		achievement_requirement->name = (*itr)->name;
		achievement_requirement->qty_req = (*itr)->qty_req;
		requirements.push_back(achievement_requirement);
	}

	rewards_in = in->GetRewards();
	for (itr2 = rewards_in->begin(); itr2 != rewards_in->end(); itr2++) {
		achievement_reward = new struct AchievementRewards;
		achievement_reward->achievement_id = (*itr2)->achievement_id;
		achievement_reward->reward = (*itr2)->reward;
		rewards.push_back(achievement_reward);
	}
}

Achievement::~Achievement() {
	vector<struct AchievementRequirements *>::iterator itr;
	vector<struct AchievementRewards *>::iterator itr2;

	for (itr = requirements.begin(); itr != requirements.end(); itr++)
		safe_delete(*itr);
	for (itr2 = rewards.begin(); itr2 != rewards.end(); itr2++)
		safe_delete(*itr2);
}

void Achievement::AddAchievementRequirement(struct AchievementRequirements *requirement) {
	assert(requirement);

	requirements.push_back(requirement);
}

void Achievement::AddAchievementReward(struct AchievementRewards *reward) {
	assert(reward);

	rewards.push_back(reward);
}

void AchievementUpdate::AddAchievementUpdateItems(struct AchievementUpdateItems *update_item) {
	assert(update_item);

	update_items.push_back(update_item);
}

MasterAchievementList::MasterAchievementList() {
	m_packetsCreated = false;
	masterPacket = 0;
	mutex_achievements.SetName("MasterAchievementList::achievements");
}

MasterAchievementList::~MasterAchievementList() {
	ClearAchievements();
}

bool MasterAchievementList::AddAchievement(Achievement *achievement) {
	bool ret = false;

	assert(achievement);

	mutex_achievements.writelock(__FUNCTION__, __LINE__);
	if (achievements.count(achievement->GetID()) == 0) {
		achievements[achievement->GetID()] = achievement;
		ret = true;
	}
	mutex_achievements.releasewritelock(__FUNCTION__, __LINE__);

	return ret;
}

Achievement * MasterAchievementList::GetAchievement(int32 achievement_id) {
	Achievement *achievement = 0;

	mutex_achievements.readlock(__FUNCTION__, __LINE__);
	if (achievements.count(achievement_id) > 0)
		achievement = achievements[achievement_id];
	mutex_achievements.releasereadlock(__FUNCTION__, __LINE__);

	return achievement;
}

void MasterAchievementList::ClearAchievements() {
	map<int32, Achievement *>::iterator itr;

	mutex_achievements.writelock(__FUNCTION__, __LINE__);
	for (itr = achievements.begin(); itr != achievements.end(); itr++)
		safe_delete(itr->second);
	achievements.clear();
	mutex_achievements.releasewritelock(__FUNCTION__, __LINE__);
}

int32 MasterAchievementList::Size() {
	int32 size;

	mutex_achievements.readlock(__FUNCTION__, __LINE__);
	size = achievements.size();
	mutex_achievements.releasereadlock(__FUNCTION__, __LINE__);

	return size;
}

PlayerAchievementList::PlayerAchievementList() {
}

PlayerAchievementList::~PlayerAchievementList() {
	ClearAchievements();
}

bool PlayerAchievementList::AddAchievement(Achievement *achievement) {
	assert(achievement);

	if (achievements.count(achievement->GetID()) == 0) {
		achievements[achievement->GetID()] = achievement;
		return true;
	}

	return false;
}

Achievement * PlayerAchievementList::GetAchievement(int32 achievement_id) {
	if (achievements.count(achievement_id) > 0)
		return achievements[achievement_id];

	return 0;
}

void PlayerAchievementList::ClearAchievements() {
	map<int32, Achievement *>::iterator itr;

	for (itr = achievements.begin(); itr != achievements.end(); itr++)
		safe_delete(itr->second);
	achievements.clear();
}

int32 PlayerAchievementList::Size() {
	return achievements.size();
}

AchievementUpdate::AchievementUpdate() {
	id = 0;
	completed_date = 0;

}

AchievementUpdate::AchievementUpdate(AchievementUpdate *in) {
	vector<struct AchievementUpdateItems *> *items_in;
	vector<struct AchievementUpdateItems *>::iterator itr;
	struct AchievementUpdateItems *items;

	assert(in);

	id = in->GetID();
	completed_date = in->GetCompletedDate();

	items_in = in->GetUpdateItems();
	for (itr = items_in->begin(); itr != items_in->end(); itr++) {
		items = new struct AchievementUpdateItems;
		items->achievement_id = (*itr)->achievement_id;
		items->item_update = (*itr)->item_update;
		update_items.push_back(items);
	}
}

AchievementUpdate::~AchievementUpdate() {
	vector<struct AchievementUpdateItems *>::iterator itr;

	for (itr = update_items.begin(); itr != update_items.end(); itr++)
		safe_delete(*itr);
}


PlayerAchievementUpdateList::PlayerAchievementUpdateList() {

}

PlayerAchievementUpdateList::~PlayerAchievementUpdateList() {
	ClearAchievementUpdates();
}

bool PlayerAchievementUpdateList::AddAchievementUpdate(AchievementUpdate *update) {
	assert(update);

	if (achievement_updates.count(update->GetID()) == 0) {
		achievement_updates[update->GetID()] = update;
		return true;
	}
	return false;
}

void PlayerAchievementUpdateList::ClearAchievementUpdates() {
	map<int32, AchievementUpdate *>::iterator itr;

	for (itr = achievement_updates.begin(); itr != achievement_updates.end(); itr++)
		safe_delete(itr->second);
	achievement_updates.clear();
}

int32 PlayerAchievementUpdateList::Size() {
	return achievement_updates.size();
}

void MasterAchievementList::CreateMasterAchievementListPacket() {
	map<int32, Achievement *>::iterator itr;
	Achievement *achievement;
	vector<AchievementRequirements *> *requirements = 0;
	vector<AchievementRequirements *>::iterator itr2;
	AchievementRequirements *requirement;
	vector<AchievementRewards *> *rewards = 0;
	vector<AchievementRewards *>::iterator itr3;
	AchievementRewards *reward;
	PacketStruct *packet;
	int16 i = 0;
	int16 j = 0;
	int16 k = 0;
	int16 version = 1096;

	if (!(packet = configReader.getStruct("WS_CharacterAchievements", version))) {
		return;
	}

	packet->setArrayLengthByName("num_achievements" , achievements.size());
	for (itr = achievements.begin(); itr != achievements.end(); itr++) {
		achievement = itr->second;
		packet->setArrayDataByName("achievement_id", achievement->GetID(), i);
		packet->setArrayDataByName("title", achievement->GetTitle(), i);
		packet->setArrayDataByName("uncompleted_text", achievement->GetUncompletedText(), i);
		packet->setArrayDataByName("completed_text", achievement->GetCompletedText(), i);
		packet->setArrayDataByName("category", achievement->GetCategory(), i);
		packet->setArrayDataByName("expansion", achievement->GetExpansion(), i);
		packet->setArrayDataByName("icon", achievement->GetIcon(), i);
		packet->setArrayDataByName("point_value", achievement->GetPointValue(), i);
		packet->setArrayDataByName("qty_req", achievement->GetQtyReq(), i);
		packet->setArrayDataByName("hide_achievement", achievement->GetHide(), i);
		packet->setArrayDataByName("unknown3", achievement->GetUnknown3a(), i);
		packet->setArrayDataByName("unknown3", achievement->GetUnknown3b(), i);
		requirements = achievement->GetRequirements();
		rewards = achievement->GetRewards();
		j = 0;
		k = 0;
		packet->setSubArrayLengthByName("num_items", requirements->size(), i, j);
		for (itr2 = requirements->begin(); itr2 != requirements->end(); itr2++) {
			requirement = *itr2;
			packet->setSubArrayDataByName("item_name", requirement->name.c_str(), i, j);
			packet->setSubArrayDataByName("item_qty_req", requirement->qty_req, i, j);
			j++;
		}
		packet->setSubArrayLengthByName("num_rewards", achievement->GetRewards()->size(), i, k);
		for (itr3 = rewards->begin(); itr3 != rewards->end(); itr3++) {
			reward = *itr3;
			packet->setSubArrayDataByName("reward_item", reward->reward.c_str(), i, k);
			k++;
		}
		i++;
	}

	//packet->PrintPacket();
	EQ2Packet* data = packet->serialize();
	masterPacket = new EQ2Packet(OP_ClientCmdMsg, data->pBuffer, data->size);
	safe_delete(packet);
	safe_delete(data);
	//DumpPacket(app);

	m_packetsCreated = true;
}
