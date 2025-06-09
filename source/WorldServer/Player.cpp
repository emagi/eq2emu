/*  
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2005 - 2025  EQ2EMulator Development Team (http://www.eq2emu.com formerly http://www.eq2emulator.net)

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

#include "Player.h"
#include "../common/MiscFunctions.h"
#include "World.h"
#include "WorldDatabase.h"
#include <math.h>
#include "classes.h"
#include "LuaInterface.h"
#include "../common/Log.h"
#include "Rules/Rules.h"
#include "Titles.h"
#include "Languages.h"
#include "SpellProcess.h"
#include <algorithm>
#include <regex>
#include "ClientPacketFunctions.h"

extern Classes classes;
extern WorldDatabase database;
extern World world;
extern ConfigReader configReader;
extern MasterSkillList master_skill_list;
extern MasterSpellList master_spell_list;
extern MasterQuestList master_quest_list;
extern Variables variables;
extern LuaInterface* lua_interface;
extern MasterItemList master_item_list;
extern RuleManager rule_manager;
extern MasterTitlesList master_titles_list;
extern MasterLanguagesList master_languages_list;
std::map<int8, int32> Player::m_levelXPReq;

Player::Player(){
	tutorial_step = 0;
	char_id = 0;
	group = 0;
	appearance.pos.grid_id = 0;
	spawn_index = 1;
	info = 0;
	movement_packet = 0;
	last_movement_activity = 0;
	//speed = 0;
	packet_num = 0;
	range_attack = false;
	old_movement_packet = 0;
	charsheet_changed = false;
	quickbar_updated = false;
	custNPC = false;
	spawn_tmp_vis_xor_packet = 0;
	spawn_tmp_pos_xor_packet = 0;
	spawn_tmp_info_xor_packet = 0;
	pending_collection_reward = 0;
	pos_packet_speed = 0;

	appearance.display_name = 1;
	appearance.show_command_icon = 1;
	appearance.player_flag = 1;
	appearance.targetable = 1;
	appearance.show_level = 1;
	spell_count = 0;
	spell_orig_packet = 0;
	spell_xor_packet = 0;
	raid_orig_packet = nullptr;
	raid_xor_packet = nullptr;
	resurrecting = false;
	spawn_id = 1;
	spawn_type = 4;
	player_spawn_id_map[1] = this;
	player_spawn_reverse_id_map[this] = 1;
	MPlayerQuests.SetName("Player::MPlayerQuests");
	test_time = 0;
	returning_from_ld = false;
	away_message = "Sorry, I am A.F.K. (Away From Keyboard)";
	AddSecondaryEntityCommand("Inspect", 10000, "inspect_player", "", 0, 0);
	AddSecondaryEntityCommand("Who", 10000, "who", "", 0, 0);
	//  commented out commands a player canNOT use on themselves... move these to Client::HandleVerbRequest()?
	//AddSecondaryEntityCommand("Assist", 10, "assist", "", 0, 0);
	//AddSecondaryEntityCommand("Duel", 10, "duel", "", 0, 0);
	//AddSecondaryEntityCommand("Duel Bet", 10, "duelbet", "", 0, 0);
	//AddSecondaryEntityCommand("Trade", 10, "trade", "", 0, 0);
	is_tracking = false;
	guild = 0;
	following = false;
	combat_target = 0;
	//InitXPTable();
	pending_deletion = false;
	spawn_vis_struct = 0;
	spawn_pos_struct = 0;
	spawn_info_struct = 0;
	spawn_header_struct = 0;
	spawn_footer_struct = 0;
	widget_footer_struct = 0;
	sign_footer_struct = 0;
	pos_xor_size = 0;
	info_xor_size = 0;
	vis_xor_size = 0;
	pos_mutex.SetName("Player::pos_mutex");
	vis_mutex.SetName("Player::vis_mutex");
	info_mutex.SetName("Player::info_mutex");
	index_mutex.SetName("Player::index_mutex");
	spawn_mutex.SetName("Player::spawn_mutex");
	m_playerSpawnQuestsRequired.SetName("Player::player_spawn_quests_required");
	m_playerSpawnHistoryRequired.SetName("Player::player_spawn_history_required");
	gm_vision = false;
	SetSaveSpellEffects(true);
	reset_mentorship = false;
	all_spells_locked = false;
	current_language_id = 0;
	active_reward = false;
	
	SortedTraitList = new map <int8, map <int8, vector<TraitData*> > >;
	ClassTraining = new map <int8, vector<TraitData*> >;
	RaceTraits = new map <int8, vector<TraitData*> >;
	InnateRaceTraits = new map <int8, vector<TraitData*> >;
	FocusEffects = new map <int8, vector<TraitData*> >;
	need_trait_update = true;
	active_food_unique_id = 0;
	active_drink_unique_id = 0;
	raidsheet_changed = false;
	hassent_raid = false;
}
Player::~Player(){
	SetSaveSpellEffects(true);
	for(int32 i=0;i<spells.size();i++){
		safe_delete(spells[i]);
	}
	for(int32 i=0;i<quickbar_items.size();i++){
		safe_delete(quickbar_items[i]);
	}
	map<int32, vector<int32>*>::iterator itr;
	for (itr = player_spawn_quests_required.begin(); itr != player_spawn_quests_required.end(); itr++){
		safe_delete(itr->second);
	}
	player_spawn_quests_required.clear();
	
	for (itr = player_spawn_history_required.begin(); itr != player_spawn_history_required.end(); itr++){
		safe_delete(itr->second);
	}
	player_spawn_history_required.clear();

	map<int8, map<int8, vector<HistoryData*> > >::iterator itr1;
	map<int8, vector<HistoryData*> >::iterator itr2;
	vector<HistoryData*>::iterator itr3;
	// Type
	for (itr1 = m_characterHistory.begin(); itr1 != m_characterHistory.end(); itr1++) {
		// Sub type
		for (itr2 = itr1->second.begin(); itr2 != itr1->second.end(); itr2++) {
			// vector of data
			for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++) {
				safe_delete(*itr3);
			}
		}
	}
	m_characterHistory.clear();

	mLUAHistory.writelock();
	map<int32, LUAHistory*>::iterator itr4;
	for (itr4 = m_charLuaHistory.begin(); itr4 != m_charLuaHistory.end(); itr4++) {
		safe_delete(itr4->second);
	}
	m_charLuaHistory.clear();
	mLUAHistory.releasewritelock();

	safe_delete_array(movement_packet);
	safe_delete_array(old_movement_packet);
	safe_delete_array(spawn_tmp_info_xor_packet);
	safe_delete_array(spawn_tmp_vis_xor_packet);
	safe_delete_array(spawn_tmp_pos_xor_packet);
	safe_delete_array(spell_xor_packet);
	safe_delete_array(spell_orig_packet);
	safe_delete_array(raid_orig_packet);
	safe_delete_array(raid_xor_packet);
	DestroyQuests();
	WritePlayerStatistics();
	RemovePlayerStatistics();
	DeleteMail();
	world.RemoveLottoPlayer(GetCharacterID());
	safe_delete(info);
	index_mutex.writelock(__FUNCTION__, __LINE__);
	player_spawn_reverse_id_map.clear();
	player_spawn_id_map.clear();
	index_mutex.releasewritelock(__FUNCTION__, __LINE__);

	info_mutex.writelock(__FUNCTION__, __LINE__);
	spawn_info_packet_list.clear();
	info_mutex.releasewritelock(__FUNCTION__, __LINE__);
	vis_mutex.writelock(__FUNCTION__, __LINE__);
	spawn_vis_packet_list.clear();
	vis_mutex.releasewritelock(__FUNCTION__, __LINE__);
	pos_mutex.writelock(__FUNCTION__, __LINE__);
	spawn_pos_packet_list.clear();
	pos_mutex.releasewritelock(__FUNCTION__, __LINE__);

	safe_delete(spawn_header_struct);
	safe_delete(spawn_footer_struct);
	safe_delete(sign_footer_struct);
	safe_delete(widget_footer_struct);
	safe_delete(spawn_info_struct);
	safe_delete(spawn_vis_struct);
	safe_delete(spawn_pos_struct);
	ClearPendingSelectableItemRewards(0, true);
	ClearPendingItemRewards();
	ClearEverything();
	
	safe_delete(SortedTraitList);
	safe_delete(ClassTraining);
	safe_delete(RaceTraits);
	safe_delete(InnateRaceTraits);
	safe_delete(FocusEffects);
	// leak fix on Language* pointer from Player::AddLanguage
	player_languages_list.Clear();
}

EQ2Packet* Player::serialize(Player* player, int16 version){
	return spawn_serialize(player, version);
}

EQ2Packet* Player::Move(float x, float y, float z, int16 version, float heading){
	PacketStruct* packet = configReader.getStruct("WS_MoveClient", version);
	if(packet){
		packet->setDataByName("x", x);
		packet->setDataByName("y", y);
		packet->setDataByName("z", z);
		packet->setDataByName("unknown", 1);	// 1 seems to force the client to re-render the zone at the new location
		packet->setDataByName("location", 0xFFFFFFFF); //added in 869
		if (heading != -1.0f)
			packet->setDataByName("heading", heading);
		EQ2Packet* outapp = packet->serialize();
		safe_delete(packet);
		return outapp;
	}
	return 0;
}

void Player::DestroyQuests(){
	MPlayerQuests.writelock(__FUNCTION__, __LINE__);
	map<int32, Quest*>::iterator itr;
	for(itr = completed_quests.begin(); itr != completed_quests.end(); itr++){
		if(itr->second) {
			safe_delete(itr->second);
		}
	}
	completed_quests.clear();
	for(itr = player_quests.begin(); itr != player_quests.end(); itr++){
		if(itr->second) {
			safe_delete(itr->second);
		}
	}
	player_quests.clear();
	for(itr = pending_quests.begin(); itr != pending_quests.end(); itr++){
		if(itr->second) {
			safe_delete(itr->second);
		}
	}
	pending_quests.clear();
	MPlayerQuests.releasewritelock(__FUNCTION__, __LINE__);
}

PlayerInfo* Player::GetPlayerInfo(){
	if(info == 0)
		info = new PlayerInfo(this);
	return info;
}

void PlayerInfo::CalculateXPPercentages(){
	int32 xp_needed = info_struct->get_xp_needed();
	if(xp_needed > 0){
		double div_percent = ((double)info_struct->get_xp() / xp_needed) * 100.0;
		int16 percentage = (int16)(div_percent) * 10;
		double whole, fractional = 0.0;
		fractional = std::modf(div_percent, &whole);
		info_struct->set_xp_yellow(percentage);
		info_struct->set_xp_blue((int16)(fractional * 1000));
		
		// vitality bars probably need a revisit
		info_struct->set_xp_blue_vitality_bar(0);
		info_struct->set_xp_yellow_vitality_bar(0);
		if(player->GetXPVitality() > 0){
			float vitality_total = player->GetXPVitality()*10 + percentage;
			vitality_total -= ((int)(percentage/100)*100);
			if(vitality_total < 100){ //10%
				info_struct->set_xp_blue_vitality_bar(info_struct->get_xp_blue() + (int16)(player->GetXPVitality() *10));
			}
			else
				info_struct->set_xp_yellow_vitality_bar(info_struct->get_xp_yellow() + (int16)(player->GetXPVitality() *10));
		}
	}
}

void PlayerInfo::CalculateTSXPPercentages(){
	int32 ts_xp_needed = info_struct->get_ts_xp_needed();
	if(ts_xp_needed > 0){
		float percentage = ((double)info_struct->get_ts_xp() / ts_xp_needed) * 1000;
		info_struct->set_tradeskill_exp_yellow((int16)percentage);
		info_struct->set_tradeskill_exp_blue((int16)((percentage - info_struct->get_tradeskill_exp_yellow()) * 1000));
		/*info_struct->xp_blue_vitality_bar = 0;
		info_struct->xp_yellow_vitality_bar = 0;
		if(player->GetXPVitality() > 0){
			float vitality_total = player->GetXPVitality()*10 + percentage;
			vitality_total -= ((int)(percentage/100)*100);
			if(vitality_total < 100){ //10%
				info_struct->xp_blue_vitality_bar = info_struct->xp_blue + (int16)(player->GetXPVitality() *10);
			}
			else
				info_struct->xp_yellow_vitality_bar = info_struct->xp_yellow + (int16)(player->GetXPVitality() *10);
		}*/
	}
}

void PlayerInfo::SetHouseZone(int32 id){
	house_zone_id = id;
}

void PlayerInfo::SetBindZone(int32 id){
	bind_zone_id = id;
}

void PlayerInfo::SetBindX(float x){
	bind_x = x;
}

void PlayerInfo::SetBindY(float y){
	bind_y = y;
}

void PlayerInfo::SetBindZ(float z){
	bind_z = z;
}

void PlayerInfo::SetBindHeading(float heading){
	bind_heading = heading;
}

int32 PlayerInfo::GetHouseZoneID(){
	return house_zone_id;
}

int32 PlayerInfo::GetBindZoneID(){
	return bind_zone_id;
}

float PlayerInfo::GetBindZoneX(){
	return bind_x;
}

float PlayerInfo::GetBindZoneY(){
	return bind_y;
}

float PlayerInfo::GetBindZoneZ(){
	return bind_z;
}

float PlayerInfo::GetBindZoneHeading(){
	return bind_heading;
}

PacketStruct* PlayerInfo::serialize2(int16 version){
	PacketStruct* packet = configReader.getStruct("WS_CharacterSheet", version);
	if(packet){
		//TODO: 2021 FIX THIS CASTING
		char deity[32];
		strncpy(deity, info_struct->get_deity().c_str(), 32);
		packet->setDataByName("deity", deity);
		
		char name[40];
		strncpy(name, info_struct->get_name().c_str(), 40);
		packet->setDataByName("character_name", name);
		packet->setDataByName("race", info_struct->get_race());
		packet->setDataByName("gender", info_struct->get_gender());
		packet->setDataByName("class1", info_struct->get_class1());
		packet->setDataByName("class2", info_struct->get_class2());
		packet->setDataByName("class3", info_struct->get_class3());
		packet->setDataByName("tradeskill_class1", info_struct->get_tradeskill_class1());
		packet->setDataByName("tradeskill_class2", info_struct->get_tradeskill_class2());
		packet->setDataByName("tradeskill_class3", info_struct->get_tradeskill_class3());
		packet->setDataByName("level", info_struct->get_level());
		packet->setDataByName("effective_level", info_struct->get_effective_level() != 0 ? info_struct->get_effective_level() : info_struct->get_level());
		packet->setDataByName("tradeskill_level", info_struct->get_tradeskill_level());
		packet->setDataByName("account_age_base", info_struct->get_account_age_base());

//		for(int8 i=0;i<19;i++)
//		{
//			packet->setDataByName("account_age_bonus", info_struct->get_account_age_bonus(i));
//		}

		//
		packet->setDataByName("current_hp", player->GetHP());
		packet->setDataByName("max_hp",player-> GetTotalHP());
		packet->setDataByName("base_hp", player->GetTotalHPBase());
		float bonus_health = floor( (float)(info_struct->get_sta() * player->CalculateBonusMod()));
		packet->setDataByName("bonus_health", bonus_health);
		packet->setDataByName("stat_bonus_health", player->CalculateBonusMod());
		packet->setDataByName("current_power", player->GetPower());
		packet->setDataByName("max_power", player->GetTotalPower());
		packet->setDataByName("base_power", player->GetTotalPowerBase());
		packet->setDataByName("bonus_power", floor( (float)(player->GetPrimaryStat() * player->CalculateBonusMod())));
		packet->setDataByName("stat_bonus_power", player->CalculateBonusMod());
		packet->setDataByName("conc_used", info_struct->get_cur_concentration());
		packet->setDataByName("conc_max", info_struct->get_max_concentration());
		packet->setDataByName("attack", info_struct->get_cur_attack());
		packet->setDataByName("attack_base", info_struct->get_attack_base());
		packet->setDataByName("absorb", info_struct->get_absorb());
		packet->setDataByName("mitigation_skill1", info_struct->get_mitigation_skill1());
		packet->setDataByName("mitigation_skill2", info_struct->get_mitigation_skill2());
		packet->setDataByName("mitigation_skill3", info_struct->get_mitigation_skill3());
		CalculateXPPercentages();
		packet->setDataByName("exp_yellow", info_struct->get_xp_yellow());
		packet->setDataByName("exp_blue", info_struct->get_xp_blue());
		packet->setDataByName("tradeskill_exp_yellow", info_struct->get_tradeskill_exp_yellow());
		packet->setDataByName("tradeskill_exp_blue", info_struct->get_tradeskill_exp_blue());
		packet->setDataByName("flags", info_struct->get_flags());
		packet->setDataByName("flags2", info_struct->get_flags2());

		packet->setDataByName("avoidance_pct", (int16)info_struct->get_avoidance_display()*10.0f);//avoidance_pct 192 = 19.2% // confirmed DoV
		packet->setDataByName("avoidance_base", (int16)info_struct->get_avoidance_base()*10.0f); // confirmed DoV
		packet->setDataByName("avoidance", info_struct->get_cur_avoidance());
		packet->setDataByName("base_avoidance_pct", info_struct->get_base_avoidance_pct());// confirmed DoV
		float parry_pct = info_struct->get_parry(); // client works off of int16, but we use floats to track the actual x/100%
		packet->setDataByName("parry",(int16)(parry_pct*10.0f));// confirmed DoV

		float block_pct = info_struct->get_block()*10.0f;
		
		packet->setDataByName("block", (int16)block_pct);// confirmed DoV
		packet->setDataByName("uncontested_block", info_struct->get_uncontested_block());// confirmed DoV

		packet->setDataByName("str", info_struct->get_str());
		packet->setDataByName("sta", info_struct->get_sta());
		packet->setDataByName("agi", info_struct->get_agi());
		packet->setDataByName("wis", info_struct->get_wis());
		packet->setDataByName("int", info_struct->get_intel());
		packet->setDataByName("str_base", info_struct->get_str_base());
		packet->setDataByName("sta_base", info_struct->get_sta_base());
		packet->setDataByName("agi_base", info_struct->get_agi_base());
		packet->setDataByName("wis_base", info_struct->get_wis_base());
		packet->setDataByName("int_base", info_struct->get_intel_base());
		packet->setDataByName("mitigation_cur", info_struct->get_cur_mitigation());
		packet->setDataByName("mitigation_max", info_struct->get_max_mitigation());
		packet->setDataByName("mitigation_base", info_struct->get_mitigation_base());
		packet->setDataByName("heat", info_struct->get_heat());
		packet->setDataByName("cold", info_struct->get_cold());
		packet->setDataByName("magic", info_struct->get_magic());
		packet->setDataByName("mental", info_struct->get_mental());
		packet->setDataByName("divine", info_struct->get_divine());
		packet->setDataByName("disease", info_struct->get_disease());
		packet->setDataByName("poison", info_struct->get_poison());
		packet->setDataByName("heat_base", info_struct->get_heat_base());
		packet->setDataByName("cold_base", info_struct->get_cold_base());
		packet->setDataByName("magic_base", info_struct->get_magic_base());
		packet->setDataByName("mental_base", info_struct->get_mental_base());
		packet->setDataByName("divine_base", info_struct->get_divine_base());
		packet->setDataByName("disease_base", info_struct->get_disease_base());
		packet->setDataByName("poison_base", info_struct->get_poison_base());
		packet->setDataByName("mitigation_cur2", info_struct->get_cur_mitigation());
		packet->setDataByName("mitigation_max2", info_struct->get_max_mitigation());
		packet->setDataByName("mitigation_base2", info_struct->get_mitigation_base());
		packet->setDataByName("coins_copper", info_struct->get_coin_copper());
		packet->setDataByName("coins_silver", info_struct->get_coin_silver());
		packet->setDataByName("coins_gold", info_struct->get_coin_gold());
		packet->setDataByName("coins_plat", info_struct->get_coin_plat());
		packet->setDataByName("weight", info_struct->get_weight());
		packet->setDataByName("max_weight", info_struct->get_max_weight());
		
		if(info_struct->get_pet_id() != 0xFFFFFFFF) {
			char pet_name[32];
			strncpy(pet_name, info_struct->get_pet_name().c_str(), version <= 373 ? 16 : 32);
			packet->setDataByName("pet_name", pet_name);
		}
		else {
			packet->setDataByName("pet_name", "No Pet");
		}
		
		packet->setDataByName("pet_health_pct", info_struct->get_pet_health_pct());
		packet->setDataByName("pet_power_pct", info_struct->get_pet_power_pct());
		
		packet->setDataByName("pet_movement", info_struct->get_pet_movement());
		packet->setDataByName("pet_behavior", info_struct->get_pet_behavior());
		
		packet->setDataByName("status_points", info_struct->get_status_points());
		if(bind_zone_id > 0){
			string bind_name = database.GetZoneName(bind_zone_id);
			if (bind_name.length() > 0)
				packet->setDataByName("bind_zone", bind_name.c_str());
		}
		else
			packet->setDataByName("bind_zone", "None");
		if(house_zone_id > 0){
			string house_name = database.GetZoneName(house_zone_id);
			if (house_name.length() > 0)
				packet->setDataByName("house_zone", house_name.c_str());
		}
		else
			packet->setDataByName("house_zone", "None");
		//packet->setDataByName("account_age_base", 14);
		packet->setDataByName("hp_regen", info_struct->get_hp_regen());
		packet->setDataByName("power_regen", info_struct->get_power_regen());
		/*packet->setDataByName("unknown11", -1, 0);
		packet->setDataByName("unknown11", -1, 1);
		packet->setDataByName("unknown13", 201, 0);
		packet->setDataByName("unknown13", 201, 1);
		packet->setDataByName("unknown13", 234, 2);
		packet->setDataByName("unknown13", 201, 3);
		packet->setDataByName("unknown13", 214, 4);
		packet->setDataByName("unknown13", 234, 5);
		packet->setDataByName("unknown13", 234, 6);

		packet->setDataByName("unknown14", 78);
		*/
		packet->setDataByName("adventure_exp_vitality", (int16)(player->GetXPVitality() *10));
		//packet->setDataByName("unknown15b", 9911);
		packet->setDataByName("unknown15a", 78);
		packet->setDataByName("xp_yellow_vitality_bar", info_struct->get_xp_yellow_vitality_bar());
		packet->setDataByName("xp_blue_vitality_bar", info_struct->get_xp_blue_vitality_bar());
		packet->setDataByName("tradeskill_exp_vitality", 100);
		packet->setDataByName("unknown15c", 200);

		//packet->setDataByName("unknown15", 100, 10);
		packet->setDataByName("unknown18", 16880, 1);
		/*packet->setDataByName("unknown19", 1);
		packet->setDataByName("unknown19", 3, 1);
		packet->setDataByName("unknown19", 1074301064, 2);
		packet->setDataByName("unknown19", 1, 3);
		packet->setDataByName("unknown19", 3, 4);
		packet->setDataByName("unknown19", 1074301064, 5);
		packet->setDataByName("unknown19", 6, 6);
		packet->setDataByName("unknown19", 14, 7);
		packet->setDataByName("unknown19", 1083179008, 8);*/
		player->SetGroupInformation(packet);
		packet->setDataByName("unknown20", 1, 107);
		packet->setDataByName("unknown20", 1, 108);
		packet->setDataByName("unknown20", 1, 109);
		packet->setDataByName("unknown20", 1, 110);
		packet->setDataByName("unknown20", 1, 111);
		//packet->setDataByName("unknown20b", 255);
		//packet->setDataByName("unknown20b", 255, 1);
		//packet->setDataByName("unknown20b", 255, 2);
		packet->setDataByName("unknown11", 123);
		packet->setDataByName("unknown11", 234, 1);
		
		//packet->setDataByName("in_combat", 32768);	
		//make name flash red
		/*packet->setDataByName("unknown20", 8);
		packet->setDataByName("unknown20", 38, 70);
		packet->setDataByName("unknown20", 17, 77);
		packet->setDataByName("unknown20", 1, 112); //melee stats and such
		packet->setDataByName("unknown20", 1, 113);
		packet->setDataByName("unknown20", 1, 114);
		packet->setDataByName("unknown20", 1, 115);

		packet->setDataByName("unknown20", 4294967295, 309);
		packet->setDataByName("unknown22", 2, 4);
		packet->setDataByName("unknown23", 2, 29);
		*/
	//packet->setDataByName("unknown20b", 1, i); // pet bar in here
	//	for(int i=0;i<19;i++)
	//		packet->setDataByName("unknown7", 257, i);
		//packet->setDataByName("unknown21", info_struct->rain, 2);
		packet->setDataByName("rain", info_struct->get_rain());
		packet->setDataByName("rain2", info_struct->get_wind()); //-102.24);
		/*packet->setDataByName("unknown22", 3, 4);
		packet->setDataByName("unknown23", 3, 161);
		packet->setDataByName("unknown20", 103);
		packet->setDataByName("unknown20", 1280, 70);
		packet->setDataByName("unknown20", 9, 71);
		packet->setDataByName("unknown20", 5, 72);
		packet->setDataByName("unknown20", 4294967271, 73);
		packet->setDataByName("unknown20", 5, 75);
		packet->setDataByName("unknown20", 1051, 77);
		packet->setDataByName("unknown20", 3, 78);
		packet->setDataByName("unknown20", 6, 104);
		packet->setDataByName("unknown20", 1, 105);
		packet->setDataByName("unknown20", 20, 106);
		packet->setDataByName("unknown20", 3, 107);
		packet->setDataByName("unknown20", 1, 108);
		packet->setDataByName("unknown20", 1, 109);
		packet->setDataByName("unknown20", 4278190080, 494);
		packet->setDataByName("unknown20b", 255);
		packet->setDataByName("unknown20b", 255, 1);
		packet->setDataByName("unknown20b", 255, 2);
		packet->setDataByName("unknown20", 50, 75);
		*/
		//packet->setDataByName("rain2", -102.24);
		for(int i=0;i<45;i++){
			if(i < 30){
				packet->setSubstructDataByName("maintained_effects", "name", info_struct->maintained_effects[i].name, i, 0);
				packet->setSubstructDataByName("maintained_effects", "target", info_struct->maintained_effects[i].target, i, 0);
				packet->setSubstructDataByName("maintained_effects", "spell_id", info_struct->maintained_effects[i].spell_id, i, 0);
				packet->setSubstructDataByName("maintained_effects", "slot_pos", info_struct->maintained_effects[i].slot_pos, i, 0);
				packet->setSubstructDataByName("maintained_effects", "icon", info_struct->maintained_effects[i].icon, i, 0);
				packet->setSubstructDataByName("maintained_effects", "icon_type", info_struct->maintained_effects[i].icon_backdrop, i, 0);
				packet->setSubstructDataByName("maintained_effects", "conc_used", info_struct->maintained_effects[i].conc_used, i, 0);
				packet->setSubstructDataByName("maintained_effects", "unknown3", 1, i, 0);
				packet->setSubstructDataByName("maintained_effects", "total_time", info_struct->maintained_effects[i].total_time, i, 0);
				packet->setSubstructDataByName("maintained_effects", "expire_timestamp", info_struct->maintained_effects[i].expire_timestamp, i, 0);
			}
			else if(version < 942)//version 942 added 15 additional spell effect slots
				break;
			packet->setSubstructDataByName("spell_effects", "spell_id", info_struct->spell_effects[i].spell_id, i, 0);
			if(info_struct->spell_effects[i].spell_id > 0 && info_struct->spell_effects[i].spell_id < 0xFFFFFFFF)
				packet->setSubstructDataByName("spell_effects", "unknown2", 514, i, 0);
			packet->setSubstructDataByName("spell_effects", "total_time", info_struct->spell_effects[i].total_time, i, 0);
			packet->setSubstructDataByName("spell_effects", "expire_timestamp", info_struct->spell_effects[i].expire_timestamp, i, 0);
			packet->setSubstructDataByName("spell_effects", "icon", info_struct->spell_effects[i].icon, i, 0);
			packet->setSubstructDataByName("spell_effects", "icon_type", info_struct->spell_effects[i].icon_backdrop, i, 0);
		}
		return packet;	
	}
	return 0;
}

EQ2Packet* PlayerInfo::serialize3(PacketStruct* packet, int16 version){
	if(packet){
		string* data = packet->serializeString();
		int32 size = data->length();
		//DumpPacket((uchar*)data->c_str(), size);
		uchar* tmp = new uchar[size];
		if(!changes){
			orig_packet = new uchar[size];
			changes = new uchar[size];
			memcpy(orig_packet, (uchar*)data->c_str(), size);
			size = Pack(tmp, (uchar*)data->c_str(), size, size, version);
		}
		else{
			memcpy(changes, (uchar*)data->c_str(), size);
			Encode(changes, orig_packet, size);
			size = Pack(tmp, changes, size, size, version);
			//cout << "INFO HERE:\n";
			//DumpPacket(tmp, size);
		}
		EQ2Packet* ret_packet = new EQ2Packet(OP_UpdateCharacterSheetMsg, tmp, size+4);
		safe_delete_array(tmp);
		safe_delete(packet);
		return ret_packet;
	}
	return 0;
}

void PlayerInfo::SetAccountAge(int32 age){
	info_struct->set_account_age_base(age);
}

