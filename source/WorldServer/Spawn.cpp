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
#include "Spawn.h"
#include <stdio.h>
#include "../common/timer.h"
#include <time.h>
#include <math.h>
#include "Entity.h"
#include "Widget.h"
#include "Sign.h"
#include "../common/MiscFunctions.h"
#include "../common/Log.h"
#include "Rules/Rules.h"
#include "World.h"
#include "LuaInterface.h"
#include "Bots/Bot.h"
#include "Zone/raycast_mesh.h"
#include "RaceTypes/RaceTypes.h"
#include "VisualStates.h"

extern ConfigReader configReader;
extern RuleManager rule_manager;
extern World world;
extern ZoneList zone_list;
extern MasterRaceTypeList race_types_list;
extern LuaInterface* lua_interface;
extern VisualStates visual_states;

Spawn::Spawn(){ 
	loot_coins = 0;
	trap_triggered = false;
	trap_state = 0;
	chest_drop_time = 0;
	trap_opened_time = 0;
	group_id = 0;
	size_offset = 0;
	merchant_id = 0;
	merchant_type = 0;
	merchant_min_level = 0;
	merchant_max_level = 0;
	memset(&appearance, 0, sizeof(AppearanceData)); 
	memset(&basic_info, 0, sizeof(BasicInfoStruct));
	appearance.pos.state = 0x4080;
	appearance.difficulty = 6;
	size = 32;
	appearance.pos.collision_radius = 32;
	id = Spawn::NextID();
	oversized_packet = 0xFF;
	zone = 0;
	spawn_location_id = 0;
	spawn_entry_id = 0;
	spawn_location_spawns_id = 0;
	respawn = 0;
	expire_time = 0;
	expire_offset = 0;
	x_offset = 0;
	y_offset = 0;
	z_offset = 0;
	database_id = 0;
	packet_num = 1;
	changed = false;
	vis_changed = false;
	position_changed = false;
	send_spawn_changes = true;
	info_changed = false;
	appearance.pos.Speed1 = 0;
	last_attacker = 0;	
	faction_id = 0;
	running_to = 0;
	tmp_visual_state = -1;
	tmp_action_state = -1;
	transporter_id = 0;
	invulnerable = false;
	spawn_group_list = 0;
	MSpawnGroup = 0;
	movement_locations = 0;
	target = 0;
	primary_command_list_id = 0;
	secondary_command_list_id = 0;
	is_pet = false;
	m_followTarget = 0;
	following = false;
	req_quests_continued_access = false;
	req_quests_override = 0;
	req_quests_private = false;
	m_illusionModel = 0;
	Cell_Info.CurrentCell = nullptr;
	Cell_Info.CellListIndex = -1;
	m_addedToWorldTimestamp = 0;
	m_spawnAnim = 0;
	m_spawnAnimLeeway = 0;
	m_Update.SetName("Spawn::m_Update");
	m_requiredHistory.SetName("Spawn::m_requiredHistory");
	m_requiredQuests.SetName("Spawn::m_requiredQuests");
	last_heading_angle = 0.0;
	last_grid_update = 0;
	last_location_update = 0.0;
	last_movement_update = Timer::GetCurrentTime2();
	forceMapCheck = false;
	m_followDistance = 0;
	MCommandMutex.SetName("Entity::MCommandMutex");
	has_spawn_proximities = false;
	pickup_item_id = 0;
	pickup_unique_item_id = 0;
	disable_sounds = false;
	has_quests_required = false;
	has_history_required = false;
	is_flying_creature = false;
	is_water_creature = false;
	region_map = nullptr;
	current_map = nullptr;
	RegionMutex.SetName("Spawn::RegionMutex");
	pause_timer.Disable();
	m_SpawnMutex.SetName("Spawn::SpawnMutex");
	appearance_equipment_list.SetAppearanceType(1);
	is_transport_spawn = false;
	rail_id = 0;
	is_omitted_by_db_flag = false;
	loot_tier = 0;
	loot_drop_type = 0;
	deleted_spawn = false;
	is_collector = false;
	trigger_widget_id = 0;
	scared_by_strong_players = false;
	is_alive = true;
	SetLockedNoLoot(ENCOUNTER_STATE_AVAILABLE);
	loot_method = GroupLootMethod::METHOD_FFA;
	loot_rarity = 0;
	loot_group_id = 0;
	looter_spawn_id = 0;
	is_loot_complete = false;
	is_loot_dispensed = false;
	reset_movement = false;
	ResetKnockedBack();
}

Spawn::~Spawn(){
	is_running = false;
	
	vector<Item*>::iterator itr;
	for (itr = loot_items.begin(); itr != loot_items.end(); itr++)
		safe_delete(*itr);
	loot_items.clear();

	RemovePrimaryCommands();

	for(int32 i=0;i<secondary_command_list.size();i++){
		safe_delete(secondary_command_list[i]);
	}
	secondary_command_list.clear();

	RemoveSpawnFromGroup();
	
	MMovementLocations.lock();
	if(movement_locations){
		while(movement_locations->size()){
			safe_delete(movement_locations->front());
			movement_locations->pop_front();
		}
		safe_delete(movement_locations);
	}
	MMovementLocations.unlock();

	MMovementLoop.lock();
	for (int32 i = 0; i < movement_loop.size(); i++)
		safe_delete(movement_loop.at(i));
	movement_loop.clear();
	MMovementLoop.unlock();

	m_requiredHistory.writelock(__FUNCTION__, __LINE__);
	map<int32, LUAHistory*>::iterator lua_itr;
	for (lua_itr = required_history.begin(); lua_itr != required_history.end(); lua_itr++) {
		safe_delete(lua_itr->second);
	}
	required_history.clear();
	m_requiredHistory.releasewritelock(__FUNCTION__, __LINE__);

	m_requiredQuests.writelock(__FUNCTION__, __LINE__);
	map<int32, vector<int16>* >::iterator rq_itr;
	for (rq_itr = required_quests.begin(); rq_itr != required_quests.end(); rq_itr++){
		safe_delete(rq_itr->second);
	}
	required_quests.clear();
	m_requiredQuests.releasewritelock(__FUNCTION__, __LINE__);

	// just in case to make sure data is destroyed
	RemoveSpawnProximities();
	
	Regions.clear();
}

void Spawn::RemovePrimaryCommands()
{
	for (int32 i = 0; i < primary_command_list.size(); i++) {
		safe_delete(primary_command_list[i]);
	}
	primary_command_list.clear();
}

void Spawn::InitializeHeaderPacketData(Player* player, PacketStruct* header, int16 index) {
	header->setDataByName("index", index);

	if (GetSpawnAnim() > 0 && Timer::GetCurrentTime2() < (GetAddedToWorldTimestamp() + GetSpawnAnimLeeway())) {
		if (header->GetVersion() >= 57080)
			header->setDataByName("spawn_anim", GetSpawnAnim());
		else
			header->setDataByName("spawn_anim", (int16)GetSpawnAnim());
	}
	else {
		if (header->GetVersion() >= 57080)
			header->setDataByName("spawn_anim", 0xFFFFFFFF);
		else
			header->setDataByName("spawn_anim", 0xFFFF);
	}

	if (primary_command_list.size() > 0){
		if (primary_command_list.size() > 1) {
			header->setArrayLengthByName("command_list", primary_command_list.size());
			for (int32 i = 0; i < primary_command_list.size(); i++) {
				header->setArrayDataByName("command_list_name", primary_command_list[i]->name.c_str(), i);
				header->setArrayDataByName("command_list_max_distance", primary_command_list[i]->distance, i);
				header->setArrayDataByName("command_list_error", primary_command_list[i]->error_text.c_str(), i);
				header->setArrayDataByName("command_list_command", primary_command_list[i]->command.c_str(), i);
			}
		}
		if (header->GetVersion() <= 561) {
			header->setMediumStringByName("default_command", primary_command_list[0]->name.c_str());
		}
		else
			header->setMediumStringByName("default_command", primary_command_list[0]->command.c_str());
		header->setDataByName("max_distance", primary_command_list[0]->distance);
	}
	if (spawn_group_list && MSpawnGroup){
		MSpawnGroup->readlock(__FUNCTION__, __LINE__);
		header->setArrayLengthByName("group_size", spawn_group_list->size());
		vector<Spawn*>::iterator itr;
		int i = 0;
		for (itr = spawn_group_list->begin(); itr != spawn_group_list->end(); itr++, i++){
			int32 idx = 0;
			idx = player->GetIDWithPlayerSpawn((*itr));
			header->setArrayDataByName("group_spawn_id", idx, i);
		}
		MSpawnGroup->releasereadlock(__FUNCTION__, __LINE__);
	}

	header->setDataByName("spawn_id", player->GetIDWithPlayerSpawn(this));
	header->setDataByName("crc", 1);
	header->setDataByName("time_stamp", Timer::GetCurrentTime2());
}

void Spawn::InitializeVisPacketData(Player* player, PacketStruct* vis_packet) {
	int16 version = vis_packet->GetVersion();

//why?
	/*if (IsPlayer()) {
		appearance.pos.grid_id = 0xFFFFFFFF;
	}*/

	int8 tag_icon = 0;

	int32 tmp_id = 0;
	if(faction_id && (tag_icon = player->MatchGMVisualFilter(GMTagFilterType::GMFILTERTYPE_FACTION, faction_id, "", true)) > 0);
	else if(IsGroundSpawn() && (tag_icon = player->MatchGMVisualFilter(GMTagFilterType::GMFILTERTYPE_GROUNDSPAWN, 1, "", true)) > 0);
	else if((this->GetSpawnGroupID() && (tag_icon = player->MatchGMVisualFilter(GMTagFilterType::GMFILTERTYPE_SPAWNGROUP, 1, "", true)) > 0) ||
			(!this->GetSpawnGroupID() && (tag_icon = player->MatchGMVisualFilter(GMTagFilterType::GMFILTERTYPE_SPAWNGROUP, 0, "", true)) > 0));
	else if((this->GetRace() && (tag_icon = player->MatchGMVisualFilter(GMTagFilterType::GMFILTERTYPE_RACE, GetRace(), "", true)) > 0));
	else if(((tmp_id = race_types_list.GetRaceType(GetModelType()) > 0) && (tag_icon = player->MatchGMVisualFilter(GMTagFilterType::GMFILTERTYPE_RACE, tmp_id, "", true)) > 0));
	else if(((tmp_id = race_types_list.GetRaceBaseType(GetModelType()) > 0) && (tag_icon = player->MatchGMVisualFilter(GMTagFilterType::GMFILTERTYPE_RACE, tmp_id, "", true)) > 0));
	else if(IsEntity() && (tag_icon = ((Entity*)this)->GetInfoStruct()->get_tag1()) > 0);
	
	vis_packet->setDataByName("tag1", tag_icon);

	if (IsPlayer())
		vis_packet->setDataByName("player", 1);
	if (version <= 561) {
		vis_packet->setDataByName("targetable", appearance.targetable);
		vis_packet->setDataByName("show_name", appearance.display_name);
		vis_packet->setDataByName("attackable", appearance.attackable);
		if(appearance.attackable == 1)
			vis_packet->setDataByName("attackable_icon", 1); 
		if (IsPlayer()) {
			if (((Player*)this)->IsGroupMember(player))
				vis_packet->setDataByName("group_member", 1);
		}

	}
	if (appearance.targetable == 1 || appearance.show_level == 1 || appearance.display_name == 1) {
		if (!IsGroundSpawn()) {
			int8 arrow_color = ARROW_COLOR_WHITE;
			sint8 npc_con = player->GetFactions()->GetCon(faction_id);

			if (IsPlayer() && !((Player*)this)->CanSeeInvis(player))
				npc_con = 0;
			else if (!IsPlayer() && IsEntity() && !((Entity*)this)->CanSeeInvis(player))
				npc_con = 0;

			if (appearance.attackable == 1)
				arrow_color = player->GetArrowColor(GetLevel());
			if (version <= 373) {
				if (GetMerchantID() > 0)
					arrow_color += 7;
				else {
					if (primary_command_list.size() > 0) {
						int16 len = strlen(primary_command_list[0]->command.c_str());
						if(len >= 4 && strncmp(primary_command_list[0]->command.c_str(), "bank", 4) == 0)
							arrow_color += 14;
						else if (len >= 4 && strncmp(primary_command_list[0]->command.c_str(), "hail", 4) == 0)
							arrow_color += 21;
						else if (len >= 6 && strncmp(primary_command_list[0]->command.c_str(), "attack", 6) == 0) {
							if (arrow_color > 5)
								arrow_color = 34;
							else
								arrow_color += 29;
						}
					}
				}
			}
			if(IsNPC() && (((Entity*)this)->GetLockedNoLoot() == ENCOUNTER_STATE_BROKEN || 
					(((Entity*)this)->GetLockedNoLoot() == ENCOUNTER_STATE_OVERMATCHED) || 
					((Entity*)this)->GetLockedNoLoot() == ENCOUNTER_STATE_LOCKED && !((NPC*)this)->Brain()->IsEntityInEncounter(player->GetID()))) {
				vis_packet->setDataByName("arrow_color", ARROW_COLOR_GRAY);
			}
			else {
				vis_packet->setDataByName("arrow_color", arrow_color);
			}
			if (appearance.attackable == 0 || IsPlayer() || IsBot() || (IsEntity() && ((Entity*)this)->GetOwner() &&
				(((Entity*)this)->GetOwner()->IsPlayer() || ((Entity*)this)->GetOwner()->IsBot()))) {
				vis_packet->setDataByName("locked_no_loot", 1);
				}
			else {
				vis_packet->setDataByName("locked_no_loot", appearance.locked_no_loot);
			}
			if (player->GetArrowColor(GetLevel()) == ARROW_COLOR_GRAY)
				if (npc_con == -4)
					npc_con = -3;
			vis_packet->setDataByName("npc_con", npc_con);
			if (appearance.attackable == 1 && IsNPC() && (player->GetFactions()->GetCon(faction_id) <= -4 || ((NPC*)this)->Brain()->GetHate(player) > 1)) {
				vis_packet->setDataByName("npc_hate", ((NPC*)this)->Brain()->GetHatePercentage(player));
				vis_packet->setDataByName("show_difficulty_arrows", 1);
			}
			int8 quest_flag = player->CheckQuestFlag(this);
			if (version < 1188 && quest_flag >= 16)
				quest_flag = 1;
			vis_packet->setDataByName("quest_flag", quest_flag);
			if (player->HasQuestUpdateRequirement(this)) {
				vis_packet->setDataByName("name_quest_icon", 1);
			}
		}
	}

	int8 vis_flags = 0;
	if (MeetsSpawnAccessRequirements(player)) {
		if (appearance.attackable == 1)
			vis_flags += 64; //attackable icon
		if (appearance.show_level == 1)
			vis_flags += 32;
		if (appearance.display_name == 1)
			vis_flags += 16;
		if (IsPlayer() || appearance.targetable == 1)
			vis_flags += 4;
		if (appearance.show_command_icon == 1)
			vis_flags += 2;
		if (this == player) {
			//if (version <= 283) {
			//	vis_flags = 1;
			//}
			//else
			vis_flags += 1;
		}
	}
	else if (req_quests_override > 0)
	{
		//Check to see if there's an override value set
		vis_flags = req_quests_override & 0xFF;
	}

	if (player->HasGMVision())
	{
		if ((vis_flags & 16) == 0 && appearance.display_name == 0)
			vis_flags += 16;
		if ((vis_flags & 4) == 0)
			vis_flags += 4;
	}

	if (version <= 546 && (vis_flags > 1 || appearance.display_hand_icon > 0)) //interactable
		vis_flags = 1;
	vis_packet->setDataByName("vis_flags", vis_flags);


	if (MeetsSpawnAccessRequirements(player)) {
		vis_packet->setDataByName("hand_flag", appearance.display_hand_icon);
	}
	else {
		if ((req_quests_override & 256) > 0)
			vis_packet->setDataByName("hand_flag", 1);
	}
	if ((version == 546 || version == 561) && GetMerchantID() > 0) {
		vis_packet->setDataByName("guild", "<Merchant>");
	}
}

void Spawn::InitializeFooterPacketData(Player* player, PacketStruct* footer) {
	if (IsWidget()){
		Widget* widget = (Widget*)this;
		if (widget->GetMultiFloorLift()) {
			footer->setDataByName("widget_x", widget->GetX());
			footer->setDataByName("widget_y", widget->GetY());
			footer->setDataByName("widget_z", widget->GetZ());	
		}
		else {
			footer->setDataByName("widget_x", widget->GetWidgetX());
			footer->setDataByName("widget_y", widget->GetWidgetY());
			footer->setDataByName("widget_z", widget->GetWidgetZ());
		}
		footer->setDataByName("widget_id", widget->GetWidgetID());
	}
	else if (IsSign()){
		Sign* sign = (Sign*)this;
		footer->setDataByName("widget_id", sign->GetWidgetID());
		footer->setDataByName("widget_x", sign->GetWidgetX());
		footer->setDataByName("widget_y", sign->GetWidgetY());
		footer->setDataByName("widget_z", sign->GetWidgetZ());
		
		int8 showSignText = 1;
		if((HasQuestsRequired() || HasHistoryRequired()) && !MeetsSpawnAccessRequirements(player)) {
			showSignText = 0;
		}
		
		if (sign->GetSignTitle())
			footer->setMediumStringByName("title", sign->GetSignTitle());
		if (sign->GetSignDescription())
			footer->setMediumStringByName("description", sign->GetSignDescription());
		footer->setDataByName("sign_distance", sign->GetSignDistance());
		footer->setDataByName("show", showSignText);
		// in live we see that the language is set when the player does not have it, otherwise its left as 00's.
		if(!player->HasLanguage(sign->GetLanguage())) {
			footer->setDataByName("language", sign->GetLanguage());
		}
	}

	if ( IsPlayer())
		footer->setDataByName("is_player", 1);


	if (strlen(appearance.name) < 1)
		strncpy(appearance.name,to_string(GetID()).c_str(),128);

	footer->setMediumStringByName("name", appearance.name);
	footer->setMediumStringByName("guild", appearance.sub_title);
	footer->setMediumStringByName("prefix", appearance.prefix_title);
	footer->setMediumStringByName("suffix", appearance.suffix_title);
	footer->setMediumStringByName("last_name", appearance.last_name);
	if (appearance.attackable == 0 && GetLevel() > 0)
		footer->setDataByName("spawn_type", 1);
	else if (appearance.attackable == 0)
		footer->setDataByName("spawn_type", 6);
	else
		footer->setDataByName("spawn_type", 3);
}

EQ2Packet* Spawn::spawn_serialize(Player* player, int16 version, int16 offset, int32 value, int16 offset2, int16 offset3, int16 offset4, int32 value2) {
	// If spawn is NPC AND is pet && owner is a player && owner is the player passed to this function && player's char sheet pet id is 0
	m_Update.writelock(__FUNCTION__, __LINE__);

	int16 index = 0;
	if ((index = player->GetIndexForSpawn(this)) > 0) {
		player->SetSpawnMapIndex(this, index);
	}
	else {
		index = player->SetSpawnMapAndIndex(this);
	}

	// Jabantiz - [Bug] Client Crash on Revive
	if (player->GetIDWithPlayerSpawn(this) == 0) {
		player->SetSpawnMap(this);
	}

	PacketStruct* header = player->GetSpawnHeaderStruct();
	header->ResetData();
	InitializeHeaderPacketData(player, header, index);

	PacketStruct* footer = 0;
	if (IsWidget())
		footer = player->GetWidgetFooterStruct();
	else if (IsSign())
		footer = player->GetSignFooterStruct();
	else if (version > 561)
		footer = player->GetSpawnFooterStruct();
	if (footer) {
		footer->ResetData();
		InitializeFooterPacketData(player, footer);
	}
	PacketStruct* vis_struct = player->GetSpawnVisStruct();
	PacketStruct* info_struct = player->GetSpawnInfoStruct();
	PacketStruct* pos_struct = player->GetSpawnPosStruct();

	player->info_mutex.writelock(__FUNCTION__, __LINE__);
	player->vis_mutex.writelock(__FUNCTION__, __LINE__);
	player->pos_mutex.writelock(__FUNCTION__, __LINE__);

	info_struct->ResetData();
	InitializeInfoPacketData(player, info_struct);
	
	vis_struct->ResetData();
	InitializeVisPacketData(player, vis_struct);

	pos_struct->ResetData();
	InitializePosPacketData(player, pos_struct);
	if (version <= 283) {
		if (offset == 777) {
			info_struct->setDataByName("name", "This is a really long name\n");
			info_struct->setDataByName("last_name", "This is a really long LAST name\n");
			info_struct->setDataByName("name_suffix", "This is a really long SUFFIX\n");
			info_struct->setDataByName("name_prefix", "This is a really long PREFIX\n");
			info_struct->setDataByName("unknown", "This is a really long UNKNOWN\n");
			info_struct->setDataByName("second_suffix", "This is a really long 2nd SUFFIX\n");
		}
		//info_struct->setDataByName("unknown2", 3, 0); // level
		//info_struct->setDataByName("unknown2", 1, 1); //unknown, two down arrows
		//info_struct->setDataByName("unknown2", 1, 2); //unknown
		//info_struct->setDataByName("unknown2", 1, 3); //unknown
		//info_struct->setDataByName("unknown2", 1, 4); //solo fight
		//info_struct->setDataByName("unknown2", 1, 5); //unknown
		//info_struct->setDataByName("unknown2", 1, 6);  //unknown
		//info_struct->setDataByName("unknown2", 1, 7); //merchant
		//info_struct->setDataByName("unknown2", 1, 8);  //unknown
		//info_struct->setDataByName("unknown2", 1, 9); //unknown
		//info_struct->setDataByName("unknown2", 1, 10);
		//112: 00 00 00 01 02 03 04 05 - 06 07 08 09 0A 00 00 00 | ................ merchant, x4
		//112: 00 00 00 01 02 03 04 05 - 00 00 00 00 00 00 00 00 | ................ x4, epic, indifferent
		//info_struct->setDataByName("body_size", 42); 
		//for(int i=0;i<8;i++)
		//	info_struct->setDataByName("persistent_spell_visuals", 329, i);
		//info_struct->setDataByName("persistent_spell_levels", 20);
	}

	string* vis_data = vis_struct->serializeString();
	string* pos_data = pos_struct->serializeString();
	string* info_data = info_struct->serializeString();

	int16 part2_size = pos_data->length() + vis_data->length() + info_data->length();
	uchar* part2 = new uchar[part2_size];

	player->AddSpawnPosPacketForXOR(id, (uchar*)pos_data->c_str(), pos_data->length());
	player->AddSpawnVisPacketForXOR(id, (uchar*)vis_data->c_str(), vis_data->length());
	player->AddSpawnInfoPacketForXOR(id, (uchar*)info_data->c_str(), info_data->length());

	int32 vislength = vis_data->length();
	int32 poslength = pos_data->length();
	int32 infolength = info_data->length();

	uchar* ptr = part2;
	memcpy(ptr, pos_data->c_str(), pos_data->length());
	ptr += pos_data->length();
	memcpy(ptr, vis_data->c_str(), vis_data->length());
	ptr += vis_data->length();
	memcpy(ptr, info_data->c_str(), info_data->length());
	player->pos_mutex.releasewritelock(__FUNCTION__, __LINE__);
	player->info_mutex.releasewritelock(__FUNCTION__, __LINE__);
	player->vis_mutex.releasewritelock(__FUNCTION__, __LINE__);

	string* part1 = header->serializeString();
	string* part3 = 0;
	if (footer)
		part3 = footer->serializeString();

	//uchar blah7[] = {0x01,0x01,0x00,0x00,0x01,0x01,0x00,0x01,0x01,0x00 };
		//uchar blah7[] = { 0x03,0x01,0x00,0x01,0x01,0x01,0x00,0x01,0x01,0x00 }; base
		//uchar blah7[] = { 0x03,0x00,0x00,0x01,0x01,0x01,0x00,0x01,0x01,0x00 }; //no change
		//uchar blah7[] = { 0x03,0x00,0x00,0x00,0x01,0x01,0x00,0x01,0x01,0x00 }; //blue instead of green
		//uchar blah7[] = { 0x03,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x01,0x00 }; //no change
		//uchar blah7[] = { 0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x00 }; //not selectable
		//uchar blah7[] = { 0x03,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x01,0x00 }; //no change
		//uchar blah7[] = { 0x03,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00 }; //no longer have the two down arrows
		//uchar blah7[] = { 0x01,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00 }; //arrow color green instead of white/gray
		//memcpy(part2 + 77, blah7, sizeof(blah7));





		//DumpPacket(part2, 885);	
	if (offset > 0) {
		if (offset4 > 0 && offset4 >= offset3) {
			uchar* ptr2 = (uchar*)part2;
			ptr2 += offset3;
			while (offset4 >= offset3) {
				int8 jumpsize = 1;
				if (value2 > 0xFFFF) {
					memcpy(ptr2, (uchar*)&value2, 4);
					jumpsize = 4;
				}
				else if (value2 > 0xFF) {
					memcpy(ptr2, (uchar*)&value2, 2);
					jumpsize = 2;
				}
				else {
					memcpy(ptr2, (uchar*)&value2, 1);
					jumpsize = 1;
				}
				ptr2 += jumpsize;
				offset4 -= jumpsize;
			}
		}
		if (offset2 > 0 && offset2 >= offset) {
			uchar* ptr2 = (uchar*)part2;
			ptr2 += offset;
			while (offset2 >= offset) {
				int8 jumpsize = 1;
				if (value > 0xFFFF) {
					memcpy(ptr2, (uchar*)&value, 4);
					jumpsize = 4;
				}
				else if (value > 0xFF) {
					memcpy(ptr2, (uchar*)&value, 2);
					jumpsize = 2;
				}
				else
					memcpy(ptr2, (uchar*)&value, 1);
				ptr2 += jumpsize;
				offset2 -= jumpsize;
			}
		}
		else {
			uchar* ptr2 = (uchar*)part2;
			ptr2 += offset;
			if (value > 0xFFFF)
				memcpy(ptr2, (uchar*)&value, 4);
			else if (value > 0xFF)
				memcpy(ptr2, (uchar*)&value, 2);
			else
				memcpy(ptr2, (uchar*)&value, 1);
		}
		cout << "setting offset: " << offset << " to: " << value << endl;
	}
	//if (offset > 0)
	//	DumpPacket(part2, part2_size);

	uchar tmp[4000];
	bool reverse = (version > 373);
	part2_size = Pack(tmp, part2, part2_size, 4000, version, reverse);
	int32 total_size = part1->length() + part2_size + 3;
	if (part3)
		total_size += part3->length();
	int32 final_packet_size = total_size + 1;

	if (version > 373)
		final_packet_size += 3;
	else {
		if (final_packet_size >= 255) {
			final_packet_size += 2;
		}
	}
	uchar* final_packet = new uchar[final_packet_size];
	ptr = final_packet;
	if (version <= 373) {
		if ((final_packet_size - total_size) > 1) {
			memcpy(ptr, &oversized_packet, sizeof(oversized_packet));
			ptr += sizeof(oversized_packet);
			memcpy(ptr, &total_size, 2);
			ptr += 2;
		}
		else {
			memcpy(ptr, &total_size, 1);
			ptr += 1;
		}
	}
	else {
		memcpy(ptr, &total_size, sizeof(total_size));
		ptr += sizeof(total_size);
	}

	memcpy(ptr, &oversized_packet, sizeof(oversized_packet));
	ptr += sizeof(oversized_packet);

	int16 opcode = 0;
	
	if (EQOpcodeManager.count(GetOpcodeVersion(version)) != 0) {	
		if(IsWidget())
			opcode = EQOpcodeManager[GetOpcodeVersion(version)]->EmuToEQ(OP_EqCreateWidgetCmd);
		else if(IsSign())
			opcode = EQOpcodeManager[GetOpcodeVersion(version)]->EmuToEQ(OP_EqCreateSignWidgetCmd);
		else
			opcode = EQOpcodeManager[GetOpcodeVersion(version)]->EmuToEQ(OP_EqCreateGhostCmd);
	}
	memcpy(ptr, &opcode, sizeof(opcode));
	ptr += sizeof(opcode);

	memcpy(ptr, part1->c_str(), part1->length());
	ptr += part1->length();


	memcpy(ptr, tmp, part2_size);
	ptr += part2_size;

	if (part3)
		memcpy(ptr, part3->c_str(), part3->length());
	delete[] part2;

	//printf("SpawnPacket %s (id: %u, index: %u) to %s: p1: %i, p2: %i, p3: %i, ts: %i. poslength: %u, infolength: %u, vislength: %u\n", GetName(), GetID(), index, player->GetName(), part1->length(), part2_size, (part3 != nullptr) ? part3->length() : -1, total_size, poslength, infolength, vislength);
	EQ2Packet* ret = new EQ2Packet(OP_ClientCmdMsg, final_packet, final_packet_size);
	delete[] final_packet;

	m_Update.releasewritelock(__FUNCTION__, __LINE__);

	return ret;
}

