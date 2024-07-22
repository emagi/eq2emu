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
#include "Quests.h"
#include "../common/ConfigReader.h"
#include "Player.h"
#include "LuaInterface.h"
#include "Spells.h"
#include "RaceTypes/RaceTypes.h"
#include "../common/Log.h"

#ifdef WIN32
	#include <time.h>
#else
	#include <sys/time.h>
#endif

extern LuaInterface* lua_interface;
extern ConfigReader configReader;
extern MasterFactionList master_faction_list;
extern MasterRaceTypeList race_types_list;

QuestStep::QuestStep(int32 in_id, int8 in_type, string in_description, vector<int32>* in_ids, int32 in_quantity, const char* in_task_group, vector<Location>* in_locations, float in_max_variation, float in_percentage, int32 in_usableitemid){
	type = in_type;
	description = in_description;
	ids = 0;
	locations = 0;
	if(in_task_group)
		task_group = string(in_task_group);
	if(type != QUEST_STEP_TYPE_LOCATION) {
		if (in_ids){
			ids = new std::map<int32, bool>();
			for(int32 i=0;i<in_ids->size();i++)
				ids->insert(make_pair(in_ids->at(i), true));
		}
	}
	else { // location step
		if (in_locations) {
			locations = new vector<Location>;
			for(int32 i=0; i < in_locations->size(); i++)
				locations->push_back(in_locations->at(i));
		}
	}
	max_variation = in_max_variation;
	quantity = in_quantity;
	step_progress = 0;
	icon =  11;
	id = in_id;
	updated = false;
	percentage = in_percentage;
	usableitemid = in_usableitemid;
}

QuestStep::QuestStep(QuestStep* old_step){
	type = old_step->type;
	description = old_step->description;
	task_group = old_step->task_group;
	quantity = old_step->quantity;
	max_variation = old_step->max_variation;
	step_progress = 0;
	ids = 0;
	locations = 0;
	if(type != QUEST_STEP_TYPE_LOCATION) {
		if (old_step->ids){
			ids = new std::map<int32, bool>();
			std::map<int32, bool>::iterator itr;
			for(itr = old_step->ids->begin();itr != old_step->ids->end();itr++)
				ids->insert(make_pair(itr->first, itr->second));
		}
	}
	else { // location step
		if (old_step->locations) {
			locations = new vector<Location>;
			for(int32 i=0; i < old_step->locations->size(); i++)
				locations->push_back(old_step->locations->at(i));
		}
	}
	icon = old_step->icon;
	id = old_step->id;
	updated = false;
	percentage = old_step->percentage;
	usableitemid = old_step->usableitemid;
}

QuestStep::~QuestStep(){
	safe_delete(ids);
	safe_delete(locations);
}

bool QuestStep::WasUpdated(){
	return updated;
}

void QuestStep::WasUpdated(bool val){
	updated = val;
}

float QuestStep::GetPercentage(){
	return percentage;
}

int32 QuestStep::GetStepID(){
	return id;
}
int32 QuestStep::GetItemID() {
	return usableitemid;
}
void QuestStep::SetComplete(){
	step_progress = quantity;
	updated = true;
}

bool QuestStep::Complete(){
	return step_progress >= quantity;
}

int8 QuestStep::GetStepType(){
	return type;
}

bool QuestStep::CheckStepReferencedID(int32 id){
	bool ret = false;
	if(ids){
		std::map<int32, bool>::iterator itr;
		itr = ids->find(id);
		if(itr != ids->end())
			ret = true;
	}
	return ret;
}

bool QuestStep::CheckStepKillRaceReqUpdate(Spawn* spawn){
	bool ret = false;
	if(ids){
		std::map<int32, bool>::iterator itr;
		for(itr = ids->begin();itr != ids->end();itr++){
			int32 curid = itr->first;
			if(curid == spawn->GetRace() ||
			curid == race_types_list.GetRaceType(spawn->GetModelType()) ||
			curid == race_types_list.GetRaceBaseType(spawn->GetModelType())){
				ret = true;
				break;
			}
		}
	}
	return ret;
}

int16 QuestStep::GetIcon(){
	return icon;
}

void QuestStep::SetIcon(int16 in_icon){
	icon = in_icon;
}

const char* QuestStep::GetUpdateName(){
	if(update_name.length() > 0)
		return update_name.c_str();
	return 0;
}

void QuestStep::SetUpdateName(const char* name){
	update_name = string(name);
}

const char* QuestStep::GetUpdateTargetName(){
	if(update_target_name.length() > 0)
		return update_target_name.c_str();
	return 0;
}

void QuestStep::SetUpdateTargetName(const char* name){
	update_target_name = string(name);
}

bool QuestStep::CheckStepLocationUpdate(float char_x, float char_y, float char_z, int32 zone_id){
	bool ret = false;
	if (locations) {
		for (int32 i=0; i < locations->size(); i++) {
			float total_diff = 0;
			Location loc = locations->at(i);
			if(loc.zone_id > 0 && loc.zone_id != zone_id)
				continue;

			float diff = loc.x - char_x; //Check X
			if(diff < 0)
				diff *= -1;
			if(diff <=  max_variation) {
				total_diff += diff;

				diff = loc.z - char_z; //Check Z (we check Z first because it is far more likely to be a much greater variation than y)
				if(diff < 0)
					diff *= -1;
				if(diff <=  max_variation) {
					total_diff += diff;
					if(total_diff <=  max_variation) { //Check Total

						diff = loc.y - char_y; //Check Y
						if(diff < 0)
							diff *= -1;
						if(diff <=  max_variation) {
							total_diff += diff;
							if(total_diff <=  max_variation) { //Check Total
								ret = true;
								break;
							}
						}
					}
				}
			}	
		}
	}
	return ret;
}

void QuestStep::SetStepProgress(int32 val){
	step_progress = val;
}

int32 QuestStep::AddStepProgress(int32 val){
	updated = true;
	if (val > (quantity - step_progress)){
		step_progress += (quantity - step_progress);
		return (quantity - step_progress);
	}
	else
		step_progress += val;
	return val;
}

int32 QuestStep::GetStepProgress() {
	return step_progress;
}

void QuestStep::ResetTaskGroup(){
	task_group = "";
}

const char* QuestStep::GetTaskGroup(){
	if(task_group.length() > 0)
		return task_group.c_str();
	return 0;
}

const char* QuestStep::GetDescription(){
	if(description.length() > 0)
		return description.c_str();
	return 0;
}

void QuestStep::SetDescription(string desc){
	description = desc;
}

int16 QuestStep::GetQuestCurrentQuantity(){
	return (int16)step_progress;
}

int16 QuestStep::GetQuestNeededQuantity(){
	return (int16)quantity;
}

Quest::Quest(int32 in_id){
	id = in_id;
	reward_status = 0;
	quest_giver = 0;
	deleted = false;
	turned_in = false;
	update_needed = true;
	player = 0;
	return_id = 0;
	task_group_num = 1;
	prereq_level = 1;
	prereq_tslevel = 0;
	prereq_max_level = 0;
	prereq_max_tslevel = 0;
	reward_coins = 0;
	reward_coins_max = 0;
	completed_flag = false;
	has_sent_last_update = false;
	enc_level = 0;
	reward_exp = 0;
	reward_tsexp = 0;
	m_featherColor = 0;
	m_repeatable = false;
	yellow_name = false;
	m_hidden = false;
	generated_coin = 0;
	m_questFlags = 0;
	m_timestamp = 0;
	m_completeCount = 0;
	MQuestSteps.SetName("Quest::MQuestSteps");
	MCompletedActions.SetName("Quest::MCompletedActions");
	MProgressActions.SetName("Quest::MProgressActions");
	MFailedActions.SetName("Quest::failed_Actions");
	quest_state_temporary = false;
	tmp_reward_status = 0;
	tmp_reward_coins = 0;
	completed_description = string("");
	quest_temporary_description = string("");
	quest_shareable_flag = 0;
	can_delete_quest = false;
}

