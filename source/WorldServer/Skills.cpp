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
#include "Skills.h"
#include "Spawn.h"
#include "LuaInterface.h"
#include "../common/Log.h"

extern ConfigReader configReader;
extern LuaInterface* lua_interface;

MasterSkillList::MasterSkillList(){
}

MasterSkillList::~MasterSkillList(){
	map<int32, Skill*>::iterator itr;
	for(itr = skills.begin(); itr != skills.end(); itr++){
		safe_delete(itr->second);
	}
	map<int16, EQ2Packet*>::iterator itr2;
	for(itr2 = populate_packets.begin(); itr2 != populate_packets.end(); itr2++){
		safe_delete(itr2->second);
	}
}

Skill::Skill(){
	skill_id = 0;
	current_val = 0;
	previous_val = 0;
	max_val = 0;
	skill_type = 0;
	display = 0;
	save_needed = false;
	active_skill = true;
}

Skill::Skill(Skill* skill){
	skill_id = skill->skill_id;
	current_val = skill->current_val;
	previous_val = skill->current_val;
	max_val = skill->max_val;
	skill_type = skill->skill_type;
	display = skill->display;
	short_name = skill->short_name;
	name = skill->name;
	description = skill->description;
	save_needed = false;
	active_skill = true;
}

map<int32, Skill*>* MasterSkillList::GetAllSkills(){
	return &skills;
}

Skill* MasterSkillList::GetSkill(int32 skill_id){
	if(skills.count(skill_id) > 0)
		return skills[skill_id];
	else
		return 0;
}

Skill* MasterSkillList::GetSkillByName(const char* skill_name) {
	Skill* skill = 0;
	map<int32, Skill*>::iterator itr;
	for (itr = skills.begin(); itr != skills.end(); itr++) {
		Skill* current_skill = itr->second;
		if (::ToLower(string(current_skill->name.data.c_str())) == ::ToLower(string(skill_name))) {
			skill = current_skill;
			break;
		}
	}
	return skill;
}

int16 MasterSkillList::GetSkillCount(){
	return skills.size();
}

void MasterSkillList::AddSkill(Skill* skill){
	if(skill)
		skills[skill->skill_id] = skill;
}

EQ2Packet* MasterSkillList::GetPopulateSkillsPacket(int16 version){
	EQ2Packet* ret = 0;
	int16 packet_version = configReader.GetStructVersion("WS_SkillMap", version);
	if(populate_packets.count(packet_version) > 0)
		ret = populate_packets[packet_version];
	else{
		PacketStruct* packet = configReader.getStruct("WS_SkillMap", packet_version);
		int32 count = skills.size();
		Skill* skill = 0;
		int32 i = 0;
		packet->setArrayLengthByName("skill_count", count);
		map<int32, Skill*>::iterator itr;
		for(itr = skills.begin(); itr != skills.end(); itr++, i++){
			skill = itr->second;
			packet->setArrayDataByName("skill_id", skill->skill_id, i);
			packet->setArrayDataByName("short_name", &skill->short_name, i);
			packet->setArrayDataByName("name", &skill->name, i);
			packet->setArrayDataByName("description", &skill->description, i);
		}
		populate_packets[packet_version] = packet->serialize();
		safe_delete(packet);
		ret = populate_packets[packet_version];
	}
	if(ret)
		return ret->Copy(); //need to return a copy as the packet is deleted after the client confirms it and we want to keep the packet to prevent constant generation of the same data
	else
		return ret;
}

PlayerSkillList::PlayerSkillList(){
	xor_packet = 0;
	orig_packet = 0;
	packet_count = 0;
	has_updates = false;
	MSkillUpdates.SetName("PlayerSkillList::MSkillUpdates");
}

PlayerSkillList::~PlayerSkillList(){
	map<int32, Skill*>::iterator itr;
	for(itr = skills.begin(); itr != skills.end(); itr++){
		safe_delete(itr->second);
	}
	MutexMap<int32, SkillBonus*>::iterator sb_itr = skill_bonus_list.begin();
	while (sb_itr.Next())
		RemoveSkillBonus(sb_itr.first);
	safe_delete_array(xor_packet);
	safe_delete_array(orig_packet);
}