EQ2Packet* PlayerInfo::serialize(int16 version, int16 modifyPos, int32 modifyValue) {
	PacketStruct* packet = configReader.getStruct("WS_CharacterSheet", version);
	//0-69, locked screen movement
	//30-69 normal movement
	//10-30 normal movement

	if (packet) {
		char name[40];
		strncpy(name,info_struct->get_name().c_str(),40);
		packet->setDataByName("character_name", name);
		packet->setDataByName("race", info_struct->get_race());
		packet->setDataByName("gender", info_struct->get_gender());
		packet->setDataByName("exiled", 0);  // need exiled data
		packet->setDataByName("class1", info_struct->get_class1());
		packet->setDataByName("class2", info_struct->get_class2());
		packet->setDataByName("class3", info_struct->get_class3());
		packet->setDataByName("tradeskill_class1", info_struct->get_tradeskill_class1());
		packet->setDataByName("tradeskill_class2", info_struct->get_tradeskill_class2());
		packet->setDataByName("tradeskill_class3", info_struct->get_tradeskill_class3());
		packet->setDataByName("level", info_struct->get_level());
		packet->setDataByName("effective_level", info_struct->get_effective_level() != 0 ? info_struct->get_effective_level() : info_struct->get_level());
		packet->setDataByName("tradeskill_level", info_struct->get_tradeskill_level());
		packet->setDataByName("account_age_base", info_struct->get_account_age_base());

		//TODO: 2021 FIX THIS CASTING
		for (int8 i = 0; i < 19; i++)
			packet->setDataByName("account_age_bonus", 0);
		//TODO: 2021 FIX THIS CASTING
		char deity[32];
		strncpy(deity, info_struct->get_deity().c_str(), 32);
		packet->setDataByName("deity", deity);
		
		packet->setDataByName("last_name", player->GetLastName());
		packet->setDataByName("current_hp", player->GetHP());
		packet->setDataByName("max_hp", player->GetTotalHP());
		packet->setDataByName("base_hp", player->GetTotalHPBase());

		packet->setDataByName("current_power", player->GetPower());
		packet->setDataByName("max_power", player->GetTotalPower());
		packet->setDataByName("base_power", player->GetTotalPowerBase());
		packet->setDataByName("conc_used", info_struct->get_cur_concentration());
		packet->setDataByName("conc_max", info_struct->get_max_concentration());
		packet->setDataByName("hp_regen", player->GetInfoStruct()->get_hp_regen());
		packet->setDataByName("power_regen", player->GetInfoStruct()->get_power_regen());

		packet->setDataByName("stat_bonus_health", player->CalculateBonusMod());//bonus health and bonus power getting same value?
		packet->setDataByName("stat_bonus_power", player->CalculateBonusMod());//bonus health and bonus power getting same value?
		float bonus_health = floor((float)(info_struct->get_sta() * player->CalculateBonusMod()));
		packet->setDataByName("bonus_health", bonus_health);
		packet->setDataByName("bonus_power", floor((float)(player->GetPrimaryStat() * player->CalculateBonusMod())));
		packet->setDataByName("stat_bonus_damage", 95); //stat_bonus_damage
		packet->setDataByName("mitigation_cur", info_struct->get_cur_mitigation());// confirmed DoV
		packet->setDataByName("mitigation_base", info_struct->get_mitigation_base());// confirmed DoV
		
		packet->setDataByName("mitigation_pct_pve", info_struct->get_mitigation_pve()); // % calculation Mitigation % vs PvE 392 = 39.2%// confirmed DoV
		packet->setDataByName("mitigation_pct_pvp", info_struct->get_mitigation_pvp()); // % calculation Mitigation % vs PvP 559 = 55.9%// confirmed DoV
		packet->setDataByName("toughness", 0);//toughness// confirmed DoV
		packet->setDataByName("toughness_resist_dmg_pvp", 0);//toughness_resist_dmg_pvp 73 = 7300% // confirmed DoV 
		packet->setDataByName("avoidance_pct", (int16)info_struct->get_avoidance_display()*10.0f);//avoidance_pct 192 = 19.2% // confirmed DoV
		packet->setDataByName("avoidance_base", (int16)info_struct->get_avoidance_base()*10.0f); // confirmed DoV
		packet->setDataByName("avoidance", info_struct->get_cur_avoidance());
		packet->setDataByName("base_avoidance_pct", info_struct->get_base_avoidance_pct());// confirmed DoV
		float parry_pct = info_struct->get_parry(); // client works off of int16, but we use floats to track the actual x/100%
		packet->setDataByName("parry",(int16)(parry_pct*10.0f));// confirmed DoV

		float block_pct = info_struct->get_block()*10.0f;
		
		packet->setDataByName("block", (int16)block_pct);// confirmed DoV
		packet->setDataByName("uncontested_block", info_struct->get_uncontested_block());// confirmed DoV
		packet->setDataByName("str", info_struct->get_str());// confirmed DoV
		packet->setDataByName("sta", info_struct->get_sta());// confirmed DoV
		packet->setDataByName("agi", info_struct->get_agi());// confirmed DoV
		packet->setDataByName("wis", info_struct->get_wis());// confirmed DoV
		packet->setDataByName("int", info_struct->get_intel());// confirmed DoV
		packet->setDataByName("str_base", info_struct->get_str_base());  // confirmed DoV
		packet->setDataByName("sta_base", info_struct->get_sta_base());// confirmed DoV
		packet->setDataByName("agi_base", info_struct->get_agi_base());// confirmed DoV
		packet->setDataByName("wis_base", info_struct->get_wis_base());// confirmed DoV
		packet->setDataByName("int_base", info_struct->get_intel_base());// confirmed DoV
		if (version <= 996) {
			packet->setDataByName("heat", info_struct->get_heat());
			packet->setDataByName("cold", info_struct->get_cold());
			packet->setDataByName("magic", info_struct->get_magic());
			packet->setDataByName("mental", info_struct->get_mental());
			packet->setDataByName("divine", info_struct->get_divine());
			packet->setDataByName("disease", info_struct->get_disease());
			packet->setDataByName("poison", info_struct->get_poison());
			packet->setDataByName("heat_base", info_struct->get_heat_base());
			packet->setDataByName("cold_base", info_struct->get_cold_base());
			packet->setDataByName("magic_base", info_struct->get_magic_base());
			packet->setDataByName("mental_base", info_struct->get_mental_base());
			packet->setDataByName("divine_base", info_struct->get_divine_base());
			packet->setDataByName("disease_base", info_struct->get_disease_base());
			packet->setDataByName("poison_base", info_struct->get_poison_base());
		}
		else {
			packet->setDataByName("elemental", info_struct->get_heat());// confirmed DoV
			packet->setDataByName("noxious", info_struct->get_poison());// confirmed DoV
			packet->setDataByName("arcane", info_struct->get_magic());// confirmed DoV
			packet->setDataByName("elemental_base", info_struct->get_elemental_base());// confirmed DoV
			packet->setDataByName("noxious_base", info_struct->get_noxious_base());// confirmed DoV
			packet->setDataByName("arcane_base", info_struct->get_arcane_base());// confirmed DoV
		}
		packet->setDataByName("elemental_absorb_pve", 0); //210 = 21.0% confirmed DoV
		packet->setDataByName("noxious_absorb_pve", 0);//210 = 21.0% confirmed DoV
		packet->setDataByName("arcane_absorb_pve", 0);//210 = 21.0% confirmed DoV
		packet->setDataByName("elemental_absorb_pvp", 0);//210 = 21.0% confirmed DoV
		packet->setDataByName("noxious_absorb_pvp", 0);//210 = 21.0% confirmed DoV
		packet->setDataByName("arcane_absorb_pvp", 0);//210 = 21.0% confirmed DoV
		packet->setDataByName("elemental_dmg_reduction", 0);// confirmed DoV
		packet->setDataByName("noxious_dmg_reduction", 0);// confirmed DoV
		packet->setDataByName("arcane_dmg_reduction", 0);// confirmed DoV
		packet->setDataByName("elemental_dmg_reduction_pct", 0);//210 = 21.0% confirmed DoV
		packet->setDataByName("noxious_dmg_reduction_pct", 0);//210 = 21.0% confirmed DoV
		packet->setDataByName("arcane_dmg_reduction_pct", 0);//210 = 21.0% confirmed DoV
		CalculateXPPercentages();
		packet->setDataByName("current_adv_xp", info_struct->get_xp()); // confirmed DoV
		packet->setDataByName("needed_adv_xp", info_struct->get_xp_needed());// confirmed DoV

		if(version >= 60114)
		{
			// AoM ends up the debt_adv_xp field is the percentage of xp to the next level needed to advance out of debt (WHYY CANT THIS JUST BE A PERCENTAGE LIKE DOV!)
			float currentPctOfLevel = (float)info_struct->get_xp() / (float)info_struct->get_xp_needed();
			float neededPctAdvanceOutOfDebt = currentPctOfLevel + (info_struct->get_xp_debt() / 100.0f);
			packet->setDataByName("debt_adv_xp", neededPctAdvanceOutOfDebt);
		}
		else
		{
			double currentPctOfLevel = (double)info_struct->get_xp() / (double)info_struct->get_xp_needed();
			double neededPctAdvanceOutOfDebt = (currentPctOfLevel + ((double)info_struct->get_xp_debt() / 100.0)) * 1000.0;
			packet->setDataByName("exp_debt", (int16)(neededPctAdvanceOutOfDebt));//95= 9500% //confirmed DoV
		}
		
		packet->setDataByName("current_trade_xp", info_struct->get_ts_xp());// confirmed DoV
		packet->setDataByName("needed_trade_xp", info_struct->get_ts_xp_needed());// confirmed DoV

		packet->setDataByName("debt_trade_xp", 0);//95= 9500% //confirmed DoV
		packet->setDataByName("server_bonus", 0);//confirmed DoV
		packet->setDataByName("adventure_vet_bonus", 145);//confirmed DoV
		packet->setDataByName("tradeskill_vet_bonus", 123);//confirmed DoV
		packet->setDataByName("recruit_friend", 110);// 110 = 11000% //confirmed DoV
		packet->setDataByName("recruit_friend_bonus", 0);//confirmed DoV

		packet->setDataByName("adventure_vitality", (int16)(player->GetXPVitality() * 10)); // a %%
		packet->setDataByName("adventure_vitality_yellow_arrow", info_struct->get_xp_yellow_vitality_bar()); //change info_struct to match struct
		packet->setDataByName("adventure_vitality_blue_arrow", info_struct->get_xp_blue_vitality_bar());  //change info_struct to match struct

		packet->setDataByName("tradeskill_vitality", 300); //300 = 30%

		packet->setDataByName("tradeskill_vitality_purple_arrow", 0);// dov confirmed
		packet->setDataByName("tradeskill_vitality_blue_arrow", 0);// dov confirmed
		packet->setDataByName("mentor_bonus", 50);//mentor_bonus //this converts wrong says mentor bonus enabled but earning 0

		packet->setDataByName("assigned_aa", player->GetAssignedAA());
		packet->setDataByName("max_aa", rule_manager.GetGlobalRule(R_Player, MaxAA)->GetInt16());
		packet->setDataByName("unassigned_aa", player->GetUnassignedAA()); // dov confirmed
		packet->setDataByName("aa_green_bar", 0);// dov confirmed
		packet->setDataByName("adv_xp_to_aa_xp_slider", 0);  // aa slider max // dov confirmed
		packet->setDataByName("adv_xp_to_aa_xp_max", 100);  // aa slider position // dov confirmed
		packet->setDataByName("aa_blue_bar", 0);// dov confirmed
		packet->setDataByName("bonus_achievement_xp", 0); // dov confirmed

		packet->setDataByName("level_events", 32);// dov confirmed
		packet->setDataByName("items_found", 62);// dov confirmed
		packet->setDataByName("named_npcs_killed", 192);// dov confirmed
		packet->setDataByName("quests_completed", 670);// dov confirmed
		packet->setDataByName("exploration_events", 435);// dov confirmed
		packet->setDataByName("completed_collections", 144);// dov confirmed
		packet->setDataByName("unknown_1096_13_MJ", 80);//unknown_1096_13_MJ
		packet->setDataByName("unknown_1096_14_MJ", 50);//unknown_1096_14_MJ
		packet->setDataByName("coins_copper", info_struct->get_coin_copper());// dov confirmed
		packet->setDataByName("coins_silver", info_struct->get_coin_silver());// dov confirmed
		packet->setDataByName("coins_gold", info_struct->get_coin_gold());// dov confirmed
		packet->setDataByName("coins_plat", info_struct->get_coin_plat());// dov confirmed
		
		Skill* skill = player->GetSkillByName("Swimming", false);
		float breath_modifier = rule_manager.GetZoneRule(player->GetZoneID(), R_Player, SwimmingSkillMinBreathLength)->GetFloat();
		if(skill) {
			int32 max_val = 450;
			if(skill->max_val > 0)
				max_val = skill->max_val;
			float diff = (float)(skill->current_val + player->GetStat(ITEM_STAT_SWIMMING)) / (float)max_val;
			float max_breath_mod = rule_manager.GetZoneRule(player->GetZoneID(), R_Player, SwimmingSkillMaxBreathLength)->GetFloat();
			float diff_mod = max_breath_mod * diff;
			if(diff_mod > max_breath_mod)
				breath_modifier = max_breath_mod;
			else if(diff_mod > breath_modifier)
				breath_modifier = diff_mod;
		}
		packet->setDataByName("breath", breath_modifier);

		packet->setDataByName("melee_pri_dmg_min", player->GetPrimaryWeaponMinDamage());// dov confirmed
		packet->setDataByName("melee_pri_dmg_max", player->GetPrimaryWeaponMaxDamage());// dov confirmed
		packet->setDataByName("melee_sec_dmg_min", player->GetSecondaryWeaponMinDamage());// dov confirmed
		packet->setDataByName("melee_sec_dmg_max", player->GetSecondaryWeaponMaxDamage());// dov confirmed // this is off when using 2 handed weapon
		packet->setDataByName("ranged_dmg_min", player->GetRangedWeaponMinDamage());// dov confirmed
		packet->setDataByName("ranged_dmg_max", player->GetRangedWeaponMaxDamage());// dov confirmed
		if (info_struct->get_attackspeed() > 0) {
			packet->setDataByName("melee_pri_delay", (((float)player->GetPrimaryWeaponDelay() * 1.33) / player->CalculateAttackSpeedMod()) * .001);// dov confirmed
			packet->setDataByName("melee_sec_delay", (((float)player->GetSecondaryWeaponDelay() * 1.33) / player->CalculateAttackSpeedMod()) * .001);// dov confirmed
			packet->setDataByName("ranged_delay", (((float)player->GetRangeWeaponDelay() * 1.33) / player->CalculateAttackSpeedMod()) * .001);// dov confirmed
		}
		else {
			packet->setDataByName("melee_pri_delay", (float)player->GetPrimaryWeaponDelay() * .001);// dov confirmed
			packet->setDataByName("melee_sec_delay", (float)player->GetSecondaryWeaponDelay() * .001);// dov confirmed
			packet->setDataByName("ranged_delay", (float)player->GetRangeWeaponDelay() * .001);// dov confirmed
		}

		packet->setDataByName("ability_mod_pve", info_struct->get_ability_modifier());// dov confirmed
		packet->setDataByName("base_melee_crit", 85);//85 = 8500% dov confirmed
		packet->setDataByName("base_spell_crit", 84);// dov confirmed
		packet->setDataByName("base_taunt_crit", 83);// dov confirmed
		packet->setDataByName("base_heal_crit", 82);// dov confirmed
		packet->setDataByName("flags", info_struct->get_flags());
		packet->setDataByName("flags2", info_struct->get_flags2());
		if (version == 546) {
			if (player->get_character_flag(CF_ANONYMOUS))
				packet->setDataByName("flags_anonymous", 1);
			if (player->get_character_flag(CF_ROLEPLAYING))
				packet->setDataByName("flags_roleplaying", 1);
			if (player->get_character_flag(CF_AFK))
				packet->setDataByName("flags_afk", 1);
			if (player->get_character_flag(CF_LFG))
				packet->setDataByName("flags_lfg", 1);
			if (player->get_character_flag(CF_LFW))
				packet->setDataByName("flags_lfw", 1);
			if (!player->get_character_flag(CF_HIDE_HOOD) && !player->get_character_flag(CF_HIDE_HELM))
				packet->setDataByName("flags_show_hood", 1);
			if (player->get_character_flag(CF_SHOW_ILLUSION))
				packet->setDataByName("flags_show_illusion_form", 1);
			if (player->get_character_flag(CF_ALLOW_DUEL_INVITES))
				packet->setDataByName("flags_show_duel_invites", 1);
			if (player->get_character_flag(CF_ALLOW_TRADE_INVITES))
				packet->setDataByName("flags_show_trade_invites", 1);
			if (player->get_character_flag(CF_ALLOW_GROUP_INVITES))
				packet->setDataByName("flags_show_group_invites", 1);
			if (player->get_character_flag(CF_ALLOW_RAID_INVITES))
				packet->setDataByName("flags_show_raid_invites", 1);
			if (player->get_character_flag(CF_ALLOW_GUILD_INVITES))
				packet->setDataByName("flags_show_guild_invites", 1);
		}

		packet->setDataByName("haste", info_struct->get_haste());// dov confirmed
		packet->setDataByName("drunk", info_struct->get_drunk());// dov confirmed

		packet->setDataByName("hate_mod", info_struct->get_hate_mod());// dov confirmed
		packet->setDataByName("adventure_effects_bonus", 55);// NEED an adventure_effects_bonus// dov confirmed
		packet->setDataByName("tradeskill_effects_bonus", 56);// NEED an tradeskill_effects_bonus// dov confirmed
		packet->setDataByName("dps", info_struct->get_dps());// dov confirmed
		packet->setDataByName("melee_ae", info_struct->get_melee_ae());// dov confirmed
		packet->setDataByName("multi_attack", info_struct->get_multi_attack());// dov confirmed
		packet->setDataByName("spell_multi_attack", info_struct->get_spell_multi_attack());// dov confirmed
		packet->setDataByName("block_chance", info_struct->get_block_chance());// dov confirmed
		packet->setDataByName("crit_chance", info_struct->get_crit_chance());// dov confirmed
		packet->setDataByName("crit_bonus", info_struct->get_crit_bonus());// dov confirmed

		packet->setDataByName("potency", info_struct->get_potency());//info_struct->get_potency);// dov confirmed

		packet->setDataByName("reuse_speed", info_struct->get_reuse_speed());// dov confirmed
		packet->setDataByName("recovery_speed", info_struct->get_recovery_speed());// dov confirmed
		packet->setDataByName("casting_speed", info_struct->get_casting_speed());// dov confirmed
		packet->setDataByName("spell_reuse_speed", info_struct->get_spell_reuse_speed());// dov confirmed
		packet->setDataByName("strikethrough", info_struct->get_strikethrough());//dov confirmed
		packet->setDataByName("accuracy", info_struct->get_accuracy());//dov confirmed
		packet->setDataByName("critical_mit", info_struct->get_critical_mitigation());//dov /confirmed

		((Entity*)player)->MStats.lock();
		packet->setDataByName("durability_mod", player->stats[ITEM_STAT_DURABILITY_MOD]);// dov confirmed
		packet->setDataByName("durability_add", player->stats[ITEM_STAT_DURABILITY_ADD]);// dov confirmed
		packet->setDataByName("progress_mod", player->stats[ITEM_STAT_PROGRESS_MOD]);// dov confirmed
		packet->setDataByName("progress_add", player->stats[ITEM_STAT_PROGRESS_ADD]);// dov confirmed
		packet->setDataByName("success_mod", player->stats[ITEM_STAT_SUCCESS_MOD]);// dov confirmed
		packet->setDataByName("crit_success_mod", player->stats[ITEM_STAT_CRIT_SUCCESS_MOD]);// dov confirmed
		((Entity*)player)->MStats.unlock();

		if (version <= 373 && info_struct->get_pet_id() == 0xFFFFFFFF)
			packet->setDataByName("pet_id", 0);
		else {
			packet->setDataByName("pet_id", info_struct->get_pet_id());
			char pet_name[32];
			strncpy(pet_name, info_struct->get_pet_name().c_str(), version <= 373 ? 16 : 32);
			packet->setDataByName("pet_name", pet_name);
		}

		packet->setDataByName("pet_health_pct", info_struct->get_pet_health_pct());
		packet->setDataByName("pet_power_pct", info_struct->get_pet_power_pct());

		packet->setDataByName("pet_movement", info_struct->get_pet_movement());
		packet->setDataByName("pet_behavior", info_struct->get_pet_behavior());
		packet->setDataByName("rain", info_struct->get_rain());
		packet->setDataByName("rain2", info_struct->get_wind()); //-102.24);
		packet->setDataByName("status_points", info_struct->get_status_points());
		packet->setDataByName("guild_status", 888888);

		if (house_zone_id > 0){
			string house_name = database.GetZoneName(house_zone_id);
			if(house_name.length() > 0)
				packet->setDataByName("house_zone", house_name.c_str());
		}
		else
			packet->setDataByName("house_zone", "None");

		if (bind_zone_id > 0){
			string bind_name = database.GetZoneName(bind_zone_id);
			if(bind_name.length() > 0)
				packet->setDataByName("bind_zone", bind_name.c_str());
		}
		else
			packet->setDataByName("bind_zone", "None");
			
			
		((Entity*)player)->MStats.lock();
		packet->setDataByName("rare_harvest_chance", player->stats[ITEM_STAT_RARE_HARVEST_CHANCE]);
		packet->setDataByName("max_crafting", player->stats[ITEM_STAT_MAX_CRAFTING]);
		packet->setDataByName("component_refund", player->stats[ITEM_STAT_COMPONENT_REFUND]);
		packet->setDataByName("ex_durability_mod", player->stats[ITEM_STAT_EX_DURABILITY_MOD]);
		packet->setDataByName("ex_durability_add", player->stats[ITEM_STAT_EX_DURABILITY_ADD]);
		packet->setDataByName("ex_crit_success_mod", player->stats[ITEM_STAT_EX_CRIT_SUCCESS_MOD]);
		packet->setDataByName("ex_crit_failure_mod", player->stats[ITEM_STAT_EX_CRIT_FAILURE_MOD]);
		packet->setDataByName("ex_progress_mod", player->stats[ITEM_STAT_EX_PROGRESS_MOD]);
		packet->setDataByName("ex_progress_add", player->stats[ITEM_STAT_EX_PROGRESS_ADD]);
		packet->setDataByName("ex_success_mod", player->stats[ITEM_STAT_EX_SUCCESS_MOD]);
		((Entity*)player)->MStats.unlock();

		packet->setDataByName("flurry", info_struct->get_flurry());
		packet->setDataByName("unknown153", 153);
		packet->setDataByName("bountiful_harvest", 0); // need bountiful harvest

		packet->setDataByName("unknown156", 156);
		packet->setDataByName("unknown157", 157);

		packet->setDataByName("unknown159", 159);
		packet->setDataByName("unknown160", 160);


		packet->setDataByName("unknown163", 163);


		packet->setDataByName("unknown168", 168);
		packet->setDataByName("decrease_falling_dmg", 169);

		if (version <= 561) {
			packet->setDataByName("exp_yellow", info_struct->get_xp_yellow() / 10);			
			packet->setDataByName("exp_blue", ((int16)info_struct->get_xp_yellow() % 100) + (info_struct->get_xp_blue() / 100));
		}
		else {
			packet->setDataByName("exp_yellow", info_struct->get_xp_yellow());
			packet->setDataByName("exp_blue", info_struct->get_xp_blue());
		}
		
		if (version <= 561) {
			packet->setDataByName("tradeskill_exp_yellow", info_struct->get_tradeskill_exp_yellow() / 10);
			packet->setDataByName("tradeskill_exp_blue", info_struct->get_tradeskill_exp_blue() / 10);
		}
		else {
			packet->setDataByName("tradeskill_exp_yellow", info_struct->get_tradeskill_exp_yellow());
			packet->setDataByName("tradeskill_exp_blue", info_struct->get_tradeskill_exp_blue());
		}		

		packet->setDataByName("attack", info_struct->get_cur_attack());
		packet->setDataByName("attack_base", info_struct->get_attack_base());
		packet->setDataByName("absorb", info_struct->get_absorb());
		packet->setDataByName("mitigation_skill1", info_struct->get_mitigation_skill1());
		packet->setDataByName("mitigation_skill2", info_struct->get_mitigation_skill2());
		packet->setDataByName("mitigation_skill3", info_struct->get_mitigation_skill3());

		packet->setDataByName("mitigation_max", info_struct->get_max_mitigation());

		packet->setDataByName("savagery", 250);
		packet->setDataByName("max_savagery", 500);
		packet->setDataByName("savagery_level", 1);
		packet->setDataByName("max_savagery_level", 5);
		packet->setDataByName("dissonance", 5000);
		packet->setDataByName("max_dissonance", 10000);

		packet->setDataByName("mitigation_cur2", info_struct->get_cur_mitigation());
		packet->setDataByName("mitigation_max2", info_struct->get_max_mitigation());
		packet->setDataByName("mitigation_base2", info_struct->get_mitigation_base());

		packet->setDataByName("weight", info_struct->get_weight());
		packet->setDataByName("max_weight", info_struct->get_max_weight());
		packet->setDataByName("unknownint32a", 777777);
		packet->setDataByName("unknownint32b", 666666);
		packet->setDataByName("mitigation2_cur", 2367);
		packet->setDataByName("uncontested_riposte", info_struct->get_uncontested_riposte());
		packet->setDataByName("uncontested_dodge", info_struct->get_uncontested_dodge());
		packet->setDataByName("uncontested_parry", info_struct->get_uncontested_parry()); //????
		packet->setDataByName("uncontested_riposte_pve", 0); //????
		packet->setDataByName("uncontested_parry_pve", 0); //????
		packet->setDataByName("total_prestige_points", player->GetPrestigeAA());
		packet->setDataByName("unassigned_prestige_points", player->GetUnassignedPretigeAA());
		packet->setDataByName("total_tradeskill_points", player->GetTradeskillAA());
		packet->setDataByName("unassigned_tradeskill_points", player->GetUnassignedTradeskillAA());
		packet->setDataByName("total_tradeskill_prestige_points", player->GetTradeskillPrestigeAA());
		packet->setDataByName("unassigned_tradeskill_prestige_points", player->GetUnassignedTradeskillPrestigeAA());

		// unknown14c = percent aa exp to next level
		packet->setDataByName("unknown14d", 100, 0);
		packet->setDataByName("unknown20", 1084227584, 72);
		packet->setDataByName("unknown15c", 200);

		player->SetGroupInformation(packet);

		packet->setDataByName("in_combat_movement_speed", 125);

		packet->setDataByName("increase_max_power", 127);
		packet->setDataByName("increase_max_power2", 128);

		packet->setDataByName("vision", info_struct->get_vision());
		packet->setDataByName("breathe_underwater", info_struct->get_breathe_underwater());

		int32 expireTimestamp = 0;
		Spawn* maintained_target = 0;
		player->GetSpellEffectMutex()->readlock(__FUNCTION__, __LINE__);
		player->GetMaintainedMutex()->readlock(__FUNCTION__, __LINE__);
		for (int i = 0; i < 45; i++) {
			if (i < 30) {
				maintained_target = player->GetZone() ? player->GetZone()->GetSpawnByID(info_struct->maintained_effects[i].target) : nullptr;
				packet->setSubstructDataByName("maintained_effects", "name", info_struct->maintained_effects[i].name, i, 0);
				if (maintained_target)
					packet->setSubstructDataByName("maintained_effects", "target", player->GetIDWithPlayerSpawn(maintained_target), i, 0);
				packet->setSubstructDataByName("maintained_effects", "target_type", info_struct->maintained_effects[i].target_type, i, 0);
				packet->setSubstructDataByName("maintained_effects", "spell_id", info_struct->maintained_effects[i].spell_id, i, 0);
				packet->setSubstructDataByName("maintained_effects", "slot_pos", info_struct->maintained_effects[i].slot_pos, i, 0);
				packet->setSubstructDataByName("maintained_effects", "icon", info_struct->maintained_effects[i].icon, i, 0);
				packet->setSubstructDataByName("maintained_effects", "icon_type", info_struct->maintained_effects[i].icon_backdrop, i, 0);
				packet->setSubstructDataByName("maintained_effects", "conc_used", info_struct->maintained_effects[i].conc_used, i, 0);
				packet->setSubstructDataByName("maintained_effects", "unknown3", 1, i, 0);
				packet->setSubstructDataByName("maintained_effects", "total_time", info_struct->maintained_effects[i].total_time, i, 0);
				expireTimestamp = info_struct->maintained_effects[i].expire_timestamp;
				if (expireTimestamp == 0xFFFFFFFF)
					expireTimestamp = 0;
				packet->setSubstructDataByName("maintained_effects", "expire_timestamp", expireTimestamp, i, 0);
			}
			else if (version < 942)//version 942 added 15 additional spell effect slots
				break;
			packet->setSubstructDataByName("spell_effects", "spell_id", info_struct->spell_effects[i].spell_id, i, 0);
			packet->setSubstructDataByName("spell_effects", "total_time", info_struct->spell_effects[i].total_time, i, 0);
			expireTimestamp = info_struct->spell_effects[i].expire_timestamp;
			if (expireTimestamp == 0xFFFFFFFF)
				expireTimestamp = 0;
			packet->setSubstructDataByName("spell_effects", "expire_timestamp", expireTimestamp, i, 0);
			packet->setSubstructDataByName("spell_effects", "icon", info_struct->spell_effects[i].icon, i, 0);
			packet->setSubstructDataByName("spell_effects", "icon_type", info_struct->spell_effects[i].icon_backdrop, i, 0);
			if(info_struct->spell_effects[i].spell && info_struct->spell_effects[i].spell->spell && info_struct->spell_effects[i].spell->spell->GetSpellData()->friendly_spell == 1)
				packet->setSubstructDataByName("spell_effects", "cancellable", 1, i); 
		}
		player->GetMaintainedMutex()->releasereadlock(__FUNCTION__, __LINE__);
		player->GetSpellEffectMutex()->releasereadlock(__FUNCTION__, __LINE__);

		int8 det_count = 0;
		//Send detriment counts as 255 if all dets of that type are incurable
		det_count = player->GetTraumaCount();
		if (det_count > 0) {
			if (!player->HasCurableDetrimentType(DET_TYPE_TRAUMA))
				det_count = 255;
		}
		packet->setDataByName("trauma_count", det_count);

		det_count = player->GetArcaneCount();
		if (det_count > 0) {
			if (!player->HasCurableDetrimentType(DET_TYPE_ARCANE))
				det_count = 255;
		}
		packet->setDataByName("arcane_count", det_count);

		det_count = player->GetNoxiousCount();
		if (det_count > 0) {
			if (!player->HasCurableDetrimentType(DET_TYPE_NOXIOUS))
				det_count = 255;
		}
		packet->setDataByName("noxious_count", det_count);

		det_count = player->GetElementalCount();
		if (det_count > 0) {
			if (!player->HasCurableDetrimentType(DET_TYPE_ELEMENTAL))
				det_count = 255;
		}
		packet->setDataByName("elemental_count", det_count);

		det_count = player->GetCurseCount();
		if (det_count > 0) {
			if (!player->HasCurableDetrimentType(DET_TYPE_CURSE))
				det_count = 255;
		}
		packet->setDataByName("curse_count", det_count);

		player->GetDetrimentMutex()->readlock(__FUNCTION__, __LINE__);
		vector<DetrimentalEffects>* det_list = player->GetDetrimentalSpellEffects();
		DetrimentalEffects det;
		int32 i = 0;
		for (i = 0; i < det_list->size(); i++) {
			det = det_list->at(i);
			packet->setSubstructDataByName("detrimental_spell_effects", "spell_id", det.spell_id, i);
			packet->setSubstructDataByName("detrimental_spell_effects", "total_time", det.total_time, i);
			packet->setSubstructDataByName("detrimental_spell_effects", "icon", det.icon, i);
			packet->setSubstructDataByName("detrimental_spell_effects", "icon_type", det.icon_backdrop, i);
			expireTimestamp = det.expire_timestamp;
			if (expireTimestamp == 0xFFFFFFFF)
				expireTimestamp = 0;
			packet->setSubstructDataByName("detrimental_spell_effects", "expire_timestamp", expireTimestamp, i);
			packet->setSubstructDataByName("detrimental_spell_effects", "unknown2", 2, i);
			if (i == 30) {
				if (version < 942)
					break;
			}
			else if (i == 45)
				break;
		}
		if (version < 942) {
			while (i < 30) {
				packet->setSubstructDataByName("detrimental_spell_effects", "spell_id", 0xFFFFFFFF, i);
				i++;
			}
		}
		else {
			while (i < 45) {
				packet->setSubstructDataByName("detrimental_spell_effects", "spell_id", 0xFFFFFFFF, i);
				i++;
			}
		}
		player->GetDetrimentMutex()->releasereadlock(__FUNCTION__, __LINE__);

		// disabling as not in use right now
		//packet->setDataByName("spirit_rank", 2);
		//packet->setDataByName("spirit", 1);
		//packet->setDataByName("spirit_progress", .67);

		packet->setDataByName("combat_exp_enabled", 1);
		
		string* data = packet->serializeString();
		int32 size = data->length();

		//printf("CharSheet size: %u for version %u\n", size, version);
		//DumpPacket((uchar*)data->c_str(), data->size());
		//packet->PrintPacket();
		uchar* tmp = new uchar[size];
		bool reverse = version > 373;
		if (!changes) {
			orig_packet = new uchar[size];
			changes = new uchar[size];
			memcpy(orig_packet, (uchar*)data->c_str(), size);
			size = Pack(tmp, orig_packet, size, size, version, reverse);
		}
		else {
			memcpy(changes, (uchar*)data->c_str(), size);
			if (modifyPos > 0) {
				uchar* ptr2 = (uchar*)changes;
				ptr2 += modifyPos - 1;
				if (modifyValue > 0xFFFF) {
					memcpy(ptr2, (uchar*)&modifyValue, 4);
				}
				else if (modifyValue > 0xFF) {
					memcpy(ptr2, (uchar*)&modifyValue, 2);
				}
				else
					memcpy(ptr2, (uchar*)&modifyValue, 1);
			}
			Encode(changes, orig_packet, size);
			if (modifyPos > 0) {
				uchar* ptr2 = (uchar*)orig_packet;
				if (modifyPos > 64)
					ptr2 += modifyPos - 64;
				int16 tmpsize = modifyPos + 128;
				if (tmpsize > size)
					tmpsize = size;
			}
			size = Pack(tmp, changes, size, size, version, reverse);
		}

		if (version >= 546 && player->GetClient()) {
			player->GetClient()->SendControlGhost();
		}

		EQ2Packet* ret_packet = new EQ2Packet(OP_UpdateCharacterSheetMsg, tmp, size);
		safe_delete(packet);
		safe_delete_array(tmp);
		return ret_packet;
	}
	return 0;
}

EQ2Packet* PlayerInfo::serializePet(int16 version) {
	PacketStruct* packet = configReader.getStruct("WS_CharacterPet", version);
	if(packet) {
		Spawn* pet = 0;
		pet = player->GetPet();
		if (!pet)
			pet = player->GetCharmedPet();

		if (pet) {
			packet->setDataByName("current_hp", pet->GetHP());
			packet->setDataByName("max_hp", pet->GetTotalHP());
			packet->setDataByName("base_hp", pet->GetTotalHPBase());

			packet->setDataByName("current_power", pet->GetPower());
			packet->setDataByName("max_power", pet->GetTotalPower());
			packet->setDataByName("base_power", pet->GetTotalPowerBase());

			packet->setDataByName("spawn_id", info_struct->get_pet_id());
			packet->setDataByName("spawn_id2", info_struct->get_pet_id());
			
			if(info_struct->get_pet_id() != 0xFFFFFFFF) {
				packet->setDataByName("pet_id", info_struct->get_pet_id());
				char pet_name[32];
				strncpy(pet_name, info_struct->get_pet_name().c_str(), 32);
				packet->setDataByName("name", pet_name);
			}
			else {
				packet->setDataByName("name", "No Pet");
				packet->setDataByName("no_pet", "No Pet");
			}

			if (version >= 57000) {
				packet->setDataByName("current_power3", pet->GetPower());
				packet->setDataByName("max_power3", pet->GetTotalPower());
				packet->setDataByName("health_pct_tooltip", (double)info_struct->get_pet_health_pct());
				packet->setDataByName("health_pct_bar", (double)info_struct->get_pet_health_pct());
			}
			else {
				packet->setDataByName("health_pct_tooltip", info_struct->get_pet_health_pct());
				packet->setDataByName("health_pct_bar", info_struct->get_pet_health_pct());
			}
			packet->setDataByName("power_pct_tooltip", info_struct->get_pet_power_pct());
			packet->setDataByName("power_pct_bar", info_struct->get_pet_power_pct());
			packet->setDataByName("unknown5", 255); // Hate % maybe
			packet->setDataByName("movement", info_struct->get_pet_movement());
			packet->setDataByName("behavior", info_struct->get_pet_behavior());
		}
		else {
			packet->setDataByName("current_hp", 0);
			packet->setDataByName("max_hp", 0);
			packet->setDataByName("base_hp", 0);
			packet->setDataByName("current_power", 0);
			packet->setDataByName("max_power", 0);
			packet->setDataByName("base_power", 0);

			packet->setDataByName("spawn_id", 0);
			packet->setDataByName("spawn_id2", 0xFFFFFFFF);
			packet->setDataByName("name", "");
			packet->setDataByName("no_pet", "No Pet");
			packet->setDataByName("health_pct_tooltip", 0);
			packet->setDataByName("health_pct_bar", 0);
			packet->setDataByName("power_pct_tooltip", 0);
			packet->setDataByName("power_pct_bar", 0);
			packet->setDataByName("unknown5", 0);
			packet->setDataByName("movement", 0);
			packet->setDataByName("behavior", 0);
		}


		string* data = packet->serializeString();
		int32 size = data->length();
		uchar* tmp = new uchar[size];
		// if this is the first time sending this packet create the buffers
		if(!pet_changes){
			pet_orig_packet = new uchar[size];
			pet_changes = new uchar[size];
			// copy the packet into the pet_orig_packet so we can xor against it in the future
			memcpy(pet_orig_packet, (uchar*)data->c_str(), size);
			// pack the packet, result ends up in tmp
			size = Pack(tmp, (uchar*)data->c_str(), size, size, version);
		}
		else{
			// copy the packet into pet_changes
			memcpy(pet_changes, (uchar*)data->c_str(), size);
			// XOR's the packet to the original, stores the new packet in the orig packet (will xor against that for the next update)
			// puts the xor packet into pet_changes.
			Encode(pet_changes, pet_orig_packet, size);
			// Pack the pet_changes packet, will put the packed size at the start, result ends up in tmp
			size = Pack(tmp, pet_changes, size, size, version);
		}

		// Create the packet that we will send
		EQ2Packet* ret_packet = new EQ2Packet(OP_CharacterPet, tmp, size+4);
		// Clean up
		safe_delete_array(tmp);
		safe_delete(packet);
		// Return the packet that will be sent to the client
		return ret_packet;
	}
	return 0;
}

bool Player::DamageEquippedItems(int8 amount, Client* client) {
	bool ret = false;
	int8 item_type;
	Item* item = 0;
	equipment_list.MEquipmentItems.readlock(__FUNCTION__, __LINE__);
	for(int8 i=0;i<NUM_SLOTS;i++){
		item = equipment_list.items[i];
		if(item) {
			item_type = item->generic_info.item_type;
			if (item->details.item_id > 0 && item_type != ITEM_TYPE_FOOD && item_type != ITEM_TYPE_BAUBLE && item_type != ITEM_TYPE_THROWN && 
			!item->CheckFlag2(INDESTRUCTABLE)){
				ret = true;
				if((item->generic_info.condition - amount) > 0)
					item->generic_info.condition -= amount;
				else
					item->generic_info.condition = 0;
				item->save_needed = true;
				if (client)
					client->QueuePacket(item->serialize(client->GetVersion(), false, this));
			}
		}
	}
	equipment_list.MEquipmentItems.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

int16 Player::ConvertSlotToClient(int8 slot, int16 version) {
	if (version <= 373) {
		if (slot == EQ2_FOOD_SLOT)
			slot = EQ2_ORIG_FOOD_SLOT;
		else if (slot == EQ2_DRINK_SLOT)
			slot = EQ2_ORIG_DRINK_SLOT;
		else if (slot > EQ2_EARS_SLOT_1 && slot <= EQ2_WAIST_SLOT)
			slot -= 1;
	}
	else if (version <= 561) {
		if (slot == EQ2_FOOD_SLOT)
			slot = EQ2_DOF_FOOD_SLOT;
		else if (slot == EQ2_DRINK_SLOT)
			slot = EQ2_DOF_DRINK_SLOT;
		else if (slot == EQ2_CHARM_SLOT_1)
			slot = EQ2_DOF_CHARM_SLOT_1;
		else if (slot == EQ2_CHARM_SLOT_2)
			slot = EQ2_DOF_CHARM_SLOT_2;
		else if (slot > EQ2_EARS_SLOT_1 && slot <= EQ2_WAIST_SLOT)
			slot -= 1;
	}
	return slot;
}

int16 Player::ConvertSlotFromClient(int8 slot, int16 version) {
	if (version <= 373) {
		if (slot == EQ2_ORIG_FOOD_SLOT)
			slot = EQ2_FOOD_SLOT;
		else if (slot == EQ2_ORIG_DRINK_SLOT)
			slot = EQ2_DRINK_SLOT;
		else if (slot > EQ2_EARS_SLOT_1 && slot <= EQ2_WAIST_SLOT)
			slot += 1;
	}
	else if (version <= 561) {
		if (slot == EQ2_DOF_FOOD_SLOT)
			slot = EQ2_FOOD_SLOT;
		else if (slot == EQ2_DOF_DRINK_SLOT)
			slot = EQ2_DRINK_SLOT;
		else if (slot == EQ2_DOF_CHARM_SLOT_1)
			slot = EQ2_CHARM_SLOT_1;
		else if (slot == EQ2_DOF_CHARM_SLOT_2)
			slot = EQ2_CHARM_SLOT_2;
		else if (slot > EQ2_EARS_SLOT_1 && slot <= EQ2_WAIST_SLOT)
			slot += 1;
	}
	return slot;
}

int16 Player::GetNumSlotsEquip(int16 version) {
	if(version <= 561) {
		return CLASSIC_NUM_SLOTS;
	}
	
	return NUM_SLOTS;
}

int8 Player::GetMaxBagSlots(int16 version) {
	if(version <= 373) {
		return CLASSIC_EQ_MAX_BAG_SLOTS;
	}
	else if(version <= 561) {
		return DOF_EQ_MAX_BAG_SLOTS;
	}
	
	return 255;
}

vector<EQ2Packet*>	Player::UnequipItem(int16 index, sint32 bag_id, int8 slot, int16 version, int8 appearance_type, bool send_item_updates) {
	vector<EQ2Packet*>	packets;
	EquipmentItemList* equipList = &equipment_list;
	
	if(appearance_type)
		equipList = &appearance_equipment_list;
		
	if(index >= NUM_SLOTS) {
		LogWrite(PLAYER__ERROR, 0, "Player", "%u index is out of range for equip items, bag_id: %i, slot: %u, version: %u, appearance: %u", index, bag_id, slot, version, appearance_type);
		return packets;
	}
	equipList->MEquipmentItems.readlock(__FUNCTION__, __LINE__);
	Item* item = equipList->items[index];
	
	if(item && !IsAllowedCombatEquip(item->details.slot_id, true)) {
		LogWrite(PLAYER__ERROR, 0, "Player", "Attempt to unequip item %s (%u) FAILED in combat!", item->name.c_str(), item->details.item_id);
		equipList->MEquipmentItems.releasereadlock(__FUNCTION__, __LINE__);
		return packets;
	}
	equipList->MEquipmentItems.releasereadlock(__FUNCTION__, __LINE__);

	if (item && bag_id == -999) {
		int8 old_slot = item->details.slot_id;
		if(item->details.equip_slot_id) {
			if (item->GetItemScript() && lua_interface)
				lua_interface->RunItemScript(item->GetItemScript(), "unequipped", item, this);
			const char* zone_script = world.GetZoneScript(GetZone()->GetZoneID());
			if (zone_script && lua_interface)
				lua_interface->RunZoneScript(zone_script, "item_unequipped", GetZone(), this, item->details.item_id, item->name.c_str(), 0, item->details.unique_id);
			item->save_needed = true;
			EQ2Packet* outapp = item_list.serialize(this, version);
			if (outapp) {
				packets.push_back(outapp);
				packets.push_back(item->serialize(version, false));
				EQ2Packet* bag_packet = SendBagUpdate(item->details.inv_slot_id, version);
				if (bag_packet)
					packets.push_back(bag_packet);
			}
			sint16 equip_slot_id = item->details.equip_slot_id;
			item->details.equip_slot_id = 0;
			equipList->RemoveItem(index);
			SetEquippedItemAppearances();
			packets.push_back(equipList->serialize(version, this));
			SetCharSheetChanged(true);
			SetEquipment(0, equip_slot_id ? equip_slot_id : old_slot);
		}
		else if (item_list.AssignItemToFreeSlot(item)) {
			if(appearance_type)
				database.DeleteItem(GetCharacterID(), item, "APPEARANCE");
			else
				database.DeleteItem(GetCharacterID(), item, "EQUIPPED");

			if (item->GetItemScript() && lua_interface)
				lua_interface->RunItemScript(item->GetItemScript(), "unequipped", item, this);
			const char* zone_script = world.GetZoneScript(GetZone()->GetZoneID());
			if (zone_script && lua_interface)
				lua_interface->RunZoneScript(zone_script, "item_unequipped", GetZone(), this, item->details.item_id, item->name.c_str(), 0, item->details.unique_id);
			item->save_needed = true;
			EQ2Packet* outapp = item_list.serialize(this, version);
			if (outapp) {
				packets.push_back(outapp);
				packets.push_back(item->serialize(version, false));
				EQ2Packet* bag_packet = SendBagUpdate(item->details.inv_slot_id, version);
				if (bag_packet)
					packets.push_back(bag_packet);
			}
			equipList->RemoveItem(index);
			SetEquippedItemAppearances();
			packets.push_back(equipList->serialize(version, this));
			SetCharSheetChanged(true);
			SetEquipment(0, old_slot);
		}
		else {
			PacketStruct* packet = configReader.getStruct("WS_DisplayText", version);
			if (packet) {
				packet->setDataByName("color", CHANNEL_COLOR_YELLOW);
				packet->setMediumStringByName("text", "Unable to unequip item: no free inventory locations.");
				packet->setDataByName("unknown02", 0x00ff);
				packets.push_back(packet->serialize());
				safe_delete(packet);
			}
		}
	}
	else if (item) {
		Item* to_item = 0;
		if(appearance_type && slot == 255)
		{
			sint16 tmpSlot = 0;
			item_list.GetFirstFreeSlot(&bag_id, &tmpSlot);
			if(tmpSlot >= 0 && tmpSlot < 255)
				slot = tmpSlot;
			else
				bag_id = 0;
		}

		item_list.MPlayerItems.readlock(__FUNCTION__, __LINE__);
		if (item_list.items.count(bag_id) > 0 && item_list.items[bag_id][BASE_EQUIPMENT].count(slot) > 0)
			to_item = item_list.items[bag_id][BASE_EQUIPMENT][slot];

		bool canEquipToSlot = false;
		if (to_item && equipList->CanItemBeEquippedInSlot(to_item, item->details.slot_id)) {
			canEquipToSlot = true;
		}
		item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);

		if (canEquipToSlot) {
			equipList->RemoveItem(index);
			if(item->details.appearance_type)
				database.DeleteItem(GetCharacterID(), item, "APPEARANCE");
			else
				database.DeleteItem(GetCharacterID(), item, "EQUIPPED");
			
			database.DeleteItem(GetCharacterID(), to_item, "NOT-EQUIPPED");

			if (item->GetItemScript() && lua_interface)
				lua_interface->RunItemScript(item->GetItemScript(), "unequipped", item, this);

			if (to_item->GetItemScript() && lua_interface)
				lua_interface->RunItemScript(to_item->GetItemScript(), "equipped", to_item, this);

			if(item->IsBag() && ( item->details.inv_slot_id != bag_id || item->details.slot_id != slot)) {
				item_list.EraseItem(item);
			}
			item_list.RemoveItem(to_item);
			equipList->SetItem(item->details.slot_id, to_item);
			to_item->save_needed = true;
			packets.push_back(to_item->serialize(version, false));
			SetEquipment(to_item);
			item->details.inv_slot_id = bag_id;
			item->details.slot_id = slot;
			item->details.appearance_type = 0;
			item->details.equip_slot_id = 0;
			
			if(!item->IsBag() && item_list.AddItem(item)) { // bags are omitted because they are equipped while remaining in inventory
				item->save_needed = true;
				SetEquippedItemAppearances();
				// SerializeItemPackets serves item and equipList in opposite order is why we don't use that function here..
				packets.push_back(item->serialize(version, false));
				packets.push_back(equipList->serialize(version, this));
				packets.push_back(item_list.serialize(this, version));
			}
			else if(item->IsBag()) {
				 // already in inventory
			}
			else {
				LogWrite(PLAYER__ERROR, 0, "Player", "failed to add item to item_list during UnequipItem, index %u, bag id %i, slot %u, version %u, appearance type %u", index, bag_id, slot, version, appearance_type);
			}
		}
		else if (to_item && to_item->IsBag() && to_item->details.num_slots > 0) {
			bool free_slot = false;
			for (int8 i = 0; i < to_item->details.num_slots; i++) {
				item_list.MPlayerItems.readlock(__FUNCTION__, __LINE__);
				int32 count = item_list.items[to_item->details.bag_id][appearance_type].count(i);
				item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
				if (count == 0) {
					SetEquipment(0, item->details.equip_slot_id ? item->details.equip_slot_id : item->details.slot_id);

					if(item->details.appearance_type)
						database.DeleteItem(GetCharacterID(), item, "APPEARANCE");
					else
						database.DeleteItem(GetCharacterID(), item, "EQUIPPED");

					if (item->GetItemScript() && lua_interface)
						lua_interface->RunItemScript(item->GetItemScript(), "unequipped", item, this);
					
					if(item->IsBag() && item != to_item) {
						item_list.EraseItem(item);
					}
					
					equipList->RemoveItem(index);
					if(!item->IsBag()) {
						item->details.inv_slot_id = to_item->details.bag_id;
						item->details.slot_id = i;
						item->details.appearance_type = to_item->details.appearance_type;
					}
					else {
						item->details.appearance_type = 0;
					}
					item->details.equip_slot_id = 0;
					
					SerializeItemPackets(equipList, &packets, item, version, to_item);
					free_slot = true;
					break;
				}
			}
			if (!free_slot) {
				PacketStruct* packet = configReader.getStruct("WS_DisplayText", version);
				if (packet) {
					packet->setDataByName("color", CHANNEL_COLOR_YELLOW);
					packet->setMediumStringByName("text", "Unable to unequip item: no free space in the bag.");
					packet->setDataByName("unknown02", 0x00ff);
					packets.push_back(packet->serialize());
					safe_delete(packet);
				}
			}
		}
		else if (to_item) {
			PacketStruct* packet = configReader.getStruct("WS_DisplayText", version);
			if (packet) {
				packet->setDataByName("color", CHANNEL_COLOR_YELLOW);
				packet->setMediumStringByName("text", "Unable to swap items: that item cannot be equipped there.");
				packet->setDataByName("unknown02", 0x00ff);
				packets.push_back(packet->serialize());
				safe_delete(packet);
			}
		}
		else {
			if ((bag_id == 0 && slot < NUM_INV_SLOTS) || (bag_id == -3 && slot < NUM_BANK_SLOTS) || (bag_id == -4 && slot < NUM_SHARED_BANK_SLOTS)) {
				if (bag_id == -4 && item->CheckFlag(NO_TRADE)) {
					PacketStruct* packet = configReader.getStruct("WS_DisplayText", version);
					if (packet) {
						packet->setDataByName("color", CHANNEL_COLOR_YELLOW);
						packet->setMediumStringByName("text", "Unable to unequip item: that item cannot be traded.");
						packet->setDataByName("unknown02", 0x00ff);
						packets.push_back(packet->serialize());
						safe_delete(packet);
					}
				}
				else {
					// need to check if appearance slot vs equipped
					SetEquipment(0, item->details.equip_slot_id ? item->details.equip_slot_id : item->details.slot_id);
					if(item->details.appearance_type)
						database.DeleteItem(GetCharacterID(), item, "APPEARANCE");
					else
						database.DeleteItem(GetCharacterID(), item, "EQUIPPED");

					if (item->GetItemScript() && lua_interface)
						lua_interface->RunItemScript(item->GetItemScript(), "unequipped", item, this);

					if(item->IsBag() && (item->details.inv_slot_id != bag_id || item->details.slot_id != slot)) {
						item_list.EraseItem(item);
					}
					equipList->RemoveItem(index);
					item->details.inv_slot_id = bag_id;
					item->details.slot_id = slot;
					item->details.appearance_type = 0;
					item->details.equip_slot_id = 0;
					SerializeItemPackets(equipList, &packets, item, version);
				}
			}
			else {
				Item* bag = item_list.GetItemFromUniqueID(bag_id, true);
				if (bag && bag->IsBag() && slot < bag->details.num_slots) {
					SetEquipment(0, item->details.equip_slot_id ? item->details.equip_slot_id : item->details.slot_id);
					if(item->details.appearance_type)
						database.DeleteItem(GetCharacterID(), item, "APPEARANCE");
					else
						database.DeleteItem(GetCharacterID(), item, "EQUIPPED");

					if (item->GetItemScript() && lua_interface)
						lua_interface->RunItemScript(item->GetItemScript(), "unequipped", item, this);

					if(item->IsBag() && ( item->details.inv_slot_id != bag_id || item->details.slot_id != slot)) {
						item_list.EraseItem(item);
					}
					equipList->RemoveItem(index);
					item->details.inv_slot_id = bag_id;
					item->details.slot_id = slot;
					item->details.appearance_type = 0;
					item->details.equip_slot_id = 0;
					SerializeItemPackets(equipList, &packets, item, version);
				}
			}
		}
		Item* bag = item_list.GetItemFromUniqueID(bag_id, true);
		if (bag && bag->IsBag())
			packets.push_back(bag->serialize(version, false, this));
	}

	if(send_item_updates && GetClient())
	{
		GetClient()->UpdateSentSpellList();
		GetClient()->ClearSentSpellList();
	}

	return packets;
}