uchar* Spawn::spawn_info_changes(Player* player, int16 version, int16* info_packet_size){
	int16 index = player->GetIndexForSpawn(this);

	PacketStruct* packet = player->GetSpawnInfoStruct();

	player->info_mutex.writelock(__FUNCTION__, __LINE__);
	packet->ResetData();
	InitializeInfoPacketData(player, packet);
	string* data = packet->serializeString();
	int32 size = data->length();
	uchar* xor_info_packet = player->GetTempInfoPacketForXOR();
	if (!xor_info_packet || size != player->GetTempInfoXorSize())
	{
		LogWrite(ZONE__DEBUG, 0, "Zone", "InstantiateInfoPacket: %i, %i", size, player->GetTempInfoXorSize());
		safe_delete(xor_info_packet);
		xor_info_packet = player->SetTempInfoPacketForXOR(size);
	}

	uchar* orig_packet = player->GetSpawnInfoPacketForXOR(id);
	if(orig_packet){
		memcpy(xor_info_packet, (uchar*)data->c_str(), size);
 		Encode(xor_info_packet, orig_packet, size);
	}


	bool changed = false;
	for (int i = 0; i < size; ++i) {
		if (xor_info_packet[i]) {
			changed = true;
			break;
		}
	}

	if (!changed) {
		player->info_mutex.releasewritelock(__FUNCTION__, __LINE__);
		return nullptr;
	}

	uchar* tmp = new uchar[size + 1000];
	size = Pack(tmp, xor_info_packet, size, size+1000, version);
	player->info_mutex.releasewritelock(__FUNCTION__, __LINE__);

	int32 orig_size = size;
	size-=sizeof(int32);
	size+=CheckOverLoadSize(index);
	*info_packet_size = size + CheckOverLoadSize(size);

	uchar* tmp2 = new uchar[*info_packet_size];
	uchar* ptr = tmp2;
	ptr += DoOverLoad(size, ptr);
	ptr += DoOverLoad(index, ptr);
	memcpy(ptr, tmp+sizeof(int32), orig_size - sizeof(int32));
	delete[] tmp;
	return tmp2;
}

uchar* Spawn::spawn_vis_changes(Player* player, int16 version, int16* vis_packet_size){
	PacketStruct* vis_struct = player->GetSpawnVisStruct();
	int16 index = player->GetIndexForSpawn(this);

	player->vis_mutex.writelock(__FUNCTION__, __LINE__);
	uchar* orig_packet = player->GetSpawnVisPacketForXOR(id);
	vis_struct->ResetData();
	InitializeVisPacketData(player, vis_struct);
	string* data = vis_struct->serializeString();
	int32 size = data->length();
	uchar* xor_vis_packet = player->GetTempVisPacketForXOR();
	if (!xor_vis_packet || size != player->GetTempVisXorSize())
	{
		LogWrite(ZONE__DEBUG, 0, "Zone", "InstantiateVisPacket: %i, %i", size, player->GetTempVisXorSize());
		safe_delete(xor_vis_packet);
		xor_vis_packet = player->SetTempVisPacketForXOR(size);
	}
	if(orig_packet){		
		memcpy(xor_vis_packet, (uchar*)data->c_str(), size);
		Encode(xor_vis_packet, orig_packet, size);
	}

	bool changed = false;
	for (int i = 0; i < size; ++i) {
		if (xor_vis_packet[i]) {
			changed = true;
			break;
		}
	}

	if (!changed) {
		player->vis_mutex.releasewritelock(__FUNCTION__, __LINE__);
		return nullptr;
	}

	uchar* tmp = new uchar[size + 1000];
	size = Pack(tmp, xor_vis_packet, size, size+1000, version);
	player->vis_mutex.releasewritelock(__FUNCTION__, __LINE__);

	int32 orig_size = size;
	size-=sizeof(int32);
	size+=CheckOverLoadSize(index);
	*vis_packet_size = size + CheckOverLoadSize(size);
	uchar* tmp2 = new uchar[*vis_packet_size];
	uchar* ptr = tmp2;
	ptr += DoOverLoad(size, ptr);
	ptr += DoOverLoad(index, ptr);
	memcpy(ptr, tmp+sizeof(int32), orig_size - sizeof(int32));
	delete[] tmp;
	return tmp2;
}

uchar* Spawn::spawn_pos_changes(Player* player, int16 version, int16* pos_packet_size, bool override_) {
	int16 index = player->GetIndexForSpawn(this);

	PacketStruct* packet = player->GetSpawnPosStruct();

	player->pos_mutex.writelock(__FUNCTION__, __LINE__);
	uchar* orig_packet = player->GetSpawnPosPacketForXOR(id);
	packet->ResetData();
	InitializePosPacketData(player, packet, true);
	string* data = packet->serializeString();
	int32 size = data->length();
	uchar* xor_pos_packet = player->GetTempPosPacketForXOR();
	if (!xor_pos_packet || size != player->GetTempPosXorSize())
	{
		LogWrite(ZONE__DEBUG, 0, "Zone", "InstantiatePosPacket: %i, %i", size, player->GetTempPosXorSize());
		safe_delete(xor_pos_packet);
		xor_pos_packet = player->SetTempPosPacketForXOR(size);
	}
	if(orig_packet){
		memcpy(xor_pos_packet, (uchar*)data->c_str(), size);
		Encode(xor_pos_packet, orig_packet, size);
	}

	bool changed = false;
	for (int i = 0; i < size; ++i) {
		if (xor_pos_packet[i]) {
			changed = true;
			break;
		}
	}

	if (!changed && !override_) {
		player->pos_mutex.releasewritelock(__FUNCTION__, __LINE__);
		return nullptr;
	}

	int16 newSize = size + 1000;
	uchar* tmp = new uchar[newSize];
	size = Pack(tmp, xor_pos_packet, size, newSize, version);
	player->pos_mutex.releasewritelock(__FUNCTION__, __LINE__);

	int32 orig_size = size;
	// Needed for CoE+ clients
	if (version >= 1188)
		size += 1;

	if(IsPlayer() && version > 561)
		size += 4;
	size-=sizeof(int32);
	size+=CheckOverLoadSize(index);

	*pos_packet_size = size + CheckOverLoadSize(size);
	uchar* tmp2 = new uchar[*pos_packet_size];
	uchar* ptr = tmp2;
	ptr += DoOverLoad(size, ptr);
	ptr += DoOverLoad(index, ptr);

	// extra byte in coe+ clients, 0 for NPC's 1 for Players
	int8 x = 0;
	if (IsPlayer() && version > 561) {
		if (version >= 1188) {
			// set x to 1 and add it to the packet
			x = 1;
			memcpy(ptr, &x, sizeof(int8));
			ptr += sizeof(int8);
		}
		int32 now = Timer::GetCurrentTime2();
		memcpy(ptr, &now, sizeof(int32));
		ptr += sizeof(int32);
	}
	else if (version >= 1188) {
		// add x to packet
		memcpy(ptr, &x, sizeof(int8));
		ptr += sizeof(int8);
	}
	memcpy(ptr, tmp+sizeof(int32), orig_size - sizeof(int32));
	delete[] tmp;
	return tmp2;
}

EQ2Packet* Spawn::player_position_update_packet(Player* player, int16 version, bool override_){
	if(!player || player->IsPlayer() == false){
		LogWrite(SPAWN__ERROR, 0, "Spawn", "Error: Called player_position_update_packet without player!");
		return 0;
	}
	else if(IsPlayer() == false){
		LogWrite(SPAWN__ERROR, 0, "Spawn", "Error: Called player_position_update_packet from spawn!");
		return 0;
	}

	static const int8 info_size = 1;
	static const int8 vis_size = 1;
	int16 pos_packet_size = 0;
	m_Update.writelock(__FUNCTION__, __LINE__);
	uchar* pos_changes = spawn_pos_changes(player, version, &pos_packet_size, override_);
	if (pos_changes == NULL)
	{
		m_Update.releasewritelock(__FUNCTION__, __LINE__);
		return NULL;
	}

	int32 size = info_size + pos_packet_size + vis_size + 8;
	if (version >= 374)
		size += 3;
	else if (version <= 373 && size >= 255) {//1 byte to 3 for overloaded val
		size += 2;
	}
	static const int8 oversized = 255;
	
	
	int16 opcode_val = 0;
	
	if (EQOpcodeManager.count(GetOpcodeVersion(version)) != 0) {	
		opcode_val = EQOpcodeManager[GetOpcodeVersion(version)]->EmuToEQ(OP_EqUpdateGhostCmd);
	}
	uchar* tmp = new uchar[size];
	memset(tmp, 0, size);
	uchar* ptr = tmp;
	if (version >= 374) {
		size -= 4;
		memcpy(ptr, &size, sizeof(int32));
		size += 4;
		ptr += sizeof(int32);
	}
	else {
		if (size >= 255) {
			memcpy(ptr, &oversized, sizeof(int8));
			ptr += sizeof(int8);
			size -= 3;
			memcpy(ptr, &size, sizeof(int16));
			size += 3;
			ptr += 3;
		}
		else {
			size -= 1;
			memcpy(ptr, &size, sizeof(int8));
			ptr += sizeof(int8);
			size += 1;
			ptr += 1;
		}
	}
	memcpy(ptr, &oversized, sizeof(int8));
	ptr += sizeof(int8);
	memcpy(ptr, &opcode_val, sizeof(int16));
	ptr += sizeof(int16);
	if (version <= 373) {
		int32 timestamp = Timer::GetCurrentTime2();
		memcpy(ptr, &timestamp, sizeof(int32));
	}
	ptr += sizeof(int32);
	ptr += info_size;
	memcpy(ptr, pos_changes, pos_packet_size);
	delete[] pos_changes;
	m_Update.releasewritelock(__FUNCTION__, __LINE__);
	EQ2Packet* ret_packet = new EQ2Packet(OP_ClientCmdMsg, tmp, size);
//	DumpPacket(ret_packet);
	delete[] tmp;
	return ret_packet;
}

EQ2Packet* Spawn::spawn_update_packet(Player* player, int16 version, bool override_changes, bool override_vis_changes){
	if(!player || player->IsPlayer() == false){
		LogWrite(SPAWN__ERROR, 0, "Spawn", "Error: Called spawn_update_packet without player!");
		return 0;
	}
	else if((IsPlayer() && info_changed == false && vis_changed == false) || (info_changed == false && vis_changed == false && position_changed == false)){
		if(!override_changes && !override_vis_changes)
			return 0;
	}

	uchar* info_changes = 0;
	uchar* pos_changes = 0;
	uchar* vis_changes = 0;
	static const int8 oversized = 255;
	int16 opcode_val = 0;
	
	if (EQOpcodeManager.count(GetOpcodeVersion(version)) != 0) {
		opcode_val = EQOpcodeManager[GetOpcodeVersion(version)]->EmuToEQ(OP_EqUpdateGhostCmd);
	}

	//We need to lock these variables up to make this thread safe
	m_Update.writelock(__FUNCTION__, __LINE__);
	//These variables are set in the spawn_info_changes, pos and vis changes functions
	int16 info_packet_size = 1;
	int16 pos_packet_size = 1;
	int16 vis_packet_size = 1;

	if (info_changed || override_changes)
		info_changes = spawn_info_changes(player, version, &info_packet_size);
	if ((position_changed || override_changes) && IsPlayer() == false)
		pos_changes = spawn_pos_changes(player, version, &pos_packet_size);
	if (vis_changed || override_changes || override_vis_changes)
		vis_changes = spawn_vis_changes(player, version, &vis_packet_size);

	int32 size = info_packet_size + pos_packet_size + vis_packet_size + 8;
	if (version >= 374)
		size += 3;
	else if (version <= 373 && size >= 255) {//1 byte to 3 for overloaded val
		size += 2;
	}
	uchar* tmp = new uchar[size];
	memset(tmp, 0, size);
	uchar* ptr = tmp;
	if (version >= 374) {
		size -= 4;
		memcpy(ptr, &size, sizeof(int32));
		size += 4;
		ptr += sizeof(int32);
	}
	else {
		if (size >= 255) {
			memcpy(ptr, &oversized, sizeof(int8));
			ptr += sizeof(int8);
			size -= 3;
			memcpy(ptr, &size, sizeof(int16));
			size += 3;
			ptr += 3;
		}
		else {
			size -= 1;
			memcpy(ptr, &size, sizeof(int8));
			ptr += sizeof(int8);
			size += 1;
		}
	}
	memcpy(ptr, &oversized, sizeof(int8));
	ptr += sizeof(int8);
	memcpy(ptr, &opcode_val, sizeof(int16));
	ptr += sizeof(int16);
	if (IsPlayer() == false || version <= 546) { //this isnt sent for player updates, it is sent on position update
		//int32 time = Timer::GetCurrentTime2();
		packet_num = Timer::GetCurrentTime2();
		memcpy(ptr, &packet_num, sizeof(int32));
	}
	ptr += sizeof(int32);
	if(info_changes)
		memcpy(ptr, info_changes, info_packet_size);
	
	ptr += info_packet_size;
	
	if(pos_changes)
		memcpy(ptr, pos_changes, pos_packet_size);
	
	ptr += pos_packet_size;
	
	if(vis_changes)
		memcpy(ptr, vis_changes, vis_packet_size);

	EQ2Packet* ret_packet = 0;
	if(info_packet_size + pos_packet_size + vis_packet_size > 0)
		ret_packet = new EQ2Packet(OP_ClientCmdMsg, tmp, size);
	delete[] tmp;
	safe_delete_array(info_changes);
	safe_delete_array(vis_changes);
	safe_delete_array(pos_changes);
	m_Update.releasewritelock(__FUNCTION__, __LINE__);
	
	return ret_packet;
}

uchar* Spawn::spawn_info_changes_ex(Player* player, int16 version, int16* info_packet_size) {
	int16 index = player->GetIndexForSpawn(this);

	PacketStruct* packet = player->GetSpawnInfoStruct();

	player->info_mutex.writelock(__FUNCTION__, __LINE__);

	packet->ResetData();
	InitializeInfoPacketData(player, packet);

	string* data = packet->serializeString();
	int32 size = data->length();
	uchar* xor_info_packet = player->GetTempInfoPacketForXOR();

	if (!xor_info_packet || size != player->GetTempInfoXorSize()) {
		LogWrite(ZONE__DEBUG, 0, "Zone", "InstantiateInfoExPacket: %i, %i", size, player->GetTempInfoXorSize());
		safe_delete(xor_info_packet);
		xor_info_packet = player->SetTempInfoPacketForXOR(size);
	}

	uchar* orig_packet = player->GetSpawnInfoPacketForXOR(id);

	if (orig_packet) {		
		//if (!IsPlayer() && this->EngagedInCombat())
			//packet->PrintPacket();
		memcpy(xor_info_packet, (uchar*)data->c_str(), size);
		Encode(xor_info_packet, orig_packet, size);
	}

	bool changed = false;
	for (int i = 0; i < size; ++i) {
		if (xor_info_packet[i]) {
			changed = true;
			break;
		}
	}

	if (!changed) {
		player->info_mutex.releasewritelock(__FUNCTION__, __LINE__);
		return nullptr;
	}

	uchar* tmp = new uchar[size + 1000];
	size = Pack(tmp, xor_info_packet, size, size+1000, version);
	player->info_mutex.releasewritelock(__FUNCTION__, __LINE__);

	int32 orig_size = size;

	size -= sizeof(int32);
	size += CheckOverLoadSize(index);
	*info_packet_size = size;

	uchar* tmp2 = new uchar[size];
	uchar* ptr = tmp2;

	ptr += DoOverLoad(index, ptr);

	memcpy(ptr, tmp + sizeof(int32), orig_size - sizeof(int32));

	delete[] tmp;

	return move(tmp2);
}

uchar* Spawn::spawn_vis_changes_ex(Player* player, int16 version, int16* vis_packet_size) {
	PacketStruct* vis_struct = player->GetSpawnVisStruct();
	int16 index = player->GetIndexForSpawn(this);

	player->vis_mutex.writelock(__FUNCTION__, __LINE__);

	uchar* orig_packet = player->GetSpawnVisPacketForXOR(id);

	vis_struct->ResetData();
	InitializeVisPacketData(player, vis_struct);

	string* data = vis_struct->serializeString();
	int32 size = data->length();
	uchar* xor_vis_packet = player->GetTempVisPacketForXOR();

	if (!xor_vis_packet || size != player->GetTempVisXorSize()) {
		LogWrite(ZONE__DEBUG, 0, "Zone", "InstantiateVisExPacket: %i, %i", size, player->GetTempVisXorSize());
		safe_delete(xor_vis_packet);
		xor_vis_packet = player->SetTempVisPacketForXOR(size);
	}

	if (orig_packet) {		
		//if (!IsPlayer() && this->EngagedInCombat())
		//	vis_struct->PrintPacket();
		memcpy(xor_vis_packet, (uchar*)data->c_str(), size);
		Encode(xor_vis_packet, orig_packet, size);
	}

	bool changed = false;
	for (int i = 0; i < size; ++i) {
		if (xor_vis_packet[i]) {
			changed = true;
			break;
		}
	}

	if (!changed) {
		player->vis_mutex.releasewritelock(__FUNCTION__, __LINE__);
		return nullptr;
	}

	uchar* tmp = new uchar[size + 1000];
	size = Pack(tmp, xor_vis_packet, size, size+1000, version);

	player->vis_mutex.releasewritelock(__FUNCTION__, __LINE__);

	int32 orig_size = size;

	size -= sizeof(int32);
	size += CheckOverLoadSize(index);
	*vis_packet_size = size;

	uchar* tmp2 = new uchar[size];
	uchar* ptr = tmp2;

	ptr += DoOverLoad(index, ptr);

	memcpy(ptr, tmp + sizeof(int32), orig_size - sizeof(int32));

	delete[] tmp;

	return move(tmp2);
}

uchar* Spawn::spawn_pos_changes_ex(Player* player, int16 version, int16* pos_packet_size) {
	int16 index = player->GetIndexForSpawn(this);

	PacketStruct* packet = player->GetSpawnPosStruct();

	player->pos_mutex.writelock(__FUNCTION__, __LINE__);

	uchar* orig_packet = player->GetSpawnPosPacketForXOR(id);

	packet->ResetData();
	InitializePosPacketData(player, packet);

	string* data = packet->serializeString();
	int32 size = data->length();
	uchar* xor_pos_packet = player->GetTempPosPacketForXOR();

	if (!xor_pos_packet || size != player->GetTempPosXorSize()) {
		LogWrite(ZONE__DEBUG, 0, "Zone", "InstantiatePosExPacket: %i, %i", size, player->GetTempPosXorSize());
		safe_delete(xor_pos_packet);
		xor_pos_packet = player->SetTempPosPacketForXOR(size);
	}

	if (orig_packet) {
		//if (!IsPlayer() && this->EngagedInCombat())
		//	packet->PrintPacket();
		memcpy(xor_pos_packet, (uchar*)data->c_str(), size);
		Encode(xor_pos_packet, orig_packet, size);
	}

	bool changed = false;
	for (int i = 0; i < size; ++i) {
		if (xor_pos_packet[i]) {
			changed = true;
			break;
		}
	}

	if (!changed && version > 561) {
		player->pos_mutex.releasewritelock(__FUNCTION__, __LINE__);
		return nullptr;
	}

	int16 newSize = size + 1000;
	uchar* tmp = new uchar[newSize];
	size = Pack(tmp, xor_pos_packet, size, newSize, version);
	player->pos_mutex.releasewritelock(__FUNCTION__, __LINE__);
	
	int32 orig_size = size;

	if (version >= 1188) {
		size += 1;
	}

	if (IsPlayer() && version > 561) {
		size += 4;
	}

	size -= sizeof(int32);
	size += CheckOverLoadSize(index);
	*pos_packet_size = size;

	uchar* tmp2 = new uchar[size];
	uchar* ptr = tmp2;

	ptr += DoOverLoad(index, ptr);

	// extra byte in coe+ clients, 0 for NPC's 1 for Players
	int8 x = 0;

	if (version > 561) {
		if (IsPlayer()) {
			if (version >= 1188) {
				x = 1;
				memcpy(ptr, &x, sizeof(int8));
				ptr += sizeof(int8);
			}
			int32 now = Timer::GetCurrentTime2();
			memcpy(ptr, &now, sizeof(int32));
			ptr += sizeof(int32);
		}
		else if (version >= 1188) {
			memcpy(ptr, &x, sizeof(int8));
			ptr += sizeof(int8);
		}
	}

	memcpy(ptr, tmp + sizeof(int32), orig_size - sizeof(int32));

	delete[] tmp;

	return move(tmp2);
}