Quest::Quest(Quest* old_quest){
	task_group_num = 1;
	name = old_quest->name;
	type = old_quest->type;
	zone = old_quest->zone;
	level = old_quest->level;
	enc_level = old_quest->enc_level;
	description = old_quest->description;
	prereq_level = old_quest->prereq_level;
	prereq_tslevel = old_quest->prereq_tslevel;
	prereq_max_level = old_quest->prereq_max_level;
	prereq_max_tslevel = old_quest->prereq_max_tslevel;
	prereq_quests = old_quest->prereq_quests;
	prereq_classes = old_quest->prereq_classes;
	prereq_tradeskillclass = old_quest->prereq_tradeskillclass;
	prereq_races = old_quest->prereq_races;
	prereq_factions = old_quest->prereq_factions;
	reward_coins = old_quest->reward_coins;
	reward_coins_max = old_quest->reward_coins_max;
	reward_factions = old_quest->reward_factions;
	reward_status = old_quest->reward_status;
	reward_comment = old_quest->reward_comment;
	reward_exp = old_quest->reward_exp;
	reward_tsexp = old_quest->reward_tsexp;
	generated_coin = old_quest->generated_coin;
	completed_flag = old_quest->completed_flag;
	has_sent_last_update = old_quest->has_sent_last_update;
	yellow_name = old_quest->yellow_name;
	m_questFlags = old_quest->m_questFlags;
	id = old_quest->id;

	vector<QuestStep*>	quest_steps;
	for(int32 i=0;i<old_quest->quest_steps.size();i++)
		AddQuestStep(new QuestStep(old_quest->quest_steps[i]));
	for(int32 i=0;i<old_quest->prereq_items.size();i++)
		AddPrereqItem(new Item(old_quest->prereq_items[i]));
	for(int32 i=0;i<old_quest->reward_items.size();i++)
		AddRewardItem(new Item(old_quest->reward_items[i]));
	for(int32 i=0;i<old_quest->selectable_reward_items.size();i++)
		AddSelectableRewardItem(new Item(old_quest->selectable_reward_items[i]));
	complete_actions = old_quest->complete_actions;
	progress_actions = old_quest->progress_actions;
	failed_actions = old_quest->failed_actions;
	time_t raw;
	struct tm *newtime;
	time(&raw);
	newtime = localtime(&raw);
    day = newtime->tm_mday;
	month = newtime->tm_mon + 1;
	year = newtime->tm_year - 100;
	visible = 1;
	step_updates.clear();
	step_failures.clear();
	completed_description = old_quest->completed_description;
	deleted = old_quest->deleted;
	update_needed = old_quest->update_needed;
	turned_in = old_quest->turned_in;
	quest_giver = old_quest->quest_giver;
	player = old_quest->player;
	return_id = old_quest->return_id;
	m_featherColor = old_quest->m_featherColor;
	m_repeatable = old_quest->IsRepeatable();
	m_hidden = false;
	m_timestamp = 0;
	m_completeCount = old_quest->GetCompleteCount();
	MQuestSteps.SetName("Quest::MQuestSteps");
	MProgressActions.SetName("Quest::MProgressActions");
	MCompletedActions.SetName("Quest::MCompletedActions");
	quest_state_temporary = false;
	tmp_reward_status = 0;
	tmp_reward_coins = 0;
	quest_temporary_description = string("");
	quest_shareable_flag = old_quest->GetQuestShareableFlag();
	can_delete_quest = old_quest->CanDeleteQuest();
}

Quest::~Quest(){
	
	MQuestSteps.lock();
	for(int32 i=0;i<quest_steps.size();i++)
		safe_delete(quest_steps[i]);
	quest_steps.clear();
	quest_step_map.clear();
	MQuestSteps.unlock();
	
	for(int32 i=0;i<prereq_items.size();i++)
		safe_delete(prereq_items[i]);
	prereq_items.clear();
	
	for(int32 i=0;i<reward_items.size();i++)
		safe_delete(reward_items[i]);
	reward_items.clear();
	
	for(int32 i=0;i<tmp_reward_items.size();i++)
		safe_delete(tmp_reward_items[i]);
	tmp_reward_items.clear();
	
	for(int32 i=0;i<selectable_reward_items.size();i++)
		safe_delete(selectable_reward_items[i]);
	selectable_reward_items.clear();

	task_group_order.clear();
	task_group.clear();
	complete_actions.clear();
	progress_actions.clear();
	failed_actions.clear();
}

void Quest::RegisterQuest(string in_name, string in_type, string in_zone, int8 in_level, string in_desc){
	name = in_name;
	type = in_type;
	zone = in_zone;
	level = in_level;
	description = in_desc;
}

const char* Quest::GetDescription(){
	return description.c_str();
}

const char* Quest::GetName(){
	return name.c_str();
}

const char* Quest::GetType(){
	return type.c_str();
}

void Quest::SetZone(string in_zone) {
	zone = in_zone;
}

const char* Quest::GetZone(){
	return zone.c_str();
}

int8 Quest::GetLevel(){
	return level;
}

int8 Quest::GetDay(){
	return day;
}

int8 Quest::GetMonth(){
	return month;
}

int8 Quest::GetYear(){
	return year;
}

int32 Quest::GetStatusPoints(){
	return reward_status;
}

void Quest::SetDay(int8 value){
	day = value;
}

void Quest::SetMonth(int8 value){
	month = value;
}

void Quest::SetYear(int8 value){
	year = value;
}

QuestStep* Quest::GetQuestStep(int32 step_id){
	QuestStep* ret = 0;
	MQuestSteps.lock();
	for(int32 i=0;i<quest_steps.size(); i++){
		if(quest_steps[i] && quest_steps[i]->GetStepID() == step_id){
			ret = quest_steps[i];
			break;
		}
	}
	MQuestSteps.unlock();
	return ret;
}

vector<QuestStep*>* Quest::GetQuestSteps(){
	return &quest_steps;
}

vector<QuestStep*>* Quest::GetQuestUpdates(){
	return &step_updates;
}

vector<QuestStep*>* Quest::GetQuestFailures(){
	return &step_failures;
}

bool Quest::CheckQuestReferencedSpawns(Spawn* spawn){
	QuestStep* step = 0;
	bool ret = false;
	int32 spawnDBID = spawn->GetDatabaseID();
	
	MQuestSteps.lock();
	for(int32 i=0;i<quest_steps.size(); i++){
		step = quest_steps[i];
		if(!step || step->Complete())
			continue;
			switch(step->GetStepType())
			{
				case QUEST_STEP_TYPE_KILL:
				case QUEST_STEP_TYPE_NORMAL: {
					if(step->CheckStepReferencedID(spawnDBID))
						ret = true;
					
					break;
				}
				case QUEST_STEP_TYPE_KILL_RACE_REQ: {
					if(step->CheckStepKillRaceReqUpdate(spawn))
						ret = true;

					break;
				}
			}
	}
	MQuestSteps.unlock();
	return ret;
}