map<int32, Item*>* Player::GetItemList(){
	return item_list.GetAllItems();
}

vector<Item*>* Player::GetEquippedItemList(){
	return equipment_list.GetAllEquippedItems();
}

vector<Item*>* Player::GetAppearanceEquippedItemList(){
	return appearance_equipment_list.GetAllEquippedItems();
}

EQ2Packet*	Player::SendBagUpdate(int32 bag_unique_id, int16 version){
	Item* bag = 0;
	if(bag_unique_id > 0)
		bag = item_list.GetItemFromUniqueID(bag_unique_id, true);

	if(bag && bag->IsBag())
		return bag->serialize(version, false, this);
	return 0;
}

void Player::SetEquippedItemAppearances(){
	vector<Item*>* items = GetEquipmentList()->GetAllEquippedItems();
	vector<Item*>* appearance_items = GetAppearanceEquipmentList()->GetAllEquippedItems();
	if(items){
		for(int32 i=0;i<items->size();i++)
			SetEquipment(items->at(i));
		
		// just have appearance items brute force replace the slots after the fact
		for(int32 i=0;i<appearance_items->size();i++)
			SetEquipment(appearance_items->at(i));
	}
	safe_delete(items);
	safe_delete(appearance_items);
	info_changed = true;
	GetZone()->SendSpawnChanges(this);
}

EQ2Packet* Player::SwapEquippedItems(int8 slot1, int8 slot2, int16 version, int16 equip_type){
	EquipmentItemList* equipList = &equipment_list;
	
	// right now client seems to pass 3 for this? Not sure why when other fields has appearance equipment as type 1
	if(equip_type == 3)
		equipList = &appearance_equipment_list;
	
	equipList->MEquipmentItems.readlock(__FUNCTION__, __LINE__);
	Item* item_from = equipList->items[slot1];
	Item* item_to = equipList->items[slot2];
	equipList->MEquipmentItems.releasereadlock(__FUNCTION__, __LINE__);

	if(item_from && equipList->CanItemBeEquippedInSlot(item_from, slot2)){
		if(item_to){
			if(!equipList->CanItemBeEquippedInSlot(item_to, slot1))
				return 0;
		}
		equipList->MEquipmentItems.writelock(__FUNCTION__, __LINE__);
		equipList->items[slot1] = nullptr;
		equipList->MEquipmentItems.releasewritelock(__FUNCTION__, __LINE__);
		equipList->SetItem(slot2, item_from);
		if(item_to)
		{
			equipList->SetItem(slot1, item_to);
			item_to->save_needed = true;
		}
		item_from->save_needed = true;
		
		if (GetClient())
		{
			//EquipmentItemList* equipList = &equipment_list;
			
			//if(appearance_type)
			//	equipList = &appearance_equipment_list;
			
			if(item_to)
				GetClient()->QueuePacket(item_to->serialize(version, false, this));
			GetClient()->QueuePacket(item_from->serialize(version, false, this));
			GetClient()->QueuePacket(item_list.serialize(this, version));
		}
		return equipList->serialize(version, this);
	}
	return 0;
}
bool Player::CanEquipItem(Item* item, int8 slot) {
	if(client && client->GetVersion() <= 561 && slot == EQ2_EARS_SLOT_2)
		return false;
	
	if (item) {
		Client* client = GetClient();
		if (client) {
			if (item->IsWeapon() && slot == 1) {
				bool dwable = item->IsDualWieldAble(client, item, slot);

				if (dwable == 0) {
					return false;
				}
			}

			if (item->CheckFlag(EVIL_ONLY) && GetAlignment() != ALIGNMENT_EVIL) {
					client->Message(0, "%s requires an evil race.", item->name.c_str());
			}
			else if (item->CheckFlag(GOOD_ONLY) && GetAlignment() != ALIGNMENT_GOOD) {
					client->Message(0, "%s requires a good race.", item->name.c_str());
			}
			else if (item->IsArmor() || item->IsWeapon() || item->IsFood() || item->IsRanged() || item->IsShield() || item->IsBauble() || item->IsAmmo() || item->IsThrown()) {
				if (((item->generic_info.skill_req1 == 0 || item->generic_info.skill_req1 == 0xFFFFFFFF || skill_list.HasSkill(item->generic_info.skill_req1)) && (item->generic_info.skill_req2 == 0 || item->generic_info.skill_req2 == 0xFFFFFFFF || skill_list.HasSkill(item->generic_info.skill_req2)))) {
					int16 override_level = item->GetOverrideLevel(GetAdventureClass(), GetTradeskillClass());
					if (override_level > 0 && override_level <= GetLevel())
						return true;
					if (item->CheckClass(GetAdventureClass(), GetTradeskillClass()))
						if (item->CheckLevel(GetAdventureClass(), GetTradeskillClass(), GetLevel()))
							return true;
						else
							client->Message(CHANNEL_COLOR_RED, "You must be at least level %u to equip %s.", item->generic_info.adventure_default_level, item->CreateItemLink(client->GetVersion()).c_str());
					else
						client->Message(CHANNEL_COLOR_RED, "Your class may not equip %s.", item->CreateItemLink(client->GetVersion()).c_str());
				}
				else {
					Skill* firstSkill = master_skill_list.GetSkill(item->generic_info.skill_req1);
					Skill* secondSkill = master_skill_list.GetSkill(item->generic_info.skill_req2);
					std::string msg("");
					if(GetClient()->GetAdminStatus() >= 200) {
						if(firstSkill && !skill_list.HasSkill(item->generic_info.skill_req1)) {
							msg += "(" + std::string(firstSkill->name.data.c_str());
						}
						
						if(secondSkill && !skill_list.HasSkill(item->generic_info.skill_req2)) {
							if(msg.length() > 0) {
								msg += ", ";
							}
							else {
								msg = "(";
							}
							msg += std::string(secondSkill->name.data.c_str());
						}
						
						if(msg.length() > 0) {
							msg += ") ";
						}
					}
					client->Message(0, "You lack the skill %srequired to equip this item.",msg.c_str());
				}
			}
			else
				client->Message(0, "Item %s isn't equipable.", item->name.c_str());
		}
	}
	return false;
}

vector<EQ2Packet*> Player::EquipItem(int16 index, int16 version, int8 appearance_type, int8 slot_id) {

	EquipmentItemList* equipList = &equipment_list;
	if(appearance_type)
		equipList = &appearance_equipment_list;

	vector<EQ2Packet*>	packets;
	item_list.MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if (item_list.indexed_items.count(index) == 0) {
		item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
		return packets;
	}
	Item* item = item_list.indexed_items[index];
	int8 orig_slot_id = slot_id;
	int8 slot = 255;
	if (item) {
		if(orig_slot_id == 255 && item->CheckFlag2(APPEARANCE_ONLY)) {
			appearance_type = 1;
			equipList = &appearance_equipment_list;
		}
		if (slot_id != 255 && !item->HasSlot(slot_id)) {
			item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
			return packets;
		}
		slot = equipList->GetFreeSlot(item, slot_id, version);
		
		bool canEquip = CanEquipItem(item,slot);
		int32 conflictSlot = 0;
		
		if(canEquip && !appearance_type && item->CheckFlag2(APPEARANCE_ONLY))
		{
			item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
			if(GetClient()) {
				GetClient()->SimpleMessage(CHANNEL_COLOR_RED, "This item is for appearance slots only.");
			}
			return packets;
		}
		else if(canEquip && (conflictSlot = equipList->CheckSlotConflict(item)) > 0) {
			bool abort = true;
			switch(conflictSlot) {
				case LORE:
					if(GetClient())
						GetClient()->SimpleMessage(CHANNEL_COLOR_RED, "Lore conflict, cannot equip this item.");
					break;
				case LORE_EQUIP:
					if(GetClient())
						GetClient()->SimpleMessage(CHANNEL_COLOR_RED, "You already have this item equipped, you cannot equip another.");
					break;
				case STACK_LORE:
					if(GetClient())
						GetClient()->SimpleMessage(CHANNEL_COLOR_RED, "Cannot equip as it exceeds lore stack.");
					break;
				default:
					abort = false;
					break;
			}
			if(abort) {
				item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
				return packets;
			}
		}
		else if (canEquip && item->CheckFlag(ATTUNEABLE)) {
			PacketStruct* packet = configReader.getStruct("WS_ChoiceWindow", version);
			char text[255];
			sprintf(text, "%s must be attuned before it can be equipped. Would you like to attune it now?", item->name.c_str());
			char accept_command[25];
			sprintf(accept_command, "attune_inv %i 1 0 -1", index);
			packet->setDataByName("text", text);
			packet->setDataByName("accept_text", "Attune");
			packet->setDataByName("accept_command", accept_command);
			packet->setDataByName("cancel_text", "Cancel");
			// No clue if we even need the following 2 unknowns, just added them so the packet matches what live sends
			packet->setDataByName("max_length", 50);
			packet->setDataByName("unknown4", 1);
			packets.push_back(packet->serialize());
			safe_delete(packet);
			item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
			return packets;
		}
		if (canEquip && slot == 255)
		{
			if (slot_id == 255) {
				if(item->slot_data.size() > 0) {
					slot = item->slot_data.at(0);
					if(!IsAllowedCombatEquip(slot, true)) {
						LogWrite(PLAYER__ERROR, 0, "Player", "Attempt to equip item %s (%u) with FAILED in combat!", item->name.c_str(), item->details.item_id);
						item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
						return packets;
					}
				}
				else {
					LogWrite(PLAYER__ERROR, 0, "Player", "Attempt to equip item %s (%u) with auto equip FAILED, no slot_data exists!  Check items table, 'slots' column value should not be 0.", item->name.c_str(), item->details.item_id);
					item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
					return packets;
				}
			}
			else
				slot = slot_id;
			item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
			packets = UnequipItem(slot, item->details.inv_slot_id, item->details.slot_id, version, appearance_type, false);
			// grab player items lock again and assure item still present
			item_list.MPlayerItems.readlock(__FUNCTION__, __LINE__);
			if (item_list.indexed_items.count(index) == 0) {
				item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
				return packets;
			}
			// If item is a 2handed weapon and something is in the secondary, unequip the secondary
			if (item->IsWeapon() && item->weapon_info->wield_type == ITEM_WIELD_TYPE_TWO_HAND && equipList->GetItem(EQ2_SECONDARY_SLOT) != 0) {
				item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
				vector<EQ2Packet*> tmp_packets = UnequipItem(EQ2_SECONDARY_SLOT, -999, 0, version, appearance_type, false);
				//packets.reserve(packets.size() + tmp_packets.size());
				packets.insert(packets.end(), tmp_packets.begin(), tmp_packets.end());
			}
			else {
				// release for delete item / scripting etc
				item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
			}
		}
		else if (canEquip && slot < 255) {
			
			if(!IsAllowedCombatEquip(slot, true)) {
				LogWrite(PLAYER__ERROR, 0, "Player", "Attempt to equip item %s (%u) with auto equip FAILED in combat!", item->name.c_str(), item->details.item_id);
				item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
				return packets;
			}
			// If item is a 2handed weapon and something is in the secondary, unequip the secondary
			if (item->IsWeapon() && item->weapon_info->wield_type == ITEM_WIELD_TYPE_TWO_HAND && equipList->GetItem(EQ2_SECONDARY_SLOT) != 0) {
				item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
				vector<EQ2Packet*> tmp_packets = UnequipItem(EQ2_SECONDARY_SLOT, -999, 0, version, appearance_type, false);
				//packets.reserve(packets.size() + tmp_packets.size());
				packets.insert(packets.end(), tmp_packets.begin(), tmp_packets.end());
			}
			else {
				// release for delete item / scripting etc
				item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
			}

			database.DeleteItem(GetCharacterID(), item, "NOT-EQUIPPED");

			if (item->GetItemScript() && lua_interface)
				lua_interface->RunItemScript(item->GetItemScript(), "equipped", item, this);
			
			if(!item->IsBag()) {
				item_list.RemoveItem(item);
			}
			equipList->SetItem(slot, item);
			item->save_needed = true;
			packets.push_back(item->serialize(version, false));
			SetEquipment(item);
			const char* zone_script = world.GetZoneScript(GetZone()->GetZoneID());
			if (zone_script && lua_interface)
				lua_interface->RunZoneScript(zone_script, "item_equipped", GetZone(), this, item->details.item_id, item->name.c_str(), 0, item->details.unique_id);
			int32 bag_id = item->details.inv_slot_id;
			if (item->generic_info.condition == 0) {
				Client* client = GetClient();
				if (client) {
					string popup_text = "Your ";
					string popup_item = item->CreateItemLink(client->GetVersion(), true).c_str();
					string popup_textcont = " is worn out and will not be effective until repaired.";
					popup_text.append(popup_item);
					popup_text.append(popup_textcont);
					//devn00b: decided to use "crimson" for the color. (220,20,60 rgb)
					client->SendPopupMessage(10, popup_text.c_str(), "", 5, 0xDC, 0x14, 0x3C);
					client->Message(CHANNEL_COLOR_RED, "Your %s is worn out and will not be effective until repaired.", item->CreateItemLink(client->GetVersion(), true).c_str());
				}
			}
			SetEquippedItemAppearances();
			packets.push_back(equipList->serialize(version, this));
			EQ2Packet* outapp = item_list.serialize(this, version);
			if (outapp) {
				packets.push_back(outapp);
				EQ2Packet* bag_packet = SendBagUpdate(bag_id, version);
				if (bag_packet)
					packets.push_back(bag_packet);
			}
			SetCharSheetChanged(true);
		}
		else {
				// clear items lock
				item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
		}
	}
	else {
			// clear items lock
			item_list.MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	}
	
	if(slot < 255) {
		if (slot == EQ2_FOOD_SLOT && item->IsFoodFood() && get_character_flag(CF_FOOD_AUTO_CONSUME)) {
			Item* item = GetEquipmentList()->GetItem(EQ2_FOOD_SLOT);
			if(item && GetClient() && GetClient()->CheckConsumptionAllowed(slot, false))
				GetClient()->ConsumeFoodDrink(item, EQ2_FOOD_SLOT);
			
			if(item)
				SetActiveFoodUniqueID(item->details.unique_id);
		}
		else if (slot == EQ2_DRINK_SLOT && item->IsFoodDrink() && get_character_flag(CF_DRINK_AUTO_CONSUME)) {
			Item* item = GetEquipmentList()->GetItem(EQ2_DRINK_SLOT);
			if(item && GetClient() && GetClient()->CheckConsumptionAllowed(slot, false))
				GetClient()->ConsumeFoodDrink(item, EQ2_DRINK_SLOT);
			
			if(item)
				SetActiveDrinkUniqueID(item->details.unique_id);
		}
	}
	
	client->UpdateSentSpellList();
	client->ClearSentSpellList();

	return packets;
}
bool Player::AddItem(Item* item, AddItemType type) {
	int32 conflictItemList = 0, conflictequipmentList = 0, conflictAppearanceEquipmentList = 0;
	int16 lore_stack_count = 0;
	if (item && item->details.item_id > 0) {
		if( ((conflictItemList = item_list.CheckSlotConflict(item, true, true, &lore_stack_count)) == LORE ||
		   (conflictequipmentList = equipment_list.CheckSlotConflict(item, true, &lore_stack_count)) == LORE ||
		   (conflictAppearanceEquipmentList = appearance_equipment_list.CheckSlotConflict(item, true, &lore_stack_count)) == LORE) && !item->CheckFlag(STACK_LORE)) {
			   
			switch(type)
			{
				case AddItemType::BUY_FROM_BROKER:
				client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "You already own this item and cannot have another.");
				break;
				default:
				client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "You cannot obtain %s due to lore conflict.", item->name.c_str());
				break;			
			}
			safe_delete(item);
			return false;
		}
		else if(conflictItemList == STACK_LORE || conflictequipmentList == STACK_LORE || 
				conflictAppearanceEquipmentList == STACK_LORE) {
					switch(type)
					{
						default:
						client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "You already have one stack of the LORE item: %s.", item->name.c_str());
						break;			
					}
			safe_delete(item);
			return false;
		}
		else if (item_list.AssignItemToFreeSlot(item)) {
			item->save_needed = true;
			CalculateApplyWeight();
			return true;
		}
		else if (item_list.AddOverflowItem(item)) {
			CalculateApplyWeight();
			return true;
		}
	}
	return false;
}
bool Player::AddItemToBank(Item* item) {

	if (item && item->details.item_id > 0) {

		sint32 bag = -3;
		sint16 slot = -1;
		if (item_list.GetFirstFreeBankSlot(&bag, &slot)) {
			item->details.inv_slot_id = bag;
			item->details.slot_id = slot;
			item->save_needed = true;

			return item_list.AddItem(item);
		}
		else if (item_list.AddOverflowItem(item))
			return true;
	}
	return false;
}
EQ2Packet* Player::SendInventoryUpdate(int16 version) {
	// assure any inventory updates are reflected in sell window
	if(GetClient() && GetClient()->GetMerchantTransactionID())
		GetClient()->SendSellMerchantList();
	
	return item_list.serialize(this, version);
}

void Player::UpdateInventory(int32 bag_id) {

	EQ2Packet* outapp = client->GetPlayer()->SendInventoryUpdate(client->GetVersion());
	client->QueuePacket(outapp);

	outapp = client->GetPlayer()->SendBagUpdate(bag_id, client->GetVersion());

	if (outapp)
		client->QueuePacket(outapp);

}
EQ2Packet* Player::MoveInventoryItem(sint32 to_bag_id, int16 from_index, int8 new_slot, int8 charges, int8 appearance_type, bool* item_deleted, int16 version) {

	Item* item = item_list.GetItemFromIndex(from_index);
	bool isOverflow = ((item != nullptr) && (item->details.inv_slot_id == -2));
	int8 result = item_list.MoveItem(to_bag_id, from_index, new_slot, appearance_type, charges);
	if (result == 1) {
		if(isOverflow && item->details.inv_slot_id != -2) {
			item_list.RemoveOverflowItem(item);
		}
		if (item) {
			if (!item->needs_deletion)
				item->save_needed = true;
			else if (item->needs_deletion) {
				database.DeleteItem(GetCharacterID(), item, 0);
				client->GetPlayer()->item_list.DestroyItem(from_index);
				client->GetPlayer()->UpdateInventory(to_bag_id);
				if(item_deleted)
					*item_deleted = true;
			}
		}
		return item_list.serialize(this, version);
	}
	else {
		PacketStruct* packet = configReader.getStruct("WS_DisplayText", version);
		if (packet) {
			packet->setDataByName("color", CHANNEL_COLOR_YELLOW);
			packet->setMediumStringByName("text", "Could not move item to that location.");
			packet->setDataByName("unknown02", 0x00ff);
			EQ2Packet* outapp = packet->serialize();
			safe_delete(packet);
			return outapp;
		}
	}
	return 0;
}

int32 Player::GetCoinsCopper(){
	return GetInfoStruct()->get_coin_copper();
}

int32 Player::GetCoinsSilver(){
	return GetInfoStruct()->get_coin_silver();
}

int32 Player::GetCoinsGold(){
	return GetInfoStruct()->get_coin_gold();
}

int32 Player::GetCoinsPlat(){
	return GetInfoStruct()->get_coin_plat();
}

int32 Player::GetBankCoinsCopper(){
	return GetInfoStruct()->get_bank_coin_copper();
}

int32 Player::GetBankCoinsSilver(){
	return GetInfoStruct()->get_bank_coin_silver();
}

int32 Player::GetBankCoinsGold(){
	return GetInfoStruct()->get_bank_coin_gold();
}

int32 Player::GetBankCoinsPlat(){
	return GetInfoStruct()->get_bank_coin_plat();
}

int32 Player::GetStatusPoints(){
	return GetInfoStruct()->get_status_points();
}

vector<QuickBarItem*>* Player::GetQuickbar(){
	return &quickbar_items;
}

bool Player::UpdateQuickbarNeeded(){
	return quickbar_updated;
}

void Player::ResetQuickbarNeeded(){
	quickbar_updated = false;
}

void Player::AddQuickbarItem(int32 bar, int32 slot, int32 type, int16 icon, int16 icon_type, int32 id, int8 tier, int32 unique_id, const char* text, bool update){
	RemoveQuickbarItem(bar, slot, false);
	QuickBarItem* ability = new QuickBarItem;
	ability->deleted = false;
	ability->hotbar = bar;
	ability->slot = slot;
	ability->type = type;
	ability->icon = icon;
	ability->tier = tier;
	ability->icon_type = icon_type;
	ability->id = id;
	if(unique_id == 0)
		unique_id = database.NextUniqueHotbarID();
	ability->unique_id = unique_id;
	if(type == QUICKBAR_TEXT_CMD && text){
		ability->text.data = string(text);
		ability->text.size = ability->text.data.length();
	}
	else
		ability->text.size = 0;
	quickbar_items.push_back(ability);
	if(update)
		quickbar_updated = true;
}

void Player::RemoveQuickbarItem(int32 bar, int32 slot, bool update){
	vector<QuickBarItem*>::iterator itr;
	QuickBarItem* qbi = 0;
	for(itr=quickbar_items.begin();itr!=quickbar_items.end();itr++){
		qbi = *itr;
		if(qbi && qbi->deleted == false && qbi->hotbar == bar && qbi->slot == slot){
			qbi->deleted = true;
			break;
		}
	}
	if(update)
		quickbar_updated = true;
}

void Player::ClearQuickbarItems(){
	quickbar_items.clear();
}

EQ2Packet* Player::GetQuickbarPacket(int16 version){
	PacketStruct* packet = configReader.getStruct("WS_QuickBarInit", version);
	if(packet){
		vector<QuickBarItem*>::iterator itr;
		packet->setArrayLengthByName("num_abilities", quickbar_items.size());
		int16 i=0;
		for(itr=quickbar_items.begin();itr != quickbar_items.end(); itr++){
			QuickBarItem* ability = *itr;
			if(!ability || ability->deleted)
				continue;
			packet->setArrayDataByName("hotbar", ability->hotbar, i);
			packet->setArrayDataByName("slot", ability->slot, i);
			packet->setArrayDataByName("type", ability->type, i);
			packet->setArrayDataByName("icon", ability->icon, i);
			packet->setArrayDataByName("icon_type", ability->icon_type, i);
			packet->setArrayDataByName("id", ability->id, i);
			packet->setArrayDataByName("unique_id", ability->tier, i);
			packet->setArrayDataByName("text", &ability->text, i);
			i++;
		}
		EQ2Packet* app = packet->serialize();
		safe_delete(packet);
		return app;
	}
	return 0;
}

void Player::AddSpellBookEntry(int32 spell_id, int8 tier, sint32 slot, int32 type, int32 timer, bool save_needed){
	SpellBookEntry* spell = new SpellBookEntry;
	spell->status = 169;
	spell->slot = slot;
	spell->spell_id = spell_id;
	spell->type = type;
	spell->tier = tier;
	spell->timer = timer;
	spell->save_needed = save_needed;
	spell->recast = 0;
	spell->recast_available = 0;
	spell->player = this;
	spell->visible = true;
	spell->in_use = false;
	spell->in_remiss = false;
	MSpellsBook.lock();
	spells.push_back(spell);
	MSpellsBook.unlock();
	
	if (type == SPELL_BOOK_TYPE_NOT_SHOWN)
		AddPassiveSpell(spell_id, tier);
}

void Player::DeleteSpellBook(int8 type_selection){
	MSpellsBook.lock();
	vector<SpellBookEntry*>::iterator itr;
	SpellBookEntry* spell = 0;
	for(itr = spells.begin(); itr != spells.end();){
		spell = *itr;
		if((type_selection & DELETE_TRADESKILLS) == 0 && spell->type == SPELL_BOOK_TYPE_TRADESKILL) {
			itr++;
			continue;
		}
		else if((type_selection & DELETE_SPELLS) == 0 && spell->type == SPELL_BOOK_TYPE_SPELL) {
			itr++;
			continue;
		}
		else if((type_selection & DELETE_COMBAT_ART) == 0 && spell->type == SPELL_BOOK_TYPE_COMBAT_ART) {
			itr++;
			continue;
		}
		else if((type_selection & DELETE_ABILITY) == 0 && spell->type == SPELL_BOOK_TYPE_ABILITY) {
			itr++;
			continue;
		}
		else if((type_selection & DELETE_NOT_SHOWN) == 0 && spell->type == SPELL_BOOK_TYPE_NOT_SHOWN) {
			itr++;
			continue;
		}
		database.DeleteCharacterSpell(GetCharacterID(), spell->spell_id);
		if (spell->type == SPELL_BOOK_TYPE_NOT_SHOWN)
			RemovePassive(spell->spell_id, spell->tier, true);
		itr = spells.erase(itr);
	}
	MSpellsBook.unlock();
}

void Player::RemoveSpellBookEntry(int32 spell_id, bool remove_passives_from_list){
	MSpellsBook.lock();
	vector<SpellBookEntry*>::iterator itr;
	SpellBookEntry* spell = 0;
	for(itr = spells.begin(); itr != spells.end(); itr++){
		spell = *itr;
		if(spell->spell_id == spell_id){
			if (spell->type == SPELL_BOOK_TYPE_NOT_SHOWN)
				RemovePassive(spell->spell_id, spell->tier, remove_passives_from_list);
			spells.erase(itr);
			break;
		}
	}
	MSpellsBook.unlock();
}

void Player::ResortSpellBook(int32 sort_by, int32 order, int32 pattern, int32 maxlvl_only, int32 book_type)
{
	//sort_by : 0 - alpha, 1 - level, 2 - category
	//order : 0 - ascending, 1 - descending
	//pattern : 0 - zigzag, 1 - down, 2 - across
	MSpellsBook.lock();

	std::vector<SpellBookEntry*> sort_spells(spells);
	
	if (!maxlvl_only)
	{
		switch (sort_by)
		{
		case 0:
			if (!order)
				stable_sort(sort_spells.begin(), sort_spells.end(), SortSpellEntryByName);
			else
				stable_sort(sort_spells.begin(), sort_spells.end(), SortSpellEntryByNameReverse);
			break;
		case 1:
			if (!order)
				stable_sort(sort_spells.begin(), sort_spells.end(), SortSpellEntryByLevel);
			else
				stable_sort(sort_spells.begin(), sort_spells.end(), SortSpellEntryByLevelReverse);
			break;
		case 2:
			if (!order)
				stable_sort(sort_spells.begin(), sort_spells.end(), SortSpellEntryByCategory);
			else
				stable_sort(sort_spells.begin(), sort_spells.end(), SortSpellEntryByCategoryReverse);
			break;
		}
	}

	vector<SpellBookEntry*>::iterator itr;
	SpellBookEntry* spell = 0;
	map<string, SpellBookEntry*> tmpSpells;
	vector<SpellBookEntry*> resultSpells;
	
	int32 i = 0;
	int8 page_book_count = 0;
	int32 last_start_point = 0;
	
	for (itr = sort_spells.begin(); itr != sort_spells.end(); itr++) {
		spell = *itr;

		if (spell->type != book_type)
			continue;

		if (maxlvl_only)
		{
			Spell* actual_spell = 0;
			actual_spell = master_spell_list.GetSpell(spell->spell_id, spell->tier);
			if(!actual_spell) {
				// we have a spell that doesn't exist here!
				continue;
			}
			std::regex re("^(.*?)(\\s(I{1,}[VX]{0,}|V{1,}[IVX]{0,})|X{1,}[IVX]{0,})$");
			std::string output = std::regex_replace(string(actual_spell->GetName()), re, "$1", std::regex_constants::format_no_copy);

			if ( output.size() < 1 )
				output = string(actual_spell->GetName());

			map<string, SpellBookEntry*>::iterator tmpItr = tmpSpells.find(output);
			if (tmpItr != tmpSpells.end())
			{
				Spell* tmpSpell = master_spell_list.GetSpell(tmpItr->second->spell_id, tmpItr->second->tier);
				if (actual_spell->GetLevelRequired(this) > tmpSpell->GetLevelRequired(this))
				{
					tmpItr->second->visible = false;
					tmpItr->second->slot = 0xFFFF;

					std::vector<SpellBookEntry*>::iterator it;
					it = find(resultSpells.begin(), resultSpells.end(), (SpellBookEntry*)tmpItr->second);
					if (it != resultSpells.end())
						resultSpells.erase(it);

					tmpSpells.erase(tmpItr);
				}
				else
					continue; // leave as-is we have the newer spell
			}
			
			spell->visible = true;
			tmpSpells.insert(make_pair(output, spell));
			resultSpells.push_back(spell);
		}
		spell->slot = i;
		
		GetSpellBookSlotSort(pattern, &i, &page_book_count, &last_start_point);
	} // end for loop for setting slots
	
	if (maxlvl_only)
	{
		switch (sort_by)
		{
		case 0:
			if (!order)
				stable_sort(resultSpells.begin(), resultSpells.end(), SortSpellEntryByName);
			else
				stable_sort(resultSpells.begin(), resultSpells.end(), SortSpellEntryByNameReverse);
			break;
		case 1:
			if (!order)
				stable_sort(resultSpells.begin(), resultSpells.end(), SortSpellEntryByLevel);
			else
				stable_sort(resultSpells.begin(), resultSpells.end(), SortSpellEntryByLevelReverse);
			break;
		case 2:
			if (!order)
				stable_sort(resultSpells.begin(), resultSpells.end(), SortSpellEntryByCategory);
			else
				stable_sort(resultSpells.begin(), resultSpells.end(), SortSpellEntryByCategoryReverse);
			break;
		}

		i = 0;
		page_book_count = 0;
		last_start_point = 0;
		vector<SpellBookEntry*>::iterator tmpItr;
		for (tmpItr = resultSpells.begin(); tmpItr != resultSpells.end(); tmpItr++) {
			((SpellBookEntry*)*tmpItr)->slot = i;
			GetSpellBookSlotSort(pattern, &i, &page_book_count, &last_start_point);
		}
	}
	
	MSpellsBook.unlock();
}

bool Player::SortSpellEntryByName(SpellBookEntry* s1, SpellBookEntry* s2)
{
	Spell* spell1 = master_spell_list.GetSpell(s1->spell_id, s1->tier);
	Spell* spell2 = master_spell_list.GetSpell(s2->spell_id, s2->tier);

	if (!spell1 || !spell2)
		return false;

	return (string(spell1->GetName()) < string(spell2->GetName()));
}

bool Player::SortSpellEntryByCategory(SpellBookEntry* s1, SpellBookEntry* s2)
{
	Spell* spell1 = master_spell_list.GetSpell(s1->spell_id, s1->tier);
	Spell* spell2 = master_spell_list.GetSpell(s2->spell_id, s2->tier);

	if (!spell1 || !spell2)
		return false;

	return (spell1->GetSpellIconBackdrop() < spell2->GetSpellIconBackdrop());
}

bool Player::SortSpellEntryByLevel(SpellBookEntry* s1, SpellBookEntry* s2)
{
	Spell* spell1 = master_spell_list.GetSpell(s1->spell_id, s1->tier);
	Spell* spell2 = master_spell_list.GetSpell(s2->spell_id, s2->tier);

	if (!spell1 || !spell2)
		return false;

	int16 lvl1 = spell1->GetLevelRequired(s1->player);
	int16 lvl2 = spell2->GetLevelRequired(s2->player);
	if (lvl1 == 0xFFFF)
		lvl1 = 0;
	if (lvl2 == 0xFFFF)
		lvl2 = 0;

	return (lvl1 < lvl2);
}

bool Player::SortSpellEntryByNameReverse(SpellBookEntry* s1, SpellBookEntry* s2)
{
	Spell* spell1 = master_spell_list.GetSpell(s1->spell_id, s1->tier);
	Spell* spell2 = master_spell_list.GetSpell(s2->spell_id, s2->tier);

	if (!spell1 || !spell2)
		return false;

	return (string(spell2->GetName()) < string(spell1->GetName()));
}

bool Player::SortSpellEntryByCategoryReverse(SpellBookEntry* s1, SpellBookEntry* s2)
{
	Spell* spell1 = master_spell_list.GetSpell(s1->spell_id, s1->tier);
	Spell* spell2 = master_spell_list.GetSpell(s2->spell_id, s2->tier);
	if (!spell1 || !spell2)
		return false;
	return (spell2->GetSpellIconBackdrop() < spell1->GetSpellIconBackdrop());
}

bool Player::SortSpellEntryByLevelReverse(SpellBookEntry* s1, SpellBookEntry* s2)
{
	Spell* spell1 = master_spell_list.GetSpell(s1->spell_id, s1->tier);
	Spell* spell2 = master_spell_list.GetSpell(s2->spell_id, s2->tier);

	if (!spell1 || !spell2)
		return false;

	int16 lvl1 = spell1->GetLevelRequired(s1->player);
	int16 lvl2 = spell2->GetLevelRequired(s2->player);
	if (lvl1 == 0xFFFF)
		lvl1 = 0;
	if (lvl2 == 0xFFFF)
		lvl2 = 0;

	return (lvl2 < lvl1);
}

int8 Player::GetSpellSlot(int32 spell_id){
	MSpellsBook.lock();
	vector<SpellBookEntry*>::iterator itr;
	SpellBookEntry* spell = 0;
	for(itr = spells.begin(); itr != spells.end(); itr++){
		spell = *itr;
		if(spell->spell_id == spell_id)
		{
			int8 slot = spell->slot;
			MSpellsBook.unlock();
			return slot;
		}
	}
	MSpellsBook.unlock();
	return 0;
}

void Player::AddSkill(int32 skill_id, int16 current_val, int16 max_val, bool save_needed){
	Skill* master_skill = master_skill_list.GetSkill(skill_id);
	if (master_skill) {
		Skill* skill = new Skill(master_skill);
		skill->current_val = current_val;
		skill->previous_val = current_val;
		skill->max_val = max_val;
		if (save_needed)
			skill->save_needed = true;
		skill_list.AddSkill(skill);
	}
}

void Player::RemovePlayerSkill(int32 skill_id, bool save) {
	Skill* skill = skill_list.GetSkill(skill_id);
	if (skill)
		RemoveSkillFromDB(skill, save);
}

void Player::RemoveSkillFromDB(Skill* skill, bool save) {
	skill_list.RemoveSkill(skill);
	if (save)
		database.DeleteCharacterSkill(GetCharacterID(), skill);
}

int16 Player::GetSpellSlotMappingCount(){
	int16 ret = 0;
	MSpellsBook.lock();
	for(int32 i=0;i<spells.size();i++){
		SpellBookEntry* spell = (SpellBookEntry*)spells[i];
		if(spell->slot >= 0 && spell->spell_id > 0 && spell->type != SPELL_BOOK_TYPE_NOT_SHOWN)
			ret++;
	}
	MSpellsBook.unlock();
	return ret;
}

int8 Player::GetSpellTier(int32 id){
	int8 ret = 0;
	MSpellsBook.lock();
	for(int32 i=0;i<spells.size();i++){
		SpellBookEntry* spell = (SpellBookEntry*)spells[i];
		if(spell->spell_id == id){
			ret = spell->tier;
			break;
		}
	}
	MSpellsBook.unlock();
	return ret;
}