EQ2Packet* Spawn::serialize(Player* player, int16 version){
	return 0;
}

Spawn* Spawn::GetTarget(){
	Spawn* ret = 0;
	
	// only attempt to get a spawn if we had a target stored
	if (target != 0)
	{
		ret = GetZone()->GetSpawnByID(target);
	
		if (!ret)
			target = 0;
	}

	return ret;
}

void Spawn::SetTarget(Spawn* spawn){
	SetInfo(&target, spawn ? spawn->GetID() : 0);
}

Spawn* Spawn::GetLastAttacker() {
	Spawn* ret = 0;
	ret = GetZone()->GetSpawnByID(last_attacker);
	if (!ret)
		last_attacker = 0;
	return ret;
}

void Spawn::SetLastAttacker(Spawn* spawn){
	last_attacker = spawn->GetID();
}

void Spawn::SetInvulnerable(bool val){
	invulnerable = val;
}

bool Spawn::GetInvulnerable(){
	return invulnerable;
}

bool Spawn::TakeDamage(int32 damage){
	if(invulnerable)
		return false;
	if (IsEntity()) {
		if (((Entity*)this)->IsMezzed())
			((Entity*)this)->RemoveAllMezSpells();

		if (damage == 0)
			return true;
	}

	int32 hp = GetHP();
	if(damage >= hp) {
		SetHP(0);
		if (IsPlayer()) {
			((Player*)this)->InCombat(false);
			((Player*)this)->SetRangeAttack(false);	
			GetZone()->TriggerCharSheetTimer(); // force char sheet updates now
		}
	}
	else {
		SetHP(hp - damage);
		// if player flag the char sheet as changed so the ui updates properly
		if (IsPlayer())
			((Player*)this)->SetCharSheetChanged(true);
	}
	return true;
}
ZoneServer*	Spawn::GetZone(){
	return zone;
}

void Spawn::SetZone(ZoneServer* in_zone, int32 version){
	zone = in_zone;
	
	if(in_zone)
	{
		region_map = world.GetRegionMap(std::string(in_zone->GetZoneFile()), version);
		current_map = world.GetMap(std::string(in_zone->GetZoneFile()), version);
	}
	else
	{
		region_map = nullptr;
		current_map = nullptr;
	}
}


/*** HIT POINT ***/
void Spawn::SetHP(sint32 new_val, bool setUpdateFlags){
	if(new_val == 0){
		ClearRunningLocations();
		CalculateRunningLocation(true);
		
		if(IsEntity()) {
			is_alive = false;
		}
	}
	if(IsNPC() && new_val > 0 && !is_alive) {
		is_alive = true;
	}
	else if(IsPlayer() && new_val > 0 && !is_alive) {
		LogWrite(SPAWN__ERROR, 0, "Spawn", "Cannot change player HP > 0 while dead (%s)!  Player must revive.", GetName());
		return;
	}
	if(new_val > basic_info.max_hp)
		SetInfo(&basic_info.max_hp, new_val, setUpdateFlags);
	SetInfo(&basic_info.cur_hp, new_val, setUpdateFlags);
	if(/*IsPlayer() &&*/ GetZone() && basic_info.cur_hp > 0 && basic_info.cur_hp < basic_info.max_hp)
		GetZone()->AddDamagedSpawn(this);

	SendGroupUpdate();

	if ( IsPlayer() && new_val == 0 ) // fixes on death not showing hp update for players
		((Player*)this)->SetCharSheetChanged(true);

	if (IsNPC() && ((NPC*)this)->IsPet() && ((NPC*)this)->GetOwner() && ((NPC*)this)->GetOwner()->IsPlayer()) {
		Player* player = (Player*)((NPC*)this)->GetOwner();
		if (player->GetPet() && player->GetCharmedPet()) {
			if (this == player->GetPet()) {
				player->GetInfoStruct()->set_pet_health_pct((float)basic_info.cur_hp / (float)basic_info.max_hp);
				player->SetCharSheetChanged(true);
			}
		}
		else {
			player->GetInfoStruct()->set_pet_health_pct((float)basic_info.cur_hp / (float)basic_info.max_hp);
			player->SetCharSheetChanged(true);
		}
	}
}
void Spawn::SetTotalHP(sint32 new_val){
	if(basic_info.hp_base == 0) {
		SetTotalHPBase(new_val);
		SetTotalHPBaseInstance(new_val);
	}
	SetInfo(&basic_info.max_hp, new_val);

	if(GetZone() && basic_info.cur_hp > 0 && basic_info.cur_hp < basic_info.max_hp)
		GetZone()->AddDamagedSpawn(this);

	SendGroupUpdate();

	if (IsNPC() && ((NPC*)this)->IsPet() && ((NPC*)this)->GetOwner() != nullptr && ((NPC*)this)->GetOwner()->IsPlayer()) {
		Player* player = (Player*)((NPC*)this)->GetOwner();
		if (basic_info.max_hp && player->GetPet() && player->GetCharmedPet()) {
			if (this == player->GetPet()) {
				player->GetInfoStruct()->set_pet_health_pct((float)basic_info.cur_hp / (float)basic_info.max_hp);
				player->SetCharSheetChanged(true);
			}
		}
		else if(basic_info.max_hp) {
			player->GetInfoStruct()->set_pet_health_pct((float)basic_info.cur_hp / (float)basic_info.max_hp);
			player->SetCharSheetChanged(true);
		}
	}
}
void Spawn::SetTotalHPBase(sint32 new_val)
{
	SetInfo(&basic_info.hp_base, new_val);
	
	if(GetZone() && basic_info.cur_hp > 0 && basic_info.cur_hp < basic_info.max_hp)
		GetZone()->AddDamagedSpawn(this);

	SendGroupUpdate();
}

void Spawn::SetTotalHPBaseInstance(sint32 new_val)
{
	SetInfo(&basic_info.hp_base_instance, new_val);
}

sint32 Spawn::GetHP()
{
	return basic_info.cur_hp;
}
sint32 Spawn::GetTotalHP()
{
	return basic_info.max_hp;
}
sint32 Spawn::GetTotalHPBase()
{
	return basic_info.hp_base;
}
sint32 Spawn::GetTotalHPBaseInstance()
{
	return basic_info.hp_base_instance;
}
sint32 Spawn::GetTotalPowerBaseInstance()
{
	return basic_info.power_base_instance;
}


/*** POWER ***/
void Spawn::SetPower(sint32 power, bool setUpdateFlags){
	if(power > basic_info.max_power)
		SetInfo(&basic_info.max_power, power, setUpdateFlags);
	SetInfo(&basic_info.cur_power, power, setUpdateFlags);
	if(/*IsPlayer() &&*/ GetZone() && basic_info.cur_power < basic_info.max_power)
		GetZone()->AddDamagedSpawn(this);

	SendGroupUpdate();

	if (IsNPC() && ((NPC*)this)->IsPet() && ((NPC*)this)->GetOwner() != nullptr && ((NPC*)this)->GetOwner()->IsPlayer()) {
		Player* player = (Player*)((NPC*)this)->GetOwner();
		if (player->GetPet() && player->GetCharmedPet()) {
			if (this == player->GetPet()) {
				player->GetInfoStruct()->set_pet_power_pct((float)basic_info.cur_power / (float)basic_info.max_power);
				player->SetCharSheetChanged(true);
			}
		}
		else {
			player->GetInfoStruct()->set_pet_power_pct((float)basic_info.cur_power / (float)basic_info.max_power);
			player->SetCharSheetChanged(true);
		}
	}
}
void Spawn::SetTotalPower(sint32 new_val)
{
	if(basic_info.power_base == 0) {
		SetTotalPowerBase(new_val);
		SetTotalPowerBaseInstance(new_val);
	}
	SetInfo(&basic_info.max_power, new_val);

	if(GetZone() && basic_info.cur_power < basic_info.max_power)
		GetZone()->AddDamagedSpawn(this);

	SendGroupUpdate();

	if (IsNPC() && ((NPC*)this)->IsPet() && ((NPC*)this)->GetOwner() != nullptr && ((NPC*)this)->GetOwner()->IsPlayer()) {
		Player* player = (Player*)((NPC*)this)->GetOwner();
		if (player->GetPet() && player->GetCharmedPet()) {
			if (this == player->GetPet()) {
				player->GetInfoStruct()->set_pet_power_pct((float)basic_info.cur_power / (float)basic_info.max_power);
				player->SetCharSheetChanged(true);
			}
		}
		else {
			player->GetInfoStruct()->set_pet_power_pct((float)basic_info.cur_power / (float)basic_info.max_power);
			player->SetCharSheetChanged(true);
		}
	}
}
void Spawn::SetTotalPowerBase(sint32 new_val)
{
	SetInfo(&basic_info.power_base, new_val);
	
	if(GetZone() && basic_info.cur_power < basic_info.max_power)
		GetZone()->AddDamagedSpawn(this);

	SendGroupUpdate();
}

void Spawn::SetTotalPowerBaseInstance(sint32 new_val)
{
	SetInfo(&basic_info.power_base_instance, new_val);
}

sint32 Spawn::GetPower()
{
	return basic_info.cur_power;
}
sint32 Spawn::GetTotalPower(){
	return basic_info.max_power;
}
sint32 Spawn::GetTotalPowerBase()
{
	return basic_info.power_base;
}


/*** SAVAGERY ***/
void Spawn::SetSavagery(sint32 savagery, bool setUpdateFlags)
{
	/* JA: extremely limited functionality until we better understand Savagery */
	if(savagery > basic_info.max_savagery)
		SetInfo(&basic_info.max_savagery, savagery, setUpdateFlags);

	SetInfo(&basic_info.cur_savagery, savagery, setUpdateFlags);
}
void Spawn::SetTotalSavagery(sint32 new_val)
{
	/* JA: extremely limited functionality until we better understand Savagery */
	if(basic_info.savagery_base == 0)
		SetTotalSavageryBase(new_val);

	SetInfo(&basic_info.max_savagery, new_val);
}
void Spawn::SetTotalSavageryBase(sint32 new_val){
	SetInfo(&basic_info.savagery_base, new_val);

	SendGroupUpdate();
}
sint32 Spawn::GetTotalSavagery()
{
	return basic_info.max_savagery;
}
sint32 Spawn::GetSavagery()
{
	return basic_info.cur_savagery;
}


/*** DISSONANCE ***/
void Spawn::SetDissonance(sint32 dissonance, bool setUpdateFlags)
{
	/* JA: extremely limited functionality until we better understand Dissonance */
	if(dissonance > basic_info.max_dissonance)
		SetInfo(&basic_info.max_dissonance, dissonance, setUpdateFlags);

	SetInfo(&basic_info.cur_dissonance, dissonance, setUpdateFlags);
}
void Spawn::SetTotalDissonance(sint32 new_val)
{
	/* JA: extremely limited functionality until we better understand Dissonance */
	if(basic_info.dissonance_base == 0)
		SetTotalDissonanceBase(new_val);

	SetInfo(&basic_info.max_dissonance, new_val);

}
void Spawn::SetTotalDissonanceBase(sint32 new_val)
{
	SetInfo(&basic_info.dissonance_base, new_val);

	SendGroupUpdate();
}
sint32 Spawn::GetTotalDissonance()
{
	return basic_info.max_dissonance;
}
sint32 Spawn::GetDissonance()
{
	return basic_info.cur_dissonance;
}

/* --< Alternate Advancement Points >-- */
void Spawn::SetAssignedAA(sint16 new_val)
{
	SetInfo(&basic_info.assigned_aa, new_val);
}

void Spawn::SetUnassignedAA(sint16 new_val)
{
	SetInfo(&basic_info.unassigned_aa, new_val);
}

void Spawn::SetTradeskillAA(sint16 new_val)
{
	SetInfo(&basic_info.tradeskill_aa, new_val);
}

void Spawn::SetUnassignedTradeskillAA(sint16 new_val)
{
	SetInfo(&basic_info.unassigned_tradeskill_aa, new_val);
}

void Spawn::SetPrestigeAA(sint16 new_val)
{
	SetInfo(&basic_info.prestige_aa, new_val);
}

void Spawn::SetUnassignedPrestigeAA(sint16 new_val)
{
	SetInfo(&basic_info.unassigned_prestige_aa, new_val);
}

void Spawn::SetTradeskillPrestigeAA(sint16 new_val)
{
	SetInfo(&basic_info.tradeskill_prestige_aa, new_val);
}

void Spawn::SetUnassignedTradeskillPrestigeAA(sint16 new_val)
{
	SetInfo(&basic_info.unassigned_tradeskill_prestige_aa, new_val);
}

void Spawn::SetAAXPRewards(int32 value)
{
	SetInfo(&basic_info.aaxp_rewards, value, false);
}

sint16 Spawn::GetAssignedAA()
{
	return basic_info.assigned_aa;
}

sint16 Spawn::GetUnassignedAA()
{
	return basic_info.unassigned_aa;
}

sint16 Spawn::GetTradeskillAA()
{
	return basic_info.tradeskill_aa;
}

sint16 Spawn::GetUnassignedTradeskillAA()
{
	return basic_info.unassigned_tradeskill_aa;
}

sint16 Spawn::GetPrestigeAA()
{
	return basic_info.prestige_aa;
}

sint16 Spawn::GetUnassignedPretigeAA()
{
	return basic_info.unassigned_prestige_aa;
}

sint16 Spawn::GetTradeskillPrestigeAA()
{
	return basic_info.tradeskill_prestige_aa;
}

sint16 Spawn::GetUnassignedTradeskillPrestigeAA()
{
	return basic_info.unassigned_tradeskill_prestige_aa;
}

int32 Spawn::GetAAXPRewards()
{
	return basic_info.aaxp_rewards;
}

float Spawn::GetDistance(float x1, float y1, float z1, float x2, float y2, float z2){
	x1 = x1 - x2;
	y1 = y1 - y2;
	z1 = z1 - z2;
	return sqrt(x1*x1 + y1*y1 + z1*z1); 
}

float Spawn::GetDistance(float x, float y, float z, float radius, bool ignore_y) {
	if (ignore_y)
		return GetDistance(x, y, z, GetX(), y, GetZ()) - radius;
	else
		return GetDistance(x, y, z, GetX(), GetY(), GetZ()) - radius;
}

float Spawn::GetDistance(float x, float y, float z, bool ignore_y) {
	return GetDistance(x, y, z, 0.0f, ignore_y);
}
float Spawn::GetDistance(Spawn* spawn, bool ignore_y, bool includeRadius){
	float ret = 0;

	if (spawn)
	{
		float radius = 0.0f;
		if (includeRadius)
			radius = CalculateRadius(spawn);
		ret = GetDistance(spawn->GetX(), spawn->GetY(), spawn->GetZ(), radius, ignore_y);
	}

	// maybe distance against ourselves, in that case we want to nullify the radius check
	if (ret < 0)
		ret = 0.0f;

	return ret;
}

float Spawn::GetDistance(Spawn* spawn, float x1, float y1, float z1, bool includeRadius) {
	float ret = 0;

	if (spawn)
	{
		float radius = 0.0f;
		if (includeRadius)
			radius = CalculateRadius(spawn);
		ret = GetDistance(x1, y1, z1, spawn->GetX(), spawn->GetY(), spawn->GetZ()) - radius;
	}

	// maybe distance against ourselves, in that case we want to nullify the radius check
	if (ret < 0)
		ret = 0.0f;

	return ret;
}

float Spawn::CalculateRadius(Spawn* target)
{
	float srcRadius = short_to_float(appearance.pos.collision_radius);
	if (target)
	{
		float targRadius = short_to_float(target->appearance.pos.collision_radius);
		return (targRadius / 32.0f) + (srcRadius / 32.0f);
	}
	else
		return (srcRadius / 32.0f);
}

int32 Spawn::GetRespawnTime(){
	return respawn;
}

void Spawn::SetRespawnTime(int32 time){
	respawn = time;
}

int32 Spawn::GetExpireOffsetTime(){
	return expire_offset;
}

void Spawn::SetExpireOffsetTime(int32 time){
	expire_offset = time;
}

int32 Spawn::GetSpawnLocationID(){
	return spawn_location_id;
}

void Spawn::SetSpawnLocationID(int32 id){
	spawn_location_id = id;
}

int32 Spawn::GetSpawnEntryID(){
	return spawn_entry_id;
}

void Spawn::SetSpawnEntryID(int32 id){
	spawn_entry_id = id;
}

int32 Spawn::GetSpawnLocationPlacementID(){
	return spawn_location_spawns_id;
}

void Spawn::SetSpawnLocationPlacementID(int32 id){
	spawn_location_spawns_id = id;
}

const char* Spawn::GetSpawnScript(){
	if(spawn_script.length() > 0)
		return spawn_script.c_str();
	else
		return 0;
}

void Spawn::SetSpawnScript(string name){
	spawn_script = name;
}

void Spawn::SetPrimaryCommand(const char* name, const char* command, float distance){
	EntityCommand* entity_command = CreateEntityCommand(name, distance, command, "", 0, 0);
	if(primary_command_list.size() > 0 && primary_command_list[0]){
		safe_delete(primary_command_list[0]);
		primary_command_list[0] = entity_command;
	}
	else
		primary_command_list.push_back(entity_command);
}

void Spawn::SetSecondaryCommands(vector<EntityCommand*>* commands){
	if(commands && commands->size() > 0){
		vector<EntityCommand*>::iterator itr;
		if(secondary_command_list.size() > 0){
			for(itr = secondary_command_list.begin(); itr != secondary_command_list.end(); itr++){
				safe_delete(*itr);
			}
			secondary_command_list.clear();
		}
		EntityCommand* command = 0;
		for(itr = commands->begin(); itr != commands->end(); itr++){
			command = CreateEntityCommand(*itr);
			secondary_command_list.push_back(command);
		}
	}
}

void Spawn::SetPrimaryCommands(vector<EntityCommand*>* commands){
	if(commands && commands->size() > 0){
		vector<EntityCommand*>::iterator itr;
		if(primary_command_list.size() > 0){
			for(itr = primary_command_list.begin(); itr != primary_command_list.end(); itr++){
				safe_delete(*itr);
			}
			primary_command_list.clear();
		}
		EntityCommand* command = 0;
		for(itr = commands->begin(); itr != commands->end(); itr++){
			command = CreateEntityCommand(*itr);
			primary_command_list.push_back(command);
		}
	}
}

EntityCommand* Spawn::FindEntityCommand(string command, bool primaryOnly) {
	EntityCommand* entity_command = 0;
	if (primary_command_list.size() > 0) {
		vector<EntityCommand*>::iterator itr;
		for (itr = primary_command_list.begin(); itr != primary_command_list.end(); itr++) {
			if ((*itr)->command.compare(command) == 0) {
				entity_command = *itr;
				break;
			}
		}
	}

	if (primaryOnly)
		return entity_command;

	if (!entity_command && secondary_command_list.size() > 0) {
		vector<EntityCommand*>::iterator itr;
		for (itr = secondary_command_list.begin(); itr != secondary_command_list.end(); itr++) {
			if ((*itr)->command == command) {
				entity_command = *itr;
				break;
			}
		}
	}
	return entity_command;
}

void Spawn::SetSizeOffset(int8 offset){
	size_offset = offset;
}

int8 Spawn::GetSizeOffset(){
	return size_offset;
}

void Spawn::SetMerchantID(int32 val){
	merchant_id = val;
}

int32 Spawn::GetMerchantID(){
	return merchant_id;
}

void Spawn::SetMerchantType(int8 val) {
	merchant_type = val;
}

int8 Spawn::GetMerchantType() {
	return merchant_type;
}

void Spawn::SetMerchantLevelRange(int32 minLvl, int32 maxLvl) {
	merchant_min_level = minLvl;
	merchant_max_level = maxLvl;
}

int32 Spawn::GetMerchantMinLevel() {
	return merchant_min_level;
}

int32 Spawn::GetMerchantMaxLevel() {
	return merchant_max_level;
}

bool Spawn::IsClientInMerchantLevelRange(Client* client, bool sendMessageIfDenied)
{
	if (!client)
		return false;

	if (GetMerchantMinLevel() && client->GetPlayer()->GetLevel() < GetMerchantMinLevel())
	{
		client->Message(CHANNEL_COLOR_RED, "You are unable to interact with this merchant due to a minimum level %u allowed.", GetMerchantMinLevel());
		return false;
	}
	else if (GetMerchantMaxLevel() && client->GetPlayer()->GetLevel() > GetMerchantMaxLevel())
	{
		client->Message(CHANNEL_COLOR_RED, "You are unable to interact with this merchant due to a maximum level %u allowed.", GetMerchantMaxLevel());
		return false;
	}
	
	return true;
}

void Spawn::SetQuestsRequired(Spawn* new_spawn){
	m_requiredQuests.writelock(__FUNCTION__, __LINE__);
	if(required_quests.size() > 0){
		map<int32, vector<int16>* >::iterator itr;
		for(itr = required_quests.begin(); itr != required_quests.end(); itr++){
			vector<int16>* quest_steps = itr->second;
			for (int32 i = 0; i < quest_steps->size(); i++)
				new_spawn->SetQuestsRequired(itr->first, quest_steps->at(i));
		}
	}
	m_requiredQuests.releasewritelock(__FUNCTION__, __LINE__);
}

void Spawn::SetQuestsRequired(int32 quest_id, int16 quest_step){
	m_requiredQuests.writelock(__FUNCTION__, __LINE__);
	if (required_quests.count(quest_id) == 0)
		required_quests.insert(make_pair(quest_id, new vector<int16>));
	else{
		for (int32 i = 0; i < required_quests[quest_id]->size(); i++){
			if (required_quests[quest_id]->at(i) == quest_step){
				m_requiredQuests.releasewritelock(__FUNCTION__, __LINE__);
				return;
			}
		}
	}

	has_quests_required = true;
	required_quests[quest_id]->push_back(quest_step);
	m_requiredQuests.releasewritelock(__FUNCTION__, __LINE__);
}

bool Spawn::HasQuestsRequired(){
	return has_quests_required;
}

bool Spawn::HasHistoryRequired(){
	return has_history_required;
}

void Spawn::SetRequiredHistory(int32 event_id, int32 value1, int32 value2){
	LUAHistory* set_value = new LUAHistory();
	set_value->Value = value1;
	set_value->Value2 = value2;
	set_value->SaveNeeded = false;
	m_requiredHistory.writelock(__FUNCTION__, __LINE__);
	if (required_history.count(event_id) == 0)
		required_history.insert(make_pair(event_id, set_value));
	else
	{
		LUAHistory* tmp_value = required_history[event_id];
		required_history[event_id] = set_value;
		safe_delete(tmp_value);
	}
	has_history_required = true;
	m_requiredHistory.releasewritelock(__FUNCTION__, __LINE__);
}

