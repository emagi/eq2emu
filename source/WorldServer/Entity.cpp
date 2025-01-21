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
#include "Entity.h"
#include <math.h>
#include "Items/Items.h"
#include "zoneserver.h"
#include "World.h"
#include "../common/Log.h"
#include "Spells.h"
#include "SpellProcess.h"
#include "classes.h"
#include "LuaInterface.h"
#include "ClientPacketFunctions.h"
#include "Skills.h"
#include "Rules/Rules.h"
#include "LuaInterface.h"

extern World world;
extern MasterItemList master_item_list;
extern MasterSpellList master_spell_list;
extern MasterSkillList master_skill_list;
extern RuleManager rule_manager;
extern Classes classes;
extern LuaInterface* lua_interface;

Entity::Entity(){
	MapInfoStruct();	
	max_speed = 6;
	base_speed = 0.0f;
	last_x = -1;
	last_y = -1;
	last_z = -1;
	last_heading = -1;
	regen_hp_rate = 0;
	regen_power_rate = 0;
	in_combat = false;
	casting = false;
	//memset(&info_struct, 0, sizeof(InfoStruct));
	memset(&features, 0, sizeof(CharFeatures));
	memset(&equipment, 0, sizeof(EQ2_Equipment));
	pet = 0;
	charmedPet = 0;
	deityPet = 0;
	cosmeticPet = 0;
	speed = 0;
	speed_multiplier = 1.0f;
	m_threatTransfer = 0;
	group_member_info = 0;
	trade = 0;
	deity = 0;
	MProcList.SetName("Entity::m_procList");
	MDetriments.SetName("Entity::MDetriments");
	MMaintainedSpells.SetName("Entity::MMaintainedSpells");
	MSpellEffects.SetName("Entity::MSpellEffects");
	m_procList.clear();
	control_effects.clear();
	for (int i = 0; i < CONTROL_MAX_EFFECTS; i++)
		control_effects[i] = NULL;

	immunities.clear();
	
	info_struct.ResetEffects(this);

	MCommandMutex.SetName("Entity::MCommandMutex");
	hasSeeInvisSpell = false;
	hasSeeHideSpell = false;

	owner = 0;
	m_petType = 0;
	m_petSpellID = 0;
	m_petSpellTier = 0;
	m_petDismissing = false;
	
	if (!IsPlayer() && GetInfoStruct()->get_max_concentration_base() == 0)
		GetInfoStruct()->set_max_concentration_base(5);
}

Entity::~Entity(){
	MutexList<BonusValues*>::iterator itr2 = bonus_list.begin();
	while(itr2.Next())
		safe_delete(itr2.value);
	ClearProcs();
	safe_delete(m_threatTransfer);
	map<int8, MutexList<LuaSpell*>*>::iterator itr3;
	for (itr3 = control_effects.begin(); itr3 != control_effects.end(); itr3++)
		safe_delete(itr3->second);
	control_effects.clear();
	map<int8, MutexList<LuaSpell*>*>::iterator itr4;
	for (itr4 = immunities.begin(); itr4 != immunities.end(); itr4++)
		safe_delete(itr4->second);
	immunities.clear();
	if(!IsPlayer())
		DeleteSpellEffects(true);
	safe_delete(m_threatTransfer);
}

void Entity::DeleteSpellEffects(bool removeClient)
{
	map<LuaSpell*,bool> deletedPtrs;

	for(int i=0;i<45;i++){
		if(i<30){
			if(GetInfoStruct()->maintained_effects[i].spell_id != 0xFFFFFFFF)
			{
				if(deletedPtrs.find(GetInfoStruct()->maintained_effects[i].spell) == deletedPtrs.end())
				{
					deletedPtrs[GetInfoStruct()->maintained_effects[i].spell] = true;
					lua_interface->RemoveSpell(GetInfoStruct()->maintained_effects[i].spell, false, removeClient, "", removeClient);
					if (IsPlayer())
						GetInfoStruct()->maintained_effects[i].icon = 0xFFFF;
				}
				
				GetInfoStruct()->maintained_effects[i].spell_id = 0xFFFFFFFF;
				GetInfoStruct()->maintained_effects[i].spell = nullptr;
			}
		}
		if(GetInfoStruct()->spell_effects[i].spell_id != 0xFFFFFFFF)
		{
			if(deletedPtrs.find(GetInfoStruct()->spell_effects[i].spell) == deletedPtrs.end()) {
				if(GetInfoStruct()->spell_effects[i].spell && GetInfoStruct()->spell_effects[i].spell->spell && 
					GetInfoStruct()->spell_effects[i].spell->spell->GetSpellData()->spell_book_type == SPELL_BOOK_TYPE_NOT_SHOWN) {
					deletedPtrs[GetInfoStruct()->spell_effects[i].spell] = true;
					lua_interface->RemoveSpell(GetInfoStruct()->spell_effects[i].spell, false, removeClient, "", removeClient);
				}
			}
			GetInfoStruct()->spell_effects[i].spell_id = 0xFFFFFFFF;
			GetInfoStruct()->spell_effects[i].spell = nullptr;
		}
	}
}

void Entity::RemoveSpells(bool unfriendlyOnly)
{
	
	for(int i=0;i<45;i++){
		if(i<30){
			if(GetInfoStruct()->maintained_effects[i].spell_id != 0xFFFFFFFF)
			{
				if(!unfriendlyOnly || (unfriendlyOnly && GetInfoStruct()->maintained_effects[i].spell && 
					!GetInfoStruct()->maintained_effects[i].spell->spell->GetSpellData()->friendly_spell))
					GetZone()->GetSpellProcess()->AddSpellCancel(GetInfoStruct()->maintained_effects[i].spell);
			}
		}
		if(GetInfoStruct()->spell_effects[i].spell_id != 0xFFFFFFFF)
		{
				if(!unfriendlyOnly || (unfriendlyOnly && GetInfoStruct()->spell_effects[i].spell && 
					!GetInfoStruct()->spell_effects[i].spell->spell->GetSpellData()->friendly_spell))
					RemoveSpellEffect(GetInfoStruct()->spell_effects[i].spell);
		}
	}
}