bool Quest::CheckQuestKillUpdate(Spawn* spawn, bool update){
	QuestStep* step = 0;
	bool ret = false;
	int32 id = spawn->GetDatabaseID();
	int32 prog_added = 0;
	MQuestSteps.lock();
	for(int32 i=0;i<quest_steps.size(); i++){
		step = quest_steps[i];
		if(!step)
			continue;

			if((step->GetStepType() == QUEST_STEP_TYPE_KILL && !step->Complete() && step->CheckStepReferencedID(id)) ||
			 (step->GetStepType() == QUEST_STEP_TYPE_KILL_RACE_REQ && !step->Complete() && step->CheckStepKillRaceReqUpdate(spawn)))
			{
				if (update == true) {
					bool passed = true;
					if (step->GetPercentage() < 100)
						passed = (step->GetPercentage() > MakeRandomFloat(0, 100));
					if (passed) {
						//Call the progress action function with the total amount of progress actually granted
						prog_added = step->AddStepProgress(1);
						if (lua_interface && progress_actions[step->GetStepID()].length() > 0 && prog_added > 0)
							lua_interface->CallQuestFunction(this, progress_actions[step->GetStepID()].c_str(), player, prog_added);
						step_updates.push_back(step);
						step->SetUpdateName(spawn->GetName());
						ret = true;
					}
					else
						step_failures.push_back(step);
				}
				else {
					ret = true;
				}
		}
	}
	MQuestSteps.unlock();
	if(ret && update)
		SetSaveNeeded(true);
	return ret;
}

int16 Quest::GetTaskGroupStep(){
	int16 ret = task_group_order.size();
	map<int16, string>::iterator itr;
	MQuestSteps.lock();
	for(itr = task_group_order.begin(); itr != task_group_order.end(); itr++){
		if(task_group.count(itr->second) > 0){
			vector<QuestStep*> tmp_steps = task_group[itr->second];
			bool complete = true;
			for(int32 i=0;i<tmp_steps.size();i++){
				if(tmp_steps[i]->Complete() == false)
					complete = false;
			}
			if(!complete){
				if(itr->first < ret)
					ret = itr->first;
			}
		}
	}
	MQuestSteps.unlock();
	return ret;
}

bool Quest::SetStepComplete(int32 step){
	QuestStep* quest_step = 0;
	bool ret = false;
	MQuestSteps.lock();
	for(int32 i=0;i<quest_steps.size(); i++){
		quest_step = quest_steps[i];
		if(quest_step && quest_step->GetStepID() == step){
			quest_step->SetComplete();
			step_updates.push_back(quest_step);
			ret = true;
			break;
		}
	}
	MQuestSteps.unlock();
	if (ret) {
		SetSaveNeeded(true);
	}

	return ret;
}

bool Quest::AddStepProgress(int32 step_id, int32 progress) {
	QuestStep* quest_step = 0;
	bool ret = false;
	int32 prog_added = 0;
	MQuestSteps.lock();
	for (int32 i = 0; i < quest_steps.size(); i++) {
		quest_step = quest_steps[i];
		if (quest_step && quest_step->GetStepID() == step_id) {
			bool passed = true;
			if(quest_step->GetPercentage() < 100 && quest_step->GetPercentage() != 0)
				passed = (quest_step->GetPercentage() > MakeRandomFloat(0, 100));
			if(passed) {
				//Call the progress action function with the total amount of progress actually granted
				prog_added = quest_step->AddStepProgress(progress);
				if(lua_interface && progress_actions[step_id].length() > 0 && prog_added > 0)
					lua_interface->CallQuestFunction(this, progress_actions[step_id].c_str(), player, prog_added);
				step_updates.push_back(quest_step);
				ret = true;
				break;
			}
			else {
				step_failures.push_back(quest_step);
				break;
			}
		}
	}
	MQuestSteps.unlock();
	if(ret)
		SetSaveNeeded(true);
	return ret;
}

int32 Quest::GetStepProgress(int32 step_id) {
	QuestStep* quest_step = 0;
	int32 ret = 0;

	MQuestSteps.lock();
	for (int32 i = 0; i < quest_steps.size(); i++) {
		quest_step = quest_steps[i];
		if (quest_step && quest_step->GetStepID() == step_id) {
			ret = quest_step->GetStepProgress();
			break;
		}
	}
	MQuestSteps.unlock();

	return ret;
}

bool Quest::GetQuestStepCompleted(int32 step_id){
	bool ret = false;
	QuestStep* quest_step = 0;
	MQuestSteps.lock();
	for(int32 i=0;i<quest_steps.size(); i++){
		quest_step = quest_steps[i];
		if(quest_step && quest_step->GetStepID() == step_id && quest_step->Complete()){
			ret = true;
			break;
		}
	}
	MQuestSteps.unlock();
	return ret;
}

int16 Quest::GetQuestStep(){
	int16 ret = 0;
	QuestStep* step = 0;
	MQuestSteps.lock();
	for(int32 i=0;i<quest_steps.size(); i++){
		step = quest_steps[i];
		if(step && !step->Complete()){
			ret = step->GetStepID();
			break;
		}
	}
	MQuestSteps.unlock();
	return ret;
}

bool Quest::QuestStepIsActive(int16 quest_step_id) {
	bool ret = false;
	QuestStep* step = 0;
	MQuestSteps.lock();
	for (int32 i = 0; i < quest_steps.size(); i++) {
		step = quest_steps[i];
		if (step->GetStepID() == quest_step_id && !step->Complete()) {
			ret = true;
			break;
		}
	}
	MQuestSteps.unlock();
	return ret;
}

bool Quest::CheckQuestChatUpdate(int32 id, bool update){
	QuestStep* step = 0;
	bool ret = false;
	int32 prog_added = 0;
	MQuestSteps.lock();
	for(int32 i=0;i<quest_steps.size(); i++){
		step = quest_steps[i];
		if(step && step->GetStepType() == QUEST_STEP_TYPE_CHAT && !step->Complete() && step->CheckStepReferencedID(id)){
			if(update){
				//Call the progress action function with the total amount of progress actually granted
				prog_added = step->AddStepProgress(1);
				if(lua_interface && progress_actions[step->GetStepID()].length() > 0 && prog_added > 0)
					lua_interface->CallQuestFunction(this, progress_actions[step->GetStepID()].c_str(), player, prog_added);
				step_updates.push_back(step);
			}
			ret = true;
		}
	}
	MQuestSteps.unlock();
	if(ret)
		SetSaveNeeded(true);
	return ret;
}

bool Quest::CheckQuestItemUpdate(int32 id, int8 quantity){
	QuestStep* step = 0;
	bool ret = false;
	int32 prog_added = 0;
	MQuestSteps.lock();
	for(int32 i=0;i<quest_steps.size(); i++){
		step = quest_steps[i];
		if(step && step->GetStepType() == QUEST_STEP_TYPE_OBTAIN_ITEM && !step->Complete() && step->CheckStepReferencedID(id)){
			bool passed = true;
			if(step->GetPercentage() < 100)
				passed = (step->GetPercentage() > MakeRandomFloat(0, 100));
			if(passed){
				//Call the progress action function with the total amount of progress actually granted
				prog_added = step->AddStepProgress(quantity);
				if(lua_interface && progress_actions[step->GetStepID()].length() > 0 && prog_added > 0)
					lua_interface->CallQuestFunction(this, progress_actions[step->GetStepID()].c_str(), player, prog_added);
				step_updates.push_back(step);
				ret = true;
			}
			else
				step_failures.push_back(step);
		}
	}
	MQuestSteps.unlock();
	if(ret)
		SetSaveNeeded(true);
	return ret;
}

bool Quest::CheckQuestRefIDUpdate(int32 id, int32 quantity){
	QuestStep* step = 0;
	bool ret = false;
	int32 prog_added = 0;
	MQuestSteps.lock();
	for(int32 i=0;i<quest_steps.size(); i++){
		step = quest_steps[i];
		if(step)
		{
			if(step->Complete())
				continue;

			switch(step->GetStepType()) {
				case QUEST_STEP_TYPE_HARVEST:
				case QUEST_STEP_TYPE_CRAFT: {
					map<int32, bool>* id_list = step->GetUpdateIDs();
					map<int32, bool>::iterator itr;
					for(itr = id_list->begin();itr != id_list->end(); itr++){
						int32 update_id = itr->first;
						if(update_id == id){
							bool passed = true;
							if(step->GetPercentage() < 100)
								passed = (step->GetPercentage() > MakeRandomFloat(0, 100));
							if(passed){
								//Call the progress action function with the total amount of progress actually granted
								prog_added = step->AddStepProgress(quantity);
								if(lua_interface && progress_actions[step->GetStepID()].length() > 0 && prog_added > 0)
									lua_interface->CallQuestFunction(this, progress_actions[step->GetStepID()].c_str(), player, prog_added);
								step_updates.push_back(step);
							}
							else
								step_failures.push_back(step);
							ret = true;
						}
					}
					break;
				}
			}
		}
	}
	MQuestSteps.unlock();
	if(ret)
		SetSaveNeeded(true);
	return ret;
}