void Spawn::SetTransporterID(int32 id){
	transporter_id = id;
}

int32 Spawn::GetTransporterID(){
	return transporter_id;
}

void Spawn::InitializePosPacketData(Player* player, PacketStruct* packet, bool bSpawnUpdate) {
	int16 version = packet->GetVersion();

	int32 new_grid_id = 0;
	int32 new_widget_id = 0;
	float new_y = 0.0f;
	float ground_diff = 0.0f;
	if(player->GetMap() != nullptr && player->GetMap()->IsMapLoaded())
	{
		m_GridMutex.writelock(__FUNCTION__, __LINE__);
		std::map<int32,TimedGridData>::iterator itr = established_grid_id.find(version);
		if ( itr == established_grid_id.end() || itr->second.npc_save || itr->second.timestamp <= (Timer::GetCurrentTime2()))
		{
			if(itr != established_grid_id.end() && itr->second.x == GetX() && itr->second.z == GetZ() && !itr->second.npc_save) {
				itr->second.timestamp = Timer::GetCurrentTime2()+100;
				itr->second.npc_save = false;
				new_grid_id = itr->second.grid_id;
				new_widget_id = itr->second.widget_id;
				new_y = itr->second.offset_y;
				
				if(player->GetMap() != GetMap()) {
					ground_diff = sqrtf((GetY() - itr->second.zone_ground_y) * (GetY() - itr->second.zone_ground_y));
				}
			}
			else {
				auto loc = glm::vec3(GetX(), GetZ(), GetY());
				new_y = player->FindBestZ(loc, nullptr, &new_grid_id, &new_widget_id);
				float zone_ground_y = new_y;
				if(player->GetMap() != GetMap()) {
					zone_ground_y = FindBestZ(loc, nullptr, nullptr, nullptr);
				}
				TimedGridData data;
				data.grid_id = new_grid_id;
				data.widget_id = new_widget_id;
				data.x = GetX();
				data.y = GetY();
				data.z = GetZ();
				data.offset_y = new_y;
				data.zone_ground_y = zone_ground_y;
				ground_diff = sqrtf((GetY() - zone_ground_y) * (GetY() - zone_ground_y));
				data.npc_save = false;
				data.timestamp = Timer::GetCurrentTime2()+100;
				established_grid_id.insert(make_pair(packet->GetVersion(), data));
			}
		}
		else {
			new_grid_id = itr->second.grid_id;
			new_widget_id = itr->second.widget_id;
			new_y = itr->second.offset_y;
			ground_diff = sqrtf((GetY() - itr->second.zone_ground_y) * (GetY() - itr->second.zone_ground_y));
		}
		m_GridMutex.releasewritelock(__FUNCTION__, __LINE__);
	}
	
	if(IsKnockedBack()) {
		packet->setDataByName("pos_grid_id", 0);
	}
	else {
		packet->setDataByName("pos_grid_id", new_grid_id != 0 ? new_grid_id : GetLocation());
	}
	
	bool include_heading = true;
	if (IsWidget() && ((Widget*)this)->GetIncludeHeading() == false)
		include_heading = false;
	else if (IsSign() && ((Sign*)this)->GetIncludeHeading() == false)
		include_heading = false;

	if (include_heading){
		packet->setDataByName("pos_heading1", appearance.pos.Dir1);
		packet->setDataByName("pos_heading2", appearance.pos.Dir2);
	}

	if (version <= 910) {
		packet->setDataByName("pos_collision_radius", appearance.pos.collision_radius > 0 ? appearance.pos.collision_radius : 32);
		packet->setDataByName("pos_size", size > 0 ? size : 32);
		packet->setDataByName("pos_size_multiplier", 32); //32 is normal
	}
	else {
		if (size == 0)
			size = 32;

		packet->setDataByName("pos_collision_radius", appearance.pos.collision_radius > 0 ? appearance.pos.collision_radius : 32);

		packet->setDataByName("pos_size", 1.0f);

		if (!IsPlayer())
			packet->setDataByName("pos_size", size > 0 ? (((float)size) / 32.0f) : 1.0f); // float not an integer
		else
			packet->setDataByName("pos_size", 1.0f);

		// please do not remove!  This makes it so NPCs for example do not resize large/small when you are in combat with them!
		packet->setDataByName("pos_size_ratio", 1.0f);
	}
	packet->setDataByName("pos_state", appearance.pos.state);
	
	bool include_location = true;
	if (IsWidget() && ((Widget*)this)->GetIncludeLocation() == false)
		include_location = false;
	else if (IsSign() && ((Sign*)this)->GetIncludeLocation() == false)
		include_location = false;

	if (include_location){
		if (IsWidget() && ((Widget*)this)->GetMultiFloorLift()) {
			Widget* widget = (Widget*)this;
			float x = appearance.pos.X - widget->GetWidgetX();
			float y = appearance.pos.Y - widget->GetWidgetY();
			float z = appearance.pos.Z - widget->GetWidgetZ();

			packet->setDataByName("pos_x", x);
			packet->setDataByName("pos_y", y);
			packet->setDataByName("pos_z", z);
		}
		else {
			packet->setDataByName("pos_x", appearance.pos.X);
			float result_y = appearance.pos.Y;
			if(!IsWidget() && !IsSign() && !IsObject() && !(IsFlyingCreature() || IsWaterCreature() || InWater())) {
				result_y = new_y;
			}
			if(GetMap() != player->GetMap()) {
				result_y = new_y;
				if(IsFlyingCreature() || IsWaterCreature() || InWater()) {
					result_y += ground_diff;
				}
			}
			packet->setDataByName("pos_y", result_y);
			packet->setDataByName("pos_z", appearance.pos.Z);
		}
		if (IsSign())
			packet->setDataByName("pos_unknown6", 3, 2);
	}

	if (IsPlayer()) {
		Skill* skill = ((Player*)this)->GetSkillByName("Swimming", false);
		sint16 swim_modifier = rule_manager.GetGlobalRule(R_Player, SwimmingSkillMinSpeed)->GetSInt16();
		if(skill) {
			sint16 max_val = 450;
			if(skill->max_val > 0)
				max_val = skill->max_val;
			sint16 max_swim_mod = rule_manager.GetGlobalRule(R_Player, SwimmingSkillMaxSpeed)->GetSInt16();
			float diff = (float)(skill->current_val + ((Player*)this)->GetStat(ITEM_STAT_SWIMMING)) / (float)max_val;
			sint16 diff_mod = (sint16)(float)max_swim_mod * diff;
			if(diff_mod > max_swim_mod)
				swim_modifier = max_swim_mod;
			else if(diff_mod > swim_modifier)
				swim_modifier = diff_mod;
		}
		packet->setDataByName("pos_swim_speed_modifier", swim_modifier);
		packet->setDataByName("pos_x_velocity", TransformFromFloat(GetSpeedX(), 5));
		packet->setDataByName("pos_y_velocity", TransformFromFloat(GetSpeedY(), 5));
		packet->setDataByName("pos_z_velocity", TransformFromFloat(GetSpeedZ(), 5));
	}
	if (appearance.pos.X2 == 0 && appearance.pos.Y2 == 0 && appearance.pos.Z2 && (appearance.pos.X != 0 || appearance.pos.Y != 0 || appearance.pos.Z != 0)) {
		appearance.pos.X2 = appearance.pos.X;
		appearance.pos.Y2 = appearance.pos.Y;
		appearance.pos.Z2 = appearance.pos.Z;
	}
	if (appearance.pos.X3 == 0 && appearance.pos.Y3 == 0 && appearance.pos.Z3 && (appearance.pos.X != 0 || appearance.pos.Y != 0 || appearance.pos.Z != 0)) {
		appearance.pos.X3 = appearance.pos.X;
		appearance.pos.Y3 = appearance.pos.Y;
		appearance.pos.Z3 = appearance.pos.Z;
	}
	//Transform To/From Float bits (original client)
	//pos_loc_offset[3]: 5
	//pos_x_velocity: 5
	//pos_y_velocity: 5
	//pos_z_velocity: 5
	//pos_heading1: 6
	//pos_heading2: 6
	//pos_speed: 8
	//pos_dest_loc_offset[3]: 5
	//pos_dest_loc_offset2[3]: 5
	//pos_heading_speed: 5
	//pos_move_type: 5 (speed_modifier)
	//pos_swim_speed_modifier: 5
	//pos_side_speed: 8
	//pos_vert_speed: 8
	//pos_requested_pitch: 6
	//pos_requested_pitch_speed: 5
	//pos_pitch: 6
	//pos_collision_radius: 5
	//pos_size: 5
	//actor_stop_range: 5
	//this is for original box client, destinations used to be offsets
	if(version <= 373 || version > 561 || !IsPlayer()) {
		if (appearance.pos.X2 != 0 || appearance.pos.Y2 != 0 || appearance.pos.Z2 != 0) {
			packet->setDataByName("pos_dest_loc_offset", TransformFromFloat(appearance.pos.X2 - appearance.pos.X, 5));
			packet->setDataByName("pos_dest_loc_offset", TransformFromFloat(appearance.pos.Y2 - appearance.pos.Y, 5), 1);
			packet->setDataByName("pos_dest_loc_offset", TransformFromFloat(appearance.pos.Z2 - appearance.pos.Z, 5), 2);
		}
		if (appearance.pos.X3 != 0 || appearance.pos.Y3 != 0 || appearance.pos.Z3 != 0) {
			packet->setDataByName("pos_dest_loc_offset2", TransformFromFloat(appearance.pos.X3 - appearance.pos.X, 5));
			packet->setDataByName("pos_dest_loc_offset2", TransformFromFloat(appearance.pos.Y3 - appearance.pos.Y, 5), 1);
			packet->setDataByName("pos_dest_loc_offset2", TransformFromFloat(appearance.pos.Z3 - appearance.pos.Z, 5), 2);
		}
	}

	bool bSendSpeed = true;

	// fixes lifts dropping back to the floor in zones like northfreeport when set as TransportSpawn (Some zones do not support multifloor lifts)
	if (IsWidget() && (((Widget*)this)->GetMultiFloorLift() || (IsTransportSpawn()))) {
		Widget* widget = (Widget*)this;

		float x;
		float y;
		float z;
		
		bool setCoords = false;
		if(!widget->GetMultiFloorLift() && version >= 561) {
			packet->setDataByName("pos_next_x", appearance.pos.X2);
			packet->setDataByName("pos_next_y", appearance.pos.Y2);
			packet->setDataByName("pos_next_z", appearance.pos.Z2);

			packet->setDataByName("pos_x3", appearance.pos.X);
			packet->setDataByName("pos_y3", appearance.pos.Y);
			packet->setDataByName("pos_z3", appearance.pos.Z);
			setCoords = true;
		}
		else if (IsRunning()){
			x = appearance.pos.X2 - widget->GetWidgetX();
			y = appearance.pos.Y2 - widget->GetWidgetY();
			z = appearance.pos.Z2 - widget->GetWidgetZ();
		}
		else {
			x = appearance.pos.X - widget->GetWidgetX();
			y = appearance.pos.Y - widget->GetWidgetY();
			z = appearance.pos.Z - widget->GetWidgetZ();
		}
		
		if(!setCoords) {
			packet->setDataByName("pos_next_x", x);
			packet->setDataByName("pos_next_y", y);
			packet->setDataByName("pos_next_z", z);

			packet->setDataByName("pos_x3", x);
			packet->setDataByName("pos_y3", y);
			packet->setDataByName("pos_z3", z);
		}
	}
	else if(IsFlyingCreature() || IsWaterCreature() || InWater()) {
			packet->setDataByName("pos_next_x", appearance.pos.X2);
			packet->setDataByName("pos_next_y", (GetMap() != player->GetMap()) ? (ground_diff + new_y) : appearance.pos.Y2);
			packet->setDataByName("pos_next_z", appearance.pos.Z2);

			packet->setDataByName("pos_x3", appearance.pos.X);
			packet->setDataByName("pos_y3", (GetMap() != player->GetMap()) ? (ground_diff + new_y) : appearance.pos.Y);
			packet->setDataByName("pos_z3", appearance.pos.Z);
	}
	//If this is a spawn update or this spawn is currently moving we can send these values, otherwise set speed and next_xyz to 0
	//This fixes the bug where spawns with movement scripts face south when initially spawning if they are at their target location.
	else if (bSpawnUpdate || memcmp(&appearance.pos.X, &appearance.pos.X2, sizeof(float) * 3) != 0) {
		packet->setDataByName("pos_next_x", appearance.pos.X2);
		packet->setDataByName("pos_next_y", appearance.pos.Y2);
		packet->setDataByName("pos_next_z", appearance.pos.Z2);
		packet->setDataByName("pos_x3", appearance.pos.X3);
		packet->setDataByName("pos_y3", appearance.pos.Y3);
		packet->setDataByName("pos_z3", appearance.pos.Z3);
	}
	else
	{
		bSendSpeed = false;
	}
	//packet->setDataByName("pos_unknown2", 4, 2);

	int16 speed_multiplier = rule_manager.GetGlobalRule(R_Spawn, SpeedMultiplier)->GetInt16(); // was 1280, 600 and now 300... investigating why
	int8 movement_mode = 0;
	if (IsPlayer()) {
		Player* player = static_cast<Player*>(this);
		sint16 pos_packet_speed = player->GetPosPacketSpeed() * speed_multiplier;
		sint16 side_speed = player->GetSideSpeed() * speed_multiplier;
		packet->setDataByName("pos_speed", pos_packet_speed);
		packet->setDataByName("pos_side_speed", side_speed);
	}
	else if (bSendSpeed && (!IsNPC() || Alive())) {
		sint16 side_speed = GetSpeed() * speed_multiplier;
		packet->setDataByName("pos_speed", side_speed);
		if(side_speed != 0 && ((IsWidget() && ((Widget*)this)->GetMultiFloorLift()) || IsTransportSpawn())) {
			movement_mode = 2;
		}
	}
	else if(((IsWidget() && ((Widget*)this)->GetMultiFloorLift()) || (!IsNPC() && version <= 561 && !IsRunning()))) {
		movement_mode = 2;
	}
	if(IsFlyingCreature() || IsWaterCreature() || InWater()) {
		movement_mode = 2;
	}
	
	if (IsNPC() || IsPlayer()) {
		packet->setDataByName("pos_move_type", 25);
	}
	else if (IsWidget() || IsSign()) {
		packet->setDataByName("pos_move_type", 11);
	}
	else if(IsGroundSpawn()) {
		packet->setDataByName("pos_move_type", 16);
	}
	
	if (!IsPlayer()) { // has to be 2 or NPC's warp around when moving
		packet->setDataByName("pos_movement_mode", movement_mode);
	}
	
	packet->setDataByName("face_actor_id", 0xFFFFFFFF);

	packet->setDataByName("pos_pitch1", appearance.pos.Pitch1);
	packet->setDataByName("pos_pitch2", appearance.pos.Pitch2);
	packet->setDataByName("pos_roll", appearance.pos.Roll);
}