void Entity::MapInfoStruct()
{
/** GETS **/
	get_string_funcs["name"] = l::bind(&InfoStruct::get_name, &info_struct);
	get_int8_funcs["class1"] = l::bind(&InfoStruct::get_class1, &info_struct);
	get_int8_funcs["class2"] = l::bind(&InfoStruct::get_class2, &info_struct);
	get_int8_funcs["class3"] = l::bind(&InfoStruct::get_class3, &info_struct);
	get_int8_funcs["race"] = l::bind(&InfoStruct::get_race, &info_struct);
	get_int8_funcs["gender"] = l::bind(&InfoStruct::get_gender, &info_struct);
	get_int16_funcs["level"] = l::bind(&InfoStruct::get_level, &info_struct);
	get_int16_funcs["max_level"] = l::bind(&InfoStruct::get_max_level, &info_struct);
	get_int16_funcs["effective_level"] = l::bind(&InfoStruct::get_effective_level, &info_struct);
	get_int16_funcs["tradeskill_level"] = l::bind(&InfoStruct::get_tradeskill_level, &info_struct);
	get_int16_funcs["tradeskill_max_level"] = l::bind(&InfoStruct::get_tradeskill_max_level, &info_struct);
	get_int8_funcs["cur_concentration"] = l::bind(&InfoStruct::get_cur_concentration, &info_struct);
	get_int8_funcs["max_concentration"] = l::bind(&InfoStruct::get_max_concentration, &info_struct);
	get_int8_funcs["max_concentration_base"] = l::bind(&InfoStruct::get_max_concentration_base, &info_struct);
	get_int16_funcs["cur_attack"] = l::bind(&InfoStruct::get_cur_attack, &info_struct);
	get_int16_funcs["attack_base"] = l::bind(&InfoStruct::get_attack_base, &info_struct);
	get_int16_funcs["cur_mitigation"] = l::bind(&InfoStruct::get_cur_mitigation, &info_struct);
	get_int16_funcs["max_mitigation"] = l::bind(&InfoStruct::get_max_mitigation, &info_struct);
	get_int16_funcs["mitigation_base"] = l::bind(&InfoStruct::get_mitigation_base, &info_struct);
	get_int16_funcs["avoidance_display"] = l::bind(&InfoStruct::get_avoidance_display, &info_struct);
	get_float_funcs["cur_avoidance"] = l::bind(&InfoStruct::get_cur_avoidance, &info_struct);
	get_int16_funcs["base_avoidance_pct"] = l::bind(&InfoStruct::get_base_avoidance_pct, &info_struct);
	get_int16_funcs["avoidance_base"] = l::bind(&InfoStruct::get_avoidance_base, &info_struct);
	get_int16_funcs["max_avoidance"] = l::bind(&InfoStruct::get_max_avoidance, &info_struct);
	get_float_funcs["parry"] = l::bind(&InfoStruct::get_parry, &info_struct);
	get_float_funcs["parry_base"] = l::bind(&InfoStruct::get_parry_base, &info_struct);
	get_float_funcs["deflection"] = l::bind(&InfoStruct::get_deflection, &info_struct);
	get_int16_funcs["deflection_base"] = l::bind(&InfoStruct::get_deflection_base, &info_struct);
	get_float_funcs["block"] = l::bind(&InfoStruct::get_block, &info_struct);
	get_int16_funcs["block_base"] = l::bind(&InfoStruct::get_block_base, &info_struct);
	get_float_funcs["str"] = l::bind(&InfoStruct::get_str, &info_struct);
	get_float_funcs["sta"] = l::bind(&InfoStruct::get_sta, &info_struct);
	get_float_funcs["agi"] = l::bind(&InfoStruct::get_agi, &info_struct);
	get_float_funcs["wis"] = l::bind(&InfoStruct::get_wis, &info_struct);
	get_float_funcs["intel"] = l::bind(&InfoStruct::get_intel, &info_struct);
	get_float_funcs["str_base"] = l::bind(&InfoStruct::get_str_base, &info_struct);
	get_float_funcs["sta_base"] = l::bind(&InfoStruct::get_sta_base, &info_struct);
	get_float_funcs["agi_base"] = l::bind(&InfoStruct::get_agi_base, &info_struct);
	get_float_funcs["wis_base"] = l::bind(&InfoStruct::get_wis_base, &info_struct);
	get_float_funcs["intel_base"] = l::bind(&InfoStruct::get_intel_base, &info_struct);
	get_int16_funcs["heat"] = l::bind(&InfoStruct::get_heat, &info_struct);
	get_int16_funcs["cold"] = l::bind(&InfoStruct::get_cold, &info_struct);
	get_int16_funcs["magic"] = l::bind(&InfoStruct::get_magic, &info_struct);
	get_int16_funcs["mental"] = l::bind(&InfoStruct::get_mental, &info_struct);
	get_int16_funcs["divine"] = l::bind(&InfoStruct::get_divine, &info_struct);
	get_int16_funcs["disease"] = l::bind(&InfoStruct::get_disease, &info_struct);
	get_int16_funcs["poison"] = l::bind(&InfoStruct::get_poison, &info_struct);
	get_int16_funcs["disease_base"] = l::bind(&InfoStruct::get_disease_base, &info_struct);
	get_int16_funcs["cold_base"] = l::bind(&InfoStruct::get_cold_base, &info_struct);
	get_int16_funcs["divine_base"] = l::bind(&InfoStruct::get_divine_base, &info_struct);
	get_int16_funcs["magic_base"] = l::bind(&InfoStruct::get_magic_base, &info_struct);
	get_int16_funcs["mental_base"] = l::bind(&InfoStruct::get_mental_base, &info_struct);
	get_int16_funcs["heat_base"] = l::bind(&InfoStruct::get_heat_base, &info_struct);
	get_int16_funcs["poison_base"] = l::bind(&InfoStruct::get_poison_base, &info_struct);
	get_int16_funcs["elemental_base"] = l::bind(&InfoStruct::get_elemental_base, &info_struct);
	get_int16_funcs["noxious_base"] = l::bind(&InfoStruct::get_noxious_base, &info_struct);
	get_int16_funcs["arcane_base"] = l::bind(&InfoStruct::get_arcane_base, &info_struct);
	get_int32_funcs["coin_copper"] = l::bind(&InfoStruct::get_coin_copper, &info_struct);
	get_int32_funcs["coin_silver"] = l::bind(&InfoStruct::get_coin_silver, &info_struct);
	get_int32_funcs["coin_gold"] = l::bind(&InfoStruct::get_coin_gold, &info_struct);
	get_int32_funcs["coin_plat"] = l::bind(&InfoStruct::get_coin_plat, &info_struct);
	get_int32_funcs["bank_coin_copper"] = l::bind(&InfoStruct::get_bank_coin_copper, &info_struct);
	get_int32_funcs["bank_coin_silver"] = l::bind(&InfoStruct::get_bank_coin_silver, &info_struct);
	get_int32_funcs["bank_coin_gold"] = l::bind(&InfoStruct::get_bank_coin_gold, &info_struct);
	get_int32_funcs["bank_coin_plat"] = l::bind(&InfoStruct::get_bank_coin_plat, &info_struct);
	get_int32_funcs["status_points"] = l::bind(&InfoStruct::get_status_points, &info_struct);
	get_string_funcs["deity"] = l::bind(&InfoStruct::get_deity, &info_struct);
	get_int32_funcs["weight"] = l::bind(&InfoStruct::get_weight, &info_struct);
	get_int32_funcs["max_weight"] = l::bind(&InfoStruct::get_max_weight, &info_struct);
	get_int8_funcs["tradeskill_class1"] = l::bind(&InfoStruct::get_tradeskill_class1, &info_struct);
	get_int8_funcs["tradeskill_class2"] = l::bind(&InfoStruct::get_tradeskill_class2, &info_struct);
	get_int8_funcs["tradeskill_class3"] = l::bind(&InfoStruct::get_tradeskill_class3, &info_struct);
	get_int32_funcs["account_age_base"] = l::bind(&InfoStruct::get_account_age_base, &info_struct);
	//	int8			account_age_bonus_[19];
	get_int16_funcs["absorb"] = l::bind(&InfoStruct::get_absorb, &info_struct);
	get_int32_funcs["xp"] = l::bind(&InfoStruct::get_xp, &info_struct);
	get_int32_funcs["xp_needed"] = l::bind(&InfoStruct::get_xp_needed, &info_struct);
	get_float_funcs["xp_debt"] = l::bind(&InfoStruct::get_xp_debt, &info_struct);
	get_int16_funcs["xp_yellow"] = l::bind(&InfoStruct::get_xp_yellow, &info_struct);
	get_int16_funcs["xp_yellow_vitality_bar"] = l::bind(&InfoStruct::get_xp_yellow_vitality_bar, &info_struct);
	get_int16_funcs["xp_blue_vitality_bar"] = l::bind(&InfoStruct::get_xp_blue_vitality_bar, &info_struct);
	get_int16_funcs["xp_blue"] = l::bind(&InfoStruct::get_xp_blue, &info_struct);
	get_int32_funcs["ts_xp"] = l::bind(&InfoStruct::get_ts_xp, &info_struct);
	get_int32_funcs["ts_xp_needed"] = l::bind(&InfoStruct::get_ts_xp_needed, &info_struct);
	get_int16_funcs["tradeskill_exp_yellow"] = l::bind(&InfoStruct::get_tradeskill_exp_yellow, &info_struct);
	get_int16_funcs["tradeskill_exp_blue"] = l::bind(&InfoStruct::get_tradeskill_exp_blue, &info_struct);
	get_int32_funcs["flags"] = l::bind(&InfoStruct::get_flags, &info_struct);
	get_int32_funcs["flags2"] = l::bind(&InfoStruct::get_flags2, &info_struct);
	get_float_funcs["xp_vitality"] = l::bind(&InfoStruct::get_xp_vitality, &info_struct);
	get_float_funcs["tradeskill_xp_vitality"] = l::bind(&InfoStruct::get_tradeskill_xp_vitality, &info_struct);
	get_int16_funcs["mitigation_skill1"] = l::bind(&InfoStruct::get_mitigation_skill1, &info_struct);
	get_int16_funcs["mitigation_skill2"] = l::bind(&InfoStruct::get_mitigation_skill2, &info_struct);
	get_int16_funcs["mitigation_skill3"] = l::bind(&InfoStruct::get_mitigation_skill3, &info_struct);
	get_int16_funcs["mitigation_pve"] = l::bind(&InfoStruct::get_mitigation_pve, &info_struct);
	get_int16_funcs["mitigation_pvp"] = l::bind(&InfoStruct::get_mitigation_pvp, &info_struct);
	get_float_funcs["ability_modifier"] = l::bind(&InfoStruct::get_ability_modifier, &info_struct);
	get_float_funcs["critical_mitigation"] = l::bind(&InfoStruct::get_critical_mitigation, &info_struct);
	get_float_funcs["block_chance"] = l::bind(&InfoStruct::get_block_chance, &info_struct);
	get_float_funcs["uncontested_parry"] = l::bind(&InfoStruct::get_uncontested_parry, &info_struct);
	get_float_funcs["uncontested_block"] = l::bind(&InfoStruct::get_uncontested_block, &info_struct);
	get_float_funcs["uncontested_dodge"] = l::bind(&InfoStruct::get_uncontested_dodge, &info_struct);
	get_float_funcs["uncontested_riposte"] = l::bind(&InfoStruct::get_uncontested_riposte, &info_struct);
	get_float_funcs["crit_chance"] = l::bind(&InfoStruct::get_crit_chance, &info_struct);
	get_float_funcs["crit_bonus"] = l::bind(&InfoStruct::get_crit_bonus, &info_struct);
	get_float_funcs["potency"] = l::bind(&InfoStruct::get_potency, &info_struct);
	get_float_funcs["hate_mod"] = l::bind(&InfoStruct::get_hate_mod, &info_struct);
	get_float_funcs["reuse_speed"] = l::bind(&InfoStruct::get_reuse_speed, &info_struct);
	get_float_funcs["casting_speed"] = l::bind(&InfoStruct::get_casting_speed, &info_struct);
	get_float_funcs["recovery_speed"] = l::bind(&InfoStruct::get_recovery_speed, &info_struct);
	get_float_funcs["spell_reuse_speed"] = l::bind(&InfoStruct::get_spell_reuse_speed, &info_struct);
	get_float_funcs["spell_multi_attack"] = l::bind(&InfoStruct::get_spell_multi_attack, &info_struct);
	get_float_funcs["dps"] = l::bind(&InfoStruct::get_dps, &info_struct);
	get_float_funcs["dps_multiplier"] = l::bind(&InfoStruct::get_dps_multiplier, &info_struct);
	get_float_funcs["attackspeed"] = l::bind(&InfoStruct::get_attackspeed, &info_struct);
	get_float_funcs["haste"] = l::bind(&InfoStruct::get_haste, &info_struct);
	get_float_funcs["multi_attack"] = l::bind(&InfoStruct::get_multi_attack, &info_struct);
	get_float_funcs["flurry"] = l::bind(&InfoStruct::get_flurry, &info_struct);
	get_float_funcs["melee_ae"] = l::bind(&InfoStruct::get_melee_ae, &info_struct);
	get_float_funcs["strikethrough"] = l::bind(&InfoStruct::get_strikethrough, &info_struct);
	get_float_funcs["accuracy"] = l::bind(&InfoStruct::get_accuracy, &info_struct);
	get_float_funcs["offensivespeed"] = l::bind(&InfoStruct::get_offensivespeed, &info_struct);
	get_float_funcs["rain"] = l::bind(&InfoStruct::get_rain, &info_struct);
	get_float_funcs["wind"] = l::bind(&InfoStruct::get_wind, &info_struct);
	get_sint8_funcs["alignment"] = l::bind(&InfoStruct::get_alignment, &info_struct);
	get_int32_funcs["pet_id"] = l::bind(&InfoStruct::get_pet_id, &info_struct);
	get_string_funcs["pet_name"] = l::bind(&InfoStruct::get_pet_name, &info_struct);
	get_float_funcs["pet_health_pct"] = l::bind(&InfoStruct::get_pet_health_pct, &info_struct);
	get_float_funcs["pet_power_pct"] = l::bind(&InfoStruct::get_pet_power_pct, &info_struct);
	get_int8_funcs["pet_movement"] = l::bind(&InfoStruct::get_pet_movement, &info_struct);
	get_int8_funcs["pet_behavior"] = l::bind(&InfoStruct::get_pet_behavior, &info_struct);
	get_int32_funcs["vision"] = l::bind(&InfoStruct::get_vision, &info_struct);
	get_int8_funcs["breathe_underwater"] = l::bind(&InfoStruct::get_breathe_underwater, &info_struct);
	get_string_funcs["biography"] = l::bind(&InfoStruct::get_biography, &info_struct);
	get_float_funcs["drunk"] = l::bind(&InfoStruct::get_drunk, &info_struct);

	get_sint16_funcs["power_regen"] = l::bind(&InfoStruct::get_power_regen, &info_struct);
	get_sint16_funcs["hp_regen"] = l::bind(&InfoStruct::get_hp_regen, &info_struct);

	get_int8_funcs["power_regen_override"] = l::bind(&InfoStruct::get_power_regen_override, &info_struct);
	get_int8_funcs["hp_regen_override"] = l::bind(&InfoStruct::get_hp_regen_override, &info_struct);
	
	get_int8_funcs["water_type"] = l::bind(&InfoStruct::get_water_type, &info_struct);
	get_int8_funcs["flying_type"] = l::bind(&InfoStruct::get_flying_type, &info_struct);
	
	get_int8_funcs["no_interrupt"] = l::bind(&InfoStruct::get_no_interrupt, &info_struct);

	get_int8_funcs["interaction_flag"] = l::bind(&InfoStruct::get_interaction_flag, &info_struct);
	get_int8_funcs["tag1"] = l::bind(&InfoStruct::get_tag1, &info_struct);
	get_int16_funcs["mood"] = l::bind(&InfoStruct::get_mood, &info_struct);
	
	get_int32_funcs["range_last_attack_time"] = l::bind(&InfoStruct::get_range_last_attack_time, &info_struct);
	get_int32_funcs["primary_last_attack_time"] = l::bind(&InfoStruct::get_primary_last_attack_time, &info_struct);
	get_int32_funcs["secondary_last_attack_time"] = l::bind(&InfoStruct::get_secondary_last_attack_time, &info_struct);
	
	get_int16_funcs["primary_attack_delay"] = l::bind(&InfoStruct::get_primary_attack_delay, &info_struct);
	get_int16_funcs["secondary_attack_delay"] = l::bind(&InfoStruct::get_secondary_attack_delay, &info_struct);
	get_int16_funcs["ranged_attack_delay"] = l::bind(&InfoStruct::get_ranged_attack_delay, &info_struct);
	
	get_int8_funcs["primary_weapon_type"] = l::bind(&InfoStruct::get_primary_weapon_type, &info_struct);
	get_int8_funcs["secondary_weapon_type"] = l::bind(&InfoStruct::get_secondary_weapon_type, &info_struct);
	get_int8_funcs["ranged_weapon_type"] = l::bind(&InfoStruct::get_ranged_weapon_type, &info_struct);
	
	get_int32_funcs["primary_weapon_damage_low"] = l::bind(&InfoStruct::get_primary_weapon_damage_low, &info_struct);
	get_int32_funcs["primary_weapon_damage_high"] = l::bind(&InfoStruct::get_primary_weapon_damage_high, &info_struct);
	get_int32_funcs["secondary_weapon_damage_low"] = l::bind(&InfoStruct::get_secondary_weapon_damage_low, &info_struct);
	get_int32_funcs["secondary_weapon_damage_high"] = l::bind(&InfoStruct::get_secondary_weapon_damage_high, &info_struct);
	get_int32_funcs["ranged_weapon_damage_low"] = l::bind(&InfoStruct::get_ranged_weapon_damage_low, &info_struct);
	get_int32_funcs["ranged_weapon_damage_high"] = l::bind(&InfoStruct::get_ranged_weapon_damage_high, &info_struct);
	
	get_int8_funcs["wield_type"] = l::bind(&InfoStruct::get_wield_type, &info_struct);
	get_int8_funcs["attack_type"] = l::bind(&InfoStruct::get_attack_type, &info_struct);
	
	get_int16_funcs["primary_weapon_delay"] = l::bind(&InfoStruct::get_primary_weapon_delay, &info_struct);
	get_int16_funcs["secondary_weapon_delay"] = l::bind(&InfoStruct::get_secondary_weapon_delay, &info_struct);
	get_int16_funcs["ranged_weapon_delay"] = l::bind(&InfoStruct::get_ranged_weapon_delay, &info_struct);
	
	get_int8_funcs["override_primary_weapon"] = l::bind(&InfoStruct::get_override_primary_weapon, &info_struct);
	get_int8_funcs["override_secondary_weapon"] = l::bind(&InfoStruct::get_override_secondary_weapon, &info_struct);
	get_int8_funcs["override_ranged_weapon"] = l::bind(&InfoStruct::get_override_ranged_weapon, &info_struct);
	
	get_int8_funcs["friendly_target_npc"] = l::bind(&InfoStruct::get_friendly_target_npc, &info_struct);
	get_int32_funcs["last_claim_time"] = l::bind(&InfoStruct::get_last_claim_time, &info_struct);
	
	get_int8_funcs["engaged_encounter"] = l::bind(&InfoStruct::get_engaged_encounter, &info_struct);
	
	get_int8_funcs["first_world_login"] = l::bind(&InfoStruct::get_first_world_login, &info_struct);
	
	get_int8_funcs["reload_player_spells"] = l::bind(&InfoStruct::get_reload_player_spells, &info_struct);
	
	get_int8_funcs["group_loot_method"] = l::bind(&InfoStruct::get_group_loot_method, &info_struct);
	get_int8_funcs["group_loot_items_rarity"] = l::bind(&InfoStruct::get_group_loot_items_rarity, &info_struct);
	get_int8_funcs["group_auto_split"] = l::bind(&InfoStruct::get_group_auto_split, &info_struct);
	get_int8_funcs["group_default_yell"] = l::bind(&InfoStruct::get_group_default_yell, &info_struct);
	get_int8_funcs["group_autolock"] = l::bind(&InfoStruct::get_group_autolock, &info_struct);
	get_int8_funcs["group_lock_method"] = l::bind(&InfoStruct::get_group_lock_method, &info_struct);
	get_int8_funcs["group_solo_autolock"] = l::bind(&InfoStruct::get_group_solo_autolock, &info_struct);
	get_int8_funcs["group_auto_loot_method"] = l::bind(&InfoStruct::get_group_auto_loot_method, &info_struct);
	get_int8_funcs["assist_auto_attack"] = l::bind(&InfoStruct::get_assist_auto_attack, &info_struct);
	
	get_string_funcs["action_state"] = l::bind(&InfoStruct::get_action_state, &info_struct);
	get_string_funcs["combat_action_state"] = l::bind(&InfoStruct::get_combat_action_state, &info_struct);
	
	get_float_funcs["max_spell_reduction"] = l::bind(&InfoStruct::get_max_spell_reduction, &info_struct);
	get_int8_funcs["max_spell_reduction_override"] = l::bind(&InfoStruct::get_max_spell_reduction_override, &info_struct);

/** SETS **/
	set_string_funcs["name"] = l::bind(&InfoStruct::set_name, &info_struct, l::_1);
	set_int8_funcs["class1"] = l::bind(&InfoStruct::set_class1, &info_struct, l::_1);
	set_int8_funcs["class2"] = l::bind(&InfoStruct::set_class2, &info_struct, l::_1);
	set_int8_funcs["class3"] = l::bind(&InfoStruct::set_class3, &info_struct, l::_1);
	set_int8_funcs["race"] = l::bind(&InfoStruct::set_race, &info_struct, l::_1);
	set_int8_funcs["gender"] = l::bind(&InfoStruct::set_gender, &info_struct, l::_1);
	set_int16_funcs["level"] = l::bind(&InfoStruct::set_level, &info_struct, l::_1);
	set_int16_funcs["max_level"] = l::bind(&InfoStruct::set_max_level, &info_struct, l::_1);
	set_int16_funcs["effective_level"] = l::bind(&InfoStruct::set_effective_level, &info_struct, l::_1);
	set_int16_funcs["tradeskill_level"] = l::bind(&InfoStruct::set_tradeskill_level, &info_struct, l::_1);
	set_int16_funcs["tradeskill_max_level"] = l::bind(&InfoStruct::set_tradeskill_max_level, &info_struct, l::_1);
	set_int8_funcs["cur_concentration"] = l::bind(&InfoStruct::set_cur_concentration, &info_struct, l::_1);
	set_int8_funcs["max_concentration"] = l::bind(&InfoStruct::set_max_concentration, &info_struct, l::_1);
	set_int8_funcs["max_concentration_base"] = l::bind(&InfoStruct::set_max_concentration_base, &info_struct, l::_1);
	set_int16_funcs["cur_attack"] = l::bind(&InfoStruct::set_cur_attack, &info_struct, l::_1);
	set_int16_funcs["attack_base"] = l::bind(&InfoStruct::set_attack_base, &info_struct, l::_1);
	set_int16_funcs["cur_mitigation"] = l::bind(&InfoStruct::set_cur_mitigation, &info_struct, l::_1);
	set_int16_funcs["max_mitigation"] = l::bind(&InfoStruct::set_max_mitigation, &info_struct, l::_1);
	set_int16_funcs["mitigation_base"] = l::bind(&InfoStruct::set_mitigation_base, &info_struct, l::_1);
	set_int16_funcs["avoidance_display"] = l::bind(&InfoStruct::set_avoidance_display, &info_struct, l::_1);
	set_float_funcs["cur_avoidance"] = l::bind(&InfoStruct::set_cur_avoidance, &info_struct, l::_1);
	set_int16_funcs["base_avoidance_pct"] = l::bind(&InfoStruct::set_base_avoidance_pct, &info_struct, l::_1);
	set_int16_funcs["avoidance_base"] = l::bind(&InfoStruct::set_avoidance_base, &info_struct, l::_1);
	set_int16_funcs["max_avoidance"] = l::bind(&InfoStruct::set_max_avoidance, &info_struct, l::_1);
	set_float_funcs["parry"] = l::bind(&InfoStruct::set_parry, &info_struct, l::_1);
	set_float_funcs["parry_base"] = l::bind(&InfoStruct::set_parry_base, &info_struct, l::_1);
	set_float_funcs["deflection"] = l::bind(&InfoStruct::set_deflection, &info_struct, l::_1);
	set_int16_funcs["deflection_base"] = l::bind(&InfoStruct::set_deflection_base, &info_struct, l::_1);
	set_float_funcs["block"] = l::bind(&InfoStruct::set_block, &info_struct, l::_1);
	set_int16_funcs["block_base"] = l::bind(&InfoStruct::set_block_base, &info_struct, l::_1);
	set_float_funcs["str"] = l::bind(&InfoStruct::set_str, &info_struct, l::_1);
	set_float_funcs["sta"] = l::bind(&InfoStruct::set_sta, &info_struct, l::_1);
	set_float_funcs["agi"] = l::bind(&InfoStruct::set_agi, &info_struct, l::_1);
	set_float_funcs["wis"] = l::bind(&InfoStruct::set_wis, &info_struct, l::_1);
	set_float_funcs["intel"] = l::bind(&InfoStruct::set_intel, &info_struct, l::_1);
	set_float_funcs["str_base"] = l::bind(&InfoStruct::set_str_base, &info_struct, l::_1);
	set_float_funcs["sta_base"] = l::bind(&InfoStruct::set_sta_base, &info_struct, l::_1);
	set_float_funcs["agi_base"] = l::bind(&InfoStruct::set_agi_base, &info_struct, l::_1);
	set_float_funcs["wis_base"] = l::bind(&InfoStruct::set_wis_base, &info_struct, l::_1);
	set_float_funcs["intel_base"] = l::bind(&InfoStruct::set_intel_base, &info_struct, l::_1);
	set_int16_funcs["heat"] = l::bind(&InfoStruct::set_heat, &info_struct, l::_1);
	set_int16_funcs["cold"] = l::bind(&InfoStruct::set_cold, &info_struct, l::_1);
	set_int16_funcs["magic"] = l::bind(&InfoStruct::set_magic, &info_struct, l::_1);
	set_int16_funcs["mental"] = l::bind(&InfoStruct::set_mental, &info_struct, l::_1);
	set_int16_funcs["divine"] = l::bind(&InfoStruct::set_divine, &info_struct, l::_1);
	set_int16_funcs["disease"] = l::bind(&InfoStruct::set_disease, &info_struct, l::_1);
	set_int16_funcs["poison"] = l::bind(&InfoStruct::set_poison, &info_struct, l::_1);
	set_int16_funcs["disease_base"] = l::bind(&InfoStruct::set_disease_base, &info_struct, l::_1);
	set_int16_funcs["cold_base"] = l::bind(&InfoStruct::set_cold_base, &info_struct, l::_1);
	set_int16_funcs["divine_base"] = l::bind(&InfoStruct::set_divine_base, &info_struct, l::_1);
	set_int16_funcs["magic_base"] = l::bind(&InfoStruct::set_magic_base, &info_struct, l::_1);
	set_int16_funcs["mental_base"] = l::bind(&InfoStruct::set_mental_base, &info_struct, l::_1);
	set_int16_funcs["heat_base"] = l::bind(&InfoStruct::set_heat_base, &info_struct, l::_1);
	set_int16_funcs["poison_base"] = l::bind(&InfoStruct::set_poison_base, &info_struct, l::_1);
	set_int16_funcs["elemental_base"] = l::bind(&InfoStruct::set_elemental_base, &info_struct, l::_1);
	set_int16_funcs["noxious_base"] = l::bind(&InfoStruct::set_noxious_base, &info_struct, l::_1);
	set_int16_funcs["arcane_base"] = l::bind(&InfoStruct::set_arcane_base, &info_struct, l::_1);
	set_int32_funcs["coin_copper"] = l::bind(&InfoStruct::set_coin_copper, &info_struct, l::_1);
	set_int32_funcs["coin_silver"] = l::bind(&InfoStruct::set_coin_silver, &info_struct, l::_1);
	set_int32_funcs["coin_gold"] = l::bind(&InfoStruct::set_coin_gold, &info_struct, l::_1);
	set_int32_funcs["coin_plat"] = l::bind(&InfoStruct::set_coin_plat, &info_struct, l::_1);
	set_int32_funcs["bank_coin_copper"] = l::bind(&InfoStruct::set_bank_coin_copper, &info_struct, l::_1);
	set_int32_funcs["bank_coin_silver"] = l::bind(&InfoStruct::set_bank_coin_silver, &info_struct, l::_1);
	set_int32_funcs["bank_coin_gold"] = l::bind(&InfoStruct::set_bank_coin_gold, &info_struct, l::_1);
	set_int32_funcs["bank_coin_plat"] = l::bind(&InfoStruct::set_bank_coin_plat, &info_struct, l::_1);
	set_int32_funcs["status_points"] = l::bind(&InfoStruct::set_status_points, &info_struct, l::_1);
	set_string_funcs["deity"] = l::bind(&InfoStruct::set_deity, &info_struct, l::_1);
	set_int32_funcs["weight"] = l::bind(&InfoStruct::set_weight, &info_struct, l::_1);
	set_int32_funcs["max_weight"] = l::bind(&InfoStruct::set_max_weight, &info_struct, l::_1);
	set_int8_funcs["tradeskill_class1"] = l::bind(&InfoStruct::set_tradeskill_class1, &info_struct, l::_1);
	set_int8_funcs["tradeskill_class2"] = l::bind(&InfoStruct::set_tradeskill_class2, &info_struct, l::_1);
	set_int8_funcs["tradeskill_class3"] = l::bind(&InfoStruct::set_tradeskill_class3, &info_struct, l::_1);
	set_int32_funcs["account_age_base"] = l::bind(&InfoStruct::set_account_age_base, &info_struct, l::_1);
	//	int8			account_age_bonus_[19];
	set_int16_funcs["absorb"] = l::bind(&InfoStruct::set_absorb, &info_struct, l::_1);
	set_int32_funcs["xp"] = l::bind(&InfoStruct::set_xp, &info_struct, l::_1);
	set_int32_funcs["xp_needed"] = l::bind(&InfoStruct::set_xp_needed, &info_struct, l::_1);
	set_float_funcs["xp_debt"] = l::bind(&InfoStruct::set_xp_debt, &info_struct, l::_1);
	set_int16_funcs["xp_yellow"] = l::bind(&InfoStruct::set_xp_yellow, &info_struct, l::_1);
	set_int16_funcs["xp_yellow_vitality_bar"] = l::bind(&InfoStruct::set_xp_yellow_vitality_bar, &info_struct, l::_1);
	set_int16_funcs["xp_blue_vitality_bar"] = l::bind(&InfoStruct::set_xp_blue_vitality_bar, &info_struct, l::_1);
	set_int16_funcs["xp_blue"] = l::bind(&InfoStruct::set_xp_blue, &info_struct, l::_1);
	set_int32_funcs["ts_xp"] = l::bind(&InfoStruct::set_ts_xp, &info_struct, l::_1);
	set_int32_funcs["ts_xp_needed"] = l::bind(&InfoStruct::set_ts_xp_needed, &info_struct, l::_1);
	set_int16_funcs["tradeskill_exp_yellow"] = l::bind(&InfoStruct::set_tradeskill_exp_yellow, &info_struct, l::_1);
	set_int16_funcs["tradeskill_exp_blue"] = l::bind(&InfoStruct::set_tradeskill_exp_blue, &info_struct, l::_1);
	set_int32_funcs["flags"] = l::bind(&InfoStruct::set_flags, &info_struct, l::_1);
	set_int32_funcs["flags2"] = l::bind(&InfoStruct::set_flags2, &info_struct, l::_1);
	set_float_funcs["xp_vitality"] = l::bind(&InfoStruct::set_xp_vitality, &info_struct, l::_1);
	set_float_funcs["tradeskill_xp_vitality"] = l::bind(&InfoStruct::set_tradeskill_xp_vitality, &info_struct, l::_1);
	set_int16_funcs["mitigation_skill1"] = l::bind(&InfoStruct::set_mitigation_skill1, &info_struct, l::_1);
	set_int16_funcs["mitigation_skill2"] = l::bind(&InfoStruct::set_mitigation_skill2, &info_struct, l::_1);
	set_int16_funcs["mitigation_skill3"] = l::bind(&InfoStruct::set_mitigation_skill3, &info_struct, l::_1);
	set_int16_funcs["mitigation_pve"] = l::bind(&InfoStruct::set_mitigation_pve, &info_struct, l::_1);
	set_int16_funcs["mitigation_pvp"] = l::bind(&InfoStruct::set_mitigation_pvp, &info_struct, l::_1);
	set_float_funcs["ability_modifier"] = l::bind(&InfoStruct::set_ability_modifier, &info_struct, l::_1);
	set_float_funcs["critical_mitigation"] = l::bind(&InfoStruct::set_critical_mitigation, &info_struct, l::_1);
	set_float_funcs["block_chance"] = l::bind(&InfoStruct::set_block_chance, &info_struct, l::_1);
	set_float_funcs["uncontested_parry"] = l::bind(&InfoStruct::set_uncontested_parry, &info_struct, l::_1);
	set_float_funcs["uncontested_block"] = l::bind(&InfoStruct::set_uncontested_block, &info_struct, l::_1);
	set_float_funcs["uncontested_dodge"] = l::bind(&InfoStruct::set_uncontested_dodge, &info_struct, l::_1);
	set_float_funcs["uncontested_riposte"] = l::bind(&InfoStruct::set_uncontested_riposte, &info_struct, l::_1);
	set_float_funcs["crit_chance"] = l::bind(&InfoStruct::set_crit_chance, &info_struct, l::_1);
	set_float_funcs["crit_bonus"] = l::bind(&InfoStruct::set_crit_bonus, &info_struct, l::_1);
	set_float_funcs["potency"] = l::bind(&InfoStruct::set_potency, &info_struct, l::_1);
	set_float_funcs["hate_mod"] = l::bind(&InfoStruct::set_hate_mod, &info_struct, l::_1);
	set_float_funcs["reuse_speed"] = l::bind(&InfoStruct::set_reuse_speed, &info_struct, l::_1);
	set_float_funcs["casting_speed"] = l::bind(&InfoStruct::set_casting_speed, &info_struct, l::_1);
	set_float_funcs["recovery_speed"] = l::bind(&InfoStruct::set_recovery_speed, &info_struct, l::_1);
	set_float_funcs["spell_reuse_speed"] = l::bind(&InfoStruct::set_spell_reuse_speed, &info_struct, l::_1);
	set_float_funcs["spell_multi_attack"] = l::bind(&InfoStruct::set_spell_multi_attack, &info_struct, l::_1);
	set_float_funcs["dps"] = l::bind(&InfoStruct::set_dps, &info_struct, l::_1);
	set_float_funcs["dps_multiplier"] = l::bind(&InfoStruct::set_dps_multiplier, &info_struct, l::_1);
	set_float_funcs["attackspeed"] = l::bind(&InfoStruct::set_attackspeed, &info_struct, l::_1);
	set_float_funcs["haste"] = l::bind(&InfoStruct::set_haste, &info_struct, l::_1);
	set_float_funcs["multi_attack"] = l::bind(&InfoStruct::set_multi_attack, &info_struct, l::_1);
	set_float_funcs["flurry"] = l::bind(&InfoStruct::set_flurry, &info_struct, l::_1);
	set_float_funcs["melee_ae"] = l::bind(&InfoStruct::set_melee_ae, &info_struct, l::_1);
	set_float_funcs["strikethrough"] = l::bind(&InfoStruct::set_strikethrough, &info_struct, l::_1);
	set_float_funcs["accuracy"] = l::bind(&InfoStruct::set_accuracy, &info_struct, l::_1);
	set_float_funcs["offensivespeed"] = l::bind(&InfoStruct::set_offensivespeed, &info_struct, l::_1);
	set_float_funcs["rain"] = l::bind(&InfoStruct::set_rain, &info_struct, l::_1);
	set_float_funcs["wind"] = l::bind(&InfoStruct::set_wind, &info_struct, l::_1);
	set_sint8_funcs["alignment"] = l::bind(&InfoStruct::set_alignment, &info_struct, l::_1);
	set_int32_funcs["pet_id"] = l::bind(&InfoStruct::set_pet_id, &info_struct, l::_1);
	set_string_funcs["pet_name"] = l::bind(&InfoStruct::set_pet_name, &info_struct, l::_1);
	set_float_funcs["pet_health_pct"] = l::bind(&InfoStruct::set_pet_health_pct, &info_struct, l::_1);
	set_float_funcs["pet_power_pct"] = l::bind(&InfoStruct::set_pet_power_pct, &info_struct, l::_1);
	set_int8_funcs["pet_movement"] = l::bind(&InfoStruct::set_pet_movement, &info_struct, l::_1);
	set_int8_funcs["pet_behavior"] = l::bind(&InfoStruct::set_pet_behavior, &info_struct, l::_1);
	set_int32_funcs["vision"] = l::bind(&InfoStruct::set_vision, &info_struct, l::_1);
	set_int8_funcs["breathe_underwater"] = l::bind(&InfoStruct::set_breathe_underwater, &info_struct, l::_1);
	set_string_funcs["biography"] = l::bind(&InfoStruct::set_biography, &info_struct, l::_1);
	set_float_funcs["drunk"] = l::bind(&InfoStruct::set_drunk, &info_struct, l::_1);

	set_sint16_funcs["power_regen"] = l::bind(&InfoStruct::set_power_regen, &info_struct, l::_1);
	set_sint16_funcs["hp_regen"] = l::bind(&InfoStruct::set_hp_regen, &info_struct, l::_1);

	set_int8_funcs["power_regen_override"] = l::bind(&InfoStruct::set_power_regen_override, &info_struct, l::_1);
	set_int8_funcs["hp_regen_override"] = l::bind(&InfoStruct::set_hp_regen_override, &info_struct, l::_1);
	
	set_int8_funcs["water_type"] = l::bind(&InfoStruct::set_water_type, &info_struct, l::_1);
	set_int8_funcs["flying_type"] = l::bind(&InfoStruct::set_flying_type, &info_struct, l::_1);
	
	set_int8_funcs["no_interrupt"] = l::bind(&InfoStruct::set_no_interrupt, &info_struct, l::_1);

	set_int8_funcs["interaction_flag"] = l::bind(&InfoStruct::set_interaction_flag, &info_struct, l::_1);
	set_int8_funcs["tag1"] = l::bind(&InfoStruct::set_tag1, &info_struct, l::_1);
	set_int16_funcs["mood"] = l::bind(&InfoStruct::set_mood, &info_struct, l::_1);

	set_int32_funcs["range_last_attack_time"] = l::bind(&InfoStruct::set_range_last_attack_time, &info_struct, l::_1);
	set_int32_funcs["primary_last_attack_time"] = l::bind(&InfoStruct::set_primary_last_attack_time, &info_struct, l::_1);
	set_int32_funcs["secondary_last_attack_time"] = l::bind(&InfoStruct::set_secondary_last_attack_time, &info_struct, l::_1);
	
	set_int16_funcs["primary_attack_delay"] = l::bind(&InfoStruct::set_primary_attack_delay, &info_struct, l::_1);
	set_int16_funcs["secondary_attack_delay"] = l::bind(&InfoStruct::set_secondary_attack_delay, &info_struct, l::_1);
	set_int16_funcs["ranged_attack_delay"] = l::bind(&InfoStruct::set_ranged_attack_delay, &info_struct, l::_1);
	
	set_int8_funcs["primary_weapon_type"] = l::bind(&InfoStruct::set_primary_weapon_type, &info_struct, l::_1);
	set_int8_funcs["secondary_weapon_type"] = l::bind(&InfoStruct::set_secondary_weapon_type, &info_struct, l::_1);
	set_int8_funcs["ranged_weapon_type"] = l::bind(&InfoStruct::set_ranged_weapon_type, &info_struct, l::_1);
	
	set_int32_funcs["primary_weapon_damage_low"] = l::bind(&InfoStruct::set_primary_weapon_damage_low, &info_struct, l::_1);
	set_int32_funcs["primary_weapon_damage_high"] = l::bind(&InfoStruct::set_primary_weapon_damage_high, &info_struct, l::_1);
	set_int32_funcs["secondary_weapon_damage_low"] = l::bind(&InfoStruct::set_secondary_weapon_damage_low, &info_struct, l::_1);
	set_int32_funcs["secondary_weapon_damage_high"] = l::bind(&InfoStruct::set_secondary_weapon_damage_high, &info_struct, l::_1);
	set_int32_funcs["ranged_weapon_damage_low"] = l::bind(&InfoStruct::set_ranged_weapon_damage_low, &info_struct, l::_1);
	set_int32_funcs["ranged_weapon_damage_high"] = l::bind(&InfoStruct::set_ranged_weapon_damage_high, &info_struct, l::_1);
	
	set_int8_funcs["wield_type"] = l::bind(&InfoStruct::set_wield_type, &info_struct, l::_1);
	set_int8_funcs["attack_type"] = l::bind(&InfoStruct::set_attack_type, &info_struct, l::_1);
	
	set_int16_funcs["primary_weapon_delay"] = l::bind(&InfoStruct::set_primary_weapon_delay, &info_struct, l::_1);
	set_int16_funcs["secondary_weapon_delay"] = l::bind(&InfoStruct::set_secondary_weapon_delay, &info_struct, l::_1);
	set_int16_funcs["ranged_weapon_delay"] = l::bind(&InfoStruct::set_ranged_weapon_delay, &info_struct, l::_1);
	
	set_int8_funcs["override_primary_weapon"] = l::bind(&InfoStruct::set_override_primary_weapon, &info_struct, l::_1);
	set_int8_funcs["override_secondary_weapon"] = l::bind(&InfoStruct::set_override_secondary_weapon, &info_struct, l::_1);
	set_int8_funcs["override_ranged_weapon"] = l::bind(&InfoStruct::set_override_ranged_weapon, &info_struct, l::_1);
	
	set_int8_funcs["friendly_target_npc"] = l::bind(&InfoStruct::set_friendly_target_npc, &info_struct, l::_1);
	set_int32_funcs["last_claim_time"] = l::bind(&InfoStruct::set_last_claim_time, &info_struct, l::_1);
	
	set_int8_funcs["engaged_encounter"] = l::bind(&InfoStruct::set_engaged_encounter, &info_struct, l::_1);
	
	set_int8_funcs["first_world_login"] = l::bind(&InfoStruct::set_first_world_login, &info_struct, l::_1);
	
	set_int8_funcs["reload_player_spells"] = l::bind(&InfoStruct::set_reload_player_spells, &info_struct, l::_1);
	
	set_int8_funcs["group_loot_method"] = l::bind(&InfoStruct::set_group_loot_method, &info_struct, l::_1);
	set_int8_funcs["group_loot_items_rarity"] = l::bind(&InfoStruct::set_group_loot_items_rarity, &info_struct, l::_1);
	set_int8_funcs["group_auto_split"] = l::bind(&InfoStruct::set_group_auto_split, &info_struct, l::_1);
	set_int8_funcs["group_default_yell"] = l::bind(&InfoStruct::set_group_default_yell, &info_struct, l::_1);
	set_int8_funcs["group_autolock"] = l::bind(&InfoStruct::set_group_autolock, &info_struct, l::_1);
	set_int8_funcs["group_lock_method"] = l::bind(&InfoStruct::set_group_lock_method, &info_struct, l::_1);
	set_int8_funcs["group_solo_autolock"] = l::bind(&InfoStruct::set_group_solo_autolock, &info_struct, l::_1);
	set_int8_funcs["group_auto_loot_method"] = l::bind(&InfoStruct::set_group_auto_loot_method, &info_struct, l::_1);
	set_int8_funcs["assist_auto_attack"] = l::bind(&InfoStruct::set_assist_auto_attack, &info_struct, l::_1);
	
	set_string_funcs["action_state"] = l::bind(&InfoStruct::set_action_state, &info_struct, l::_1);
	set_string_funcs["combat_action_state"] = l::bind(&InfoStruct::set_combat_action_state, &info_struct, l::_1);

	set_float_funcs["max_spell_reduction"] = l::bind(&InfoStruct::set_max_spell_reduction, &info_struct, l::_1);
	set_int8_funcs["max_spell_reduction_override"] = l::bind(&InfoStruct::set_max_spell_reduction_override, &info_struct, l::_1);
}