bool Quest::CheckQuestLocationUpdate(float char_x, float char_y, float char_z, int32 zone_id){
	QuestStep* step = 0;
	bool ret = false;
	int32 prog_added = 0;
	MQuestSteps.lock();
	for(int32 i=0;i<quest_steps.size(); i++){
		step = quest_steps[i];
		if(step && step->GetStepType() == QUEST_STEP_TYPE_LOCATION && !step->Complete() && step->CheckStepLocationUpdate(char_x, char_y, char_z, zone_id)){
			//Call the progress action function with the total amount of progress actually granted
			prog_added = step->AddStepProgress(1);
			if(lua_interface && progress_actions[step->GetStepID()].length() > 0 && prog_added > 0)
				lua_interface->CallQuestFunction(this, progress_actions[step->GetStepID()].c_str(), player, prog_added);
			step_updates.push_back(step);
			ret = true;
		}
	}
	MQuestSteps.unlock();
	if(ret)
		SetSaveNeeded(true);
	return ret;
}

bool Quest::CheckQuestSpellUpdate(Spell* spell){
	QuestStep* step = 0;
	bool ret = false;
	int32 id = spell->GetSpellID();
	int32 prog_added = 0;
	MQuestSteps.lock();
	for (int32 i = 0; i < quest_steps.size(); i++) {
		step = quest_steps[i];
		if(step && step->GetStepType() == QUEST_STEP_TYPE_SPELL && !step->Complete() && step->CheckStepReferencedID(id)){
			bool passed = true;
			if(step->GetPercentage() < 100)
				passed = (step->GetPercentage() > MakeRandomFloat(0, 100));
			if(passed) {
				//Call the progress action function with the total amount of progress actually granted
				prog_added = step->AddStepProgress(1);
				if(lua_interface && progress_actions[step->GetStepID()].length() > 0 && prog_added > 0)
					lua_interface->CallQuestFunction(this, progress_actions[step->GetStepID()].c_str(), player, prog_added);
				step_updates.push_back(step);
				//step->SetUpdateName(spawn->GetName());
				ret = true;
			}
			else
				step_failures.push_back(step);
		}
	}
	MQuestSteps.unlock();
	if (ret)
		SetSaveNeeded(true);
	return ret;
}

void Quest::SetQuestID(int32 in_id){
	id = in_id;
}

EQ2Packet* Quest::OfferQuest(int16 version, Player* player){
	PacketStruct* packet = configReader.getStruct("WS_OfferQuest", version);
	if(packet){	
		packet->setDataByName("reward", "Quest Reward!");
		if(packet->GetVersion() <= 373) {
				std::string quotedName = std::string("\"" + name + "\"");
			packet->setDataByName("title", quotedName.c_str());
		}
		else {
			packet->setDataByName("title", name.c_str());
		}
		packet->setDataByName("description", description.c_str());
		if(type == "Tradeskill")
			packet->setDataByName("quest_difficulty", player->GetTSArrowColor(level));
		else
			packet->setDataByName("quest_difficulty", player->GetArrowColor(level));
		packet->setDataByName("encounter_level", enc_level);
		packet->setDataByName("unknown0", 255);
		if (version >= 1188) 
			packet->setDataByName("unknown4b", 1);
		else
			packet->setDataByName("unknown", 5);
		packet->setDataByName("level", level);
		if(reward_coins > 0){
			packet->setDataByName("min_coin", reward_coins);
			if (reward_coins_max)
				packet->setDataByName("max_coin", reward_coins_max);
		}
		packet->setDataByName("status_points", reward_status);
		if(reward_comment.length() > 0)
			packet->setDataByName("text", reward_comment.c_str());
		if(reward_items.size() > 0){
			player->GetClient()->PopulateQuestRewardItems(&reward_items, packet);
		}
		if(selectable_reward_items.size() > 0){
			player->GetClient()->PopulateQuestRewardItems(&selectable_reward_items, packet, std::string("num_select_rewards"), 
										std::string("select_reward_id"), std::string("select_item"));
		}
		map<int32, sint32>* reward_factions = GetRewardFactions();
		if (reward_factions && reward_factions->size() > 0) {
			packet->setArrayLengthByName("num_factions", reward_factions->size());
			map<int32, sint32>::iterator itr;
			int16 index = 0;
			for (itr = reward_factions->begin(); itr != reward_factions->end(); itr++) {
				int32 faction_id = itr->first;
				sint32 amount = itr->second;
				const char* faction_name = master_faction_list.GetFactionNameByID(faction_id);
				if (faction_name) {
					packet->setArrayDataByName("faction_name", const_cast<char*>(faction_name), index);
					packet->setArrayDataByName("amount", amount, index);
				}
				index++;
			}
		}
		char accept[35] = {0};
		char decline[35] = {0};
		sprintf(accept, "q_accept_pending_quest %u", id);
		sprintf(decline, "q_deny_pending_quest %u", id);
		packet->setDataByName("accept_command", accept);
		packet->setDataByName("decline_command", decline);
		EQ2Packet* outapp = packet->serialize();
#if EQDEBUG >= 9
		DumpPacket(outapp);
#endif
		safe_delete(packet);
		return outapp;
	}
	return 0;
}

int32 Quest::GetQuestID(){
	return id;
}

int8 Quest::GetQuestLevel(){
	return level;
}

int8 Quest::GetVisible(){
	return visible;
}