void PlayerSkillList::AddSkill(Skill* new_skill){
	std::unique_lock lock(MPlayerSkills);
	Skill* tmpSkill = nullptr;
	if(skills.count(new_skill->skill_id)) {
		tmpSkill = skills[new_skill->skill_id];
	}
	skills[new_skill->skill_id] = new_skill;
	if(tmpSkill) {
		lua_interface->SetLuaUserDataStale(tmpSkill);
		safe_delete(tmpSkill);
	}
	name_skill_map.clear();
}

void PlayerSkillList::RemoveSkill(Skill* skill) {
	std::unique_lock lock(MPlayerSkills);
	if (skill) {
		lua_interface->SetLuaUserDataStale(skill);
		skill->active_skill = false;
		name_skill_map.clear();
	}
}

map<int32, Skill*>* PlayerSkillList::GetAllSkills(){
	return &skills;
}

void PlayerSkillList::SetSkillValuesByType(int8 type, int16 value, bool send_update) {
	map<int32, Skill*>::iterator itr;
	for (itr = skills.begin(); itr != skills.end(); itr++) {
		if (itr->second && itr->second->skill_type == type)
			SetSkill(itr->second, value, send_update);
	}
}

void PlayerSkillList::SetSkillCapsByType(int8 type, int16 value){
	map<int32, Skill*>::iterator itr;
	for(itr = skills.begin(); itr != skills.end(); itr++){
		if(itr->second && itr->second->skill_type == type)
			SetSkillCap(itr->second, value);
	}
}

void PlayerSkillList::IncreaseSkillCapsByType(int8 type, int16 value){
	map<int32, Skill*>::iterator itr;
	for(itr = skills.begin(); itr != skills.end(); itr++){
		if(itr->second && itr->second->skill_type == type)
			IncreaseSkillCap(itr->second, value);
	}
}

void PlayerSkillList::IncreaseAllSkillCaps(int16 value){
	map<int32, Skill*>::iterator itr;
	for(itr = skills.begin(); itr != skills.end(); itr++){
		IncreaseSkillCap(itr->second, value);
	}
}

bool PlayerSkillList::HasSkill(int32 skill_id){
	std::shared_lock lock(MPlayerSkills);
	return (skills.count(skill_id) > 0 && skills[skill_id]->active_skill);
}

Skill* PlayerSkillList::GetSkill(int32 skill_id){
	std::shared_lock lock(MPlayerSkills);
	if(skills.count(skill_id) > 0 && skills[skill_id]->active_skill)
		return skills[skill_id];
	else
		return 0;
}

void PlayerSkillList::IncreaseSkill(Skill* skill, int16 amount){
	if(skill){
		skill->previous_val = skill->current_val;
		skill->current_val += amount;
		if(skill->current_val > skill->max_val)
			skill->max_val = skill->current_val;
		AddSkillUpdateNeeded(skill);
		skill->save_needed = true;
	}
}

void PlayerSkillList::IncreaseSkill(int32 skill_id, int16 amount){
	IncreaseSkill(GetSkill(skill_id), amount);
}

void PlayerSkillList::DecreaseSkill(Skill* skill, int16 amount){
	if(skill){
		skill->previous_val = skill->current_val;
		if((skill->current_val - amount) < 0)
			skill->current_val = 0;
		else
			skill->current_val -= amount;
		skill->save_needed = true;
		AddSkillUpdateNeeded(skill);
	}
}

void PlayerSkillList::DecreaseSkill(int32 skill_id, int16 amount){
	DecreaseSkill(GetSkill(skill_id), amount);
}

void PlayerSkillList::SetSkill(Skill* skill, int16 value, bool send_update){
	if(skill){
		skill->previous_val = skill->current_val;
		skill->current_val = value;
		if(skill->current_val > skill->max_val)
			skill->max_val = skill->current_val;
		skill->save_needed = true;
		if(send_update)
			AddSkillUpdateNeeded(skill);
	}
}