bool Entity::HasMoved(bool include_heading){
	if(GetX() == last_x && GetY() == last_y && GetZ() == last_z && ((!include_heading) || (include_heading && GetHeading() == last_heading)))
		return false;
	bool ret_val = true;
	if(last_x == -1 && last_y == -1 && last_z == -1 && last_heading == -1){
		ret_val = false;
	}
	last_x = GetX();
	last_y = GetY();
	last_z = GetZ();
	last_heading = GetHeading();
	return ret_val;
}

int16 Entity::GetStr(){
	return GetInfoStruct()->get_str();
}

int16 Entity::GetSta(){
	return GetInfoStruct()->get_sta();
}

int16 Entity::GetInt(){
	return GetInfoStruct()->get_intel();
}

int16 Entity::GetWis(){
	return GetInfoStruct()->get_wis();
}

int16 Entity::GetAgi(){
	return GetInfoStruct()->get_agi();
}

int16 Entity::GetPrimaryStat(){
	int8 base_class = classes.GetBaseClass(GetAdventureClass());
	if (base_class == FIGHTER) 
		return GetInfoStruct()->get_str();	
	else if (base_class == PRIEST) 
		return GetInfoStruct()->get_wis();
	else if (base_class == MAGE) 
		return GetInfoStruct()->get_intel();
	else
		return GetInfoStruct()->get_agi();
}

int16 Entity::GetHeatResistance(){
	return GetInfoStruct()->get_heat();
}

int16 Entity::GetColdResistance(){
	return GetInfoStruct()->get_cold();
}

int16 Entity::GetMagicResistance(){
	return GetInfoStruct()->get_magic();
}

int16 Entity::GetMentalResistance(){
	return GetInfoStruct()->get_mental();
}

int16 Entity::GetDivineResistance(){
	return GetInfoStruct()->get_divine();
}

int16 Entity::GetDiseaseResistance(){
	return GetInfoStruct()->get_disease();
}

int16 Entity::GetPoisonResistance(){
	return GetInfoStruct()->get_poison();
}

int8 Entity::GetConcentrationCurrent() {
	return GetInfoStruct()->get_cur_concentration();
}

int8 Entity::GetConcentrationMax() {
	return GetInfoStruct()->get_max_concentration();
}

int16 Entity::GetStrBase(){
	return GetInfoStruct()->get_str_base();
}

int16 Entity::GetStaBase(){
	return GetInfoStruct()->get_sta_base();
}

int16 Entity::GetIntBase(){
	return GetInfoStruct()->get_intel_base();
}

int16 Entity::GetWisBase(){
	return GetInfoStruct()->get_wis_base();
}

int16 Entity::GetAgiBase(){
	return GetInfoStruct()->get_agi_base();
}

int16 Entity::GetHeatResistanceBase(){
	return GetInfoStruct()->get_heat_base();
}

int16 Entity::GetColdResistanceBase(){
	return GetInfoStruct()->get_cold_base();
}

int16 Entity::GetMagicResistanceBase(){
	return GetInfoStruct()->get_magic_base();
}

int16 Entity::GetMentalResistanceBase(){
	return GetInfoStruct()->get_mental_base();
}

int16 Entity::GetDivineResistanceBase(){
	return GetInfoStruct()->get_divine_base();
}

int16 Entity::GetDiseaseResistanceBase(){
	return GetInfoStruct()->get_disease_base();
}

int16 Entity::GetPoisonResistanceBase(){
	return GetInfoStruct()->get_poison_base();
}

sint8 Entity::GetAlignment(){
	return GetInfoStruct()->get_alignment();
}

bool Entity::IsCasting(){
	return casting;
}

void Entity::IsCasting(bool val){
	casting = val;
}

int32 Entity::GetRangeLastAttackTime(){
	return GetInfoStruct()->get_range_last_attack_time();
}

void Entity::SetRangeLastAttackTime(int32 time){
	GetInfoStruct()->set_range_last_attack_time(time);
}

int16 Entity::GetRangeAttackDelay(){
	return GetInfoStruct()->get_ranged_attack_delay();
//	if(IsPlayer()){
//		Item* item = ((Player*)this)->GetEquipmentList()->GetItem(EQ2_RANGE_SLOT);
//		if(item && item->IsRanged())
//			return item->ranged_info->weapon_info.delay*100;
//	}
//	return 3000;
}

int32 Entity::GetPrimaryLastAttackTime(){
	return GetInfoStruct()->get_primary_last_attack_time();
}

int16 Entity::GetPrimaryAttackDelay(){
	return GetInfoStruct()->get_primary_attack_delay();
}

void Entity::SetPrimaryAttackDelay(int16 new_delay){
	GetInfoStruct()->set_primary_attack_delay(new_delay);
}

void Entity::SetPrimaryLastAttackTime(int32 new_time){
	GetInfoStruct()->set_primary_last_attack_time(new_time);
}

int32 Entity::GetSecondaryLastAttackTime(){
	return GetInfoStruct()->get_secondary_last_attack_time();
}

int16 Entity::GetSecondaryAttackDelay(){
	return GetInfoStruct()->get_secondary_attack_delay();
}

void Entity::SetSecondaryAttackDelay(int16 new_delay){
	GetInfoStruct()->set_secondary_attack_delay(new_delay);
}

void Entity::SetSecondaryLastAttackTime(int32 new_time){
	GetInfoStruct()->set_secondary_last_attack_time(new_time);
}

void Entity::GetWeaponDamage(Item* item, int32* low_damage, int32* high_damage) {
	if(!low_damage || !high_damage)
		return;
	int32 selected_low_dmg = item->weapon_info->damage_low3;
	int32 selected_high_dmg = item->weapon_info->damage_high3;
	
	if(IsPlayer()) {
		float skillMultiplier = rule_manager.GetZoneRule(GetZoneID(), R_Player, LevelMasterySkillMultiplier)->GetFloat();
		if(skillMultiplier <= 0.0f) {
			skillMultiplier = 1.0f;
		}
		int32 min_level_skill = (int32)((float)item->generic_info.adventure_default_level*skillMultiplier);
		int32 rec_level_skill = (int32)((float)item->details.recommended_level*skillMultiplier);
		if(min_level_skill > rec_level_skill) {
			rec_level_skill = rec_level_skill;
		}
		
		Skill* masterySkill = ((Player*)this)->skill_list.GetSkill(item->generic_info.skill_req2);
		if(masterySkill) {
		LogWrite(PLAYER__DEBUG, 0, "Player", "Item %s has skill %s %u requirement", item->name.c_str(), masterySkill->name.data.c_str(), item->generic_info.skill_req2);
			int16 skillID = master_item_list.GetItemStatIDByName(masterySkill->name.data);
			int32 skill_chance = (int32)CalculateSkillWithBonus((char*)masterySkill->name.data.c_str(), master_item_list.GetItemStatIDByName(masterySkill->name.data), false);
			if(skill_chance >= min_level_skill && skill_chance < rec_level_skill) {
				int32 diff_skill = rec_level_skill - skill_chance;
				if(diff_skill < 1) {
					selected_low_dmg = item->weapon_info->damage_low2;
					selected_high_dmg = item->weapon_info->damage_high2;
				}
				else {
					diff_skill += 1;
					double logResult = log((double)diff_skill) / skillMultiplier;
					if(logResult > 1.0f) {
						logResult = .95f;
					}
					
					selected_low_dmg = (int32)((double)item->weapon_info->damage_low2 * (1.0 - logResult));
					if(selected_low_dmg < item->weapon_info->damage_low3) {
						selected_low_dmg = item->weapon_info->damage_low3;
					}
					selected_high_dmg = (int32)((double)item->weapon_info->damage_high2 * (1.0 - logResult));
					if(selected_high_dmg < item->weapon_info->damage_high3) {
						selected_high_dmg = item->weapon_info->damage_high3;
					}
				}
			}
			else if(skill_chance >= rec_level_skill) {
				selected_low_dmg = item->weapon_info->damage_low2;
				selected_high_dmg = item->weapon_info->damage_high2;
			}
		}
	}

	*low_damage = selected_low_dmg;
	*high_damage = selected_high_dmg;
}

void Entity::ChangePrimaryWeapon(){
	if(GetInfoStruct()->get_override_primary_weapon()) {
		return;
	}
	
	int32 str_offset_dmg = GetStrengthDamage();
	Item* item = equipment_list.GetItem(EQ2_PRIMARY_SLOT);
	if(item && item->details.item_id > 0 && item->IsWeapon()){
		int32 selected_low_dmg = item->weapon_info->damage_low3;
		int32 selected_high_dmg = item->weapon_info->damage_high3;
		GetWeaponDamage(item, &selected_low_dmg, &selected_high_dmg);
		GetInfoStruct()->set_primary_weapon_delay(item->weapon_info->delay * 100);
		GetInfoStruct()->set_primary_weapon_damage_low(selected_low_dmg + str_offset_dmg);
		GetInfoStruct()->set_primary_weapon_damage_high(selected_high_dmg + str_offset_dmg);
		GetInfoStruct()->set_primary_weapon_type(item->GetWeaponType());
		GetInfoStruct()->set_wield_type(item->weapon_info->wield_type);
	}
	else{
		int16 effective_level = GetInfoStruct()->get_effective_level();
		if ( !effective_level )
			effective_level = GetLevel();

			GetInfoStruct()->set_primary_weapon_delay(2000);		
			GetInfoStruct()->set_primary_weapon_damage_low((int32)1 + (effective_level * .2) + str_offset_dmg);
			GetInfoStruct()->set_primary_weapon_damage_high((int32)(5 + effective_level * (effective_level/5)) + str_offset_dmg);
			if(GetInfoStruct()->get_attack_type() > 0) {
				GetInfoStruct()->set_primary_weapon_type(GetInfoStruct()->get_attack_type());
			}
			else {
				GetInfoStruct()->set_primary_weapon_type(1);
			}
			GetInfoStruct()->set_wield_type(2);
	}
}

void Entity::ChangeSecondaryWeapon(){
	if(GetInfoStruct()->get_override_secondary_weapon()) {
		return;
	}
	
	int32 str_offset_dmg = GetStrengthDamage();
	
	Item* item = equipment_list.GetItem(EQ2_SECONDARY_SLOT);
	if(item && item->details.item_id > 0 && item->IsWeapon()){
		int32 selected_low_dmg = item->weapon_info->damage_low3;
		int32 selected_high_dmg = item->weapon_info->damage_high3;
		GetWeaponDamage(item, &selected_low_dmg, &selected_high_dmg);
		GetInfoStruct()->set_secondary_weapon_delay(item->weapon_info->delay * 100);
		GetInfoStruct()->set_secondary_weapon_damage_low(selected_low_dmg + str_offset_dmg);
		GetInfoStruct()->set_secondary_weapon_damage_high(selected_high_dmg + str_offset_dmg);
		GetInfoStruct()->set_secondary_weapon_type(item->GetWeaponType());
	}
	else{
		int16 effective_level = GetInfoStruct()->get_effective_level();
		if ( !effective_level )
			effective_level = GetLevel();

		GetInfoStruct()->set_secondary_weapon_delay(2000);
		GetInfoStruct()->set_secondary_weapon_damage_low((int32)(1 + effective_level * .2) + str_offset_dmg);
		GetInfoStruct()->set_secondary_weapon_damage_high((int32)(5 + effective_level * (effective_level/6)) + str_offset_dmg);
		GetInfoStruct()->set_secondary_weapon_type(1);
	}
}

void Entity::ChangeRangedWeapon(){
	if(GetInfoStruct()->get_override_ranged_weapon()) {
		return;
	}
	
	int32 str_offset_dmg = GetStrengthDamage();
	
	Item* item = equipment_list.GetItem(EQ2_RANGE_SLOT);
	if(item && item->details.item_id > 0 && item->IsRanged()){
		GetInfoStruct()->set_ranged_weapon_delay(item->ranged_info->weapon_info.delay*100);
		GetInfoStruct()->set_ranged_weapon_damage_low(item->ranged_info->weapon_info.damage_low3 + str_offset_dmg);
		GetInfoStruct()->set_ranged_weapon_damage_high(item->ranged_info->weapon_info.damage_high3 + str_offset_dmg);
		GetInfoStruct()->set_ranged_weapon_type(item->GetWeaponType());
	}
}

void Entity::UpdateWeapons() {
	ChangePrimaryWeapon();
	ChangeSecondaryWeapon();
	ChangeRangedWeapon();
}

int32 Entity::GetStrengthDamage() {
	int32 str_offset = 1;
	if(IsNPC()) {
		str_offset = rule_manager.GetZoneRule(GetZoneID(), R_Combat, StrengthNPC)->GetInt32();
		if(str_offset < 1)
			str_offset = 1;
	}
	else {
		str_offset = rule_manager.GetZoneRule(GetZoneID(), R_Combat, StrengthOther)->GetInt32();
		if(str_offset < 1)
			str_offset = 1;
		
	}
	int32 str_offset_dmg = (int32)((GetInfoStruct()->get_str() / str_offset));
	return str_offset_dmg;
}

int32 Entity::GetPrimaryWeaponMinDamage(){
	return GetInfoStruct()->get_primary_weapon_damage_low();
}

int32 Entity::GetPrimaryWeaponMaxDamage(){
	return GetInfoStruct()->get_primary_weapon_damage_high();
}

int16 Entity::GetPrimaryWeaponDelay(){
	return GetInfoStruct()->get_primary_weapon_delay();
}

int16 Entity::GetSecondaryWeaponDelay(){
	return GetInfoStruct()->get_secondary_weapon_delay();
}

int32 Entity::GetSecondaryWeaponMinDamage(){
	return GetInfoStruct()->get_secondary_weapon_damage_low();
}

int32 Entity::GetSecondaryWeaponMaxDamage(){
	return GetInfoStruct()->get_secondary_weapon_damage_high();
}

int8 Entity::GetPrimaryWeaponType(){
	return GetInfoStruct()->get_primary_weapon_type();
}

int8 Entity::GetSecondaryWeaponType(){
	return GetInfoStruct()->get_secondary_weapon_type();
}

int32 Entity::GetRangedWeaponMinDamage(){
	return GetInfoStruct()->get_ranged_weapon_damage_low();
}

int32 Entity::GetRangedWeaponMaxDamage(){
	return GetInfoStruct()->get_ranged_weapon_damage_high();
}

int8 Entity::GetRangedWeaponType(){
	return GetInfoStruct()->get_ranged_weapon_type();
}

bool Entity::IsDualWield(){
	return GetInfoStruct()->get_wield_type() == 1;
}

int8 Entity::GetWieldType(){
	return GetInfoStruct()->get_wield_type();
}

int16 Entity::GetRangeWeaponDelay(){
	return GetInfoStruct()->get_ranged_weapon_delay();
}

void Entity::SetRangeWeaponDelay(int16 new_delay){
	GetInfoStruct()->set_ranged_weapon_delay(new_delay * 100);
}
void Entity::SetRangeAttackDelay(int16 new_delay){
	GetInfoStruct()->set_ranged_attack_delay(new_delay);
}

void Entity::SetPrimaryWeaponDelay(int16 new_delay){
	GetInfoStruct()->set_primary_weapon_delay(new_delay * 100);
}

void Entity::SetSecondaryWeaponDelay(int16 new_delay){
	GetInfoStruct()->set_primary_weapon_delay(new_delay * 100);
}

bool Entity::BehindTarget(Spawn* target){
	return BehindSpawn(target, GetX(), GetZ());
}

bool Entity::FlankingTarget(Spawn* target) {
	return IsFlankingSpawn(target, GetX(), GetZ());
}

float Entity::GetDodgeChance(){
	float ret = 0;
	
	return ret;
}

bool Entity::EngagedInCombat(){
	return in_combat;
}

void Entity::InCombat(bool val){
	bool changeCombatState = false;
	if((in_combat && !val) || (!in_combat && val))
		changeCombatState = true;

	in_combat = val;
	
	bool update_regen = false;
	if(GetInfoStruct()->get_engaged_encounter()) {
		if(!IsAggroed() || !IsEngagedInEncounter()) {
			GetInfoStruct()->set_engaged_encounter(0);
			update_regen = true;
		}
	}

	if(changeCombatState || update_regen)
		SetRegenValues((GetInfoStruct()->get_effective_level() > 0) ? GetInfoStruct()->get_effective_level() : GetLevel());
}

void Entity::DoRegenUpdate(){
	if(!Alive() || GetHP() == 0)//dead
		return;
	sint32 hp = GetHP();
	sint32 power = GetPower();

	if(hp < GetTotalHP()){
		sint16 temp = GetInfoStruct()->get_hp_regen();

		if((hp + temp) > GetTotalHP())
			SetHP(GetTotalHP());
		else
			SetHP(hp + temp);
	}
	if(GetPower() < GetTotalPower()){
		sint16 temp = GetInfoStruct()->get_power_regen();
		
		if((power + temp) > GetTotalPower())
			SetPower(GetTotalPower());
		else
			SetPower(power + temp);
	}
}