int16 Player::GetSpellPacketCount(){
	int16 ret = 0;
	MSpellsBook.lock();
	for(int32 i=0;i<spells.size();i++){
		SpellBookEntry* spell = (SpellBookEntry*)spells[i];
		if(spell->spell_id > 0 && spell->type != SPELL_BOOK_TYPE_NOT_SHOWN)
			ret++;
	}
	MSpellsBook.unlock();
	return ret;
}

void Player::LockAllSpells() {
	vector<SpellBookEntry*>::iterator itr;

	MSpellsBook.writelock(__FUNCTION__, __LINE__);
	for (itr = spells.begin(); itr != spells.end(); itr++) {
		if ((*itr)->type != SPELL_BOOK_TYPE_TRADESKILL)
			RemoveSpellStatus((*itr), SPELL_STATUS_LOCK, false);
	}

	all_spells_locked = true;

	MSpellsBook.releasewritelock(__FUNCTION__, __LINE__);
}

void Player::UnlockAllSpells(bool modify_recast, Spell* exception) {
	vector<SpellBookEntry*>::iterator itr;
	int32 exception_spell_id = 0;
	if (exception)
		exception_spell_id = exception->GetSpellID();
	MSpellsBook.writelock(__FUNCTION__, __LINE__);
	for (itr = spells.begin(); itr != spells.end(); itr++) {
		MaintainedEffects* effect = 0;
		if((effect = GetMaintainedSpell((*itr)->spell_id)) && effect->spell->spell->GetSpellData()->duration_until_cancel)
			continue;

		if ((*itr)->in_use == false && 
			 (((*itr)->spell_id != exception_spell_id || 
			 (*itr)->timer > 0 && (*itr)->timer != exception->GetSpellData()->linked_timer)
		&& (*itr)->type != SPELL_BOOK_TYPE_TRADESKILL)) {
			AddSpellStatus((*itr), SPELL_STATUS_LOCK, modify_recast);
			(*itr)->recast_available = 0;
		}
		else if((*itr)->in_remiss)
		{
			AddSpellStatus((*itr), SPELL_STATUS_LOCK);
			(*itr)->recast_available = 0;
			(*itr)->in_remiss = false;
		}
	}

	all_spells_locked = false;

	MSpellsBook.releasewritelock(__FUNCTION__, __LINE__);
}

void Player::LockSpell(Spell* spell, int16 recast) {
	vector<SpellBookEntry*>::iterator itr;
	SpellBookEntry* spell2;
	
	MSpellsBook.writelock(__FUNCTION__, __LINE__);
	for (itr = spells.begin(); itr != spells.end(); itr++) {
		spell2 = *itr;
		if (spell2->spell_id == spell->GetSpellID() || (spell->GetSpellData()->linked_timer > 0 && spell->GetSpellData()->linked_timer == spell2->timer))
		{
			spell2->in_use = true;
			RemoveSpellStatus(spell2, SPELL_STATUS_LOCK, true, recast);
		}
		else if(spell2->in_use)
			RemoveSpellStatus(spell2, SPELL_STATUS_LOCK, false, 0);
	}
	MSpellsBook.releasewritelock(__FUNCTION__, __LINE__);
}

void Player::UnlockSpell(Spell* spell) {
	if (spell->GetStayLocked())
		return;
	vector<SpellBookEntry*>::iterator itr;
	SpellBookEntry* spell2;	
	MSpellsBook.writelock(__FUNCTION__, __LINE__);
	for (itr = spells.begin(); itr != spells.end(); itr++) {
		spell2 = *itr;
		if (spell2->spell_id == spell->GetSpellID() || (spell->GetSpellData() && spell->GetSpellData()->linked_timer > 0 && spell->GetSpellData()->linked_timer == spell2->timer))
		{
			spell2->in_use = false;
			spell2->recast_available = 0;
			if(all_spells_locked)
				spell2->in_remiss = true;
			else
				AddSpellStatus(spell2, SPELL_STATUS_LOCK, false);
		}
	}
	MSpellsBook.releasewritelock(__FUNCTION__, __LINE__);
}


void Player::UnlockSpell(int32 spell_id, int32 linked_timer_id) {
	vector<SpellBookEntry*>::iterator itr;
	SpellBookEntry* spell2;	
	MSpellsBook.writelock(__FUNCTION__, __LINE__);
	for (itr = spells.begin(); itr != spells.end(); itr++) {
		spell2 = *itr;
		if (spell2->spell_id == spell_id || (linked_timer_id > 0 && linked_timer_id == spell2->timer))
		{
			spell2->in_use = false;
			spell2->recast_available = 0;
			if(all_spells_locked)
				spell2->in_remiss = true;
			else
				AddSpellStatus(spell2, SPELL_STATUS_LOCK, false);
		}
	}
	MSpellsBook.releasewritelock(__FUNCTION__, __LINE__);
}

void Player::LockTSSpells() {
	vector<SpellBookEntry*>::iterator itr;

	MSpellsBook.writelock(__FUNCTION__, __LINE__);
	for (itr = spells.begin(); itr != spells.end(); itr++) {
		if ((*itr)->type == SPELL_BOOK_TYPE_TRADESKILL)
			RemoveSpellStatus(*itr, SPELL_STATUS_LOCK);
	}

	MSpellsBook.releasewritelock(__FUNCTION__, __LINE__);
	// Unlock all other types
	UnlockAllSpells();
}

void Player::UnlockTSSpells() {
	vector<SpellBookEntry*>::iterator itr;

	MSpellsBook.writelock(__FUNCTION__, __LINE__);
	for (itr = spells.begin(); itr != spells.end(); itr++) {
		if ((*itr)->type == SPELL_BOOK_TYPE_TRADESKILL)
			AddSpellStatus(*itr, SPELL_STATUS_LOCK);
	}

	MSpellsBook.releasewritelock(__FUNCTION__, __LINE__);
	// Lock all other types
	LockAllSpells();
}

void Player::QueueSpell(Spell* spell) {
	vector<SpellBookEntry*>::iterator itr;
	SpellBookEntry* spell2;
	MSpellsBook.writelock(__FUNCTION__, __LINE__);
	for (itr = spells.begin(); itr != spells.end(); itr++) {
		spell2 = *itr;
		if (spell2->spell_id == spell->GetSpellID())
			AddSpellStatus(spell2, SPELL_STATUS_QUEUE, false);
	}
	MSpellsBook.releasewritelock(__FUNCTION__, __LINE__);
}

void Player::UnQueueSpell(Spell* spell) {
	vector<SpellBookEntry*>::iterator itr;
	SpellBookEntry* spell2;
	MSpellsBook.writelock(__FUNCTION__, __LINE__);
	for (itr = spells.begin(); itr != spells.end(); itr++) {
		spell2 = *itr;
		if (spell2->spell_id == spell->GetSpellID())
			RemoveSpellStatus(spell2, SPELL_STATUS_QUEUE, false);
	}
	MSpellsBook.releasewritelock(__FUNCTION__, __LINE__);
}

vector<Spell*> Player::GetSpellBookSpellsByTimer(Spell* spell, int32 timerID) {
	vector<Spell*> ret;
	vector<SpellBookEntry*>::iterator itr;
	MSpellsBook.readlock(__FUNCTION__, __LINE__);
	for (itr = spells.begin(); itr != spells.end(); itr++) {
		if ((*itr)->timer == timerID && spell->GetSpellID() != (*itr)->spell_id)
			ret.push_back(master_spell_list.GetSpell((*itr)->spell_id, (*itr)->tier));
	}
	MSpellsBook.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

void Player::ModifySpellStatus(SpellBookEntry* spell, sint16 value, bool modify_recast, int16 recast) {
	SetSpellEntryRecast(spell, modify_recast, recast);
	if (modify_recast || spell->recast_available <= Timer::GetCurrentTime2() || value == 4) {
		spell->status += value; // use set/remove spell status now
	}
}

void Player::AddSpellStatus(SpellBookEntry* spell, sint16 value, bool modify_recast, int16 recast) {
	SetSpellEntryRecast(spell, modify_recast, recast);
	if (modify_recast || spell->recast_available <= Timer::GetCurrentTime2() || value == 4) {
		spell->status = spell->status | value;
	}
}

void Player::RemoveSpellStatus(SpellBookEntry* spell, sint16 value, bool modify_recast, int16 recast) {
	SetSpellEntryRecast(spell, modify_recast, recast);
	if (modify_recast || spell->recast_available <= Timer::GetCurrentTime2() || value == 4) {
		spell->status = spell->status & ~value;
	}
}

void Player::SetSpellStatus(Spell* spell, int8 status){
	MSpellsBook.lock();
	vector<SpellBookEntry*>::iterator itr;
	SpellBookEntry* spell2 = 0;
	for(itr = spells.begin(); itr != spells.end(); itr++){
		spell2 = *itr;
		if(spell2->spell_id == spell->GetSpellData()->id){
			spell2->status = spell2->status | status;
			break;
		}
	}
	MSpellsBook.unlock();
}

void Player::SetSpellEntryRecast(SpellBookEntry* spell, bool modify_recast, int16 recast) {
	if (modify_recast) {
		spell->recast = recast / 100;
		Spell* spell_ = master_spell_list.GetSpell(spell->spell_id, spell->tier);
		if(spell_) {
			float override_recast = 0.0f;
			if(recast > 0) {
				override_recast = static_cast<float>(recast);
			}
			int32 recast_time = spell_->CalculateRecastTimer(this, override_recast);
			
			spell->recast = recast_time / 100;
			spell->recast_available = Timer::GetCurrentTime2() + recast_time;
		}
		else {
			spell->recast_available = Timer::GetCurrentTime2() + recast;
		}
	}
}

vector<SpellBookEntry*>* Player::GetSpellsSaveNeeded(){
	vector<SpellBookEntry*>* ret = 0;
	vector<SpellBookEntry*>::iterator itr;
	MSpellsBook.lock();
	SpellBookEntry* spell = 0;
	for(itr = spells.begin(); itr != spells.end(); itr++){
		spell = *itr;
		if(spell->save_needed){
			if(!ret)
				ret = new vector<SpellBookEntry*>;
			ret->push_back(spell);
		}
	}
	MSpellsBook.unlock();
	return ret;
}

int16 Player::GetTierUp(int16 tier)
{
	switch(tier)
	{
		case 0:
			break;
		case 7:
		case 9:
			tier -= 2;
			break;
		default:
			tier -= 1;
		break;
	}

	return tier;
}
bool Player::HasSpell(int32 spell_id, int8 tier, bool include_higher_tiers, bool include_possible_scribe){
	bool ret = false;
	vector<SpellBookEntry*>::iterator itr;
	MSpellsBook.lock();
	SpellBookEntry* spell = 0;
	for(itr = spells.begin(); itr != spells.end(); itr++){
		spell = *itr;
		if(spell->spell_id == spell_id && (tier == 255 || spell->tier == tier || (include_higher_tiers && spell->tier > tier) || (include_possible_scribe && tier <= spell->tier))){
			ret = true;
			break;
		}
	}
	MSpellsBook.unlock();
	return ret;
}

sint32 Player::GetFreeSpellBookSlot(int32 type){
	sint32 ret = 0;
	MSpellsBook.lock();
	vector<SpellBookEntry*>::iterator itr;
	SpellBookEntry* spell = 0;
	for(itr = spells.begin(); itr != spells.end(); itr++){
		spell = *itr;
		if(spell->type == type && spell->slot > ret) //get last slot (add 1 to it on return)
			ret = spell->slot;
	}
	MSpellsBook.unlock();
	return ret+1;
}

SpellBookEntry* Player::GetSpellBookSpell(int32 spell_id){
	MSpellsBook.lock();
	vector<SpellBookEntry*>::iterator itr;
	SpellBookEntry* ret = 0;
	SpellBookEntry* spell = 0;
	for(itr = spells.begin(); itr != spells.end(); itr++){
		spell = *itr;
		if(spell->spell_id == spell_id){
			ret = spell;
			break;
		}
	}
	MSpellsBook.unlock();
	return ret;
}

vector<int32> Player::GetSpellBookSpellIDBySkill(int32 skill_id) {
	vector<int32> ret;

	MSpellsBook.readlock(__FUNCTION__, __LINE__);
	vector<SpellBookEntry*>::iterator itr;
	Spell* spell = 0;
	for(itr = spells.begin(); itr != spells.end(); itr++){
		spell = master_spell_list.GetSpell((*itr)->spell_id, (*itr)->tier);
		if(spell && spell->GetSpellData()->mastery_skill == skill_id)
			ret.push_back(spell->GetSpellData()->id);
	}	
	MSpellsBook.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}


EQ2Packet* Player::GetSpellSlotMappingPacket(int16 version){
	PacketStruct* packet = configReader.getStruct("WS_SpellSlotMapping", version);
	if(packet){
		int16 count = GetSpellSlotMappingCount();
		int16 ptr = 0;
		if(count > 0){
			packet->setArrayLengthByName("spell_count", count);
			MSpellsBook.lock();
			for(int32 i=0;i<spells.size();i++){
				SpellBookEntry* spell = (SpellBookEntry*)spells[i];
				if(spell->type == SPELL_BOOK_TYPE_NOT_SHOWN || spell->slot < 0 || spell->spell_id == 0)
					continue;
				packet->setArrayDataByName("spell_id", spell->spell_id, ptr);
				packet->setArrayDataByName("slot_id", (int16)spell->slot, ptr);
				ptr++;
			}
			MSpellsBook.unlock();
			EQ2Packet* ret = packet->serialize();
			safe_delete(packet);
			return ret;
		}
		safe_delete(packet);
	}
	return 0;
}

EQ2Packet* Player::GetSpellBookUpdatePacket(int16 version) {
    std::unique_lock lock(spell_packet_update_mutex);
	PacketStruct* packet = configReader.getStruct("WS_UpdateSpellBook", version);
	EQ2Packet* ret = 0;
	if (packet) {
		Spell* spell = 0;
		SpellBookEntry* spell_entry = 0;
		int16 count = GetSpellPacketCount();
		int16 ptr = 0;
		// Get the packet size
		PacketStruct* packet2 = configReader.getStruct("SubStruct_UpdateSpellBook", version);
		int32 total_bytes = packet2->GetTotalPacketSize();
		safe_delete(packet2);
		packet->setArrayLengthByName("spell_count", count);
		
		LogWrite(PLAYER__DEBUG, 5, "Player", "%s: GetSpellBookUpdatePacket Spell Count: %u, Spell Entry Book Size: %u", GetName(), count, total_bytes);

		if (count > 0) {
			if (count > spell_count) {
				uchar* tmp = 0;
				if (spell_orig_packet) {
					tmp = new uchar[count * total_bytes];
					memset(tmp, 0, total_bytes * count);
					memcpy(tmp, spell_orig_packet, spell_count * total_bytes);
					safe_delete_array(spell_orig_packet);
					safe_delete_array(spell_xor_packet);
					spell_orig_packet = tmp;
				}
				else {
					spell_orig_packet = new uchar[count * total_bytes];
					memset(spell_orig_packet, 0, total_bytes * count);
				}
				spell_xor_packet = new uchar[count * total_bytes];
				memset(spell_xor_packet, 0, count * total_bytes);
			}
			spell_count = count;
			MSpellsBook.lock();
			for (int32 i = 0; i < spells.size(); i++) {
				spell_entry = (SpellBookEntry*)spells[i];
				if (spell_entry->spell_id == 0 || spell_entry->type == SPELL_BOOK_TYPE_NOT_SHOWN)
					continue;
				spell = master_spell_list.GetSpell(spell_entry->spell_id, spell_entry->tier);
				if (spell) {			
					if (spell_entry->recast_available == 0 || Timer::GetCurrentTime2() > spell_entry->recast_available) {
						packet->setSubstructArrayDataByName("spells", "available", 1, 0, ptr);
					}
					LogWrite(PLAYER__DEBUG, 9, "Player", "%s: GetSpellBookUpdatePacket Send Spell %u in position %u\n",GetName(), spell_entry->spell_id, ptr);
					packet->setSubstructArrayDataByName("spells", "spell_id", spell_entry->spell_id, 0, ptr);
					packet->setSubstructArrayDataByName("spells", "type", spell_entry->type, 0, ptr);
					packet->setSubstructArrayDataByName("spells", "recast_available", spell_entry->recast_available, 0, ptr);
					packet->setSubstructArrayDataByName("spells", "recast_time", spell_entry->recast, 0, ptr);
					packet->setSubstructArrayDataByName("spells", "status", spell_entry->status, 0, ptr);
					packet->setSubstructArrayDataByName("spells", "icon", (spell->TranslateClientSpellIcon(version) * -1) - 1, 0, ptr);
					packet->setSubstructArrayDataByName("spells", "icon_type", spell->GetSpellIconBackdrop(), 0, ptr);
					packet->setSubstructArrayDataByName("spells", "icon2", spell->GetSpellIconHeroicOp(), 0, ptr);
					packet->setSubstructArrayDataByName("spells", "unique_id", (spell_entry->tier + 1) * -1, 0, ptr); //this is actually GetSpellNameCrc(spell->GetName()), but hijacking it for spell tier
					packet->setSubstructArrayDataByName("spells", "charges", 255, 0, ptr);
					// Beastlord and Channeler spell support
					if (spell->GetSpellData()->savage_bar == 1)
						packet->setSubstructArrayDataByName("spells", "unknown6", 32, 0, ptr); // advantages
					else if (spell->GetSpellData()->savage_bar == 2)
						packet->setSubstructArrayDataByName("spells", "unknown6", 64, 0, ptr); // primal
					else if (spell->GetSpellData()->savage_bar == 3) {
						packet->setSubstructArrayDataByName("spells", "unknown6", 6, 1, ptr); // 6 = channeler
						// Slot req for channelers
						// bitmask for slots 1 = slot 1, 2 = slot 2, 4 = slot 3, 8 = slot 4, 16 = slot 5, 32 = slot 6, 64 = slot 7, 128 = slot 8
						packet->setSubstructArrayDataByName("spells", "savage_bar_slot", spell->GetSpellData()->savage_bar_slot, 0, ptr);
					}

					ptr++;
				}
			}
			MSpellsBook.unlock();
		}
		ret = packet->serializeCountPacket(version, 0, spell_orig_packet, spell_xor_packet);
		//packet->PrintPacket();
		//DumpPacket(ret);
		safe_delete(packet);
	}
	return ret;
}
EQ2Packet* Player::GetRaidUpdatePacket(int16 version) {
    std::unique_lock lock(raid_update_mutex);
	
	std::vector<int32> raidGroups;
	PacketStruct* packet = configReader.getStruct("WS_RaidUpdate", version);
	EQ2Packet* ret = 0;
	Entity* member = 0;
	int8 det_count = 0;
	int8 total_groups = 0;
	if (packet) {
		int16 ptr = 0;
		// Get the packet size
		PacketStruct* packet2 = configReader.getStruct("Substruct_RaidMember", version);
		int32 total_bytes = packet2->GetTotalPacketSize();
		safe_delete(packet2);
			world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);
			if (GetGroupMemberInfo()) {
				PlayerGroup* group = world.GetGroupManager()->GetGroup(GetGroupMemberInfo()->group_id);
				if (group)
				{
					group->GetRaidGroups(&raidGroups);
					std::vector<int32>::iterator raid_itr;
					int32 group_pos = 0;
					for(raid_itr = raidGroups.begin(); raid_itr != raidGroups.end(); raid_itr++) {
						group = world.GetGroupManager()->GetGroup((*raid_itr));
						if(!group)
							continue;
					total_groups++;
					group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
					deque<GroupMemberInfo*>* members = group->GetMembers();
					deque<GroupMemberInfo*>::iterator itr;
					GroupMemberInfo* info = 0;
					int x = 1;
					int lastpos = 1;
					bool gotleader = false;
					for (itr = members->begin(); itr != members->end(); itr++) {
						info = *itr;
						
						if(!info)
							continue;
						
						member = info->member;
						
						std::string prop_name("group_member");
						if(!gotleader && info->leader) {
							lastpos = x;
							x = 0;
							gotleader = true;
						}
						else if(lastpos) {
							x = lastpos;
							lastpos = 0;
						}
						prop_name.append(std::to_string(x) + "_" + std::to_string(group_pos));
						x++;
						if (member && member->GetZone() == GetZone()) {
							packet->setSubstructDataByName(prop_name.c_str(), "spawn_id", GetIDWithPlayerSpawn(member), 0);

							if (member->HasPet()) {
								if (member->GetPet())
									packet->setSubstructDataByName(prop_name.c_str(), "pet_id", GetIDWithPlayerSpawn(member->GetPet()), 0);
								else
									packet->setSubstructDataByName(prop_name.c_str(), "pet_id", GetIDWithPlayerSpawn(member->GetCharmedPet()), 0);
							}
							else
								packet->setSubstructDataByName(prop_name.c_str(), "pet_id", 0xFFFFFFFF, 0);

							//Send detriment counts as 255 if all dets of that type are incurable
							det_count = member->GetTraumaCount();
							if (det_count > 0) {
								if (!member->HasCurableDetrimentType(DET_TYPE_TRAUMA))
									det_count = 255;
							}
							packet->setSubstructDataByName(prop_name.c_str(), "trauma_count", det_count, 0);

							det_count = member->GetArcaneCount();
							if (det_count > 0) {
								if (!member->HasCurableDetrimentType(DET_TYPE_ARCANE))
									det_count = 255;
							}
							packet->setSubstructDataByName(prop_name.c_str(), "arcane_count", det_count, 0);

							det_count = member->GetNoxiousCount();
							if (det_count > 0) {
								if (!member->HasCurableDetrimentType(DET_TYPE_NOXIOUS))
									det_count = 255;
							}
							packet->setSubstructDataByName(prop_name.c_str(), "noxious_count", det_count, 0);

							det_count = member->GetElementalCount();
							if (det_count > 0) {
								if (!member->HasCurableDetrimentType(DET_TYPE_ELEMENTAL))
									det_count = 255;
							}
							packet->setSubstructDataByName(prop_name.c_str(), "elemental_count", det_count, 0);

							det_count = member->GetCurseCount();
							if (det_count > 0) {
								if (!member->HasCurableDetrimentType(DET_TYPE_CURSE))
									det_count = 255;
							}
							packet->setSubstructDataByName(prop_name.c_str(), "curse_count", det_count, 0);

							packet->setSubstructDataByName(prop_name.c_str(), "zone_status", 1, 0);
						}
						else {
							packet->setSubstructDataByName(prop_name.c_str(), "pet_id", 0xFFFFFFFF, 0);
							//packet->setSubstructDataByName(prop_name.c_str(), "unknown5", 1, 0, 1); // unknown5 > 1 = name is blue
							packet->setSubstructDataByName(prop_name.c_str(), "zone_status", 2, 0);
						}

						packet->setSubstructDataByName(prop_name.c_str(), "name", info->name.c_str(), 0);
						packet->setSubstructDataByName(prop_name.c_str(), "hp_current", info->hp_current, 0);
						packet->setSubstructDataByName(prop_name.c_str(), "hp_max", info->hp_max, 0);
						packet->setSubstructDataByName(prop_name.c_str(), "hp_current2", info->hp_current, 0);
						packet->setSubstructDataByName(prop_name.c_str(), "power_current", info->power_current, 0);
						packet->setSubstructDataByName(prop_name.c_str(), "power_max", info->power_max, 0);
						packet->setSubstructDataByName(prop_name.c_str(), "level_current", info->level_current, 0);
						packet->setSubstructDataByName(prop_name.c_str(), "level_max", info->level_max, 0);
						packet->setSubstructDataByName(prop_name.c_str(), "zone", info->zone.c_str(), 0);
						packet->setSubstructDataByName(prop_name.c_str(), "race_id", info->race_id, 0);
						packet->setSubstructDataByName(prop_name.c_str(), "class_id", info->class_id, 0);
					}
					
					group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
					group_pos += 1;
				}
		}
	}
		world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
		//packet->PrintPacket();

		hassent_raid = true;
		string* data = packet->serializeString();
		int32 size = data->length();
		
		uchar* tmp = new uchar[size];
		if(!raid_xor_packet){
			raid_orig_packet = new uchar[size];
			raid_xor_packet = new uchar[size];
			memcpy(raid_orig_packet, (uchar*)data->c_str(), size);
			size = Pack(tmp, (uchar*)data->c_str(), size, size, version);
		}
		else{
			memcpy(raid_xor_packet, (uchar*)data->c_str(), size);
			Encode(raid_xor_packet, raid_orig_packet, size);
			size = Pack(tmp, raid_xor_packet, size, size, version);
		}
		
		ret = new EQ2Packet(OP_UpdateRaidMsg, tmp, size);
		safe_delete_array(tmp);
		safe_delete(packet);
		//DumpPacket(ret);
	}
	return ret;
}

PlayerInfo::~PlayerInfo(){
	RemoveOldPackets();
}

PlayerInfo::PlayerInfo(Player* in_player){
	orig_packet = 0;
	changes = 0;
	pet_orig_packet = 0;
	pet_changes = 0;
	player = in_player;
	info_struct = player->GetInfoStruct();
	info_struct->set_name(std::string(player->GetName()));
	info_struct->set_deity(std::string("None"));

	info_struct->set_class1(classes.GetBaseClass(player->GetAdventureClass()));
	info_struct->set_class2(classes.GetSecondaryBaseClass(player->GetAdventureClass()));
	info_struct->set_class3(player->GetAdventureClass());

	info_struct->set_race(player->GetRace());
	info_struct->set_gender(player->GetGender());
	info_struct->set_level(player->GetLevel());
	info_struct->set_tradeskill_level(player->GetTSLevel());
	info_struct->set_tradeskill_class1(classes.GetTSBaseClass(player->GetTradeskillClass()));
	info_struct->set_tradeskill_class2(classes.GetSecondaryTSBaseClass(player->GetTradeskillClass()));
	info_struct->set_tradeskill_class3(player->GetTradeskillClass());

	for(int i=0;i<45;i++){
		if(i<30){
			info_struct->maintained_effects[i].spell_id = 0xFFFFFFFF;
			info_struct->maintained_effects[i].icon = 0xFFFF;
			info_struct->maintained_effects[i].spell = nullptr;
		}
		info_struct->spell_effects[i].spell_id = 0xFFFFFFFF;	
		info_struct->spell_effects[i].icon = 0;		
		info_struct->spell_effects[i].icon_backdrop = 0;
		info_struct->spell_effects[i].tier = 0;
		info_struct->spell_effects[i].total_time = 0.0f;
		info_struct->spell_effects[i].expire_timestamp = 0;
		info_struct->spell_effects[i].spell = nullptr;	
	}
	
	house_zone_id = 0;
	bind_zone_id = 0;
	bind_x = 0;
	bind_y = 0;
	bind_z = 0;
	bind_heading = 0;
	boat_x_offset = 0;
	boat_y_offset = 0;
	boat_z_offset = 0;
	boat_spawn = 0;
}