void Spawn::InitializeInfoPacketData(Player* spawn, PacketStruct* packet) {
	int16 version = packet->GetVersion();

	bool spawnHiddenFromClient = false;

	int8 classicFlags = 0;
	// radius of 0 is always seen, -1 is never seen (unless items/spells override), larger than 0 is a defined radius to restrict visibility
	sint32 radius = rule_manager.GetGlobalRule(R_PVP, InvisPlayerDiscoveryRange)->GetSInt32();
	if (radius != 0 && (Spawn*)spawn != this && this->IsPlayer() && !spawn->CanSeeInvis((Entity*)this))
		spawnHiddenFromClient = true;

	if (!spawnHiddenFromClient && (appearance.targetable == 1 || appearance.show_level == 1 || appearance.display_name == 1)) {
		if (!IsObject() && !IsGroundSpawn() && !IsWidget() && !IsSign()) {
			int8 percent = 0;
			if (GetHP() > 0)
				percent = (int8)(((float)GetHP() / GetTotalHP()) * 100);
			
			if (version >= 373) {
				if (percent < 100) {
					packet->setDataByName("hp_remaining", 100 ^ percent);
				}
				else
					packet->setDataByName("hp_remaining", 0);
			}
			else {
				if (percent > 100)
					percent = 100;
				packet->setDataByName("hp_remaining", percent);
			}
			if (GetTotalPower() > 0) {
				percent = (int8)(((float)GetPower() / GetTotalPower()) * 100);
				if (percent > 0)
					packet->setDataByName("power_percent", percent);
				else
					packet->setDataByName("power_percent", 0);
			}
		}
	}

	if (version <= 561) {
		packet->setDataByName("name", appearance.name);
		for (int8 i = 0; i < 8; i++)
			packet->setDataByName("unknown1", 0xFF, i);
		if (appearance.show_level == 0)
			packet->setDataByName("hide_health", 1);
	}
	if (GetHP() <= 0 && IsEntity()) {
		packet->setDataByName("corpse", 1);
		if(HasLoot())
			packet->setDataByName("loot_icon", 1); 
	}
	if (!IsPlayer())
		packet->setDataByName("npc", 1);
	if (GetMerchantID() > 0)
		packet->setDataByName("merchant", 1);
	
	packet->setDataByName("effective_level", IsEntity() && ((Entity*)this)->GetInfoStruct()->get_effective_level() != 0 ? (int8)((Entity*)this)->GetInfoStruct()->get_effective_level() : (int8)GetLevel());
	packet->setDataByName("level", (int8)GetLevel());
	packet->setDataByName("unknown4", (int8)GetLevel());
	packet->setDataByName("difficulty", GetDifficulty()); //6);
	packet->setDataByName("unknown6", 1);
	packet->setDataByName("heroic_flag", appearance.heroic_flag);
	packet->setDataByName("class", appearance.adventure_class);

	int16 model_type = appearance.model_type;
	if (GetIllusionModel() != 0) {
		if (IsPlayer()) {
			if (((Player*)this)->get_character_flag(CF_SHOW_ILLUSION)) {
				model_type = GetIllusionModel();
			}
		}
		else
			model_type = GetIllusionModel();
	}

	int16 sogaModelType = appearance.soga_model_type;
	if (spawnHiddenFromClient)
	{
		model_type = 0;
		sogaModelType = 0;
	}

	if(version <= 373 && (model_type == 5864 || model_type == 5865 || model_type == 4015)) {
		model_type = 4034;
	}
	else if(version <= 561 && model_type == 7039) { // goblin
	
		model_type = 145;
	}
	packet->setDataByName("model_type", model_type);
	if (appearance.soga_model_type == 0)
		packet->setDataByName("soga_model_type", model_type);
	else
		packet->setDataByName("soga_model_type", sogaModelType);

	int16 action_state = appearance.action_state;
	if(IsEntity()) {
		std::string actionState = "";
		if (GetTempActionState() >= 0) {
			action_state = GetTempActionState();
			actionState = ((Entity*)this)->GetInfoStruct()->get_combat_action_state();
		}
		else {
			actionState = ((Entity*)this)->GetInfoStruct()->get_action_state();
		}
		
		Client* client = spawn->GetClient();
		if(IsEntity() && client) {
			if(actionState.size() > 0) {
				Emote* emote = visual_states.FindEmote(actionState, client->GetVersion());
				if(emote != NULL)
					action_state = emote->GetVisualState();
			}
		}
	}
	packet->setDataByName("action_state", action_state);
	
	bool scaredOfPlayer = false;
	
	if(IsCollector() && spawn->GetCollectionList()->HasCollectionsToHandIn())
		packet->setDataByName("visual_state", VISUAL_STATE_COLLECTION_TURN_IN);
	else if(!IsRunning() && IsNPC() && IsScaredByStrongPlayers() && spawn->GetArrowColor(GetLevel()) == ARROW_COLOR_GRAY &&
	(GetDistance(spawn)) <= ((NPC*)this)->GetAggroRadius() && CheckLoS(spawn)) {
		packet->setDataByName("visual_state", VISUAL_STATE_IDLE_AFRAID);
		scaredOfPlayer = true;
	}
	else if (GetTempVisualState() >= 0)
		packet->setDataByName("visual_state", GetTempVisualState());
	else
		packet->setDataByName("visual_state", appearance.visual_state);
	
	if (IsNPC() && !IsPet() && !scaredOfPlayer)
	{
		if(((Entity*)this)->GetInfoStruct()->get_interaction_flag()) {
			if(((Entity*)this)->GetInfoStruct()->get_interaction_flag() == 255) { 
				packet->setDataByName("interaction_flag", 0);
				classicFlags += INFO_CLASSIC_FLAG_NOLOOK;
			}
			else {
				packet->setDataByName("interaction_flag", ((Entity*)this)->GetInfoStruct()->get_interaction_flag()); //this makes NPCs head turn to look at you (12)
			}
		}
		else {
			packet->setDataByName("interaction_flag", 12); //turn head since no other value is set
		}
	}
	packet->setDataByName("emote_state", appearance.emote_state);
	packet->setDataByName("mood_state", appearance.mood_state);
	packet->setDataByName("gender", appearance.gender);
	packet->setDataByName("race", appearance.race);
	if (IsEntity()) {
		Entity* entity = ((Entity*)this);
		packet->setDataByName("combat_voice", entity->GetCombatVoice());
		packet->setDataByName("emote_voice", entity->GetEmoteVoice());
		for (int i = 0; i < 25; i++) {
			if (i == 2) { //don't send helm if hidden flag
				if (IsPlayer()) {
					if (((Player*)this)->get_character_flag(CF_HIDE_HELM)) {
						packet->setDataByName("equipment_types", 0, i);
						packet->setColorByName("equipment_colors", 0, i);
						packet->setColorByName("equipment_highlights", 0, i);
						continue;
					}
				}
				if (IsBot()) {
					if (!((Bot*)this)->ShowHelm) {
						packet->setDataByName("equipment_types", 0, i);
						packet->setColorByName("equipment_colors", 0, i);
						packet->setColorByName("equipment_highlights", 0, i);
						continue;
					}
				}
			}
			else if (i == 19) { //don't send cloak if hidden
				if (IsPlayer()) {
					if (!((Player*)this)->get_character_flag(CF_SHOW_CLOAK)) {
						packet->setDataByName("equipment_types", 0, i);
						packet->setColorByName("equipment_colors", 0, i);
						packet->setColorByName("equipment_highlights", 0, i);
						continue;
					}
				}
				if (IsBot()) {
					if (!((Bot*)this)->ShowCloak) {
						packet->setDataByName("equipment_types", 0, i);
						packet->setColorByName("equipment_colors", 0, i);
						packet->setColorByName("equipment_highlights", 0, i);
						continue;
					}
				}
			}
			entity->MEquipment.lock();
			packet->setDataByName("equipment_types", entity->equipment.equip_id[i], i);
			packet->setColorByName("equipment_colors", entity->equipment.color[i], i);
			packet->setColorByName("equipment_highlights", entity->equipment.highlight[i], i);
			entity->MEquipment.unlock();
			
		}
		packet->setDataByName("mount_type", entity->GetMount());

		// find the visual flags
		int8 vis_flag = 0;
		//Invis + crouch flag check
		if (entity->IsStealthed()) {
			vis_flag += (INFO_VIS_FLAG_INVIS + INFO_VIS_FLAG_CROUCH);
			classicFlags += INFO_VIS_FLAG_INVIS + INFO_VIS_FLAG_CROUCH;
		}
		//Invis flag check
		else if (entity->IsInvis()) {
			vis_flag += INFO_VIS_FLAG_INVIS;
			classicFlags += INFO_VIS_FLAG_INVIS;
		}
		
		//Mount flag check
		if (entity->GetMount() > 0) {
			vis_flag += INFO_VIS_FLAG_MOUNTED;
		}
		
		//Hide hood check
		bool vis_hide_hood = false;
		if (IsPlayer() && ((Player*)this)->get_character_flag(CF_HIDE_HOOD)) {
			if(version > 561) {
				vis_flag += INFO_VIS_FLAG_HIDE_HOOD;
			}
			vis_hide_hood = true;
		}
		else if(IsPlayer()) {
			classicFlags += INFO_CLASSIC_FLAG_SHOW_HOOD;
		}
		
		if(!vis_hide_hood && appearance.hide_hood && version > 561) {
			vis_flag += INFO_VIS_FLAG_HIDE_HOOD;
		}
			
		if(version <= 561) {
			packet->setDataByName("flags", classicFlags);
		}
		packet->setDataByName("visual_flag", vis_flag);
		packet->setColorByName("mount_saddle_color", entity->GetMountSaddleColor());
		packet->setColorByName("mount_color", entity->GetMountColor());
		packet->setDataByName("hair_type_id", entity->features.hair_type);
		packet->setDataByName("chest_type_id", entity->features.chest_type);
		packet->setDataByName("wing_type_id", entity->features.wing_type);
		packet->setDataByName("legs_type_id", entity->features.legs_type);
		packet->setDataByName("soga_hair_type_id", entity->features.soga_hair_type);
		packet->setDataByName("facial_hair_type_id", entity->features.hair_face_type);
		packet->setDataByName("soga_facial_hair_type_id", entity->features.soga_hair_face_type);
		for (int i = 0; i < 3; i++) {
			packet->setDataByName("eye_type", entity->features.eye_type[i], i);
			packet->setDataByName("ear_type", entity->features.ear_type[i], i);
			packet->setDataByName("eye_brow_type", entity->features.eye_brow_type[i], i);
			packet->setDataByName("cheek_type", entity->features.cheek_type[i], i);
			packet->setDataByName("lip_type", entity->features.lip_type[i], i);
			packet->setDataByName("chin_type", entity->features.chin_type[i], i);
			packet->setDataByName("nose_type", entity->features.nose_type[i], i);
			packet->setDataByName("soga_eye_type", entity->features.soga_eye_type[i], i);
			packet->setDataByName("soga_ear_type", entity->features.soga_ear_type[i], i);
			packet->setDataByName("soga_eye_brow_type", entity->features.soga_eye_brow_type[i], i);
			packet->setDataByName("soga_cheek_type", entity->features.soga_cheek_type[i], i);
			packet->setDataByName("soga_lip_type", entity->features.soga_lip_type[i], i);
			packet->setDataByName("soga_chin_type", entity->features.soga_chin_type[i], i);
			packet->setDataByName("soga_nose_type", entity->features.soga_nose_type[i], i);
		}
		
		packet->setColorByName("skin_color", entity->features.skin_color);
		packet->setColorByName("model_color", entity->features.model_color);
		packet->setColorByName("eye_color", entity->features.eye_color);
		packet->setColorByName("hair_type_color", entity->features.hair_type_color);
		packet->setColorByName("hair_type_highlight_color", entity->features.hair_type_highlight_color);
		packet->setColorByName("hair_face_color", entity->features.hair_face_color);
		packet->setColorByName("hair_face_highlight_color", entity->features.hair_face_highlight_color);
		packet->setColorByName("hair_highlight", entity->features.hair_highlight_color);
		packet->setColorByName("wing_color1", entity->features.wing_color1);
		packet->setColorByName("wing_color2", entity->features.wing_color2);
		packet->setColorByName("hair_color1", entity->features.hair_color1);
		packet->setColorByName("hair_color2", entity->features.hair_color2);
		packet->setColorByName("soga_skin_color", entity->features.soga_skin_color);
		packet->setColorByName("soga_model_color", entity->features.soga_model_color);
		packet->setColorByName("soga_eye_color", entity->features.soga_eye_color);
		packet->setColorByName("soga_hair_color1", entity->features.soga_hair_color1);
		packet->setColorByName("soga_hair_color2", entity->features.soga_hair_color2);
		packet->setColorByName("soga_hair_type_color", entity->features.soga_hair_type_color);
		packet->setColorByName("soga_hair_type_highlight_color", entity->features.soga_hair_type_highlight_color);
		packet->setColorByName("soga_hair_face_color", entity->features.soga_hair_face_color);
		packet->setColorByName("soga_hair_face_highlight_color", entity->features.soga_hair_face_highlight_color);
		packet->setColorByName("soga_hair_highlight", entity->features.soga_hair_highlight_color);
		
		packet->setDataByName("body_size", entity->features.body_size);
		packet->setDataByName("body_age", entity->features.body_age);

		packet->setDataByName("soga_body_size", entity->features.soga_body_size);
		packet->setDataByName("soga_body_age", entity->features.soga_body_age);
	}
	else {
		EQ2_Color empty;
		empty.red = 255;
		empty.blue = 255;
		empty.green = 255;
		packet->setColorByName("skin_color", empty);
		packet->setColorByName("model_color", empty);
		packet->setColorByName("eye_color", empty);
		packet->setColorByName("soga_skin_color", empty);
		packet->setColorByName("soga_model_color", empty);
		packet->setColorByName("soga_eye_color", empty);
	}
	if (appearance.icon == 0) {
		if (appearance.attackable == 1)
			appearance.icon = 0;
		else if (GetDifficulty() > 0)
			appearance.icon = 4;
		else
			appearance.icon = 6;
	}

	// If Coe+ clients modify the values before we send
	// if not then just send the value we have.
	int8 temp_icon = appearance.icon;

	//Check if we need to add the hand icon..
	if ((temp_icon & 6) != 6 && appearance.display_hand_icon) {
		temp_icon |= 6;
	}

	//Icon value 28 for boats, set this without modifying the value
	if (version >= 1188 && temp_icon != 28) {
		if ((temp_icon & 64) > 0) {
			temp_icon -= 64;   // remove the DoV value;
			temp_icon += 128; // add the CoE value
		}
		if ((temp_icon & 32) > 0) {
			temp_icon -= 32;   // remove the DoV value;
			temp_icon += 64; // add the CoE value
		}
		if ((temp_icon & 4) > 0) {
			temp_icon -= 4;   // remove DoV value
			temp_icon += 8;   // add the CoE icon
		}
		if ((temp_icon & 6) > 0) {
			temp_icon -= 10;   // remove DoV value
			temp_icon += 12;   // add the CoE icon
		}
	}
	packet->setDataByName("icon", temp_icon);//appearance.icon);
	if(GetPickupItemID()) {
		if(version <= 546) {
			packet->setDataByName("house_icon", 1);//appearance.icon);
		}
	}
	int32 temp_activity_status = 0;

	if (!Alive() && GetTotalHP() > 0 && !IsObject() && !IsGroundSpawn())
		temp_activity_status = 1;
	
	if(version >= 1188 && GetChestDropTime()) {
		temp_activity_status = 0;
	}
	temp_activity_status += (IsNPC() || IsObject() || IsGroundSpawn()) ? 1 << 1 : 0;
	if (version > 561) {
		// Fix widget or sign having 'Play Legends of Norrath' or 'Tell' options in right click (client hard-coded entity commands)
		if(IsWidget() || IsSign())
			temp_activity_status = 2;

		if (IsGroundSpawn() || GetShowHandIcon())
			temp_activity_status += ACTIVITY_STATUS_INTERACTABLE_1188;

		if ((appearance.activity_status & ACTIVITY_STATUS_ROLEPLAYING) > 0)
			temp_activity_status += ACTIVITY_STATUS_ROLEPLAYING_1188;

		if ((appearance.activity_status & ACTIVITY_STATUS_ANONYMOUS) > 0)
			temp_activity_status += ACTIVITY_STATUS_ANONYMOUS_1188;

		if ((appearance.activity_status & ACTIVITY_STATUS_LINKDEAD) > 0)
			temp_activity_status += ACTIVITY_STATUS_LINKDEAD_1188;

		if ((appearance.activity_status & ACTIVITY_STATUS_CAMPING) > 0)
			temp_activity_status += ACTIVITY_STATUS_CAMPING_1188;

		if ((appearance.activity_status & ACTIVITY_STATUS_LFG) > 0)
			temp_activity_status += ACTIVITY_STATUS_LFG_1188;

		if ((appearance.activity_status & ACTIVITY_STATUS_LFW) > 0)
			temp_activity_status += ACTIVITY_STATUS_LFW_1188;

		if ((appearance.activity_status & ACTIVITY_STATUS_SOLID) > 0)
			temp_activity_status += ACTIVITY_STATUS_SOLID_1188;

		if ((appearance.activity_status & ACTIVITY_STATUS_IMMUNITY_GAINED) > 0)
			temp_activity_status += ACTIVITY_STATUS_IMMUNITY_GAINED_1188;

		if ((appearance.activity_status & ACTIVITY_STATUS_IMMUNITY_REMAINING) > 0)
			temp_activity_status += ACTIVITY_STATUS_IMMUNITY_REMAINING_1188;

		if ((appearance.activity_status & ACTIVITY_STATUS_AFK) > 0)
			temp_activity_status += ACTIVITY_STATUS_AFK_1188;

		if (EngagedInCombat())
			temp_activity_status += ACTIVITY_STATUS_INCOMBAT_1188;

		// if this is either a boat or lift let the client be manipulated by the object
		// doesn't work for DoF client version 546
		if (appearance.icon == 28 || appearance.icon == 12 || IsTransportSpawn())
		{
			// there is some other flags that setting with a transport breaks their solidity/ability to properly transport
			// thus we just consider the following flags for now as all necessary
			temp_activity_status = ACTIVITY_STATUS_SOLID_1188;
			temp_activity_status += ACTIVITY_STATUS_ISTRANSPORT_1188;
		}
	}
	else if (version == 561) {
		// Fix widget or sign having 'Play Legends of Norrath' or 'Tell' options in right click (client hard-coded entity commands)
		if(IsWidget() || IsSign())
			temp_activity_status = 2;

		if (IsGroundSpawn() || GetShowHandIcon())
			temp_activity_status += ACTIVITY_STATUS_INTERACTABLE_561;

		if ((appearance.activity_status & ACTIVITY_STATUS_ROLEPLAYING) > 0)
			temp_activity_status += ACTIVITY_STATUS_ROLEPLAYING_561;

		if ((appearance.activity_status & ACTIVITY_STATUS_ANONYMOUS) > 0)
			temp_activity_status += ACTIVITY_STATUS_ANONYMOUS_561;

		if ((appearance.activity_status & ACTIVITY_STATUS_LINKDEAD) > 0)
			temp_activity_status += ACTIVITY_STATUS_LINKDEAD_561;

		if ((appearance.activity_status & ACTIVITY_STATUS_CAMPING) > 0)
			temp_activity_status += ACTIVITY_STATUS_CAMPING_561;

		if ((appearance.activity_status & ACTIVITY_STATUS_LFG) > 0)
			temp_activity_status += ACTIVITY_STATUS_LFG_561;

		if ((appearance.activity_status & ACTIVITY_STATUS_LFW) > 0)
			temp_activity_status += ACTIVITY_STATUS_LFW_561;

		if ((appearance.activity_status & ACTIVITY_STATUS_SOLID) > 0)
			temp_activity_status += ACTIVITY_STATUS_SOLID_561;

		if ((appearance.activity_status & ACTIVITY_STATUS_IMMUNITY_GAINED) > 0)
			temp_activity_status += ACTIVITY_STATUS_IMMUNITY_GAINED_561;

		if ((appearance.activity_status & ACTIVITY_STATUS_IMMUNITY_REMAINING) > 0)
			temp_activity_status += ACTIVITY_STATUS_IMMUNITY_REMAINING_561;

		if ((appearance.activity_status & ACTIVITY_STATUS_AFK) > 0)
			temp_activity_status += ACTIVITY_STATUS_AFK_561;

		if (EngagedInCombat())
			temp_activity_status += ACTIVITY_STATUS_INCOMBAT_561;

		// if this is either a boat or lift let the client be manipulated by the object
		// doesn't work for DoF client version 546
		if (appearance.icon == 28 || appearance.icon == 12 || IsTransportSpawn())
		{
			// there is some other flags that setting with a transport breaks their solidity/ability to properly transport
			// thus we just consider the following flags for now as all necessary
			temp_activity_status = ACTIVITY_STATUS_SOLID_561;
			temp_activity_status += ACTIVITY_STATUS_ISTRANSPORT_561;
		}
	}
	else
	{
		temp_activity_status = appearance.activity_status;
		if(IsNPC())
			temp_activity_status = 0xFF;

		// this only partially fixes lifts in classic 283 client if you move just as the lift starts to move
		if (appearance.icon == 28 || appearance.icon == 12)
			packet->setDataByName("is_transport", 1);

		if (MeetsSpawnAccessRequirements(spawn))
			packet->setDataByName("hand_icon", appearance.display_hand_icon);
		else {
			if ((req_quests_override & 256) > 0)
				packet->setDataByName("hand_icon", 1);
		}

		if (IsPlayer()) {
			if (((Player*)this)->get_character_flag(CF_AFK))
				packet->setDataByName("afk", 1);
			if ((appearance.activity_status & ACTIVITY_STATUS_ROLEPLAYING) > 0)
				packet->setDataByName("roleplaying", 1);
			if ((appearance.activity_status & ACTIVITY_STATUS_ANONYMOUS) > 0)
				packet->setDataByName("anonymous", 1);
			if ((appearance.activity_status & ACTIVITY_STATUS_LINKDEAD) > 0)
				packet->setDataByName("linkdead", 1);
			if ((appearance.activity_status & ACTIVITY_STATUS_CAMPING) > 0)
				packet->setDataByName("camping", 1);
			if ((appearance.activity_status & ACTIVITY_STATUS_LFG) > 0)
				packet->setDataByName("lfg", 1);
		}
		
		if (EngagedInCombat()) {
			packet->setDataByName("auto_attack", 1);
		}
		
		if ((appearance.activity_status & ACTIVITY_STATUS_SOLID) > 0)
			packet->setDataByName("solid_object", 1);
	}

	packet->setDataByName("activity_status", temp_activity_status); //appearance.activity_status);
	// If player and player has a follow target
	/* Jan 2021 Note!! Setting follow_target 0xFFFFFFFF has the result in causing strange behavior in swimming.  Targetting a mob makes you focus down to its swim level, unable to swim above it.
	** in the same respect the player will drop like a rock to the bottom of the ocean (seems to be when self set to that flag?)
	** for now disabling this, if DoF needs it enabled for whatever reason then we need a version check added.
	*/
	if (IsPlayer()) {
		if (((Player*)this)->GetFollowTarget())
			packet->setDataByName("follow_target", version <= 561 ? (((Player*)this)->GetIDWithPlayerSpawn(((Player*)this)->GetFollowTarget())) : ((((Player*)this)->GetIDWithPlayerSpawn(((Player*)this)->GetFollowTarget()) * -1) - 1));
		else if(version <= 561) {
			packet->setDataByName("follow_target", 0xFFFFFFFF);
		}
		//else
		//	packet->setDataByName("follow_target", 0xFFFFFFFF);
	}
	//else if (!IsPet()) {
	//	packet->setDataByName("follow_target", 0xFFFFFFFF);
	//}
	
	// i think this is used in DoF as a way to make a client say they are in combat with this target and cannot camp, it forces you to stand up if self spawn sends this data
	if ((version > 561 || spawn != this) && GetTarget() && GetTarget()->GetTargetable())
		packet->setDataByName("target_id", ((spawn->GetIDWithPlayerSpawn(GetTarget()) * -1) - 1));
	else
		packet->setDataByName("target_id", 0xFFFFFFFF);

	//Send spell effects for target window
	if(IsEntity()){
		InfoStruct* info = ((Entity*)this)->GetInfoStruct();
		int8 i = 0;
		int16 backdrop = 0;
		int16 spell_icon = 0;
		int32 spell_id = 0;
		LuaSpell* spell = 0;
		((Entity*)this)->GetSpellEffectMutex()->readlock(__FUNCTION__, __LINE__);
		while(i < 30){
			//Change value of spell id for this packet if spell exists
			spell_id = info->spell_effects[i].spell_id;
			if(spell_id > 0)
				spell_id = 0xFFFFFFFF - spell_id;
			else
				spell_id = 0;
			packet->setSubstructDataByName("spell_effects", "spell_id", spell_id, i);

			//Change value of spell icon for this packet if spell exists
			spell_icon = info->spell_effects[i].icon;
			if(spell_icon > 0){
				if(!(spell_icon == 0xFFFF))
					spell_icon = 0xFFFF - spell_icon;
			}
			else
				spell_icon = 0;
			packet->setSubstructDataByName("spell_effects", "spell_icon", spell_icon, i);

			//Change backdrop values to match values in this packet
			backdrop = info->spell_effects[i].icon_backdrop;
			switch(backdrop){
				case 312:
					backdrop = 33080;
					break;
				case 313:
					backdrop = 33081;
					break;
				case 314:
					backdrop = 33082;
					break;
				case 315:
					backdrop = 33083;
					break;
				case 316:
					backdrop = 33084;
					break;
				case 317:
					backdrop = 33085;
					break;
				case (318 || 319):
					backdrop = 33086;
					break;
				default:
					break;
			}

			packet->setSubstructDataByName("spell_effects", "spell_icon_backdrop", backdrop, i);
			spell = info->spell_effects[i].spell;
			if (spell)
				packet->setSubstructDataByName("spell_effects", "spell_triggercount", spell->num_triggers, i);
			i++;
		}
		((Entity*)this)->GetSpellEffectMutex()->releasereadlock(__FUNCTION__, __LINE__);
	}
}

void Spawn::MoveToLocation(Spawn* spawn, float distance, bool immediate, bool mapped){
	if(!spawn)
		return;
	SetRunningTo(spawn);
	FaceTarget(spawn, false);

	if (!IsPlayer() && distance > 0.0f)
	{
		if ((IsFlyingCreature() || IsWaterCreature() || InWater()) && CheckLoS(spawn))
		{
			if (immediate)
				ClearRunningLocations();

			AddRunningLocation(spawn->GetX(), spawn->GetY(), spawn->GetZ(), GetSpeed(), distance, true, true, "", true);
		}
		else if (/*!mapped && */GetZone())
		{
			GetZone()->movementMgr->NavigateTo((Entity*)this, spawn->GetX(), spawn->GetY(), spawn->GetZ());
			last_grid_update = Timer::GetCurrentTime2();
		}
		else
		{
			if (immediate)
				ClearRunningLocations();

			AddRunningLocation(spawn->GetX(), spawn->GetY(), spawn->GetZ(), GetSpeed(), distance, true, true, "", mapped);
		}
	}
}

void Spawn::ProcessMovement(bool isSpawnListLocked){
	CheckProximities();
	
	if(IsBot() && ((Bot*)this)->IsCamping()) {
		((Bot*)this)->Begin_Camp();
	} 
	if(IsPlayer()){
		//Check if player is riding a boat, if so update pos (boat's current location + XYZ offsets)
		Player* player = ((Player*)this);
		int32 boat_id = player->GetBoatSpawn();
		Spawn* boat = 0;
		if(boat_id > 0)
			boat = GetZone()->GetSpawnByID(boat_id, isSpawnListLocked);

		//TODO: MAYBE do this for real boats, not lifts... GetWidgetTypeNameByTypeID
		/*if(boat){
			SetX(boat->GetX() + player->GetBoatX());
			SetY(boat->GetY() + player->GetBoatY());
			SetZ(boat->GetZ() + player->GetBoatZ());
		}*/
		return;
	}
	
	if(IsKnockedBack()) {
		if(CalculateSpawnProjectilePosition(GetX(), GetY(), GetZ()))
			return; // being launched!
	}
	
	if(reset_movement) {
		ResetMovement();
		reset_movement = false;
	}

	if (forceMapCheck && GetZone() != nullptr && GetMap() != nullptr && GetMap()->IsMapLoaded())
	{
		FixZ(true);
		forceMapCheck = false;
	}

	if (GetHP() <= 0 && !IsWidget())
		return;

	if (EngagedInCombat())
	{
		if(IsEntity() && (((Entity*)this)->IsMezzedOrStunned() || ((Entity*)this)->IsRooted())) {
			SetAppearancePosition(GetX(),GetY(),GetZ());
			if ( IsEntity() )
				((Entity*)this)->SetSpeed(0.0f);
			
			SetSpeed(0.0f);
			position_changed = true;
			changed = true;
			GetZone()->AddChangedSpawn(this);
			StopMovement();
			return;
		}
		int locations = 0;
		
		MMovementLocations.lock_shared();
		if (movement_locations) {
			locations = movement_locations->size();
		}
		MMovementLocations.unlock_shared();
		
		if (locations < 1 && GetZone() && ((Entity*)this)->IsFeared())
		{
			CalculateNewFearpoint();
			ValidateRunning(true, true);
		}
	}

	Spawn* followTarget = GetZone()->GetSpawnByID(m_followTarget, isSpawnListLocked);
	if (!followTarget && m_followTarget > 0)
		m_followTarget = 0;
	if (following && !IsPauseMovementTimerActive() && followTarget && !((Entity*)this)->IsFeared()) {

		// Need to clear m_followTarget before the zoneserver deletes it
		if (followTarget->GetHP() <= 0) {
			followTarget = 0;
			return;
		}

		if (!IsEntity() || (!((Entity*)this)->IsCasting() && !((Entity*)this)->IsMezzedOrStunned() && !((Entity*)this)->IsRooted())) {
			if (GetBaseSpeed() > 0) {
				CalculateRunningLocation();
			}
			else {
				float speed = 4.0f;
				if (IsEntity())
					speed = ((Entity*)this)->GetMaxSpeed();
				if (IsEntity())
					((Entity*)this)->SetSpeed(speed);
				
					SetSpeed(speed);
			}
			MovementLocation tmpLoc;
			MovementLocation* loc = 0;
			MMovementLocations.lock_shared();
			if(movement_locations && movement_locations->size() > 0){
				loc = movement_locations->front();
				if(loc) {
					tmpLoc.attackable = loc->attackable;
					tmpLoc.gridid = loc->gridid;
					tmpLoc.lua_function = string(loc->lua_function);
					tmpLoc.mapped = loc->mapped;
					tmpLoc.reset_hp_on_runback = loc->reset_hp_on_runback;
					tmpLoc.speed = loc->speed;
					tmpLoc.stage = loc->stage;
					tmpLoc.x = loc->x;
					tmpLoc.y = loc->y;
					tmpLoc.z = loc->z;
					loc = &tmpLoc;
				}
			}
			MMovementLocations.unlock_shared();

			float dist = GetDistance(followTarget, true);
			if ((!EngagedInCombat() && m_followDistance > 0 && dist <= m_followDistance) || 
				(dist <= rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat())) {
				ClearRunningLocations();
				CalculateRunningLocation(true);
			}
			else if (loc) {
				float distance = GetDistance(followTarget, loc->x, loc->y, loc->z);
				if ( (!EngagedInCombat() && m_followDistance > 0 && distance > m_followDistance) ||
					 ( EngagedInCombat() && distance > rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat())) {
					MoveToLocation(followTarget, rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat(), true, loc->mapped);
					CalculateRunningLocation();
				}
			}
			else {
				MoveToLocation(followTarget, rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat(), false);
				CalculateRunningLocation();
			}
		}
	}

	bool movementCase = false;
	// Movement loop is only for scripted paths
	if(!EngagedInCombat() && !IsPauseMovementTimerActive() && !NeedsToResumeMovement() && (!IsNPC() || !((NPC*)this)->m_runningBack)){
		MMovementLoop.writelock();
		if(movement_loop.size() > 0 && movement_index < movement_loop.size())
		{
			movementCase = true;
			// Get the target location
			MovementData* data = movement_loop[movement_index];
			// need to resume our movement
			if(resume_movement){
				MMovementLocations.lock();
				if (movement_locations){
					while (movement_locations->size()){
						safe_delete(movement_locations->front());
						movement_locations->pop_front();
					}
					movement_locations->clear();
				}
				MMovementLocations.unlock();

				data = movement_loop[movement_index];
				
				if(data)
				{
					if(IsEntity()) {
						((Entity*)this)->SetSpeed(data->speed);
					}
					SetSpeed(data->speed);
					if(data->use_movement_location_heading)
						SetHeading(data->heading);
					else if(!IsWidget())
						FaceTarget(data->x, data->z);
					// 0 delay at target location, need to set multiple locations
					if(data->delay == 0 && movement_loop.size() > 0) {
						int16 tmp_index = movement_index+1;
						MovementData* data2 = 0;
						if(tmp_index < movement_loop.size()) 
							data2 = movement_loop[tmp_index];
						else
							data2 = movement_loop[0];
						AddRunningLocation(data->x, data->y, data->z, data->speed, 0, true, false, "", true);				
						AddRunningLocation(data2->x, data2->y, data2->z, data2->speed, 0, true, true, "", true);
					}
					// delay at target location, only need to set 1 location
					else
						AddRunningLocation(data->x, data->y, data->z, data->speed);
				}
				movement_start_time = 0;
				resume_movement = false;
			}
			// If we are not moving or we have arrived at our destination
			else if(!IsRunning() || (data && data->x == GetX() && data->y == GetY() && data->z == GetZ())){
				// If we were moving remove the last running location (the point we just arrived at)
				if(IsRunning()) {
					RemoveRunningLocation();
				}

				// If this waypoint has a delay and we just arrived here (movement_start_time == 0)
				if(data && data->delay > 0 && movement_start_time == 0){
					// Set the current time
					movement_start_time = Timer::GetCurrentTime2();
					// If this waypoint had a lua function then call it
					if(data->lua_function.length() > 0)
						GetZone()->CallSpawnScript(this, SPAWN_SCRIPT_CUSTOM, 0, data->lua_function.c_str());

					int16 nextMove;
					if ((int16)(movement_index + 1) < movement_loop.size())
						nextMove = movement_index + 1;
					else
						nextMove = 0;
					// Get the next target location
					data = movement_loop[nextMove];
					
					//Go ahead and face the next location
					if(data) {
						FaceTarget(data->x, data->z);
					}
				}
				// If this waypoint has no delay or we have waited the required time (current time >= delay + movement_start_time)
				else if(data && data->delay == 0 || (data && data->delay > 0 && Timer::GetCurrentTime2() >= (data->delay+movement_start_time))) {
					// if no delay at this waypoint but a lua function for it then call the function
					if(data->delay == 0 && data->lua_function.length() > 0)
						GetZone()->CallSpawnScript(this, SPAWN_SCRIPT_CUSTOM, 0, data->lua_function.c_str());
					// since we ran a lua function make sure the movement loop is still alive and accurate
					if(movement_loop.size() > 0)
					{
						// Advance the current movement loop index
						if((int16)(movement_index+1) < movement_loop.size())
							movement_index++;
						else
							movement_index = 0;
						// Get the next target location
						data = movement_loop[movement_index];
						
						// set the speed for that location
						SetSpeed(data->speed);

						if(!IsWidget())
						// turn towards the location
							FaceTarget(data->x, data->z);
						
						// If 0 delay at location get and set data for the point after it
						if(data->delay == 0 && movement_loop.size() > 0){
							MMovementLocations.lock();
							if(movement_locations)
							{
								while (movement_locations->size()){
									safe_delete(movement_locations->front());
									movement_locations->pop_front();
								}
								// clear current target locations
								movement_locations->clear();
							}
							MMovementLocations.unlock();
							// get the data for the location after out new location
							int16 tmp_index = movement_index+1;
							MovementData* data2 = 0;
							if(tmp_index < movement_loop.size()) 
								data2 = movement_loop[tmp_index];
							else
								data2 = movement_loop[0];
							// set the first location (adds it to movement_locations that we just cleared)
							AddRunningLocation(data->x, data->y, data->z, data->speed, 0, true, false, "", true);
							// set the location after that
							AddRunningLocation(data2->x, data2->y, data2->z, data2->speed, 0, true, true, "", true);
						}
						// there is a delay at the next location so we only need to set it
						else {
							AddRunningLocation(data->x, data->y, data->z, data->speed, 0, true, true, "", true);
						}

						// reset this timer to 0 now that we are moving again
						movement_start_time = 0;
					}
				}
			}
			// moving and not at target location yet
			else if(GetBaseSpeed() > 0) {
				CalculateRunningLocation();
			}
			// not moving, have a target location but not at it yet
			else if (data) {
				SetSpeed(data->speed);
				AddRunningLocation(data->x, data->y, data->z, data->speed);
			}
		}
		MMovementLoop.releasewritelock();
	}
	
	if (!movementCase && IsRunning() && !IsPauseMovementTimerActive()) {
		CalculateRunningLocation();
		//last_movement_update = Timer::GetCurrentTime2();
	}
	else if(movementCase)
	{
		//last_movement_update = Timer::GetCurrentTime2();
	}
	/*else if (IsNPC() && !IsRunning() && !EngagedInCombat() && ((NPC*)this)->GetRunbackLocation()) {
		// Is an npc that is not moving and not engaged in combat but has a run back location set then clear the runback location
		LogWrite(NPC_AI__DEBUG, 7, "NPC_AI", "Clear runback location for %s", GetName());
		((NPC*)this)->ClearRunback();
		resume_movement = true;
		NeedsToResumeMovement(false);
	}*/
}