void Entity::AddMaintainedSpell(LuaSpell* luaspell){
	if (!luaspell)
		return;

	Spell* spell = luaspell->spell;
	MaintainedEffects* effect = GetFreeMaintainedSpellSlot();

	if (effect){
		MMaintainedSpells.writelock(__FUNCTION__, __LINE__);
		effect->spell = luaspell;
		effect->spell_id = spell->GetSpellData()->id;
		LogWrite(NPC__SPELLS, 5, "NPC", "AddMaintainedSpell Spell ID: %u, Concentration: %u", spell->GetSpellData()->id, spell->GetSpellData()->req_concentration);
		effect->conc_used = spell->GetSpellData()->req_concentration;
		effect->total_time = spell->GetSpellDuration() / 10;
		effect->tier = spell->GetSpellData()->tier;
		if (spell->GetSpellData()->duration_until_cancel)
			effect->expire_timestamp = 0xFFFFFFFF;
		else
			effect->expire_timestamp = Timer::GetCurrentTime2() + (spell->GetSpellDuration() * 100);
		MMaintainedSpells.releasewritelock(__FUNCTION__, __LINE__);
	}
}

void Entity::AddSpellEffect(LuaSpell* luaspell, int32 override_expire_time){
	if (!luaspell || !luaspell->caster)
		return;

	Spell* spell = luaspell->spell;
	SpellEffects* old_effect = GetSpellEffect(spell->GetSpellID(), luaspell->caster);
	SpellEffects* effect = 0;
	if (old_effect){
		GetZone()->RemoveTargetFromSpell(old_effect->spell, this);
		RemoveSpellEffect(old_effect->spell);
	}
	
	LogWrite(SPELL__DEBUG, 0, "Spell", "%s AddSpellEffect %s (%u).", spell->GetName(), GetName(), GetID());
	
	if(!effect)
		effect = GetFreeSpellEffectSlot();

	if(effect){
		MSpellEffects.writelock(__FUNCTION__, __LINE__);
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
		MSpellEffects.releasewritelock(__FUNCTION__, __LINE__);
		changed = true;
		info_changed = true;
		AddChangedZoneSpawn();

		if(luaspell->caster && luaspell->caster->IsPlayer() && luaspell->caster != this)
			((Player*)luaspell->caster)->GetClient()->TriggerSpellSave();
	}
}

void Entity::RemoveMaintainedSpell(LuaSpell* luaspell){
	if (!luaspell)
		return;

	bool found = false;
	MMaintainedSpells.writelock(__FUNCTION__, __LINE__);
	for (int i = 0; i<30; i++){
		// If we already found the spell then we are bumping all other up one so there are no gaps
		// This check needs to be first so found can never be true on the first iteration (i = 0)
		if (found) {
			GetInfoStruct()->maintained_effects[i].slot_pos = i - 1;
			GetInfoStruct()->maintained_effects[i - 1] = GetInfoStruct()->maintained_effects[i];

		}
		// Compare spells, if we found a match set the found flag
		if (GetInfoStruct()->maintained_effects[i].spell == luaspell)
			found = true;

	}
	// if we found the spell in the array then we need to set the last element to empty
	if (found) {
		memset(&GetInfoStruct()->maintained_effects[29], 0, sizeof(MaintainedEffects));
		GetInfoStruct()->maintained_effects[29].spell_id = 0xFFFFFFFF;
		GetInfoStruct()->maintained_effects[29].icon = 0xFFFF;
		GetInfoStruct()->maintained_effects[29].spell = nullptr;
	}
	MMaintainedSpells.releasewritelock(__FUNCTION__, __LINE__);
}

void Entity::RemoveSpellEffect(LuaSpell* spell) {
	bool found = false;
	MSpellEffects.writelock(__FUNCTION__, __LINE__);
	for(int i=0;i<45;i++) {
		if (found) {
			GetInfoStruct()->spell_effects[i-1] = GetInfoStruct()->spell_effects[i];
		}
		if (GetInfoStruct()->spell_effects[i].spell == spell)
			found = true;
	}
	if (found) {
		LogWrite(SPELL__DEBUG, 0, "Spell", "%s RemoveSpellEffect %s (%u).", spell->spell->GetName(), GetName(), GetID());
		GetZone()->GetSpellProcess()->RemoveTargetFromSpell(spell, this);
		memset(&GetInfoStruct()->spell_effects[44], 0, sizeof(SpellEffects));
		GetInfoStruct()->spell_effects[44].spell_id = 0xFFFFFFFF;
		GetInfoStruct()->spell_effects[44].spell = nullptr;
		changed = true;
		info_changed = true;
		AddChangedZoneSpawn();
		
		if(IsPlayer()) {
			((Player*)this)->SetCharSheetChanged(true);
		}
	}

	MSpellEffects.releasewritelock(__FUNCTION__, __LINE__);
}

MaintainedEffects* Entity::GetFreeMaintainedSpellSlot(){
	MaintainedEffects* ret = 0;
	InfoStruct* info = GetInfoStruct();
	MMaintainedSpells.readlock(__FUNCTION__, __LINE__);
	for (int i = 0; i<NUM_MAINTAINED_EFFECTS; i++){
		if (info->maintained_effects[i].spell_id == 0xFFFFFFFF){
			ret = &info->maintained_effects[i];
			ret->spell_id = 0;
			ret->slot_pos = i;
			break;
		}
	}
	MMaintainedSpells.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

MaintainedEffects* Entity::GetMaintainedSpell(int32 spell_id){
	MaintainedEffects* ret = 0;
	InfoStruct* info = GetInfoStruct();
	MMaintainedSpells.readlock(__FUNCTION__, __LINE__);
	for (int i = 0; i<NUM_MAINTAINED_EFFECTS; i++){
		if (info->maintained_effects[i].spell_id == spell_id){
			ret = &info->maintained_effects[i];
			break;
		}
	}
	MMaintainedSpells.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

SpellEffects* Entity::GetFreeSpellEffectSlot(){
	SpellEffects* ret = 0;
	InfoStruct* info = GetInfoStruct();
	MSpellEffects.readlock(__FUNCTION__, __LINE__);
	for(int i=0;i<45;i++){
		if(info->spell_effects[i].spell_id == 0xFFFFFFFF){
			ret = &info->spell_effects[i];
			ret->spell_id = 0;
			break;
		}
	}
	MSpellEffects.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

SpellEffects* Entity::GetSpellEffect(int32 id, Entity* caster) {
	SpellEffects* ret = 0;
	InfoStruct* info = GetInfoStruct();
	MSpellEffects.readlock(__FUNCTION__, __LINE__);
	for(int i = 0; i < 45; i++) {
		if(info->spell_effects[i].spell_id == id) {
			if (!caster || info->spell_effects[i].caster == caster){
				ret = &info->spell_effects[i];
				break;
			}
		}
	}
	MSpellEffects.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

SpellEffects* Entity::GetSpellEffectBySpellType(int8 spell_type) {
	SpellEffects* ret = 0;
	InfoStruct* info = GetInfoStruct();
	MSpellEffects.readlock(__FUNCTION__, __LINE__);
	for(int i = 0; i < 45; i++) {
		if(info->spell_effects[i].spell_id != 0xFFFFFFFF && info->spell_effects[i].spell->spell->GetSpellData()->spell_type == spell_type) {
			ret = &info->spell_effects[i];
			break;
		}
	}
	MSpellEffects.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

SpellEffects* Entity::GetSpellEffectWithLinkedTimer(int32 id, int32 linked_timer, sint32 type_group_spell_id, Entity* caster) {
	SpellEffects* ret = 0;
	InfoStruct* info = GetInfoStruct();
	MSpellEffects.readlock(__FUNCTION__, __LINE__);
	for(int i = 0; i < 45; i++) {
		if(info->spell_effects[i].spell_id != 0xFFFFFFFF)
		{
			if(  (info->spell_effects[i].spell_id == id && linked_timer == 0 && type_group_spell_id == 0) ||
				 (linked_timer > 0 && info->spell_effects[i].spell->spell->GetSpellData()->linked_timer == linked_timer) ||
				(type_group_spell_id > 0 && info->spell_effects[i].spell->spell->GetSpellData()->type_group_spell_id == type_group_spell_id))
			{
				if (type_group_spell_id >= -1 && (!caster || info->spell_effects[i].caster == caster)){
					ret = &info->spell_effects[i];
					break;
				}
			}
		}
	}
	MSpellEffects.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

LuaSpell* Entity::HasLinkedTimerID(LuaSpell* spell, Spawn* target, bool stackWithOtherPlayers) {
	if(!spell->spell->GetSpellData()->linked_timer && !spell->spell->GetSpellData()->type_group_spell_id)
		return nullptr;
	LuaSpell* ret = nullptr;
	InfoStruct* info = GetInfoStruct();
	MSpellEffects.readlock(__FUNCTION__, __LINE__);
	//this for loop primarily handles self checks and 'friendly' checks
	for(int i = 0; i < NUM_MAINTAINED_EFFECTS; i++) {
			if(info->maintained_effects[i].spell_id != 0xFFFFFFFF)
			{
				if( ((info->maintained_effects[i].spell_id == spell->spell->GetSpellID() && spell->spell->GetSpellData()->type_group_spell_id >= 0) ||
					(info->maintained_effects[i].spell->spell->GetSpellData()->linked_timer > 0 && info->maintained_effects[i].spell->spell->GetSpellData()->linked_timer == spell->spell->GetSpellData()->linked_timer) || 
					(spell->spell->GetSpellData()->type_group_spell_id > 0 && spell->spell->GetSpellData()->type_group_spell_id == info->maintained_effects[i].spell->spell->GetSpellData()->type_group_spell_id)) && 
					((spell->spell->GetSpellData()->friendly_spell) || 
					(!spell->spell->GetSpellData()->friendly_spell && spell->spell->GetSpellData()->type_group_spell_id >= -1 && spell->caster == info->maintained_effects[i].spell->caster) ) &&
					(target == nullptr || info->maintained_effects[i].spell->initial_target == target->GetID())) {
					ret = info->maintained_effects[i].spell;
					break;
				}
			}
	}
	MSpellEffects.releasereadlock(__FUNCTION__, __LINE__);

	if(!ret && !stackWithOtherPlayers && target && target->IsEntity())
	{
		SpellEffects* effect = ((Entity*)target)->GetSpellEffectWithLinkedTimer(spell->spell->GetSpellID(), spell->spell->GetSpellData()->linked_timer, spell->spell->GetSpellData()->type_group_spell_id, nullptr);
		if(effect)
			ret = effect->spell;
	}
	
	return ret;
}

InfoStruct* Entity::GetInfoStruct(){ 
	return &info_struct; 
}

Skill* Entity::GetSkillByName(const char* name, bool check_update){
	// NPC::GetSkillByName in NPC.cpp exists for NPC's
	// Player::GetSkillByName in Player.cpp exists for Player's
	return 0;
}

Skill* Entity::GetSkillByID(int32 id, bool check_update){
	// NPC::GetSkillByID in NPC.cpp exists for NPC's
	// Player::GetSkillByID in Player.cpp exists for Player's
	return 0;
}

float Entity::GetMaxSpeed(){
	return max_speed;
}

void Entity::SetMaxSpeed(float val){
	max_speed = val;
}

float Entity::CalculateSkillStatChance(char* skillName, int16 item_stat, float max_cap, float modifier, bool add_to_skill)
{
	float skillAndItemsChance = 0.0f;
	float maxBonusCap = (float)GetLevel()*rule_manager.GetZoneRule(GetZoneID(), R_Combat, MaxSkillBonusByLevel)->GetFloat();
	Skill* skill = GetSkillByName(skillName, false);
	if(skill){
		MStats.lock();
		float item_chance_or_skill = stats[item_stat];
		MStats.unlock();
		if(item_chance_or_skill > maxBonusCap) {
			item_chance_or_skill = maxBonusCap;
		}
		
		if(add_to_skill)
		{
			skillAndItemsChance = (((float)skill->current_val+item_chance_or_skill)/10.0f); // do we know 25 is accurate?  10 gives more 'skill' space, most cap at 70% with items
		}
		else
		{
			skillAndItemsChance = ((float)skill->current_val/10.0f); // do we know 25 is accurate?  10 gives more 'skill' space, most cap at 70% with items

			if(modifier > maxBonusCap) {
				modifier = maxBonusCap;
			}
			// take chance percentage and add the item stats % (+1 = 1% or .01f)
			skillAndItemsChance += (skillAndItemsChance*((item_chance_or_skill + modifier)/100.0f));
		}
	}

	if ( max_cap > 0.0f && skillAndItemsChance > max_cap )
		skillAndItemsChance = max_cap;

	return skillAndItemsChance;
}

float Entity::CalculateSkillWithBonus(char* skillName, int16 item_stat, bool chance_skill_increase)
{
	float skillAndItemsChance = 0.0f;
	float maxBonusCap = GetRuleSkillMaxBonus();
	
	Skill* skill = GetSkillByName(skillName, chance_skill_increase);
	if(skill){
		float item_chance_or_skill = 0.0f;
		if(item_stat != 0xFFFF) {
			MStats.lock();
			item_chance_or_skill = stats[item_stat];
			MStats.unlock();
		}
		if(item_chance_or_skill > maxBonusCap)  { // would we be using their effective mentored level or actual level?
			item_chance_or_skill = maxBonusCap;
		}
		skillAndItemsChance = skill->current_val+item_chance_or_skill;
	}

	return skillAndItemsChance;
}

float Entity::GetRuleSkillMaxBonus() {
	return (float)GetLevel()*rule_manager.GetZoneRule(GetZoneID(), R_Combat, MaxSkillBonusByLevel)->GetFloat();
}

void Entity::CalculateBonuses(){
	if(lua_interface->IsLuaSystemReloading())
		return;

	InfoStruct* info = &info_struct;

	int16 effective_level = info->get_effective_level() != 0 ? info->get_effective_level() : GetLevel();

	CalculateMaxReduction();
	
	info->set_block(info->get_block_base());
	
	info->set_cur_attack(info->get_attack_base());
	info->set_base_avoidance_pct(info->get_avoidance_base());

	info->set_disease(info->get_disease_base());
	info->set_divine(info->get_divine_base());
	info->set_heat(info->get_heat_base());
	info->set_magic(info->get_magic_base());
	info->set_mental(info->get_mental_base());
	info->set_cold(info->get_cold_base());
	info->set_poison(info->get_poison_base());
	info->set_elemental_base(info->get_heat());
	info->set_noxious_base(info->get_poison());
	info->set_arcane_base(info->get_magic());

	info->set_sta(info->get_sta_base());
	info->set_agi(info->get_agi_base());
	info->set_str(info->get_str_base());
	info->set_wis(info->get_wis_base());
	info->set_intel(info->get_intel_base());
	info->set_ability_modifier(0);
	info->set_critical_mitigation(0);

	info->set_block_chance(0);
	info->set_crit_chance(0);
	info->set_crit_bonus(0);
	info->set_potency(0);
	info->set_hate_mod(0);
	info->set_reuse_speed(0);
	info->set_casting_speed(0);
	info->set_recovery_speed(0);
	info->set_spell_reuse_speed(0);
	info->set_spell_multi_attack(0);
	info->set_dps(0);
	info->set_dps_multiplier(0);
	info->set_haste(0);
	info->set_attackspeed(0);
	info->set_multi_attack(0);
	info->set_flurry(0);
	info->set_melee_ae(0);

	info->set_strikethrough(0);

	info->set_accuracy(0);

	info->set_offensivespeed(0);

	MStats.lock();
	stats.clear();
	MStats.unlock();
		
	ItemStatsValues* values = equipment_list.CalculateEquipmentBonuses(this);
	CalculateSpellBonuses(values);
	
	info->set_cur_mitigation(info->get_mitigation_base());
	
	int32 calc_mit_cap = effective_level * rule_manager.GetZoneRule(GetZoneID(), R_Combat, CalculatedMitigationCapLevel)->GetInt32();
	info->set_max_mitigation(calc_mit_cap);
	
	int16 mit_percent = (int16)(CalculateMitigation() * 1000.0f);
	info->set_mitigation_pve(mit_percent);
	mit_percent = (int16)(CalculateMitigation(DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE,0,0,true) * 1000.0f);
	info->set_mitigation_pvp(mit_percent);
	
	info->add_sta((float)values->sta);
	info->add_str((float)values->str);
	info->add_agi((float)values->agi);
	info->add_wis((float)values->wis);
	info->add_intel((float)values->int_);

	info->add_disease(values->vs_disease);
	info->add_divine(values->vs_divine);
	info->add_heat(values->vs_heat);
	info->add_magic(values->vs_magic);
	int32 sta_hp_bonus = 0.0;
	int32 prim_power_bonus = 0.0;
	float bonus_mod = 0.0;
	if (IsPlayer()) {
		bonus_mod = CalculateBonusMod(); 
		sta_hp_bonus = info->get_sta() * bonus_mod;
		prim_power_bonus = GetPrimaryStat() * bonus_mod;
	}
	prim_power_bonus = floor(float(prim_power_bonus));
	sta_hp_bonus = floor(float(sta_hp_bonus));
	SetTotalHP(GetTotalHPBaseInstance() + values->health + sta_hp_bonus);
	SetTotalPower(GetTotalPowerBaseInstance() + values->power + prim_power_bonus);
	if(GetHP() > GetTotalHP())
		SetHP(GetTotalHP());
	if(GetPower() > GetTotalPower())
		SetPower(GetTotalPower());

	info->add_mental(values->vs_mental);

	info->add_poison(values->vs_poison);

	info->set_max_concentration(info->get_max_concentration_base() + values->concentration);

	info->add_cold(values->vs_cold);

	info->add_mitigation_skill1(values->vs_slash);
	info->add_mitigation_skill2(values->vs_pierce);
	info->add_mitigation_skill3(values->vs_crush);
	info->add_ability_modifier(values->ability_modifier);
	info->add_critical_mitigation(values->criticalmitigation);
	info->add_block_chance(values->extrashieldblockchance);
	info->add_crit_chance(values->beneficialcritchance);
	info->add_crit_bonus(values->critbonus);
	info->add_potency(values->potency);
	info->add_hate_mod(values->hategainmod);
	info->add_reuse_speed(values->abilityreusespeed);
	info->add_casting_speed(values->abilitycastingspeed);
	info->add_recovery_speed(values->abilityrecoveryspeed);
	info->add_spell_reuse_speed(values->spellreusespeed);
	info->add_spell_multi_attack(values->spellmultiattackchance);
	info->add_dps(values->dps);
	info->add_dps_multiplier(CalculateDPSMultiplier());
	info->add_haste(values->attackspeed);
	info->add_multi_attack(values->multiattackchance);
	info->add_flurry(values->flurry);
	info->add_melee_ae(values->aeautoattackchance);
	info->add_strikethrough(values->strikethrough);
	info->add_accuracy(values->accuracy);
	info->add_offensivespeed(values->offensivespeed);
	info->add_uncontested_block(values->uncontested_block);
	info->add_uncontested_parry(values->uncontested_parry);
	info->add_uncontested_dodge(values->uncontested_dodge);
	info->add_uncontested_riposte(values->uncontested_riposte);

	info->set_ability_modifier(values->ability_modifier);
	
	float full_pct_hit = 100.0f;

	MStats.lock();
	float parryStat = stats[ITEM_STAT_PARRY];
	MStats.unlock();
	float parry_pct = CalculateSkillStatChance("Parry", ITEM_STAT_PARRYCHANCE, 70.0f, parryStat);
	parry_pct += parry_pct * (info->get_cur_avoidance()/100.0f);
	if(parry_pct > 70.0f)
		parry_pct = 70.0f;

	info->set_parry(parry_pct);

	full_pct_hit -= parry_pct;
	
	float block_pct = 0.0f;

	if(GetAdventureClass() != BRAWLER)
	{
		Item* item = equipment_list.GetItem(EQ2_SECONDARY_SLOT);
		if(item && item->details.item_id > 0 && item->IsShield()){
			// if high is set and greater than low use high, otherwise use low
			int16 mitigation = item->armor_info->mitigation_high > item->armor_info->mitigation_low ? item->armor_info->mitigation_high : item->armor_info->mitigation_low;
			// we frankly don't know the formula for Block, only that it uses the 'Protection' of the shield, which is the mitigation_low/mitigation_high in the armor_info
			if(mitigation)
			{
				/*DOF Prima Guide: Shields now have the following base chances
				to block: Tower (10%), Kite (10%), Round
				(5%), Buckler (3%). Your chances to block
				scale up or down based on the con of your
				opponent.*/
				Skill* skill = master_skill_list.GetSkill(item->generic_info.skill_req1);
				float baseBlock = 0.0f;
				if(skill)
				{
					if(skill->short_name.data == "towershield" || skill->short_name.data == "kiteshield")
						baseBlock = 10.0f;
					else if (skill->short_name.data == "roundshield")
						baseBlock = 5.0f;
					else if (skill->short_name.data == "buckler") 
						baseBlock = 3.0f;
				}
				if(effective_level > mitigation)
					block_pct = log10f((float)mitigation/((float)effective_level*10.0f));
				else
					block_pct = log10f(((float)effective_level/(float)mitigation)) * log10f(effective_level) * 2.0f;
				
				if(block_pct < 0.0f)
					block_pct *= -1.0f;

				block_pct += baseBlock;

				block_pct += block_pct * (info->get_cur_avoidance()/100.0f);
				if(block_pct > 70.0f)
					block_pct = 70.0f;
			}
		}
	}
	else
	{
		MStats.lock();
		float deflectionStat = stats[ITEM_STAT_DEFLECTION];
		MStats.unlock();
		block_pct = CalculateSkillStatChance("Deflection", ITEM_STAT_MINIMUMDEFLECTIONCHANCE, 70.0f, deflectionStat+1.0f);
		block_pct += block_pct * (info->get_cur_avoidance()/100.0f);
	}

	float block_actual = 0.0f;
	if(full_pct_hit > 0.0f)
		block_actual = block_pct * (full_pct_hit / 100.0f);

	info->set_block(block_actual);
	full_pct_hit -= block_actual;

	MStats.lock();
	float defenseStat = stats[ITEM_STAT_DEFENSE];
	float baseAvoidanceStat = stats[ITEM_STAT_BASEAVOIDANCEBONUS];
	MStats.unlock();
	
	float dodge_pct = (baseAvoidanceStat/100.0f) + CalculateSkillStatChance("Defense", ITEM_STAT_DODGECHANCE, 100.0f, defenseStat);
	dodge_pct += dodge_pct * (info->get_cur_avoidance()/100.0f);

	float dodge_actual = dodge_pct * (full_pct_hit / 100.0f) + CalculateLevelStatBonus(GetAgi());

	info->set_avoidance_base(dodge_actual);

	float total_avoidance = parry_pct + block_actual + dodge_actual;
	info->set_avoidance_display(total_avoidance);

	SetRegenValues(effective_level);
	
	CalculateApplyWeight();
	
	UpdateWeapons();
	
	safe_delete(values);
}

float Entity::CalculateLevelStatBonus(int16 stat_value) {
	int16 effective_level = GetInfoStruct()->get_effective_level() != 0 ? GetInfoStruct()->get_effective_level() : GetLevel();
	float result = (log10f(effective_level * stat_value) / 50.0f); // todo: break this down by stat type and give independent modifiers
	return result;
}

void Entity::CalculateApplyWeight() {
	if (IsPlayer()) {
		int32 prev_weight = GetInfoStruct()->get_weight();
		int32 inv_weight = ((Player*)this)->item_list.GetWeight();
		
		// calculate coin
		int32 coin_copper = GetInfoStruct()->get_coin_copper();
		int32 coin_silver = GetInfoStruct()->get_coin_silver();
		int32 coin_gold = GetInfoStruct()->get_coin_gold();
		int32 coin_plat = GetInfoStruct()->get_coin_plat();
		
		float weight_per_stone = rule_manager.GetZoneRule(GetZoneID(), R_Player, CoinWeightPerStone)->GetFloat();
		if(weight_per_stone < 0.0f) {
			weight_per_stone = 0.0f;
		}
		
		double weight_copper = ((double)coin_copper / weight_per_stone);
		double weight_silver = ((double)coin_silver / weight_per_stone);
		double weight_gold = ((double)coin_gold / weight_per_stone);
		double weight_platinum = ((double)coin_plat / weight_per_stone);
		int32 total_weight = (int32)(weight_copper + weight_silver + weight_gold + weight_platinum);
		LogWrite(PLAYER__DEBUG, 0, "Debug", "Coin Weight Calculated to: %u.  Weight_Copper: %f, Weight_Silver: %f, Weight_Gold: %f, Weight_Platinum: %f", total_weight, weight_copper, weight_silver, weight_gold, weight_platinum);
		
		total_weight += (int32)((double)inv_weight / 10.0);
		
		GetInfoStruct()->set_weight(total_weight);
		
		SetSpeedMultiplier(GetHighestSnare());
		((Player*)this)->SetSpeed(GetSpeed());
		if(((Player*)this)->GetClient()) {
			((Player*)this)->GetClient()->SendControlGhost();
		}
		info_changed = true;
		changed = true;
		AddChangedZoneSpawn();
		((Player*)this)->SetCharSheetChanged(true);
	}
	int32 max_weight = 0;
	float weight_str_multiplier = rule_manager.GetZoneRule(GetZoneID(), R_Player, MaxWeightStrengthMultiplier)->GetFloat();
	int32 base_weight = rule_manager.GetZoneRule(GetZoneID(), R_Player, BaseWeight)->GetInt32();
	if(weight_str_multiplier < 0.0f) {
		weight_str_multiplier = 0.0f;
	}
	
	if(GetInfoStruct()->get_str() <= 0.0f) {
		max_weight = base_weight; // rule for base strength
	}
	else {
		max_weight = (int32)((double)GetInfoStruct()->get_str() * weight_str_multiplier); // rule multipler for strength
		max_weight += base_weight; // rule for base strength
	}
	GetInfoStruct()->set_max_weight(max_weight);
}

void Entity::SetRegenValues(int16 effective_level)
{
	bool classicRegen = rule_manager.GetZoneRule(GetZoneID(), R_Spawn, ClassicRegen)->GetBool();
	bool override_ = (IsPlayer() && !GetInfoStruct()->get_engaged_encounter());
	
	if(!GetInfoStruct()->get_hp_regen_override())
	{
		sint16 regen_hp_rate = 0;
		sint16 temp = 0;

		MStats.lock();
		
		if(!IsAggroed() || override_)
		{
			if(classicRegen)
			{
				// classic regen only gives OUT OF COMBAT, doesn't combine in+out of combat
				regen_hp_rate = (int)(effective_level*.75)+1;
				temp = regen_hp_rate + stats[ITEM_STAT_HPREGEN];
				temp += stats[ITEM_STAT_HPREGENPPT];
			}
			else
			{
				regen_hp_rate = (int)(effective_level*.75)+(int)(effective_level/10) + 1;
				temp = regen_hp_rate + stats[ITEM_STAT_HPREGEN];
				temp += stats[ITEM_STAT_HPREGENPPT] + stats[ITEM_STAT_COMBATHPREGENPPT];
			}
		}
		else
		{
			regen_hp_rate = (sint16)(effective_level / 10) + 1;
			temp = regen_hp_rate + stats[ITEM_STAT_COMBATHPREGENPPT];
		}
		MStats.unlock();

		GetInfoStruct()->set_hp_regen(temp);
	}

	if(!GetInfoStruct()->get_power_regen_override())
	{
		sint16 regen_power_rate = 0;
		sint16 temp = 0;

		MStats.lock();
		if(!IsAggroed() || override_)
		{
			if(classicRegen)
			{				
				regen_power_rate = effective_level + 1;
				temp = regen_power_rate + stats[ITEM_STAT_MANAREGEN];
				temp += stats[ITEM_STAT_MPREGENPPT];
			}
			else
			{
				regen_power_rate = effective_level + (int)(effective_level/10) + 1;
				temp = regen_power_rate + stats[ITEM_STAT_MANAREGEN];
				temp += stats[ITEM_STAT_MPREGENPPT] + stats[ITEM_STAT_COMBATMPREGENPPT];
			}
		}
		else
		{
			regen_power_rate = (sint16)(effective_level / 10) + 1;
			temp = regen_power_rate + stats[ITEM_STAT_COMBATMPREGENPPT];
		}
		MStats.unlock();

		GetInfoStruct()->set_power_regen(temp);
	}
}

EquipmentItemList* Entity::GetEquipmentList(){
	return &equipment_list;
}

EquipmentItemList* Entity::GetAppearanceEquipmentList(){
	return &appearance_equipment_list;
}

void Entity::SetEquipment(Item* item, int8 slot){
	std::lock_guard<std::mutex> lk(MEquipment);
	if(!item && slot < NUM_SLOTS){
		SetInfo(&equipment.equip_id[slot], 0);
		SetInfo(&equipment.color[slot].red, 0);
		SetInfo(&equipment.color[slot].green, 0);
		SetInfo(&equipment.color[slot].blue, 0);
		SetInfo(&equipment.highlight[slot].red, 0);
		SetInfo(&equipment.highlight[slot].green, 0);
		SetInfo(&equipment.highlight[slot].blue, 0);
	}
	else{
		if ( slot >= NUM_SLOTS ) 
			slot = item->details.slot_id;

		if( slot >= NUM_SLOTS )
			return;
		
		SetInfo(&equipment.equip_id[slot], item->generic_info.appearance_id);
		SetInfo(&equipment.color[slot].red, item->generic_info.appearance_red);
		SetInfo(&equipment.color[slot].green, item->generic_info.appearance_green);
		SetInfo(&equipment.color[slot].blue, item->generic_info.appearance_blue);
		SetInfo(&equipment.highlight[slot].red, item->generic_info.appearance_highlight_red);
		SetInfo(&equipment.highlight[slot].green, item->generic_info.appearance_highlight_green);
		SetInfo(&equipment.highlight[slot].blue, item->generic_info.appearance_highlight_blue);
	}
}

bool Entity::CheckSpellBonusRemoval(LuaSpell* spell, int16 type){
	MutexList<BonusValues*>::iterator itr = bonus_list.begin();
	while(itr.Next()){
		if(itr.value->luaspell == spell && itr.value->type == type){
			bonus_list.Remove(itr.value, true);
			return true;
		}
	}
	return false;
}

void Entity::AddSpellBonus(LuaSpell* spell, int16 type, float value, int64 class_req, vector<int16> race_req, vector<int16> faction_req){
	CheckSpellBonusRemoval(spell, type); 
	BonusValues* bonus = new BonusValues;
	if(spell && spell->spell) {
		bonus->spell_id = spell->spell->GetSpellID();
	}
	else {
		bonus->spell_id = 0;
	}
	bonus->luaspell = spell;
	bonus->type = type;
	bonus->value = value;
	bonus->class_req = class_req;
	bonus->race_req = race_req;
	bonus->faction_req = faction_req;
	bonus->tier = (spell && spell->spell) ? spell->spell->GetSpellTier() : 0;
	bonus_list.Add(bonus);

	if(IsNPC() || IsPlayer())
		CalculateBonuses();
}

BonusValues* Entity::GetSpellBonus(int32 spell_id) {
	BonusValues *ret = 0;
	MutexList<BonusValues*>::iterator itr = bonus_list.begin();
	while (itr.Next()) {
		if (itr.value->spell_id == spell_id) {
			ret = itr.value;
			break;
		}
	}

	return ret;
}

vector<BonusValues*>* Entity::GetAllSpellBonuses(LuaSpell* spell) {
	vector<BonusValues*>* list = new vector<BonusValues*>;
	MutexList<BonusValues*>::iterator itr = bonus_list.begin();
	while (itr.Next()) {
		if (itr.value->luaspell == spell)
			list->push_back(itr.value);
	}
	return list;
}

void Entity::RemoveSpellBonus(const LuaSpell* spell, bool remove_all){
	// spell can be null!
	MutexList<BonusValues*>::iterator itr = bonus_list.begin();
	while(itr.Next()){
		if(itr.value->luaspell == spell || remove_all)
		bonus_list.Remove(itr.value, true);
	}
	
	if(IsNPC() || IsPlayer())
		CalculateBonuses();
}

void Entity::CalculateSpellBonuses(ItemStatsValues* stats){
	if(stats){
		MutexList<BonusValues*>::iterator itr = bonus_list.begin();
		vector<BonusValues*> bv;
		//First check if we meet the requirement for each bonus
		bool race_match = false;
		while(itr.Next()) {
			if (itr.value->race_req.size() > 0) {
				for (int8 i = 0; i < itr.value->race_req.size(); i++) {
					if (GetRace() == itr.value->race_req[i]) {
						race_match = true;
					}
				}
			}
			else
				race_match = true; // if the race_req.size = 0 then there is no race requirement and the race_match will be true
			int64 const class1 = pow(2.0, (GetAdventureClass() - 1));
			int64 const class2 = pow(2.0, (classes.GetSecondaryBaseClass(GetAdventureClass()) - 1));
			int64 const class3 = pow(2.0, (classes.GetBaseClass(GetAdventureClass()) - 1));
			if (itr.value->class_req == 0 || (itr.value->class_req & class1) == class1 || (itr.value->class_req & class2) == class2 || (itr.value->class_req & class3) == class3 && race_match )
				bv.push_back(itr.value);
		}
		//Sort the bonuses by spell id and luaspell
		BonusValues* bonus = nullptr;
		map <int32, map<LuaSpell*, vector<BonusValues*> > > sort;
		for (int8 i = 0; i < bv.size(); i++){
			bonus = bv.at(i);
			sort[bonus->spell_id][bonus->luaspell].push_back(bonus);
		}
		//Now check for the highest tier of each spell id and apply those bonuses
		map<LuaSpell*, vector<BonusValues*> >::iterator tier_itr;
		map <int32, map<LuaSpell*, vector<BonusValues*> > >::iterator sort_itr;
		for (sort_itr = sort.begin(); sort_itr != sort.end(); sort_itr++){
			LuaSpell* key = nullptr;
			sint8 highest_tier = -1;
			//Find the highest tier for this spell id
			for (tier_itr = sort_itr->second.begin(); tier_itr != sort_itr->second.end(); tier_itr++){
				LuaSpell* current_spell = tier_itr->first;
				sint8 current_tier = 0;
				if (current_spell && current_spell->spell && ((current_tier = current_spell->spell->GetSpellTier()) > highest_tier)) {
					highest_tier = current_tier;
					key = current_spell;
				}
			}
			//We've found the highest tier for this spell id, so add the bonuses
			vector<BonusValues*>* final_bonuses = &sort_itr->second[key];
			for (int8 i = 0; i < final_bonuses->size(); i++)
				world.AddBonuses(nullptr, stats, final_bonuses->at(i)->type, final_bonuses->at(i)->value, this);
		}
	}
}

void Entity::AddMezSpell(LuaSpell* spell) {
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_MEZ])
		control_effects[CONTROL_EFFECT_TYPE_MEZ] = new MutexList<LuaSpell*>;

	MutexList<LuaSpell*>* mez_spells = control_effects[CONTROL_EFFECT_TYPE_MEZ];

	if (IsPlayer() && !IsStunned() && !IsMezImmune() && mez_spells->size(true) == 0){
		((Player*)this)->SetPlayerControlFlag(1, 16, true);
		if (!IsRooted())
			((Player*)this)->SetPlayerControlFlag(1, 8, true);
		if (!IsStifled() && !IsFeared())
			GetZone()->LockAllSpells((Player*)this);
	}

	if (IsNPC() && !IsMezImmune())
	{
		HaltMovement();
	}

	mez_spells->Add(spell);
}

void Entity::RemoveMezSpell(LuaSpell* spell) {
	MutexList<LuaSpell*>* mez_spells = control_effects[CONTROL_EFFECT_TYPE_MEZ];
	if (!mez_spells || mez_spells->size(true) == 0)
		return;

	mez_spells->Remove(spell);
	if (mez_spells->size(true) == 0){
		if (IsPlayer() && !IsMezImmune() && !IsStunned()){
			if (!IsStifled() && !IsFeared())
				GetZone()->UnlockAllSpells((Player*)this);
			((Player*)this)->SetPlayerControlFlag(1, 16, false);
			if (!IsRooted())
				((Player*)this)->SetPlayerControlFlag(1, 8, false);
		}		
		
		if(!IsPlayer()) {
			GetZone()->movementMgr->StopNavigation((Entity*)this);
			((Spawn*)this)->StopMovement();
		}
	}
}

void Entity::RemoveAllMezSpells() {
	MutexList<LuaSpell*>* mez_spells = control_effects[CONTROL_EFFECT_TYPE_MEZ];
	if (!mez_spells)
		return;

	MutexList<LuaSpell*>::iterator itr = mez_spells->begin();
	while (itr.Next()){
		LuaSpell* spell = itr.value;
		if (!spell)
			continue;
		GetZone()->RemoveTargetFromSpell(spell, this);
		RemoveDetrimentalSpell(spell);
		RemoveSpellEffect(spell);
		if (IsPlayer())
			((Player*)this)->RemoveSkillBonus(spell->spell->GetSpellID());
	}

	mez_spells->clear();
	if (IsPlayer() && !IsMezImmune() && !IsStunned()){
		if (!IsStifled() && !IsFeared())
			GetZone()->UnlockAllSpells((Player*)this);
		((Player*)this)->SetPlayerControlFlag(1, 16, false);
		if (!IsRooted())
			((Player*)this)->SetPlayerControlFlag(1, 8, false);
	}
}

void Entity::AddStifleSpell(LuaSpell* spell) {
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_STIFLE])
		control_effects[CONTROL_EFFECT_TYPE_STIFLE] = new MutexList<LuaSpell*>;

	if (IsPlayer() && control_effects[CONTROL_EFFECT_TYPE_STIFLE]->size(true) == 0 && !IsStifleImmune() && !IsMezzedOrStunned())
		GetZone()->LockAllSpells((Player*)this);

	control_effects[CONTROL_EFFECT_TYPE_STIFLE]->Add(spell);
}

void Entity::RemoveStifleSpell(LuaSpell* spell) {
	MutexList<LuaSpell*>* stifle_list = control_effects[CONTROL_EFFECT_TYPE_STIFLE];
	if (!stifle_list || stifle_list->size(true) == 0)
		return;

	stifle_list->Remove(spell);

	if (IsPlayer() && stifle_list->size(true) == 0 && !IsStifleImmune() && !IsMezzedOrStunned())
		GetZone()->UnlockAllSpells((Player*)this);
}

void Entity::AddDazeSpell(LuaSpell* spell) {
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_DAZE])
		control_effects[CONTROL_EFFECT_TYPE_DAZE] = new MutexList<LuaSpell*>;

	control_effects[CONTROL_EFFECT_TYPE_DAZE]->Add(spell);
}