void PlayerSkillList::SetSkill(int32 skill_id, int16 value, bool send_update){
	SetSkill(GetSkill(skill_id), value, send_update);
}

void PlayerSkillList::IncreaseSkillCap(Skill* skill, int16 amount){
	if(skill){
		skill->max_val += amount;
		skill->save_needed = true;
	}
}

void PlayerSkillList::IncreaseSkillCap(int32 skill_id, int16 amount){
	IncreaseSkillCap(GetSkill(skill_id), amount);
}

void PlayerSkillList::DecreaseSkillCap(Skill* skill, int16 amount){
	if(skill){
		if((skill->max_val - amount) < 0)
			skill->max_val = 0;
		else
			skill->max_val -= amount;		
		if(skill->current_val > skill->max_val){
			skill->previous_val = skill->current_val;
			skill->current_val = skill->max_val;
		}
		AddSkillUpdateNeeded(skill);
		skill->save_needed = true;
	}
}

void PlayerSkillList::DecreaseSkillCap(int32 skill_id, int16 amount){
	DecreaseSkillCap(GetSkill(skill_id), amount);
}

void PlayerSkillList::SetSkillCap(Skill* skill, int16 value){
	if(skill){
		skill->max_val = value;
		if(skill->current_val > skill->max_val){
			skill->previous_val = skill->current_val;
			skill->current_val = skill->max_val;
		}
		AddSkillUpdateNeeded(skill);
		skill->save_needed = true;
	}
}

void PlayerSkillList::SetSkillCap(int32 skill_id, int16 value){
	SetSkillCap(GetSkill(skill_id), value);
}

int16 PlayerSkillList::CalculateSkillValue(int32 skill_id, int16 current_val){
	if (current_val > 5) {
		int16 new_val = current_val;
		MutexMap<int32, SkillBonus*>::iterator itr = skill_bonus_list.begin();
		while (itr.Next()) {
			SkillBonus* sb = itr.second;
			map<int32, SkillBonusValue*>::iterator sbv_itr;
			for (sbv_itr = sb->skills.begin(); sbv_itr != sb->skills.end(); sbv_itr++) {
				SkillBonusValue* sbv = sbv_itr->second;
				if (sbv->skill_id == skill_id)
					new_val += (int16)sbv->value;
			}
		}
		return new_val;
	}

	return current_val;
}

int16 PlayerSkillList::CalculateSkillMaxValue(int32 skill_id, int16 max_val) {
	int16 new_val = max_val;
	MutexMap<int32, SkillBonus*>::iterator itr = skill_bonus_list.begin();
	while (itr.Next()) {
		SkillBonus* sb = itr->second;
		map<int32, SkillBonusValue*>::iterator sbv_itr;
		for (sbv_itr = sb->skills.begin(); sbv_itr != sb->skills.end(); sbv_itr++) {
			SkillBonusValue* sbv = sbv_itr->second;
			if (sbv->skill_id == skill_id)
				new_val += (int16)sbv->value;
		}
	}
	return new_val;
}