EQ2Packet* Quest::QuestJournalReply(int16 version, int32 player_crc, Player* player, QuestStep* updateStep, int8 update_count, bool old_completed_quest, bool quest_failure, bool display_quest_helper) {
	PacketStruct* packet = configReader.getStruct("WS_QuestJournalReply", version);
	if (packet) {
		packet->setDataByName("quest_id", id);
		packet->setDataByName("player_crc", player_crc);
		packet->setDataByName("name", name.c_str());
		packet->setDataByName("description", description.c_str());
		packet->setDataByName("zone", zone.c_str());
		packet->setDataByName("type", type.c_str());
		packet->setDataByName("complete_header", "To complete this quest, I must do the following tasks:");
		packet->setDataByName("day", day);
		packet->setDataByName("month", month);
		packet->setDataByName("year", year);
		packet->setDataByName("level", level);
		packet->setDataByName("visible", 1);		
		/* To get the quest timer to work you need to set unknown, index 4 to 1 and the time stamp
		to the current time + the time in seconds you want to show in the journal*/
		if (m_timestamp > 0) {
			packet->setDataByName("unknown", 1, 4);
			packet->setDataByName("time_stamp", m_timestamp);
		}

		int8 difficulty = 0;
		if (type == "Tradeskill")
			difficulty = player->GetTSArrowColor(level);
		else
			difficulty = player->GetArrowColor(level);
		packet->setDataByName("difficulty", difficulty);
		if (enc_level > 4)
			packet->setDataByName("encounter_level", enc_level);
		else
			packet->setDataByName("encounter_level", 4);
		packet->setDataByName("unknown2b", 4);
		int16 task_groups_completed = 0;
		int16 total_task_groups = 0;
		vector<QuestStep*> primary_order;
		vector<QuestStep*> secondary_order;
		packet->setDataByName("unknown8", 1, 1);
		packet->setSubstructArrayDataByName("reward_data", "unknown8", 1, 1);
		packet->setDataByName("unknown8b", 255);

		if (version >= 1096) {
			packet->setDataByName("deletable", (int8)CanDeleteQuest());
			packet->setDataByName("shareable", (int8)CanShareQuestCriteria(player->GetClient(),false));
		}
		else {
			packet->setDataByName("unknown3", 1, 5); // this supposed to be CanDeleteQuest?
			packet->setDataByName("unknown3", 1, 6); // this supposed to be CanShareQuestCriteria?
		}

		int8 map_data_count = 0;

		packet->setDataByName("bullets", 1);
		if (old_completed_quest) {
			if (version >= 1096 || version == 546 || version == 561) {
				packet->setDataByName("complete", 1);
				packet->setDataByName("complete2", 1);
				packet->setDataByName("complete3", 1);
			}
			else {
				packet->setDataByName("unknown3", 1);
				packet->setDataByName("unknown3", 1, 1);
				packet->setDataByName("unknown3", 1, 2);
				packet->setDataByName("unknown3", 1, 5);
				packet->setDataByName("unknown3", 1, 6);
			}
		}
		// must always send for newer clients like AoM or else crash!
		else if (GetCompleted() && ((version >= 1096) || ((version == 561 || version == 546) && HasSentLastUpdate()))) { //need to send last quest update before erasing all progress of the quest
			packet->setDataByName("complete", 1);
			packet->setDataByName("complete2", 1);
			packet->setDataByName("complete3", 1);
			packet->setDataByName("num_task_groups", 1);
			packet->setArrayDataByName("task_group", completed_description.c_str());
			packet->setArrayDataByName("unknown4", 0xFFFFFFFF);
			packet->setDataByName("onscreen_update", 1);
			packet->setDataByName("update_task_number", 1);
			packet->setDataByName("onscreen_update", 1);
			packet->setDataByName("onscreen_update_text", completed_description.c_str());
			if (updateStep)
				packet->setDataByName("onscreen_update_icon", updateStep->GetIcon());
			else
				packet->setDataByName("onscreen_update_icon", 0);
			// Is Heritage or db entry for classic eq quest turn in sound
			packet->setDataByName("classic_eq_sound", 0);
			// No clue what this does, set it to mach live packets
			packet->setDataByName("unknown12b", 1, 0);
		}
		else {
			map<QuestStep*, string> task_group_names;
			map<int16, string>::iterator order_itr;
			vector<QuestStep*>* steps = 0;
			MQuestSteps.lock();
			for (order_itr = task_group_order.begin(); order_itr != task_group_order.end(); order_itr++) {
				//The following is kind of crazy, but necessary to order the quests with completed ones first.  This is to make the packet like live's packet
				//for(itr = task_group.begin(); itr != task_group.end(); itr++){
				bool complete = true;
				if (task_group.count(order_itr->second) > 0) {
					steps = &(task_group[order_itr->second]);
					for (int32 i = 0; i < steps->size(); i++) {
						if (steps->at(i)->Complete() == false)
							complete = false;
					}
					if (complete) {
						for (int32 i = 0; i < steps->size(); i++) {
							if (i == 0)
								task_group_names[steps->at(i)] = order_itr->second;
							primary_order.push_back(steps->at(i));
						}
						task_groups_completed++;
						total_task_groups++;
					}
					else {
						for (int32 i = 0; i < steps->size(); i++) {
							if (i == 0)
								task_group_names[steps->at(i)] = order_itr->second;
							secondary_order.push_back(steps->at(i));
						}
						total_task_groups++;
					}
				}
			}
			packet->setDataByName("task_groups_completed", task_groups_completed);
			packet->setArrayLengthByName("num_task_groups", total_task_groups);
			if (IsTracked() /*display_quest_helper*/ && task_groups_completed < total_task_groups)
				packet->setDataByName("display_quest_helper", 1);
			int16 index = 0;
			QuestStep* step = 0;
			for (int32 i = 0; i < primary_order.size(); i++) {
				if (primary_order[i]->GetTaskGroup()) {
					if (task_group_names.count(primary_order[i]) > 0)
						packet->setArrayDataByName("task_group", primary_order[i]->GetTaskGroup(), index);
					else
						continue;
					packet->setSubArrayLengthByName("num_tasks", task_group[primary_order[i]->GetTaskGroup()].size(), index);
					packet->setSubArrayLengthByName("num_updates", task_group[primary_order[i]->GetTaskGroup()].size(), index);
					map_data_count += task_group[primary_order[i]->GetTaskGroup()].size();
					if (task_group[primary_order[i]->GetTaskGroup()].size() > 0) {
						packet->setDataByName("bullets", 1);						
					}
					for (int32 x = 0; x < task_group[primary_order[i]->GetTaskGroup()].size(); x++) {
						step = task_group[primary_order[i]->GetTaskGroup()].at(x);
						if (!step)
							continue;
						if (step->GetDescription())
							packet->setSubArrayDataByName("task", step->GetDescription(), index, x);
						packet->setSubArrayDataByName("task_completed", 1, index, x);
						packet->setSubArrayDataByName("index", x, index, x);
						packet->setSubArrayDataByName("update_currentval", step->GetQuestCurrentQuantity(), index, x);
						if(step->GetQuestCurrentQuantity() > 0)
							packet->setDataByName("journal_updated", 1);
						packet->setSubArrayDataByName("update_maxval", step->GetQuestNeededQuantity(), index, x);
						if (step->GetUpdateTargetName())
							packet->setSubArrayDataByName("update_target_name", step->GetUpdateTargetName(), index, x);
						packet->setSubArrayDataByName("icon", step->GetIcon(), index, x);
						if (updateStep && step == updateStep) {
							packet->setDataByName("update", 1);							
							//	packet->setDataByName("unknown5d", 1);
							if (!quest_failure)
								packet->setDataByName("onscreen_update", 1);
							packet->setDataByName("onscreen_update_count", update_count);
							packet->setDataByName("onscreen_update_icon", step->GetIcon());
							if (step->GetUpdateName())
								packet->setDataByName("onscreen_update_text", step->GetUpdateName());
							else if (step->GetDescription())
								packet->setDataByName("onscreen_update_text", step->GetDescription());
							packet->setDataByName("update_task_number", x);
							packet->setDataByName("update_taskgroup_number", index);
						}
						packet->setArrayDataByName("unknown4", 0xFFFFFFFF, x);
					}
					index++;
				}
				else {
					if (task_group_names.count(primary_order[i]) > 0) {
						step = primary_order[i];
						if (updateStep && step == updateStep) {
							packet->setDataByName("update", 1);
							//	packet->setDataByName("unknown5d", 1);
							if (!quest_failure)
								packet->setDataByName("onscreen_update", 1);
							packet->setDataByName("onscreen_update_count", update_count);
							packet->setDataByName("onscreen_update_icon", step->GetIcon());
							if (step->GetUpdateName())
								packet->setDataByName("onscreen_update_text", step->GetUpdateName());
							else if (step->GetDescription())
								packet->setDataByName("onscreen_update_text", step->GetDescription());
							packet->setDataByName("update_task_number", i);
							packet->setDataByName("update_taskgroup_number", index);
						}
						packet->setArrayDataByName("task_group", primary_order[i]->GetDescription(), index);
						index++;
					}
				}
			}
			for (int32 i = 0; i < secondary_order.size(); i++) {
				if (secondary_order[i]->GetTaskGroup()) {
					if (task_group_names.count(secondary_order[i]) > 0)
						packet->setArrayDataByName("task_group", secondary_order[i]->GetTaskGroup(), index);
					else
						continue;
					packet->setSubArrayLengthByName("num_tasks", task_group[secondary_order[i]->GetTaskGroup()].size(), index);
					packet->setSubArrayLengthByName("num_updates", task_group[secondary_order[i]->GetTaskGroup()].size(), index);
					map_data_count += task_group[secondary_order[i]->GetTaskGroup()].size();
					if (task_group[secondary_order[i]->GetTaskGroup()].size() > 0)
						packet->setDataByName("bullets", 1);
					for (int32 x = 0; x < task_group[secondary_order[i]->GetTaskGroup()].size(); x++) {
						step = task_group[secondary_order[i]->GetTaskGroup()].at(x);
						if (!step)
							continue;
						if (step->GetDescription())
							packet->setSubArrayDataByName("task", step->GetDescription(), index, x);
						if (step->Complete())
							packet->setSubArrayDataByName("task_completed", 1, index, x);
						else
							packet->setSubArrayDataByName("task_completed", 0, index, x);
						packet->setSubArrayDataByName("index", x, index, x);
						packet->setSubArrayDataByName("update_currentval", step->GetQuestCurrentQuantity(), index, x);
						if (step->GetQuestCurrentQuantity() > 0)
							packet->setDataByName("journal_updated", 1);
						packet->setSubArrayDataByName("update_maxval", step->GetQuestNeededQuantity(), index, x);
						packet->setSubArrayDataByName("icon", step->GetIcon(), index, x);
						if (step->GetUpdateTargetName())
							packet->setSubArrayDataByName("update_target_name", step->GetUpdateTargetName(), index, x);
						if (updateStep && step == updateStep) {
							packet->setDataByName("update", 1);
							if (!quest_failure)
								packet->setDataByName("onscreen_update", 1);
							packet->setDataByName("onscreen_update_count", update_count);
							packet->setDataByName("onscreen_update_icon", step->GetIcon());
							if (step->GetUpdateName())
								packet->setDataByName("onscreen_update_text", step->GetUpdateName());
							else if (step->GetDescription())
								packet->setDataByName("onscreen_update_text", step->GetDescription());
							if (quest_failure)
								packet->setDataByName("onscreen_update_text2", "failed");
							packet->setDataByName("update_task_number", x);
							packet->setDataByName("update_taskgroup_number", index);
						}
						packet->setArrayDataByName("unknown4", 0xFFFFFFFF, x);
					}
					index++;
				}
				else {
					if (task_group_names.count(secondary_order[i]) > 0) {
						step = secondary_order[i];
						if (updateStep && step == updateStep) {							
							packet->setDataByName("update", 1);
							if (!quest_failure)
								packet->setDataByName("onscreen_update", 1);
							packet->setDataByName("onscreen_update_count", update_count);
							packet->setDataByName("onscreen_update_icon", step->GetIcon());
							if (step->GetUpdateName())
								packet->setDataByName("onscreen_update_text", step->GetUpdateName());
							else if (step->GetDescription())
								packet->setDataByName("onscreen_update_text", step->GetDescription());
							packet->setDataByName("update_task_number", i);
							packet->setDataByName("update_taskgroup_number", index);
						}
						if (task_group_names.size() == 1) {
							packet->setSubArrayLengthByName("num_tasks", 1, index);
							packet->setSubArrayDataByName("task", secondary_order[i]->GetDescription(), index);
							packet->setDataByName("bullets", 0);
						}
						else
							packet->setArrayDataByName("task_group", secondary_order[i]->GetDescription(), index);
						index++;
					}
				}
			}

			for (int16 i = 0; i < total_task_groups; i++)
				packet->setArrayDataByName("unknown4", 0xFFFFFFFF, i);

			if (step && step->GetItemID() > 0) {
				packet->setArrayLengthByName("usable_item_count", 1);
				Item* item = player->GetPlayerItemList()->GetItemFromID(step->GetItemID(), 1);
				if (item) {
					packet->setArrayDataByName("item_id", item->details.item_id, 0);
					packet->setArrayDataByName("item_unique_id", item->details.unique_id, 0);
					packet->setArrayDataByName("item_icon", item->GetIcon(version), 0);
					packet->setArrayDataByName("unknown2", 0xFFFFFFFF, 0);//item->details.item_id, 0);

				}
			}
			if (GetCompleted()) { //mark the last update as being sent, next time we send the quest reply, it will only be a brief portion
				SetSentLastUpdate(true);
			}
		}
		MQuestSteps.unlock();


		string reward_str = "";
		if (version >= 1096 || version == 546 ||  version == 561)
			reward_str = "reward_data_";
		string tmp = reward_str + "reward";
		packet->setDataByName(tmp.c_str(), "Quest Reward!");
		if (reward_coins > 0) {
			tmp = reward_str + "min_coin";
			packet->setDataByName(tmp.c_str(), reward_coins);
			tmp = reward_str + "max_coin";
			packet->setDataByName(tmp.c_str(), reward_coins_max);
		}
		tmp = reward_str + "status_points";
		packet->setDataByName(tmp.c_str(), reward_status);
		if (reward_comment.length() > 0) {
			tmp = reward_str + "text";
			packet->setDataByName(tmp.c_str(), reward_comment.c_str());
		}
		if (reward_items.size() > 0) {
			tmp = reward_str + "num_rewards";
			packet->setArrayLengthByName(tmp.c_str(), reward_items.size());
			Item* item = 0;
			for (int32 i = 0; i < reward_items.size(); i++) {
				item = reward_items[i];
				packet->setArrayDataByName("reward_id", item->details.item_id, i);
				if (version < 860)
					packet->setItemArrayDataByName("item", item, player, i, 0, version <= 373 ? -2 : -1);
				else if (version < 1193)
					packet->setItemArrayDataByName("item", item, player, i);
				else
					packet->setItemArrayDataByName("item", item, player, i, 0, 2);
			}
		}
		if (selectable_reward_items.size() > 0) {
			tmp = reward_str + "num_select_rewards";
			packet->setArrayLengthByName(tmp.c_str(), selectable_reward_items.size());
			Item* item = 0;
			for (int32 i = 0; i < selectable_reward_items.size(); i++) {
				item = selectable_reward_items[i];
				packet->setArrayDataByName("select_reward_id", item->details.item_id, i);
				if (version < 860)
					packet->setItemArrayDataByName("select_item", item, player, i, 0, version <= 373 ? -2 : -1);
				else if (version < 1193)
					packet->setItemArrayDataByName("select_item", item, player, i);
				else
					packet->setItemArrayDataByName("select_item", item, player, i, 0, 2);
			}
		}
		map<int32, sint32>* reward_factions = GetRewardFactions();
		if (reward_factions && reward_factions->size() > 0) {
			tmp = reward_str + "num_factions";
			packet->setArrayLengthByName(tmp.c_str(), reward_factions->size());
			map<int32, sint32>::iterator itr;
			int16 index = 0;
			for (itr = reward_factions->begin(); itr != reward_factions->end(); itr++) {
				int32 faction_id = itr->first;
				sint32 amount = itr->second;
				const char* faction_name = master_faction_list.GetFactionNameByID(faction_id);
				if (faction_name) {
					packet->setArrayDataByName("faction_name", const_cast<char*>(faction_name), index);
					packet->setArrayDataByName("amount", amount, index);
				}
				index++;
			}
		}

		//packet->setArrayLengthByName("map_data_array_size", map_data_count);

		EQ2Packet* outapp = packet->serialize();
		//packet->PrintPacket();
		//DumpPacket(outapp);
		safe_delete(packet);
		return outapp;
	}

	return 0;
}