void Entity::RemoveDazeSpell(LuaSpell* spell) {
	MutexList<LuaSpell*>* daze_list = control_effects[CONTROL_EFFECT_TYPE_DAZE];
	if (!daze_list || daze_list->size(true) == 0)
		return;

	daze_list->Remove(spell);
}

void Entity::AddStunSpell(LuaSpell* spell) {
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_STUN])
		control_effects[CONTROL_EFFECT_TYPE_STUN] = new MutexList<LuaSpell*>;

	if (IsPlayer() && control_effects[CONTROL_EFFECT_TYPE_STUN]->size(true) == 0 && !IsStunImmune()){
		if (!IsMezzed()){
			((Player*)this)->SetPlayerControlFlag(1, 16, true);
			if (!IsRooted())
				((Player*)this)->SetPlayerControlFlag(1, 8, true);
			if (!IsStifled() && !IsFeared())
				GetZone()->LockAllSpells((Player*)this);
		}
	}

	control_effects[CONTROL_EFFECT_TYPE_STUN]->Add(spell);
}

void Entity::RemoveStunSpell(LuaSpell* spell) {
	MutexList<LuaSpell*>* stun_list = control_effects[CONTROL_EFFECT_TYPE_STUN];
	if (!stun_list || stun_list->size(true) == 0)
		return;

	stun_list->Remove(spell);
	if (stun_list->size(true) == 0){
		if (IsPlayer() && !IsMezzed() && !IsStunImmune()){
			((Player*)this)->SetPlayerControlFlag(1, 16, false);
			if (!IsRooted())
				((Player*)this)->SetPlayerControlFlag(1, 8, false);
			if (!IsStifled() && !IsFeared())
				GetZone()->UnlockAllSpells((Player*)this);
		}		
		
		if(!IsPlayer()) {
			GetZone()->movementMgr->StopNavigation((Entity*)this);
			((Spawn*)this)->StopMovement();
		}
	}
}

void Entity::HideDeityPet(bool val) {
	if (!deityPet)
		return;

	if (val) {
		deityPet->AddAllowAccessSpawn(deityPet);
		GetZone()->HidePrivateSpawn(deityPet);
	}
	else
		deityPet->MakeSpawnPublic();
}

void Entity::HideCosmeticPet(bool val) {
	if (!cosmeticPet)
		return;

	if (val) {
		cosmeticPet->AddAllowAccessSpawn(cosmeticPet);
		GetZone()->HidePrivateSpawn(cosmeticPet);
	}
	else
		cosmeticPet->MakeSpawnPublic();
}

void Entity::DismissAllPets(bool from_death, bool spawnListLocked)
{
	DismissPet(GetPet(), from_death, spawnListLocked);
	DismissPet(GetCharmedPet(), from_death, spawnListLocked);
	DismissPet(GetDeityPet(), from_death, spawnListLocked);
	DismissPet(GetCosmeticPet(), from_death, spawnListLocked);
}

void Entity::DismissPet(Entity* pet, bool from_death, bool spawnListLocked) {
	if (!pet)
		return;

	Entity* PetOwner = pet->GetOwner();

	if(pet->IsNPC())
	{
		((NPC*)pet)->SetDismissing(true);

		// Remove the spell maintained spell
		Spell* spell = master_spell_list.GetSpell(pet->GetPetSpellID(), pet->GetPetSpellTier());
		if (spell)
			GetZone()->GetSpellProcess()->DeleteCasterSpell(this, spell, from_death == true ? (string)"pet_death" : (string)"canceled");
	}

	if (pet->GetPetType() == PET_TYPE_CHARMED) {
		if(PetOwner)
			PetOwner->SetCharmedPet(0);

		if (!from_death) {
			// set the pet flag to false, owner to 0, and give the mob its old brain back
			pet->SetPet(false);
			pet->SetOwner(0);
			if(pet->IsNPC())
				((NPC*)pet)->SetBrain(new Brain((NPC*)pet));

			pet->SetDismissing(false);
		}
	}
	else if (PetOwner && pet->GetPetType() == PET_TYPE_COMBAT)
		PetOwner->SetCombatPet(0);
	else if (PetOwner && pet->GetPetType() == PET_TYPE_DEITY)
		PetOwner->SetDeityPet(0);
	else if (PetOwner && pet->GetPetType() == PET_TYPE_COSMETIC)
		PetOwner->SetCosmeticPet(0);

	// if owner is player and no combat pets left reset the pet info
	if (PetOwner && PetOwner->IsPlayer()) {
		if (!PetOwner->GetPet() && !PetOwner->GetCharmedPet())
			((Player*)PetOwner)->ResetPetInfo();
	}

	// remove the spawn from the world
	if (!from_death && pet->GetPetType() != PET_TYPE_CHARMED)
		GetZone()->RemoveSpawn(pet);
}

float Entity::CalculateBonusMod() {
	int8 level = GetLevel();
	if (level <= 20)
		return 3.0;
	else if (level >= 90)
		return 10.0;
	else
		return (level - 20) * .1 + 3.0;
}

float Entity::CalculateDPSMultiplier(){
	float dps = GetInfoStruct()->get_dps();

	if (dps > 0){
		if (dps <= 100)
			return (dps / 100 + 1);
		else if (dps <= 200)
			return (((dps - 100) * .25 + 100) / 100 + 1);
		else if (dps <= 300)
			return (((dps - 200) * .1 + 125) / 100 + 1);
		else if (dps <= 900)
			return (((dps - 300) * .05 + 135) / 100 + 1);
		else
			return (((dps - 900) * .01 + 165) / 100 + 1);
	}
	return 1;
}	

void Entity::AddWard(int32 spellID, WardInfo* ward) {
	if (m_wardList.count(spellID) == 0) {
		m_wardList[spellID] = ward;
	}
}

WardInfo* Entity::GetWard(int32 spellID) {
	WardInfo* ret = 0;

	if (m_wardList.count(spellID) > 0)
		ret = m_wardList[spellID];

	return ret;
}

void Entity::RemoveWard(int32 spellID) {
	if (m_wardList.count(spellID) > 0) {
		// Delete the ward info
		safe_delete(m_wardList[spellID]);
		// Remove from the ward list
		m_wardList.erase(spellID);
	}
}