void Spawn::ResetMovement(){
	MMovementLoop.writelock();
	
	vector<MovementData*>::iterator itr;
	for(itr = movement_loop.begin(); itr != movement_loop.end(); itr++){
		safe_delete(*itr);
	}
	movement_loop.clear();
	movement_index = 0;
	resume_movement = true;
	ClearRunningLocations();

	MMovementLoop.releasewritelock();
	
	ValidateRunning(true, true);
}

void Spawn::AddMovementLocation(float x, float y, float z, float speed, int16 delay, const char* lua_function, float heading, bool include_heading){

	LogWrite(LUA__DEBUG, 5, "LUA", "AddMovementLocation: x: %.2f, y: %.2f, z: %.2f, speed: %.2f, delay: %i, lua: %s",
		x, y, z, speed, delay, string(lua_function).c_str());

	MovementData* data = new MovementData;
	data->x = x;
	data->y = y;
	data->z = z;
	data->speed = speed;
	data->delay = delay*1000;
	if(lua_function)
		data->lua_function = string(lua_function);
	
	data->heading = heading;
	data->use_movement_location_heading = include_heading;
	MMovementLoop.lock();
	movement_loop.push_back(data);
	MMovementLoop.unlock();
}

bool Spawn::ValidateRunning(bool lockMovementLocation, bool lockMovementLoop) {
	bool movement = false;

	if(lockMovementLocation) {
		MMovementLocations.lock_shared();
	}
	
	if(movement_locations) {
	movement = movement_locations->size() > 0;
	}
	
	if(lockMovementLocation) {
		MMovementLocations.unlock_shared();
	}
	
	if(IsPauseMovementTimerActive() || (IsEntity() && (((Entity*)this)->IsMezzedOrStunned() || ((Entity*)this)->IsRooted()))) {
		is_running = false;
		return false;
	}
	
	if(movement) {
		is_running = true;
		return true;
	}

	if(lockMovementLoop) {
		MMovementLoop.lock();
	}
	
	movement = movement_loop.size() > 0;
	
	if(movement) {
		is_running = true;
	}
	else {
		is_running = false;
	}
	
	if(lockMovementLoop) {
		MMovementLoop.unlock();
	}
	return movement;
}
bool Spawn::IsRunning(){
	return is_running;
}

void Spawn::RunToLocation(float x, float y, float z, float following_x, float following_y, float following_z){
	if(IsPauseMovementTimerActive() || (IsEntity() && (((Entity*)this)->IsMezzedOrStunned() || ((Entity*)this)->IsRooted()))) {
		is_running = false;
		return;
	}
	
	if(!IsWidget() && (!EngagedInCombat() || GetDistance(GetTarget()) > rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat()))
		FaceTarget(x, z);
	SetPos(&appearance.pos.X2, x, false);
	SetPos(&appearance.pos.Z2, z, false);
	SetPos(&appearance.pos.Y2, y, false);
	if(following_x == 0 && following_y == 0 && following_z == 0){
		SetPos(&appearance.pos.X3, x, false);
		SetPos(&appearance.pos.Z3, z, false);
		SetPos(&appearance.pos.Y3, y, false);
	}
	else{
		SetPos(&appearance.pos.X3, following_x, false);
		SetPos(&appearance.pos.Y3, following_y, false);
		SetPos(&appearance.pos.Z3, following_z, false);
	}

	is_running = true;
	position_changed = true;
	changed = true;
	GetZone()->AddChangedSpawn(this);
}

MovementLocation* Spawn::GetCurrentRunningLocation(){
	MovementLocation* ret = 0;
	MMovementLocations.lock_shared();
	if(movement_locations && movement_locations->size() > 0){
		ret = movement_locations->front();
	}
	MMovementLocations.unlock_shared();
	return ret;
}

MovementLocation* Spawn::GetLastRunningLocation(){
	MovementLocation* ret = 0;
	MMovementLocations.lock_shared();
	if(movement_locations && movement_locations->size() > 0){
		ret = movement_locations->back();
	}
	MMovementLocations.unlock_shared();
	return ret;
}

void Spawn::AddRunningLocation(float x, float y, float z, float speed, float distance_away, bool attackable, bool finished_adding_locations, string lua_function, bool isMapped){
	if(speed == 0)
		return;

	if ( IsEntity() )
		((Entity*)this)->SetSpeed(speed);
	else
		this->SetSpeed(speed);

	MovementLocation* current_location = 0;

	float distance = GetDistance(x, y, z, distance_away != 0);
	if(distance_away != 0){
		distance -= distance_away;

		x = x - (GetX() - x)*distance_away/distance;
		z = z - (GetZ() - z)*distance_away/distance;
	}
	
	MMovementLocations.lock();
	if(!movement_locations){
		movement_locations = new deque<MovementLocation*>();
	}
	MMovementLocations.unlock();
	
	MovementLocation* data = new MovementLocation;
	data->mapped = isMapped;
	data->x = x;
	data->y = y;
	data->z = z;
	data->speed = speed;
	data->attackable = attackable;
	data->lua_function = lua_function;
	data->gridid = 0; // used for runback defaults
	data->reset_hp_on_runback = false;

	MMovementLocations.lock_shared();
	if(movement_locations->size() > 0)
		current_location = movement_locations->back();
	MMovementLocations.unlock_shared();
	
	if(!current_location){
		SetSpawnOrigX(GetX());
		SetSpawnOrigY(GetY());
		SetSpawnOrigZ(GetZ());
		SetSpawnOrigHeading(GetHeading());
	}
	is_running = true;
	
	MMovementLocations.lock();
	movement_locations->push_back(data);
	MMovementLocations.unlock();
	if(!IsPauseMovementTimerActive() && finished_adding_locations){
		MMovementLocations.lock();
		current_location = movement_locations->front();
		SetSpeed(current_location->speed);
		if(movement_locations->size() > 1){		
			data = movement_locations->at(1);
			RunToLocation(current_location->x, current_location->y, current_location->z, data->x, data->y, data->z);
		}
		else
			RunToLocation(current_location->x, current_location->y, current_location->z, 0, 0, 0);
		MMovementLocations.unlock();
	}
}

bool Spawn::RemoveRunningLocation(){
	bool ret = false;
	MMovementLocations.lock();
	if(movement_locations && movement_locations->size() > 0){
		delete movement_locations->front();
		movement_locations->pop_front();
		ret = true;
	}
	MMovementLocations.unlock();
	
	ValidateRunning(true, false);
	return ret;
}

void Spawn::ClearRunningLocations(){
	while(RemoveRunningLocation()){}
}

void Spawn::NewWaypointChange(MovementLocation* data){	
	if(data){					
		if(NeedsToResumeMovement()){
				resume_movement = true;
				NeedsToResumeMovement(false);
		}
		if(!data->attackable)
			SetHeading(GetSpawnOrigHeading());
	}
		
	if (data && data->lua_function.length() > 0)
	GetZone()->CallSpawnScript(this, SPAWN_SCRIPT_CUSTOM, 0, data->lua_function.c_str());

	RemoveRunningLocation();
}

bool Spawn::CalculateChange(){
	bool remove_needed = false;
	MovementLocation* data = 0;
	MovementLocation tmpLoc;
	MMovementLocations.lock_shared();
	if(movement_locations){
		if(movement_locations->size() > 0){
			// Target location
			data = movement_locations->front();
			if(data) {
				tmpLoc.attackable = data->attackable;
				tmpLoc.gridid = data->gridid;
				tmpLoc.lua_function = string(data->lua_function);
				tmpLoc.mapped = data->mapped;
				tmpLoc.reset_hp_on_runback = data->reset_hp_on_runback;
				tmpLoc.speed = data->speed;
				tmpLoc.stage = data->stage;
				tmpLoc.x = data->x;
				tmpLoc.y = data->y;
				tmpLoc.z = data->z;
				data = &tmpLoc;
			}
			// If no target or we are at the target location need to remove this point
			if(!data || (data->x == GetX() && data->y == GetY() && data->z == GetZ()))
				remove_needed = true;
			}
	}
	MMovementLocations.unlock_shared();
		
	if(remove_needed){
		NewWaypointChange(data);
	}
	else if(data){
		// Speed is per second so we need a time_step (amount of time since the last update) to modify movement by
		float time_step = (Timer::GetCurrentTime2() - last_movement_update) * 0.001; // * 0.001 is the same as / 1000, float muliplications is suppose to be faster though

		// Get current location
		float nx = GetX();
		float ny = GetY();
		float nz = GetZ();
		
		// Get Forward vecotr
		float tar_vx = data->x - nx;
		float tar_vy = data->y - ny;
		float tar_vz = data->z - nz;

		// Multiply speed by the time_step to get how much should have changed over the last tick
		float speed = GetSpeed() * time_step;

		// Normalize the forward vector and multiply by speed, this gives us our change in coords, just need to add them to our current coords
		float len = sqrtf(tar_vx * tar_vx + tar_vy * tar_vy + tar_vz * tar_vz);
		tar_vx = (tar_vx / len) * speed;
		tar_vy = (tar_vy / len) * speed;
		tar_vz = (tar_vz / len) * speed;

		// Distance less then 0.5 just set the npc to the target location
		if (GetDistance(data->x, data->y, data->z, IsWidget() ? false : true) <= speed) {
			SetX(data->x, false);
			SetZ(data->z, false);
			SetY(data->y, false, true);
			remove_needed = true;
			NewWaypointChange(data);
		}
		else {
			SetX(nx + tar_vx, false);
			SetZ(nz + tar_vz, false);
			if ( IsWidget() )
				SetY(ny + tar_vy, false, true);
			else
				SetY(ny + tar_vy, false);
		}
	
		int32 newGrid = GetLocation();
		if (GetMap() != nullptr) {
			m_GridMutex.writelock(__FUNCTION__, __LINE__);
			std::map<int32,TimedGridData>::iterator itr = established_grid_id.begin();
			if ( itr == established_grid_id.end() || itr->second.timestamp <= (Timer::GetCurrentTime2()))
			{
				if(itr != established_grid_id.end() && itr->second.x == GetX() && itr->second.z == GetZ()) {
					itr->second.timestamp = Timer::GetCurrentTime2()+1000;
					itr->second.npc_save = true;
					newGrid = itr->second.grid_id;
				}
				else {
					auto loc = glm::vec3(GetX(), GetZ(), GetY());
					float new_z = FindBestZ(loc, nullptr, &newGrid);
					TimedGridData data;
					data.grid_id = newGrid;
					data.x = GetX();
					data.y = GetY();
					data.z = GetZ();
					data.npc_save = true;
					data.zone_ground_y = new_z;
					data.offset_y = new_z;
					data.timestamp = Timer::GetCurrentTime2()+1000;
					established_grid_id.insert(make_pair(0, data));
				}
			}
			else
				newGrid = itr->second.grid_id;
			m_GridMutex.releasewritelock(__FUNCTION__, __LINE__);
		}

		if ((!IsFlyingCreature() || IsTransportSpawn()) && newGrid != 0 && newGrid != GetLocation())
			SetLocation(newGrid);
	}
	return remove_needed;
}

void Spawn::CalculateRunningLocation(bool stop){

	bool pauseTimerEnabled = IsPauseMovementTimerActive();

	if (!pauseTimerEnabled && !stop && (last_location_update + 100) > Timer::GetCurrentTime2())
		return;
	else if (!pauseTimerEnabled && !stop)
		last_location_update = Timer::GetCurrentTime2();
	bool continueElseIf = true;
	bool removed = CalculateChange();
	if (stop || pauseTimerEnabled) {
		//following = false;
		SetPos(&appearance.pos.X2, GetX(), false);
		SetPos(&appearance.pos.Y2, GetY(), false);
		SetPos(&appearance.pos.Z2, GetZ(), false);
		SetPos(&appearance.pos.X3, GetX(), false);
		SetPos(&appearance.pos.Y3, GetY(), false);
		SetPos(&appearance.pos.Z3, GetZ(), false);
		continueElseIf = false;
	}
	else if (removed) {
		MMovementLocations.lock_shared();
		if(movement_locations) {
			if(movement_locations->size() > 0) {
				MovementLocation* current_location = movement_locations->at(0);
				if (movement_locations->size() > 1) {
					MovementLocation* data = movement_locations->at(1);
					RunToLocation(current_location->x, current_location->y, current_location->z, data->x, data->y, data->z);
				}
				else
					RunToLocation(current_location->x, current_location->y, current_location->z, 0, 0, 0);
				
				continueElseIf = false;
			}
		}
		MMovementLocations.unlock_shared();
	}
	
	if (continueElseIf && GetZone() && GetTarget() != NULL && EngagedInCombat())
	{
		if (GetDistance(GetTarget()) > rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat())
		{
			if ((IsFlyingCreature() || IsWaterCreature() || InWater()) && CheckLoS(GetTarget()))
				AddRunningLocation(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ(), GetSpeed(), 0, false);
			else
				GetZone()->movementMgr->NavigateTo((Entity*)this, GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ());
		}
		else
			((Entity*)this)->HaltMovement();
	}
	else if (continueElseIf && !following)
	{
		position_changed = true;
		changed = true;
		GetZone()->AddChangedSpawn(this);
	}
}
float Spawn::GetFaceTarget(float x, float z) {
	float angle;

	double diff_x = x - GetX();
	double diff_z = z - GetZ();

	if (diff_z == 0) {
		if (diff_x > 0)
			angle = 90;
		else
			angle = 270;
	}
	else
		angle = ((atan(diff_x / diff_z)) * 180) / 3.14159265358979323846;

	if(angle < 0)
		angle = angle + 360;
	else if(angle > -0.0000001 && angle < 0.0000001)
		angle = 0;
	else
		angle = angle + 180;

	if ((diff_x < 0 && diff_z != 0.0) || (diff_x == 0 && diff_z > 0.0))
		angle = angle + 180;

	if(angle > 360)
		angle = angle - 360.0;

	return angle;
}

void Spawn::FaceTarget(float x, float z){

	float angle;

	double diff_x = x - GetX();
	double diff_z = z - GetZ();

	if (diff_z == 0) {
		if (diff_x > 0)
			angle = 90;
		else
			angle = 270;
	}
	else
		angle = ((atan(diff_x / diff_z)) * 180) / 3.14159265358979323846;

	if(angle < 0)
		angle = angle + 360;
	else if(angle > -0.0000001 && angle < 0.0000001)
		angle = 0;
	else
		angle = angle + 180;
	
	if ((diff_x < 0 && diff_z != 0.0) || (diff_x == 0 && diff_z > 0.0))
		angle = angle + 180;

	if(angle > 360)
		angle = angle - 360.0;
	
	SetHeading(angle);
}

void Spawn::FaceTarget(Spawn* target, bool disable_action_state){
	if(!target)
		return;
	if(GetHP() > 0 && target->IsPlayer() && !EngagedInCombat()){
		if(!IsPet() && disable_action_state) {
			if(IsNPC()) {
				((NPC*)this)->StartRunback();
				((NPC*)this)->PauseMovement(30000);
			}
			SetTempActionState(0);
		}
	}
	FaceTarget(target->GetX(), target->GetZ());
}

bool Spawn::MeetsSpawnAccessRequirements(Player* player){
	bool ret = false;
	Quest* quest = 0;
	//Check if we meet all quest requirements first..
	m_requiredQuests.readlock(__FUNCTION__, __LINE__);
	if (player && required_quests.size() > 0) {
		map<int32, vector<int16>* >::iterator itr;
		for (itr = required_quests.begin(); itr != required_quests.end(); itr++) {
			player->AddQuestRequiredSpawn(this, itr->first);
			vector<int16>* quest_steps = itr->second;
			for (int32 i = 0; i < quest_steps->size(); i++) {
				quest = player->GetQuest(itr->first);
				if (req_quests_continued_access) {
					if (quest) {
						if (quest->GetQuestStepCompleted(quest_steps->at(i))) {
							ret = true;
							break;
						}
					}
					else if (player->HasQuestBeenCompleted(itr->first)) {
						ret = true;
						break;
					}
				}
				if (quest && quest->QuestStepIsActive(quest_steps->at(i))) {
					ret = true;
					break;
				}
			}
		}
	}
	else
		ret = true;
	m_requiredQuests.releasereadlock(__FUNCTION__, __LINE__);
	if (!ret)
		return ret;

	//Now check if the player meets all history requirements
	m_requiredHistory.readlock(__FUNCTION__, __LINE__);
	if (required_history.size() > 0){
		map<int32, LUAHistory*>::iterator itr;
		for (itr = required_history.begin(); itr != required_history.end(); itr++){
			player->AddHistoryRequiredSpawn(this, itr->first);
			LUAHistory* player_history = player->GetLUAHistory(itr->first);
			if (player_history){
				if (player_history->Value != itr->second->Value || player_history->Value2 != itr->second->Value2)
					ret = false;
			}
			else
				ret = false;
			if (!ret)
				break;
		}
	}
	m_requiredHistory.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

vector<Spawn*>* Spawn::GetSpawnGroup(){
	vector<Spawn*>* ret_list = 0;
	if(spawn_group_list){
		ret_list = new vector<Spawn*>();
		if(MSpawnGroup)
			MSpawnGroup->readlock(__FUNCTION__, __LINE__);
		ret_list->insert(ret_list->begin(), spawn_group_list->begin(), spawn_group_list->end());
		if(MSpawnGroup)
			MSpawnGroup->releasereadlock(__FUNCTION__, __LINE__);
	}
	return ret_list;
}

bool Spawn::HasSpawnGroup() {
	return spawn_group_list && spawn_group_list->size() > 0;
}

bool Spawn::IsInSpawnGroup(Spawn* spawn) {
	bool ret = false;
	if (HasSpawnGroup() && spawn) {
		vector<Spawn*>::iterator itr;
		for (itr = spawn_group_list->begin(); itr != spawn_group_list->end(); itr++) {
			if ((*itr) == spawn) {
				ret = true;
				break;
			}
		}
	}
	return ret;
}

Spawn* Spawn::IsSpawnGroupMembersAlive(Spawn* ignore_spawn, bool npc_only) {
	Spawn* ret = nullptr;
	if (MSpawnGroup && HasSpawnGroup()) {
		
		MSpawnGroup->readlock(__FUNCTION__, __LINE__);
		vector<Spawn*>::iterator itr;
		for (itr = spawn_group_list->begin(); itr != spawn_group_list->end(); itr++) {
			if ((*itr) != ignore_spawn && (*itr)->Alive() && (!npc_only || (npc_only && (*itr)->IsNPC()))) {
				ret = (*itr);
				break;
			}
		}
		MSpawnGroup->releasereadlock(__FUNCTION__, __LINE__);
	}
	return ret;
}

void Spawn::UpdateEncounterState(int8 new_state) {
	if (MSpawnGroup && HasSpawnGroup()) {
		MSpawnGroup->readlock(__FUNCTION__, __LINE__);
		vector<Spawn*>::iterator itr;
		for (itr = spawn_group_list->begin(); itr != spawn_group_list->end(); itr++) {
			if ((*itr)->Alive() && (*itr)->IsNPC()) {
				NPC* npc = (NPC*)(*itr);
				(*itr)->SetLockedNoLoot(new_state);
				if(new_state == ENCOUNTER_STATE_BROKEN && npc->Brain()) {
					npc->Brain()->ClearEncounter();
				}
			}
		}
		MSpawnGroup->releasereadlock(__FUNCTION__, __LINE__);
	}
}

void Spawn::CheckEncounterState(Entity* victim, bool test_auto_lock) {
	if (!IsEntity() || !victim->IsNPC())
		return;

	Entity* ent = ((Entity*)this);
	if (victim->GetLockedNoLoot() == ENCOUNTER_STATE_AVAILABLE) {
		if(IsInSpawnGroup(victim))
			return; // can't aggro your own members

		Entity* attacker = nullptr;
		if (ent->GetOwner())
			attacker = ent->GetOwner();
		else
			attacker = ent;

		bool matchedAutoLock = false;
		if (attacker->IsEntity() && ((Entity*)attacker)->GetGroupMemberInfo()) {
			world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);
			GroupMemberInfo* gmi = ((Entity*)attacker)->GetGroupMemberInfo();
			if (gmi && gmi->group_id)
			{
				PlayerGroup* group = world.GetGroupManager()->GetGroup(gmi->group_id);
				if (group && ((group->GetGroupOptions()->group_lock_method && group->GetGroupOptions()->group_autolock == 1) || attacker->GetGroupMemberInfo()->leader))
				{
					matchedAutoLock = true;
					group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
					deque<GroupMemberInfo*>* members = group->GetMembers();

					for (int8 i = 0; i < members->size(); i++) {
						Entity* member = members->at(i)->member;
						if (!member || member->GetZone() != attacker->GetZone())
							continue;

						if (member->IsEntity()) {
							if (!member->GetInfoStruct()->get_engaged_encounter()) {
								member->GetInfoStruct()->set_engaged_encounter(1);
							}
							if (((NPC*)victim)->Brain()) {
								((NPC*)victim)->Brain()->AddHate(member, 0);
								((NPC*)victim)->Brain()->AddToEncounter(member);
								victim->AddTargetToEncounter(member);
							}
						}
					}
					group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
				}
			}
			world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
		}
		else if (attacker->GetInfoStruct()->get_group_solo_autolock()) {
			matchedAutoLock = true;
			if (((NPC*)victim)->Brain()) {
				((NPC*)victim)->Brain()->AddHate(attacker, 0);
				((NPC*)victim)->Brain()->AddToEncounter(attacker);
				victim->AddTargetToEncounter(attacker);
			}
		}

		if (test_auto_lock && !matchedAutoLock) {
			return;
		}

		if (!ent->GetInfoStruct()->get_engaged_encounter()) {
			ent->GetInfoStruct()->set_engaged_encounter(1);
		}

		if (!attacker->GetInfoStruct()->get_engaged_encounter()) {
			attacker->GetInfoStruct()->set_engaged_encounter(1);
		}

		int8 skip_loot_gray_mob_flag = rule_manager.GetGlobalRule(R_Loot, SkipLootGrayMob)->GetInt8();

		int8 difficulty = attacker->GetArrowColor(victim->GetLevel());

		int8 new_enc_state = ENCOUNTER_STATE_AVAILABLE;
		if (skip_loot_gray_mob_flag && difficulty == ARROW_COLOR_GRAY) {
			if (!attacker->IsPlayer() && !attacker->IsBot()) {
				new_enc_state = ENCOUNTER_STATE_BROKEN;
			}
			else {
				new_enc_state = ENCOUNTER_STATE_OVERMATCHED;
			}
		}
		else {
			if (attacker->IsPlayer() || attacker->IsBot()) {
				new_enc_state = ENCOUNTER_STATE_LOCKED;
			}
			else {
				new_enc_state = ENCOUNTER_STATE_BROKEN;
			}
		}

		victim->SetLockedNoLoot(new_enc_state);
		victim->UpdateEncounterState(new_enc_state);
	}
}