MaintainedEffects* Player::GetFreeMaintainedSpellSlot(){
	MaintainedEffects* ret = 0;
	InfoStruct* info = GetInfoStruct();
	GetMaintainedMutex()->readlock(__FUNCTION__, __LINE__);
	for(int i=0;i<NUM_MAINTAINED_EFFECTS;i++){
		if(info->maintained_effects[i].spell_id == 0xFFFFFFFF){
			ret = &info->maintained_effects[i];
			ret->spell_id = 0;
			ret->slot_pos = i;
			break;
		}
	}
	GetMaintainedMutex()->releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

MaintainedEffects* Player::GetMaintainedSpell(int32 id){
	MaintainedEffects* ret = 0;
	InfoStruct* info = GetInfoStruct();
	GetMaintainedMutex()->readlock(__FUNCTION__, __LINE__);
	for(int i=0;i<NUM_MAINTAINED_EFFECTS;i++){
		if(info->maintained_effects[i].spell_id == id){
			ret = &info->maintained_effects[i];
			break;
		}
	}
	GetMaintainedMutex()->releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

MaintainedEffects* Player::GetMaintainedSpellBySlot(int8 slot){
	MaintainedEffects* ret = 0;
	InfoStruct* info = GetInfoStruct();
	GetMaintainedMutex()->readlock(__FUNCTION__, __LINE__);
	for(int i=0;i<NUM_MAINTAINED_EFFECTS;i++){
		if(info->maintained_effects[i].slot_pos == slot){
			ret = &info->maintained_effects[i];
			break;
		}
	}
	GetMaintainedMutex()->releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

MaintainedEffects* Player::GetMaintainedSpells() {
	return GetInfoStruct()->maintained_effects;
}

SpellEffects* Player::GetFreeSpellEffectSlot(){
	SpellEffects* ret = 0;
	InfoStruct* info = GetInfoStruct();
	GetSpellEffectMutex()->readlock(__FUNCTION__, __LINE__);
	for(int i=0;i<45;i++){
		if(info->spell_effects[i].spell_id == 0xFFFFFFFF){
			ret = &info->spell_effects[i];
			ret->spell_id = 0;
			break;
		}
	}
	GetSpellEffectMutex()->releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

SpellEffects* Player::GetSpellEffects() {
	return GetInfoStruct()->spell_effects;
}

// call inside info_mutex
void Player::ClearRemovalTimers(){
	map<int32, SpawnQueueState*>::iterator itr;
	for(itr = spawn_state_list.begin(); itr != spawn_state_list.end();) {
		SpawnQueueState* sr = itr->second;
		itr = spawn_state_list.erase(itr);
		safe_delete(sr);
	}
}

void Player::ClearEverything(){
	index_mutex.writelock(__FUNCTION__, __LINE__);
	player_spawn_id_map.clear();
	player_spawn_reverse_id_map.clear();
	index_mutex.releasewritelock(__FUNCTION__, __LINE__);
	map<int32, vector<int32>*>::iterator itr;
	m_playerSpawnQuestsRequired.writelock(__FUNCTION__, __LINE__);
	for (itr = player_spawn_quests_required.begin(); itr != player_spawn_quests_required.end(); itr++){
		safe_delete(itr->second);
	}
	player_spawn_quests_required.clear();
	m_playerSpawnQuestsRequired.releasewritelock(__FUNCTION__, __LINE__);

	m_playerSpawnHistoryRequired.writelock(__FUNCTION__, __LINE__);
	for (itr = player_spawn_history_required.begin(); itr != player_spawn_history_required.end(); itr++){
		safe_delete(itr->second);
	}
	player_spawn_history_required.clear();
	m_playerSpawnHistoryRequired.releasewritelock(__FUNCTION__, __LINE__);

	spawn_mutex.writelock(__FUNCTION__, __LINE__);
	ClearRemovalTimers();
	spawn_packet_sent.clear();
	spawn_mutex.releasewritelock(__FUNCTION__, __LINE__);

	info_mutex.writelock(__FUNCTION__, __LINE__);
	spawn_info_packet_list.clear();
	info_mutex.releasewritelock(__FUNCTION__, __LINE__);

	vis_mutex.writelock(__FUNCTION__, __LINE__);
	spawn_vis_packet_list.clear();
	vis_mutex.releasewritelock(__FUNCTION__, __LINE__);

	pos_mutex.writelock(__FUNCTION__, __LINE__);
	spawn_pos_packet_list.clear();
	pos_mutex.releasewritelock(__FUNCTION__, __LINE__);
}
bool Player::IsResurrecting(){
	return resurrecting;
}
void Player::SetResurrecting(bool val){
	resurrecting = val;
}
void Player::AddMaintainedSpell(LuaSpell* luaspell){
	if(!luaspell)
		return;

	Spell* spell = luaspell->spell;
	MaintainedEffects* effect = GetFreeMaintainedSpellSlot();
	int32 target_type = 0;
	Spawn* spawn = 0;

	if(effect && luaspell->caster && luaspell->caster->GetZone()){
		GetMaintainedMutex()->writelock(__FUNCTION__, __LINE__);
		strcpy(effect->name, spell->GetSpellData()->name.data.c_str());
		effect->target = luaspell->initial_target;

		spawn = luaspell->caster->GetZone()->GetSpawnByID(luaspell->initial_target);
		if (spawn){
			if (spawn == this)
				target_type = 0;
			else if (GetPet() == spawn || GetCharmedPet() == spawn)
				target_type = 1;
			else
				target_type = 2;
		}
		effect->target_type = target_type;

		effect->spell = luaspell;
		if(!luaspell->slot_pos)
			luaspell->slot_pos = effect->slot_pos;
		effect->spell_id = spell->GetSpellData()->id;
		LogWrite(PLAYER__DEBUG, 5, "Player", "AddMaintainedSpell Spell ID: %u, req concentration: %u", spell->GetSpellData()->id, spell->GetSpellData()->req_concentration);
		effect->icon = spell->GetSpellData()->icon;
		effect->icon_backdrop = spell->GetSpellData()->icon_backdrop;
		effect->conc_used = spell->GetSpellData()->req_concentration;
		effect->total_time = spell->GetSpellDuration()/10;
		effect->tier = spell->GetSpellData()->tier;
		if (spell->GetSpellData()->duration_until_cancel)
			effect->expire_timestamp = 0xFFFFFFFF;
		else
			effect->expire_timestamp = Timer::GetCurrentTime2() + (spell->GetSpellDuration()*100);
		GetMaintainedMutex()->releasewritelock(__FUNCTION__, __LINE__);
		charsheet_changed = true;
	}
}
void Player::AddSpellEffect(LuaSpell* luaspell, int32 override_expire_time){
	if(!luaspell || !luaspell->caster)
		return;

	Spell* spell = luaspell->spell;
	SpellEffects* old_effect = GetSpellEffect(spell->GetSpellID(), luaspell->caster);
	SpellEffects* effect = 0;
	if (old_effect){
		GetZone()->RemoveTargetFromSpell(old_effect->spell, this);
		RemoveSpellEffect(old_effect->spell);
	}
	
	LogWrite(SPELL__DEBUG, 0, "Spell", "%s AddSpellEffect %s (%u).", spell->GetName(), GetName(), GetID());
	
	effect = GetFreeSpellEffectSlot();

	if(effect){
		GetSpellEffectMutex()->writelock(__FUNCTION__, __LINE__);
		effect->spell = luaspell;
		effect->spell_id = spell->GetSpellData()->id;
		effect->caster = luaspell->caster;
		effect->total_time = spell->GetSpellDuration()/10;
		if (spell->GetSpellData()->duration_until_cancel)
			effect->expire_timestamp = 0xFFFFFFFF;
		else if(override_expire_time)
			effect->expire_timestamp = Timer::GetCurrentTime2() + override_expire_time;
		else
			effect->expire_timestamp = Timer::GetCurrentTime2() + (spell->GetSpellDuration()*100);
		effect->icon = spell->GetSpellData()->icon;
		effect->icon_backdrop = spell->GetSpellData()->icon_backdrop;
		effect->tier = spell->GetSpellTier();
		GetSpellEffectMutex()->releasewritelock(__FUNCTION__, __LINE__);
		charsheet_changed = true;

		if(luaspell->caster && luaspell->caster->IsPlayer() && luaspell->caster != this)
		{
			if(GetClient()) {
				GetClient()->TriggerSpellSave();
			}
			if(((Player*)luaspell->caster)->GetClient()) {
				((Player*)luaspell->caster)->GetClient()->TriggerSpellSave();
			}
		}
	}	
}

void Player::RemoveMaintainedSpell(LuaSpell* luaspell){
	if(!luaspell)
		return;

	bool found = false;
	Client* client = GetClient();
	LuaSpell* old_spell = 0;
	LuaSpell* current_spell = 0;
	GetMaintainedMutex()->writelock(__FUNCTION__, __LINE__);
	for(int i=0;i<30;i++){
		// If we already found the spell then we are bumping all other up one so there are no gaps in the ui
		// This check needs to be first so found can never be true on the first iteration (i = 0)
		if (found) {
			old_spell = GetInfoStruct()->maintained_effects[i - 1].spell;
			current_spell = GetInfoStruct()->maintained_effects[i].spell;

		    //Update the maintained window uses_remaining and damage_remaining values
			if (current_spell && current_spell->num_triggers > 0)
				ClientPacketFunctions::SendMaintainedExamineUpdate(client, i - 1, current_spell->num_triggers, 0);
			else if (current_spell && current_spell->damage_remaining > 0)
				ClientPacketFunctions::SendMaintainedExamineUpdate(client, i - 1, current_spell->damage_remaining, 1);
			else if (old_spell && old_spell->had_triggers)
				ClientPacketFunctions::SendMaintainedExamineUpdate(client, i - 1, 0, 0);
			else if (old_spell && old_spell->had_dmg_remaining)
				ClientPacketFunctions::SendMaintainedExamineUpdate(client, i - 1, 0, 1);


			GetInfoStruct()->maintained_effects[i].slot_pos = i - 1;
			GetInfoStruct()->maintained_effects[i - 1] = GetInfoStruct()->maintained_effects[i];
			if (current_spell)
				current_spell->slot_pos = i - 1;
		}
		// Compare spells, if we found a match set the found flag
		if(GetInfoStruct()->maintained_effects[i].spell == luaspell)
			found = true;
	}
	// if we found the spell in the array then we need to flag the char sheet as changed and set the last element to empty
	if (found) {
		memset(&GetInfoStruct()->maintained_effects[29], 0, sizeof(MaintainedEffects));
		GetInfoStruct()->maintained_effects[29].spell_id = 0xFFFFFFFF;
		GetInfoStruct()->maintained_effects[29].icon = 0xFFFF;
		GetInfoStruct()->maintained_effects[29].spell = nullptr;
		charsheet_changed = true;
	}
	GetMaintainedMutex()->releasewritelock(__FUNCTION__, __LINE__);
}

void Player::RemoveSpellEffect(LuaSpell* spell){
	bool found = false;
	GetSpellEffectMutex()->writelock(__FUNCTION__, __LINE__);
	for(int i=0;i<45;i++){
		if (found) {
			GetInfoStruct()->spell_effects[i-1] = GetInfoStruct()->spell_effects[i];
		}
		if(GetInfoStruct()->spell_effects[i].spell == spell)
			found = true;
	}
	if (found) {
		memset(&GetInfoStruct()->spell_effects[44], 0, sizeof(SpellEffects));
		GetInfoStruct()->spell_effects[44].spell_id = 0xFFFFFFFF;
		GetInfoStruct()->spell_effects[44].spell = nullptr;
		changed = true;
		info_changed = true;
		AddChangedZoneSpawn();
		charsheet_changed = true;
	}
	GetSpellEffectMutex()->releasewritelock(__FUNCTION__, __LINE__);
}

void Player::PrepareIncomingMovementPacket(int32 len, uchar* data, int16 version, bool dead_window_sent)
{
	if((GetClient() && GetClient()->IsReloadingZone()) || dead_window_sent)
		return;

	LogWrite(PLAYER__DEBUG, 7, "Player", "Enter: %s", __FUNCTION__); // trace

	// XML structs may be to slow to use in this portion of the code as a single
	// client sends a LOT of these packets when they are moving.  I have commented
	// out all the code for xml structs, to switch to it just uncomment
	// the code and comment the 2 if/else if/else blocks, both have a comment
	// above them to let you know wich ones they are.

	//PacketStruct* update = configReader.getStruct("WS_PlayerPosUpdate", version);
	int16 total_bytes;		// = update->GetTotalPacketSize();

	// Comment out this if/else if/else block if you switch to xml structs
	if (version >= 1144)
		total_bytes = sizeof(Player_Update1144);
	else if (version >= 1096)
		total_bytes = sizeof(Player_Update1096);
	else if (version <= 373)
		total_bytes = sizeof(Player_Update283);
	else
		total_bytes = sizeof(Player_Update);

	if (!movement_packet)
		movement_packet = new uchar[total_bytes];
	else if (!old_movement_packet)
		old_movement_packet = new uchar[total_bytes];
	if (movement_packet && old_movement_packet)
		memcpy(old_movement_packet, movement_packet, total_bytes);
	bool reverse = version > 373;
	Unpack(len, data, movement_packet, total_bytes, 0, reverse);
	if (!movement_packet || !old_movement_packet)
		return;
	Decode(movement_packet, old_movement_packet, total_bytes);

	//update->LoadPacketData(movement_packet, total_bytes);

	int32 activity;		// = update->getType_int32_ByName("activity");
	int32 grid_id;		// = update->getType_int32_ByName("grid_location");
	float direction1;	// = update->getType_float_ByName("direction1");
	float direction2;	// = update->getType_float_ByName("direction2");;
	float speed;		// = update->getType_float_ByName("speed");;
	float side_speed;
	float vert_speed;
	float x;			// = update->getType_float_ByName("x");;
	float y;			// = update->getType_float_ByName("y");;
	float z;			// = update->getType_float_ByName("z");;
	float x_speed;
	float y_speed;
	float z_speed;
	float client_pitch;

	// comment out this if/else if/else block if you use xml structs
	if (version >= 1144) {
		Player_Update1144* update = (Player_Update1144*)movement_packet;
		activity = update->activity;
		grid_id = update->grid_location;
		direction1 = update->direction1;
		direction2 = update->direction2;
		speed = update->speed;
		side_speed = update->side_speed;
		vert_speed = update->vert_speed;
		x = update->x;
		y = update->y;
		z = update->z;
		x_speed = update->speed_x;
		y_speed = update->speed_y;
		z_speed = update->speed_z;
		client_pitch = update->pitch;
		
		SetPitch(180 + update->pitch);
	}
	else if (version >= 1096) {
		Player_Update1096* update = (Player_Update1096*)movement_packet;
		activity = update->activity;
		grid_id = update->grid_location;
		direction1 = update->direction1;
		direction2 = update->direction2;
		speed = update->speed;
		side_speed = update->side_speed;
		vert_speed = update->vert_speed;
		x = update->x;
		y = update->y;
		z = update->z;
		x_speed = update->speed_x;
		y_speed = update->speed_y;
		z_speed = update->speed_z;
		client_pitch = update->pitch;
		
		SetPitch(180 + update->pitch);
	}
	else if (version <= 373) {
		Player_Update283* update = (Player_Update283*)movement_packet;
		activity = update->activity;
		grid_id = update->grid_location;
		direction1 = update->direction1;
		direction2 = update->direction2;
		speed = update->speed;
		side_speed = update->side_speed;
		vert_speed = update->vert_speed;
		client_pitch = update->pitch;
		
		x = update->x;
		y = update->y;
		z = update->z;
		x_speed = update->speed_x;
		y_speed = update->speed_y;
		z_speed = update->speed_z;
		appearance.pos.X2 = update->orig_x;
		appearance.pos.Y2 = update->orig_y;
		appearance.pos.Z2 = update->orig_z;
		appearance.pos.X3 = update->orig_x2;
		appearance.pos.Y3 = update->orig_y2;
		appearance.pos.Z3 = update->orig_z2;
		if (update->pitch != 0)
			SetPitch(180 + update->pitch);
	}
	else {
		Player_Update* update = (Player_Update*)movement_packet;
		activity = update->activity;
		grid_id = update->grid_location;
		direction1 = update->direction1;
		direction2 = update->direction2;
		speed = update->speed;
		side_speed = update->side_speed;
		vert_speed = update->vert_speed;
		x = update->x;
		y = update->y;
		z = update->z;
		x_speed = update->speed_x;
		y_speed = update->speed_y;
		z_speed = update->speed_z;
		appearance.pos.X2 = update->orig_x;
		appearance.pos.Y2 = update->orig_y;
		appearance.pos.Z2 = update->orig_z;
		appearance.pos.X3 = update->orig_x2;
		appearance.pos.Y3 = update->orig_y2;
		appearance.pos.Z3 = update->orig_z2;
		client_pitch = update->pitch;
		
		SetPitch(180 + update->pitch);
	}
	
	SetHeading((sint16)(direction1 * 64), (sint16)(direction2 * 64));
	
	if (activity != last_movement_activity) {
		switch(activity) {
			case UPDATE_ACTIVITY_RUNNING:
			case UPDATE_ACTIVITY_RUNNING_AOM:
			case UPDATE_ACTIVITY_IN_WATER_ABOVE:
			case UPDATE_ACTIVITY_IN_WATER_BELOW:
			case UPDATE_ACTIVITY_MOVE_WATER_ABOVE_AOM:
			case UPDATE_ACTIVITY_MOVE_WATER_BELOW_AOM: {
				if(GetZone() && GetZone()->GetDrowningVictim(this))
					GetZone()->RemoveDrowningVictim(this);
				
				break;
			}
			case UPDATE_ACTIVITY_DROWNING:
			case UPDATE_ACTIVITY_DROWNING2:
			case UPDATE_ACTIVITY_DROWNING_AOM:
			case UPDATE_ACTIVITY_DROWNING2_AOM: {
				if(GetZone() && !GetInvulnerable()) {
					GetZone()->AddDrowningVictim(this);
				}
				break;
			}
			case UPDATE_ACTIVITY_JUMPING:
			case UPDATE_ACTIVITY_JUMPING_AOM:
			case UPDATE_ACTIVITY_FALLING:
			case UPDATE_ACTIVITY_FALLING_AOM: {
				if(IsCasting()) {
					GetZone()->Interrupted(this, 0, SPELL_ERROR_INTERRUPTED, false, true);
				}
				if(GetInitialState() != 1024) {
					SetInitialState(1024);
				}
				else if(GetInitialState() == 1024) {
					if(activity == UPDATE_ACTIVITY_JUMPING_AOM) {
						SetInitialState(UPDATE_ACTIVITY_JUMPING_AOM);
					}
					else {
						SetInitialState(16512);
					}
				}
				break;
			}
		}
		
		last_movement_activity = activity;
	}
	//Player is riding a lift, update lift XYZ offsets and the lift's spawn pointer
	if (activity & UPDATE_ACTIVITY_RIDING_BOAT) {
		Spawn* boat = 0;

		float boat_x = x;
		float boat_y = y;
		float boat_z = z;

		if (GetBoatSpawn() == 0 && GetZone()) {
			boat = GetZone()->GetClosestTransportSpawn(GetX(), GetY(), GetZ());
			SetBoatSpawn(boat);
			if(boat)
			{
				LogWrite(PLAYER__DEBUG, 0, "Player", "Set Player %s (%u) on Boat: %s", 
					GetName(), GetCharacterID(), boat ? boat->GetName() : "notset");
				boat->AddRailPassenger(GetCharacterID());
				GetZone()->CallSpawnScript(boat, SPAWN_SCRIPT_BOARD, this);
			}
		}

		if (boat || (GetBoatSpawn() && GetZone())) {
			if (!boat)
				boat = GetZone()->GetSpawnByID(GetBoatSpawn());

			if (boat && boat->IsWidget() && ((Widget*)boat)->GetMultiFloorLift()) {
				boat_x -= boat->GetX();
				boat_y -= boat->GetY();
				boat_z -= boat->GetZ();
			}
		}

		SetBoatX(boat_x);
		SetBoatY(boat_y);
		SetBoatZ(boat_z);
		pos_packet_speed = speed;
		grid_id = GetLocation();
	}
	else if (GetBoatSpawn() > 0 && !lift_cooldown.Enabled())
	{
		lift_cooldown.Start(100, true);
	}
	else if(lift_cooldown.Check())
	{
		if(GetBoatSpawn())
		{
			Spawn* boat = GetZone()->GetSpawnByID(GetBoatSpawn());
			if(boat)
			{
				LogWrite(PLAYER__DEBUG, 0, "Player", "Remove Player %s (%u) from Boat: %s", 
					GetName(), GetCharacterID(), boat ? boat->GetName() : "notset");
				boat->RemoveRailPassenger(GetCharacterID());
				GetZone()->CallSpawnScript(boat, SPAWN_SCRIPT_DEBOARD, this);
			}
		}
		SetBoatSpawn(0);
		lift_cooldown.Disable();
	}

	if (!IsResurrecting() && !GetBoatSpawn())
	{
		if (!IsRooted() && !IsMezzedOrStunned()) {
			SetX(x);
			SetY(y, true, true);
			SetZ(z);
			SetSpeedX(x_speed);
			SetSpeedY(y_speed);
			SetSpeedZ(z_speed);
			SetSideSpeed(side_speed);
			SetVertSpeed(vert_speed);
			SetClientHeading1(direction1);
			SetClientHeading2(direction2);
			SetClientPitch(client_pitch);
			if(version > 373) {
				pos_packet_speed = speed;
			}
		}
		else {
			SetSpeedX(0.0f);
			SetSpeedY(0.0f);
			SetSpeedZ(0.0f);
			SetSideSpeed(0.0f);
			SetVertSpeed(0.0f);
			SetClientHeading1(direction1);
			SetClientHeading2(direction2);
			SetClientPitch(client_pitch);
			pos_packet_speed = 0;
		}
	}

	if (GetLocation() != grid_id)
	{
		LogWrite(PLAYER__DEBUG, 0, "Player", "%s left grid %u and entered grid %u", appearance.name, GetLocation(), grid_id);
		const char* zone_script = world.GetZoneScript(GetZone()->GetZoneID());

		if (zone_script && lua_interface) {
			lua_interface->RunZoneScript(zone_script, "leave_location", GetZone(), this, GetLocation());
		}
		
		SetLocation(grid_id);
		
		if (zone_script && lua_interface) {
			lua_interface->RunZoneScript(zone_script, "enter_location", GetZone(), this, grid_id);
		}
	}
	if (activity == UPDATE_ACTIVITY_IN_WATER_ABOVE || activity == UPDATE_ACTIVITY_IN_WATER_BELOW ||
		activity == UPDATE_ACTIVITY_MOVE_WATER_BELOW_AOM || activity == UPDATE_ACTIVITY_MOVE_WATER_ABOVE_AOM) {
		if (MakeRandomFloat(0, 100) < 25 && InWater())
			GetSkillByName("Swimming", true);
	}
	// don't have to uncomment the print packet but you MUST uncomment the safe_delete() for xml structs
	//update->PrintPacket();
	//safe_delete(update);

	LogWrite(PLAYER__DEBUG, 7, "Player", "Exit: %s", __FUNCTION__); // trace
}

int16 Player::GetLastMovementActivity(){
	return last_movement_activity;
}

void Player::AddSpawnInfoPacketForXOR(int32 spawn_id, uchar* packet, int16 packet_size){
	spawn_info_packet_list[spawn_id] = string((char*)packet, packet_size);
}

void Player::AddSpawnPosPacketForXOR(int32 spawn_id, uchar* packet, int16 packet_size){
	spawn_pos_packet_list[spawn_id] = string((char*)packet, packet_size);
}

uchar* Player::GetSpawnPosPacketForXOR(int32 spawn_id){
	uchar* ret = 0;
	if(spawn_pos_packet_list.count(spawn_id) == 1)
		ret = (uchar*)spawn_pos_packet_list[spawn_id].c_str();
	return ret;
}
uchar* Player::GetSpawnInfoPacketForXOR(int32 spawn_id){
	uchar* ret = 0;
	if(spawn_info_packet_list.count(spawn_id) == 1)
		ret = (uchar*)spawn_info_packet_list[spawn_id].c_str();
	return ret;
}
void Player::AddSpawnVisPacketForXOR(int32 spawn_id, uchar* packet, int16 packet_size){
	spawn_vis_packet_list[spawn_id] = string((char*)packet, packet_size);
}

uchar* Player::GetSpawnVisPacketForXOR(int32 spawn_id){
	uchar* ret = 0;
	if(spawn_vis_packet_list.count(spawn_id) == 1)
		ret = (uchar*)spawn_vis_packet_list[spawn_id].c_str();
	return ret;
}

uchar* Player::GetTempInfoPacketForXOR(){
	return spawn_tmp_info_xor_packet;
}

uchar* Player::GetTempVisPacketForXOR(){
	return spawn_tmp_vis_xor_packet;
}

uchar* Player::GetTempPosPacketForXOR(){
	return spawn_tmp_pos_xor_packet;
}

uchar* Player::SetTempInfoPacketForXOR(int16 size){
	spawn_tmp_info_xor_packet = new uchar[size];
	info_xor_size = size;
	return spawn_tmp_info_xor_packet;
}

uchar* Player::SetTempVisPacketForXOR(int16 size){
	spawn_tmp_vis_xor_packet = new uchar[size];
	vis_xor_size = size;
	return spawn_tmp_vis_xor_packet;
}

uchar* Player::SetTempPosPacketForXOR(int16 size){
	spawn_tmp_pos_xor_packet = new uchar[size];
	pos_xor_size = size;
	return spawn_tmp_pos_xor_packet;
}

bool Player::CheckPlayerInfo(){
	return info != 0;
}

bool Player::SetSpawnSentState(Spawn* spawn, SpawnState state) {
	bool val = true;
	spawn_mutex.writelock(__FUNCTION__, __LINE__);
	int16 index = GetIndexForSpawn(spawn);
	if(index > 0 && (state == SpawnState::SPAWN_STATE_SENDING)) {
		LogWrite(PLAYER__WARNING, 0, "Player", "Spawn ALREADY INDEXED for Player %s (%u).  Spawn %s (index %u) attempted to state %u.", 
			GetName(), GetCharacterID(), spawn->GetName(), index, state);
			if(GetClient() && GetClient()->IsReloadingZone()) {
				spawn_packet_sent.insert(make_pair(spawn->GetID(), state));
				val = false;
			}
		// we don't do anything this spawn is already populated by the player 
	}
	else {
		LogWrite(PLAYER__DEBUG, 0, "Player", "Spawn for Player %s (%u).  Spawn %s (index %u) in state %u.", 
			GetName(), GetCharacterID(), spawn->GetName(), index, state);
		
		map<int32,int8>::iterator itr = spawn_packet_sent.find(spawn->GetID());
		if(itr != spawn_packet_sent.end())
			itr->second = state;
		else
			spawn_packet_sent.insert(make_pair(spawn->GetID(), state));
		if(state == SPAWN_STATE_SENT_WAIT) {
			map<int32,SpawnQueueState*>::iterator state_itr;
			if((state_itr = spawn_state_list.find(spawn->GetID())) != spawn_state_list.end()) {
				safe_delete(state_itr->second);
				spawn_state_list.erase(state_itr);
			}
			
			SpawnQueueState* removal = new SpawnQueueState;
			removal->index_id = index;
			removal->spawn_state_timer = Timer(500, true);
			removal->spawn_state_timer.Start();
			spawn_state_list.insert(make_pair(spawn->GetID(),removal));
		}
		else if(state == SpawnState::SPAWN_STATE_REMOVING && 
			spawn_state_list.count(spawn->GetID()) == 0) {
			SpawnQueueState* removal = new SpawnQueueState;
			removal->index_id = index;
			removal->spawn_state_timer = Timer(1000, true);
			removal->spawn_state_timer.Start();
			spawn_state_list.insert(make_pair(spawn->GetID(),removal));
		}
	}
	spawn_mutex.releasewritelock(__FUNCTION__, __LINE__);
	return val;
}

void Player::CheckSpawnStateQueue() {
	if(!GetClient() || !GetClient()->IsReadyForUpdates())
		return;

	spawn_mutex.writelock(__FUNCTION__, __LINE__);
	map<int32, SpawnQueueState*>::iterator itr;
	for(itr = spawn_state_list.begin(); itr != spawn_state_list.end();) {
		if(itr->second->spawn_state_timer.Check()) {
			map<int32, int8>::iterator sent_itr = spawn_packet_sent.find(itr->first);
			LogWrite(PLAYER__DEBUG, 0, "Player", "Spawn for Player %s (%u).  Spawn index %u in state %u.", 
				GetName(), GetCharacterID(), itr->second->index_id, sent_itr->second);
			switch(sent_itr->second) {
				case SpawnState::SPAWN_STATE_SENT_WAIT: {
					sent_itr->second = SpawnState::SPAWN_STATE_SENT;
					SpawnQueueState* sr = itr->second;
					itr = spawn_state_list.erase(itr);
					safe_delete(sr);
					break;
				}
				case SpawnState::SPAWN_STATE_REMOVING: {
					if(itr->first == GetID() && GetClient()->IsReloadingZone()) {
						itr->second->spawn_state_timer.Disable();
						continue;
					}
					
					if(itr->second->index_id) {
						PacketStruct* packet = packet = configReader.getStruct("WS_DestroyGhostCmd", GetClient()->GetVersion());
						packet->setDataByName("spawn_index", itr->second->index_id);
						packet->setDataByName("delete", 1);	
						GetClient()->QueuePacket(packet->serialize());
						safe_delete(packet);
					}
					sent_itr->second = SpawnState::SPAWN_STATE_REMOVING_SLEEP;
					itr++;
					break;
				}
				case SpawnState::SPAWN_STATE_REMOVING_SLEEP: {
					map<int32, int8>::iterator sent_itr = spawn_packet_sent.find(itr->first);
					sent_itr->second = SpawnState::SPAWN_STATE_REMOVED;
					SpawnQueueState* sr = itr->second;
					itr = spawn_state_list.erase(itr);
					safe_delete(sr);
					break;
				}
				default: {
					// reset
					itr->second->spawn_state_timer.Disable();
					break;
				}
			}
		}
		else
			itr++;
	}
	spawn_mutex.releasewritelock(__FUNCTION__, __LINE__);
}

bool Player::WasSentSpawn(int32 spawn_id){
	if(GetID() == spawn_id)
		return true;

	bool ret = false;
	spawn_mutex.readlock(__FUNCTION__, __LINE__);
	map<int32, int8>::iterator itr = spawn_packet_sent.find(spawn_id);
	if(itr != spawn_packet_sent.end() && itr->second == SpawnState::SPAWN_STATE_SENT) {
		ret = true;
	}
	spawn_mutex.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

bool Player::IsSendingSpawn(int32 spawn_id){
	bool ret = false;
	spawn_mutex.readlock(__FUNCTION__, __LINE__);
	map<int32, int8>::iterator itr = spawn_packet_sent.find(spawn_id);
	if(itr != spawn_packet_sent.end() && (itr->second == SpawnState::SPAWN_STATE_SENDING || itr->second == SPAWN_STATE_SENT_WAIT)) {
		ret = true;
	}
	spawn_mutex.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

bool Player::IsRemovingSpawn(int32 spawn_id){
	bool ret = false;
	spawn_mutex.readlock(__FUNCTION__, __LINE__);
	map<int32, int8>::iterator itr = spawn_packet_sent.find(spawn_id);
	if(itr != spawn_packet_sent.end() && 
		(itr->second == SpawnState::SPAWN_STATE_REMOVING || itr->second == SpawnState::SPAWN_STATE_REMOVING_SLEEP)) {
		ret = true;
	}
	spawn_mutex.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

PlayerSkillList* Player::GetSkills(){
	return &skill_list;
}

void Player::InCombat(bool val, bool range) {
	if (val)
		GetInfoStruct()->set_flags(GetInfoStruct()->get_flags() | (1 << (range?CF_RANGED_AUTO_ATTACK:CF_AUTO_ATTACK)));
	else
		GetInfoStruct()->set_flags(GetInfoStruct()->get_flags() & ~(1 << (range?CF_RANGED_AUTO_ATTACK:CF_AUTO_ATTACK)));

	bool changeCombatState = false;
	
	if((in_combat && !val) || (!in_combat && val))
		changeCombatState = true;

	in_combat = val;
	if(in_combat)
		AddIconValue(64);
	else
		RemoveIconValue(64);
	
	bool update_regen = false;
	if(GetInfoStruct()->get_engaged_encounter()) {
		if(!IsAggroed() || !IsEngagedInEncounter()) {
			GetInfoStruct()->set_engaged_encounter(0);
			update_regen = true;
		}
	}
	
	if(changeCombatState || update_regen)
		SetRegenValues((GetInfoStruct()->get_effective_level() > 0) ? GetInfoStruct()->get_effective_level() : GetLevel());

	charsheet_changed = true;
	info_changed = true;
}

void Player::SetCharSheetChanged(bool val){
	charsheet_changed = val;
}

bool Player::GetCharSheetChanged(){
	return charsheet_changed;
}

void Player::SetRaidSheetChanged(bool val){
	raidsheet_changed = val;
}

bool Player::GetRaidSheetChanged(){
	return raidsheet_changed;
}

bool Player::AdventureXPEnabled(){
	return (GetInfoStruct()->get_flags() & (1 << CF_COMBAT_EXPERIENCE_ENABLED));
}

bool Player::TradeskillXPEnabled() {
	// TODO: need to identify the flag to togle tradeskill xp
	return true;
}

void Player::set_character_flag(int flag){
	LogWrite(PLAYER__DEBUG, 0, "Player", "Flag: %u", flag);
	LogWrite(PLAYER__DEBUG, 0, "Player", "Flags before: %u, Flags2: %u", GetInfoStruct()->get_flags(), GetInfoStruct()->get_flags2());

	if (flag > CF_MAXIMUM_FLAG) return;
	if (flag < 32) GetInfoStruct()->set_flags(GetInfoStruct()->get_flags() | (1 << flag));
	else GetInfoStruct()->set_flags2(GetInfoStruct()->get_flags2() | (1 << (flag - 32)));
	charsheet_changed = true;
	info_changed = true;

	LogWrite(PLAYER__DEBUG, 0, "Player", "Flags after: %u, Flags2: %u", GetInfoStruct()->get_flags(), GetInfoStruct()->get_flags2());
}

void Player::reset_character_flag(int flag){
	LogWrite(PLAYER__DEBUG, 0, "Player", "Flag: %u", flag);
	LogWrite(PLAYER__DEBUG, 0, "Player", "Flags before: %u, Flags2: %u", GetInfoStruct()->get_flags(), GetInfoStruct()->get_flags2());

	if (flag > CF_MAXIMUM_FLAG) return;
	if (flag < 32)
	{
		int8 origflag = GetInfoStruct()->get_flags();
		GetInfoStruct()->set_flags(origflag &= ~(1 << flag));
	}
	else
	{
		int8 flag2 = GetInfoStruct()->get_flags2();
		GetInfoStruct()->set_flags2(flag2  &= ~(1 << (flag - 32)));
	}
	charsheet_changed = true;
	info_changed = true;

	LogWrite(PLAYER__DEBUG, 0, "Player", "Flags after: %u, Flags2: %u", GetInfoStruct()->get_flags(), GetInfoStruct()->get_flags2());
}

void Player::toggle_character_flag(int flag){
	LogWrite(PLAYER__DEBUG, 0, "Player", "Flag: %u", flag);
	LogWrite(PLAYER__DEBUG, 0, "Player", "Flags before: %u, Flags2: %u", GetInfoStruct()->get_flags(), GetInfoStruct()->get_flags2());

	if (flag > CF_MAXIMUM_FLAG) return;
	if (flag < 32)
	{
		int32 origflag = GetInfoStruct()->get_flags();
		GetInfoStruct()->set_flags(origflag ^= (1 << flag));
	}
	else
	{
		int32 flag2 = GetInfoStruct()->get_flags2();
		GetInfoStruct()->set_flags2(flag2  ^= (1 << (flag - 32)));
	}
	charsheet_changed = true;
	info_changed = true;

	LogWrite(PLAYER__DEBUG, 0, "Player", "Flags after: %u, Flags2: %u", GetInfoStruct()->get_flags(), GetInfoStruct()->get_flags2());
}

bool Player::get_character_flag(int flag){
	bool ret = false;

	if (flag > CF_MAXIMUM_FLAG){
		LogWrite(PLAYER__DEBUG, 0, "Player", "Player::get_character_flag error: attempted to check flag %i", flag);
		return ret;
	}
	if (flag < 32) ret = ((GetInfoStruct()->get_flags()) >> flag & 1);
	else ret = ((GetInfoStruct()->get_flags2()) >> (flag - 32) & 1);

	return ret;
}

float Player::GetXPVitality(){
	return GetInfoStruct()->get_xp_vitality();
}

float Player::GetTSXPVitality() {
	return GetInfoStruct()->get_tradeskill_xp_vitality();
}

bool Player::DoubleXPEnabled(){
	return GetInfoStruct()->get_xp_vitality() > 0;
}

void Player::SetCharacterID(int32 new_id){
	char_id = new_id;
}

int32 Player::GetCharacterID(){
	return char_id;
}

float Player::CalculateXP(Spawn* victim){
	if(AdventureXPEnabled() == false || !victim)
		return 0;
	float multiplier = 0;

	float zone_xp_modifier = 1;				// let's be safe!!
	if( GetZone()->GetXPModifier() != 0 ) {
		zone_xp_modifier = GetZone()->GetXPModifier();
		LogWrite(PLAYER__DEBUG, 5, "XP", "Zone XP Modifier = %.2f", zone_xp_modifier);
	}

	switch(GetArrowColor(victim->GetLevel())){
		case ARROW_COLOR_GRAY:
			LogWrite(PLAYER__DEBUG, 5, "XP", "Gray Arrow = No XP");
			return 0.0f;
			break;
		case ARROW_COLOR_GREEN:
			multiplier = 3.25;
			LogWrite(PLAYER__DEBUG, 5, "XP", "Green Arrow Multiplier = %.2f", multiplier);
			break;
		case ARROW_COLOR_BLUE:
			multiplier = 3.5;
			LogWrite(PLAYER__DEBUG, 5, "XP", "Blue Arrow Multiplier = %.2f", multiplier);
			break;
		case ARROW_COLOR_WHITE:
			multiplier = 4;
			LogWrite(PLAYER__DEBUG, 5, "XP", "White Arrow Multiplier = %.2f", multiplier);
			break;
		case ARROW_COLOR_YELLOW:
			multiplier = 4.25;
			LogWrite(PLAYER__DEBUG, 5, "XP", "Yellow Arrow Multiplier = %.2f", multiplier);
			break;
		case ARROW_COLOR_ORANGE:
			multiplier = 4.5;
			LogWrite(PLAYER__DEBUG, 5, "XP", "Orange Arrow Multiplier = %.2f", multiplier);
			break;
		case ARROW_COLOR_RED:
			multiplier = 6;
			LogWrite(PLAYER__DEBUG, 5, "XP", "Red Arrow Multiplier = %.2f", multiplier);
			break;
	}
	float total = multiplier * 8;	
	LogWrite(PLAYER__DEBUG, 5, "XP", "Multiplier * 8 = %.2f", total);

	if(victim->GetDifficulty() > 6) { // no need to multiply by 1 if this is a normal mob
		total *= (victim->GetDifficulty() - 5);
		LogWrite(PLAYER__DEBUG, 5, "XP", "Encounter > 6, total = %.2f", total);
	}
	else if(victim->GetDifficulty() <= 5) {
		total /= (7 - victim->GetDifficulty()); //1 down mobs are worth half credit, 2 down worth .25, etc
		LogWrite(PLAYER__DEBUG, 5, "XP", "Encounter <= 5, total = %.2f", total);
	}

	if(victim->GetHeroic() > 1) {
		total *= victim->GetHeroic();
		LogWrite(PLAYER__DEBUG, 5, "XP", "Heroic, total = %.2f", total);
	}
	if(DoubleXPEnabled()) {
		LogWrite(PLAYER__DEBUG, 5, "XP", "Calculating Double XP!");

		float percent = (((float)(total))/GetNeededXP()) *100;
		LogWrite(PLAYER__DEBUG, 5, "XP", "Percent of total / XP Needed * 100, percent = %.2f", percent);
		float xp_vitality = GetXPVitality();
		if(xp_vitality >= percent) {
			GetInfoStruct()->set_xp_vitality(xp_vitality - percent);
			total *= 2;
			LogWrite(PLAYER__DEBUG, 5, "XP", "Vitality >= Percent, total = %.2f", total);
		}
		else {
			total += ((GetXPVitality() / percent) *2)*total;
			GetInfoStruct()->set_xp_vitality(0);
			LogWrite(PLAYER__DEBUG, 5, "XP", "Vitality < Percent, total = %.2f", total);
		}
	}
	LogWrite(PLAYER__DEBUG, 5, "XP", "Final total = %.2f", (total * world.GetXPRate() * zone_xp_modifier));
	return total * world.GetXPRate() * zone_xp_modifier;
}

float Player::CalculateTSXP(int8 level){
	if(TradeskillXPEnabled() == false)
		return 0;
	float multiplier = 0;

	float zone_xp_modifier = 1;				// let's be safe!!
	if( GetZone()->GetXPModifier() != 0 ) {
		zone_xp_modifier = GetZone()->GetXPModifier();
		LogWrite(PLAYER__DEBUG, 5, "XP", "Zone XP Modifier = %.2f", zone_xp_modifier);
	}

	sint16 diff = level - GetTSLevel();
	if(GetTSLevel() < 10)
		diff *= 3;
	else if(GetTSLevel() <= 20)
		diff *= 2;
	if(diff >= 9)
		multiplier = 6;
	else if(diff >= 5)
		multiplier = 4.5;
	else if(diff >= 1)
		multiplier = 4.25;
	else if(diff == 0)
		multiplier = 4;
	else if(diff <= -11)
		multiplier = 0;
	else if(diff <= -6)
		multiplier = 3.25;
	else //if(diff < 0)
		multiplier = 3.5;


	float total = multiplier * 8;	
	LogWrite(PLAYER__DEBUG, 5, "XP", "Multiplier * 8 = %.2f", total);

	if(DoubleXPEnabled()) {
		LogWrite(PLAYER__DEBUG, 5, "XP", "Calculating Double XP!");

		float percent = (((float)(total))/GetNeededTSXP()) *100;
		LogWrite(PLAYER__DEBUG, 5, "XP", "Percent of total / XP Needed * 100, percent = %.2f", percent);

		float ts_xp_vitality = GetTSXPVitality();
		if(ts_xp_vitality >= percent) {
			GetInfoStruct()->set_tradeskill_xp_vitality(ts_xp_vitality - percent);
			total *= 2;
			LogWrite(PLAYER__DEBUG, 5, "XP", "Vitality >= Percent, total = %.2f", total);
		}
		else {
			total += ((GetTSXPVitality() / percent) *2)*total;
			GetInfoStruct()->set_tradeskill_xp_vitality(0);
			LogWrite(PLAYER__DEBUG, 5, "XP", "Vitality < Percent, total = %.2f", total);
		}
	}
	LogWrite(PLAYER__DEBUG, 5, "XP", "Final total = %.2f", (total * world.GetXPRate() * zone_xp_modifier));
	return total * world.GetXPRate() * zone_xp_modifier;
}

void Player::CalculateOfflineDebtRecovery(int32 unix_timestamp)
{
	float xpDebt = GetXPDebt();
	// not a real timestamp to work with
	if(unix_timestamp < 1 || xpDebt == 0.0f)
		return;

	uint32 diff = (Timer::GetUnixTimeStamp() - unix_timestamp)/1000;
	
	float recoveryDebtPercentage = rule_manager.GetGlobalRule(R_Combat, ExperienceDebtRecoveryPercent)->GetFloat()/100.0f;
	int32 recoveryPeriodSeconds = rule_manager.GetGlobalRule(R_Combat, ExperienceDebtRecoveryPeriod)->GetInt32();
	if(recoveryDebtPercentage == 0.0f || recoveryPeriodSeconds < 1)
		return;


	float periodsPassed = (float)diff/(float)recoveryPeriodSeconds;

	// not enough time passed to calculate debt xp recovered
	if(periodsPassed < 1.0f)
		return;

	float debtToSubtract = xpDebt * ((recoveryDebtPercentage*periodsPassed)/100.0f);

	if(debtToSubtract >= xpDebt)
		GetInfoStruct()->set_xp_debt(0.0f);
	else
		GetInfoStruct()->set_xp_debt(xpDebt - debtToSubtract);
}

void Player::SetNeededXP(int32 val){
	GetInfoStruct()->set_xp_needed(val);
}

void Player::SetNeededXP(){
	//GetInfoStruct()->xp_needed = GetLevel() * 100;
	// Get xp needed to get to the next level
	int16 level = GetLevel() + 1;
	SetNeededXP(GetNeededXPByLevel(level));
}

int32 Player::GetNeededXPByLevel(int8 level) {
	int32 exp_required = 0;
	if (!Player::m_levelXPReq.count(level) && level > 95 && Player::m_levelXPReq.count(95)) {
		exp_required = (Player::m_levelXPReq[95] * ((level - 95) + 1));
	}
	else if(Player::m_levelXPReq.count(level))
		exp_required = Player::m_levelXPReq[level];
	else
		exp_required = 0;
	
	return exp_required;
}

void Player::SetXP(int32 val){
	GetInfoStruct()->set_xp(val);
}

void Player::SetNeededTSXP(int32 val) {
	GetInfoStruct()->set_ts_xp_needed(val);
}

void Player::SetNeededTSXP() {
	GetInfoStruct()->set_ts_xp_needed(GetTSLevel() * 100);
}

void Player::SetTSXP(int32 val) {
	GetInfoStruct()->set_ts_xp(val);
}

float Player::GetXPDebt(){
	return GetInfoStruct()->get_xp_debt();
}

int32 Player::GetNeededXP(){
	return GetInfoStruct()->get_xp_needed();
}

int32 Player::GetXP(){
	return GetInfoStruct()->get_xp();
}

int32 Player::GetNeededTSXP() {
	return GetInfoStruct()->get_ts_xp_needed();
}

int32 Player::GetTSXP() {
	return GetInfoStruct()->get_ts_xp();
}

bool Player::AddXP(int32 xp_amount){
	if(!GetClient()) // potential linkdead player
		return false;
	
	MStats.lock();
	xp_amount += (int32)(((float)xp_amount) * stats[ITEM_STAT_COMBATEXPMOD]) / 100;
	MStats.unlock();

	if(GetInfoStruct()->get_xp_debt())
	{
		float expRatioToDebt = rule_manager.GetGlobalRule(R_Combat, ExperienceToDebt)->GetFloat()/100.0f;
		int32 amountToTakeFromDebt = (int32)((float)expRatioToDebt * (float)xp_amount);
		int32 amountRequiredClearDebt = (GetInfoStruct()->get_xp_debt()/100.0f) * xp_amount;

		if(amountToTakeFromDebt > amountRequiredClearDebt)
		{
			GetInfoStruct()->set_xp_debt(0.0f);
			if(amountRequiredClearDebt > xp_amount)
				xp_amount = 0;
			else
				xp_amount -= amountRequiredClearDebt;
		}
		else
		{
			float amountRemovedPct = ((float)amountToTakeFromDebt/(float)amountRequiredClearDebt);
			GetInfoStruct()->set_xp_debt(GetInfoStruct()->get_xp_debt()-amountRemovedPct);
			if(amountToTakeFromDebt > xp_amount)
				xp_amount = 0;
			else
				xp_amount -= amountToTakeFromDebt;
		}
	}
	
	// used up in xp debt
	if(!xp_amount) {
		SetCharSheetChanged(true);
		return true;
	}

	int32 prev_level = GetLevel();
	float current_xp_percent = ((float)GetXP()/(float)GetNeededXP())*100;
	int32 mini_ding_pct = rule_manager.GetGlobalRule(R_Player, MiniDingPercentage)->GetInt32();
	float miniding_min_percent = 0.0f;
	if(mini_ding_pct < 10 || mini_ding_pct > 50) {
		mini_ding_pct = 0;
	}
	else {
		miniding_min_percent = ((int)(current_xp_percent/mini_ding_pct)+1)*mini_ding_pct;
	}
	while((xp_amount + GetXP()) >= GetNeededXP()){
		if (!CheckLevelStatus(GetLevel() + 1)) {
			if(GetClient()) {
				GetClient()->SimpleMessage(CHANNEL_COLOR_RED, "You do not have the required status to level up anymore!");
			}
			SetCharSheetChanged(true);	
			return false;
		}
		int32 prev_xp_amount = xp_amount;
		xp_amount -= GetNeededXP() - GetXP();
		if(GetClient()->ChangeLevel(GetLevel(), GetLevel()+1, prev_xp_amount))
			SetLevel(GetLevel() + 1);
		else {
			SetXP(GetXP() + prev_xp_amount);
			SetCharSheetChanged(true);	
			return false;
		}
	}
	
	// set the actual end xp_amount result
	SetXP(GetXP() + xp_amount);
	
	if(GetClient()) {
		GetClient()->Message(CHANNEL_REWARD, "You gain %u experience!", (int32)xp_amount);
	}
	
	GetPlayerInfo()->CalculateXPPercentages();
	current_xp_percent = ((float)GetXP()/(float)GetNeededXP())*100;
	if(miniding_min_percent > 0.0f && current_xp_percent >= miniding_min_percent){
		if(GetClient() && rule_manager.GetGlobalRule(R_Spells, UseClassicSpellLevel)->GetInt8())
			GetClient()->SendNewAdventureSpells(); // mini ding involves checking spells again in classic level settings
		SetHP(GetTotalHP());
		SetPower(GetTotalPower());
		GetZone()->SendCastSpellPacket(332, this, this); //send mini level up spell effect
	}
		
	SetCharSheetChanged(true);
	return true;
}

bool Player::AddTSXP(int32 xp_amount){
	MStats.lock();
	xp_amount += ((xp_amount)*stats[ITEM_STAT_TRADESKILLEXPMOD]) / 100;
	MStats.unlock();

	float current_xp_percent = ((float)GetTSXP()/(float)GetNeededTSXP())*100;
	
	int32 mini_ding_pct = rule_manager.GetGlobalRule(R_Player, MiniDingPercentage)->GetInt32();
	float miniding_min_percent = 0.0f;
	if(mini_ding_pct < 10 || mini_ding_pct > 50) {
		mini_ding_pct = 0;
	}
	else {
		miniding_min_percent = ((int)(current_xp_percent/mini_ding_pct)+1)*mini_ding_pct;
	}
	
	while((xp_amount + GetTSXP()) >= GetNeededTSXP()){
		if (!CheckLevelStatus(GetTSLevel() + 1)) {
			if(GetClient()) {
				GetClient()->SimpleMessage(CHANNEL_COLOR_RED, "You do not have the required status to level up anymore!");
			}
			return false;
		}
		int32 prev_xp_amount = xp_amount;
		xp_amount -= GetNeededTSXP() - GetTSXP();
		if(GetClient()->ChangeTSLevel(GetLevel(), GetLevel()+1, prev_xp_amount)) {
			SetTSLevel(GetTSLevel() + 1);
			SetTSXP(0);
			SetNeededTSXP();
		}
		else {
			SetTSXP(GetTSXP() + prev_xp_amount);
			SetCharSheetChanged(true);	
			return false;
		}
	}
	SetTSXP(GetTSXP() + xp_amount);
	GetPlayerInfo()->CalculateXPPercentages();
	current_xp_percent = ((float)GetTSXP()/(float)GetNeededTSXP())*100;
	if(current_xp_percent >= miniding_min_percent){
		SetHP(GetTotalHP());
		SetPower(GetTotalPower());
	}

	if (GetTradeskillClass() == 0){
		SetTradeskillClass(1);
		GetInfoStruct()->set_tradeskill_class1(1);
		GetInfoStruct()->set_tradeskill_class2(1);
		GetInfoStruct()->set_tradeskill_class3(1);
	}
	
	SetCharSheetChanged(true);
	return true;
}

void Player::CalculateLocation(){
	if(GetSpeed() > 0 ){
		if(GetHeading() >= 270 && GetHeading() <= 360){
			SetX(GetX() + (GetSpeed()*.5)*((360-GetHeading())/90));
			SetZ(GetZ() - (GetSpeed()*.5)*((GetHeading()-270)/90));
		}
		else if(GetHeading() >= 180 && GetHeading() < 270){
			SetX(GetX() + (GetSpeed()*.5)*((GetHeading()-180)/90));
			SetZ(GetZ() + (GetSpeed()*.5)*((270-GetHeading())/90));
		}
		else if(GetHeading() >= 90 && GetHeading() < 180){
			SetX(GetX() - (GetSpeed()*.5)*((180-GetHeading())/90));
			SetZ(GetZ() + (GetSpeed()*.5)*((GetHeading()-90)/90));
		}
		else if(GetHeading() >= 0 && GetHeading() < 90){
			SetX(GetX() - (GetSpeed()*.5)*(GetHeading()/90));
			SetZ(GetZ() - (GetSpeed()*.5)*((90-GetHeading())/90));
		}
	}
}

Spawn* Player::GetSpawnByIndex(int16 index){
	Spawn* spawn = 0;

	index_mutex.readlock(__FUNCTION__, __LINE__);
	if(player_spawn_id_map.count(index) > 0)
		spawn = player_spawn_id_map[index];
	index_mutex.releasereadlock(__FUNCTION__, __LINE__);

	return spawn;
}

int16 Player::GetIndexForSpawn(Spawn* spawn) {
	int16 val = 0;

	index_mutex.readlock(__FUNCTION__, __LINE__);
	if(player_spawn_reverse_id_map.count(spawn) > 0)
		val = player_spawn_reverse_id_map[spawn];
	index_mutex.releasereadlock(__FUNCTION__, __LINE__);

	return val;
}

bool Player::WasSpawnRemoved(Spawn* spawn){
	bool wasRemoved = false;

	if(IsRemovingSpawn(spawn->GetID()))
		return false;
	
	spawn_mutex.readlock(__FUNCTION__, __LINE__);
	map<int32, int8>::iterator itr = spawn_packet_sent.find(spawn_id);
	if(itr != spawn_packet_sent.end() && itr->second == SpawnState::SPAWN_STATE_REMOVED) {
		wasRemoved = true;
	}
	spawn_mutex.releasereadlock(__FUNCTION__, __LINE__);

	return wasRemoved;
}

void Player::ResetSpawnPackets(int32 id) {
	info_mutex.writelock(__FUNCTION__, __LINE__);
	vis_mutex.writelock(__FUNCTION__, __LINE__);
	pos_mutex.writelock(__FUNCTION__, __LINE__);
	index_mutex.writelock(__FUNCTION__, __LINE__);

	if (spawn_info_packet_list.count(id))
		spawn_info_packet_list.erase(id);

	if (spawn_pos_packet_list.count(id))
		spawn_pos_packet_list.erase(id);

	if (spawn_vis_packet_list.count(id))
		spawn_vis_packet_list.erase(id);
	
	index_mutex.releasewritelock(__FUNCTION__, __LINE__);
	vis_mutex.releasewritelock(__FUNCTION__, __LINE__);
	pos_mutex.releasewritelock(__FUNCTION__, __LINE__);
	info_mutex.releasewritelock(__FUNCTION__, __LINE__);
}

void Player::RemoveSpawn(Spawn* spawn, bool delete_spawn)
{
	LogWrite(PLAYER__DEBUG, 3, "Player", "Remove Spawn '%s' (%u)", spawn->GetName(), spawn->GetID());

	SetSpawnSentState(spawn, delete_spawn ? SpawnState::SPAWN_STATE_REMOVING : SpawnState::SPAWN_STATE_REMOVING_SLEEP);
	
	info_mutex.writelock(__FUNCTION__, __LINE__);
	vis_mutex.writelock(__FUNCTION__, __LINE__);
	pos_mutex.writelock(__FUNCTION__, __LINE__);

	index_mutex.writelock(__FUNCTION__, __LINE__);

	if (player_spawn_reverse_id_map[spawn] && player_spawn_id_map.count(player_spawn_reverse_id_map[spawn]) > 0)
		player_spawn_id_map.erase(player_spawn_reverse_id_map[spawn]);

	if (player_spawn_reverse_id_map.count(spawn) > 0)
		player_spawn_reverse_id_map.erase(spawn);

	if (player_spawn_id_map.count(spawn->GetID()) && player_spawn_id_map[spawn->GetID()] == spawn)
		player_spawn_id_map.erase(spawn->GetID());

	int32 id = spawn->GetID();
	if (spawn_info_packet_list.count(id))
		spawn_info_packet_list.erase(id);

	if (spawn_pos_packet_list.count(id))
		spawn_pos_packet_list.erase(id);

	if (spawn_vis_packet_list.count(id))
		spawn_vis_packet_list.erase(id);

	index_mutex.releasewritelock(__FUNCTION__, __LINE__);
	pos_mutex.releasewritelock(__FUNCTION__, __LINE__);
	vis_mutex.releasewritelock(__FUNCTION__, __LINE__);
	info_mutex.releasewritelock(__FUNCTION__, __LINE__);
}

vector<int32> Player::GetQuestIDs(){
	vector<int32> ret;
	map<int32, Quest*>::iterator itr;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	for(itr = player_quests.begin(); itr != player_quests.end(); itr++){
		if(itr->second)
			ret.push_back(itr->second->GetQuestID());
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

vector<Quest*>* Player::CheckQuestsItemUpdate(Item* item){
	vector<Quest*>* quest_updates = 0;
	map<int32, Quest*>::iterator itr;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	for(itr = player_quests.begin(); itr != player_quests.end(); itr++){
		if(itr->second && itr->second->CheckQuestItemUpdate(item->details.item_id, item->details.count)){
			if(!quest_updates)
				quest_updates = new vector<Quest*>();
			quest_updates->push_back(itr->second);
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return quest_updates;
}

void Player::CheckQuestsCraftUpdate(Item* item, int32 qty){
	map<int32, Quest*>::iterator itr;
	vector<Quest*>* update_list = new vector<Quest*>;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	for(itr = player_quests.begin(); itr != player_quests.end(); itr++){
		if(itr->second){
			if(item && qty > 0){
				if(itr->second->CheckQuestRefIDUpdate(item->details.item_id, qty)){
					update_list->push_back(itr->second);
				}
			}
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	if(update_list && update_list->size() > 0){
		Client* client = GetClient();
		if(client){
			for(int8 i=0;i<update_list->size(); i++){
				client->SendQuestUpdate(update_list->at(i));
				client->SendQuestFailure(update_list->at(i));
			}
		}
	}
	update_list->clear();
	safe_delete(update_list);
}

void Player::CheckQuestsHarvestUpdate(Item* item, int32 qty){
	map<int32, Quest*>::iterator itr;
	vector<Quest*>* update_list = new vector<Quest*>;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	for(itr = player_quests.begin(); itr != player_quests.end(); itr++){
		if(itr->second){
			if(item && qty > 0){
				if(itr->second->CheckQuestRefIDUpdate(item->details.item_id, qty)){
					update_list->push_back(itr->second);
				}
			}
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	if(update_list && update_list->size() > 0){
		Client* client = GetClient();
		if(client){
			for(int8 i=0;i<update_list->size(); i++){
				client->SendQuestUpdate(update_list->at(i));
				client->SendQuestFailure(update_list->at(i));
			}
		}
	}
	update_list->clear();
	safe_delete(update_list);
}

vector<Quest*>* Player::CheckQuestsSpellUpdate(Spell* spell) {
	vector<Quest*>* quest_updates = 0;
	map<int32, Quest*>::iterator itr;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	for (itr = player_quests.begin(); itr != player_quests.end(); itr++){
		if (itr->second && itr->second->CheckQuestSpellUpdate(spell)) {
			if (!quest_updates)
				quest_updates = new vector<Quest*>();
			quest_updates->push_back(itr->second);
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return quest_updates;
}

PacketStruct* Player::GetQuestJournalPacket(bool all_quests, int16 version, int32 crc, int32 current_quest_id, bool updated){
	PacketStruct* packet = configReader.getStruct("WS_QuestJournalUpdate", version);
	Quest* quest = 0;
	if(packet){
		int16 total_quests_num = 0;
		int16 total_completed_quests = 0;
		MPlayerQuests.readlock(__FUNCTION__, __LINE__);
		map<int32, Quest*> total_quests = player_quests;
		if(all_quests && completed_quests.size() > 0)
			total_quests.insert(completed_quests.begin(), completed_quests.end());
		if(total_quests.size() > 0){
			map<string, int16> quest_types;
			map<int32, Quest*>::iterator itr;
			int16 zone_id = 0;
			for(itr = total_quests.begin(); itr != total_quests.end(); itr++){
				if(itr->first && itr->second){
					if(current_quest_id == 0 && itr->second->GetTurnedIn() == false)
						current_quest_id = itr->first;
					if(itr->second->GetTurnedIn())
						total_completed_quests++;
					if(itr->second->GetType()){
						if(quest_types.count(itr->second->GetType()) == 0){
							quest_types[itr->second->GetType()] = zone_id;
							zone_id++;
						}
					}
					if(itr->second->GetZone()){
						if(quest_types.count(itr->second->GetZone()) == 0){
							quest_types[itr->second->GetZone()] = zone_id; // Fix #490 - incorrect ordering of quests in journal
							zone_id++;
						}
					}
					total_quests_num++;
				}
				else
					continue;
			}
			packet->setArrayLengthByName("num_quests", total_quests_num);
			int16 i = 0;
			for(itr = total_quests.begin(); itr != total_quests.end(); itr++){
				if(i == 0 && quest_types.size() > 0){
					packet->setArrayLengthByName("num_quest_zones", quest_types.size());
					map<string, int16>::iterator type_itr;
					int16 x = 0;
					for(type_itr = quest_types.begin(); type_itr != quest_types.end(); type_itr++){
						packet->setArrayDataByName("quest_zones_zone", type_itr->first.c_str(), x);
						packet->setArrayDataByName("quest_zones_zone_id", type_itr->second, x);
						x++;
					}
				}
				if(itr->first == 0 || !itr->second)
					continue;
				if(!all_quests && !itr->second->GetUpdateRequired())
					continue;
				quest = itr->second;
				if(!quest->GetDeleted())
					packet->setArrayDataByName("active", 1, i);
				packet->setArrayDataByName("name", quest->GetName(), i);
				packet->setArrayDataByName("quest_type", quest->GetType(), i);
				packet->setArrayDataByName("quest_zone", quest->GetZone(), i);
				int8 display_status = QUEST_DISPLAY_STATUS_SHOW;
				if(itr->second->GetCompleted())
					packet->setArrayDataByName("completed", 1, i);
				if(itr->second->GetTurnedIn()){
					packet->setArrayDataByName("turned_in", 1, i);
					packet->setArrayDataByName("completed", 1, i);
					packet->setArrayDataByName("visible", 1, i);
					packet->setArrayDataByName("unknown3", 1, i);
					display_status += QUEST_DISPLAY_STATUS_COMPLETED;					
				}
				if (updated) {
					packet->setArrayDataByName("quest_updated", 1, i);
					packet->setArrayDataByName("journal_updated", 1, i);
				}
				packet->setArrayDataByName("quest_id", quest->GetQuestID(), i);
				packet->setArrayDataByName("day", quest->GetDay(), i);
				packet->setArrayDataByName("month", quest->GetMonth(), i);
				packet->setArrayDataByName("year", quest->GetYear(), i);
				packet->setArrayDataByName("level", quest->GetQuestLevel(), i);
				int8 difficulty = 0;
				string category = quest->GetType();
				if(category == "Tradeskill")
					difficulty = GetTSArrowColor(quest->GetQuestLevel());
				else
					difficulty = GetArrowColor(quest->GetQuestLevel());
				packet->setArrayDataByName("difficulty", difficulty, i);
				if (itr->second->GetEncounterLevel() > 4)
					packet->setArrayDataByName("encounter_level", quest->GetEncounterLevel(), i);
				else
					packet->setArrayDataByName("encounter_level", 4, i);
				if(version >= 931 && quest_types.count(quest->GetType()) > 0)
					packet->setArrayDataByName("zonetype_id", quest_types[quest->GetType()], i);
				if(version >= 931 && quest_types.count(quest->GetZone()) > 0)
					packet->setArrayDataByName("zone_id", quest_types[quest->GetZone()], i);
				if(version >= 931 && quest->GetVisible()){
					if (quest->GetCompletedFlag())
						display_status += QUEST_DISPLAY_STATUS_COMPLETE_FLAG;
					else if (quest->IsRepeatable())
						display_status += QUEST_DISPLAY_STATUS_REPEATABLE;
					if (quest->GetYellowName() || quest->CheckCategoryYellow())
						display_status += QUEST_DISPLAY_STATUS_YELLOW;
					
					if (quest->IsTracked())
						display_status += QUEST_DISPLAY_STATUS_CHECK;
					else
						display_status += QUEST_DISPLAY_STATUS_NO_CHECK;

					if (quest->IsHidden() && !quest->GetTurnedIn()) {
						display_status += QUEST_DISPLAY_STATUS_HIDDEN;
						display_status -= QUEST_DISPLAY_STATUS_SHOW;
					}
					
					if(quest->CanShareQuestCriteria(GetClient(),false)) {
						display_status += QUEST_DISPLAY_STATUS_CAN_SHARE;
					}
				}
				else
					packet->setArrayDataByName("visible", quest->GetVisible(), i);
				if (itr->second->IsRepeatable())
					packet->setArrayDataByName("repeatable", 1, i);
				
				packet->setArrayDataByName("display_status", display_status, i);
				i++;
			}
			//packet->setDataByName("unknown4", 0);
			packet->setDataByName("visible_quest_id", current_quest_id);
		}
		MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);		
		packet->setDataByName("player_crc", crc);
		packet->setDataByName("player_name", GetName());
		packet->setDataByName("used_quests", total_quests_num - total_completed_quests);
		packet->setDataByName("max_quests", 75);

		LogWrite(PLAYER__PACKET, 0, "Player", "Dump/Print Packet in func: %s, line: %i", __FUNCTION__, __LINE__);
#if EQDEBUG >= 9
		packet->PrintPacket();
#endif
	}
	return packet;
}

PacketStruct* Player::GetQuestJournalPacket(Quest* quest, int16 version, int32 crc, bool updated) {
	if (!quest)
		return 0;

	PacketStruct* packet = configReader.getStruct("WS_QuestJournalUpdate", version);
	if (packet) {
		packet->setArrayLengthByName("num_quests", 1);
		packet->setArrayLengthByName("num_quest_zones", 1);
		packet->setArrayDataByName("quest_zones_zone", quest->GetType());
		packet->setArrayDataByName("quest_zones_zone_id", 0);
		
		if(!quest->GetDeleted() && !quest->GetCompleted())
			packet->setArrayDataByName("active", 1);

		packet->setArrayDataByName("name", quest->GetName());
		// don't see these two in the struct
		packet->setArrayDataByName("quest_type", quest->GetType());
		packet->setArrayDataByName("quest_zone", quest->GetZone());

		int8 display_status = QUEST_DISPLAY_STATUS_SHOW;
		if(quest->GetCompleted())
			packet->setArrayDataByName("completed", 1);
		if(quest->GetTurnedIn()) {
			packet->setArrayDataByName("turned_in", 1);
			packet->setArrayDataByName("completed", 1);
			packet->setArrayDataByName("visible", 1);	
			display_status += QUEST_DISPLAY_STATUS_COMPLETED;
		}		
		packet->setArrayDataByName("quest_id", quest->GetQuestID());
		packet->setArrayDataByName("day", quest->GetDay());
		packet->setArrayDataByName("month", quest->GetMonth());
		packet->setArrayDataByName("year", quest->GetYear());
		packet->setArrayDataByName("level", quest->GetQuestLevel());
		int8 difficulty = 0;
		string category = quest->GetType();
		if(category == "Tradeskill")
			difficulty = GetTSArrowColor(quest->GetQuestLevel());
		else
			difficulty = GetArrowColor(quest->GetQuestLevel());

		packet->setArrayDataByName("difficulty", difficulty);
		if (quest->GetEncounterLevel() > 4)
			packet->setArrayDataByName("encounter_level", quest->GetEncounterLevel());
		else
			packet->setArrayDataByName("encounter_level", 4);

		if (version >= 931) {
			packet->setArrayDataByName("zonetype_id", 0);
			packet->setArrayDataByName("zone_id", 0);
		}
		if(version >= 931 && quest->GetVisible()){
			if (quest->GetCompletedFlag())
				display_status += QUEST_DISPLAY_STATUS_COMPLETE_FLAG;
			else if (quest->IsRepeatable())
				display_status += QUEST_DISPLAY_STATUS_REPEATABLE;
			if (quest->GetYellowName() || quest->CheckCategoryYellow())
				display_status += QUEST_DISPLAY_STATUS_YELLOW;

			if (quest->IsTracked())
				display_status += QUEST_DISPLAY_STATUS_CHECK;
			else
				display_status += QUEST_DISPLAY_STATUS_NO_CHECK;

			if (quest->IsHidden() && !quest->GetTurnedIn()) {
				display_status += QUEST_DISPLAY_STATUS_HIDDEN;
				display_status -= QUEST_DISPLAY_STATUS_SHOW;
			}
			
			if(quest->CanShareQuestCriteria(GetClient(),false)) {
				display_status += QUEST_DISPLAY_STATUS_CAN_SHARE;
			}
		}
		else
			packet->setArrayDataByName("visible", quest->GetVisible());
		if (quest->IsRepeatable())
			packet->setArrayDataByName("repeatable", 1);
				
		packet->setArrayDataByName("display_status", display_status);
		if (updated) {
			packet->setArrayDataByName("quest_updated", 1);
			packet->setArrayDataByName("journal_updated", 1);
		}
		if(version >= 546)
			packet->setDataByName("unknown3", 1);
		packet->setDataByName("visible_quest_id", quest->GetQuestID());
		packet->setDataByName("player_crc", crc);
		packet->setDataByName("player_name", GetName());
		packet->setDataByName("used_quests", player_quests.size());
		packet->setDataByName("unknown4a", 1);
		packet->setDataByName("max_quests", 75);
	}

	return packet;
}

Quest* Player::SetStepComplete(int32 id, int32 step){
	Quest* ret = 0;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	if(player_quests.count(id) > 0){
		if(player_quests[id] && player_quests[id]->SetStepComplete(step))
			ret = player_quests[id];
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

Quest* Player::AddStepProgress(int32 quest_id, int32 step, int32 progress) {
	Quest* ret = 0;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	if (player_quests.count(quest_id) > 0) {
		if (player_quests[quest_id] && player_quests[quest_id]->AddStepProgress(step, progress))
			ret = player_quests[quest_id];
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

int32 Player::GetStepProgress(int32 quest_id, int32 step_id) {
	int32 ret = 0;

	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	if (player_quests.count(quest_id) > 0 && player_quests[quest_id])
		ret = player_quests[quest_id]->GetStepProgress(step_id);
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

void Player::RemoveQuest(int32 id, bool delete_quest){
	MPlayerQuests.writelock(__FUNCTION__, __LINE__);
	map<int32, Quest*>::iterator itr = player_quests.find(id);
	if(itr != player_quests.end()) {
		player_quests.erase(itr);
	}
	
	if(delete_quest){
		safe_delete(player_quests[id]);
	}
	
	MPlayerQuests.releasewritelock(__FUNCTION__, __LINE__);
	SendQuestRequiredSpawns(id);
}

vector<Quest*>* Player::CheckQuestsLocationUpdate(){
	vector<Quest*>* quest_updates = 0;
	map<int32, Quest*>::iterator itr;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	for(itr = player_quests.begin(); itr != player_quests.end(); itr++){
		if(itr->second && itr->second->CheckQuestLocationUpdate(GetX(), GetY(), GetZ(), (GetZoneID()))){
			if(!quest_updates)
				quest_updates = new vector<Quest*>();
			quest_updates->push_back(itr->second);
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return quest_updates;
}

vector<Quest*>* Player::CheckQuestsFailures(){
	vector<Quest*>* quest_failures = 0;
	map<int32, Quest*>::iterator itr;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	for(itr = player_quests.begin(); itr != player_quests.end(); itr++){
		if(itr->second && itr->second->GetQuestFailures()->size() > 0){
			if(!quest_failures)
				quest_failures = new vector<Quest*>();
			quest_failures->push_back(itr->second);
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return quest_failures;
}

vector<Quest*>* Player::CheckQuestsKillUpdate(Spawn* spawn, bool update){
	vector<Quest*>* quest_updates = 0;
	map<int32, Quest*>::iterator itr;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	for(itr = player_quests.begin(); itr != player_quests.end(); itr++){
		if(itr->second && itr->second->CheckQuestKillUpdate(spawn, update)){
			if(!quest_updates)
				quest_updates = new vector<Quest*>();
			quest_updates->push_back(itr->second);
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return quest_updates;
}

bool Player::HasQuestUpdateRequirement(Spawn* spawn){
	bool reqMet = false;
	map<int32, Quest*>::iterator itr;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	for(itr = player_quests.begin(); itr != player_quests.end(); itr++){
		if(itr->second && itr->second->CheckQuestReferencedSpawns(spawn)){
			reqMet = true;
			break;
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return reqMet;
}

vector<Quest*>* Player::CheckQuestsChatUpdate(Spawn* spawn){
	vector<Quest*>* quest_updates = 0;
	map<int32, Quest*>::iterator itr;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	for(itr = player_quests.begin(); itr != player_quests.end(); itr++){
		if(itr->second && itr->second->CheckQuestChatUpdate(spawn->GetDatabaseID())){
			if(!quest_updates)
				quest_updates = new vector<Quest*>();
			quest_updates->push_back(itr->second);
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return quest_updates;
}

int16 Player::GetTaskGroupStep(int32 quest_id){
	Quest* quest = 0;
	int16 step = 0;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	if(player_quests.count(quest_id) > 0){
		quest = player_quests[quest_id];
		if(quest) {
			step = quest->GetTaskGroupStep();
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return step;
}

bool Player::GetQuestStepComplete(int32 quest_id, int32 step_id){
	bool ret = false;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	if(player_quests.count(quest_id) > 0){
		Quest* quest = player_quests[quest_id];
		if ( quest != NULL )
			ret = quest->GetQuestStepCompleted(step_id);
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

int16 Player::GetQuestStep(int32 quest_id){
	Quest* quest = 0;
	int16 step = 0;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	if(player_quests.count(quest_id) > 0){
		quest = player_quests[quest_id];
		if(quest) {
			step = quest->GetQuestStep();
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return step;
}

map<int32, Quest*>*	Player::GetPlayerQuests(){
	return &player_quests;
}

map<int32, Quest*>*	Player::GetCompletedPlayerQuests(){
	return &completed_quests;
}

Quest* Player::GetAnyQuest(int32 quest_id) {
	if(player_quests.count(quest_id) > 0)
		return player_quests[quest_id];
	if(completed_quests.count(quest_id) > 0)
		return completed_quests[quest_id];
	
	return 0;
}
Quest* Player::GetCompletedQuest(int32 quest_id){
	if(completed_quests.count(quest_id) > 0)
		return completed_quests[quest_id];
	return 0;
}

bool Player::HasQuestBeenCompleted(int32 quest_id){
	bool ret = false;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	if(completed_quests.count(quest_id) > 0 && completed_quests[quest_id])
		ret = true;
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	
	return ret;
}

bool Player::HasActiveQuest(int32 quest_id){
	bool ret = false;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	if(player_quests.count(quest_id) > 0 && player_quests[quest_id])
		ret = true;
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	
	return ret;
}

bool Player::HasAnyQuest(int32 quest_id){
	bool ret = false;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	if(player_quests.count(quest_id) > 0)
		ret = true;
	if(completed_quests.count(quest_id) > 0)
		ret = true;
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	
	return ret;
}

int32 Player::GetQuestCompletedCount(int32 quest_id) {
	int32 count = 0;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	Quest* quest = GetCompletedQuest(quest_id);
	if(quest) {
		count = quest->GetCompleteCount();
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	return count;
}

Quest* Player::GetQuest(int32 quest_id){
	if(player_quests.count(quest_id) > 0)
		return player_quests[quest_id];
	return 0;
}

void Player::AddCompletedQuest(Quest* quest){
	Quest* existing = GetCompletedQuest(quest->GetQuestID());
	MPlayerQuests.writelock(__FUNCTION__, __LINE__);
	completed_quests[quest->GetQuestID()] = quest;
	if(existing && existing != quest) {
		safe_delete(existing);
	}
	
	quest->SetSaveNeeded(true);
	quest->SetTurnedIn(true);
	if(quest->GetCompletedDescription())
		quest->SetDescription(string(quest->GetCompletedDescription()));
	quest->SetUpdateRequired(true);
	MPlayerQuests.releasewritelock(__FUNCTION__, __LINE__);
}

bool Player::CheckQuestRemoveFlag(Spawn* spawn){
	if(current_quest_flagged.count(spawn) > 0){
		current_quest_flagged.erase(spawn);
		return true;
	}
	return false;
}

bool Player::CheckQuestRequired(Spawn* spawn){
	if(spawn)
		return spawn->MeetsSpawnAccessRequirements(this);
	return false;
}

int8 Player::CheckQuestFlag(Spawn* spawn){
	int8 ret = 0;

	if (!spawn) {
		LogWrite(PLAYER__ERROR, 0, "Player", "CheckQuestFlag() called with an invalid spawn");
		return ret;
	}
	if(spawn->HasProvidedQuests()){
		vector<int32>* quests = spawn->GetProvidedQuests();
		Quest* quest = 0;
		for(int32 i=0;i<quests->size();i++){
			MPlayerQuests.readlock(__FUNCTION__, __LINE__);
			if(player_quests.count(quests->at(i)) > 0){
				if(player_quests[quests->at(i)] && player_quests[quests->at(i)]->GetCompleted() && player_quests[quests->at(i)]->GetQuestReturnNPC() == spawn->GetDatabaseID()){
					ret = 2;
					MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
					break;
				}
			}
			MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
			int8 flag = 0;
			if (CanReceiveQuest(quests->at(i), &flag)){
				if(flag) {
					ret = flag;
					break;
				}
				master_quest_list.LockQuests();
				quest = master_quest_list.GetQuest(quests->at(i), false);
				master_quest_list.UnlockQuests();
				if(quest){
					int8 color = quest->GetFeatherColor();
					// purple
					if (color == 1)
						ret = 16;
					// green
					else if (color == 2)
						ret = 32;
					// blue
					else if (color == 3)
						ret = 64;
					// normal
					else
						ret = 1;
					break;
				}
			}
		}
	}
	map<int32, Quest*>::iterator itr;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	for(itr = player_quests.begin(); itr != player_quests.end(); itr++){
		// must make sure the quest ptr is alive or nullptr
		if(itr->second && itr->second->CheckQuestChatUpdate(spawn->GetDatabaseID(), false))
			ret = 2;
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	if(ret > 0)
		current_quest_flagged[spawn] = true;
	return ret;
}

bool Player::CanReceiveQuest(int32 quest_id, int8* ret){
	bool passed = true;
	int32 x;
	master_quest_list.LockQuests();
	Quest* quest = master_quest_list.GetQuest(quest_id, false);
	master_quest_list.UnlockQuests();
	if (!quest)
		passed = false;
	//check if quest is already completed, and not repeatable
	else if (HasQuestBeenCompleted(quest_id) && !quest->IsRepeatable())
		passed = false;
	//check if the player already has this quest
	else if (player_quests.count(quest_id) > 0)
		passed = false;
	//Check Prereq Adv Levels
	else if (quest->GetPrereqLevel() > GetLevel())
		passed = false;
	else if (quest->GetPrereqMaxLevel() > 0){
		if (GetLevel() > quest->GetPrereqMaxLevel())
			passed = false;
	}
	//Check Prereq TS Levels
	else if (quest->GetPrereqTSLevel() > GetTSLevel())
		passed = false;
	else if (quest->GetPrereqMaxTSLevel() > 0){
		if (GetTSLevel() > quest->GetPrereqMaxLevel())
			passed = false;
	}


	// Check quest pre req
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	vector<int32>* prereq_quests = quest->GetPrereqQuests();
	if(passed && prereq_quests && prereq_quests->size() > 0){
		for(int32 x=0;x<prereq_quests->size();x++){
			if(completed_quests.count(prereq_quests->at(x)) == 0){
				passed = false;
				break;
			}
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);

	//Check Prereq Classes
	vector<int8>* prereq_classes = quest->GetPrereqClasses();
	if(passed && prereq_classes && prereq_classes->size() > 0){
		for(int32 x=0;x<prereq_classes->size();x++){
			if(prereq_classes->at(x) == GetAdventureClass()){
				passed = true;
				break;
			}
			else
				passed = false;
		}
	}

	//Check Prereq TS Classes
	vector<int8>* prereq_tsclasses = quest->GetPrereqTradeskillClasses();
	if(passed && prereq_tsclasses && prereq_tsclasses->size() > 0){
		for( x=0;x<prereq_tsclasses->size();x++){
			if(prereq_tsclasses->at(x) == GetTradeskillClass()){
				passed = true;
				break;
			}
			else
				passed = false;
		}
	}


	// Check model prereq
	vector<int16>* prereq_model_types = quest->GetPrereqModelTypes();
	if(passed && prereq_model_types && prereq_model_types->size() > 0){
		for(x=0;x<prereq_model_types->size();x++){
			if(prereq_model_types->at(x) == GetModelType()){
				passed = true;
				break;
			}
			else
				passed = false;
		}
	}


	// Check faction pre req
	vector<QuestFactionPrereq>* prereq_factions = quest->GetPrereqFactions();
	if(passed && prereq_factions && prereq_factions->size() > 0){
		sint32 val = 0;
		for(x=0;x<prereq_factions->size();x++){
			val = GetFactions()->GetFactionValue(prereq_factions->at(x).faction_id);
			if(val >= prereq_factions->at(x).min && (prereq_factions->at(x).max == 0 || val <= prereq_factions->at(x).max)){
				passed = true;
				break;
			}
			else
				passed = false;
		}
	}

	LogWrite(MISC__TODO, 1, "TODO", "Check prereq items\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);

	// Check race pre req
	vector<int8>* prereq_races = quest->GetPrereqRaces();
	if(passed && prereq_races && prereq_races->size() > 0){
		for(x=0;x<prereq_races->size();x++){
			if(prereq_races->at(x) == GetRace()){
				passed = true;
				break;
			}
			else
				passed = false;
		}
	}
	
	int32 flag = 0;
	if(lua_interface->CallQuestFunction(quest, "ReceiveQuestCriteria", this, 0xFFFFFFFF, &flag)) {
		if(ret)
			*ret = flag;
		if(!flag) {
			passed = false;
		}
		else {
			passed = true;
		}
	}

	return passed;
}

bool Player::UpdateQuestReward(int32 quest_id, QuestRewardData* qrd) {
	if(!GetClient())
		return false;
	
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	Quest* quest = GetAnyQuest(quest_id);
	
	if(!quest) {
		MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
		return false;
	}
	
	quest->SetQuestTemporaryState(qrd->is_temporary, qrd->description);
	if(qrd->is_temporary) {
		quest->SetStatusTmpReward(qrd->tmp_status);
		quest->SetCoinTmpReward(qrd->tmp_coin);
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	
	GetClient()->GiveQuestReward(quest, qrd->has_displayed);
	SetActiveReward(true);
	
	return true;
}


Quest* Player::PendingQuestAcceptance(int32 quest_id, int32 item_id, bool* quest_exists) {
	vector<Item*>* items = 0;
	bool ret = false;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	Quest* quest = GetAnyQuest(quest_id);
	if(!quest) {
		if(quest_exists) {
			*quest_exists = false;
		}
		MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
		return nullptr;
	}
	
	if(quest_exists) {
		*quest_exists = true;
	}
	if(quest->GetQuestTemporaryState())
		items = quest->GetTmpRewardItems();
	else
		items = quest->GetRewardItems();
	if (item_id == 0) {
		ret = true;
	}
	else {
		items = quest->GetSelectableRewardItems();
		if (items && items->size() > 0) {
			for (int32 i = 0; i < items->size(); i++) {
				if (items->at(i)->details.item_id == item_id) {
					ret = true;
					break;
				}
			}
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);

	return quest;
}


bool Player::AcceptQuestReward(int32 item_id, int32 selectable_item_id) {
	if(!GetClient()) {
		return false;
	}
	
	Collection *collection = 0;
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	Quest* quest = client->GetPendingQuestAcceptance(item_id);
	if(quest){
		GetClient()->AcceptQuestReward(quest, item_id);
		MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
		return true;
	}
	bool collectedItems = false;
	if (client->GetPlayer()->HasPendingItemRewards()) {
		vector<Item*> items = client->GetPlayer()->GetPendingItemRewards();
		if (items.size() > 0) {
			collectedItems = true;
			for (int i = 0; i < items.size(); i++) {
				client->GetPlayer()->AddItem(new Item(items[i]));
			}
			client->GetPlayer()->ClearPendingItemRewards();
			client->GetPlayer()->SetActiveReward(false);
		}
		map<int32, Item*> selectable_item = client->GetPlayer()->GetPendingSelectableItemReward(item_id);
		if (selectable_item.size() > 0) {
			collectedItems = true;
			map<int32, Item*>::iterator itr;
			for (itr = selectable_item.begin(); itr != selectable_item.end(); itr++) {
				client->GetPlayer()->AddItem(new Item(itr->second));
				client->GetPlayer()->ClearPendingSelectableItemRewards(itr->first);
			}
			client->GetPlayer()->SetActiveReward(false);
		}
	}
	else if (collection = GetPendingCollectionReward())
	{
		client->AcceptCollectionRewards(collection, (selectable_item_id > 0) ? selectable_item_id : item_id);
		collectedItems = true;
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	
	return collectedItems;
}


bool Player::SendQuestStepUpdate(int32 quest_id, int32 quest_step_id, bool display_quest_helper) {	
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	Quest* quest = GetAnyQuest(quest_id);
	if(!quest) {
		MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
		return false;
	}
	
	QuestStep* quest_step = quest->GetQuestStep(quest_step_id);
	if (quest_step) {
		if(GetClient()) {
			GetClient()->QueuePacket(quest->QuestJournalReply(GetClient()->GetVersion(), GetClient()->GetNameCRC(), this, quest_step, 1, false, false, display_quest_helper));
		}
		quest_step->WasUpdated(false);
	}
	bool turnedIn = quest->GetTurnedIn();
	
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
	
	if(turnedIn && GetClient()) //update the journal so the old quest isn't the one displayed in the client's quest helper
		GetClient()->SendQuestJournal();

	return true;
}

void Player::SendQuest(int32 quest_id) {
	if(!GetClient()) {
		return;
	}

	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	Quest* quest = GetQuest(quest_id);
	if (quest)
		GetClient()->QueuePacket(quest->QuestJournalReply(GetClient()->GetVersion(), GetClient()->GetNameCRC(), this));
	else {
		quest = GetCompletedQuest(quest_id);
		
		if (quest)
			GetClient()->QueuePacket(quest->QuestJournalReply(GetClient()->GetVersion(), GetClient()->GetNameCRC(), this, 0, 1, true));
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
}


void Player::UpdateQuestCompleteCount(int32 quest_id) {
	if(!GetClient()) {
		return;
	}
	
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	// If character has already completed this quest once update the given date in the database
	Quest* quest = GetQuest(id);
	Quest* quest2 = GetCompletedQuest(id);
	if (quest && quest2) {
		quest->SetCompleteCount(quest2->GetCompleteCount());
		database.SaveCharRepeatableQuest(GetClient(), id, quest->GetCompleteCount());
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
}

void Player::GetQuestTemporaryRewards(int32 quest_id, std::vector<Item*>* items) {
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	Quest* quest = GetAnyQuest(quest_id);
	if(quest) {
		quest->GetTmpRewardItemsByID(items);
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
}

void Player::AddQuestTemporaryReward(int32 quest_id, int32 item_id, int16 item_count) {
	MPlayerQuests.readlock(__FUNCTION__, __LINE__);
	Quest* quest = GetAnyQuest(quest_id);
	if(quest) {
		Item* item = master_item_list.GetItem(item_id);
		if(item) {
			Item* tmpItem = new Item(item);
			tmpItem->details.count = item_count;
			quest->AddTmpRewardItem(tmpItem);
		}
	}
	MPlayerQuests.releasereadlock(__FUNCTION__, __LINE__);
}

bool Player::ShouldSendSpawn(Spawn* spawn){
	if(spawn->IsDeletedSpawn() || IsSendingSpawn(spawn->GetID()) || IsRemovingSpawn(spawn->GetID()))
		return false;
	else if((WasSentSpawn(spawn->GetID()) == false) && (!spawn->IsPrivateSpawn() || spawn->AllowedAccess(this)))
		return true;
	
	return false;
}

int8 Player::GetTSArrowColor(int8 level){
	int8 color = 0;
	sint16 diff = level - GetTSLevel();
	if(GetLevel() < 10)
		diff *= 3;
	else if(GetLevel() <= 20)
		diff *= 2;
	if(diff >= 9)
		color = ARROW_COLOR_RED;
	else if(diff >= 5)
		color = ARROW_COLOR_ORANGE;
	else if(diff >= 1)
		color = ARROW_COLOR_YELLOW;
	else if(diff == 0)
		color = ARROW_COLOR_WHITE;	
	else if(diff <= -11)
		color = ARROW_COLOR_GRAY;
	else if(diff <= -6)
		color = ARROW_COLOR_GREEN;
	else //if(diff < 0)
		color = ARROW_COLOR_BLUE;
	return color;
}

void Player::AddCoins(int64 val){
	int32 tmp = 0;
	UpdatePlayerStatistic(STAT_PLAYER_TOTAL_WEALTH, (GetCoinsCopper() + (GetCoinsSilver() * 100) + (GetCoinsGold() * 10000) + (GetCoinsPlat() * 1000000)) + val, true);
	if(val >= 1000000){
		tmp = val / 1000000;
		val -= tmp*1000000;
		GetInfoStruct()->add_coin_plat(tmp);
	}
	if(val >= 10000){
		tmp = val / 10000;
		val -= tmp*10000;
		GetInfoStruct()->add_coin_gold(tmp);
	}
	if(val >= 100){
		tmp = val / 100;
		val -= tmp*100;
		GetInfoStruct()->add_coin_silver(tmp);
	}
	GetInfoStruct()->add_coin_copper(val);
	int32 new_copper_value = GetInfoStruct()->get_coin_copper();
	if(new_copper_value >= 100){
		tmp = new_copper_value/100;
		GetInfoStruct()->set_coin_copper(new_copper_value - (100 * tmp));
		GetInfoStruct()->add_coin_silver(tmp);
	}
	int32 new_silver_value = GetInfoStruct()->get_coin_silver();
	if(new_silver_value >= 100){
		tmp = new_silver_value/100;
		GetInfoStruct()->set_coin_silver(new_silver_value - (100 * tmp));
		GetInfoStruct()->add_coin_gold(tmp);
	}
	int32 new_gold_value = GetInfoStruct()->get_coin_gold();
	if(new_gold_value >= 100){
		tmp = new_gold_value/100;
		GetInfoStruct()->set_coin_gold(new_gold_value - (100 * tmp));
		GetInfoStruct()->add_coin_plat(tmp);
	}
	charsheet_changed = true;
}

bool Player::RemoveCoins(int64 val){
	int64 total_coins = GetInfoStruct()->get_coin_plat()*1000000;
	total_coins += GetInfoStruct()->get_coin_gold()*10000;
	total_coins += GetInfoStruct()->get_coin_silver()*100;
	total_coins += GetInfoStruct()->get_coin_copper();
	if(total_coins >= val){
		total_coins -= val;
		GetInfoStruct()->set_coin_plat(0);
		GetInfoStruct()->set_coin_gold(0);
		GetInfoStruct()->set_coin_silver(0);
		GetInfoStruct()->set_coin_copper(0);
		AddCoins(total_coins);
		return true;
	}
	return false;
}

bool Player::HasCoins(int64 val) {
	int64 total_coins = GetInfoStruct()->get_coin_plat()*1000000;
	total_coins += GetInfoStruct()->get_coin_gold()*10000;
	total_coins += GetInfoStruct()->get_coin_silver()*100;
	total_coins += GetInfoStruct()->get_coin_copper();
	if(total_coins >= val)
		return true;

	return false;
}

bool Player::HasPendingLootItems(int32 id){
	return (pending_loot_items.count(id) > 0 && pending_loot_items[id].size() > 0);
}

bool Player::HasPendingLootItem(int32 id, int32 item_id){
	return (pending_loot_items.count(id) > 0 && pending_loot_items[id].count(item_id) > 0);
}
vector<Item*>* Player::GetPendingLootItems(int32 id){
	vector<Item*>* ret = 0;
	if(pending_loot_items.count(id) > 0){
		ret = new vector<Item*>();
		map<int32, bool>::iterator itr;
		for(itr = pending_loot_items[id].begin(); itr != pending_loot_items[id].end(); itr++){
			if(master_item_list.GetItem(itr->first))
				ret->push_back(master_item_list.GetItem(itr->first));
		}
	}
	return ret;
}

void Player::RemovePendingLootItem(int32 id, int32 item_id){
	if(pending_loot_items.count(id) > 0){
		pending_loot_items[id].erase(item_id);
		if(pending_loot_items[id].size() == 0)
			pending_loot_items.erase(id);
	}
}

void Player::RemovePendingLootItems(int32 id){
	if(pending_loot_items.count(id) > 0)
		pending_loot_items.erase(id);
}

void Player::AddPendingLootItems(int32 id, vector<Item*>* items){
	if(items){
		Item* item = 0;
		for(int32 i=0;i<items->size();i++){
			item = items->at(i);
			if(item)
				pending_loot_items[id][item->details.item_id] = true;
		}
	}
}

void Player::AddPlayerStatistic(int32 stat_id, sint32 stat_value, int32 stat_date) {
	if (statistics.count(stat_id) == 0) {
		Statistic* stat = new Statistic;
		stat->stat_id = stat_id;
		stat->stat_value = stat_value;
		stat->stat_date = stat_date;
		stat->save_needed = false;
		statistics[stat_id] = stat;
	}
}

void Player::UpdatePlayerStatistic(int32 stat_id, sint32 stat_value, bool overwrite) {
	if (statistics.count(stat_id) == 0)
		AddPlayerStatistic(stat_id, 0, 0);
	Statistic* stat = statistics[stat_id];
	overwrite == true ? stat->stat_value = stat_value : stat->stat_value += stat_value;
	stat->stat_date = Timer::GetUnixTimeStamp();
	stat->save_needed = true;
}

void Player::WritePlayerStatistics() {
	map<int32, Statistic*>::iterator stat_itr;
	for (stat_itr = statistics.begin(); stat_itr != statistics.end(); stat_itr++) {
		Statistic* stat = stat_itr->second;
		if (stat->save_needed) {
			stat->save_needed = false;
			database.WritePlayerStatistic(this, stat);
		}
	}
}

sint64 Player::GetPlayerStatisticValue(int32 stat_id) {
	if (stat_id >= 0 && statistics.count(stat_id) > 0)
		return statistics[stat_id]->stat_value;
	return 0;
}

void Player::RemovePlayerStatistics() {
	map<int32, Statistic*>::iterator stat_itr;
	for (stat_itr = statistics.begin(); stat_itr != statistics.end(); stat_itr++)
		safe_delete(stat_itr->second);
	statistics.clear();
}

void Player::SetGroup(PlayerGroup* new_group){
	group = new_group;
}

/*PlayerGroup* Player::GetGroup(){
	return group;
}*/

bool Player::IsGroupMember(Entity* player) {
	bool ret = false;
	if (GetGroupMemberInfo() && player) {
		//world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);
		ret = world.GetGroupManager()->IsInGroup(GetGroupMemberInfo()->group_id, player);

		/*deque<GroupMemberInfo*>::iterator itr;
		deque<GroupMemberInfo*>* members = world.GetGroupManager()->GetGroupMembers(GetGroupMemberInfo()->group_id);
		for (itr = members->begin(); itr != members->end(); itr++) {
			GroupMemberInfo* gmi = *itr;
			if (gmi->client && gmi->client->GetPlayer() == player) {
				ret = true;
				break;
			}
		}

		world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);*/
	}
	return ret;
}





void Player::SetGroupInformation(PacketStruct* packet){
	int8 det_count = 0;
	Entity* member = 0;

	world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);
	if (GetGroupMemberInfo()) {
		PlayerGroup* group = world.GetGroupManager()->GetGroup(GetGroupMemberInfo()->group_id);
		if (group)
		{
			group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
			deque<GroupMemberInfo*>* members = group->GetMembers();
			deque<GroupMemberInfo*>::iterator itr;
			GroupMemberInfo* info = 0;
			int x = 0;

			for (itr = members->begin(); itr != members->end(); itr++) {
				info = *itr;

				if (info == GetGroupMemberInfo()) {
					if (info->leader)
						packet->setDataByName("group_leader_id", 0xFFFFFFFF);	// If this player is the group leader then fill this element with FF FF FF FF

					continue;
				}
				else {
					if (info->leader)
						packet->setDataByName("group_leader_id", x);			// If leader is some one else then fill with the slot number they are in
				}

				member = info->member;

				if (member && member->GetZone() == GetZone()) {
					packet->setSubstructDataByName("group_members", "spawn_id", GetIDWithPlayerSpawn(member), x);

					if (member->HasPet()) {
						if (member->GetPet())
							packet->setSubstructDataByName("group_members", "pet_id", GetIDWithPlayerSpawn(member->GetPet()), x);
						else
							packet->setSubstructDataByName("group_members", "pet_id", GetIDWithPlayerSpawn(member->GetCharmedPet()), x);
					}
					else
						packet->setSubstructDataByName("group_members", "pet_id", 0xFFFFFFFF, x);

					//Send detriment counts as 255 if all dets of that type are incurable
					det_count = member->GetTraumaCount();
					if (det_count > 0) {
						if (!member->HasCurableDetrimentType(DET_TYPE_TRAUMA))
							det_count = 255;
					}
					packet->setSubstructDataByName("group_members", "trauma_count", det_count, x);

					det_count = member->GetArcaneCount();
					if (det_count > 0) {
						if (!member->HasCurableDetrimentType(DET_TYPE_ARCANE))
							det_count = 255;
					}
					packet->setSubstructDataByName("group_members", "arcane_count", det_count, x);

					det_count = member->GetNoxiousCount();
					if (det_count > 0) {
						if (!member->HasCurableDetrimentType(DET_TYPE_NOXIOUS))
							det_count = 255;
					}
					packet->setSubstructDataByName("group_members", "noxious_count", det_count, x);

					det_count = member->GetElementalCount();
					if (det_count > 0) {
						if (!member->HasCurableDetrimentType(DET_TYPE_ELEMENTAL))
							det_count = 255;
					}
					packet->setSubstructDataByName("group_members", "elemental_count", det_count, x);

					det_count = member->GetCurseCount();
					if (det_count > 0) {
						if (!member->HasCurableDetrimentType(DET_TYPE_CURSE))
							det_count = 255;
					}
					packet->setSubstructDataByName("group_members", "curse_count", det_count, x);

					packet->setSubstructDataByName("group_members", "zone_status", 1, x);
				}
				else {
					packet->setSubstructDataByName("group_members", "pet_id", 0xFFFFFFFF, x);
					//packet->setSubstructDataByName("group_members", "unknown5", 1, x, 1); // unknown5 > 1 = name is blue
					packet->setSubstructDataByName("group_members", "zone_status", 2, x);
				}

				packet->setSubstructDataByName("group_members", "name", info->name.c_str(), x);
				packet->setSubstructDataByName("group_members", "hp_current", info->hp_current, x);
				packet->setSubstructDataByName("group_members", "hp_max", info->hp_max, x);
				packet->setSubstructDataByName("group_members", "power_current", info->power_current, x);
				packet->setSubstructDataByName("group_members", "power_max", info->power_max, x);
				packet->setSubstructDataByName("group_members", "level_current", info->level_current, x);
				packet->setSubstructDataByName("group_members", "level_max", info->level_max, x);
				packet->setSubstructDataByName("group_members", "zone", info->zone.c_str(), x);
				packet->setSubstructDataByName("group_members", "race_id", info->race_id, x);
				packet->setSubstructDataByName("group_members", "class_id", info->class_id, x);

				x++;
			}
		}
		group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
	}
	//packet->PrintPacket();
	world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
}

PlayerItemList*	Player::GetPlayerItemList(){
	return &item_list;
}

void Player::ResetSavedSpawns(){
	spawn_mutex.writelock(__FUNCTION__, __LINE__);
	ClearRemovalTimers();
	spawn_packet_sent.clear();
	spawn_mutex.releasewritelock(__FUNCTION__, __LINE__);

	info_mutex.writelock(__FUNCTION__, __LINE__);
	spawn_info_packet_list.clear();
	info_mutex.releasewritelock(__FUNCTION__, __LINE__);

	vis_mutex.writelock(__FUNCTION__, __LINE__);
	spawn_vis_packet_list.clear();
	vis_mutex.releasewritelock(__FUNCTION__, __LINE__);

	pos_mutex.writelock(__FUNCTION__, __LINE__);
	spawn_pos_packet_list.clear();
	pos_mutex.releasewritelock(__FUNCTION__, __LINE__);

	index_mutex.writelock(__FUNCTION__, __LINE__);
	player_spawn_reverse_id_map.clear();
	player_spawn_id_map.clear();
	player_spawn_id_map[1] = this;
	player_spawn_reverse_id_map[this] = 1;
	spawn_index = 1;
	index_mutex.releasewritelock(__FUNCTION__, __LINE__);

	m_playerSpawnQuestsRequired.writelock(__FUNCTION__, __LINE__);
	player_spawn_quests_required.clear();
	m_playerSpawnQuestsRequired.releasewritelock(__FUNCTION__, __LINE__);
	if(info)
		info->RemoveOldPackets();
	
	safe_delete_array(movement_packet);
	safe_delete_array(old_movement_packet);
}

void Player::SetReturningFromLD(bool val){
    std::unique_lock lock(spell_packet_update_mutex);
	if(val && val != returning_from_ld)
	{
		if(GetPlayerItemList())
			GetPlayerItemList()->ResetPackets();
		
		GetEquipmentList()->ResetPackets();
		GetAppearanceEquipmentList()->ResetPackets();
		skill_list.ResetPackets();
		safe_delete_array(spell_orig_packet);
		safe_delete_array(spell_xor_packet);
		spell_orig_packet=0;
		spell_xor_packet=0;
		spell_count = 0;
		
		safe_delete_array(raid_orig_packet);
		safe_delete_array(raid_xor_packet);
		raid_orig_packet=0;
		raid_xor_packet=0;

		reset_character_flag(CF_IS_SITTING);
		if (GetActivityStatus() & ACTIVITY_STATUS_CAMPING)
			SetActivityStatus(GetActivityStatus() - ACTIVITY_STATUS_CAMPING);

		if (GetActivityStatus() & ACTIVITY_STATUS_LINKDEAD)
			SetActivityStatus(GetActivityStatus() - ACTIVITY_STATUS_LINKDEAD);
		
		SetTempVisualState(0);

		safe_delete_array(spawn_tmp_info_xor_packet);
		safe_delete_array(spawn_tmp_vis_xor_packet);
		safe_delete_array(spawn_tmp_pos_xor_packet);
		spawn_tmp_info_xor_packet = 0;
		spawn_tmp_vis_xor_packet = 0;
		spawn_tmp_pos_xor_packet = 0;
		pos_xor_size = 0;
		info_xor_size = 0;
		vis_xor_size = 0;
		
		index_mutex.writelock(__FUNCTION__, __LINE__);
		player_spawn_id_map[1] = this;
		player_spawn_reverse_id_map[this] = 1;
		spawn_index = 1;
		index_mutex.releasewritelock(__FUNCTION__, __LINE__);
	}
	
	returning_from_ld = val;
}

bool Player::IsReturningFromLD(){
	return returning_from_ld;
}

void Player::AddFriend(const char* name, bool save){
	if(name){
		if(save)
			friend_list[name] = 1;
		else
			friend_list[name] = 0;
	}
}

bool Player::IsFriend(const char* name){
	if(name && friend_list.count(name) > 0 && friend_list[name] < 2)
		return true;
	return false;
}

void Player::RemoveFriend(const char* name){
	if(friend_list.count(name) > 0)
		friend_list[name] = 2;
}

map<string, int8>* Player::GetFriends(){
	return &friend_list;
}

void Player::AddIgnore(const char* name, bool save){
	if(name){
		if(save)
			ignore_list[name] = 1;
		else
			ignore_list[name] = 0;
	}
}

bool Player::IsIgnored(const char* name){
	if(name && ignore_list.count(name) > 0 && ignore_list[name] < 2)
		return true;
	return false;
}

void Player::RemoveIgnore(const char* name){
	if(name && ignore_list.count(name) > 0)
		ignore_list[name] = 2;
}

map<string, int8>* Player::GetIgnoredPlayers(){
	return &ignore_list;
}

bool Player::CheckLevelStatus(int16 new_level) {
	int16 LevelCap					= rule_manager.GetGlobalRule(R_Player, MaxLevel)->GetInt16();
	int16 LevelCapOverrideStatus	= rule_manager.GetGlobalRule(R_Player, MaxLevelOverrideStatus)->GetInt16();
	int16 MaxLevelPlayer			= GetInfoStruct()->get_max_level();
	if ( GetClient() && (LevelCap < new_level) && (LevelCapOverrideStatus > GetClient()->GetAdminStatus()) && (MaxLevelPlayer < 1 || MaxLevelPlayer < new_level) )
			return false;
	return true;
}

Skill* Player::GetSkillByName(const char* name, bool check_update){
	Skill* ret = skill_list.GetSkillByName(name);
	if(check_update)
		{
			if(skill_list.CheckSkillIncrease(ret))
				CalculateBonuses();
		}
	return ret;
}

Skill* Player::GetSkillByID(int32 id, bool check_update){
	Skill* ret = skill_list.GetSkill(id);
	if(check_update)
		{
			if(skill_list.CheckSkillIncrease(ret))
				CalculateBonuses();
		}
	return ret;
}

void Player::SetRangeAttack(bool val){
	range_attack = val;
}

bool Player::GetRangeAttack(){
	return range_attack;
}

bool Player::AddMail(Mail* mail) {
	bool ret = false;
	if (mail) {
		mail_list.Put(mail->mail_id, mail);
		ret = true;
	}
	return ret;
}

MutexMap<int32, Mail*>* Player::GetMail() {
	return &mail_list;
}

Mail* Player::GetMail(int32 mail_id) {
	Mail* mail = 0;
	if (mail_list.count(mail_id) > 0)
		mail = mail_list.Get(mail_id);
	return mail;
}

void Player::DeleteMail(bool from_database) {
	MutexMap<int32, Mail*>::iterator itr = mail_list.begin();
	while (itr.Next())
		DeleteMail(itr->first, from_database);
	mail_list.clear();
}

void Player::DeleteMail(int32 mail_id, bool from_database) {
	if (mail_list.count(mail_id) > 0) {
		if (from_database)
			database.DeletePlayerMail(mail_list.Get(mail_id));
		mail_list.erase(mail_id, true, true); // need to delete the mail ptr
	}
}

/*			CharacterInstances			*/

CharacterInstances::CharacterInstances() {
	m_instanceList.SetName("Mutex::m_instanceList");
}

CharacterInstances::~CharacterInstances() {
	RemoveInstances();
}

void CharacterInstances::AddInstance(int32 db_id, int32 instance_id, int32 last_success_timestamp, int32 last_failure_timestamp, int32 success_lockout_time, int32 failure_lockout_time, int32 zone_id, int8 zone_instancetype, string zone_name) {
	InstanceData data;
	data.db_id = db_id;
	data.instance_id = instance_id;
	data.zone_id = zone_id;
	data.zone_instance_type = zone_instancetype;
	data.zone_name = zone_name;
	data.last_success_timestamp = last_success_timestamp;
	data.last_failure_timestamp = last_failure_timestamp;
	data.success_lockout_time = success_lockout_time;
	data.failure_lockout_time = failure_lockout_time;
	instanceList.push_back(data);
}

void CharacterInstances::RemoveInstances() {
	instanceList.clear();
}

bool CharacterInstances::RemoveInstanceByZoneID(int32 zone_id) {
	vector<InstanceData>::iterator itr;
	m_instanceList.writelock(__FUNCTION__, __LINE__);
	for(itr = instanceList.begin(); itr != instanceList.end(); itr++) {
		InstanceData* data = &(*itr);
		if (data->zone_id == zone_id) {
			instanceList.erase(itr);
			m_instanceList.releasewritelock(__FUNCTION__, __LINE__);
			return true;
		}
	}
	m_instanceList.releasewritelock(__FUNCTION__, __LINE__);
	return false;
}

bool CharacterInstances::RemoveInstanceByInstanceID(int32 instance_id) {
	vector<InstanceData>::iterator itr;
	m_instanceList.writelock(__FUNCTION__, __LINE__);
	for(itr = instanceList.begin(); itr != instanceList.end(); itr++) {
		InstanceData* data = &(*itr);
		if (data->instance_id == instance_id) {
			instanceList.erase(itr);
			m_instanceList.releasewritelock(__FUNCTION__, __LINE__);
			return true;
		}
	}
	m_instanceList.releasewritelock(__FUNCTION__, __LINE__);
	return false;
}

InstanceData* CharacterInstances::FindInstanceByZoneID(int32 zone_id) {
	m_instanceList.readlock(__FUNCTION__, __LINE__);
	for(int32 i = 0; i < instanceList.size(); i++) {
		InstanceData* data = &instanceList.at(i);
		if (data->zone_id == zone_id) {
			m_instanceList.releasereadlock(__FUNCTION__, __LINE__);
			return data;
		}
	}
	m_instanceList.releasereadlock(__FUNCTION__, __LINE__);

	return 0;
}

InstanceData* CharacterInstances::FindInstanceByDBID(int32 db_id) {
	m_instanceList.readlock(__FUNCTION__, __LINE__);
	for(int32 i = 0; i < instanceList.size(); i++) {
		InstanceData* data = &instanceList.at(i);
		if (data->db_id == db_id) {
			m_instanceList.releasereadlock(__FUNCTION__, __LINE__);
			return data;
		}
	}
	m_instanceList.releasereadlock(__FUNCTION__, __LINE__);

	return 0;
}

InstanceData* CharacterInstances::FindInstanceByInstanceID(int32 instance_id) {
	m_instanceList.readlock(__FUNCTION__, __LINE__);
	for(int32 i = 0; i < instanceList.size(); i++) {
		InstanceData* data = &instanceList.at(i);
		if (data->instance_id == instance_id) {
			m_instanceList.releasereadlock(__FUNCTION__, __LINE__);
			return data;
		}
	}
	m_instanceList.releasereadlock(__FUNCTION__, __LINE__);

	return 0;
}
vector<InstanceData> CharacterInstances::GetLockoutInstances() {
	vector<InstanceData> ret;
	m_instanceList.readlock(__FUNCTION__, __LINE__);
	for (int32 i = 0; i < instanceList.size(); i++) {
		InstanceData* data = &instanceList.at(i);
		if (data->zone_instance_type == SOLO_LOCKOUT_INSTANCE || data->zone_instance_type == GROUP_LOCKOUT_INSTANCE || data->zone_instance_type == RAID_LOCKOUT_INSTANCE)
			ret.push_back(*data);
	}
	m_instanceList.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

vector<InstanceData> CharacterInstances::GetPersistentInstances() {
	vector<InstanceData> ret;
	m_instanceList.readlock(__FUNCTION__, __LINE__);
	for (int32 i = 0; i < instanceList.size(); i++) {
		InstanceData* data = &instanceList.at(i);
		if (data->zone_instance_type == SOLO_PERSIST_INSTANCE || data->zone_instance_type == GROUP_PERSIST_INSTANCE || data->zone_instance_type == RAID_PERSIST_INSTANCE)
			ret.push_back(*data);
	}
	m_instanceList.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

void CharacterInstances::ProcessInstanceTimers(Player* player) {

	// Only need to check persistent instances here, lockout should be handled by zone shutting down

	// Check instance id, if > 0 check timers, if timers expired  set instance id to 0 and update the db `character_instance` to instance id 0,
	// delete instance from `instances` if no more characters assigned to it
	
	m_instanceList.readlock(__FUNCTION__, __LINE__);
	for (int32 i = 0; i < instanceList.size(); i++) {
		InstanceData* data = &instanceList.at(i);

		// Check to see if we have a valid instance and if it is persistant
		if (data->instance_id > 0) {
			
			if (data->zone_instance_type == SOLO_PERSIST_INSTANCE || data->zone_instance_type == GROUP_PERSIST_INSTANCE || data->zone_instance_type == RAID_PERSIST_INSTANCE) {
				// Check max duration (last success + success time)
				// if the zone does not have a success lockout time, we should not apply this logic
				if (data->success_lockout_time > 0 && (Timer::GetUnixTimeStamp() >= (data->last_success_timestamp + data->success_lockout_time))) {
					// Max duration has passed, instance has expired lets remove the player from it,
					// this will also delete the instace if all players have been removed from it
					database.DeleteCharacterFromInstance(player->GetCharacterID(), data->instance_id);
					data->instance_id = 0;
				}
			}

			if (data->zone_instance_type == SOLO_LOCKOUT_INSTANCE || data->zone_instance_type == GROUP_LOCKOUT_INSTANCE || data->zone_instance_type == RAID_LOCKOUT_INSTANCE) {
				// Need to check lockout instance ids to ensure they are still valid.
				if (!database.VerifyInstanceID(player->GetCharacterID(), data->instance_id))
					data->instance_id = 0;
			}
		}
	}
	m_instanceList.releasereadlock(__FUNCTION__, __LINE__);

	/*for(int32 i=0;i<instanceList->size();i++)
	{
		bool valuesUpdated = false;
		InstanceData data = instanceList->at(i);
		if ( data.grant_reenter_time_left > 0 )
		{
			if ( ( data.grant_reenter_time_left - msDiff ) < 1 )
				data.grant_reenter_time_left = 0;
			else
				data.grant_reenter_time_left -= msDiff;

			valuesUpdated = true;
		}
		if ( data.grant_reset_time_left > 0 )
		{
			if ( ( data.grant_reset_time_left - msDiff ) < 1 )
				data.grant_reset_time_left = 0;
			else
				data.grant_reset_time_left -= msDiff;

			valuesUpdated = true;
		}
		if ( data.lockout_time > 0 )
		{
			if ( ( data.lockout_time - msDiff ) < 1 )
			{
				data.lockout_time = 0;
				// this means that their timer ran out and we need to clear it from db and player
				RemoveInstanceByInstanceID(data.instance_id);
				database.DeleteCharacterFromInstance(player->GetCharacterID(),data.instance_id);
			}
			else
				data.lockout_time -= msDiff;

			valuesUpdated = true;
		}

		if ( valuesUpdated )
			database.UpdateCharacterInstanceTimers(player->GetCharacterID(),data.instance_id,data.lockout_time,data.grant_reset_time_left,data.grant_reenter_time_left);
	}*/
}

int32 CharacterInstances::GetInstanceCount() {
	return instanceList.size();
}

void Player::SetPlayerAdventureClass(int8 new_class, bool set_by_gm_command ){
	int8 old_class = GetAdventureClass();
	SetAdventureClass(new_class);
	GetInfoStruct()->set_class1(classes.GetBaseClass(new_class));
	GetInfoStruct()->set_class2(classes.GetSecondaryBaseClass(new_class));
	GetInfoStruct()->set_class3(new_class);
	charsheet_changed = true;
	if(GetZone())
		GetZone()->TriggerCharSheetTimer();
	if(GetClient())
		GetClient()->UpdateTimeStampFlag ( CLASS_UPDATE_FLAG );
	
	const char* playerScript = world.GetPlayerScript(0); // 0 = global script
	const char* playerZoneScript = world.GetPlayerScript(GetZoneID()); // zone script
	if(playerScript || playerZoneScript) {
		std::vector<LuaArg> args = {
			LuaArg(GetZone()), 
			LuaArg(this), 
			LuaArg(old_class), 
			LuaArg(new_class)
		};
		if(playerScript) {
			lua_interface->RunPlayerScriptWithReturn(playerScript, "on_class_change", args);
		}
		if(playerZoneScript) {
			lua_interface->RunPlayerScriptWithReturn(playerZoneScript, "on_class_change", args);
		}
	}
}

void Player::AddSkillBonus(int32 spell_id, int32 skill_id, float value) {
	GetSkills()->AddSkillBonus(spell_id, skill_id, value);
}

SkillBonus* Player::GetSkillBonus(int32 spell_id) {
	return GetSkills()->GetSkillBonus(spell_id);
}

void Player::RemoveSkillBonus(int32 spell_id) {
	GetSkills()->RemoveSkillBonus(spell_id);
}

bool Player::HasFreeBankSlot() {
	return item_list.HasFreeBankSlot();
}

int8 Player::FindFreeBankSlot() {
	return item_list.FindFreeBankSlot();
}

void Player::AddTitle(sint32 title_id, const char *name, int8 prefix, bool save_needed){
	Title* new_title = new Title;
	new_title->SetID(title_id);
	new_title->SetName(name);
	new_title->SetPrefix(prefix);
	new_title->SetSaveNeeded(save_needed);
	player_titles_list.Add(new_title);
}

void Player::AddAAEntry(int16 template_id, int8 tab_id, int32 aa_id, int16 order,int8 treeid) {
	
	
	
}
void Player::AddLanguage(int32 id, const char *name, bool save_needed){
	Skill* skill = master_skill_list.GetSkillByName(name);
	if(skill && !GetSkills()->HasSkill(skill->skill_id)) {
		AddSkill(skill->skill_id, 1, skill->max_val, true);
	}
	// Check to see if the player already has the language
	if (HasLanguage(id))
		return;

	// Doesn't already have the language so add it
	Language* new_language = new Language;
	new_language->SetID(id);
	new_language->SetName(name);
	player_languages_list.Add(new_language);

	if (save_needed)
		database.SaveCharacterLang(GetCharacterID(), id);
}

bool Player::HasLanguage(int32 id){
	list<Language*>* languages = player_languages_list.GetAllLanguages();
	list<Language*>::iterator itr;
	Language* language = 0;
	bool ret = false;
	for(itr = languages->begin(); itr != languages->end(); itr++){
		language = *itr;
		if(language->GetID() == id){
			ret = true;
			break;
		}
	}
	return ret;
}

bool Player::HasLanguage(const char* name){
	list<Language*>* languages = player_languages_list.GetAllLanguages();
	list<Language*>::iterator itr;
	Language* language = 0;
	bool ret = false;
	for(itr = languages->begin(); itr != languages->end(); itr++){
		language = *itr;
		if(!strncmp(language->GetName(), name, strlen(name))){
			ret = true;
			break;
		}
	}
	return ret;
}

void Player::AddPassiveSpell(int32 id, int8 tier)
{
	// Add the spell to the list of passives this player currently has
	// list is used for quickly going over only the passive spells the
	// player has instead of going through all their spells.
	passive_spells.push_back(id);

	Client* client = GetClient();

	// Don not apply passives if the client is null, zoning, or reviving
	if (client == NULL || client->IsZoning() || IsResurrecting())
		return;

	Spell* spell = 0;
	spell = master_spell_list.GetSpell(id, tier);
	if (spell) {
		SpellProcess* spellProcess = 0;
		// Get the current zones spell process
		spellProcess = GetZone()->GetSpellProcess();
		// Cast the spell, CastPassives() bypasses the spell queue
		spellProcess->CastPassives(spell, this);
	}
}

void Player::ApplyPassiveSpells()
{
	Spell* spell = 0;
	SpellBookEntry* spell2 = 0;
	vector<int32>::iterator itr;
	SpellProcess* spellProcess = 0;
	spellProcess = GetZone()->GetSpellProcess();

	for (itr = passive_spells.begin(); itr != passive_spells.end(); itr++)
	{
		spell2 = GetSpellBookSpell((*itr));
		spell = master_spell_list.GetSpell(spell2->spell_id, spell2->tier);
		if (spell) {
			spellProcess->CastPassives(spell, this);
		}
	}
}

void Player::RemovePassive(int32 id, int8 tier, bool remove_from_list)
{
	Spell* spell = 0;
	spell = master_spell_list.GetSpell(id, tier);
	if (spell) {
		SpellProcess* spellProcess = 0;
		spellProcess = GetZone()->GetSpellProcess();
		spellProcess->CastPassives(spell, this, true);

		if (remove_from_list) {
			vector<int32>::iterator remove;
			remove = find(passive_spells.begin(), passive_spells.end(), id);
			if (remove != passive_spells.end())
				passive_spells.erase(remove);
		}

		database.DeleteCharacterSpell(GetCharacterID(), spell->GetSpellID());
	}
}

void Player::RemoveAllPassives()
{
	vector<int32>::iterator itr;
	for (itr = passive_spells.begin(); itr != passive_spells.end(); itr++)
		RemoveSpellBookEntry((*itr), false);

	passive_spells.clear();
}

void Player::ResetPetInfo() {
	GetInfoStruct()->set_pet_id(0xFFFFFFFF);
	GetInfoStruct()->set_pet_movement(0);
	GetInfoStruct()->set_pet_behavior(0);
	GetInfoStruct()->set_pet_health_pct(0.0f);
	GetInfoStruct()->set_pet_power_pct(0.0f);
	// Make sure the values get sent to the client
	SetCharSheetChanged(true);
}

bool Player::HasRecipeBook(int32 recipe_id){
	return recipebook_list.HasRecipeBook(recipe_id);
}

bool Player::DiscoveredLocation(int32 locationID) {
	bool ret = false;

	// No discovery type entry then return false
	if (m_characterHistory.count(HISTORY_TYPE_DISCOVERY) == 0)
		return false;

	// Is a discovery type entry but not a location subtype return false
	if (m_characterHistory[HISTORY_TYPE_DISCOVERY].count(HISTORY_SUBTYPE_LOCATION) == 0)
		return false;

	vector<HistoryData*>::iterator itr;

	for (itr = m_characterHistory[HISTORY_TYPE_DISCOVERY][HISTORY_SUBTYPE_LOCATION].begin(); itr != m_characterHistory[HISTORY_TYPE_DISCOVERY][HISTORY_SUBTYPE_LOCATION].end(); itr++) {
		if ((*itr)->Value == locationID) {
			ret = true;
			break;
		}
	}

	return ret;
}

void Player::UpdatePlayerHistory(int8 type, int8 subtype, int32 value, int32 value2) {
	switch (type) {
	case HISTORY_TYPE_NONE:
		HandleHistoryNone(subtype, value, value2);
		break;
	case HISTORY_TYPE_DEATH:
		HandleHistoryDeath(subtype, value, value2);
		break;
	case HISTORY_TYPE_DISCOVERY:
		HandleHistoryDiscovery(subtype, value, value2);
		break;
	case HISTORY_TYPE_XP:
		HandleHistoryXP(subtype, value, value2);
		break;
	default:
		// Not a valid history event so return out before trying to save into the db
		return;
	}
}

void Player::HandleHistoryNone(int8 subtype, int32 value, int32 value2) {
}

void Player::HandleHistoryDeath(int8 subtype, int32 value, int32 value2) {
}

void Player::HandleHistoryDiscovery(int8 subtype, int32 value, int32 value2) {
	switch (subtype) {
	case HISTORY_SUBTYPE_NONE:
		break;
	case HISTORY_SUBTYPE_ADVENTURE:
		break;
	case HISTORY_SUBTYPE_TRADESKILL:
		break;
	case HISTORY_SUBTYPE_QUEST:
		break;
	case HISTORY_SUBTYPE_AA:
		break;
	case HISTORY_SUBTYPE_ITEM:
		break;
	case HISTORY_SUBTYPE_LOCATION: {
		HistoryData* hd = new HistoryData;
		hd->Value = value;
		hd->Value2 = value2;
		hd->EventDate = Timer::GetUnixTimeStamp();
		strcpy(hd->Location, GetZone()->GetZoneName());
		hd->needs_save = true;

		m_characterHistory[HISTORY_TYPE_DISCOVERY][HISTORY_SUBTYPE_LOCATION].push_back(hd);
		break;
	}
	default:
		// Invalid subtype, default to NONE
		break;
	}
}

void Player::HandleHistoryXP(int8 subtype, int32 value, int32 value2) {
	switch (subtype) {
	case HISTORY_SUBTYPE_NONE:
		break;
	case HISTORY_SUBTYPE_ADVENTURE: {
		HistoryData* hd = new HistoryData;
		hd->Value = value;
		hd->Value2 = value2;
		hd->EventDate = Timer::GetUnixTimeStamp();
		strcpy(hd->Location, GetZone()->GetZoneName());
		hd->needs_save = true;

		m_characterHistory[HISTORY_TYPE_XP][HISTORY_SUBTYPE_ADVENTURE].push_back(hd);
	}
		break;
	case HISTORY_SUBTYPE_TRADESKILL:
		break;
	case HISTORY_SUBTYPE_QUEST:
		break;
	case HISTORY_SUBTYPE_AA:
		break;
	case HISTORY_SUBTYPE_ITEM:
		break;
	case HISTORY_SUBTYPE_LOCATION:
		break;
	default:
		// Invalid subtype, default to NONE
		break;
	}
}

void Player::LoadPlayerHistory(int8 type, int8 subtype, HistoryData* hd) {
	m_characterHistory[type][subtype].push_back(hd);
}

void Player::SaveHistory() {
	LogWrite(PLAYER__DEBUG, 0, "Player", "Saving History for Player: '%s'", GetName());

	map<int8, map<int8, vector<HistoryData*> > >::iterator itr;
	map<int8, vector<HistoryData*> >::iterator itr2;
	vector<HistoryData*>::iterator itr3;
	for (itr = m_characterHistory.begin(); itr != m_characterHistory.end(); itr++) {
		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++) {
			for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++) {
				
				if((*itr3)->needs_save) {
					database.SaveCharacterHistory(this, itr->first, itr2->first, (*itr3)->Value, (*itr3)->Value2, (*itr3)->Location, (*itr3)->EventDate);
					(*itr3)->needs_save = false;
				}
			}
		}
	}
}

void Player::InitXPTable() {
    int i = 2;
    while (i >= 2 && i <= 95) {
        m_levelXPReq[i] = database.GetMysqlExpCurve(i);
        i++;
    }
}

void Player::SendQuestRequiredSpawns(int32 quest_id){
	bool locked = true;
	m_playerSpawnQuestsRequired.readlock(__FUNCTION__, __LINE__);
	if (player_spawn_quests_required.size() > 0 ) {
		ZoneServer* zone = GetZone();
		Client* client = GetClient();
		if (!client){
			m_playerSpawnQuestsRequired.releasereadlock(__FUNCTION__, __LINE__);
			return;
		}
		int xxx = player_spawn_quests_required.count(quest_id);
		if (player_spawn_quests_required.count(quest_id) > 0){
			vector<int32> spawns = *player_spawn_quests_required[quest_id];
			m_playerSpawnQuestsRequired.releasereadlock(__FUNCTION__, __LINE__);
			Spawn* spawn = 0;
			vector<int32>::iterator itr;
			for (itr = spawns.begin(); itr != spawns.end();){
				spawn = zone->GetSpawnByID(*itr);
				if (spawn)
					zone->SendSpawnChanges(spawn, client, false, true);
				else {
					itr = spawns.erase(itr);
					continue;
				}
				itr++;
			}
			locked = false;
		}
	}
	if (locked)
		m_playerSpawnQuestsRequired.releasereadlock(__FUNCTION__, __LINE__);
}

void Player::SendHistoryRequiredSpawns(int32 event_id){
	bool locked = true;
	m_playerSpawnHistoryRequired.readlock(__FUNCTION__, __LINE__);
	if (player_spawn_history_required.size() > 0) {
		ZoneServer* zone = GetZone();
		Client* client = GetClient();
		if (!client){
			m_playerSpawnHistoryRequired.releasereadlock(__FUNCTION__, __LINE__);
			return;
		}
		if (player_spawn_history_required.count(event_id) > 0){
			vector<int32> spawns = *player_spawn_history_required[event_id];
			m_playerSpawnHistoryRequired.releasereadlock(__FUNCTION__, __LINE__);
			Spawn* spawn = 0;
			vector<int32>::iterator itr;
			for (itr = spawns.begin(); itr != spawns.end();){
				spawn = zone->GetSpawnByID(*itr);
				if (spawn)
					zone->SendSpawnChanges(spawn, client, false, true);
				else {
					itr = spawns.erase(itr);
					continue;
				}
				itr++;
			}
			locked = false;
		}
	}
	if (locked)
		m_playerSpawnHistoryRequired.releasereadlock(__FUNCTION__, __LINE__);
}

void Player::AddQuestRequiredSpawn(Spawn* spawn, int32 quest_id){
	if(!spawn || !quest_id)
		return;
	m_playerSpawnQuestsRequired.writelock(__FUNCTION__, __LINE__);
	if(player_spawn_quests_required.count(quest_id) == 0)
		player_spawn_quests_required[quest_id] = new vector<int32>;
	vector<int32>* quest_spawns = player_spawn_quests_required[quest_id];
	int32 current_spawn = 0;
	for(int32 i=0;i<quest_spawns->size();i++){
		current_spawn = quest_spawns->at(i);
		if (current_spawn == spawn->GetID()){
			m_playerSpawnQuestsRequired.releasewritelock(__FUNCTION__, __LINE__);
			return;
		}
	}
	player_spawn_quests_required[quest_id]->push_back(spawn->GetID());
	m_playerSpawnQuestsRequired.releasewritelock(__FUNCTION__, __LINE__);
}

void Player::AddHistoryRequiredSpawn(Spawn* spawn, int32 event_id){
	if (!spawn || !event_id)
		return;
	m_playerSpawnHistoryRequired.writelock(__FUNCTION__, __LINE__);
	if (player_spawn_history_required.count(event_id) == 0)
		player_spawn_history_required[event_id] = new vector<int32>;
	vector<int32>* history_spawns = player_spawn_history_required[event_id];
	vector<int32>::iterator itr = find(history_spawns->begin(), history_spawns->end(), spawn->GetID());
	if (itr == history_spawns->end())
		history_spawns->push_back(spawn->GetID());
	m_playerSpawnHistoryRequired.releasewritelock(__FUNCTION__, __LINE__);
}

int32 PlayerInfo::GetBoatSpawn(){
	return boat_spawn;
}

void PlayerInfo::SetBoatSpawn(Spawn* spawn){
	if(spawn)
		boat_spawn = spawn->GetID();
	else
		boat_spawn = 0;
}

void PlayerInfo::RemoveOldPackets(){
	safe_delete_array(changes);
	safe_delete_array(orig_packet);
	safe_delete_array(pet_changes);
	safe_delete_array(pet_orig_packet);
	changes = 0;
	orig_packet = 0;
	pet_changes = 0;
	pet_orig_packet = 0;
}

PlayerControlFlags::PlayerControlFlags(){
	MControlFlags.SetName("PlayerControlFlags::MControlFlags");
	MFlagChanges.SetName("PlayerControlFlags::MFlagChanges");
	flags_changed = false;
	flag_changes.clear();
	current_flags.clear();
}

PlayerControlFlags::~PlayerControlFlags(){
	flag_changes.clear();
	current_flags.clear();
}

bool PlayerControlFlags::ControlFlagsChanged(){
	bool ret;
	MFlagChanges.writelock(__FUNCTION__, __LINE__);
	ret = flags_changed;
	MFlagChanges.releasewritelock(__FUNCTION__, __LINE__);
	return ret;
}

void PlayerControlFlags::SetPlayerControlFlag(int8 param, int8 param_value, bool is_active){
	if (!param || !param_value)
		return;

	bool active_changed = false;
	MControlFlags.writelock(__FUNCTION__, __LINE__);
	active_changed = (current_flags[param][param_value] != is_active);
	if (active_changed){
		current_flags[param][param_value] = is_active;
		MFlagChanges.writelock(__FUNCTION__, __LINE__);
		flag_changes[param][param_value] = is_active ? 1 : 0;
		flags_changed = true;
		MFlagChanges.releasewritelock(__FUNCTION__, __LINE__);
	}
	MControlFlags.releasewritelock(__FUNCTION__, __LINE__);
}

void PlayerControlFlags::SendControlFlagUpdates(Client* client){
	if (!client)
		return;

	map<int8, int8>* ptr = 0;
	map<int8, map<int8, int8> >::iterator itr;
	map<int8, int8>::iterator itr2;

	MFlagChanges.writelock(__FUNCTION__, __LINE__);
	for (itr = flag_changes.begin(); itr != flag_changes.end(); itr++){
		ptr = &itr->second;
		for (itr2 = ptr->begin(); itr2 != ptr->end(); itr2++){
			int32 param = itr2->first;
			if(client->GetVersion() <= 561) {
				if(itr->first == 1) { // first set of flags DoF only supports these
					bool skip = false;
					switch(itr2->first) {
						case 1: // flymode for DoF
						case 2: // no clip mode for DoF
						case 4: // we don't know
						case 32: { // safe fall (DoF is low gravity for this parameter)
							skip = true;
							break;
						}
					}
				
					if(skip) {
						continue;
					}
				}
				
				bool bypassFlag = true;
				// remap control effects to old DoF from AoM
				switch(itr->first) {
					case 4: {
						if(itr2->first == 64) { // stun
							ClientPacketFunctions::SendServerControlFlagsClassic(client, 8, itr2->second);
							param = 16;
							bypassFlag = false;
						}
						break;
					}
				}
				if(itr->first > 1 && bypassFlag) {
					continue; // we don't support flag sets higher than 1 for DoF
				}
				ClientPacketFunctions::SendServerControlFlagsClassic(client, param, itr2->second);
			}
			else {
				ClientPacketFunctions::SendServerControlFlags(client, itr->first, itr2->first, itr2->second);
			}
		}
	}
	flag_changes.clear();
	flags_changed = false;
	MFlagChanges.releasewritelock(__FUNCTION__, __LINE__);
}

bool Player::ControlFlagsChanged(){
	return control_flags.ControlFlagsChanged();
}

void Player::SetPlayerControlFlag(int8 param, int8 param_value, bool is_active){
	control_flags.SetPlayerControlFlag(param, param_value, is_active);
}

void Player::SendControlFlagUpdates(Client* client){
	control_flags.SendControlFlagUpdates(client);
}

void Player::LoadLUAHistory(int32 event_id, LUAHistory* history) {
	mLUAHistory.writelock();
	if (m_charLuaHistory.count(event_id) > 0) {
		LogWrite(PLAYER__ERROR, 0, "Player", "Attempted to added a dupicate event (%u) to character LUA history", event_id);
		safe_delete(history);
		mLUAHistory.releasewritelock();
		return;
	}

	m_charLuaHistory.insert(make_pair(event_id,history));
	mLUAHistory.releasewritelock();
}

void Player::SaveLUAHistory() {
	mLUAHistory.readlock();
	LogWrite(PLAYER__DEBUG, 0, "Player", "Saving LUA History for Player: '%s'", GetName());

	map<int32, LUAHistory*>::iterator itr;
	for (itr = m_charLuaHistory.begin(); itr != m_charLuaHistory.end(); itr++) {
		if (itr->second->SaveNeeded) {
			database.SaveCharacterLUAHistory(this, itr->first, itr->second->Value, itr->second->Value2);
			itr->second->SaveNeeded = false;
		}
	}
	mLUAHistory.releasereadlock();
}

void Player::UpdateLUAHistory(int32 event_id, int32 value, int32 value2) {
	mLUAHistory.writelock();
	LUAHistory* hd = 0;

	if (m_charLuaHistory.count(event_id) > 0)
		hd = m_charLuaHistory[event_id];
	else {
		hd = new LUAHistory;
		m_charLuaHistory.insert(make_pair(event_id,hd));
	}

	hd->Value = value;
	hd->Value2 = value2;
	hd->SaveNeeded = true;
	mLUAHistory.releasewritelock();
	// release the mLUAHistory lock, we will maintain a readlock to avoid any further writes until we complete SendHistoryRequiredSpawns
	// through Spawn::SendSpawnChanges -> Spawn::InitializeVisPacketData -> Spawn::MeetsSpawnAccessRequirements-> Player::GetLUAHistory (this was causing a deadlock)
	mLUAHistory.readlock();
	SendHistoryRequiredSpawns(event_id);
	mLUAHistory.releasereadlock();
}

LUAHistory* Player::GetLUAHistory(int32 event_id) {
	LUAHistory* ret = 0;

	mLUAHistory.readlock();

	if (m_charLuaHistory.count(event_id) > 0)
		ret = m_charLuaHistory[event_id];

	mLUAHistory.releasereadlock();

	return ret;
}

bool Player::CanSeeInvis(Entity* target)
{
	if (!target->IsStealthed() && !target->IsInvis())
		return true;
	if (target->IsStealthed() && HasSeeHideSpell())
		return true;
	else if (target->IsInvis() && HasSeeInvisSpell())
		return true;

	sint32 radius = rule_manager.GetZoneRule(GetZoneID(), R_PVP, InvisPlayerDiscoveryRange)->GetSInt32();

	if (radius == 0) // radius of 0 is always seen
		return true;
	// radius of -1 is never seen except through items/spells, radius > -1 means we will show the player if they get into the inner radius
	else if (radius > -1 && this->GetDistance((Spawn*)target) < (float)radius)
		return true;

	// TODO: Implement See Invis Spells! http://cutpon.com:3000/devn00b/EQ2EMu/issues/43

	Item* item = 0;
	vector<Item*>* equipped_list = GetEquippedItemList();
	bool seeInvis = false;
	bool seeStealth = false;
	for (int32 i = 0; i < equipped_list->size(); i++)
	{
		item = equipped_list->at(i);
		seeInvis = item->HasStat(ITEM_STAT_SEEINVIS);
		seeStealth = item->HasStat(ITEM_STAT_SEESTEALTH);
		if (target->IsStealthed() && seeStealth)
			return true;
		else if (target->IsInvis() && seeInvis)
			return true;
	}

	return false;
}

// returns true if we need to update target info due to see invis status change
bool Player::CheckChangeInvisHistory(Entity* target)
{
	std::map<int32, bool>::iterator it;

	it = target_invis_history.find(target->GetID());
	if (it != target_invis_history.end())
	{
		//canSeeStatus
		if (it->second)
		{
			if (!this->CanSeeInvis(target))
			{
				UpdateTargetInvisHistory(target->GetID(), false);
				return true;
			}
			else
				return false;
		}
		else
		{
			if (this->CanSeeInvis(target))
			{
				UpdateTargetInvisHistory(target->GetID(), true);
				return true;
			}
			else
				return false;
		}
	}

	if (!this->CanSeeInvis(target))
		UpdateTargetInvisHistory(target->GetID(), false);
	else
		UpdateTargetInvisHistory(target->GetID(), true);

	return true;
}

void Player::UpdateTargetInvisHistory(int32 targetID, bool canSeeStatus)
{
	target_invis_history[targetID] = canSeeStatus;
}

void Player::RemoveTargetInvisHistory(int32 targetID)
{
	target_invis_history.erase(targetID);
}

int16 Player::GetNextSpawnIndex(Spawn* spawn, bool set_lock)
{
	if(set_lock)
		index_mutex.writelock(__FUNCTION__, __LINE__);
	int16 next_index = 0;
	int16 max_count = 0;
	bool not_found = true;
	do {
		next_index = (spawn_index++);
		max_count++;
		if(max_count > 0xFFFE) {
			LogWrite(PLAYER__ERROR, 0, "Player", "%s: This is bad we ran out of spawn indexes!", GetName());
			break;
		}
		if(next_index == 1 && spawn != this) { // only self can occupy index 1
			continue;
		}
		if(next_index == 0 || next_index == 255) { // avoid 0 and overloads (255)
			continue;
		}
		Spawn* tmp_spawn = nullptr;
		if(player_spawn_id_map.count(next_index) > 0)
			tmp_spawn = player_spawn_id_map[next_index];
		
		if(tmp_spawn && tmp_spawn != spawn) { // spawn index already taken and it is not this spawn
			continue;
		}
		not_found = false;
	}
	while(not_found);
	
	if(set_lock)
		index_mutex.releasewritelock(__FUNCTION__, __LINE__);
	
	return next_index;
}

bool Player::SetSpawnMap(Spawn* spawn)
{
	if(!client->GetPlayer()->SetSpawnSentState(spawn, SpawnState::SPAWN_STATE_SENDING)) {
		return false;
	}
	
	index_mutex.writelock(__FUNCTION__, __LINE__);
	int32 tmp_id = GetNextSpawnIndex(spawn, false);
	
	player_spawn_id_map[tmp_id] = spawn;

	if(player_spawn_reverse_id_map.count(spawn))
		player_spawn_reverse_id_map.erase(spawn);

	player_spawn_reverse_id_map.insert(make_pair(spawn,tmp_id));
	index_mutex.releasewritelock(__FUNCTION__, __LINE__);
	return true;
}

int16 Player::SetSpawnMapAndIndex(Spawn* spawn)
{	
	index_mutex.writelock(__FUNCTION__, __LINE__);
	int32 new_index = GetNextSpawnIndex(spawn, false);
	
	player_spawn_id_map[new_index] = spawn;
	player_spawn_reverse_id_map[spawn] = new_index;
	index_mutex.releasewritelock(__FUNCTION__, __LINE__);

	return new_index;
}

NPC* Player::InstantiateSpiritShard(float origX, float origY, float origZ, float origHeading, int32 origGridID, ZoneServer* origZone)
{
		NPC* npc = new NPC();
		string newName(GetName());
		newName.append("'s spirit shard");

		strcpy(npc->appearance.name, newName.c_str());
		/*vector<EntityCommand*>* primary_command_list = zone->GetEntityCommandList(result.GetInt32(9));
		vector<EntityCommand*>* secondary_command_list = zone->GetEntityCommandList(result.GetInt32(10));
		if(primary_command_list){
			npc->SetPrimaryCommands(primary_command_list);
			npc->primary_command_list_id = result.GetInt32(9);
		}
		if(secondary_command_list){
			npc->SetSecondaryCommands(secondary_command_list);
			npc->secondary_command_list_id = result.GetInt32(10);
		}*/
		npc->appearance.level =	GetLevel();
		npc->appearance.race = GetRace();
		npc->appearance.gender = GetGender();
		npc->appearance.adventure_class = GetAdventureClass();
		
		npc->appearance.model_type = GetModelType();
		npc->appearance.soga_model_type = GetSogaModelType();
		npc->appearance.display_name = 1;
		npc->features.hair_type = GetHairType();
		npc->features.hair_face_type = GetFacialHairType();
		npc->features.wing_type = GetWingType();
		npc->features.chest_type = GetChestType();
		npc->features.legs_type = GetLegsType();
		npc->features.soga_hair_type = GetSogaHairType();
		npc->features.soga_hair_face_type = GetSogaFacialHairType();
		npc->appearance.attackable = 0;
		npc->appearance.show_level = 1;
		npc->appearance.targetable = 1;
		npc->appearance.show_command_icon = 1;
		npc->appearance.display_hand_icon = 0;
		npc->appearance.hide_hood = GetHideHood();
		npc->size = GetSize();
		npc->appearance.pos.collision_radius = appearance.pos.collision_radius;
		npc->appearance.action_state = appearance.action_state;
		npc->appearance.visual_state = 6193; // ghostly look
		npc->appearance.mood_state = appearance.mood_state;
		npc->appearance.emote_state = appearance.emote_state;
		npc->appearance.pos.state = appearance.pos.state;
		npc->appearance.activity_status = appearance.activity_status;
		strncpy(npc->appearance.sub_title, appearance.sub_title, sizeof(npc->appearance.sub_title));
		npc->SetPrefixTitle(GetPrefixTitle());
		npc->SetSuffixTitle(GetSuffixTitle());
		npc->SetLastName(GetLastName());
		npc->SetX(origX);
		npc->SetY(origY);
		npc->SetZ(origZ);
		npc->SetHeading(origHeading);
		npc->SetSpawnOrigX(origX);
		npc->SetSpawnOrigY(origY);
		npc->SetSpawnOrigZ(origZ);
		npc->SetSpawnOrigHeading(origHeading);
		npc->SetLocation(origGridID);
		npc->SetAlive(false);
		const char* script = rule_manager.GetGlobalRule(R_Combat, SpiritShardSpawnScript)->GetString();

		int32 dbid = database.CreateSpiritShard(newName.c_str(), GetLevel(), GetRace(), GetGender(), GetAdventureClass(), GetModelType(), GetSogaModelType(), 
		GetHairType(), GetFacialHairType(), GetWingType(), GetChestType(), GetLegsType(), GetSogaHairType(), GetSogaFacialHairType(), GetHideHood(),
		GetSize(), npc->appearance.pos.collision_radius, npc->appearance.action_state, npc->appearance.visual_state, npc->appearance.mood_state, 
		npc->appearance.emote_state, npc->appearance.pos.state, npc->appearance.activity_status, npc->appearance.sub_title, GetPrefixTitle(), GetSuffixTitle(),
		GetLastName(), origX, origY, origZ, origHeading, origGridID, GetCharacterID(), origZone->GetZoneID(), origZone->GetInstanceID());

		npc->SetShardID(dbid);
		npc->SetShardCharID(GetCharacterID());
		npc->SetShardCreatedTimestamp(Timer::GetCurrentTime2());

		if(script)
			npc->SetSpawnScript(script);
		
		return npc;
}

void Player::SaveSpellEffects()
{
	if(stop_save_spell_effects)
	{
		LogWrite(PLAYER__WARNING, 0, "Player", "SaveSpellEffects called while player constructing / deconstructing!");
		return;
	}

	SpellProcess* spellProcess = 0;
	// Get the current zones spell process
	spellProcess = GetZone()->GetSpellProcess();

	Query savedEffects;
	savedEffects.AddQueryAsync(GetCharacterID(), &database, Q_DELETE, "delete from character_spell_effects where charid=%u", GetCharacterID());
	savedEffects.AddQueryAsync(GetCharacterID(), &database, Q_DELETE, "delete from character_spell_effect_targets where caster_char_id=%u", GetCharacterID());
	InfoStruct* info = GetInfoStruct();
	MSpellEffects.readlock(__FUNCTION__, __LINE__);
	MMaintainedSpells.readlock(__FUNCTION__, __LINE__);
	for(int i = 0; i < 45; i++) {
		if(info->spell_effects[i].spell_id != 0xFFFFFFFF)
		{
			Spawn* spawn = nullptr;
			int32 target_char_id = 0;
			if(info->spell_effects[i].spell->initial_target_char_id != 0)
				target_char_id = info->spell_effects[i].spell->initial_target_char_id;
			else if((spawn = GetZone()->GetSpawnByID(info->spell_effects[i].spell->initial_target)) != nullptr && spawn->IsPlayer())
				target_char_id = ((Player*)spawn)->GetCharacterID();

			int32 timestamp = 0xFFFFFFFF;
			if(info->spell_effects[i].spell->spell->GetSpellData() && !info->spell_effects[i].spell->spell->GetSpellData()->duration_until_cancel)
				timestamp = info->spell_effects[i].expire_timestamp - Timer::GetCurrentTime2();
			
			int32 caster_char_id = info->spell_effects[i].spell->initial_caster_char_id;

			if(caster_char_id == 0)
				continue;
			
			savedEffects.AddQueryAsync(GetCharacterID(), &database, Q_INSERT, 
			"insert into character_spell_effects (name, caster_char_id, target_char_id, target_type, db_effect_type, spell_id, effect_slot, slot_pos, icon, icon_backdrop, conc_used, tier, total_time, expire_timestamp, lua_file, custom_spell, charid, damage_remaining, effect_bitmask, num_triggers, had_triggers, cancel_after_triggers, crit, last_spellattack_hit, interrupted, resisted, has_damaged, custom_function, caster_level) values ('%s', %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %f, %u, '%s', %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, '%s', %u)", 
			database.getSafeEscapeString(info->spell_effects[i].spell->spell->GetName()).c_str(), caster_char_id,
			target_char_id,  0 /*no target_type for spell_effects*/, DB_TYPE_SPELLEFFECTS /* db_effect_type for spell_effects */, info->spell_effects[i].spell->spell->IsCopiedSpell() ? info->spell_effects[i].spell->spell->GetSpellData()->inherited_spell_id : info->spell_effects[i].spell_id, i, info->spell_effects[i].spell->slot_pos, 
			info->spell_effects[i].icon, info->spell_effects[i].icon_backdrop, 0 /* no conc_used for spell_effects */, info->spell_effects[i].tier, 
			info->spell_effects[i].total_time, timestamp, database.getSafeEscapeString(info->spell_effects[i].spell->file_name.c_str()).c_str(), info->spell_effects[i].spell->spell->IsCopiedSpell(), GetCharacterID(), 
			info->spell_effects[i].spell->damage_remaining, info->spell_effects[i].spell->effect_bitmask, info->spell_effects[i].spell->num_triggers, info->spell_effects[i].spell->had_triggers, info->spell_effects[i].spell->cancel_after_all_triggers,
			info->spell_effects[i].spell->crit, info->spell_effects[i].spell->last_spellattack_hit, info->spell_effects[i].spell->interrupted, info->spell_effects[i].spell->resisted, info->spell_effects[i].spell->has_damaged, (info->maintained_effects[i].expire_timestamp) == 0xFFFFFFFF ? "" : database.getSafeEscapeString(spellProcess->SpellScriptTimerCustomFunction(info->spell_effects[i].spell).c_str()).c_str(), info->spell_effects[i].spell->initial_caster_level);
		}
		if (i < NUM_MAINTAINED_EFFECTS && info->maintained_effects[i].spell_id != 0xFFFFFFFF){
			Spawn* spawn = GetZone()->GetSpawnByID(info->maintained_effects[i].spell->initial_target);

			int32 target_char_id = 0;
		
			if(info->maintained_effects[i].spell->initial_target_char_id != 0)
				target_char_id = info->maintained_effects[i].spell->initial_target_char_id;
			else if(!info->maintained_effects[i].spell->initial_target)
				target_char_id = GetCharacterID();
			else if(spawn && spawn->IsPlayer())
				target_char_id = ((Player*)spawn)->GetCharacterID();
			else if (spawn && spawn->IsPet() && ((Entity*)spawn)->GetOwner() == (Entity*)this)
				target_char_id = 0xFFFFFFFF;
			
			int32 caster_char_id = info->maintained_effects[i].spell->initial_caster_char_id;
			
			int32 timestamp = 0xFFFFFFFF;
			if(info->maintained_effects[i].spell->spell->GetSpellData() && !info->maintained_effects[i].spell->spell->GetSpellData()->duration_until_cancel)
				timestamp = info->maintained_effects[i].expire_timestamp - Timer::GetCurrentTime2();
			savedEffects.AddQueryAsync(GetCharacterID(), &database, Q_INSERT, 
			"insert into character_spell_effects (name, caster_char_id, target_char_id, target_type, db_effect_type, spell_id, effect_slot, slot_pos, icon, icon_backdrop, conc_used, tier, total_time, expire_timestamp, lua_file, custom_spell, charid, damage_remaining, effect_bitmask, num_triggers, had_triggers, cancel_after_triggers, crit, last_spellattack_hit, interrupted, resisted, has_damaged, custom_function, caster_level) values ('%s', %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %f, %u, '%s', %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, '%s', %u)", 
			database.getSafeEscapeString(info->maintained_effects[i].name).c_str(), caster_char_id, target_char_id,  info->maintained_effects[i].target_type, DB_TYPE_MAINTAINEDEFFECTS /* db_effect_type for maintained_effects */, info->maintained_effects[i].spell->spell->IsCopiedSpell() ? info->maintained_effects[i].spell->spell->GetSpellData()->inherited_spell_id : info->maintained_effects[i].spell_id, i, info->maintained_effects[i].slot_pos, 
			info->maintained_effects[i].icon, info->maintained_effects[i].icon_backdrop, info->maintained_effects[i].conc_used, info->maintained_effects[i].tier, 
			info->maintained_effects[i].total_time, timestamp, database.getSafeEscapeString(info->maintained_effects[i].spell->file_name.c_str()).c_str(), info->maintained_effects[i].spell->spell->IsCopiedSpell(), GetCharacterID(), 
			info->maintained_effects[i].spell->damage_remaining, info->maintained_effects[i].spell->effect_bitmask, info->maintained_effects[i].spell->num_triggers, info->maintained_effects[i].spell->had_triggers, info->maintained_effects[i].spell->cancel_after_all_triggers,
			info->maintained_effects[i].spell->crit, info->maintained_effects[i].spell->last_spellattack_hit, info->maintained_effects[i].spell->interrupted, info->maintained_effects[i].spell->resisted, info->maintained_effects[i].spell->has_damaged, (info->maintained_effects[i].expire_timestamp) == 0xFFFFFFFF ? "" : database.getSafeEscapeString(spellProcess->SpellScriptTimerCustomFunction(info->maintained_effects[i].spell).c_str()).c_str(), info->maintained_effects[i].spell->initial_caster_level);

			info->maintained_effects[i].spell->MSpellTargets.readlock(__FUNCTION__, __LINE__);
			std::string insertTargets = string("insert into character_spell_effect_targets (caster_char_id, target_char_id, target_type, db_effect_type, spell_id, effect_slot, slot_pos) values ");
			bool firstTarget = true;
			map<Spawn*, int8> targetsInserted;
			for (int8 t = 0; t < info->maintained_effects[i].spell->targets.size(); t++) {
				int32 spawn_id = info->maintained_effects[i].spell->targets.at(t);
					Spawn* spawn = GetZone()->GetSpawnByID(spawn_id);
					LogWrite(SPELL__DEBUG, 0, "Spell", "%s has target %u to identify for spell %s", GetName(), spawn_id, info->maintained_effects[i].spell->spell->GetName());
					if(spawn && (spawn->IsPlayer() || spawn->IsPet()))
					{
						int32 tmpCharID = 0;
						int8 type = 0;

						if(targetsInserted.find(spawn) != targetsInserted.end())
							continue;
						
						if(spawn->IsPlayer())
							tmpCharID = ((Player*)spawn)->GetCharacterID();
						else if (spawn->IsPet() && ((Entity*)spawn)->GetOwner() == (Entity*)this)
						{
							tmpCharID = 0xFFFFFFFF;
						}
						else if(spawn->IsPet() && ((Entity*)spawn)->GetOwner() && 
									((Entity*)spawn)->GetOwner()->IsPlayer())
						{
							type = ((Entity*)spawn)->GetPetType();
							Player* petOwner =  (Player*)((Entity*)spawn)->GetOwner();
							tmpCharID = petOwner->GetCharacterID();
						}

						if(!firstTarget)
							insertTargets.append(", ");
						
						targetsInserted.insert(make_pair(spawn, true));


						LogWrite(SPELL__DEBUG, 0, "Spell", "%s has target %s (%u) added to spell %s", GetName(), spawn ? spawn->GetName() : "NA", tmpCharID, info->maintained_effects[i].spell->spell->GetName());
						insertTargets.append("(" + std::to_string(caster_char_id) + ", " + std::to_string(tmpCharID) + ", " + std::to_string(type) + ", " + 
						std::to_string(DB_TYPE_MAINTAINEDEFFECTS) + ", " + std::to_string(info->maintained_effects[i].spell_id) + ", " + std::to_string(i) + 
						", " + std::to_string(info->maintained_effects[i].slot_pos) + ")");
						firstTarget = false;
					}
			}
			multimap<int32,int8>::iterator entries;
			for(entries = info->maintained_effects[i].spell->char_id_targets.begin(); entries != info->maintained_effects[i].spell->char_id_targets.end(); entries++)
			{
				if(!firstTarget)
					insertTargets.append(", ");

				LogWrite(SPELL__DEBUG, 0, "Spell", "%s has target %s (%u) added to spell %s", GetName(), spawn ? spawn->GetName() : "NA", entries->first, info->maintained_effects[i].spell->spell->GetName());
				insertTargets.append("(" + std::to_string(caster_char_id) + ", " + std::to_string(entries->first) + ", " + std::to_string(entries->second) + ", " + 
				std::to_string(DB_TYPE_MAINTAINEDEFFECTS) + ", " + std::to_string(info->maintained_effects[i].spell_id) + ", " + std::to_string(i) + 
				", " + std::to_string(info->maintained_effects[i].slot_pos) + ")");

				firstTarget = false;
			}
			info->maintained_effects[i].spell->MSpellTargets.releasereadlock(__FUNCTION__, __LINE__);
			if(!firstTarget) {
				savedEffects.AddQueryAsync(GetCharacterID(), &database, Q_INSERT, insertTargets.c_str());
			}
		}
	}
	MMaintainedSpells.releasereadlock(__FUNCTION__, __LINE__);
	MSpellEffects.releasereadlock(__FUNCTION__, __LINE__);
}

void Player::MentorTarget()
{
	if(client->GetPlayer()->GetGroupMemberInfo() && client->GetPlayer()->GetGroupMemberInfo()->mentor_target_char_id)
	{
		client->GetPlayer()->GetGroupMemberInfo()->mentor_target_char_id = 0;
		reset_mentorship = true;
		client->Message(CHANNEL_COMMAND_TEXT, "You stop mentoring, and return to level %u.", client->GetPlayer()->GetLevel());
	}
	else if(!reset_mentorship && client->GetPlayer()->GetTarget())
	{
		if(client->GetPlayer()->GetTarget()->IsPlayer())
		{
			Player* tmpPlayer = (Player*)client->GetPlayer()->GetTarget();
			if(tmpPlayer->GetGroupMemberInfo() && tmpPlayer->GetGroupMemberInfo()->mentor_target_char_id)
			{
				client->Message(CHANNEL_COMMAND_TEXT, "You cannot mentor %s at this time.",tmpPlayer->GetName());
				return;
			}
			if(client->GetPlayer()->group_id > 0 && client->GetPlayer()->GetTarget()->group_id == client->GetPlayer()->group_id)
			{
				if(client->GetPlayer()->GetGroupMemberInfo() && !client->GetPlayer()->GetGroupMemberInfo()->mentor_target_char_id && client->GetPlayer()->GetZone() == client->GetPlayer()->GetTarget()->GetZone() && client->GetPlayer()->GetTarget()->GetName() != client->GetPlayer()->GetName())
				{
					SetMentorStats(client->GetPlayer()->GetTarget()->GetLevel(), tmpPlayer->GetCharacterID());
					client->Message(CHANNEL_COMMAND_TEXT, "You are now mentoring %s, reducing your effective level to %u.",client->GetPlayer()->GetTarget()->GetName(), client->GetPlayer()->GetTarget()->GetLevel());
				}
				if(client->GetPlayer()->GetTarget()->GetName() == client->GetPlayer()->GetName()) {
					client->Message(CHANNEL_COMMAND_TEXT, "You cannot mentor yourself.");
				}
			}
		}
	}
}

void Player::SetMentorStats(int32 effective_level, int32 target_char_id, bool update_stats)
{
	if(update_stats) {
		RemoveSpells();
	}
	if(client->GetPlayer()->GetGroupMemberInfo())
		client->GetPlayer()->GetGroupMemberInfo()->mentor_target_char_id = target_char_id;
	InfoStruct* info = GetInfoStruct();
	info->set_effective_level(effective_level);
	CalculatePlayerHPPower(effective_level);
	client->GetPlayer()->CalculateBonuses();
	if(update_stats) {
		client->GetPlayer()->SetHP(GetTotalHP());
		client->GetPlayer()->SetPower(GetTotalPower());
	}
	/*info->set_agi_base(effective_level * 2 + 15);
	info->set_intel_base(effective_level * 2 + 15);
	info->set_wis_base(effective_level * 2 + 15);
	info->set_str_base(effective_level * 2 + 15);
	info->set_sta_base(effective_level * 2 + 15);
	info->set_cold_base((int16)(effective_level * 1.5 + 10));
	info->set_heat_base((int16)(effective_level * 1.5 + 10));
	info->set_disease_base((int16)(effective_level * 1.5 + 10));
	info->set_mental_base((int16)(effective_level * 1.5 + 10));
	info->set_magic_base((int16)(effective_level * 1.5 + 10));
	info->set_divine_base((int16)(effective_level * 1.5 + 10));
	info->set_poison_base((int16)(effective_level * 1.5 + 10));*/
	GetClient()->ClearSentItemDetails();
	if(GetClient())
	{
		EQ2Packet* app = GetEquipmentList()->serialize(GetClient()->GetVersion(), this);
		if (app) {
			GetClient()->QueuePacket(app);
		}
	}
	GetEquipmentList()->SendEquippedItems(this);
}

void Player::SetLevel(int16 level, bool setUpdateFlags) {
	if(!GetGroupMemberInfo() || GetGroupMemberInfo()->mentor_target_char_id == 0) {
		GetInfoStruct()->set_effective_level(level);
	}
	SetInfo(&appearance.level, level, setUpdateFlags);
	SetXP(0);
	SetNeededXP();
}

bool Player::SerializeItemPackets(EquipmentItemList* equipList, vector<EQ2Packet*>* packets, Item* item, int16 version, Item* to_item) {
	if(item_list.AddItem(item)) {
		item->save_needed = true;
		SetEquippedItemAppearances();
		packets->push_back(equipList->serialize(version, this));
		packets->push_back(item->serialize(version, false));
		if(to_item)
			packets->push_back(to_item->serialize(version, false, this));
		packets->push_back(item_list.serialize(this, version));
		return true;
	}
	else {
		LogWrite(PLAYER__ERROR, 0, "Player", "failed to add item to item_list");
	}
	return false;
}

void Player::AddGMVisualFilter(int32 filter_type, int32 filter_value, char* filter_search_str, int16 visual_tag) {
	if(MatchGMVisualFilter(filter_type, filter_value, filter_search_str) > 0)
		return;

	vis_mutex.writelock(__FUNCTION__, __LINE__);
	GMTagFilter filter;
	filter.filter_type = filter_type;
	filter.filter_value = filter_value;
	memset(filter.filter_search_criteria, 0, sizeof(filter.filter_search_criteria));
	if(filter_search_str)
		memcpy(&filter.filter_search_criteria, filter_search_str, strnlen(filter_search_str, sizeof(filter.filter_search_criteria)));

	filter.visual_tag = visual_tag;
	gm_visual_filters.push_back(filter);
	vis_mutex.releasewritelock(__FUNCTION__, __LINE__);
}

int16 Player::MatchGMVisualFilter(int32 filter_type, int32 filter_value, char* filter_search_str, bool in_vismutex_lock) {
	if(!in_vismutex_lock)
		vis_mutex.readlock(__FUNCTION__, __LINE__);
	int16 tag_id = 0;
	vector<GMTagFilter>::iterator itr = gm_visual_filters.begin();
	for(;itr != gm_visual_filters.end();itr++) {
		if(itr->filter_type == filter_type && itr->filter_value == filter_value) {
			if(filter_search_str && !strcasecmp(filter_search_str, itr->filter_search_criteria)) {
				tag_id = itr->visual_tag;
				break;
			}
		}
	}
	if(!in_vismutex_lock)
		vis_mutex.releasereadlock(__FUNCTION__, __LINE__);
	return tag_id;
}
void Player::ClearGMVisualFilters() {
	vis_mutex.writelock(__FUNCTION__, __LINE__);
	gm_visual_filters.clear();
	vis_mutex.releasewritelock(__FUNCTION__, __LINE__);
}

int Player::GetPVPAlignment(){
	int bind_zone = GetPlayerInfo()->GetBindZoneID();
	int alignment = 0;

	if(bind_zone && bind_zone != 0){
	//0 is good.
	//1 is evil.
	//2 is neutral aka haven players.
			switch(bind_zone){
			//good zones
				case 114: //Gfay
				case 221: //Qeynos Harbor
				case 222: //North Qeynos
				case 231: //South Qeynos
				case 233: //Nettleville
				case 234: //Starcrest
				case 235: //Graystone
				case 236: //CastleView
				case 237: //Willowood
				case 238: //Baubbleshire
				case 470: //Frostfang
				case 589: //Qeynos Combined 1
				case 660: //Qeynos Combined 2
					alignment = 0; //good
					break;
				//evil zones				
				case 128: //East Freeport
				case 134: //Big Bend
				case 135: //Stonestair
				case 136: //Temple St.
				case 137: //Beggars Ct.
				case 138: //Longshadow
				case 139: //Scale Yard
				case 144: //North Freeport
				case 166: //South Freeport
				case 168: //West Freeport
				case 184: //Neriak
				case 644: //BigBend2
				case 645: //Stonestair2
				case 646: //Temple St2
				case 647: //Beggars Ct2
				case 648: //LongShadow2
				case 649: //Scale Yard2
					alignment = 1; //evil
					break;
				//Neutral (MajDul?)
				case 45: //haven
				case 46: //MajDul
					alignment = 2;
					break;
				
				default:
					alignment = -1; //error
			}
		//return -1 (error), 0 (good), 1 (evil), or 2 (Neutral)
		return alignment;
	}
	return -1; //error
}

void Player::GetSpellBookSlotSort(int32 pattern, int32* i, int8* page_book_count, int32* last_start_point) {
	switch(pattern) {
		case 1: { // down
			*i = (*i) + 2;
			(*page_book_count)++;
			if(*page_book_count > 3) {
				if(((*i) % 2) == 0) {
					(*i) = (*last_start_point) + 1;
				}
				else {
					(*last_start_point) = (*last_start_point) + 8;
					(*i) = (*last_start_point);
				}
				(*page_book_count) = 0;
			}
			break;
		}
		case 2: { // across
			(*page_book_count)++;
			switch(*page_book_count) {
				case 1:
				case 3:	{
					(*i)++;
					break;
				}
				case 2: {
					(*i) = (*i) + 7;
					break;
				}
				case 4: {
					(*last_start_point) = (*last_start_point) + 2;
					(*i) = (*last_start_point);
					(*page_book_count) = 0;
					break;
				}
			}
			break;
		}
		default: { // zig-zag
			(*i)++;
			break;
		}
	}
}


bool Player::IsSpawnInRangeList(int32 spawn_id) {
    std::shared_lock lock(spawn_aggro_range_mutex);
	map<int32, bool>::iterator spawn_itr = player_aggro_range_spawns.find(spawn_id);
	if(spawn_itr != player_aggro_range_spawns.end()) {
		return spawn_itr->second;
	}
	return false;
}

void Player::SetSpawnInRangeList(int32 spawn_id, bool in_range) {
    std::unique_lock lock(spawn_aggro_range_mutex);
	player_aggro_range_spawns[spawn_id] = in_range;
}

void Player::ProcessSpawnRangeUpdates() {
    std::unique_lock lock(spawn_aggro_range_mutex);
	if(GetClient()->GetCurrentZone() == nullptr) {
		return;
	}
	
	map<int32, bool>::iterator spawn_itr;
	for(spawn_itr = player_aggro_range_spawns.begin(); spawn_itr != player_aggro_range_spawns.end();) {
		if(spawn_itr->second) {
			Spawn* spawn = GetClient()->GetCurrentZone()->GetSpawnByID(spawn_itr->first);
			if(spawn && spawn->IsNPC() && (GetDistance(spawn)) > ((NPC*)spawn)->GetAggroRadius()) {
				GetClient()->GetCurrentZone()->SendSpawnChanges((NPC*)spawn, GetClient(), true, true);
				spawn_itr->second = false;
				spawn_itr = player_aggro_range_spawns.erase(spawn_itr);
				continue;
			}
		}
		spawn_itr++;
	}
}

void Player::CalculatePlayerHPPower(int16 new_level) {
	if(IsPlayer()) {
		int16 effective_level = GetInfoStruct()->get_effective_level() != 0 ? GetInfoStruct()->get_effective_level() : GetLevel();
		if(new_level < 1) {
			new_level = effective_level;
		}
		
		float hp_rule_mod = rule_manager.GetGlobalRule(R_Player, StartHPLevelMod)->GetFloat();
		float power_rule_mod = rule_manager.GetGlobalRule(R_Player, StartPowerLevelMod)->GetFloat();
		
		sint32 base_hp = rule_manager.GetGlobalRule(R_Player, StartHPBase)->GetFloat();
		sint32 base_power = rule_manager.GetGlobalRule(R_Player, StartPowerBase)->GetSInt32();
		
		sint32 new_hp = (sint32)((float)new_level * (float)new_level * hp_rule_mod + base_hp);
		sint32 new_power = (sint32)((float)new_level * (float)new_level * power_rule_mod + base_power);
		
		if(new_hp < 1) {
			LogWrite(PLAYER__WARNING, 0, "Player", "Player HP Calculation for %s too low at level %u due to ruleset, StartPowerLevelMod %f, BasePower %i", GetName(), new_level, hp_rule_mod, base_hp);
			new_hp = 1;
		}
		if(new_power < 1) {
			LogWrite(PLAYER__WARNING, 0, "Player", "Player Power Calculations for %s too low at level %u due to ruleset, StartPowerLevelMod %f, BasePower %i", GetName(), new_level, power_rule_mod, base_power);
			new_power = 1;
		}
		
		SetTotalHPBase(new_hp);
		SetTotalHPBaseInstance(new_hp); // we need the hp base to override the instance as the new default
		
		SetTotalPowerBase(new_power);
		SetTotalPowerBaseInstance(new_power); // we need the hp base to override the instance as the new default
		
		LogWrite(PLAYER__INFO, 0, "Player", "Player %s: Level %u, Set Base HP %i, Set Base Power: %i", GetName(), new_level, new_hp, new_power);
	}
}

bool Player::IsAllowedCombatEquip(int8 slot, bool send_message) {
	bool rule_pass = true;
	if(EngagedInCombat() && rule_manager.GetZoneRule(GetZoneID(), R_Player, AllowPlayerEquipCombat)->GetInt8() == 0) {
		switch(slot) {
			case EQ2_PRIMARY_SLOT:
			case EQ2_SECONDARY_SLOT:
			case EQ2_RANGE_SLOT:
			case EQ2_AMMO_SLOT: {
				// good to go!
				break;
			}
			default: {
				if(send_message && GetClient()) {
					GetClient()->SimpleMessage(CHANNEL_COLOR_RED, "You may not unequip/equip items while in combat.");
				}
				rule_pass = false;
				break;
			}
		}
	}
	return rule_pass;
}

void Player::SetActiveFoodUniqueID(int32 unique_id, bool update_db) {
	active_food_unique_id = unique_id;
	if(update_db) {
		database.insertCharacterProperty(client, CHAR_PROPERTY_SETACTIVEFOOD, (char*)std::to_string(unique_id).c_str());
	}
}

void Player::SetActiveDrinkUniqueID(int32 unique_id, bool update_db) {
	active_drink_unique_id = unique_id;
	if(update_db) {
	database.insertCharacterProperty(client, CHAR_PROPERTY_SETACTIVEDRINK, (char*)std::to_string(unique_id).c_str());
	}
}
	