int32 Entity::CheckWards(Entity* attacker, int32 damage, int8 damage_type) {
	map<int32, WardInfo*>::iterator itr;
	WardInfo* ward = 0;
	LuaSpell* spell = 0;

	while (m_wardList.size() > 0 && damage > 0) {
		// Get the ward with the lowest base damage
		for (itr = m_wardList.begin(); itr != m_wardList.end(); itr++) {
			if(itr->second->RoundTriggered)
				continue;
			
			if (!ward || itr->second->BaseDamage < ward->BaseDamage) {
				if ((itr->second->AbsorbAllDamage || itr->second->DamageLeft > 0) &&
					(itr->second->WardType == WARD_TYPE_ALL ||
					(itr->second->WardType == WARD_TYPE_PHYSICAL && damage_type >= DAMAGE_PACKET_DAMAGE_TYPE_SLASH && damage_type <= DAMAGE_PACKET_DAMAGE_TYPE_PIERCE) ||
					(itr->second->WardType == WARD_TYPE_MAGICAL && ((itr->second->DamageType == 0 && damage_type >= DAMAGE_PACKET_DAMAGE_TYPE_PIERCE) || (damage_type >= DAMAGE_PACKET_DAMAGE_TYPE_PIERCE && itr->second->DamageType == damage_type)))))
					ward = itr->second;
			}
		}

		if (!ward)
			break;

		spell = ward->Spell;

		// damage to redirect at the source (like intercept)
		int32 redirectDamage = 0;
		if (ward->RedirectDamagePercent)
			redirectDamage = (int32)(double)damage * ((double)ward->RedirectDamagePercent / 100.0);

		// percentage the spell absorbs of all possible damage
		int32 damageToAbsorb = 0;
		if (ward->DamageAbsorptionPercentage > 0)
			damageToAbsorb = (int32)(double)damage * ((double)ward->DamageAbsorptionPercentage/100.0);
		else
			damageToAbsorb = damage;

		int32 maxDamageAbsorptionAllowed = 0;

		// spells like Divine Aura have caps on health, eg. anything more than 50% damage is not absorbed
		if (ward->DamageAbsorptionMaxHealthPercent > 0)
			maxDamageAbsorptionAllowed = (int32)(double)GetTotalHP() * ((double)ward->DamageAbsorptionMaxHealthPercent / 100.0);

		if (maxDamageAbsorptionAllowed > 0 && damageToAbsorb >= maxDamageAbsorptionAllowed)
			damageToAbsorb = 0; // its over or equal to 50% of the total hp allowed, thus this damage is not absorbed

		int32 baseDamageRemaining = damage - damageToAbsorb;

		bool hasSpellBeenRemoved = false;
		if (ward->AbsorbAllDamage)
		{
			ward->LastAbsorbedDamage = ward->DamageLeft;

			if (!redirectDamage)
				GetZone()->SendHealPacket(ward->Spell->caster, this, HEAL_PACKET_TYPE_ABSORB, damage, spell->spell->GetName());

			damage = 0;
		}
		else if (damageToAbsorb >= ward->DamageLeft) {
			// Damage is greater than or equal to the amount left on the ward

			ward->LastAbsorbedDamage = ward->DamageLeft;
			// remove what damage we can absorb 
			damageToAbsorb -= ward->DamageLeft;

			// move back what couldn't be absorbed to the base dmg and apply to the overall damage
			baseDamageRemaining += damageToAbsorb;
			damage = baseDamageRemaining;
			ward->DamageLeft = 0;
			spell->damage_remaining = 0;

			if(!redirectDamage)
				GetZone()->SendHealPacket(spell->caster, this, HEAL_PACKET_TYPE_ABSORB, ward->DamageLeft, spell->spell->GetName());

			if (!ward->keepWard) {
				hasSpellBeenRemoved = true;
				RemoveWard(spell->spell->GetSpellID());
				GetZone()->GetSpellProcess()->DeleteCasterSpell(spell, "purged");
			}
		}
		else {
			ward->LastAbsorbedDamage = damageToAbsorb;
			// Damage is less then the amount left on the ward
			ward->DamageLeft -= damageToAbsorb;

			spell->damage_remaining = ward->DamageLeft;
			if (spell->caster->IsPlayer())
				ClientPacketFunctions::SendMaintainedExamineUpdate(((Player*)spell->caster)->GetClient(), spell->slot_pos, ward->DamageLeft, 1);

			if (!redirectDamage)
				GetZone()->SendHealPacket(ward->Spell->caster, this, HEAL_PACKET_TYPE_ABSORB, damage, spell->spell->GetName());

			// remaining damage not absorbed by percentage must be set
			damage = baseDamageRemaining;
		}

		if (redirectDamage)
		{
			ward->LastRedirectDamage = redirectDamage;
			if (this->IsPlayer())
			{
				Client* client = this->GetClient();
				if(client) {
					client->Message(CHANNEL_COMBAT, "%s intercepted some of the damage intended for you!", spell->caster->GetName());
				}
			}
			if (spell->caster && spell->caster->IsPlayer())
			{
				Client* client = ((Player*)spell->caster)->GetClient();
				if(client) {
					client->Message(CHANNEL_COMBAT, "YOU intercept some of the damage intended for %s!", this->GetName());
				}
			}

			if (attacker && spell->caster)
				attacker->DamageSpawn(spell->caster, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, damage_type, redirectDamage, redirectDamage, 0, 0, false, false, false, false, spell);
		}

		bool shouldRemoveSpell = false;
		ward->HitCount++; // increment hit count
		ward->RoundTriggered = true;
		
		if (ward->MaxHitCount && spell->num_triggers && spell->caster->GetZone())
		{
			spell->num_triggers--;
			if(spell->caster->IsPlayer()) {
				ClientPacketFunctions::SendMaintainedExamineUpdate(((Player*)spell->caster)->GetClient(), spell->slot_pos, spell->num_triggers, 0);
			}
		}
		
		if (ward->MaxHitCount && ward->HitCount >= ward->MaxHitCount) // there isn't a max hit requirement with the hit count, so just go based on hit count
			shouldRemoveSpell = true;

		if (shouldRemoveSpell && !hasSpellBeenRemoved)
		{
			RemoveWard(spell->spell->GetSpellID());
			GetZone()->GetSpellProcess()->DeleteCasterSpell(spell, "purged");
		}

		// Reset ward pointer
		ward = 0;
	}
	
	for (itr = m_wardList.begin(); itr != m_wardList.end(); itr++) {
		itr->second->RoundTriggered = false;
	}

	return damage;
}

float Entity::CalculateCastingSpeedMod() {
	float cast_speed = info_struct.get_casting_speed();
	
	if(cast_speed > 0)
		return 100 * max((float) 0.5, (float) (1 + (1 - (1 / (1 + (cast_speed * .01))))));
	else if (cast_speed < 0)
		return 100 * min((float) 1.5, (float) (1 + (1 - (1 / (1 + (cast_speed * -.01))))));
	return 0;
}

float Entity::GetSpeed() {
	float ret = speed > GetBaseSpeed() ? speed : GetBaseSpeed();
	if(IsPlayer()) {
		ret = GetBaseSpeed();
	}
	MStats.lock();
	
	if ((IsStealthed() || IsInvis()) && stats.count(ITEM_STAT_STEALTHINVISSPEEDMOD)) {
		ret += stats[ITEM_STAT_STEALTHINVISSPEEDMOD];
	}
	
	if (!GetInfoStruct()->get_engaged_encounter()) {
		if (stats.count(ITEM_STAT_SPEED) && stats.count(ITEM_STAT_MOUNTSPEED)) {
			ret += max(stats[ITEM_STAT_SPEED], stats[ITEM_STAT_MOUNTSPEED]);
		}
		else if (stats.count(ITEM_STAT_SPEED)) {
			ret += stats[ITEM_STAT_SPEED];
		}
		else if (stats.count(ITEM_STAT_MOUNTSPEED)) {
			ret += stats[ITEM_STAT_MOUNTSPEED];
		}
	}
	else if (GetInfoStruct()->get_engaged_encounter()) {
		
		if (GetMaxSpeed() > 0.0f)
			ret = GetMaxSpeed();
		
		if (stats.count(ITEM_STAT_OFFENSIVESPEED)) {
			ret += stats[ITEM_STAT_OFFENSIVESPEED];
		}
	}

	MStats.unlock();
	ret *= speed_multiplier;
	return ret;
}

float Entity::GetAirSpeed() {
	float ret = speed;

	if (!GetInfoStruct()->get_engaged_encounter())
		ret += stats[ITEM_STAT_MOUNTAIRSPEED];

	ret *= speed_multiplier;
	return ret;
}

void Entity::SetThreatTransfer(ThreatTransfer* transfer) {
	safe_delete(m_threatTransfer);
	m_threatTransfer = transfer;	
}
int8 Entity::GetTraumaCount() {
	return det_count_list[DET_TYPE_TRAUMA];
}

int8 Entity::GetArcaneCount() {
	return det_count_list[DET_TYPE_ARCANE];
}

int8 Entity::GetNoxiousCount() {
	return det_count_list[DET_TYPE_NOXIOUS];
}

int8 Entity::GetElementalCount() {
	return det_count_list[DET_TYPE_ELEMENTAL];
}

int8 Entity::GetCurseCount() {
	return det_count_list[DET_TYPE_CURSE];
}

Mutex* Entity::GetDetrimentMutex() {
	return &MDetriments;
}

Mutex* Entity::GetMaintainedMutex() {
	return &MMaintainedSpells;
}

Mutex* Entity::GetSpellEffectMutex() {
	return &MSpellEffects;
}

bool Entity::HasCurableDetrimentType(int8 det_type) {
	DetrimentalEffects* det;
	bool ret = false;
	MDetriments.readlock(__FUNCTION__, __LINE__);
	for (int32 i = 0; i < detrimental_spell_effects.size(); i++){
		det = &detrimental_spell_effects.at(i);
		if(det && det->det_type == det_type && !det->incurable){
			ret = true;
			break;
		}
	}
	MDetriments.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

void Entity::ClearAllDetriments() {
	MDetriments.writelock(__FUNCTION__, __LINE__);
	detrimental_spell_effects.clear();
	det_count_list.clear();
	MDetriments.releasewritelock(__FUNCTION__, __LINE__);
}

void Entity::CureDetrimentByType(int8 cure_count, int8 det_type, string cure_name, Entity* caster, int8 cure_level) {
	if (cure_count <= 0 || GetDetTypeCount(det_type) <= 0)
		return;

	vector<DetrimentalEffects>* det_list = &detrimental_spell_effects;
	DetrimentalEffects* det;
	vector<LuaSpell*> remove_list;
	LuaSpell* spell = 0;
	vector<LevelArray*>* levels;
	int8 caster_class1 = 0;
	int8 caster_class2 = 0;
	int8 caster_class3 = 0;
	InfoStruct* info_struct = 0;
	bool pass_level_check = false;

	MDetriments.readlock(__FUNCTION__, __LINE__);
	for (int32 i = 0; i<det_list->size(); i++){
		det = &det_list->at(i);
		if (det && det->det_type == det_type && !det->incurable){
			levels = det->spell->spell->GetSpellLevels();
			info_struct = det->caster->GetInfoStruct();
			pass_level_check = false;
			bool has_level_checks = false;
			for (int32 x = 0; x < levels->size(); x++){
				has_level_checks = true;
				int8 use_classic_lvls = rule_manager.GetGlobalRule(R_Spells, UseClassicSpellLevel)->GetInt8();
				if(levels->at(x)->classic_spell_level == 0.0f)
					use_classic_lvls = 0;
				// class checks are worthless we can't guarantee the caster is that class
				if (!cure_level || (use_classic_lvls && cure_level >= std::floor(levels->at(x)->classic_spell_level)) || (!use_classic_lvls && cure_level >= (levels->at(x)->spell_level / 10))){
					pass_level_check = true;
					break;
				}
			}
			
			if (pass_level_check || !has_level_checks){
				remove_list.push_back(det->spell);
				cure_count--;
				if (cure_count == 0)
					break;
			}
		}
	}
	MDetriments.releasereadlock(__FUNCTION__, __LINE__);

	for (int32 i = 0; i<remove_list.size(); i++){
		spell = remove_list.at(i);
		
		LogWrite(PLAYER__ERROR, 0, "Debug", "Remove spell %s", remove_list.at(i)->spell->GetName());
		GetZone()->SendDispellPacket(caster, this, cure_name, (string)remove_list.at(i)->spell->GetName(), DISPELL_TYPE_CURE);
		GetZone()->GetSpellProcess()->DeleteCasterSpell(spell, "cured", false, this);
	}
	remove_list.clear();
}

void Entity::CureDetrimentByControlEffect(int8 cure_count, int8 control_type, string cure_name, Entity* caster, int8 cure_level) {
	if (cure_count <= 0 || GetDetCount() <= 0)
		return;

	vector<DetrimentalEffects>* det_list = &detrimental_spell_effects;
	DetrimentalEffects* det;
	vector<LuaSpell*> remove_list;
	LuaSpell* spell = 0;
	vector<LevelArray*>* levels;
	int8 caster_class1 = 0;
	int8 caster_class2 = 0;
	int8 caster_class3 = 0;
	InfoStruct* info_struct = 0;
	bool pass_level_check = false;

	MDetriments.readlock(__FUNCTION__, __LINE__);
	for (int32 i = 0; i<det_list->size(); i++){
		det = &det_list->at(i);
		if (det && det->control_effect == control_type && !det->incurable){
			levels = det->spell->spell->GetSpellLevels();
			info_struct = det->caster->GetInfoStruct();
			pass_level_check = false;
			for (int32 x = 0; x < levels->size(); x++){
				int8 use_classic_lvls = rule_manager.GetGlobalRule(R_Spells, UseClassicSpellLevel)->GetInt8();
				if(levels->at(x)->classic_spell_level == 0.0f)
					use_classic_lvls = 0;
				if (!cure_level || (use_classic_lvls && cure_level >= std::floor(levels->at(x)->classic_spell_level)) || (!use_classic_lvls && cure_level >= (levels->at(x)->spell_level / 10))){
					pass_level_check = true;
					break;
				}
			}
			if (pass_level_check){
				remove_list.push_back(det->spell);
				cure_count--;
				if (cure_count == 0)
					break;
			}
		}
	}
	MDetriments.releasereadlock(__FUNCTION__, __LINE__);

	for (int32 i = 0; i<remove_list.size(); i++){
		spell = remove_list.at(i);
		GetZone()->SendDispellPacket(caster, this, cure_name, (string)remove_list.at(i)->spell->GetName(), DISPELL_TYPE_CURE);
		if (GetZone())
			GetZone()->RemoveTargetFromSpell(spell, this);
		RemoveSpellEffect(spell);
		RemoveDetrimentalSpell(spell);
	}
	remove_list.clear();
}

void Entity::RemoveDetrimentalSpell(LuaSpell* spell) {
	if(!spell || (spell->spell && spell->spell->GetSpellData() && spell->spell->GetSpellData()->det_type == 0))
		return;
	MDetriments.writelock(__FUNCTION__, __LINE__);
	vector<DetrimentalEffects>* det_list = &detrimental_spell_effects;
	vector<DetrimentalEffects>::iterator itr;
	for(itr = det_list->begin(); itr != det_list->end(); itr++){
		if((*itr).spell == spell){
			det_count_list[(*itr).det_type]--;
			det_list->erase(itr);
			if(IsPlayer())
				((Player*)this)->SetCharSheetChanged(true);
			break;
		}
	}
	MDetriments.releasewritelock(__FUNCTION__, __LINE__);
}

int8 Entity::GetDetTypeCount(int8 det_type){
	return det_count_list[det_type];
}

int8 Entity::GetDetCount() {
	int8 det_count = 0;
	map<int8, int8>::iterator itr;

	for(itr=det_count_list.begin(); itr != det_count_list.end(); itr++)
		det_count += (*itr).second;
	
	return det_count;
}

vector<DetrimentalEffects>* Entity::GetDetrimentalSpellEffects() {
	return &detrimental_spell_effects;
}

void Entity::AddDetrimentalSpell(LuaSpell* luaspell, int32 override_expire_timestamp){
	if(!luaspell || !luaspell->caster)
		return;
	
	Spell* spell = luaspell->spell;
	DetrimentalEffects* det = GetDetrimentalEffect(spell->GetSpellID(), luaspell->caster);
	DetrimentalEffects new_det;
	if(det)
		RemoveDetrimentalSpell(det->spell);

	SpellData* data = spell->GetSpellData();
	if(!data)
		return;

	new_det.caster = luaspell->caster;
	new_det.spell = luaspell;
	if (spell->GetSpellData()->duration_until_cancel)
		new_det.expire_timestamp = 0xFFFFFFFF;
	else if(override_expire_timestamp)
		new_det.expire_timestamp = override_expire_timestamp;
	else
		new_det.expire_timestamp = Timer::GetCurrentTime2() + (spell->GetSpellDuration()*100);
	new_det.icon = data->icon;
	new_det.icon_backdrop = data->icon_backdrop;
	new_det.tier = data->tier;
	new_det.det_type = data->det_type;
	new_det.incurable = data->incurable;
	new_det.spell_id = spell->GetSpellID();
	new_det.control_effect = data->control_effect_type;
	new_det.total_time = spell->GetSpellDuration()/10;

	MDetriments.writelock(__FUNCTION__, __LINE__);
	detrimental_spell_effects.push_back(new_det);
	det_count_list[new_det.det_type]++;
	MDetriments.releasewritelock(__FUNCTION__, __LINE__);
}

DetrimentalEffects* Entity::GetDetrimentalEffect(int32 spell_id, Entity* caster){
	vector<DetrimentalEffects>* det_list = &detrimental_spell_effects;
	DetrimentalEffects* ret = 0;
	MDetriments.readlock(__FUNCTION__, __LINE__);
	for(int32 i=0; i<det_list->size(); i++){
		if (det_list->at(i).spell_id == spell_id && det_list->at(i).caster == caster)
			ret = &det_list->at(i);
	}
	MDetriments.releasereadlock(__FUNCTION__, __LINE__);
	
	return ret;
}

void Entity::CancelAllStealth() {
	bool did_change = false;
	MutexList<LuaSpell*>* stealth_list = control_effects[CONTROL_EFFECT_TYPE_STEALTH];
	if (stealth_list){
		MutexList<LuaSpell*>::iterator itr = stealth_list->begin();
		while (itr.Next()){
			if (itr.value->caster == this)
				GetZone()->GetSpellProcess()->AddSpellCancel(itr.value);
			else{
				GetZone()->RemoveTargetFromSpell(itr.value, this);
				RemoveSpellEffect(itr.value);
			}
			did_change = true;
		}
	}
	MutexList<LuaSpell*>* invis_list = control_effects[CONTROL_EFFECT_TYPE_INVIS];
	if (invis_list){
		MutexList<LuaSpell*>::iterator invis_itr = invis_list->begin();
		while (invis_itr.Next()){
			if (invis_itr.value->caster == this)
				GetZone()->GetSpellProcess()->AddSpellCancel(invis_itr.value);
			else{
				GetZone()->RemoveTargetFromSpell(invis_itr.value, this);
				RemoveSpellEffect(invis_itr.value);
			}
			did_change = true;
		}
	}

	if (did_change){
		info_changed = true;
		changed = true;
		AddChangedZoneSpawn();
		if (IsPlayer())
			((Player*)this)->SetCharSheetChanged(true);
	}
}

bool Entity::IsStealthed(){
	MutexList<LuaSpell*>* stealth_list = control_effects[CONTROL_EFFECT_TYPE_STEALTH];
	return  (!stealth_list || stealth_list->size(true) == 0) == false;
}

bool Entity::CanSeeInvis(Entity* target) {
	if (!target)
		return true;

	if (!target->IsStealthed() && !target->IsInvis())
		return true;
	if (target->IsStealthed() && HasSeeHideSpell())
		return true;
	else if (target->IsInvis() && HasSeeInvisSpell())
		return true;

	return false;
}

bool Entity::IsInvis(){
	MutexList<LuaSpell*>* invis_list = control_effects[CONTROL_EFFECT_TYPE_INVIS];
	return  (!invis_list || invis_list->size(true) == 0) == false;
}

void Entity::AddStealthSpell(LuaSpell* spell) {
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_STEALTH])
		control_effects[CONTROL_EFFECT_TYPE_STEALTH] = new MutexList<LuaSpell*>;

	control_effects[CONTROL_EFFECT_TYPE_STEALTH]->Add(spell);
	if (control_effects[CONTROL_EFFECT_TYPE_STEALTH]->size(true) == 1){
		info_changed = true;
		changed = true;
		AddChangedZoneSpawn();
		if (IsPlayer() && ((Player*)this)->GetClient())
		{
			((Player*)this)->SetCharSheetChanged(true);
			GetZone()->SendAllSpawnsForVisChange(((Player*)this)->GetClient());
		}
	}
}

void Entity::AddInvisSpell(LuaSpell* spell) {
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_INVIS])
		control_effects[CONTROL_EFFECT_TYPE_INVIS] = new MutexList<LuaSpell*>;

	control_effects[CONTROL_EFFECT_TYPE_INVIS]->Add(spell);
	if (control_effects[CONTROL_EFFECT_TYPE_INVIS]->size(true) == 1){
		info_changed = true;
		changed = true;
		AddChangedZoneSpawn();
		if (IsPlayer() && ((Player*)this)->GetClient())
		{
			((Player*)this)->SetCharSheetChanged(true);
			GetZone()->SendAllSpawnsForVisChange(((Player*)this)->GetClient());
		}
	}
}

void Entity::RemoveInvisSpell(LuaSpell* spell) {
	MutexList<LuaSpell*>* invis_list = control_effects[CONTROL_EFFECT_TYPE_INVIS];
	if (!invis_list || invis_list->size(true) == 0)
		return;

	invis_list->Remove(spell);
	RemoveSpellEffect(spell);
	if (invis_list->size(true) == 0){
		info_changed = true;
		changed = true;
		AddChangedZoneSpawn();
		if (IsPlayer() && ((Player*)this)->GetClient())
		{
			((Player*)this)->SetCharSheetChanged(true);
			GetZone()->SendAllSpawnsForVisChange(((Player*)this)->GetClient());
		}
	}
}

void Entity::RemoveStealthSpell(LuaSpell* spell) {
	MutexList<LuaSpell*>* stealth_list = control_effects[CONTROL_EFFECT_TYPE_STEALTH];
	if (!stealth_list || stealth_list->size(true) == 0)
		return;

	stealth_list->Remove(spell);
	RemoveSpellEffect(spell);
	if (stealth_list->size() == 0){
		info_changed = true;
		changed = true;
		AddChangedZoneSpawn();
		if (IsPlayer() && ((Player*)this)->GetClient())
		{
			((Player*)this)->SetCharSheetChanged(true);
			GetZone()->SendAllSpawnsForVisChange(((Player*)this)->GetClient());
		}
	}
}