EQ2Packet* PlayerSkillList::GetSkillPacket(int16 version){
	std::unique_lock lock(MPlayerSkills);
	PacketStruct* packet = configReader.getStruct("WS_UpdateSkillBook", version);
	if(packet){
			int16 skill_count = 0;
			map<int32, Skill*>::iterator itr;
			for (itr = skills.begin(); itr != skills.end(); itr++) {
				if (itr->second && itr->second->active_skill)
					skill_count++;
			}
			int16 size = 0;
			if (version > 561) {
				size = 21 * skill_count + 8;
			}
			else if (version < 373) {
				size = 12 * skill_count + 6;
			}
			else if (version <= 373) {
				size = 15 * skill_count + 6;
			}
			else if (version <= 561) {
				size = 21 * skill_count + 7;
			}
			
			if (skill_count > packet_count) {
			uchar* tmp = 0;
			if (orig_packet) {
				tmp = new uchar[size];
				memset(tmp, 0, size);
				memcpy(tmp, orig_packet, orig_packet_size);
				safe_delete_array(orig_packet);
				safe_delete_array(xor_packet);
				orig_packet = tmp;
			}
			else {
				orig_packet = new uchar[size];
				memset(orig_packet, 0, size);
			}
			xor_packet = new uchar[size];
			memset(xor_packet, 0, size);
		}
		packet_count = skill_count;
		orig_packet_size = size;
		packet->setArrayLengthByName("skill_count", skill_count);
		Skill* skill = 0;
		int32 i=0;
		for(itr = skills.begin(); itr != skills.end(); itr++){
			skill = itr->second;
			if(skill && skill->active_skill){
				int16 skill_max_with_bonuses = CalculateSkillMaxValue(skill->skill_id, skill->max_val);
				int16 skill_with_bonuses = int(CalculateSkillValue(skill->skill_id, skill->current_val));
				packet->setArrayDataByName("skill_id", skill->skill_id, i);
				if (version <= 561 && skill->skill_type >= SKILL_TYPE_GENERAL) { //covert it to DOF types
					packet->setArrayDataByName("type", skill->skill_type-2, i);					
				}
				else if(version >= 60085 && skill->skill_type >= 12) {
					packet->setArrayDataByName("type", skill->skill_type-1, i);
				}
				else {
					packet->setArrayDataByName("type", skill->skill_type, i);
				}
				
				int16 current_val = skill->current_val;
				
				if(skill->skill_type == SKILL_TYPE_LANGUAGE) { // 13 is language in the DB?? 14 is the skill type though
					packet->setArrayDataByName("language_unknown", skill->skill_id, i);
					packet->setArrayDataByName("display_maxval", 1, i);
					packet->setArrayDataByName("max_val", 1, i);
				}
				else {
					packet->setArrayDataByName("max_val", skill->max_val, i);
					packet->setArrayDataByName("display_minval", skill->display, i);
					packet->setArrayDataByName("display_maxval", skill->display, i);
					packet->setArrayDataByName("skill_delta", 0, i);// skill_with_bonuses- skill->current_val
					packet->setArrayDataByName("skill_delta2", skill_max_with_bonuses - skill->max_val, i);// skill_max_with_bonuses - skill->max_val, i);
				}
				
				packet->setArrayDataByName("current_val", current_val, i);
				packet->setArrayDataByName("base_val", current_val, i);
				i++;
			}
		}
		int8 offset = 1;
		if (version <= 373)
			offset = 0;
		EQ2Packet* ret = packet->serializeCountPacket(version, offset, orig_packet, xor_packet);
		//packet->PrintPacket();
		//DumpPacket(orig_packet, orig_packet_size);
		//DumpPacket(ret);
		safe_delete(packet);
		return ret;
	}
	return 0;
}

bool PlayerSkillList::CheckSkillIncrease(Skill* skill){
	if(!skill || skill->current_val >= skill->max_val)
		return false;
	// Assuming that skills will be used more at higher levels, increase chances are:
	// skill val of 1 ~ 20% chance, value of 100 ~ 10%, value of 400 ~ 4%
	int8 percent = (int8)(((float)((float)100/(float)(50 + skill->current_val)))*10);
	if(rand()%100 < percent){ // skill increase
		IncreaseSkill(skill, 1);
		return true;
	}
	else
		return false;
}

Skill* PlayerSkillList::GetSkillByName(const char* name){
	std::shared_lock lock(MPlayerSkills);
	if(name_skill_map.size() == 0){
		map<int32, Skill*>::iterator itr;
		Skill* skill = 0;
		for(itr = skills.begin(); itr != skills.end(); itr++){
			skill = itr->second;
			name_skill_map[skill->name.data] = skill;
		}
	}
	if(name_skill_map.count(name) > 0)
		return name_skill_map[name];
	else
		return 0;
}