bool Quest::AddQuestStep(QuestStep* step){
	bool ret = true;
	MQuestSteps.lock();
	if(quest_step_map.count(step->GetStepID()) == 0){
		quest_steps.push_back(step);
		quest_step_reverse_map[step] = step->GetStepID(); 
		quest_step_map[step->GetStepID()] = step;
		if(step->GetTaskGroup()){
			string tmp_task_group = string(step->GetTaskGroup());
			if(task_group.count(tmp_task_group) == 0){
				task_group_order[task_group_num] = tmp_task_group;
				task_group_num++;
			}
			task_group[tmp_task_group].push_back(step);
		}
		else{
			string tmp_task_group = string(step->GetDescription());
			if(task_group.count(tmp_task_group) == 0){
				task_group_order[task_group_num] = tmp_task_group;
				task_group_num++;
			}
			task_group[tmp_task_group].push_back(step);
		}
	}
	else{
		LogWrite(QUEST__ERROR, 0, "Quest", "Quest Warning in '%s': step %u used multiple times!", GetName(), step->GetStepID());
		ret = false;
	}
	MQuestSteps.unlock();
	return ret;
}

bool Quest::RemoveQuestStep(int32 step, Client* client) {
	bool ret = true;
	MQuestSteps.writelock(__FUNCTION__, __LINE__);
	if (quest_step_map.count(step) > 0) {
		QuestStep* quest_step = quest_step_map[step];
		if (quest_step) {
			client->QueuePacket(QuestJournalReply(client->GetVersion(), client->GetNameCRC(), client->GetPlayer(), quest_step, 0, 0, true));
			int32 step2 = step - 1;
			QuestStep*  reset_step = quest_step_map[step2];
			reset_step->SetStepProgress(0);
			MCompletedActions.lock();
			complete_actions.erase(step);
			MCompletedActions.unlock();
			string tmp_task_group = string(quest_step->GetTaskGroup());
			if(task_group.count(tmp_task_group) > 0) {
				task_group.erase(tmp_task_group);
				int task_num = 0;
				map<int16, string>::iterator itr;
				for (itr = task_group_order.begin(); itr != task_group_order.end(); itr++) {
					if (itr->second == tmp_task_group) {
						task_num = itr->first;
						break;
					}
				}
				if (task_num > 0) {
					task_group_order.erase(task_num);
					task_group_num--;
				}
			}

			// Remove the step from the various maps before we delete it
			quest_step_map.erase(step);
			quest_step_reverse_map.erase(quest_step);
			vector<QuestStep*>::iterator itr;
			for (itr = quest_steps.begin(); itr != quest_steps.end(); itr++) {
				if ((*itr) == quest_step) {
					quest_steps.erase(itr);
					break;
				}
			}
			safe_delete(quest_step);
		}
		else {
			LogWrite(QUEST__ERROR, 0, "Quest", "Unable to get a valid step (%u) for quest %s", step, GetName());
			ret = false;
		}
	}
	else {
		LogWrite(QUEST__ERROR, 0, "Quest", "Unable to remove step (%u) for quest %s", step, GetName());
		ret = false;
	}
	MQuestSteps.releasewritelock(__FUNCTION__, __LINE__);
	return ret;
}