void Entity::AddRootSpell(LuaSpell* spell) {
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_ROOT])
		control_effects[CONTROL_EFFECT_TYPE_ROOT] = new MutexList<LuaSpell*>;

	if (control_effects[CONTROL_EFFECT_TYPE_ROOT]->size(true) == 0 && !IsRootImmune()) {
		if (IsPlayer()){
			if (!IsMezzedOrStunned())
				((Player*)this)->SetPlayerControlFlag(1, 8, true); // heading movement only
		}
		else
			SetSpeedMultiplier(0.0f);
	}

	control_effects[CONTROL_EFFECT_TYPE_ROOT]->Add(spell);
}

void Entity::RemoveRootSpell(LuaSpell* spell) {
	MutexList<LuaSpell*>* root_list = control_effects[CONTROL_EFFECT_TYPE_ROOT];
	if (!root_list || root_list->size(true) == 0)
		return;

	root_list->Remove(spell);
	if (root_list->size(true) == 0 && !IsRootImmune()) {
		if (IsPlayer()){
			if (!IsMezzedOrStunned())
				((Player*)this)->SetPlayerControlFlag(1, 8, false); // heading movement only
		}
		else {
			// GetHighestSnare() will return 1.0f if no snares returning the spawn to full speed
			SetSpeedMultiplier(GetHighestSnare());
		}
	
		if(!IsPlayer()) {
			GetZone()->movementMgr->StopNavigation((Entity*)this);
			((Spawn*)this)->StopMovement();
		}
	}
}

void Entity::AddFearSpell(LuaSpell* spell){
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_FEAR])
		control_effects[CONTROL_EFFECT_TYPE_FEAR] = new MutexList<LuaSpell*>;

	if (IsPlayer() && control_effects[CONTROL_EFFECT_TYPE_FEAR]->size(true) == 0 && !IsFearImmune()){
		((Player*)this)->SetPlayerControlFlag(4, 4, true); // feared
		if (!IsMezzedOrStunned() && !IsStifled())
			GetZone()->LockAllSpells((Player*)this);
	}

	if (!IsFearImmune() && IsNPC())
	{
		HaltMovement();
	}

	control_effects[CONTROL_EFFECT_TYPE_FEAR]->Add(spell);
}

void Entity::RemoveFearSpell(LuaSpell* spell){
	MutexList<LuaSpell*>* fear_list = control_effects[CONTROL_EFFECT_TYPE_FEAR];
	if (!fear_list || fear_list->size(true) == 0)
		return;

	fear_list->Remove(spell);

	if (IsPlayer() && fear_list->size(true) == 0 && !IsFearImmune()){
		((Player*)this)->SetPlayerControlFlag(4, 4, false); // feared disabled
		if (!IsMezzedOrStunned() && !IsStifled())
			GetZone()->LockAllSpells((Player*)this);
	}

	if (IsNPC())
	{
		HaltMovement();
	}
}

void Entity::AddSnareSpell(LuaSpell* spell) {
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_SNARE])
		control_effects[CONTROL_EFFECT_TYPE_SNARE] = new MutexList<LuaSpell*>;

	control_effects[CONTROL_EFFECT_TYPE_SNARE]->Add(spell);

	// Don't set speed multiplier if there is a root or no snare values
	MutexList<LuaSpell*>* roots = control_effects[CONTROL_EFFECT_TYPE_ROOT];
	if ((!roots || roots->size(true) == 0) && snare_values.size() > 0)
		SetSpeedMultiplier(GetHighestSnare());
}

void Entity::RemoveSnareSpell(LuaSpell* spell) {
	MutexList<LuaSpell*>* snare_list = control_effects[CONTROL_EFFECT_TYPE_SNARE];
	if (!snare_list || snare_list->size(true) == 0)
		return;

	snare_list->Remove(spell);
	snare_values.erase(spell);

	//LogWrite(PLAYER__ERROR, 0, "Debug", "snare_values.size() = %u", snare_values.size());

	// only change speeds if there are no roots
	MutexList<LuaSpell*>* roots = control_effects[CONTROL_EFFECT_TYPE_ROOT];
	if (!roots || roots->size(true) == 0) {
		float multiplier = GetHighestSnare();
		//LogWrite(PLAYER__ERROR, 0, "Debug", "GetHighestSnare() = %f", multiplier);
		SetSpeedMultiplier(multiplier);
	}
}

void Entity::SetSnareValue(LuaSpell* spell, float snare_val) {
	if (!spell)
		return;

	snare_values[spell] = snare_val;
}

float Entity::GetHighestSnare() {
	// For simplicity this will return the highest snare value, which is actually the lowest value
	float ret = 1.0f;
	float weight_diff = 0.0f;
	if (IsPlayer() && rule_manager.GetZoneRule(GetZoneID(), R_Player, WeightInflictsSpeed)->GetBool()) {
		float weight_pct_impact = rule_manager.GetZoneRule(GetZoneID(), R_Player, WeightPercentImpact)->GetFloat();
		float weight_pct_cap = rule_manager.GetZoneRule(GetZoneID(), R_Player, WeightPercentCap)->GetFloat();
		if(weight_pct_impact > 1.0f) {
			weight_pct_impact = 1.0f;
		}
		if(weight_pct_cap < weight_pct_impact) {
			weight_pct_impact = weight_pct_cap;
		}
		int32 weight = GetInfoStruct()->get_weight();
		int32 max_weight = GetInfoStruct()->get_max_weight();
		if(weight > max_weight) {
			int32 diff = weight - max_weight;
			weight_diff = (float)diff * weight_pct_impact; // percentage impact rule on weight "per stone", default 1%
			if(weight_diff > weight_pct_cap) // cap weight impact rule
				weight_diff = weight_pct_cap; // cap weight impact rule
		}
	}
	if (snare_values.size() == 0)
		return ((ret - weight_diff) < 0.0f ) ? 0.0f : (ret - weight_diff);

	map<LuaSpell*, float>::iterator itr;
	for (itr = snare_values.begin(); itr != snare_values.end(); itr++) {
		if (itr->second < ret)
			ret = itr->second;
	}
	
	return ((ret - weight_diff) < 0.0f ) ? 0.0f : (ret - weight_diff);
}

bool Entity::IsSnared() {
	if (control_effects.size() < 1 || !control_effects[CONTROL_EFFECT_TYPE_SNARE])
		return false;

	MutexList<LuaSpell*>* snare_list = control_effects[CONTROL_EFFECT_TYPE_SNARE];
	return (!snare_list || snare_list->size(true) == 0) == false;
}

bool Entity::IsMezzed(){
	if (control_effects.size() < 1 || !control_effects[CONTROL_EFFECT_TYPE_MEZ])
		return false;

	MutexList<LuaSpell*>* mez_spells = control_effects[CONTROL_EFFECT_TYPE_MEZ];
	return  (!mez_spells || mez_spells->size(true) == 0 || IsMezImmune()) == false;
}

bool Entity::IsStifled(){
	if (!control_effects[CONTROL_EFFECT_TYPE_STIFLE])
		return false;

	MutexList<LuaSpell*>* stifle_list = control_effects[CONTROL_EFFECT_TYPE_STIFLE];
	return  (!stifle_list || stifle_list->size(true) == 0 || IsStifleImmune()) == false;
}

bool Entity::IsDazed(){
	if (control_effects.size() < 1 || !control_effects[CONTROL_EFFECT_TYPE_DAZE])
		return false;

	MutexList<LuaSpell*>* daze_list = control_effects[CONTROL_EFFECT_TYPE_DAZE];
	return  (!daze_list || daze_list->size(true) == 0 || IsDazeImmune()) == false;
}

bool Entity::IsStunned(){
	if (!control_effects[CONTROL_EFFECT_TYPE_STUN])
		return false;

	MutexList<LuaSpell*>* stun_list = control_effects[CONTROL_EFFECT_TYPE_STUN];
	return (!stun_list || stun_list->size(true) == 0 || IsStunImmune()) == false;
}

bool Entity::IsRooted(){
	if (control_effects.size() < 1 || !control_effects[CONTROL_EFFECT_TYPE_ROOT])
		return false;

	MutexList<LuaSpell*>* root_list = control_effects[CONTROL_EFFECT_TYPE_ROOT];
	return (!root_list || root_list->size(true) == 0 || IsRootImmune()) == false;
}

bool Entity::IsFeared(){
	if (control_effects.size() < 1 || !control_effects[CONTROL_EFFECT_TYPE_FEAR])
		return false;

	MutexList<LuaSpell*>* fear_list = control_effects[CONTROL_EFFECT_TYPE_FEAR];
	return (!fear_list || fear_list->size(true) == 0 || IsFearImmune()) == false;
}

void Entity::AddWaterwalkSpell(LuaSpell* spell){
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_WALKUNDERWATER])
		control_effects[CONTROL_EFFECT_TYPE_WALKUNDERWATER] = new MutexList<LuaSpell*>;

	control_effects[CONTROL_EFFECT_TYPE_WALKUNDERWATER]->Add(spell);
	if (control_effects[CONTROL_EFFECT_TYPE_WALKUNDERWATER]->size(true) == 1 && IsPlayer())
		((Player*)this)->SetPlayerControlFlag(3, 128, true); // enable walking underwater
}

void Entity::RemoveWaterwalkSpell(LuaSpell* spell){
	MutexList<LuaSpell*>* waterwalk_list = control_effects[CONTROL_EFFECT_TYPE_WALKUNDERWATER];
	if (!waterwalk_list || waterwalk_list->size(true) == 0)
		return;

	waterwalk_list->Remove(spell);
	if (waterwalk_list->size(true) == 0 && IsPlayer())
		((Player*)this)->SetPlayerControlFlag(3, 128, false); // disable walking underwater
}

void Entity::AddWaterjumpSpell(LuaSpell* spell){
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_JUMPUNDERWATER])
		control_effects[CONTROL_EFFECT_TYPE_JUMPUNDERWATER] = new MutexList<LuaSpell*>;

	control_effects[CONTROL_EFFECT_TYPE_JUMPUNDERWATER]->Add(spell);
	if (control_effects[CONTROL_EFFECT_TYPE_JUMPUNDERWATER]->size(true) == 1 && IsPlayer())
		((Player*)this)->SetPlayerControlFlag(4, 1, true); // enable moonjumps underwater
}

void Entity::RemoveWaterjumpSpell(LuaSpell* spell){
	MutexList<LuaSpell*>* waterjump_list = control_effects[CONTROL_EFFECT_TYPE_JUMPUNDERWATER];
	if (!waterjump_list || waterjump_list->size(true) == 0)
		return;

	waterjump_list->Remove(spell);
	if (waterjump_list->size(true) == 0 && IsPlayer())
		((Player*)this)->SetPlayerControlFlag(4, 1, false); // disable moonjumps underwater
}

void Entity::AddAOEImmunity(LuaSpell* spell){
	if (!spell)
		return;

	if (!immunities[IMMUNITY_TYPE_AOE])
		immunities[IMMUNITY_TYPE_AOE] = new MutexList<LuaSpell*>;

	immunities[IMMUNITY_TYPE_AOE]->Add(spell);
}

void Entity::RemoveAOEImmunity(LuaSpell* spell){
	MutexList<LuaSpell*>* aoe_list = immunities[IMMUNITY_TYPE_AOE];
	if (!aoe_list || aoe_list->size(true) == 0)
		return;
	aoe_list->Remove(spell);
}

bool Entity::IsAOEImmune(){
	return (immunities[IMMUNITY_TYPE_AOE] && immunities[IMMUNITY_TYPE_AOE]->size(true));
}

void Entity::AddStunImmunity(LuaSpell* spell){
	if (!spell)
		return;

	if (!immunities[IMMUNITY_TYPE_STUN])
		immunities[IMMUNITY_TYPE_STUN] = new MutexList<LuaSpell*>;

	if (IsPlayer() && IsStunned() && !IsMezzed()){
		((Player*)this)->SetPlayerControlFlag(4, 64, false);
		if (!IsFeared() && !IsStifled())
			((Player*)this)->UnlockAllSpells();
	}

	immunities[IMMUNITY_TYPE_STUN]->Add(spell);
}

void Entity::RemoveStunImmunity(LuaSpell* spell){
	MutexList<LuaSpell*>* stun_list = immunities[IMMUNITY_TYPE_STUN];
	if (!stun_list || stun_list->size(true) == 0)
		return;

	stun_list->Remove(spell);

	if (IsPlayer() && IsStunned() && !IsMezzed()){
		((Player*)this)->SetPlayerControlFlag(4, 64, true);
		if (!IsFeared() && !IsStifled())
			((Player*)this)->UnlockAllSpells();
	}
}

bool Entity::IsStunImmune(){
	return (immunities[IMMUNITY_TYPE_STUN] && immunities[IMMUNITY_TYPE_STUN]->size(true) > 0);
}

void Entity::AddStifleImmunity(LuaSpell* spell){
	if (!spell)
		return;

	if (!immunities[IMMUNITY_TYPE_STIFLE])
		immunities[IMMUNITY_TYPE_STIFLE] = new MutexList<LuaSpell*>;

	if (IsPlayer() && immunities[IMMUNITY_TYPE_STIFLE]->size(true) == 0){
		if (IsStifled() && !IsMezzedOrStunned() && !IsFeared())
			((Player*)this)->UnlockAllSpells();
	}

	immunities[IMMUNITY_TYPE_STIFLE]->Add(spell);
}

void Entity::RemoveStifleImmunity(LuaSpell* spell){
	MutexList<LuaSpell*>* stifle_list = immunities[IMMUNITY_TYPE_STIFLE];
	if (!stifle_list || stifle_list->size(true) == 0)
		return;

	stifle_list->Remove(spell);

	if (IsPlayer() && IsStifled() && !IsMezzedOrStunned() && !IsFeared())
		((Player*)this)->UnlockAllSpells();
}

bool Entity::IsStifleImmune(){
	return (immunities[IMMUNITY_TYPE_STIFLE] && immunities[IMMUNITY_TYPE_STIFLE]->size(true) > 0);
}

void Entity::AddMezImmunity(LuaSpell* spell){
	if (!spell)
		return;

	if (!immunities[IMMUNITY_TYPE_MEZ])
		immunities[IMMUNITY_TYPE_MEZ] = new MutexList<LuaSpell*>;

	if (IsPlayer() && IsMezzed() && !IsStunned()){
		((Player*)this)->SetPlayerControlFlag(4, 64, false);
		if (!IsFeared() && !IsStifled())
			((Player*)this)->UnlockAllSpells();
	}

	immunities[IMMUNITY_TYPE_MEZ]->Add(spell);
}

void Entity::RemoveMezImmunity(LuaSpell* spell){
	MutexList<LuaSpell*>* mez_list = immunities[IMMUNITY_TYPE_MEZ];
	if (!mez_list || mez_list->size(true) == 0)
		return;

	mez_list->Remove(spell);

	if (IsPlayer() && IsMezzed() && !IsStunned()){
		((Player*)this)->SetPlayerControlFlag(4, 64, true);
		if (!IsFeared() && !IsStifled())
			((Player*)this)->LockAllSpells();
	}
}

bool Entity::IsMezImmune(){
	return (immunities[IMMUNITY_TYPE_MEZ] && immunities[IMMUNITY_TYPE_MEZ]->size(true) > 0);
}

void Entity::AddRootImmunity(LuaSpell* spell){
	if (!spell)
		return;

	if (!immunities[IMMUNITY_TYPE_ROOT])
		immunities[IMMUNITY_TYPE_ROOT] = new MutexList<LuaSpell*>;

	if (IsPlayer() && IsRooted())
		((Player*)this)->SetPlayerControlFlag(1, 8, false);

	immunities[IMMUNITY_TYPE_ROOT]->Add(spell);
}

void Entity::RemoveRootImmunity(LuaSpell* spell){
	MutexList<LuaSpell*>* root_list = immunities[IMMUNITY_TYPE_ROOT];
	if (!root_list || root_list->size(true) == 0)
		return;

	root_list->Remove(spell);

	if (IsPlayer() && IsRooted())
		((Player*)this)->SetPlayerControlFlag(1, 8, true);
}

bool Entity::IsRootImmune(){
	return (immunities[IMMUNITY_TYPE_ROOT] && immunities[IMMUNITY_TYPE_ROOT]->size(true) > 0);
}

void Entity::AddFearImmunity(LuaSpell* spell){
	if (!spell)
		return;

	if (!immunities[IMMUNITY_TYPE_FEAR])
		immunities[IMMUNITY_TYPE_FEAR] = new MutexList<LuaSpell*>;

	if (IsPlayer() && IsFeared()){
		if (!IsMezzedOrStunned() && !IsStifled())
			((Player*)this)->UnlockAllSpells();
		((Player*)this)->SetPlayerControlFlag(4, 4, false);
	}

	immunities[IMMUNITY_TYPE_FEAR]->Add(spell);
}

void Entity::RemoveFearImmunity(LuaSpell* spell){
	MutexList<LuaSpell*>* fear_list = immunities[IMMUNITY_TYPE_FEAR];
	if (!fear_list || fear_list->size(true) == 0)
		return;

	fear_list->Remove(spell);

	if (IsPlayer() && IsFeared()){
		if (!IsMezzedOrStunned() && !IsStifled())
			((Player*)this)->LockAllSpells();
		((Player*)this)->SetPlayerControlFlag(4, 4, true);
	}
}

bool Entity::IsFearImmune(){
	return (immunities[IMMUNITY_TYPE_FEAR] && immunities[IMMUNITY_TYPE_FEAR]->size(true) > 0);
}

void Entity::AddDazeImmunity(LuaSpell* spell){
	if (!spell)
		return;

	if (!immunities[IMMUNITY_TYPE_DAZE])
		immunities[IMMUNITY_TYPE_DAZE] = new MutexList<LuaSpell*>;

	immunities[IMMUNITY_TYPE_DAZE]->Add(spell);
}

void Entity::RemoveDazeImmunity(LuaSpell* spell){
	MutexList<LuaSpell*>* daze_list = immunities[IMMUNITY_TYPE_DAZE];
	if (!daze_list || daze_list->size(true) == 0)
		return;

	daze_list->Remove(spell);
}

bool Entity::IsDazeImmune(){
	return (immunities[IMMUNITY_TYPE_DAZE] && immunities[IMMUNITY_TYPE_DAZE]->size(true) > 0);
}

void Entity::AddImmunity(LuaSpell* spell, int8 type){
	if (!spell)
		return;

	if (!immunities[type])
		immunities[type] = new MutexList<LuaSpell*>;

	immunities[type]->Add(spell);
}

void Entity::RemoveImmunity(LuaSpell* spell, int8 type){
	MutexList<LuaSpell*>* list = immunities[type];
	if (!list || list->size(true) == 0)
		return;

	list->Remove(spell);
}

bool Entity::IsImmune(int8 type){
	return (immunities[type] && immunities[type]->size(true) > 0);
}

void Entity::RemoveEffectsFromLuaSpell(LuaSpell* spell){
	if (!spell)
		return;

	//Attempt to remove all applied effects from this spell when spell has been removed from just this target. Should improve performance/easier maitenance
	int32 effect_bitmask = spell->effect_bitmask;
	if (effect_bitmask == 0)
		return;

	if (effect_bitmask & EFFECT_FLAG_STUN)
		RemoveStunSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_ROOT)
		RemoveRootSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_MEZ)
		RemoveMezSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_STIFLE)
		RemoveStifleSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_DAZE)
		RemoveDazeSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_FEAR)
		RemoveFearSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_SPELLBONUS)
		RemoveSpellBonus(spell);
	if (effect_bitmask & EFFECT_FLAG_SKILLBONUS)
		RemoveSkillBonus(spell->spell->GetSpellID());
	if (effect_bitmask & EFFECT_FLAG_STEALTH)
		RemoveStealthSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_INVIS)
		RemoveInvisSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_SNARE)
		RemoveSnareSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_WATERWALK)
		RemoveWaterwalkSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_WATERJUMP)
		RemoveWaterjumpSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_FLIGHT)
		RemoveFlightSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_GLIDE)
		RemoveGlideSpell(spell);
	if (effect_bitmask & EFFECT_FLAG_AOE_IMMUNE)
		RemoveAOEImmunity(spell);
	if (effect_bitmask & EFFECT_FLAG_STUN_IMMUNE)
		RemoveStunImmunity(spell);
	if (effect_bitmask & EFFECT_FLAG_MEZ_IMMUNE)
		RemoveMezImmunity(spell);
	if (effect_bitmask & EFFECT_FLAG_DAZE_IMMUNE)
		RemoveDazeImmunity(spell);
	if (effect_bitmask & EFFECT_FLAG_ROOT_IMMUNE)
		RemoveRootImmunity(spell);
	if (effect_bitmask & EFFECT_FLAG_STIFLE_IMMUNE)
		RemoveStifleImmunity(spell);
	if (effect_bitmask & EFFECT_FLAG_FEAR_IMMUNE)
		RemoveFearImmunity(spell);
	if (effect_bitmask & EFFECT_FLAG_SAFEFALL)
		RemoveSafefallSpell(spell);
}

void Entity::RemoveSkillBonus(int32 spell_id){
	//This is a virtual, just making it so we don't have to do extra checks for player/npcs
	return;
}

void Entity::AddFlightSpell(LuaSpell* spell){
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_FLIGHT])
		control_effects[CONTROL_EFFECT_TYPE_FLIGHT] = new MutexList<LuaSpell*>;

	if (IsPlayer() && control_effects[CONTROL_EFFECT_TYPE_FLIGHT]->size(true) == 0)
		((Player*)this)->SetPlayerControlFlag(5, 32, true);

	control_effects[CONTROL_EFFECT_TYPE_FLIGHT]->Add(spell);
}

void Entity::RemoveFlightSpell(LuaSpell* spell){
	MutexList<LuaSpell*>* flight_list = control_effects[CONTROL_EFFECT_TYPE_FLIGHT];
	if (!flight_list || flight_list->size(true) == 0)
		return;

	flight_list->Remove(spell);
	if (IsPlayer() && flight_list->size(true) == 0)
		((Player*)this)->SetPlayerControlFlag(5, 32, false);
}

void Entity::AddGlideSpell(LuaSpell* spell){
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_GLIDE])
		control_effects[CONTROL_EFFECT_TYPE_GLIDE] = new MutexList<LuaSpell*>;

	if (IsPlayer() && control_effects[CONTROL_EFFECT_TYPE_GLIDE]->size(true) == 0)
		((Player*)this)->SetPlayerControlFlag(4, 16, true);

	control_effects[CONTROL_EFFECT_TYPE_GLIDE]->Add(spell);
}

void Entity::RemoveGlideSpell(LuaSpell* spell){
	MutexList<LuaSpell*>* glide_list = control_effects[CONTROL_EFFECT_TYPE_GLIDE];
	if (!glide_list || glide_list->size(true) == 0)
		return;

	glide_list->Remove(spell);
	if (IsPlayer() && glide_list->size(true) == 0)
		((Player*)this)->SetPlayerControlFlag(4, 16, false);
}