void Spawn::AddTargetToEncounter(Entity* entity) {
	if (MSpawnGroup && HasSpawnGroup()) {
		MSpawnGroup->readlock(__FUNCTION__, __LINE__);
		vector<Spawn*>::iterator itr;
		for (itr = spawn_group_list->begin(); itr != spawn_group_list->end(); itr++) {
			if ((*itr) != this && (*itr)->Alive() && (*itr)->IsNPC()) {
				((NPC*)(*itr))->Brain()->AddToEncounter(entity);
			}
		}
		MSpawnGroup->releasereadlock(__FUNCTION__, __LINE__);
	}
}

void Spawn::AddSpawnToGroup(Spawn* spawn){
	if(!spawn)
		return;
	if(!spawn_group_list){
		spawn_group_list = new vector<Spawn*>();
		spawn_group_list->push_back(this);
		safe_delete(MSpawnGroup);
		MSpawnGroup = new Mutex();
		MSpawnGroup->SetName("Spawn::MSpawnGroup");
	}
	vector<Spawn*>::iterator itr;
	MSpawnGroup->writelock(__FUNCTION__, __LINE__);
	for(itr = spawn_group_list->begin(); itr != spawn_group_list->end(); itr++){
		if((*itr) == spawn){
			MSpawnGroup->releasewritelock(__FUNCTION__, __LINE__);
			return;
		}
	}
	spawn_group_list->push_back(spawn);
	spawn->SetSpawnGroupList(spawn_group_list, MSpawnGroup);
	MSpawnGroup->releasewritelock(__FUNCTION__, __LINE__);
}


void Spawn::SetSpawnGroupList(vector<Spawn*>* list, Mutex* mutex){
	spawn_group_list = list;
	MSpawnGroup = mutex;
}

void Spawn::RemoveSpawnFromGroup(bool erase_all){
	SetSpawnGroupID(0);
	bool del = false;
	if(MSpawnGroup){
		MSpawnGroup->writelock(__FUNCTION__, __LINE__);
		if(spawn_group_list){
			vector<Spawn*>::iterator itr;
			Spawn* spawn = 0;
			if(spawn_group_list->size() == 1)
				erase_all = true;
			for(itr = spawn_group_list->begin(); itr != spawn_group_list->end(); itr++){
				spawn = *itr;
				if (spawn) {
					if(!erase_all){
						if(spawn == this){
							spawn_group_list->erase(itr);
							MSpawnGroup->releasewritelock(__FUNCTION__, __LINE__);
							spawn_group_list = 0;						
							MSpawnGroup = 0;
							return;
						}
					}
					else{
						if (spawn != this)
							spawn->SetSpawnGroupList(0, 0);
					}
				}
			}
			if (erase_all)
				spawn_group_list->clear();
			del = (spawn_group_list->size() == 0);
		}
		MSpawnGroup->releasewritelock(__FUNCTION__, __LINE__);
		if (del){
			safe_delete(MSpawnGroup);
			safe_delete(spawn_group_list);
		}
	}
}

void Spawn::SetSpawnGroupID(int32 id){
	m_SpawnMutex.writelock();
	group_id = id;
	m_SpawnMutex.releasewritelock();
}

int32 Spawn::GetSpawnGroupID(){
	int32 groupid = 0;
	m_SpawnMutex.readlock();
	groupid = group_id;
	m_SpawnMutex.releasereadlock();
	return groupid;
}

void Spawn::AddChangedZoneSpawn(){
	if(send_spawn_changes && GetZone())
		GetZone()->AddChangedSpawn(this);
}

void Spawn::RemoveSpawnAccess(Spawn* spawn) {
	if (allowed_access.count(spawn->GetID()) > 0) {
		allowed_access.erase(spawn->GetID());
		GetZone()->HidePrivateSpawn(this);
	}
}

void Spawn::SetFollowTarget(Spawn* spawn, int32 follow_distance) {
	if (spawn && spawn != this) {
		m_followTarget = spawn->GetID();
		m_followDistance = follow_distance;
	}
	else {
		m_followTarget = 0;
		if (following)
			following = false;
		m_followDistance = 0;
	}
}

void Spawn::AddTempVariable(string var, string val) {
	m_tempVariableTypes[var] = 5;
	m_tempVariables[var] = val;
}

void Spawn::AddTempVariable(string var, Spawn* val) {
	m_tempVariableTypes[var] = 1;
	m_tempVariableSpawn[var] = val->GetID();
}

void Spawn::AddTempVariable(string var, ZoneServer* val) {
	m_tempVariableTypes[var] = 2;
	m_tempVariableZone[var] = val;
}

void Spawn::AddTempVariable(string var, Item* val) {
	m_tempVariableTypes[var] = 3;
	m_tempVariableItem[var] = val;
}

void Spawn::AddTempVariable(string var, Quest* val) {
	m_tempVariableTypes[var] = 4;
	m_tempVariableQuest[var] = val;
}

string Spawn::GetTempVariable(string var) {
	string ret = "";

	if (m_tempVariables.count(var) > 0)
		ret = m_tempVariables[var];

	return ret;
}

Spawn* Spawn::GetTempVariableSpawn(string var) {
	Spawn* ret = 0;
	
	if (m_tempVariableSpawn.count(var) > 0)
		ret = GetZone()->GetSpawnByID(m_tempVariableSpawn[var]);

	return ret;
}

ZoneServer* Spawn::GetTempVariableZone(string var) {
	ZoneServer* ret = 0;

	if (m_tempVariableZone.count(var) > 0)
		ret = m_tempVariableZone[var];

	return ret;
}

Item* Spawn::GetTempVariableItem(string var) {
	Item* ret = 0;

	if (m_tempVariableItem.count(var) > 0)
		ret = m_tempVariableItem[var];

	return ret;
}

Quest* Spawn::GetTempVariableQuest(string var) {
	Quest* ret = 0;

	if (m_tempVariableQuest.count(var) > 0)
		ret = m_tempVariableQuest[var];

	return ret;
}

int8 Spawn::GetTempVariableType(string var) {
	int8 ret = 0;

	if (m_tempVariableTypes.count(var) > 0)
		ret = m_tempVariableTypes[var];

	return ret;
}

void Spawn::DeleteTempVariable(string var) {
	int8 type = GetTempVariableType(var);

	switch (type) {
	case 1:
		m_tempVariableSpawn.erase(var);
		break;
	case 2:
		m_tempVariableZone.erase(var);
		break;
	case 3:
		m_tempVariableItem.erase(var);
		break;
	case 4:
		m_tempVariableQuest.erase(var);
		break;
	case 5:
		m_tempVariables.erase(var);
		break;
	}

	m_tempVariableTypes.erase(var);
}

Spawn* Spawn::GetRunningTo() {
	return GetZone()->GetSpawnByID(running_to);
}

Spawn* Spawn::GetFollowTarget() {
	return GetZone()->GetSpawnByID(m_followTarget);
}

void Spawn::CopySpawnAppearance(Spawn* spawn){
	if (!spawn)
		return;

	//This function copies the appearace of the provided spawn to this one
	if (spawn->IsEntity() && IsEntity()){
		memcpy(&((Entity*)this)->features, &((Entity*)spawn)->features, sizeof(CharFeatures));
		memcpy(&((Entity*)this)->equipment, &((Entity*)spawn)->equipment, sizeof(EQ2_Equipment));
	}

	SetSize(spawn->GetSize());
	SetModelType(spawn->GetModelType());
}

void Spawn::SetY(float y, bool updateFlags, bool disableYMapCheck)
{
	SetPos(&appearance.pos.Y, y, updateFlags);
	if (!disableYMapCheck)
		FixZ();
}

float Spawn::FindDestGroundZ(glm::vec3 dest, float z_offset)
{
	float best_z = BEST_Z_INVALID;
	if (GetZone() != nullptr && GetMap() != nullptr)
	{
		dest.z += z_offset;
		best_z = FindBestZ(dest, nullptr);
	}
	return best_z;
}

float Spawn::FindBestZ(glm::vec3 loc, glm::vec3* result, int32* new_grid_id, int32* new_widget_id) {
	std::shared_lock lock(MIgnoredWidgets);
	
	if(!GetMap())
		return BEST_Z_INVALID;
	
	float new_z = GetMap()->FindBestZ(loc, nullptr, &ignored_widgets, new_grid_id, new_widget_id);
	return new_z;
}

float Spawn::GetFixedZ(const glm::vec3& destination, int32 z_find_offset) {
	BenchTimer timer;
	timer.reset();

	float new_z = destination.z;

	if (GetZone() != nullptr && GetMap() != nullptr) {

/*		if (flymode == GravityBehavior::Flying)
			return new_z;
			*/
/*		if (zone->HasWaterMap() && zone->watermap->InLiquid(glm::vec3(m_Position)))
			return new_z;
			*/
		/*
		 * Any more than 5 in the offset makes NPC's hop/snap to ceiling in small corridors
		 */
		new_z = this->FindDestGroundZ(destination, z_find_offset);
		if (new_z != BEST_Z_INVALID) {
			if (new_z < -2000) {
				new_z = GetY();
			}
		}

		auto duration = timer.elapsed();
		LogWrite(MAP__DEBUG, 0, "Map", "Mob::GetFixedZ() ([{%s}]) returned [{%f}] at [{%f}], [{%f}], [{%f}] - Took [{%f}]",
			this->GetName(),
			new_z,
			destination.x,
			destination.y,
			destination.z,
			duration);
	}

	return new_z;
}


void Spawn::FixZ(bool forceUpdate) {
	if (!GetZone()) {
		return;
	}
	/*
	if (flymode == GravityBehavior::Flying) {
		return;
	}*/
	/*
	if (zone->watermap && zone->watermap->InLiquid(m_Position)) {
		return;
	}*/
	
	// we do the inwater check here manually to avoid double calling for a Z coordinate
	glm::vec3 current_loc(GetX(), GetZ(), GetY());

	uint32 GridID = 0;
	uint32 WidgetID = 0;
	float new_z = GetY();
	if(GetMap() != nullptr) {
		new_z = FindBestZ(current_loc, nullptr, &GridID, &WidgetID);

		if ((IsTransportSpawn() || !IsFlyingCreature()) && GridID != 0 && GridID != GetLocation()) {
			LogWrite(PLAYER__DEBUG, 0, "Player", "%s left grid %u and entered grid %u", appearance.name, GetLocation(), GridID);
			
			const char* zone_script = world.GetZoneScript(GetZone()->GetZoneID());

			if (zone_script && lua_interface) {
				lua_interface->RunZoneScript(zone_script, "leave_location", GetZone(), this, GetLocation());
			}
			
			SetLocation(GridID);
				
			if (zone_script && lua_interface) {
				lua_interface->RunZoneScript(zone_script, "enter_location", GetZone(), this, GridID);
			}
		}
		trigger_widget_id = WidgetID;
	}

	// no need to go any further for players, flying creatures or objects, just needed the grid id set
	if (IsPlayer() || IsFlyingCreature() || IsObject()) {
		return;
	}
	
	if ( region_map != nullptr )
	{
		glm::vec3 targPos(GetX(), GetY(), GetZ());
		
		if(region_map->InWater(targPos, GetLocation()))
			return;
	}
	
	if (new_z == GetY())
		return;

	if ((new_z > -2000) && new_z != BEST_Z_INVALID) {
		SetY(new_z, forceUpdate, true);
	}
	else {
		LogWrite(MAP__DEBUG, 0, "Map", "[{%s}] is failing to find Z [{%f}]", this->GetName(), std::abs(GetY() - new_z));
	}
}

bool Spawn::CheckLoS(Spawn* target)
{
	float radiusSrc = 2.0f;
	float radiusTarg = 2.0f;

	glm::vec3 targpos(target->GetX(), target->GetZ(), target->GetY()+radiusTarg);
	glm::vec3 pos(GetX(), GetZ(), GetY()+radiusSrc);
	return CheckLoS(pos, targpos);
}

bool Spawn::CheckLoS(glm::vec3 myloc, glm::vec3 oloc)
{
	bool res = false;
	ZoneServer* zone = GetZone();
	if (zone == NULL || GetMap() == NULL || !GetMap()->IsMapLoaded())
		return true;
	else {
		MIgnoredWidgets.lock_shared();
		res = GetMap()->CheckLoS(myloc, oloc, &ignored_widgets);
		MIgnoredWidgets.unlock_shared();
	}

	return res;
}

void Spawn::CalculateNewFearpoint()
{
	if (GetZone() && GetZone()->pathing) {
		auto Node = zone->pathing->GetRandomLocation(glm::vec3(GetX(), GetZ(), GetY()));
		if (Node.x != 0.0f || Node.y != 0.0f || Node.z != 0.0f) {
			AddRunningLocation(Node.x, Node.y, Node.z, GetSpeed(), 0, true, true, "", true);
		}
	}
}

Item* Spawn::LootItem(int32 id) {
	Item* ret = 0;
	vector<Item*>::iterator itr;
	MLootItems.lock();
	for (itr = loot_items.begin(); itr != loot_items.end(); itr++) {
		if ((*itr)->details.item_id == id) {
			ret = *itr;
			loot_items.erase(itr);
			break;
		}
	}
	MLootItems.unlock();
	return ret;
}

void Spawn::TransferLoot(Spawn* spawn) {
	if(spawn == this || spawn == nullptr)
		return; // mmm no

	vector<Item*>::iterator itr;
	MLootItems.lock();
	for (itr = loot_items.begin(); itr != loot_items.end();) {
		if (!(*itr)->IsBodyDrop()) {
			spawn->AddLootItem(*itr);
			itr = loot_items.erase(itr);
		}
		else {
			itr++;
		}
	}
	MLootItems.unlock();
}

int32 Spawn::GetLootItemID() {
	int32 ret = 0;
	vector<Item*>::iterator itr;
	MLootItems.lock();
	for (itr = loot_items.begin(); itr != loot_items.end(); itr++) {
		ret = (*itr)->details.item_id;
		break;
	}
	MLootItems.unlock();
	return ret;
}

void Spawn::GetLootItemsList(std::vector<int32>* out_entries) {
	if(!out_entries)
		return;
	
	vector<Item*>::iterator itr;
	for (itr = loot_items.begin(); itr != loot_items.end(); itr++) {
		out_entries->push_back((*itr)->details.item_id);
	}
}