vector<Skill*>* PlayerSkillList::GetSaveNeededSkills(){
	std::shared_lock lock(MPlayerSkills);
	vector<Skill*>* ret = new vector<Skill*>;
	map<int32, Skill*>::iterator itr;
	for(itr = skills.begin(); itr != skills.end(); itr++){
		if(itr->second->save_needed){
			ret->push_back(itr->second);
			itr->second->save_needed = false;
		}
	}
	return ret;
}

void PlayerSkillList::AddSkillUpdateNeeded(Skill* skill){
	MSkillUpdates.writelock(__FUNCTION__, __LINE__);
	skill_updates.push_back(skill);
	has_updates = true;
	MSkillUpdates.releasewritelock(__FUNCTION__, __LINE__);
}

vector<Skill*>* PlayerSkillList::GetSkillUpdates(){
	vector<Skill*>* ret = 0;
	vector<Skill*>::iterator itr;
	MSkillUpdates.writelock(__FUNCTION__, __LINE__);
	if(skill_updates.size() > 0){
		ret = new vector<Skill*>();
		ret->insert(ret->begin(), skill_updates.begin(), skill_updates.end());
		skill_updates.clear();		
	}
	has_updates = false;
	MSkillUpdates.releasewritelock(__FUNCTION__, __LINE__);
	return ret;
}

bool PlayerSkillList::HasSkillUpdates(){
	return has_updates;
}

void PlayerSkillList::AddSkillBonus(int32 spell_id, int32 skill_id, float value) {
	if (value != 0) {
		SkillBonus* sb;
		if (skill_bonus_list.count(spell_id) == 0) {
			sb = new SkillBonus;
			sb->spell_id = spell_id;
			skill_bonus_list.Put(spell_id, sb);
		}
		else
			sb = skill_bonus_list.Get(spell_id);
		
		if (sb->skills[skill_id] == nullptr) {
			SkillBonusValue* sbv = new SkillBonusValue;
			sbv->skill_id = skill_id;
			sbv->value = value;
			sb->skills[skill_id] = sbv;
		}
	}
}

void PlayerSkillList::ResetPackets() {
	std::unique_lock lock(MPlayerSkills);
	safe_delete_array(orig_packet);
	safe_delete_array(xor_packet);
	orig_packet_size = 0;
	orig_packet = 0;
	xor_packet = 0;
	packet_count = 0;
}

SkillBonus* PlayerSkillList::GetSkillBonus(int32 spell_id) {
	SkillBonus *ret = 0;
	
	if (skill_bonus_list.count(spell_id) > 0)
		ret = skill_bonus_list.Get(spell_id);

	return ret;
}

void PlayerSkillList::RemoveSkillBonus(int32 spell_id) {
	if (skill_bonus_list.count(spell_id) > 0) {
		SkillBonus* sb = skill_bonus_list.Get(spell_id);
		skill_bonus_list.erase(spell_id);
		map<int32, SkillBonusValue*>::iterator itr;
		for (itr = sb->skills.begin(); itr != sb->skills.end(); itr++)
			safe_delete(itr->second);
		safe_delete(sb);
	}
}

int Skill::CheckDisarmSkill(int16 targetLevel, int8 chest_difficulty)
{
	if (chest_difficulty < 2) // no triggers on this chest type
		return 1;

	if (targetLevel < 1)
		targetLevel = 1;

	int	chest_diff_result = targetLevel * chest_difficulty;
	float base_difficulty = 15.0f;
	float fail_threshold = 10.0f;


	float chance = ((100.0f - base_difficulty) * ((float)current_val / (float)chest_diff_result));

	if (chance > (100.0f - base_difficulty))
	{
		chance = 100.0f - base_difficulty;
	}

	float d100 = (float)MakeRandomFloat(0, 100);

	if (d100 <= chance)
		return 1;
	else
	{
		if (d100 > (chance + fail_threshold))
			return -1;
	}

	return 0;
}