void Entity::AddSafefallSpell(LuaSpell* spell){
	if (!spell)
		return;

	if (!control_effects[CONTROL_EFFECT_TYPE_SAFEFALL])
		control_effects[CONTROL_EFFECT_TYPE_SAFEFALL] = new MutexList<LuaSpell*>;

	if (IsPlayer() && control_effects[CONTROL_EFFECT_TYPE_SAFEFALL]->size(true) == 0)
		((Player*)this)->SetPlayerControlFlag(4, 32, true);

	control_effects[CONTROL_EFFECT_TYPE_SAFEFALL]->Add(spell);
}

void Entity::RemoveSafefallSpell(LuaSpell* spell){
	MutexList<LuaSpell*>* safe_list = control_effects[CONTROL_EFFECT_TYPE_SAFEFALL];
	if (!safe_list || safe_list->size(true) == 0)
		return;

	safe_list->Remove(spell);
	if (IsPlayer() && safe_list->size(true) == 0)
		((Player*)this)->SetPlayerControlFlag(4, 32, false);
}

void Entity::UpdateGroupMemberInfo(bool inGroupMgrLock, bool groupMembersLocked) {
	if (!group_member_info || group_id == 0)
		return;

	if(!inGroupMgrLock)
		world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);

	PlayerGroup* group = world.GetGroupManager()->GetGroup(group_id);

	if (group)
		group->UpdateGroupMemberInfo(this, groupMembersLocked);

	if(!inGroupMgrLock)
		world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
}

#include "WorldDatabase.h"
extern WorldDatabase database;
void Entity::CustomizeAppearance(PacketStruct* packet) {

	bool		is_soga						= packet->getType_int8_ByName("is_soga") == 1 ? true : false;
	int16		model_id					= database.GetAppearanceID(packet->getType_EQ2_16BitString_ByName("race_file").data);
	EQ2_Color	skin_color					= packet->getType_EQ2_Color_ByName("skin_color");
	EQ2_Color	skin_color2					= packet->getType_EQ2_Color_ByName("skin_color2");
	EQ2_Color	eye_color					= packet->getType_EQ2_Color_ByName("eye_color");
	EQ2_Color	hair_color1					= packet->getType_EQ2_Color_ByName("hair_color1");
	EQ2_Color	hair_color2					= packet->getType_EQ2_Color_ByName("hair_color2");
	EQ2_Color	hair_highlight				= packet->getType_EQ2_Color_ByName("hair_highlight");
	int16		hair_id						= database.GetAppearanceID(packet->getType_EQ2_16BitString_ByName("hair_file").data);
	EQ2_Color	hair_type_color				= packet->getType_EQ2_Color_ByName("hair_type_color");
	EQ2_Color	hair_type_highlight_color	= packet->getType_EQ2_Color_ByName("hair_type_highlight_color");
	int16		face_id						= database.GetAppearanceID(packet->getType_EQ2_16BitString_ByName("face_file").data);
	EQ2_Color	hair_face_color				= packet->getType_EQ2_Color_ByName("hair_face_color");
	EQ2_Color	hair_face_highlight_color	= packet->getType_EQ2_Color_ByName("hair_face_highlight_color");
	int16		wing_id						= database.GetAppearanceID(packet->getType_EQ2_16BitString_ByName("wing_file").data);
	EQ2_Color	wing_color1					= packet->getType_EQ2_Color_ByName("wing_color1");
	EQ2_Color	wing_color2					= packet->getType_EQ2_Color_ByName("wing_color2");
	int16		chest_id					= database.GetAppearanceID(packet->getType_EQ2_16BitString_ByName("chest_file").data);
	EQ2_Color	shirt_color					= packet->getType_EQ2_Color_ByName("shirt_color");
	EQ2_Color	unknown_chest_color			= packet->getType_EQ2_Color_ByName("unknown_chest_color");
	int16		legs_id						= database.GetAppearanceID(packet->getType_EQ2_16BitString_ByName("legs_file").data);
	EQ2_Color	pants_color					= packet->getType_EQ2_Color_ByName("pants_color");
	EQ2_Color	unknown_legs_color			= packet->getType_EQ2_Color_ByName("unknown_legs_color");
	EQ2_Color	unknown2					= packet->getType_EQ2_Color_ByName("unknown2");

	float eyes2[3];
	eyes2[0] = packet->getType_float_ByName("eyes2", 0) * 100;
	eyes2[1] = packet->getType_float_ByName("eyes2", 1) * 100;
	eyes2[2] = packet->getType_float_ByName("eyes2", 2) * 100;

	float ears[3];
	ears[0] = packet->getType_float_ByName("ears", 0) * 100;
	ears[1] = packet->getType_float_ByName("ears", 1) * 100;
	ears[2] = packet->getType_float_ByName("ears", 2) * 100;

	float eye_brows[3];
	eye_brows[0] = packet->getType_float_ByName("eye_brows", 0) * 100;
	eye_brows[1] = packet->getType_float_ByName("eye_brows", 1) * 100;
	eye_brows[2] = packet->getType_float_ByName("eye_brows", 2) * 100;

	float cheeks[3];
	cheeks[0] = packet->getType_float_ByName("cheeks", 0) * 100;
	cheeks[1] = packet->getType_float_ByName("cheeks", 1) * 100;
	cheeks[2] = packet->getType_float_ByName("cheeks", 2) * 100;

	float lips[3];
	lips[0] = packet->getType_float_ByName("lips", 0) * 100;
	lips[1] = packet->getType_float_ByName("lips", 1) * 100;
	lips[2] = packet->getType_float_ByName("lips", 2) * 100;

	float chin[3];
	chin[0] = packet->getType_float_ByName("chin", 0) * 100;
	chin[1] = packet->getType_float_ByName("chin", 1) * 100;
	chin[2] = packet->getType_float_ByName("chin", 2) * 100;

	float nose[3];
	nose[0] = packet->getType_float_ByName("nose", 0) * 100;
	nose[1] = packet->getType_float_ByName("nose", 1) * 100;
	nose[2] = packet->getType_float_ByName("nose", 2) * 100;

	sint8 body_size = (sint8)(packet->getType_float_ByName("body_size") * 100);
	sint8 body_age = (sint8)(packet->getType_float_ByName("body_age") * 100);

	if (is_soga) {
		appearance.soga_model_type = model_id;
		features.soga_skin_color = skin_color;
		features.soga_eye_color = eye_color;
		features.soga_hair_color1 = hair_color1;
		features.soga_hair_color2 = hair_color2;
		features.soga_hair_highlight_color = hair_highlight;
		features.soga_hair_type = hair_id;
		features.soga_hair_type_color = hair_type_color;
		features.soga_hair_type_highlight_color = hair_type_highlight_color;
		features.soga_hair_face_type = face_id;
		features.soga_hair_face_color = hair_face_color;
		features.soga_hair_face_highlight_color = hair_face_highlight_color;
		features.wing_type = wing_id;
		features.wing_color1 = wing_color1;
		features.wing_color2 = wing_color2;
		features.soga_chest_type = chest_id;
		features.shirt_color = shirt_color;
		features.soga_legs_type = legs_id;
		features.pants_color = pants_color;
		features.soga_eye_type[0] = eyes2[0];
		features.soga_eye_type[1] = eyes2[1];
		features.soga_eye_type[2] = eyes2[2];
		features.soga_ear_type[0] = ears[0];
		features.soga_ear_type[0] = ears[1];
		features.soga_ear_type[0] = ears[2];
		features.soga_eye_brow_type[0] = eye_brows[0];
		features.soga_eye_brow_type[1] = eye_brows[1];
		features.soga_eye_brow_type[2] = eye_brows[2];
		features.soga_cheek_type[0] = cheeks[0];
		features.soga_cheek_type[1] = cheeks[1];
		features.soga_cheek_type[2] = cheeks[2];
		features.soga_lip_type[0] = lips[0];
		features.soga_lip_type[1] = lips[1];
		features.soga_lip_type[2] = lips[2];
		features.soga_chin_type[0] = chin[0];
		features.soga_chin_type[1] = chin[1];
		features.soga_chin_type[2] = chin[2];
		features.soga_nose_type[0] = nose[0];
		features.soga_nose_type[1] = nose[1];
		features.soga_nose_type[2] = nose[2];
	}
	else {
		appearance.model_type = model_id;
		features.skin_color = skin_color;
		features.eye_color = eye_color;
		features.hair_color1 = hair_color1;
		features.hair_color2 = hair_color2;
		features.hair_highlight_color = hair_highlight;
		features.hair_type = hair_id;
		features.hair_type_color = hair_type_color;
		features.hair_type_highlight_color = hair_type_highlight_color;
		features.hair_face_type = face_id;
		features.hair_face_color = hair_face_color;
		features.hair_face_highlight_color = hair_face_highlight_color;
		features.wing_type = wing_id;
		features.wing_color1 = wing_color1;
		features.wing_color2 = wing_color2;
		features.chest_type = chest_id;
		features.shirt_color = shirt_color;
		features.legs_type = legs_id;
		features.pants_color = pants_color;
		features.eye_type[0] = eyes2[0];
		features.eye_type[1] = eyes2[1];
		features.eye_type[2] = eyes2[2];
		features.ear_type[0] = ears[0];
		features.ear_type[0] = ears[1];
		features.ear_type[0] = ears[2];
		features.eye_brow_type[0] = eye_brows[0];
		features.eye_brow_type[1] = eye_brows[1];
		features.eye_brow_type[2] = eye_brows[2];
		features.cheek_type[0] = cheeks[0];
		features.cheek_type[1] = cheeks[1];
		features.cheek_type[2] = cheeks[2];
		features.lip_type[0] = lips[0];
		features.lip_type[1] = lips[1];
		features.lip_type[2] = lips[2];
		features.chin_type[0] = chin[0];
		features.chin_type[1] = chin[1];
		features.chin_type[2] = chin[2];
		features.nose_type[0] = nose[0];
		features.nose_type[1] = nose[1];
		features.nose_type[2] = nose[2];
	}

	features.body_size = body_size;
	features.body_age = body_age;
	features.soga_body_size = body_size;
	features.soga_body_age = body_age;
	info_changed = true;
	changed = true;
}

void Entity::AddSkillBonus(int32 spell_id, int32 skill_id, float value) {
	// handled in npc or player
	return;
}

bool Entity::HasControlEffect(int8 type)
{
	if (type >= CONTROL_MAX_EFFECTS)
		return false;

	MutexList<LuaSpell*>* spell_list = control_effects[type];
	if (!spell_list || spell_list->size(true) == 0)
		return false;

	return true;
}

void Entity::HaltMovement()
{
	this->ClearRunningLocations();

	if (GetZone())
		GetZone()->movementMgr->StopNavigation(this);

	RunToLocation(GetX(), GetY(), GetZ());
}	

std::string Entity::GetInfoStructString(std::string field)
{
		map<string, boost::function<std::string()>>::const_iterator itr = get_string_funcs.find(field);
		if(itr != get_string_funcs.end())
		{
			auto func = (itr->second)();
			return func;
		}
	return std::string("");
}

int8 Entity::GetInfoStructInt8(std::string field)
{
		map<string, boost::function<int8()>>::const_iterator itr = get_int8_funcs.find(field);
		if(itr != get_int8_funcs.end())
		{
			auto func = (itr->second)();
			return func;
		}
	return 0;
}

int16 Entity::GetInfoStructInt16(std::string field)
{
		map<string, boost::function<int16()>>::const_iterator itr = get_int16_funcs.find(field);
		if(itr != get_int16_funcs.end())
		{
			auto func = (itr->second)();
			return func;
		}
	return 0;
}

int32 Entity::GetInfoStructInt32(std::string field)
{
		map<string, boost::function<int32()>>::const_iterator itr = get_int32_funcs.find(field);
		if(itr != get_int32_funcs.end())
		{
			auto func = (itr->second)();
			return func;
		}
	return 0;
}

int64 Entity::GetInfoStructInt64(std::string field)
{
		map<string, boost::function<int64()>>::const_iterator itr = get_int64_funcs.find(field);
		if(itr != get_int64_funcs.end())
		{
			auto func = (itr->second)();
			return func;
		}
	return 0;
}

sint8 Entity::GetInfoStructSInt8(std::string field)
{
		map<string, boost::function<sint8()>>::const_iterator itr = get_sint8_funcs.find(field);
		if(itr != get_sint8_funcs.end())
		{
			auto func = (itr->second)();
			return func;
		}
	return 0;
}

sint16 Entity::GetInfoStructSInt16(std::string field)
{
		map<string, boost::function<sint16()>>::const_iterator itr = get_sint16_funcs.find(field);
		if(itr != get_sint16_funcs.end())
		{
			auto func = (itr->second)();
			return func;
		}
	return 0;
}

sint32 Entity::GetInfoStructSInt32(std::string field)
{
		map<string, boost::function<sint32()>>::const_iterator itr = get_sint32_funcs.find(field);
		if(itr != get_sint32_funcs.end())
		{
			auto func = (itr->second)();
			return func;
		}
	return 0;
}

sint64 Entity::GetInfoStructSInt64(std::string field)
{
		map<string, boost::function<sint64()>>::const_iterator itr = get_sint64_funcs.find(field);
		if(itr != get_sint64_funcs.end())
		{
			auto func = (itr->second)();
			return func;
		}
	return 0;
}

float Entity::GetInfoStructFloat(std::string field)
{
		map<string, boost::function<float()>>::const_iterator itr = get_float_funcs.find(field);
		if(itr != get_float_funcs.end())
		{
			auto func = (itr->second)();
			return func;
		}
	return 0.0f;
}

int64 Entity::GetInfoStructUInt(std::string field)
{
		map<string, boost::function<int8()>>::const_iterator itr = get_int8_funcs.find(field);
		if(itr != get_int8_funcs.end())
		{
			auto func = (itr->second)();
			return func;
		}
		map<string, boost::function<int16()>>::const_iterator itr2 = get_int16_funcs.find(field);
		if(itr2 != get_int16_funcs.end())
		{
			auto func = (itr2->second)();
			return func;
		}
		map<string, boost::function<int32()>>::const_iterator itr3 = get_int32_funcs.find(field);
		if(itr3 != get_int32_funcs.end())
		{
			auto func = (itr3->second)();
			return func;
		}
		map<string, boost::function<int64()>>::const_iterator itr4 = get_int64_funcs.find(field);
		if(itr4 != get_int64_funcs.end())
		{
			auto func = (itr4->second)();
			return func;
		}
	return 0;
}

sint64 Entity::GetInfoStructSInt(std::string field)
{
		map<string, boost::function<sint8()>>::const_iterator itr = get_sint8_funcs.find(field);
		if(itr != get_sint8_funcs.end())
		{
			auto func = (itr->second)();
			return func;
		}
		map<string, boost::function<sint16()>>::const_iterator itr2 = get_sint16_funcs.find(field);
		if(itr2 != get_sint16_funcs.end())
		{
			auto func = (itr2->second)();
			return func;
		}
		map<string, boost::function<sint32()>>::const_iterator itr3 = get_sint32_funcs.find(field);
		if(itr3 != get_sint32_funcs.end())
		{
			auto func = (itr3->second)();
			return func;
		}
		map<string, boost::function<sint64()>>::const_iterator itr4 = get_sint64_funcs.find(field);
		if(itr4 != get_sint64_funcs.end())
		{
			auto func = (itr4->second)();
			return func;
		}
	return 0;
}


bool Entity::SetInfoStructString(std::string field, std::string value)
{
		map<string, boost::function<void(std::string)>>::const_iterator itr = set_string_funcs.find(field);
		if(itr != set_string_funcs.end())
		{
			(itr->second)(value);
			return true;
		}
	return false;
}


bool Entity::SetInfoStructUInt(std::string field, int64 value)
{
		map<string, boost::function<void(int8)>>::const_iterator itr = set_int8_funcs.find(field);
		if(itr != set_int8_funcs.end())
		{
			(itr->second)((int8)value);
			return true;
		}
		map<string, boost::function<void(int16)>>::const_iterator itr2 = set_int16_funcs.find(field);
		if(itr2 != set_int16_funcs.end())
		{
			(itr2->second)((int16)value);
			return true;
		}
		map<string, boost::function<void(int32)>>::const_iterator itr3 = set_int32_funcs.find(field);
		if(itr3 != set_int32_funcs.end())
		{
			(itr3->second)((int32)value);
			return true;
		}
		map<string, boost::function<void(int64)>>::const_iterator itr4 = set_int64_funcs.find(field);
		if(itr4 != set_int64_funcs.end())
		{
			(itr4->second)(value);
			return true;
		}
	return false;
}

bool Entity::SetInfoStructSInt(std::string field, sint64 value)
{
		map<string, boost::function<void(sint8)>>::const_iterator itr = set_sint8_funcs.find(field);
		if(itr != set_sint8_funcs.end())
		{
			(itr->second)((sint8)value);
			return true;
		}
		map<string, boost::function<void(sint16)>>::const_iterator itr2 = set_sint16_funcs.find(field);
		if(itr2 != set_sint16_funcs.end())
		{
			(itr2->second)((sint16)value);
			return true;
		}
		map<string, boost::function<void(sint32)>>::const_iterator itr3 = set_sint32_funcs.find(field);
		if(itr3 != set_sint32_funcs.end())
		{
			(itr3->second)((sint32)value);
			return true;
		}
		map<string, boost::function<void(sint64)>>::const_iterator itr4 = set_sint64_funcs.find(field);
		if(itr4 != set_sint64_funcs.end())
		{
			(itr4->second)(value);
			return true;
		}
	return false;
}

bool Entity::SetInfoStructFloat(std::string field, float value)
{
		map<string, boost::function<void(float)>>::const_iterator itr = set_float_funcs.find(field);
		if(itr != set_float_funcs.end())
		{
			(itr->second)(value);
			return true;
		}
	return false;
}

Entity*	Entity::GetOwner() {
	Entity* ent = nullptr;

	if(!GetZone()) {
		return ent;
	}
	Spawn* spawn = GetZone()->GetSpawnByID(owner);
	if ( spawn && spawn->IsEntity() )
		ent = (Entity*)spawn;

	return ent;
}

bool Entity::IsEngagedInEncounter(Spawn** res) {
	if(res) {
		*res = nullptr;
	}
	bool ret = false;
	set<int32>::iterator itr;
	MHatedBy.lock();
	if(IsPlayer()) {
		for (itr = HatedBy.begin(); itr != HatedBy.end(); itr++) {
			Spawn* spawn = GetZone()->GetSpawnByID(*itr);
			if (spawn && spawn->IsNPC() && ((NPC*)spawn)->Brain() && (spawn->GetLockedNoLoot() == ENCOUNTER_STATE_LOCKED || spawn->GetLockedNoLoot() == ENCOUNTER_STATE_OVERMATCHED)) {
				if((ret = ((NPC*)spawn)->Brain()->IsPlayerInEncounter(((Player*)this)->GetCharacterID()))) {
					if(res)
						*res = spawn;
					break;
				}
			}
		}
	}
	else {
		for (itr = HatedBy.begin(); itr != HatedBy.end(); itr++) {
			Spawn* spawn = GetZone()->GetSpawnByID(*itr);
			if (spawn && spawn->IsNPC() && ((NPC*)spawn)->Brain() && (spawn->GetLockedNoLoot() == ENCOUNTER_STATE_LOCKED || spawn->GetLockedNoLoot() == ENCOUNTER_STATE_OVERMATCHED)) {
				if((ret = ((NPC*)spawn)->Brain()->IsEntityInEncounter(GetID()))) {
					if(res)
						*res = spawn;
					break;
				}
			}
		}

	}
	MHatedBy.unlock();
	
	return ret;
}

bool Entity::IsEngagedBySpawnID(int32 id) {
	bool ret = false;
	Spawn* spawn = GetZone()->GetSpawnByID(id);
	if (spawn && spawn->IsNPC() && ((NPC*)spawn)->Brain()) {
		ret = ((NPC*)spawn)->Brain()->IsEntityInEncounter(GetID());
	}
	
	return ret;
}

void Entity::SendControlEffectDetailsToClient(Client* client) {
	client->Message(CHANNEL_COLOR_YELLOW, "Current control effects on %s", GetName());
	client->Message(CHANNEL_COLOR_YELLOW, "-------------------------------");
	for (int i = 0; i < CONTROL_MAX_EFFECTS; i++) {
		if(control_effects[i]) {
			MutexList<LuaSpell*>* spells = control_effects[i];
			if(spells->size() > 0) {
				MutexList<LuaSpell*>::iterator itr = spells->begin();		
				while(itr.Next()){
					LuaSpell* spell = itr->value;
					if(spell && spell->spell && spell->spell->GetSpellData()) {
						client->Message(CHANNEL_COLOR_YELLOW, "Spell %s (%u) control effect %s", spell->spell->GetName(), spell->spell->GetSpellData()->id, GetControlEffectName(i).c_str());
					}
				}
			}
		}
	}
	client->Message(CHANNEL_COLOR_YELLOW, "-------------------------------");
}

void Entity::TerminateTrade() {
	Trade* tmpTradePtr = trade;
	if (tmpTradePtr) {
		tmpTradePtr->CancelTrade(this);
		safe_delete(tmpTradePtr);
	}
}

void Entity::CalculateMaxReduction() {
	if(GetInfoStruct()->get_max_spell_reduction_override()) {
		return; // override enabled, don't touch the max reduction
	}
	int16 effective_level = GetInfoStruct()->get_effective_level() != 0 ? GetInfoStruct()->get_effective_level() : GetLevel();
	// Define thresholds and corresponding maximum reduction factors
	const int thresholds[] = {1, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120}; // Thresholds for level differences
	const float maxReductions[] = {0.2f, 0.2f, 0.15f, 0.15f, 0.1f,0.1f,0.1f,0.1f,0.1f,0.075f,0.075f,0.075f,0.05f,0.05f,0.05f,0.05f,0.05f,0.05f,0.045f,0.045f,0.045f,0.045f,0.045f,0.045f,0.045f}; // Maximum reduction factors for each threshold
	int numThresholds = sizeof(thresholds) / sizeof(thresholds[0]);

	// Find the appropriate maximum reduction factor based on level difference
	float maxReduction = .1f; // Default maximum reduction factor
	for (int i = 0; i < numThresholds; ++i) {
		if (effective_level >= thresholds[i]) {
			maxReduction = maxReductions[i];
		}
		else {
			break; // No need to check further
		}
	}

	GetInfoStruct()->set_max_spell_reduction(maxReduction);
}