QuestStep* Quest::AddQuestStep(int32 id, int8 in_type, string in_description, vector<int32>* in_ids, int32 in_quantity, const char* in_task_group, vector<Location>* in_locations, float in_max_variation, float in_percentage,int32 in_usableitemid){
	QuestStep* step = new QuestStep(id, in_type, in_description, in_ids, in_quantity, in_task_group, in_locations, in_max_variation, in_percentage, in_usableitemid);
	if(!AddQuestStep(step)){
		safe_delete(step);
		return 0;
	}
	return step;
}

void Quest::AddPrereqClass(int8 class_id){
	prereq_classes.push_back(class_id);
}

void Quest::AddPrereqTradeskillClass(int8 class_id) {
	prereq_tradeskillclass.push_back(class_id);
}

void Quest::AddPrereqModelType(int16 model_type){
	prereq_model_types.push_back(model_type);
}

void Quest::AddPrereqRace(int8 race){
	prereq_races.push_back(race);
}

void Quest::SetPrereqLevel(int8 lvl){
	prereq_level = lvl;
}

void Quest::SetPrereqTSLevel(int8 lvl) {
	prereq_tslevel = lvl;
}

void Quest::AddPrereqQuest(int32 quest_id){
	prereq_quests.push_back(quest_id);
}

void Quest::AddPrereqFaction(int32 faction_id, sint32 min, sint32 max){
	QuestFactionPrereq faction;
	faction.faction_id = faction_id;
	faction.min = min;
	faction.max = max;
	prereq_factions.push_back(faction);
}

void Quest::AddPrereqItem(Item* item){
	prereq_items.push_back(item);
}

void Quest::AddRewardItemVec(vector<Item*>* items, Item* item, bool combine_items) {
	if(!items || !item)
		return;
	
	bool stacked = false;

	if(combine_items) {
		for(int32 i=0;i<items->size();i++) {
			Item* cur_item = (Item*)items->at(i);
			if(cur_item->stack_count > 1) {
				if(cur_item->details.item_id == item->details.item_id && cur_item->details.count+1 < cur_item->stack_count) {
					if(!cur_item->details.count) // sometimes the count is 0, so we want it to be 2 now
						cur_item->details.count = 2;
					else
						cur_item->details.count += 1;
					stacked = true;
					break;
				}
			}
		}
	}
	
	if(!stacked) {
		items->push_back(item);
	}
}

void Quest::AddSelectableRewardItem(Item* item){
	AddRewardItemVec(&selectable_reward_items, item, false);
}

void Quest::AddRewardItem(Item* item){
	AddRewardItemVec(&reward_items, item);
}

void Quest::AddTmpRewardItem(Item* item){
	AddRewardItemVec(&tmp_reward_items, item);
}

void Quest::GetTmpRewardItemsByID(std::vector<Item*>* items) {
	if(!items)
		return;
	for(int32 i=0;i<tmp_reward_items.size();i++)
		items->push_back(tmp_reward_items[i]);
}

void Quest::AddRewardCoins(int32 copper, int32 silver, int32 gold, int32 plat){
	reward_coins = copper + (silver*100) + (gold*10000) + ((int64)plat*1000000);
}

void Quest::AddRewardCoinsMax(int64 coins){
	reward_coins_max = coins;
}

void Quest::AddRewardFaction(int32 faction_id, sint32 amount){
	reward_factions[faction_id] = amount;
}

void Quest::SetRewardStatus(int32 amount){
	reward_status = amount;
}

void Quest::SetRewardComment(string comment){
	reward_comment = comment;
}

void Quest::SetRewardXP(int32 xp){
	reward_exp = xp;
}

vector<int8>* Quest::GetPrereqClasses(){
	return &prereq_classes;
}

vector<int8>* Quest::GetPrereqTradeskillClasses(){
	return &prereq_tradeskillclass;
}

vector<QuestFactionPrereq>* Quest::GetPrereqFactions(){
	return &prereq_factions;
}

vector<int8>* Quest::GetPrereqRaces(){
	return &prereq_races;
}

vector<int16>* Quest::GetPrereqModelTypes(){
	return &prereq_model_types;
}

int8 Quest::GetPrereqLevel(){
	return prereq_level;
}

int8 Quest::GetPrereqTSLevel() {
	return prereq_tslevel;
}

vector<int32>* Quest::GetPrereqQuests(){
	return &prereq_quests;
}

vector<Item*>* Quest::GetPrereqItems(){
	return &prereq_items;
}

vector<Item*>* Quest::GetRewardItems(){
	return &reward_items;
}

vector<Item*>* Quest::GetTmpRewardItems(){
	return &tmp_reward_items;
}

vector<Item*>* Quest::GetSelectableRewardItems(){
	return &selectable_reward_items;
}

map<int32, sint32>*	Quest::GetRewardFactions() {
	return &reward_factions;
}

void Quest::SetTaskGroupDescription(int32 step, string desc, bool display_bullets){
	MQuestSteps.lock();
	if(step <= task_group_num && task_group_order.count(step) > 0){
		string old_desc = task_group_order[step];
		if(display_bullets)
			task_group[desc] = task_group[old_desc];
		else{
			if(task_group[old_desc].size() > 0){
				task_group[desc].push_back(task_group[old_desc].at(0));
				task_group[desc].at(0)->ResetTaskGroup();
				task_group[desc].at(0)->SetDescription(desc);
			}
		}
		task_group.erase(old_desc);
		task_group_order[step] = desc;
	}
	MQuestSteps.unlock();
}