bool Spawn::HasLootItemID(int32 id) {
	bool ret = false;

	vector<Item*>::iterator itr;
	MLootItems.readlock(__FUNCTION__, __LINE__);
	for (itr = loot_items.begin(); itr != loot_items.end(); itr++) {
		if ((*itr)->details.item_id == id) {
			ret = true;
			break;
		}
	}
	MLootItems.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

void Spawn::CheckProximities()
{
	if (!has_spawn_proximities)
		return;

	if (spawn_proximities.size() > 0)
	{
		MutexList<SpawnProximity*>::iterator itr = spawn_proximities.begin();
		while (itr.Next()) {
			SpawnProximity* prox = itr.value;
			map<int32, bool>::iterator spawnsItr;
			for (spawnsItr = prox->spawns_in_proximity.begin(); spawnsItr != prox->spawns_in_proximity.end(); spawnsItr++) {
				Spawn* tmpSpawn = 0;
				if (spawnsItr->first &&
					((prox->spawn_type == SPAWNPROXIMITY_DATABASE_ID && (tmpSpawn = GetZone()->GetSpawnByDatabaseID(spawnsItr->first)) != 0) ||
						(prox->spawn_type == SPAWNPROXIMITY_LOCATION_ID && (tmpSpawn = GetZone()->GetSpawnByLocationID(spawnsItr->first)) != 0)))
				{
					if (!spawnsItr->second && tmpSpawn->GetDistance(this) <= prox->distance)
					{
						if (prox->in_range_lua_function.size() > 0)
							GetZone()->CallSpawnScript(this, SPAWN_SCRIPT_CUSTOM, tmpSpawn, prox->in_range_lua_function.c_str());
						spawnsItr->second = true;
					}
					else if (spawnsItr->second && tmpSpawn->GetDistance(this) > prox->distance)
					{
						if (prox->leaving_range_lua_function.size() > 0)
							GetZone()->CallSpawnScript(this, SPAWN_SCRIPT_CUSTOM, tmpSpawn, prox->leaving_range_lua_function.c_str());
						spawnsItr->second = false;
					}
				}
			}
		}
	}
}

void Spawn::AddSpawnToProximity(int32 spawnValue, SpawnProximityType type)
{
	if (!has_spawn_proximities)
		return;

	if (spawn_proximities.size() > 0)
	{
		MutexList<SpawnProximity*>::iterator itr = spawn_proximities.begin();
		while (itr.Next()) {
			SpawnProximity* prox = itr->value;
			if (prox->spawn_value == spawnValue && prox->spawn_type == type)
				prox->spawns_in_proximity.insert(make_pair(spawnValue, false));
		}
	}
}

void Spawn::RemoveSpawnFromProximity(int32 spawnValue, SpawnProximityType type)
{
	if (!has_spawn_proximities)
		return;

	if (spawn_proximities.size() > 0)
	{
		MutexList<SpawnProximity*>::iterator itr = spawn_proximities.begin();
		while (itr.Next()) {
			SpawnProximity* prox = itr->value;
			if (prox->spawn_value == spawnValue && prox->spawn_type == type &&
				prox->spawns_in_proximity.count(spawnValue) > 0)
				prox->spawns_in_proximity.erase(spawnValue);
		}
	}
}

void Spawn::AddPrimaryEntityCommand(const char* name, float distance, const char* command, const char* error_text, int16 cast_time, int32 spell_visual, bool defaultDenyList, Player* player) {

	EntityCommand* cmd = FindEntityCommand(string(command), true);

	bool newCommand = false;
	if (!cmd)
	{
		newCommand = true;
		cmd = CreateEntityCommand(name, distance, command, error_text, cast_time, spell_visual, !defaultDenyList);
	}

	if (defaultDenyList)
		SetPermissionToEntityCommand(cmd, player, true);

	if (newCommand)
		primary_command_list.push_back(cmd);
}

void Spawn::RemovePrimaryEntityCommand(const char* command) {
	vector<EntityCommand*>::iterator itr;
	string tmpStr(command);
	for (itr = primary_command_list.begin(); itr != primary_command_list.end(); itr++) {
		EntityCommand* cmd = *itr;
		if (cmd->command.compare(tmpStr) == 0)
		{
			primary_command_list.erase(itr);
			delete cmd;
			break;
		}
	}
}

bool Spawn::SetPermissionToEntityCommand(EntityCommand* command, Player* player, bool permissionValue)
{
	if(!player)
		return false;
	
	return SetPermissionToEntityCommandByCharID(command, player->GetCharacterID(), permissionValue);
}

bool Spawn::SetPermissionToEntityCommandByCharID(EntityCommand* command, int32 charID, bool permissionValue)
{
	map<int32, bool>::iterator itr = command->allow_or_deny.find(charID);
	if (itr == command->allow_or_deny.end())
		command->allow_or_deny.insert(make_pair(charID, permissionValue));
	else if (itr->second != permissionValue)
		itr->second = permissionValue;
	
	return true;
}

void Spawn::RemoveSpawnFromPlayer(Player* player)
{
	m_Update.writelock(__FUNCTION__, __LINE__);
	player->RemoveSpawn(this); // sets it as removed
	m_Update.releasewritelock(__FUNCTION__, __LINE__);
}

bool Spawn::InWater()
{
	bool inWater = false;

		if ( region_map != nullptr )
		{
			glm::vec3 targPos(GetX(), GetY(), GetZ());
			if ( IsGroundSpawn() )
				targPos.y -= .5f;
					
			if(region_map->InWater(targPos, GetLocation()))
				inWater = true;
		}

	return inWater;
}

bool Spawn::InLava()
{
	bool inLava = false;

		if ( region_map != nullptr )
		{
			glm::vec3 targPos(GetX(), GetY(), GetZ());
			if ( IsGroundSpawn() )
				targPos.y -= .5f;
					
			if(region_map->InLava(targPos, GetLocation()))
				inLava = true;
		}

	return inLava;
}

void Spawn::DeleteRegion(Region_Node* inNode, ZBSP_Node* rootNode)
{
	map<map<Region_Node*, ZBSP_Node*>, Region_Status>::iterator testitr;
	for (testitr = Regions.begin(); testitr != Regions.end(); testitr++)
	{
		map<Region_Node*, ZBSP_Node*>::const_iterator actualItr = testitr->first.begin();
		Region_Node* node = actualItr->first;
		ZBSP_Node* BSP_Root = actualItr->second;
		if(inNode == node && rootNode == BSP_Root )
		{
			testitr = Regions.erase(testitr);
			break;
		}
	}
}

bool Spawn::InRegion(Region_Node* inNode, ZBSP_Node* rootNode)
{
	map<map<Region_Node*, ZBSP_Node*>, Region_Status>::iterator testitr;
	for (testitr = Regions.begin(); testitr != Regions.end(); testitr++)
	{
		map<Region_Node*, ZBSP_Node*>::const_iterator actualItr = testitr->first.begin();
		Region_Node* node = actualItr->first;
		ZBSP_Node* BSP_Root = actualItr->second;
		if(inNode == node && rootNode == BSP_Root )
		{
			return testitr->second.inRegion;
		}
	}
		
	return false;
}

int32 Spawn::GetRegionType(Region_Node* inNode, ZBSP_Node* rootNode)
{
	map<map<Region_Node*, ZBSP_Node*>, Region_Status>::iterator testitr;
	for (testitr = Regions.begin(); testitr != Regions.end(); testitr++)
	{
		map<Region_Node*, ZBSP_Node*>::const_iterator actualItr = testitr->first.begin();
		Region_Node* node = actualItr->first;
		ZBSP_Node* BSP_Root = actualItr->second;
		if(inNode == node && rootNode == BSP_Root )
		{
			return testitr->second.regionType;
		}
	}
	
	return false;
}

float Spawn::SpawnAngle(Spawn* target, float selfx, float selfz)
{
	if (!target || target == this)
		return 0.0f;

	float angle, lengthb, vectorx, vectorz, dotp;
	float spx = (target->GetX());	// mob xloc (inverse because eq)
	float spz = -(target->GetZ());		// mob yloc
	float heading = target->GetHeading();	// mob heading
	if (heading < 270)
		heading += 90;
	else
		heading -= 270;

	heading = heading * 3.1415f / 180.0f;	// convert to radians
	vectorx = spx + (10.0f * std::cos(heading));	// create a vector based on heading
	vectorz = spz + (10.0f * std::sin(heading));	// of spawn length 10

	// length of spawn to player vector
	lengthb = (float) std::sqrt(((selfx - spx) * (selfx - spx)) + ((-selfz - spz) * (-selfz - spz)));

	// calculate dot product to get angle
	// Handle acos domain errors due to floating point rounding errors
	dotp = ((vectorx - spx) * (selfx - spx) +
			(vectorz - spz) * (-selfz - spz)) / (10.0f * lengthb);

	if (dotp > 1)
		return 0.0f;
	else if (dotp < -1)
		return 180.0f;

	angle = std::acos(dotp);
	angle = angle * 180.0f / 3.1415f;

	return angle;
}

void Spawn::StopMovement()
{
	reset_movement = true;
}

bool Spawn::PauseMovement(int32 period_of_time_ms)
{
	if(period_of_time_ms < 1)
		period_of_time_ms = 1;
	
	RunToLocation(GetX(),GetY(),GetZ());
	pause_timer.Start(period_of_time_ms, true);

	return true;
}

bool Spawn::IsPauseMovementTimerActive()
{
	if(pause_timer.Check())
		pause_timer.Disable();
	
	return pause_timer.Enabled();
}

bool Spawn::IsFlyingCreature()
{
	if(!IsEntity())
		return false;

	return ((Entity*)this)->GetInfoStruct()->get_flying_type();
}

bool Spawn::IsWaterCreature()
{
	if(!IsEntity())
		return false;

	return ((Entity*)this)->GetInfoStruct()->get_water_type();
}


void Spawn::SetFlyingCreature() {
	if(!IsEntity() || !rule_manager.GetGlobalRule(R_Spawn, UseHardCodeFlyingModelType)->GetInt8())
		return;

	if(((Entity*)this)->GetInfoStruct()->get_flying_type() > 0) // DB spawn npc flag already set
		return;

	switch (GetModelType())
	{
	case 260:
	case 295:
		((Entity*)this)->GetInfoStruct()->set_flying_type(1);
		is_flying_creature = true;
		break;
	default:
		((Entity*)this)->GetInfoStruct()->set_flying_type(0);
		break;
	}
}
	
void Spawn::SetWaterCreature() {
	if(!IsEntity() || !rule_manager.GetGlobalRule(R_Spawn, UseHardCodeWaterModelType)->GetInt8())
		return;

	if(((Entity*)this)->GetInfoStruct()->get_water_type() > 0) // DB spawn npc flag already set
		return;

	switch (GetModelType())
	{
	case 194:
	case 204:
	case 210:
	case 241:
	case 242:
	case 254:
	case 10668:
	case 20828:
		((Entity*)this)->GetInfoStruct()->set_water_type(1);
		break;
	default:
		((Entity*)this)->GetInfoStruct()->set_water_type(0);
		break;
	}
}

void Spawn::AddRailPassenger(int32 char_id)
{
	std::lock_guard<std::mutex> lk(m_RailMutex);
	rail_passengers.insert(make_pair(char_id,true));
}

void Spawn::RemoveRailPassenger(int32 char_id)
{
	std::lock_guard<std::mutex> lk(m_RailMutex);
	std::map<int32, bool>::iterator itr = rail_passengers.find(char_id);
	if(itr != rail_passengers.end())
		rail_passengers.erase(itr);
}

vector<Spawn*> Spawn::GetPassengersOnRail() {
	vector<Spawn*> tmp_list;
	Spawn* spawn;
	m_RailMutex.lock();
	std::map<int32, bool>::iterator itr = rail_passengers.begin();
	while(itr != rail_passengers.end()){
		Client* client = zone_list.GetClientByCharID(itr->first);
		if(!client || !client->GetPlayer())
			continue;

		tmp_list.push_back(client->GetPlayer());
		itr++;
	}
	m_RailMutex.unlock();
	return tmp_list;
}

void Spawn::SetAppearancePosition(float x, float y, float z) {
	appearance.pos.X = x;
	appearance.pos.Y = y;
	appearance.pos.Z = z;
	appearance.pos.X2 = appearance.pos.X;
	appearance.pos.Y2 = appearance.pos.Y;
	appearance.pos.Z2 = appearance.pos.Z;
	appearance.pos.X3 = appearance.pos.X;
	appearance.pos.Y3 = appearance.pos.Y;
	appearance.pos.Z3 = appearance.pos.Z;
		
	SetSpeedX(0);
	SetSpeedY(0);
	SetSpeedZ(0);
	if(IsPlayer()) {
		((Player*)this)->SetSideSpeed(0);
		((Player*)this)->pos_packet_speed = 0;
	}
}


int32 Spawn::InsertRegionToSpawn(Region_Node* node, ZBSP_Node* bsp_root, WaterRegionType regionType, bool in_region) {
	std::map<Region_Node*, ZBSP_Node*> newMap;
	newMap.insert(make_pair(node, bsp_root));
	Region_Status status;
	status.inRegion = in_region;
	status.regionType = regionType;
	int32 returnValue = 0;
	if(in_region) {
		lua_interface->RunRegionScript(node->regionScriptName, "EnterRegion", GetZone(), this, regionType, &returnValue);
	}
	status.timerTic = returnValue;
	status.lastTimerTic = returnValue ? Timer::GetCurrentTime2() : 0;
	Regions.insert(make_pair(newMap, status));	
	return returnValue;
}


bool Spawn::HasRegionTracked(Region_Node* node, ZBSP_Node* bsp_root, bool in_region) {
	map<map<Region_Node*, ZBSP_Node*>, Region_Status>::iterator testitr;
	for (testitr = Regions.begin(); testitr != Regions.end(); testitr++)
	{
		map<Region_Node*, ZBSP_Node*>::const_iterator actualItr = testitr->first.begin();
		Region_Node *node = actualItr->first;
		ZBSP_Node *BSP_Root = actualItr->second;
		if(node == actualItr->first && BSP_Root == actualItr->second) {
			if(testitr->second.inRegion == in_region)
				return true;
			else
				break;
		}
	}
	
	return false;
}


void Spawn::SetLocation(int32 id, bool setUpdateFlags)
{
	if(GetZone()) {
		GetZone()->RemoveSpawnFromGrid(this, GetLocation());
		SetPos(&appearance.pos.grid_id, id, setUpdateFlags);
		GetZone()->AddSpawnToGrid(this, id);
	}
	else {
		SetPos(&appearance.pos.grid_id, id, setUpdateFlags);
	}
}

int8 Spawn::GetArrowColor(int8 spawn_level){
	int8 color = 0;
	sint16 diff = spawn_level - GetLevel();
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

void Spawn::AddIgnoredWidget(int32 id) {
	std::unique_lock lock(MIgnoredWidgets);
	if(ignored_widgets.find(id) == ignored_widgets.end()) {
		ignored_widgets.insert(make_pair(id,true));
	}
}

void Spawn::SendGroupUpdate() {
	if (IsEntity() && ((Entity*)this)->GetGroupMemberInfo()) {
		((Entity*)this)->UpdateGroupMemberInfo();
		if (IsPlayer()) {
			Client* client = ((Player*)this)->GetClient();
			if(client) {
				world.GetGroupManager()->SendGroupUpdate(((Entity*)this)->GetGroupMemberInfo()->group_id, client);
			}
		}
		else
			world.GetGroupManager()->SendGroupUpdate(((Entity*)this)->GetGroupMemberInfo()->group_id);
	}
}

bool Spawn::AddNeedGreedItemRequest(int32 item_id, int32 spawn_id, bool need_item) {
	LogWrite(LOOT__INFO, 0, "Loot", "%s: AddNeedGreedItemRequest Item ID: %u, Spawn ID: %u, Need Item: %u", GetName(), item_id, spawn_id, need_item);
	if (HasSpawnNeedGreedEntry(item_id, spawn_id)) {
		return false;
	}

	need_greed_items.insert(make_pair(item_id, std::make_pair(spawn_id, need_item)));

	AddSpawnLootWindowCompleted(spawn_id, false);
	return true;
}

bool Spawn::AddLottoItemRequest(int32 item_id, int32 spawn_id) {
	LogWrite(LOOT__INFO, 0, "Loot", "%s: AddLottoItemRequest Item ID: %u, Spawn ID: %u", GetName(), item_id, spawn_id);
	if (HasSpawnLottoEntry(item_id, spawn_id)) {
		return false;
	}

	lotto_items.insert(make_pair(item_id, spawn_id));

	AddSpawnLootWindowCompleted(spawn_id, false);
	return true;
}

void Spawn::AddSpawnLootWindowCompleted(int32 spawn_id, bool status_) {
	if (loot_complete.find(spawn_id) == loot_complete.end()) {
		loot_complete.insert(make_pair(spawn_id, status_));
	}

	is_loot_complete = HasLootWindowCompleted();
}

bool Spawn::SetSpawnLootWindowCompleted(int32 spawn_id) {
	std::map<int32, bool>::iterator itr = loot_complete.find(spawn_id);
	if (itr != loot_complete.end()) {
		itr->second = true;
		is_loot_complete = HasLootWindowCompleted();
		return true;
	}
	return false;
}

bool Spawn::HasSpawnLootWindowCompleted(int32 spawn_id) {
	std::map<int32, bool>::iterator itr = loot_complete.find(spawn_id);
	if (itr != loot_complete.end() && itr->second) {
		return true;
	}
	return false;
}

bool Spawn::HasSpawnNeedGreedEntry(int32 item_id, int32 spawn_id) {
	for (auto [itr, rangeEnd] = need_greed_items.equal_range(item_id); itr != rangeEnd; itr++) {
		LogWrite(LOOT__DEBUG, 8, "Loot", "%s: HasSpawnNeedGreedEntry Item ID: %u, Spawn ID: %u", GetName(), itr->first, itr->second.first);
		if (spawn_id == itr->second.first) {
			return true;
		}
	}
	return false;
}

bool Spawn::HasSpawnLottoEntry(int32 item_id, int32 spawn_id) {
	for (auto [itr, rangeEnd] = lotto_items.equal_range(item_id); itr != rangeEnd; itr++) {
		LogWrite(LOOT__DEBUG, 8, "Loot", "%s: HasSpawnLottoEntry Item ID: %u, Spawn ID: %u", GetName(), itr->first, itr->second);
		if (spawn_id == itr->second) {
			return true;
		}
	}
	return false;
}

void Spawn::GetSpawnLottoEntries(int32 item_id, std::map<int32, int32>* out_entries) {
	if (!out_entries)
		return;

	std::map<int32, bool> spawn_matches;
	for (auto [itr, endrange] = lotto_items.equal_range(item_id); itr != endrange; itr++) {
		out_entries->insert(std::make_pair(itr->second, (int32)MakeRandomInt(0, 100)));
		spawn_matches[itr->second] = true;
	}

	// 0xFFFFFFFF represents selecting "All" on the lotto screen
	for (auto [itr, endrange] = lotto_items.equal_range(0xFFFFFFFF); itr != endrange; itr++) {
		if (spawn_matches.find(itr->second) == spawn_matches.end()) {
			out_entries->insert(std::make_pair(itr->second, (int32)MakeRandomInt(0, 100)));
		}
	}
}

void Spawn::GetSpawnNeedGreedEntries(int32 item_id, bool need_item, std::map<int32, int32>* out_entries) {
	if (!out_entries)
		return;

	for (auto [itr, rangeEnd] = need_greed_items.equal_range(item_id); itr != rangeEnd; itr++) {
		out_entries->insert(std::make_pair(itr->second.first, (int32)MakeRandomInt(0, 100)));
	}
}

bool Spawn::HasLootWindowCompleted() {
	std::map<int32, bool>::iterator itr;
	for (itr = loot_complete.begin(); itr != loot_complete.end(); itr++) {
		if (!itr->second)
			return false;
	}

	return true;
}

void Spawn::StartLootTimer(Spawn* looter) {
	if (!IsLootTimerRunning()) {
		int32 loot_timer_time = rule_manager.GetGlobalRule(R_Loot, LootDistributionTime)->GetInt32() * 1000;
		if(rule_manager.GetGlobalRule(R_Loot, AllowChestUnlockByDropTime)->GetBool() && loot_timer_time > rule_manager.GetGlobalRule(R_Loot, ChestUnlockedTimeDrop)->GetInt32()*1000) {
			loot_timer_time = (rule_manager.GetGlobalRule(R_Loot, ChestUnlockedTimeDrop)->GetInt32()*1000) / 2;
		}
		
		if(rule_manager.GetGlobalRule(R_Loot, AllowChestUnlockByTrapTime)->GetBool() && loot_timer_time > rule_manager.GetGlobalRule(R_Loot, ChestUnlockedTimeTrap)->GetInt32()*1000) {
			loot_timer_time = (rule_manager.GetGlobalRule(R_Loot, ChestUnlockedTimeTrap)->GetInt32()*1000) / 2;
		}
		
		if(loot_timer_time < 1000) {
			loot_timer_time = 60000; // hardcode assure they aren't setting some really ridiculous low number
		}
		
		loot_timer.Start(loot_timer_time, true);
	}
	if (looter) {
		looter_spawn_id = looter->GetID();
	}
}

void Spawn::CloseLoot(Spawn* sender) {
	if (sender) {
		SetSpawnLootWindowCompleted(sender->GetID());
	}
	if (sender && looter_spawn_id > 0 && sender->GetID() != looter_spawn_id) {
		LogWrite(LOOT__ERROR, 0, "Loot", "%s: CloseLoot Looter Spawn ID: %u does not match sender %u.", GetName(), looter_spawn_id, sender->GetID());
		return;
	}
	if (!IsLootTimerRunning() && GetLootMethod() != GroupLootMethod::METHOD_LOTTO && GetLootMethod() != GroupLootMethod::METHOD_NEED_BEFORE_GREED) {
		loot_timer.Disable();
	}
	looter_spawn_id = 0;
}

void Spawn::SetLootMethod(GroupLootMethod method, int8 item_rarity, int32 group_id) {
	LogWrite(LOOT__INFO, 0, "Loot", "%s: Set Loot Method : %u, group id : %u", GetName(), (int32)method, group_id);
	loot_group_id = group_id;
	loot_method = method;
	loot_rarity = item_rarity;
	if (loot_name.size() < 1) {
		loot_name = std::string(GetName());
	}
}

bool Spawn::IsItemInLootTier(Item* item) {
	if (!item)
		return true;

	bool skipItem = true;
	switch (GetLootRarity()) {
	case LootTier::ITEMS_TREASURED_PLUS: {
		if (item->details.tier >= ITEM_TAG_TREASURED) {
			skipItem = false;
		}
		break;
	}
	case LootTier::ITEMS_LEGENDARY_PLUS: {
		if (item->details.tier >= ITEM_TAG_LEGENDARY) {
			skipItem = false;
		}
		break;
	}
	case LootTier::ITEMS_FABLED_PLUS: {
		if (item->details.tier >= ITEM_TAG_FABLED) {
			skipItem = false;
		}
		break;
	}
	default: {
		skipItem = false;
		break;
	}
	}

	return skipItem;
}

void Spawn::DistributeGroupLoot_RoundRobin(std::vector<int32>* item_list, bool roundRobinTrashLoot) {

	std::vector<int32>::iterator item_itr;

	for (item_itr = item_list->begin(); item_itr != item_list->end(); item_itr++) {
		int32 item_id = *item_itr;
		Item* tmpItem = master_item_list.GetItem(item_id);
		Spawn* looter = nullptr;

		bool skipItem = IsItemInLootTier(tmpItem);

		if ((skipItem && !roundRobinTrashLoot) || (!skipItem && roundRobinTrashLoot))
			continue;

		world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);
		PlayerGroup* group = world.GetGroupManager()->GetGroup(GetLootGroupID());
		if (group) {
			group->MGroupMembers.writelock(__FUNCTION__, __LINE__);
			deque<GroupMemberInfo*>* members = group->GetMembers();

			int8 index = group->GetLastLooterIndex();
			if (index >= members->size()) {
				index = 0;
			}

			GroupMemberInfo* gmi = members->at(index);
			if (gmi) {
				looter = gmi->member;
			}
			bool loopAttempted = false;
			while (looter) {
				if (!looter->IsPlayer()) {
					index++;
					if (index >= members->size()) {
						if (loopAttempted) {
							looter = nullptr;
							break;
						}
						loopAttempted = true;
						index = 0;
					}
					gmi = members->at(index);
					if (gmi) {
						looter = gmi->member;
					}
					continue;
				}
				else {
					break;
				}
			}
			index += 1;
			group->SetNextLooterIndex(index);
			group->MGroupMembers.releasewritelock(__FUNCTION__, __LINE__);
		}
		world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);

		if (looter) {
			if (looter->IsPlayer()) {
				Item* item = LootItem(item_id);
				bool success = false;
				success = ((Player*)looter)->GetClient()->HandleLootItem(this, item, ((Player*)looter), roundRobinTrashLoot);

				if (!success)
					AddLootItem(item);
			}
			else {
				Item* item = LootItem(item_id);
				safe_delete(item);
			}
		}
	}
}

const double g = 9.81; // acceleration due to gravity (m/s^2)

void Spawn::CalculateInitialVelocity(float heading, float distanceHorizontal, float distanceVertical, float distanceDepth, float duration) {
    float vx = distanceHorizontal / duration;
    float vy = (distanceVertical + 0.5 * g * duration * duration) / duration;
    float vz = distanceDepth / duration;

    // Convert heading angle to radians
    knocked_velocity.x = vx * cos(heading);
    knocked_velocity.y = vy;
    knocked_velocity.z = vz * sin(heading);
}

// Function to calculate the projectile position at a given time
glm::vec3 Spawn::CalculateProjectilePosition(glm::vec3 initialVelocity, float time) {
    glm::vec3 position;
	
    position.x = knocked_back_start_x + initialVelocity.x * time;
    position.y = knocked_back_start_y + initialVelocity.y * time - 0.5 * g * time * time;
    position.z = knocked_back_start_z + initialVelocity.z * time;
	
	auto loc = glm::vec3(position.x, position.z, position.y);
	float new_z = FindBestZ(loc, nullptr);
	if(new_z > position.y)
		position.y = new_z;
	
    return position;
}

bool Spawn::CalculateSpawnProjectilePosition(float x, float y, float z) {
	float currentTimeOffset = (Timer::GetCurrentTime2() - last_movement_update) * 0.001; // * 0.001 is the same as / 1000, float muliplications is suppose to be faster though
	float stepAheadOne = currentTimeOffset+currentTimeOffset;
	
	knocked_back_time_step += currentTimeOffset;
	if(Timer::GetCurrentTime2() >= knocked_back_end_time) {
		ResetKnockedBack();
		FixZ(true);
		return false;
	}
	glm::vec3 position = CalculateProjectilePosition(knocked_velocity, knocked_back_time_step);
	glm::vec3 position_two = position;
	
	if(Timer::GetCurrentTime2() <= knocked_back_end_time+stepAheadOne) {
		position_two = CalculateProjectilePosition(knocked_velocity, knocked_back_time_step+stepAheadOne);
	}
	
	if(GetMap()) {
		glm::vec3 loc(GetX(), GetZ(), GetY() + .5f);
		glm::vec3 dest_loc(position_two.x, position_two.z, position_two.y);
		MIgnoredWidgets.lock_shared();
		glm::vec3 outNorm;
		float dist = 0.0f;
		bool collide_ = GetMap()->DoCollisionCheck(loc, dest_loc, &ignored_widgets, outNorm, dist);
		if(collide_) {
			LogWrite(SPAWN__ERROR, 0, "Spawn", "Collision Hit: cur loc x,y,z: %f %f %f.  to loc %f %f %f.  TimeOffset: %f Total Time: %f Duration: %f", GetX(),GetY(),GetZ(),position_two.x,position_two.y,position_two.z, currentTimeOffset, knocked_back_time_step, knocked_back_duration);
			MIgnoredWidgets.unlock_shared();
			ResetKnockedBack();
			FixZ(true);
			return false;
		}
		MIgnoredWidgets.unlock_shared();
	}
	
	LogWrite(SPAWN__ERROR, 0, "Spawn", "x,y,z: %f %f %f.  Final %f %f %f.  TimeOffset: %f Total Time: %f Duration: %f", GetX(),GetY(),GetZ(),position.x,position.y,position.z, currentTimeOffset, knocked_back_time_step, knocked_back_duration);
	
	SetX(position.x, false);
	SetZ(position.z, false);
	SetY(position.y, false, true);
	
	SetPos(&appearance.pos.X2, position_two.x, false);
	SetPos(&appearance.pos.Z2, position_two.z, false);
	SetPos(&appearance.pos.Y2, position_two.y, false);
	SetPos(&appearance.pos.X3, position_two.x, false);
	SetPos(&appearance.pos.Z3, position_two.z, false);
	SetPos(&appearance.pos.Y3, position_two.y, false);
	
	position_changed = true;
	changed = true;
	GetZone()->AddChangedSpawn(this);
	return true;
}

void Spawn::SetKnockback(Spawn* target, int32 duration, float vertical, float horizontal) {
	if(knocked_back) {
		return; // already being knocked back
	}
	
	    // Calculate the direction vector from source to destination
    glm::vec3 direction = {GetX() - target->GetX(), GetZ() - target->GetZ(), GetY() - target->GetY()};

    // Calculate the heading angle in radians
    double headingRad = atan2(direction.y, sqrt(direction.x * direction.x + direction.z * direction.z));

	knocked_angle = headingRad;
	knocked_back_start_x = GetX();
	knocked_back_start_y = GetY();
	knocked_back_start_z = GetZ();
	knocked_back_h_distance = horizontal / 10.0f;
	knocked_back_v_distance = vertical / 10.0f;
	knocked_back_duration = static_cast<float>(duration) / 1000.0f;
	knocked_back_end_time = Timer::GetCurrentTime2() + duration;
	CalculateInitialVelocity(knocked_angle, knocked_back_h_distance, knocked_back_h_distance, knocked_back_h_distance, knocked_back_duration);
	knocked_back = true;
}

void Spawn::ResetKnockedBack() {
	knocked_back = false;
	knocked_back_time_step = 0.0f;
	knocked_back_h_distance = 0.0f;
	knocked_back_v_distance = 0.0f;
	knocked_back_duration = 0.0f;
	knocked_back_end_time = 0;
	knocked_back_start_x = 0.0f;
	knocked_back_start_y = 0.0f;
	knocked_back_start_z = 0.0f;
	knocked_angle = 0.0f;
	knocked_velocity.x = 0.0f;
	knocked_velocity.y = 0.0f;
	knocked_velocity.z = 0.0f;
}