void Quest::SetStepDescription(int32 step, string desc){
	MQuestSteps.lock();
	for(int32 i=0;i<quest_steps.size();i++){
		if(quest_steps[i]->GetStepID() == step){
			quest_steps[i]->SetDescription(desc);
			break;
		}
	}
	MQuestSteps.unlock();
}

void Quest::SetDescription(string desc){
	description = desc;
}

void Quest::SetCompletedDescription(string desc){
	completed_description = desc;
}

const char* Quest::GetCompletedDescription(){
	return completed_description.c_str();
}

void Quest::AddCompleteAction(int32 step, string action){
	MCompletedActions.lock();
	complete_actions[step] = action;
	MCompletedActions.unlock();
}

void Quest::AddProgressAction(int32 step, string action){
	MProgressActions.lock();
	progress_actions[step] = action;
	MProgressActions.unlock();
}

void Quest::AddFailedAction(int32 step, string action) {
	MFailedActions.writelock(__FUNCTION__, __LINE__);
	failed_actions[step] = action;
	MFailedActions.releasewritelock(__FUNCTION__, __LINE__);
}

void Quest::SetCompleteAction(string action){
	complete_action = action;
}

const char* Quest::GetCompleteAction(){
	if(complete_action.length() > 0)
		return complete_action.c_str();
	return 0;
}

const char* Quest::GetCompleteAction(int32 step){
	const char* ret = 0;
	MCompletedActions.lock();
	if(complete_actions.count(step) > 0)
		ret = complete_actions[step].c_str();
	MCompletedActions.unlock();
	return ret;
}

void Quest::SetQuestReturnNPC(int32 id){
	return_id = id;
}

int32 Quest::GetQuestReturnNPC(){
	return return_id;
}

void  Quest::SetQuestGiver(int32 id){
	quest_giver = id;
	if(return_id == 0)
		return_id = id;
}

bool Quest::GetCompleted(){
	bool ret = true;
	for(int32 i=0;i<quest_steps.size();i++){
		if(quest_steps[i]->Complete() == false){
			ret = false;
			break;
		}
	}
	return ret;
}

bool Quest::CheckCategoryYellow(){
	bool ret = false;
	string category = GetType();

	//Check for these category names, return true to set category as yellow in the quest journal
	if (category == "Signature" || category == "Heritage" || category == "Hallmark" || category == "Deity" || category == "Miscellaneaous" || category == "Language" || category == "Lore and Legend" || category == "World Event" || category == "Tradeskill")
		ret = true;
	return ret;
}

Player* Quest::GetPlayer(){
	return player;
}

void Quest::SetPlayer(Player* in_player){
	player = in_player;
}

void Quest::SetGeneratedCoin(int64 coin){
	generated_coin = coin;
}

int32 Quest::GetQuestGiver(){
	return quest_giver;
}

int64 Quest::GetCoinsReward(){
	return reward_coins;
}

int64 Quest::GetCoinsRewardMax(){
	return reward_coins_max;
}

int64 Quest::GetGeneratedCoin(){
	return generated_coin;
}

int32 Quest::GetExpReward(){
	return reward_exp;
}

void Quest::GiveQuestReward(Player* player){
	if(reward_coins > 0)
		player->AddCoins(reward_coins);

	if(!GetQuestTemporaryState())
		reward_items.clear();
}

bool Quest::GetDeleted(){
	return deleted;
}

void Quest::SetDeleted(bool val){
	deleted = val;
}

bool Quest::GetUpdateRequired(){
	return update_needed;
}

void Quest::SetUpdateRequired(bool val){
	update_needed = val;
}

void Quest::SetTurnedIn(bool val){
	turned_in = val;
	visible = 0;
}

bool Quest::GetTurnedIn(){
	return turned_in;
}

void Quest::SetName(string in_name) {
	name = in_name;
}

void Quest::SetType(string in_type) {
	type = in_type;
}

void Quest::SetLevel(int8 in_level) {
	level = in_level;
}

void Quest::SetCompletedFlag(bool val) { 
	completed_flag = val; 
	SetUpdateRequired(true);
	if(player && player->GetClient())
		player->GetClient()->SendQuestJournalUpdate(this, true);
}

void Quest::SetStepTimer(int32 duration) {
		m_timestamp = duration == 0 ? 0 : Timer::GetUnixTimeStamp() + duration;
}

void Quest::StepFailed(int32 step) {
	if(lua_interface && failed_actions.count(step) > 0 && failed_actions[step].length() > 0)
		lua_interface->CallQuestFunction(this, failed_actions[step].c_str(), player);
}

MasterQuestList::MasterQuestList(){
	MQuests.SetName("MasterQuestList::MQuests");
}

void MasterQuestList::Reload(){
	MQuests.lock();
	quests.clear(); //deletes taken care of in LuaInterface
	if(lua_interface)
		lua_interface->DestroyQuests(true);
	MQuests.unlock();
}

void MasterQuestList::LockQuests(){
	MQuests.lock();
}

void MasterQuestList::UnlockQuests(){
	MQuests.unlock();
}

void Quest::SetQuestTemporaryState(bool tempState, std::string customDescription)
{
	if(!tempState)
	{
		tmp_reward_coins = 0;
		tmp_reward_status = 0;
		
		for(int32 i=0;i<tmp_reward_items.size();i++)
			safe_delete(tmp_reward_items[i]);

		tmp_reward_items.clear();
	}

	quest_state_temporary = tempState;
	quest_temporary_description = customDescription;
}

bool Quest::CanShareQuestCriteria(Client* quest_sharer, bool display_client_msg) {
	Quest* clientQuest = quest_sharer->GetPlayer()->GetQuest(GetQuestID()); // gets active quest if available
	if(((GetQuestShareableFlag() & QUEST_SHAREABLE_COMPLETED) == 0) && quest_sharer->GetPlayer()->HasQuestBeenCompleted(GetQuestID())) {
		if(display_client_msg)
			quest_sharer->SimpleMessage(CHANNEL_COLOR_RED, "You cannot share this quest after it is already completed.");
		return false;
	}
	else if((GetQuestShareableFlag() == QUEST_SHAREABLE_COMPLETED) && !quest_sharer->GetPlayer()->HasQuestBeenCompleted(GetQuestID())) {
		if(display_client_msg)
			quest_sharer->SimpleMessage(CHANNEL_COLOR_RED, "You cannot share this quest until it is completed.");
		return false;
	}
	else if(((GetQuestShareableFlag() & QUEST_SHAREABLE_DURING) == 0) && clientQuest && clientQuest->GetQuestStep() > 1) {
		if(display_client_msg)
			quest_sharer->SimpleMessage(CHANNEL_COLOR_RED, "You cannot share this quest after already completing a step.");
		return false;
	}
	else if(((GetQuestShareableFlag() & QUEST_SHAREABLE_ACTIVE) == 0) && clientQuest) {
		if(display_client_msg)
			quest_sharer->SimpleMessage(CHANNEL_COLOR_RED, "You cannot share this quest while it is active.");
		return false;
	}
	else if(!GetQuestShareableFlag()) {
		if(display_client_msg)
			quest_sharer->SimpleMessage(CHANNEL_COLOR_RED, "You cannot share this quest.");
		return false;
	}
	else if(((GetQuestShareableFlag() & QUEST_SHAREABLE_COMPLETED) == 0) && clientQuest == nullptr) {
		if(display_client_msg)
			quest_sharer->SimpleMessage(CHANNEL_COLOR_RED, "You do not have this quest.");
		return false;
	}
	
	return true;
}