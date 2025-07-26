/*  
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2005 - 2026  EQ2EMulator Development Team (http://www.eq2emu.com formerly http://www.eq2emulator.net)

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
#ifndef __EQ2_ENTITY__
#define __EQ2_ENTITY__
#include "Spawn.h"
#include "../common/Mutex.h"
#include "Skills.h"
#include "MutexList.h"
#include "MutexVector.h"
#include "Trade.h"
#include <set>
#include <mutex>
#include <vector>
#include <boost/function.hpp>
#include <boost/lambda/bind.hpp>

namespace l = boost::lambda;

class Entity;
class NPC;
class Trade;
struct LuaSpell;
struct GroupMemberInfo;

struct BonusValues{
	int32	spell_id;
	int8    tier;
	int16	type;
	float	value;
	int64	class_req;
	vector<int16>	race_req;
	vector<int16>	faction_req;
	LuaSpell* luaspell;
};

struct MaintainedEffects{
	char	name[60]; //name of the spell
	int32	target;
	int8	target_type;
	int32	spell_id; 
	int32	inherited_spell_id; 
	int32	slot_pos; 
	int16	icon;
	int16	icon_backdrop;
	int8	conc_used;
	int8	tier;
	float	total_time;
	int32	expire_timestamp;
	LuaSpell* spell;
};

struct SpellEffects{
	int32	spell_id; 
	int32	inherited_spell_id; 
	Entity*	caster;
	float	total_time;
	int32	expire_timestamp;
	int16	icon;
	int16	icon_backdrop;
	int8	tier;
	LuaSpell* spell;
};

struct DetrimentalEffects {
	int32	spell_id; 
	int32	inherited_spell_id; 
	Entity*	caster;
	int32	expire_timestamp;
	int16	icon;
	int16	icon_backdrop;
	int8	tier;
	int8    det_type;
	bool    incurable;
	LuaSpell*  spell;
	int8    control_effect;
	float   total_time;
};

enum RACE_ALIGNMENT {
	ALIGNMENT_EVIL=0,
	ALIGNMENT_GOOD=1
	// neutral?
};
struct InfoStruct{
	InfoStruct()
	{
		name_ = std::string("");
		class1_ = 0;
		class2_ = 0;
		class3_ = 0;
		race_ = 0;
		gender_ = 0;
		level_ = 0;
		max_level_ = 0;
		effective_level_ = 0;
		tradeskill_level_ = 0;
		tradeskill_max_level_ = 0;
		cur_concentration_ = 0;
		max_concentration_ = 0;
		cur_attack_ = 0;
		attack_base_ = 0;
		cur_mitigation_ = 0;
		max_mitigation_ = 0;
		mitigation_base_ = 0;
		mitigation_modifier_ = 0;
		avoidance_display_ = 0;
		cur_avoidance_ = 0.0f;
		base_avoidance_pct_ = 0;
		avoidance_base_ = 0;
		max_avoidance_ = 0;
		parry_ = 0.0f;
		parry_base_ = 0.0f;
		deflection_ = 0;
		deflection_base_ = 0;
		block_ = 0;
		block_base_ = 0;
		str_ = 0.0f;
		sta_ = 0.0f;
		agi_ = 0.0f;
		wis_ = 0.0f;
		intel_ = 0.0f;
		str_base_ = 0.0f;
		sta_base_ = 0.0f;
		agi_base_ = 0.0f;
		wis_base_ = 0.0f;
		intel_base_ = 0.0f;
		heat_ = 0;
		cold_ = 0;
		magic_ = 0;
		mental_ = 0;
		divine_ = 0;
		disease_ = 0;
		poison_ = 0;
		disease_base_ = 0;
		cold_base_ = 0;
		divine_base_ = 0;
		magic_base_ = 0;
		mental_base_ = 0;
		heat_base_ = 0;
		poison_base_ = 0;
		elemental_base_ = 0;
		noxious_base_ = 0;
		arcane_base_ = 0;
		coin_copper_ = 0;
		coin_silver_ = 0;
		coin_gold_ = 0;
		coin_plat_ = 0;
		bank_coin_copper_ = 0;
		bank_coin_silver_ = 0;
		bank_coin_gold_ = 0;
		bank_coin_plat_ = 0;
		status_points_ = 0;
		deity_ = std::string("");
		weight_ = 0;
		max_weight_ = 0;
		tradeskill_class1_ = 0;
		tradeskill_class2_ = 0;
		tradeskill_class3_ = 0;
		account_age_base_ = 0;

		memset(account_age_bonus_,0,19);
		absorb_ = 0;
		xp_ = 0;
		xp_needed_ = 0;
		xp_debt_ = 0.0f;
		xp_yellow_ = 0;
		xp_yellow_vitality_bar_ = 0;
		xp_blue_vitality_bar_ = 0;
		xp_blue_ = 0;
		ts_xp_ = 0;
		ts_xp_needed_ = 0;
		tradeskill_exp_yellow_ = 0;
		tradeskill_exp_blue_ = 0;
		flags_ = 0;
		flags2_ = 0;
		xp_vitality_ = 0;
		tradeskill_xp_vitality_ = 0;
		mitigation_skill1_ = 0;
		mitigation_skill2_ = 0;
		mitigation_skill3_ = 0;
		mitigation_pve_ = 0;
		mitigation_pvp_ = 0;
		ability_modifier_ = 0;
		critical_mitigation_ = 0;
		block_chance_ = 0;
		uncontested_parry_ = 0;
		uncontested_block_ = 0;
		uncontested_dodge_ = 0;
		uncontested_riposte_ = 0;
		crit_chance_ = 0;
		crit_bonus_ = 0;
		potency_ = 0;
		hate_mod_ = 0;
		reuse_speed_ = 0;
		casting_speed_ = 0;
		recovery_speed_ = 0;
		spell_reuse_speed_ = 0;
		spell_multi_attack_ = 0;
		size_mod_ = 0.0f;
		ignore_size_mod_calc_ = 0;
		dps_ = 0;
		dps_multiplier_ = 0;
		attackspeed_ = 0;
		haste_ = 0;
		multi_attack_ = 0;
		flurry_ = 0;
		melee_ae_ = 0;
		strikethrough_ = 0;
		accuracy_ = 0;
		offensivespeed_ = 0;
		rain_ = 0;
		wind_ = 0;
		alignment_ = 0;
		pet_id_ = 0;
		pet_name_ = std::string("");
		pet_health_pct_ = 0.0f;
		pet_power_pct_ = 0.0f;
		pet_movement_ = 0;
		pet_behavior_ = 0;
		vision_ = 0;
		breathe_underwater_ = 0;
		biography_ = std::string("");
		drunk_ = 0;
		power_regen_ = 0;
		hp_regen_ = 0;

		power_regen_override_ = 0;
		hp_regen_override_ = 0;

		water_type_ = 0;
		flying_type_ = 0;

		no_interrupt_ = 0;

		interaction_flag_ = 0;
		tag1_ = 0;
		mood_ = 0;
		
		range_last_attack_time_ = 0;
		primary_last_attack_time_ = 0;
		secondary_last_attack_time_ = 0;
		primary_attack_delay_ = 0;
		secondary_attack_delay_ = 0;
		ranged_attack_delay_ = 0;
		primary_weapon_type_ = 0;
		secondary_weapon_type_ = 0;
		ranged_weapon_type_ = 0;
		primary_weapon_damage_low_ = 0;
		primary_weapon_damage_high_ = 0;
		secondary_weapon_damage_low_ = 0;
		secondary_weapon_damage_high_ = 0;
		ranged_weapon_damage_low_ = 0;
		ranged_weapon_damage_high_ = 0;
		wield_type_ = 0;
		attack_type_ = 0;
		primary_weapon_delay_ = 0;
		secondary_weapon_delay_ = 0;
		ranged_weapon_delay_ = 0;
		
		override_primary_weapon_ = 0;
		override_secondary_weapon_ = 0;
		override_ranged_weapon_ = 0;
		
		friendly_target_npc_ = 0;
		last_claim_time_ = 0;
		
		engaged_encounter_ = 0;
		lockable_encounter_ = 1;
		
		first_world_login_ = 0;
		reload_player_spells_ = 0;
		
		group_loot_method_ = 1;
		group_loot_items_rarity_ = 0;
		group_auto_split_ = 1;
		group_default_yell_ = 1;
		group_autolock_ = 0;
		group_lock_method_ = 0;
		group_solo_autolock_ = 0;
		group_auto_loot_method_ = 0;
		assist_auto_attack_ = 0;
		
		action_state_ = std::string("");
		combat_action_state_ = std::string("");
		
		max_spell_reduction_ = .1f;
		max_spell_reduction_override_ = 0;
		max_chase_distance_ = 0.0f;
	}


	void SetInfoStruct(InfoStruct* oldStruct)
	{
		if(!oldStruct)
			return;
		std::lock_guard<std::mutex> lk(classMutex);

		name_ = std::string(oldStruct->get_name());
		class1_ = oldStruct->get_class1();
		class2_ = oldStruct->get_class2();
		class3_ = oldStruct->get_class3();
		race_ = oldStruct->get_race();
		gender_ = oldStruct->get_gender();
		level_ = oldStruct->get_level();
		max_level_ = oldStruct->get_max_level();
		effective_level_ = oldStruct->get_effective_level();
		tradeskill_level_ = oldStruct->get_tradeskill_level();
		tradeskill_max_level_ = oldStruct->get_tradeskill_max_level();
		cur_concentration_ = oldStruct->get_cur_concentration();
		max_concentration_ = oldStruct->get_max_concentration();
		cur_attack_ = oldStruct->get_cur_attack();
		attack_base_ = oldStruct->get_attack_base();
		cur_mitigation_ = oldStruct->get_cur_mitigation();
		max_mitigation_ = oldStruct->get_max_mitigation();
		mitigation_base_ = oldStruct->get_mitigation_base();
		mitigation_modifier_ = oldStruct->get_mitigation_modifier();
		avoidance_display_ = oldStruct->get_avoidance_display();
		cur_avoidance_ = oldStruct->get_cur_avoidance();
		base_avoidance_pct_ = oldStruct->get_base_avoidance_pct();
		avoidance_base_ = oldStruct->get_avoidance_base();
		max_avoidance_ = oldStruct->get_max_avoidance();
		parry_ = oldStruct->get_parry();
		parry_base_ = oldStruct->get_parry_base();
		deflection_ = oldStruct->get_deflection();
		deflection_base_ = oldStruct->get_deflection_base();
		block_ = oldStruct->get_block();
		block_base_ = oldStruct->get_block_base();
		str_ = oldStruct->get_str();
		sta_ = oldStruct->get_sta();
		agi_ = oldStruct->get_agi();
		wis_ = oldStruct->get_wis();
		intel_ = oldStruct->get_intel();
		str_base_ = oldStruct->get_str_base();
		sta_base_ = oldStruct->get_sta_base();
		agi_base_ = oldStruct->get_agi_base();
		wis_base_ = oldStruct->get_wis_base();
		intel_base_ = oldStruct->get_intel_base();
		heat_ = oldStruct->get_heat();
		cold_ = oldStruct->get_cold();
		magic_ = oldStruct->get_magic();
		mental_ = oldStruct->get_mental();
		divine_ = oldStruct->get_divine();
		disease_ = oldStruct->get_disease();
		poison_ = oldStruct->get_poison();
		disease_base_ = oldStruct->get_disease_base();
		cold_base_ = oldStruct->get_cold_base();
		divine_base_ = oldStruct->get_divine_base();
		magic_base_ = oldStruct->get_magic_base();
		mental_base_ = oldStruct->get_mental_base();
		heat_base_ = oldStruct->get_heat_base();
		poison_base_ = oldStruct->get_poison_base();
		elemental_base_ = oldStruct->get_elemental_base();
		noxious_base_ = oldStruct->get_noxious_base();
		arcane_base_ = oldStruct->get_arcane_base();
		coin_copper_ = oldStruct->get_coin_copper();
		coin_silver_ = oldStruct->get_coin_silver();
		coin_gold_ = oldStruct->get_coin_gold();
		coin_plat_ = oldStruct->get_coin_plat();
		bank_coin_copper_ = oldStruct->get_bank_coin_copper();
		bank_coin_silver_ = oldStruct->get_bank_coin_silver();
		bank_coin_gold_ = oldStruct->get_bank_coin_gold();
		bank_coin_plat_ = oldStruct->get_bank_coin_plat();
		status_points_ = oldStruct->get_status_points();
		deity_ = std::string("");
		weight_ = oldStruct->get_weight();
		max_weight_ = oldStruct->get_max_weight();
		tradeskill_class1_ = oldStruct->get_tradeskill_class1();
		tradeskill_class2_ = oldStruct->get_tradeskill_class2();
		tradeskill_class3_ = oldStruct->get_tradeskill_class3();
		account_age_base_ = oldStruct->get_account_age_base();

		memset(account_age_bonus_,0,19);
		absorb_ = oldStruct->get_absorb();
		xp_ = oldStruct->get_xp();
		xp_needed_ = oldStruct->get_xp_needed();
		xp_debt_ = oldStruct->get_xp_debt();
		xp_yellow_ = oldStruct->get_xp_yellow();
		xp_yellow_vitality_bar_ = oldStruct->get_xp_yellow_vitality_bar();
		xp_blue_vitality_bar_ = oldStruct->get_xp_blue_vitality_bar();
		xp_blue_ = oldStruct->get_xp_blue();
		ts_xp_ = oldStruct->get_ts_xp();
		ts_xp_needed_ = oldStruct->get_ts_xp_needed();
		tradeskill_exp_yellow_ = oldStruct->get_tradeskill_exp_yellow();
		tradeskill_exp_blue_ = oldStruct->get_tradeskill_exp_blue();
		flags_ = oldStruct->get_flags();
		flags2_ = oldStruct->get_flags2();
		xp_vitality_ = oldStruct->get_xp_vitality();
		tradeskill_xp_vitality_ = oldStruct->get_tradeskill_xp_vitality();
		mitigation_skill1_ = oldStruct->get_mitigation_skill1();
		mitigation_skill2_ = oldStruct->get_mitigation_skill2();
		mitigation_skill3_ = oldStruct->get_mitigation_skill3();
		mitigation_pve_ = oldStruct->get_mitigation_pve();
		mitigation_pvp_ = oldStruct->get_mitigation_pvp();
		ability_modifier_ = oldStruct->get_ability_modifier();
		critical_mitigation_ = oldStruct->get_critical_mitigation();
		block_chance_ = oldStruct->get_block_chance();
		uncontested_parry_ = oldStruct->get_uncontested_parry();
		uncontested_block_ = oldStruct->get_uncontested_block();
		uncontested_dodge_ = oldStruct->get_uncontested_dodge();
		uncontested_riposte_ = oldStruct->get_uncontested_riposte();
		crit_chance_ = oldStruct->get_crit_chance();
		crit_bonus_ = oldStruct->get_crit_bonus();
		potency_ = oldStruct->get_potency();
		hate_mod_ = oldStruct->get_hate_mod();
		reuse_speed_ = oldStruct->get_reuse_speed();
		casting_speed_ = oldStruct->get_casting_speed();
		recovery_speed_ = oldStruct->get_recovery_speed();
		spell_reuse_speed_ = oldStruct->get_spell_reuse_speed();
		spell_multi_attack_ = oldStruct->get_spell_multi_attack();
		dps_ = oldStruct->get_dps();
		
		size_mod_ = oldStruct->get_size_mod();
		ignore_size_mod_calc_ = oldStruct->get_ignore_size_mod_calc();
		
		dps_multiplier_ = oldStruct->get_dps_multiplier();
		attackspeed_ = oldStruct->get_attackspeed();
		haste_ = oldStruct->get_haste();
		multi_attack_ = oldStruct->get_multi_attack();
		flurry_ = oldStruct->get_flurry();
		melee_ae_ = oldStruct->get_melee_ae();
		strikethrough_ = oldStruct->get_strikethrough();
		accuracy_ = oldStruct->get_accuracy();
		offensivespeed_ = oldStruct->get_offensivespeed();
		rain_ = oldStruct->get_rain();
		wind_ = oldStruct->get_wind();
		alignment_ = oldStruct->get_alignment();
		pet_id_ = oldStruct->get_pet_id();
		pet_name_ = std::string(oldStruct->get_pet_name());
		pet_health_pct_ = oldStruct->get_pet_health_pct();
		pet_power_pct_ = oldStruct->get_pet_power_pct();
		pet_movement_ = oldStruct->get_pet_movement();
		pet_behavior_ = oldStruct->get_pet_behavior();
		vision_ = oldStruct->get_vision();
		breathe_underwater_ = oldStruct->get_breathe_underwater();
		biography_ = std::string(oldStruct->get_biography());
		drunk_ = oldStruct->get_drunk();
		power_regen_ = oldStruct->get_power_regen();
		hp_regen_ = oldStruct->get_hp_regen();

		power_regen_override_ = oldStruct->get_power_regen_override();
		hp_regen_override_ = oldStruct->get_hp_regen_override();

		water_type_ = oldStruct->get_water_type();
		flying_type_ = oldStruct->get_flying_type();

		no_interrupt_ = oldStruct->get_no_interrupt();

		interaction_flag_ = oldStruct->get_interaction_flag();
		tag1_ = oldStruct->get_tag1();
		mood_ = oldStruct->get_mood();
		
		range_last_attack_time_ = oldStruct->get_range_last_attack_time();
		primary_last_attack_time_ = oldStruct->get_primary_last_attack_time();;
		secondary_last_attack_time_ = oldStruct->get_secondary_last_attack_time();;
		primary_attack_delay_ = oldStruct->get_primary_attack_delay();
		secondary_attack_delay_ = oldStruct->get_secondary_attack_delay();
		ranged_attack_delay_ = oldStruct->get_ranged_attack_delay();
		primary_weapon_type_ = oldStruct->get_primary_weapon_type();
		secondary_weapon_type_ = oldStruct->get_secondary_weapon_type();
		ranged_weapon_type_ = oldStruct->get_ranged_weapon_type();
		primary_weapon_damage_low_ = oldStruct->get_primary_weapon_damage_low();
		primary_weapon_damage_high_ = oldStruct->get_primary_weapon_damage_high();
		secondary_weapon_damage_low_ = oldStruct->get_secondary_weapon_damage_low();
		secondary_weapon_damage_high_ = oldStruct->get_secondary_weapon_damage_high();
		ranged_weapon_damage_low_ = oldStruct->get_ranged_weapon_damage_low();
		ranged_weapon_damage_high_ = oldStruct->get_ranged_weapon_damage_high();
		wield_type_ = oldStruct->get_wield_type();
		attack_type_ = oldStruct->get_attack_type();
		primary_weapon_delay_ = oldStruct->get_primary_weapon_delay();
		secondary_weapon_delay_ = oldStruct->get_secondary_weapon_delay();
		ranged_weapon_delay_ = oldStruct->get_ranged_weapon_delay();
		
		override_primary_weapon_ = oldStruct->get_override_primary_weapon();
		override_secondary_weapon_ = oldStruct->get_override_secondary_weapon();
		override_ranged_weapon_ = oldStruct->get_override_ranged_weapon();
		friendly_target_npc_ = oldStruct->get_friendly_target_npc();
		last_claim_time_ = oldStruct->get_last_claim_time();
		
		engaged_encounter_ = oldStruct->get_engaged_encounter();
		lockable_encounter_ = oldStruct->get_lockable_encounter();
		
		first_world_login_ = oldStruct->get_first_world_login();
		reload_player_spells_ = oldStruct->get_reload_player_spells();
		
		action_state_ = oldStruct->get_action_state();
		combat_action_state_ = oldStruct->get_combat_action_state();
		
		group_loot_method_ = oldStruct->get_group_loot_method();
		group_loot_items_rarity_ = oldStruct->get_group_loot_items_rarity();
		group_auto_split_ = oldStruct->get_group_auto_split();
		group_default_yell_ = oldStruct->get_group_default_yell();
		group_autolock_ = oldStruct->get_group_autolock();
		group_lock_method_ = oldStruct->get_group_lock_method();
		group_solo_autolock_ = oldStruct->get_group_solo_autolock();
		group_auto_loot_method_ = oldStruct->get_group_auto_loot_method();
		assist_auto_attack_ = oldStruct->get_assist_auto_attack();
		
		max_spell_reduction_ = oldStruct->get_max_spell_reduction();
		max_spell_reduction_override_ = oldStruct->get_max_spell_reduction_override();
		max_chase_distance_ = oldStruct->get_max_chase_distance();
	}
	//mutable std::shared_mutex mutex_;
    std::string get_name() { std::lock_guard<std::mutex> lk(classMutex); return name_; }
	int8	 get_class1() { std::lock_guard<std::mutex> lk(classMutex); return class1_; }
	int8	 get_class2() { std::lock_guard<std::mutex> lk(classMutex); return class2_; }
	int8	 get_class3() { std::lock_guard<std::mutex> lk(classMutex); return class3_; }
	int8	 get_race() { std::lock_guard<std::mutex> lk(classMutex); return race_; }
	int8	 get_gender() { std::lock_guard<std::mutex> lk(classMutex); return gender_; }
	int16 	 get_level() { std::lock_guard<std::mutex> lk(classMutex); return level_; }
	int16	 get_max_level() { std::lock_guard<std::mutex> lk(classMutex); return max_level_; }
	int16	 get_effective_level() { std::lock_guard<std::mutex> lk(classMutex); return effective_level_; } 
	int16	 get_tradeskill_level() { std::lock_guard<std::mutex> lk(classMutex); return tradeskill_level_; }
	int16	 get_tradeskill_max_level() { std::lock_guard<std::mutex> lk(classMutex); return tradeskill_max_level_; }

	int8	 get_cur_concentration() { std::lock_guard<std::mutex> lk(classMutex); return cur_concentration_; }
	int8	 get_max_concentration() { std::lock_guard<std::mutex> lk(classMutex); return max_concentration_; }
	int8	 get_max_concentration_base() { std::lock_guard<std::mutex> lk(classMutex); return max_concentration_base_; }
	int16	 get_cur_attack() { std::lock_guard<std::mutex> lk(classMutex); return cur_attack_; }
	int16	 get_attack_base() { std::lock_guard<std::mutex> lk(classMutex); return attack_base_; }
	int16	 get_cur_mitigation() { std::lock_guard<std::mutex> lk(classMutex); return cur_mitigation_; }
	int16	 get_max_mitigation() { std::lock_guard<std::mutex> lk(classMutex); return max_mitigation_; }

	int16	 get_mitigation_base() { std::lock_guard<std::mutex> lk(classMutex); return mitigation_base_; }
	sint16	 get_mitigation_modifier() { std::lock_guard<std::mutex> lk(classMutex); return mitigation_modifier_; }
	int16	 get_avoidance_display() { std::lock_guard<std::mutex> lk(classMutex); return avoidance_display_; }
	float	 get_cur_avoidance() { std::lock_guard<std::mutex> lk(classMutex); return cur_avoidance_; }
	int16	 get_base_avoidance_pct() { std::lock_guard<std::mutex> lk(classMutex); return base_avoidance_pct_; }
	int16	 get_avoidance_base() { std::lock_guard<std::mutex> lk(classMutex); return avoidance_base_; }

	float	 get_parry() { std::lock_guard<std::mutex> lk(classMutex); return parry_; }
	float	 get_parry_base() { std::lock_guard<std::mutex> lk(classMutex); return parry_base_; }
	
	int16	 get_max_avoidance() { std::lock_guard<std::mutex> lk(classMutex); return max_avoidance_; }

	float	 get_deflection() { std::lock_guard<std::mutex> lk(classMutex); return deflection_; }
	int16	 get_deflection_base() { std::lock_guard<std::mutex> lk(classMutex); return deflection_base_; }

	float	 get_block() { std::lock_guard<std::mutex> lk(classMutex); return block_; }
	int16	 get_block_base() { std::lock_guard<std::mutex> lk(classMutex); return block_base_; }

	float	 get_str() { std::lock_guard<std::mutex> lk(classMutex); return str_; }
	float	 get_sta() { std::lock_guard<std::mutex> lk(classMutex); return sta_; }
	float	 get_agi() { std::lock_guard<std::mutex> lk(classMutex); return agi_; }
	float	 get_wis() { std::lock_guard<std::mutex> lk(classMutex); return wis_; }
	float	 get_intel() { std::lock_guard<std::mutex> lk(classMutex); return intel_; }
	float	 get_str_base() { std::lock_guard<std::mutex> lk(classMutex); return str_base_; }
	float	 get_sta_base() { std::lock_guard<std::mutex> lk(classMutex); return sta_base_; }
	float	 get_agi_base() { std::lock_guard<std::mutex> lk(classMutex); return agi_base_; }
	float	 get_wis_base() { std::lock_guard<std::mutex> lk(classMutex); return wis_base_; }
	float	 get_intel_base() { std::lock_guard<std::mutex> lk(classMutex); return intel_base_; }
	int16	 get_heat() { std::lock_guard<std::mutex> lk(classMutex); return heat_; }
	int16	 get_cold() { std::lock_guard<std::mutex> lk(classMutex); return cold_; }
	int16	 get_magic() { std::lock_guard<std::mutex> lk(classMutex); return magic_; }
	int16	 get_mental() { std::lock_guard<std::mutex> lk(classMutex); return mental_; }
	int16	 get_divine() { std::lock_guard<std::mutex> lk(classMutex); return divine_; }
	int16	 get_disease() { std::lock_guard<std::mutex> lk(classMutex); return disease_; }
	int16	 get_poison() { std::lock_guard<std::mutex> lk(classMutex); return poison_; }
	int16	 get_disease_base() { std::lock_guard<std::mutex> lk(classMutex); return disease_base_; }
	int16	 get_cold_base() { std::lock_guard<std::mutex> lk(classMutex); return cold_base_; }
	int16	 get_divine_base() { std::lock_guard<std::mutex> lk(classMutex); return divine_base_; }
	int16	 get_magic_base() { std::lock_guard<std::mutex> lk(classMutex); return magic_base_; }
	int16	 get_mental_base() { std::lock_guard<std::mutex> lk(classMutex); return mental_base_; }
	int16	 get_heat_base() { std::lock_guard<std::mutex> lk(classMutex); return heat_base_; }
	int16	 get_poison_base() { std::lock_guard<std::mutex> lk(classMutex); return poison_base_; }
	int16	 get_elemental_base() { std::lock_guard<std::mutex> lk(classMutex); return elemental_base_; }
	int16	 get_noxious_base() { std::lock_guard<std::mutex> lk(classMutex); return noxious_base_; }
	int16	 get_arcane_base() { std::lock_guard<std::mutex> lk(classMutex); return arcane_base_; }
	int32	 get_coin_copper() { std::lock_guard<std::mutex> lk(classMutex); return coin_copper_; }
	int32	 get_coin_silver() { std::lock_guard<std::mutex> lk(classMutex); return coin_silver_; }
	int32	 get_coin_gold() { std::lock_guard<std::mutex> lk(classMutex); return coin_gold_; }
	int32	 get_coin_plat() { std::lock_guard<std::mutex> lk(classMutex); return coin_plat_; }
	int32	 get_bank_coin_copper() { std::lock_guard<std::mutex> lk(classMutex); return bank_coin_copper_; }
	int32	 get_bank_coin_silver() { std::lock_guard<std::mutex> lk(classMutex); return bank_coin_silver_; }
	int32	 get_bank_coin_gold() { std::lock_guard<std::mutex> lk(classMutex); return bank_coin_gold_; }
	int32	 get_bank_coin_plat() { std::lock_guard<std::mutex> lk(classMutex); return bank_coin_plat_; }
	int32	 get_status_points() { std::lock_guard<std::mutex> lk(classMutex); return status_points_; }
	std::string get_deity() { std::lock_guard<std::mutex> lk(classMutex); return deity_; }
	int32	 get_weight() { std::lock_guard<std::mutex> lk(classMutex); return weight_; }
	int32	 get_max_weight() { std::lock_guard<std::mutex> lk(classMutex); return max_weight_; }

		
	//SpellEffects* & get_spell_effects() { std::lock_guard<std::mutex> lk(classMutex); return spell_effects_; }
	//MaintainedEffects* & get_maintained_effects() { std::lock_guard<std::mutex> lk(classMutex); return maintained_effects_; }
	int8	 get_tradeskill_class1() { std::lock_guard<std::mutex> lk(classMutex); return tradeskill_class1_; }
	int8	 get_tradeskill_class2() { std::lock_guard<std::mutex> lk(classMutex); return tradeskill_class2_; }
	int8	 get_tradeskill_class3() { std::lock_guard<std::mutex> lk(classMutex); return tradeskill_class3_; }

	int32	 get_account_age_base() { std::lock_guard<std::mutex> lk(classMutex); return account_age_base_; }

	int8	 get_account_age_bonus(int8 field) { std::lock_guard<std::mutex> lk(classMutex); return account_age_bonus_[field]; }
	int16	 get_absorb() { std::lock_guard<std::mutex> lk(classMutex); return absorb_; }
	int32	 get_xp() { std::lock_guard<std::mutex> lk(classMutex); return xp_; }
	int32	 get_xp_needed() { std::lock_guard<std::mutex> lk(classMutex); return xp_needed_; }
	float	 get_xp_debt() { std::lock_guard<std::mutex> lk(classMutex); return xp_debt_; }
	int16	 get_xp_yellow() { std::lock_guard<std::mutex> lk(classMutex); return xp_yellow_; }
	int16	 get_xp_yellow_vitality_bar() { std::lock_guard<std::mutex> lk(classMutex); return xp_yellow_vitality_bar_; }
	int16	 get_xp_blue_vitality_bar() { std::lock_guard<std::mutex> lk(classMutex); return xp_blue_vitality_bar_; }
	int16	 get_xp_blue() { std::lock_guard<std::mutex> lk(classMutex); return xp_blue_; }
	int32	 get_ts_xp() { std::lock_guard<std::mutex> lk(classMutex); return ts_xp_; }
	int32	 get_ts_xp_needed() { std::lock_guard<std::mutex> lk(classMutex); return ts_xp_needed_; }
	int16	 get_tradeskill_exp_yellow() { std::lock_guard<std::mutex> lk(classMutex); return tradeskill_exp_yellow_; }
	int16	 get_tradeskill_exp_blue() { std::lock_guard<std::mutex> lk(classMutex); return tradeskill_exp_blue_; }
	int32	 get_flags() { std::lock_guard<std::mutex> lk(classMutex); return flags_; }
	int32	 get_flags2() { std::lock_guard<std::mutex> lk(classMutex); return flags2_; }
	float	 get_xp_vitality() { std::lock_guard<std::mutex> lk(classMutex); return xp_vitality_; }
	float	 get_tradeskill_xp_vitality() { std::lock_guard<std::mutex> lk(classMutex); return tradeskill_xp_vitality_; }

	int16	 get_mitigation_skill1() { std::lock_guard<std::mutex> lk(classMutex); return mitigation_skill1_; }
	int16	 get_mitigation_skill2() { std::lock_guard<std::mutex> lk(classMutex); return mitigation_skill2_; }
	int16	 get_mitigation_skill3() { std::lock_guard<std::mutex> lk(classMutex); return mitigation_skill3_; }
	
	int16	 get_mitigation_pve() { std::lock_guard<std::mutex> lk(classMutex); return mitigation_pve_; }
	int16	 get_mitigation_pvp() { std::lock_guard<std::mutex> lk(classMutex); return mitigation_pvp_; }

	float	 get_ability_modifier() { std::lock_guard<std::mutex> lk(classMutex); return ability_modifier_; }
	float	 get_critical_mitigation() { std::lock_guard<std::mutex> lk(classMutex); return critical_mitigation_; }
	float	 get_block_chance() { std::lock_guard<std::mutex> lk(classMutex); return block_chance_; }
	float	 get_uncontested_parry() { std::lock_guard<std::mutex> lk(classMutex); return uncontested_parry_; }
	float	 get_uncontested_block() { std::lock_guard<std::mutex> lk(classMutex); return uncontested_block_; }
	float	 get_uncontested_dodge() { std::lock_guard<std::mutex> lk(classMutex); return uncontested_dodge_; }
	float	 get_uncontested_riposte() { std::lock_guard<std::mutex> lk(classMutex); return uncontested_riposte_; }
	float	 get_crit_chance() { std::lock_guard<std::mutex> lk(classMutex); return crit_chance_; }
	float	 get_crit_bonus() { std::lock_guard<std::mutex> lk(classMutex); return crit_bonus_; }
	float	 get_potency() { std::lock_guard<std::mutex> lk(classMutex); return potency_; }
	float	 get_hate_mod() { std::lock_guard<std::mutex> lk(classMutex); return hate_mod_; }
	float	 get_reuse_speed() { std::lock_guard<std::mutex> lk(classMutex); return reuse_speed_; }
	float	 get_casting_speed() { std::lock_guard<std::mutex> lk(classMutex); return casting_speed_; }
	float	 get_recovery_speed() { std::lock_guard<std::mutex> lk(classMutex); return recovery_speed_; }
	float	 get_spell_reuse_speed() { std::lock_guard<std::mutex> lk(classMutex); return spell_reuse_speed_; }
	float	 get_spell_multi_attack() { std::lock_guard<std::mutex> lk(classMutex); return spell_multi_attack_; }
	float	 get_size_mod() { std::lock_guard<std::mutex> lk(classMutex); return size_mod_; }
	int8	 get_ignore_size_mod_calc() { std::lock_guard<std::mutex> lk(classMutex); return ignore_size_mod_calc_; }
	float	 get_dps() { std::lock_guard<std::mutex> lk(classMutex); return dps_; }
	float	 get_dps_multiplier() { std::lock_guard<std::mutex> lk(classMutex); return dps_multiplier_; }
	float	 get_attackspeed() { std::lock_guard<std::mutex> lk(classMutex); return attackspeed_; }
	float	 get_haste() { std::lock_guard<std::mutex> lk(classMutex); return haste_; }
	float	 get_multi_attack() { std::lock_guard<std::mutex> lk(classMutex); return multi_attack_; }
	float	 get_flurry() { std::lock_guard<std::mutex> lk(classMutex); return flurry_; }
	float	 get_melee_ae() { std::lock_guard<std::mutex> lk(classMutex); return melee_ae_; }
	float	 get_strikethrough() { std::lock_guard<std::mutex> lk(classMutex); return strikethrough_; }
	float	 get_accuracy() { std::lock_guard<std::mutex> lk(classMutex); return accuracy_; }
	float	 get_offensivespeed() { std::lock_guard<std::mutex> lk(classMutex); return offensivespeed_; }
	float	 get_rain() { std::lock_guard<std::mutex> lk(classMutex); return rain_; }
	float	 get_wind() { std::lock_guard<std::mutex> lk(classMutex); return wind_; }
	sint8	 get_alignment() { std::lock_guard<std::mutex> lk(classMutex); return alignment_; }
	int32	 get_pet_id() { std::lock_guard<std::mutex> lk(classMutex); return pet_id_; }

	std::string get_pet_name() { std::lock_guard<std::mutex> lk(classMutex); return pet_name_; }
	float	get_pet_health_pct() { std::lock_guard<std::mutex> lk(classMutex); return pet_health_pct_; }
	float	get_pet_power_pct() { std::lock_guard<std::mutex> lk(classMutex); return pet_power_pct_; }
	int8	get_pet_movement() { std::lock_guard<std::mutex> lk(classMutex); return pet_movement_; }
	int8	get_pet_behavior() { std::lock_guard<std::mutex> lk(classMutex); return pet_behavior_; }
	int32	get_vision() { std::lock_guard<std::mutex> lk(classMutex); return vision_; }
	int8	get_breathe_underwater() { std::lock_guard<std::mutex> lk(classMutex); return breathe_underwater_; }
	std::string get_biography() { std::lock_guard<std::mutex> lk(classMutex); return biography_; }
	float	get_drunk() { std::lock_guard<std::mutex> lk(classMutex); return drunk_; }

	sint16	get_power_regen() { std::lock_guard<std::mutex> lk(classMutex); return power_regen_; }
	sint16	get_hp_regen() { std::lock_guard<std::mutex> lk(classMutex); return hp_regen_; }

	int8	get_power_regen_override() { std::lock_guard<std::mutex> lk(classMutex); return power_regen_override_; }
	int8	get_hp_regen_override() { std::lock_guard<std::mutex> lk(classMutex); return hp_regen_override_; }

	int8	get_water_type() { std::lock_guard<std::mutex> lk(classMutex); return water_type_; }
	int8	get_flying_type() { std::lock_guard<std::mutex> lk(classMutex); return flying_type_; }

	int8	get_no_interrupt() { std::lock_guard<std::mutex> lk(classMutex); return no_interrupt_; }

	int8	get_interaction_flag() { std::lock_guard<std::mutex> lk(classMutex); return interaction_flag_; }
	int8	get_tag1() { std::lock_guard<std::mutex> lk(classMutex); return tag1_; }
	int16	get_mood() { std::lock_guard<std::mutex> lk(classMutex); return mood_; }

	int32	get_range_last_attack_time() { std::lock_guard<std::mutex> lk(classMutex); return range_last_attack_time_; }
	int32	get_primary_last_attack_time() { std::lock_guard<std::mutex> lk(classMutex); return primary_last_attack_time_; }
	int32	get_secondary_last_attack_time() { std::lock_guard<std::mutex> lk(classMutex); return secondary_last_attack_time_; }
	
	int16	get_primary_attack_delay() { std::lock_guard<std::mutex> lk(classMutex); return primary_attack_delay_; }
	int16	get_secondary_attack_delay() { std::lock_guard<std::mutex> lk(classMutex); return secondary_attack_delay_; }
	int16	get_ranged_attack_delay() { std::lock_guard<std::mutex> lk(classMutex); return ranged_attack_delay_; }
	
	int8	get_primary_weapon_type() { std::lock_guard<std::mutex> lk(classMutex); return primary_weapon_type_; }
	int8	get_secondary_weapon_type() { std::lock_guard<std::mutex> lk(classMutex); return secondary_weapon_type_; }
	int8	get_ranged_weapon_type() { std::lock_guard<std::mutex> lk(classMutex); return ranged_weapon_type_; }

	int32	get_primary_weapon_damage_low() { std::lock_guard<std::mutex> lk(classMutex); return primary_weapon_damage_low_; }
	int32	get_primary_weapon_damage_high() { std::lock_guard<std::mutex> lk(classMutex); return primary_weapon_damage_high_; }
	int32	get_secondary_weapon_damage_low() { std::lock_guard<std::mutex> lk(classMutex); return secondary_weapon_damage_low_; }
	int32	get_secondary_weapon_damage_high() { std::lock_guard<std::mutex> lk(classMutex); return secondary_weapon_damage_high_; }
	int32	get_ranged_weapon_damage_low() { std::lock_guard<std::mutex> lk(classMutex); return ranged_weapon_damage_low_; }
	int32	get_ranged_weapon_damage_high() { std::lock_guard<std::mutex> lk(classMutex); return ranged_weapon_damage_high_; }

	int8	get_wield_type() { std::lock_guard<std::mutex> lk(classMutex); return wield_type_; }
	int8	get_attack_type() { std::lock_guard<std::mutex> lk(classMutex); return attack_type_; }

	int16	get_primary_weapon_delay() { std::lock_guard<std::mutex> lk(classMutex); return primary_weapon_delay_; }
	int16	get_secondary_weapon_delay() { std::lock_guard<std::mutex> lk(classMutex); return secondary_weapon_delay_; }
	int16	get_ranged_weapon_delay() { std::lock_guard<std::mutex> lk(classMutex); return ranged_weapon_delay_; }
	
	int8	get_override_primary_weapon() { std::lock_guard<std::mutex> lk(classMutex); return override_primary_weapon_; }
	int8	get_override_secondary_weapon() { std::lock_guard<std::mutex> lk(classMutex); return override_secondary_weapon_; }
	int8	get_override_ranged_weapon() { std::lock_guard<std::mutex> lk(classMutex); return override_ranged_weapon_; }
	
	int8	get_friendly_target_npc() { std::lock_guard<std::mutex> lk(classMutex); return friendly_target_npc_; }
	int32	get_last_claim_time() { std::lock_guard<std::mutex> lk(classMutex); return last_claim_time_; }
	
	int8	get_engaged_encounter() { std::lock_guard<std::mutex> lk(classMutex); return engaged_encounter_; }
	int8	get_lockable_encounter() { std::lock_guard<std::mutex> lk(classMutex); return lockable_encounter_; }
	
	int8	get_first_world_login() { std::lock_guard<std::mutex> lk(classMutex); return first_world_login_; }
	
	int8	get_reload_player_spells() { std::lock_guard<std::mutex> lk(classMutex); return reload_player_spells_; }
	
	int8	get_group_loot_method() { std::lock_guard<std::mutex> lk(classMutex); return group_loot_method_; }
	int8	get_group_loot_items_rarity() { std::lock_guard<std::mutex> lk(classMutex); return group_loot_items_rarity_; }
	int8	get_group_auto_split() { std::lock_guard<std::mutex> lk(classMutex); return group_auto_split_; }
	int8	get_group_default_yell() { std::lock_guard<std::mutex> lk(classMutex); return group_default_yell_; }
	int8	get_group_autolock() { std::lock_guard<std::mutex> lk(classMutex); return group_autolock_; }
	int8	get_group_lock_method() { std::lock_guard<std::mutex> lk(classMutex); return group_lock_method_; }
	int8	get_group_solo_autolock() { std::lock_guard<std::mutex> lk(classMutex); return group_solo_autolock_; }
	int8	get_group_auto_loot_method() { std::lock_guard<std::mutex> lk(classMutex); return group_auto_loot_method_; }
	int8	get_assist_auto_attack() { std::lock_guard<std::mutex> lk(classMutex); return assist_auto_attack_; }
	
	std::string get_action_state() { std::lock_guard<std::mutex> lk(classMutex); return action_state_; }
	
	std::string get_combat_action_state() { std::lock_guard<std::mutex> lk(classMutex); return combat_action_state_; }
	
	float	get_max_spell_reduction() { std::lock_guard<std::mutex> lk(classMutex); return max_spell_reduction_; }
	int8	get_max_spell_reduction_override() { std::lock_guard<std::mutex> lk(classMutex); return max_spell_reduction_override_; }
	
	float	get_max_chase_distance() { std::lock_guard<std::mutex> lk(classMutex); return max_chase_distance_; }
	
	void	set_name(std::string value) { std::lock_guard<std::mutex> lk(classMutex); name_ = value; }
	
	void	set_deity(std::string value) { std::lock_guard<std::mutex> lk(classMutex); deity_ = value; }

	void	set_class1(int8 value) { std::lock_guard<std::mutex> lk(classMutex); class1_ = value; }
	void	set_class2(int8 value) { std::lock_guard<std::mutex> lk(classMutex); class2_ = value; }
	void	set_class3(int8 value) { std::lock_guard<std::mutex> lk(classMutex); class3_ = value; }

	void	set_race(int8 value) { std::lock_guard<std::mutex> lk(classMutex); race_ = value; }
	void	set_gender(int8 value) { std::lock_guard<std::mutex> lk(classMutex); gender_ = value; }
	void	set_level(int16 value) { std::lock_guard<std::mutex> lk(classMutex); level_ = value; }
	void	set_max_level(int16 value) { std::lock_guard<std::mutex> lk(classMutex); max_level_ = value; }
	void	set_effective_level(int16 value) { std::lock_guard<std::mutex> lk(classMutex); effective_level_ = value; }

	void	set_cur_concentration(int8 value) { std::lock_guard<std::mutex> lk(classMutex); cur_concentration_ = value; }
	void	set_max_concentration(int8 value) { std::lock_guard<std::mutex> lk(classMutex); max_concentration_ = value; }
	void	set_max_concentration_base(int8 value) { std::lock_guard<std::mutex> lk(classMutex); max_concentration_base_ = value; }

	void	add_cur_concentration(int8 value) { std::lock_guard<std::mutex> lk(classMutex); cur_concentration_ += value; }
	void	add_max_concentration(int8 value) { std::lock_guard<std::mutex> lk(classMutex); max_concentration_ += value; }

	void	set_cur_attack(int16 value) { std::lock_guard<std::mutex> lk(classMutex); cur_attack_ = value; }
	void	set_attack_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); attack_base_ = value; }
	void	set_cur_mitigation(int16 value) { std::lock_guard<std::mutex> lk(classMutex); cur_mitigation_ = value; }
	void	set_max_mitigation(int16 value) { std::lock_guard<std::mutex> lk(classMutex); max_mitigation_ = value; }
	void	set_mitigation_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); mitigation_base_ = value; }
	void	add_mitigation_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); mitigation_base_ += value; }
	void	set_mitigation_modifier(sint16 value) { std::lock_guard<std::mutex> lk(classMutex); mitigation_modifier_ = value; }

	void	set_avoidance_display(int16 value) { std::lock_guard<std::mutex> lk(classMutex); avoidance_display_ = value; }
	void	set_cur_avoidance(float value) { std::lock_guard<std::mutex> lk(classMutex); cur_avoidance_ = value; }
	void	set_base_avoidance_pct(int16 value) { std::lock_guard<std::mutex> lk(classMutex); base_avoidance_pct_ = value; }
	void	set_avoidance_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); avoidance_base_ = value; }
	void	set_max_avoidance(int16 value) { std::lock_guard<std::mutex> lk(classMutex); max_avoidance_ = value; }
	void	set_parry(float value) { std::lock_guard<std::mutex> lk(classMutex); parry_ = value; }
	void	set_parry_base(float value) { std::lock_guard<std::mutex> lk(classMutex); parry_base_ = value; }
	void	set_deflection(int16 value) { std::lock_guard<std::mutex> lk(classMutex); deflection_ = value; }
	void	set_deflection_base(float value) { std::lock_guard<std::mutex> lk(classMutex); deflection_base_ = value; }
	void	set_block(float value) { std::lock_guard<std::mutex> lk(classMutex); block_ = value; }
	void	set_block_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); block_base_ = value; }

	void	set_str(float value) { std::lock_guard<std::mutex> lk(classMutex); str_ = value; }
	void	set_sta(float value) { std::lock_guard<std::mutex> lk(classMutex); sta_ = value; }
	void	set_agi(float value) { std::lock_guard<std::mutex> lk(classMutex); agi_ = value; }
	void	set_wis(float value) { std::lock_guard<std::mutex> lk(classMutex); wis_ = value; }
	void	set_intel(float value) { std::lock_guard<std::mutex> lk(classMutex); intel_ = value; }

	void	add_str(float value) { std::lock_guard<std::mutex> lk(classMutex); if(str_ + value < 0.0f) str_ = 0.0f; else str_ += value; }
	void	add_sta(float value) { std::lock_guard<std::mutex> lk(classMutex); if(sta_ + value < 0.0f) sta_ = 0.0f; else sta_ += value; }
	void	add_agi(float value) { std::lock_guard<std::mutex> lk(classMutex); if(agi_ + value < 0.0f) agi_ = 0.0f; else agi_ += value; }
	void	add_wis(float value) { std::lock_guard<std::mutex> lk(classMutex); if(wis_ + value < 0.0f) wis_ = 0.0f; else wis_ += value; }
	void	add_intel(float value) { std::lock_guard<std::mutex> lk(classMutex); if(intel_ + value < 0.0f) intel_ = 0.0f; else intel_ += value; }

	void	set_str_base(float value) { std::lock_guard<std::mutex> lk(classMutex); str_base_ = value; }
	void	set_sta_base(float value) { std::lock_guard<std::mutex> lk(classMutex); sta_base_ = value; }
	void	set_agi_base(float value) { std::lock_guard<std::mutex> lk(classMutex); agi_base_ = value; }
	void	set_wis_base(float value) { std::lock_guard<std::mutex> lk(classMutex); wis_base_ = value; }
	void	set_intel_base(float value) { std::lock_guard<std::mutex> lk(classMutex); intel_base_ = value; }

	void	set_heat(int16 value) { std::lock_guard<std::mutex> lk(classMutex); heat_ = value; }
	void	set_cold(int16 value) { std::lock_guard<std::mutex> lk(classMutex); cold_ = value; }
	void	set_magic(int16 value) { std::lock_guard<std::mutex> lk(classMutex); magic_ = value; }
	void	set_mental(int16 value) { std::lock_guard<std::mutex> lk(classMutex); mental_ = value; }
	void	set_divine(int16 value) { std::lock_guard<std::mutex> lk(classMutex); divine_ = value; }
	void	set_disease(int16 value) { std::lock_guard<std::mutex> lk(classMutex); disease_ = value; }
	void	set_poison(int16 value) { std::lock_guard<std::mutex> lk(classMutex); poison_ = value; }

	void	add_heat(sint16 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint32)heat_ + value < 0) heat_ = 0; else heat_ += value; }
	void	add_cold(sint16 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint32)cold_ + value < 0) cold_ = 0; else cold_ += value; }
	void	add_magic(sint16 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint32)magic_ + value < 0) magic_ = 0; else magic_ += value; }
	void	add_mental(sint16 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint32)mental_ + value < 0) mental_ = 0; else mental_ += value; }
	void	add_divine(sint16 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint32)divine_ + value < 0) divine_ = 0; else divine_ += value; }
	void	add_disease(sint16 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint32)disease_ + value < 0) disease_ = 0; else disease_ += value; }
	void	add_poison(sint16 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint32)poison_ + value < 0) poison_ = 0; else poison_ += value; }

	void	set_disease_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); disease_base_ = value; }
	void	set_cold_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); cold_base_ = value; }
	void	set_divine_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); divine_base_ = value; }
	void	set_magic_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); magic_base_ = value; }
	void	set_mental_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); mental_base_ = value; }
	void	set_heat_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); heat_base_ = value; }
	void	set_poison_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); poison_base_ = value; }
	void	set_elemental_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); elemental_base_ = value; }
	void	set_noxious_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); noxious_base_ = value; }
	void	set_arcane_base(int16 value) { std::lock_guard<std::mutex> lk(classMutex); arcane_base_ = value; }

	void	set_tradeskill_level(int16 value) { std::lock_guard<std::mutex> lk(classMutex); tradeskill_level_ = value; }
	void	set_tradeskill_max_level(int16 value) { std::lock_guard<std::mutex> lk(classMutex); tradeskill_max_level_ = value; }

	void	set_tradeskill_class1(int8 value) { std::lock_guard<std::mutex> lk(classMutex); tradeskill_class1_ = value; }
	void	set_tradeskill_class2(int8 value) { std::lock_guard<std::mutex> lk(classMutex); tradeskill_class2_ = value; }
	void	set_tradeskill_class3(int8 value) { std::lock_guard<std::mutex> lk(classMutex); tradeskill_class3_ = value; }

	void	set_account_age_base(int32 value) { std::lock_guard<std::mutex> lk(classMutex); account_age_base_ = value; }

	void	set_xp_vitality(float value) { std::lock_guard<std::mutex> lk(classMutex); xp_vitality_ = value; }

	void	add_xp_vitality(float value) { std::lock_guard<std::mutex> lk(classMutex); xp_vitality_ += value; }

	void	set_tradeskill_xp_vitality(float value) { std::lock_guard<std::mutex> lk(classMutex); tradeskill_xp_vitality_ = value; }

	void	set_absorb(int16 value) { std::lock_guard<std::mutex> lk(classMutex); absorb_ = value; }

	void	set_xp(int32 value) { std::lock_guard<std::mutex> lk(classMutex); xp_ = value; }
	void	set_xp_needed(int32 value) { std::lock_guard<std::mutex> lk(classMutex); xp_needed_ = value; }

	void	set_xp_debt(float value) { std::lock_guard<std::mutex> lk(classMutex); if(std::isnan(value)) value = 0.0f; xp_debt_ = value; }

	void	set_xp_yellow(int16 value) { std::lock_guard<std::mutex> lk(classMutex); xp_yellow_ = value; }
	void	set_xp_blue(int16 value) { std::lock_guard<std::mutex> lk(classMutex); xp_blue_ = value; }

	void	set_xp_yellow_vitality_bar(int16 value) { std::lock_guard<std::mutex> lk(classMutex); xp_yellow_vitality_bar_ = value; }
	void	set_xp_blue_vitality_bar(int16 value) { std::lock_guard<std::mutex> lk(classMutex); xp_blue_vitality_bar_ = value; }

	void	set_ts_xp(int32 value) { std::lock_guard<std::mutex> lk(classMutex); ts_xp_ = value; }
	void	set_ts_xp_needed(int32 value) { std::lock_guard<std::mutex> lk(classMutex); ts_xp_needed_ = value; }

	void	set_tradeskill_exp_yellow(int16 value) { std::lock_guard<std::mutex> lk(classMutex); tradeskill_exp_yellow_ = value; }
	void	set_tradeskill_exp_blue(int16 value) { std::lock_guard<std::mutex> lk(classMutex); tradeskill_exp_blue_ = value; }

	void	set_flags(int32 value) { std::lock_guard<std::mutex> lk(classMutex); flags_ = value; }
	void	set_flags2(int32 value) { std::lock_guard<std::mutex> lk(classMutex); flags2_ = value; }

	void	set_coin_plat(int32 value) { std::lock_guard<std::mutex> lk(classMutex); coin_plat_ = value; }
	void	set_coin_gold(int32 value) { std::lock_guard<std::mutex> lk(classMutex); coin_gold_ = value; }
	void	set_coin_silver(int32 value) { std::lock_guard<std::mutex> lk(classMutex); coin_silver_ = value; }
	void	set_coin_copper(int32 value) { std::lock_guard<std::mutex> lk(classMutex); coin_copper_ = value; }

	void	add_coin_plat(int32 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint64)coin_plat_ + value < 0) coin_plat_ = 0; else coin_plat_ += value; }
	void	add_coin_gold(int32 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint64)coin_gold_ + value < 0) coin_gold_ = 0; else coin_gold_ += value; }
	void	add_coin_silver(int32 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint64)coin_silver_ + value < 0) coin_silver_ = 0; else coin_silver_ += value; }
	void	add_coin_copper(int32 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint64)coin_copper_ + value < 0) coin_copper_ = 0; else coin_copper_ += value; }

	void	set_bank_coin_plat(int32 value) { std::lock_guard<std::mutex> lk(classMutex); bank_coin_plat_ = value; }
	void	set_bank_coin_gold(int32 value) { std::lock_guard<std::mutex> lk(classMutex); bank_coin_gold_ = value; }
	void	set_bank_coin_silver(int32 value) { std::lock_guard<std::mutex> lk(classMutex); bank_coin_silver_ = value; }
	void	set_bank_coin_copper(int32 value) { std::lock_guard<std::mutex> lk(classMutex); bank_coin_copper_ = value; }

	void	add_bank_coin_plat(int32 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint64)bank_coin_plat_ + value < 0) bank_coin_plat_ = 0; else bank_coin_plat_ += value; }
	void	add_bank_coin_gold(int32 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint64)bank_coin_gold_ + value < 0) bank_coin_gold_ = 0; else bank_coin_gold_ += value; }
	void	add_bank_coin_silver(int32 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint64)bank_coin_silver_ + value < 0) bank_coin_silver_ = 0; else bank_coin_silver_ += value; }
	void	add_bank_coin_copper(int32 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint64)bank_coin_copper_ + value < 0) bank_coin_copper_ = 0; else bank_coin_copper_ += value; }

	void	set_status_points(int32 value) { std::lock_guard<std::mutex> lk(classMutex); status_points_ = value; }
	void	add_status_points(int32 value) { std::lock_guard<std::mutex> lk(classMutex);  if((sint64)status_points_ + value < 0) status_points_ = 0; else status_points_ += value; }
	bool	subtract_status_points(int32 value) { std::lock_guard<std::mutex> lk(classMutex); if(value > status_points_) return false; status_points_ -= value; return true; }
	
	void	set_mitigation_skill1(int16 value) { std::lock_guard<std::mutex> lk(classMutex); mitigation_skill1_ = value; }
	void	set_mitigation_skill2(int16 value) { std::lock_guard<std::mutex> lk(classMutex); mitigation_skill2_ = value; }
	void	set_mitigation_skill3(int16 value) { std::lock_guard<std::mutex> lk(classMutex); mitigation_skill3_ = value; }
	
	void	set_mitigation_pve(int16 value) { std::lock_guard<std::mutex> lk(classMutex); mitigation_pve_ = value; }
	void	set_mitigation_pvp(int16 value) { std::lock_guard<std::mutex> lk(classMutex); mitigation_pvp_ = value; }

	void	add_mitigation_skill1(int16 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint32)mitigation_skill1_ + value < 0) mitigation_skill1_ = 0; else mitigation_skill1_ += value; }
	void	add_mitigation_skill2(int16 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint32)mitigation_skill2_ + value < 0) mitigation_skill2_ = 0; else mitigation_skill2_ += value; }
	void	add_mitigation_skill3(int16 value) { std::lock_guard<std::mutex> lk(classMutex); if((sint32)mitigation_skill3_ + value < 0) mitigation_skill3_ = 0; else mitigation_skill3_ += value; }

	void	set_ability_modifier(float value) { std::lock_guard<std::mutex> lk(classMutex); ability_modifier_ = value; }

	void	add_ability_modifier(float value) { std::lock_guard<std::mutex> lk(classMutex); if(ability_modifier_ + value < 0.0f) ability_modifier_ = 0.0f; else ability_modifier_ += value; }

	void	set_critical_mitigation(float value) { std::lock_guard<std::mutex> lk(classMutex); critical_mitigation_ = value; }

	void	add_critical_mitigation(float value) { std::lock_guard<std::mutex> lk(classMutex); if(critical_mitigation_ + value < 0.0f) critical_mitigation_ = 0.0f; else critical_mitigation_ += value; }

	void	set_block_chance(float value) { std::lock_guard<std::mutex> lk(classMutex); block_chance_ = value; }
	void	set_uncontested_parry(float value) { std::lock_guard<std::mutex> lk(classMutex); uncontested_parry_ = value; }
	void	set_uncontested_block(float value) { std::lock_guard<std::mutex> lk(classMutex); uncontested_block_ = value; }
	void	set_uncontested_dodge(float value) { std::lock_guard<std::mutex> lk(classMutex); uncontested_dodge_ = value; }
	void	set_uncontested_riposte(float value) { std::lock_guard<std::mutex> lk(classMutex); uncontested_riposte_ = value; }
	void	set_crit_chance(float value) { std::lock_guard<std::mutex> lk(classMutex); crit_chance_ = value; }
	void	set_crit_bonus(float value) { std::lock_guard<std::mutex> lk(classMutex); crit_bonus_ = value; }
	void	set_potency(float value) { std::lock_guard<std::mutex> lk(classMutex); potency_ = value; }
	void	set_hate_mod(float value) { std::lock_guard<std::mutex> lk(classMutex); hate_mod_ = value; }
	void	set_reuse_speed(float value) { std::lock_guard<std::mutex> lk(classMutex); reuse_speed_ = value; }
	void	set_casting_speed(float value) { std::lock_guard<std::mutex> lk(classMutex); casting_speed_ = value; }
	void	set_recovery_speed(float value) { std::lock_guard<std::mutex> lk(classMutex); recovery_speed_ = value; }
	void	set_spell_reuse_speed(float value) { std::lock_guard<std::mutex> lk(classMutex); spell_reuse_speed_ = value; }
	void	set_spell_multi_attack(float value) { std::lock_guard<std::mutex> lk(classMutex); spell_multi_attack_ = value; }
	void	set_size_mod(float value) { std::lock_guard<std::mutex> lk(classMutex); size_mod_ = value; }
	void	set_ignore_size_mod_calc(int8 value) { std::lock_guard<std::mutex> lk(classMutex); ignore_size_mod_calc_ = value; }
	void	set_dps(float value) { std::lock_guard<std::mutex> lk(classMutex); dps_ = value; }
	void	set_dps_multiplier(float value) { std::lock_guard<std::mutex> lk(classMutex); dps_multiplier_ = value; }
	void	set_attackspeed(float value) { std::lock_guard<std::mutex> lk(classMutex); attackspeed_ = value; }
	void	set_haste(float value) { std::lock_guard<std::mutex> lk(classMutex); haste_ = value; }
	void	set_multi_attack(float value) { std::lock_guard<std::mutex> lk(classMutex); multi_attack_ = value; }
	void	set_flurry(float value) { std::lock_guard<std::mutex> lk(classMutex); flurry_ = value; }
	void	set_melee_ae(float value) { std::lock_guard<std::mutex> lk(classMutex); melee_ae_ = value; }
	void	set_strikethrough(float value) { std::lock_guard<std::mutex> lk(classMutex); strikethrough_ = value; }
	void	set_accuracy(float value) { std::lock_guard<std::mutex> lk(classMutex); accuracy_ = value; }
	void	set_offensivespeed(float value) { std::lock_guard<std::mutex> lk(classMutex); offensivespeed_ = value; }
	
	// crash client if float values above 1.0 are sent
	void	set_rain(float value) { std::lock_guard<std::mutex> lk(classMutex); if(value > 1.0f) value = 1.0f; else if(value < 0.0f) value = 0.0f; rain_ = value; }
	void	set_wind(float value) { std::lock_guard<std::mutex> lk(classMutex); if(value > 1.0f) value = 1.0f; else if(value < 0.0f) value = 0.0f; wind_ = value; }

	void	set_max_chase_distance(float value) { std::lock_guard<std::mutex> lk(classMutex); max_chase_distance_ = value; }
	
	void	add_block_chance(float value) { std::lock_guard<std::mutex> lk(classMutex); if(block_chance_ + value < 0.0f) block_chance_ = 0.0f; else block_chance_ += value; }
	void	add_uncontested_parry(float value) { std::lock_guard<std::mutex> lk(classMutex); if(uncontested_parry_ + value < 0.0f) uncontested_parry_ = 0.0f; else uncontested_parry_ += value; }
	void	add_uncontested_block(float value) { std::lock_guard<std::mutex> lk(classMutex); if(uncontested_block_ + value < 0.0f) uncontested_block_ = 0.0f; else uncontested_block_ += value; }
	void	add_uncontested_dodge(float value) { std::lock_guard<std::mutex> lk(classMutex); if(uncontested_dodge_ + value < 0.0f) uncontested_dodge_ = 0.0f; else uncontested_dodge_ += value; }
	void	add_uncontested_riposte(float value) { std::lock_guard<std::mutex> lk(classMutex); if(uncontested_riposte_ + value < 0.0f) uncontested_riposte_ = 0.0f; else uncontested_riposte_ += value; }
	void	add_crit_chance(float value) { std::lock_guard<std::mutex> lk(classMutex); if(crit_chance_ + value < 0.0f) crit_chance_ = 0.0f; else crit_chance_ += value; }
	void	add_crit_bonus(float value) { std::lock_guard<std::mutex> lk(classMutex); if(crit_bonus_ + value < 0.0f) crit_bonus_ = 0.0f; else crit_bonus_ += value; }
	void	add_potency(float value) { std::lock_guard<std::mutex> lk(classMutex); if(potency_ + value < 0.0f) potency_ = 0.0f; else potency_ += value; }
	void	add_hate_mod(float value) { std::lock_guard<std::mutex> lk(classMutex); if(hate_mod_ + value < 0.0f) hate_mod_ = 0.0f; else hate_mod_ += value; }
	void	add_reuse_speed(float value) { std::lock_guard<std::mutex> lk(classMutex); reuse_speed_ += value; }
	void	add_casting_speed(float value) { std::lock_guard<std::mutex> lk(classMutex); casting_speed_ += value; }
	void	add_recovery_speed(float value) { std::lock_guard<std::mutex> lk(classMutex); recovery_speed_ += value; }
	void	add_spell_reuse_speed(float value) { std::lock_guard<std::mutex> lk(classMutex); spell_reuse_speed_ += value; }
	void	add_spell_multi_attack(float value) { std::lock_guard<std::mutex> lk(classMutex); spell_multi_attack_ += value; }
	void	add_size_mod(float value) { std::lock_guard<std::mutex> lk(classMutex); size_mod_ += value; }
	void	add_dps(float value) { std::lock_guard<std::mutex> lk(classMutex); if(dps_ + value < 0.0f) dps_ = 0.0f; else dps_ += value; }
	void	add_dps_multiplier(float value) { std::lock_guard<std::mutex> lk(classMutex); if(dps_multiplier_ + value < 0.0f) dps_multiplier_ = 0.0f; else dps_multiplier_ += value; }
	void	add_attackspeed(float value) { std::lock_guard<std::mutex> lk(classMutex); if(attackspeed_ + value < 0.0f) attackspeed_ = 0.0f; else attackspeed_ += value; }
	void	add_haste(float value) { std::lock_guard<std::mutex> lk(classMutex); if(haste_ + value < 0.0f) haste_ = 0.0f; else haste_ += value; }
	void	add_multi_attack(float value) { std::lock_guard<std::mutex> lk(classMutex); if(multi_attack_ + value < 0.0f) multi_attack_ = 0.0f; else multi_attack_ += value; }
	void	add_flurry(float value) { std::lock_guard<std::mutex> lk(classMutex); if(flurry_ + value < 0.0f) flurry_ = 0.0f; else flurry_ += value; }
	void	add_melee_ae(float value) { std::lock_guard<std::mutex> lk(classMutex); if(melee_ae_ + value < 0.0f) melee_ae_ = 0.0f; else melee_ae_ += value; }
	void	add_strikethrough(float value) { std::lock_guard<std::mutex> lk(classMutex); if(strikethrough_ + value < 0.0f) strikethrough_ = 0.0f; else strikethrough_ += value; }
	void	add_accuracy(float value) { std::lock_guard<std::mutex> lk(classMutex); if(accuracy_ + value < 0.0f) accuracy_ = 0.0f; else accuracy_ += value; }
	void	add_offensivespeed(float value) { std::lock_guard<std::mutex> lk(classMutex); if(offensivespeed_ + value < 0.0f) offensivespeed_ = 0.0f; else offensivespeed_ += value; }
	void	add_rain(float value) { std::lock_guard<std::mutex> lk(classMutex); if(rain_ + value < 0.0f) rain_ = 0.0f; else rain_ += value; }
	void	add_wind(float value) { std::lock_guard<std::mutex> lk(classMutex); if(wind_ + value < 0.0f) wind_ = 0.0f; else wind_ += value; }

	void	set_alignment(int8 value) { std::lock_guard<std::mutex> lk(classMutex); alignment_ = value; }

	void	set_pet_id(int32 value) { std::lock_guard<std::mutex> lk(classMutex); pet_id_ = value; }
	void	set_pet_name(std::string value) { std::lock_guard<std::mutex> lk(classMutex); pet_name_ = value; }

	void	set_pet_movement(int8 value) { std::lock_guard<std::mutex> lk(classMutex); pet_movement_ = value; }
	void	set_pet_behavior(int8 value) { std::lock_guard<std::mutex> lk(classMutex); pet_behavior_ = value; }
	void	set_pet_health_pct(float value) { std::lock_guard<std::mutex> lk(classMutex); pet_health_pct_ = value; }
	void	set_pet_power_pct(float value) { std::lock_guard<std::mutex> lk(classMutex); pet_power_pct_ = value; }

	void	set_weight(int32 value) { std::lock_guard<std::mutex> lk(classMutex); weight_ = value; }
	void	set_max_weight(int32 value) { std::lock_guard<std::mutex> lk(classMutex); max_weight_ = value; }

	void	set_vision(int32 value) { std::lock_guard<std::mutex> lk(classMutex); vision_ = value; }
	void	set_breathe_underwater(int8 value) { std::lock_guard<std::mutex> lk(classMutex); breathe_underwater_ = value; }
	void	set_drunk(float value) { std::lock_guard<std::mutex> lk(classMutex); drunk_ = value; }

	void	set_biography(std::string value) { std::lock_guard<std::mutex> lk(classMutex); biography_ = value; }

	void	set_power_regen(sint16 value) { std::lock_guard<std::mutex> lk(classMutex); power_regen_ = value; }
	void	set_hp_regen(sint16 value) { std::lock_guard<std::mutex> lk(classMutex); hp_regen_ = value; }

	void	set_power_regen_override(int8 value) { std::lock_guard<std::mutex> lk(classMutex); power_regen_override_ = value; }
	void	set_hp_regen_override(int8 value) { std::lock_guard<std::mutex> lk(classMutex); hp_regen_override_ = value; }

	void	set_water_type(int8 value) { std::lock_guard<std::mutex> lk(classMutex); water_type_ = value; }
	void	set_flying_type(int8 value) { std::lock_guard<std::mutex> lk(classMutex); flying_type_ = value; }

	void	set_no_interrupt(int8 value) { std::lock_guard<std::mutex> lk(classMutex); no_interrupt_ = value; }

	void	set_interaction_flag(int8 value) { std::lock_guard<std::mutex> lk(classMutex); interaction_flag_ = value; }
	void	set_tag1(int8 value) { std::lock_guard<std::mutex> lk(classMutex); tag1_ = value; }
	void	set_mood(int16 value) { std::lock_guard<std::mutex> lk(classMutex); mood_ = value; }

	void	set_range_last_attack_time(int32 value) { std::lock_guard<std::mutex> lk(classMutex); range_last_attack_time_ = value; }
	void	set_primary_last_attack_time(int32 value) { std::lock_guard<std::mutex> lk(classMutex); primary_last_attack_time_ = value; }
	void	set_secondary_last_attack_time(int32 value) { std::lock_guard<std::mutex> lk(classMutex); secondary_last_attack_time_ = value; }
	
	void	set_primary_attack_delay(int16 value) { std::lock_guard<std::mutex> lk(classMutex); primary_attack_delay_ = value; }
	void	set_secondary_attack_delay(int16 value) { std::lock_guard<std::mutex> lk(classMutex); secondary_attack_delay_ = value; }
	void	set_ranged_attack_delay(int16 value) { std::lock_guard<std::mutex> lk(classMutex); ranged_attack_delay_ = value; }

	void	set_primary_weapon_type(int8 value) { std::lock_guard<std::mutex> lk(classMutex); primary_weapon_type_ = value; }
	void	set_secondary_weapon_type(int8 value) { std::lock_guard<std::mutex> lk(classMutex); secondary_weapon_type_ = value; }
	void	set_ranged_weapon_type(int8 value) { std::lock_guard<std::mutex> lk(classMutex); ranged_weapon_type_ = value; }
	
	void	set_primary_weapon_damage_low(int32 value) { std::lock_guard<std::mutex> lk(classMutex); primary_weapon_damage_low_ = value; }
	void	set_primary_weapon_damage_high(int32 value) { std::lock_guard<std::mutex> lk(classMutex); primary_weapon_damage_high_ = value; }
	void	set_secondary_weapon_damage_low(int32 value) { std::lock_guard<std::mutex> lk(classMutex); secondary_weapon_damage_low_ = value; }
	void	set_secondary_weapon_damage_high(int32 value) { std::lock_guard<std::mutex> lk(classMutex); secondary_weapon_damage_high_ = value; }
	void	set_ranged_weapon_damage_low(int32 value) { std::lock_guard<std::mutex> lk(classMutex); ranged_weapon_damage_low_ = value; }
	void	set_ranged_weapon_damage_high(int32 value) { std::lock_guard<std::mutex> lk(classMutex); ranged_weapon_damage_high_ = value; }
	
	void	set_wield_type(int8 value) { std::lock_guard<std::mutex> lk(classMutex); wield_type_ = value; }
	void	set_attack_type(int8 value) { std::lock_guard<std::mutex> lk(classMutex); attack_type_ = value; }
	
	void	set_primary_weapon_delay(int16 value) { std::lock_guard<std::mutex> lk(classMutex); primary_weapon_delay_ = value; }
	void	set_secondary_weapon_delay(int16 value) { std::lock_guard<std::mutex> lk(classMutex); secondary_weapon_delay_ = value; }
	void	set_ranged_weapon_delay(int16 value) { std::lock_guard<std::mutex> lk(classMutex); ranged_weapon_delay_ = value; }
	
	void	set_override_primary_weapon(int8 value) { std::lock_guard<std::mutex> lk(classMutex); override_primary_weapon_ = value; }
	void	set_override_secondary_weapon(int8 value) { std::lock_guard<std::mutex> lk(classMutex); override_secondary_weapon_ = value; }
	void	set_override_ranged_weapon(int8 value) { std::lock_guard<std::mutex> lk(classMutex); override_ranged_weapon_ = value; }
	void	set_friendly_target_npc(int8 value) { std::lock_guard<std::mutex> lk(classMutex); friendly_target_npc_ = value; }
	void	set_last_claim_time(int32 value) { std::lock_guard<std::mutex> lk(classMutex); last_claim_time_ = value; }
	
	void	set_engaged_encounter(int8 value) { std::lock_guard<std::mutex> lk(classMutex); engaged_encounter_ = value; }
	void	set_lockable_encounter(int8 value) { std::lock_guard<std::mutex> lk(classMutex); lockable_encounter_ = value; }
	
	void	set_first_world_login(int8 value) { std::lock_guard<std::mutex> lk(classMutex); first_world_login_ = value; }
	
	void	set_reload_player_spells(int8 value) { std::lock_guard<std::mutex> lk(classMutex); reload_player_spells_ = value; }
	
	void	set_group_loot_method(int8 value) { std::lock_guard<std::mutex> lk(classMutex); group_loot_method_ = value; }
	void	set_group_loot_items_rarity(int8 value) { std::lock_guard<std::mutex> lk(classMutex); group_loot_items_rarity_ = value; }
	void	set_group_auto_split(int8 value) { std::lock_guard<std::mutex> lk(classMutex); group_auto_split_ = value;  }
	void	set_group_default_yell(int8 value) { std::lock_guard<std::mutex> lk(classMutex); group_default_yell_ = value; }
	void	set_group_autolock(int8 value) { std::lock_guard<std::mutex> lk(classMutex); group_autolock_ = value;  }
	void	set_group_lock_method(int8 value) { std::lock_guard<std::mutex> lk(classMutex); group_lock_method_ = value;  }
	void	set_group_solo_autolock(int8 value) { std::lock_guard<std::mutex> lk(classMutex); group_solo_autolock_ = value; }
	void	set_group_auto_loot_method(int8 value) { std::lock_guard<std::mutex> lk(classMutex); group_auto_loot_method_ = value; }
	
	void	set_assist_auto_attack(int8 value) { std::lock_guard<std::mutex> lk(classMutex); assist_auto_attack_ = value; }

	void	set_action_state(std::string value) { std::lock_guard<std::mutex> lk(classMutex); action_state_ = value; }
	
	void	set_combat_action_state(std::string value) { std::lock_guard<std::mutex> lk(classMutex); combat_action_state_ = value; }
	
	void	set_max_spell_reduction(float value) { std::lock_guard<std::mutex> lk(classMutex); max_spell_reduction_ = value; }
	
	void	set_max_spell_reduction_override(int8 value) { std::lock_guard<std::mutex> lk(classMutex); max_spell_reduction_override_ = value; }
	
	void	ResetEffects(Spawn* spawn)
	{
		for(int i=0;i<45;i++){
			if(i<30){
				maintained_effects[i].spell_id = 0xFFFFFFFF;
				maintained_effects[i].inherited_spell_id = 0;
				if (spawn->IsPlayer())
					maintained_effects[i].icon = 0xFFFF;

				maintained_effects[i].spell = nullptr;
			}
			spell_effects[i].icon = 0;	
			spell_effects[i].spell_id = 0xFFFFFFFF;	
			spell_effects[i].inherited_spell_id = 0;
			spell_effects[i].icon_backdrop = 0;
			spell_effects[i].tier = 0;
			spell_effects[i].total_time = 0.0f;
			spell_effects[i].expire_timestamp = 0;
			spell_effects[i].spell = nullptr;
		}
	}
	
	// maintained via their own mutex
	SpellEffects	spell_effects[45];
	MaintainedEffects maintained_effects[30];
private:
	std::string		name_;
	int8			class1_;
	int8			class2_;
	int8			class3_;
	int8			race_;
	int8			gender_;
	int16			level_;
	int16			max_level_;
	int16			effective_level_;
	int16			tradeskill_level_;
	int16			tradeskill_max_level_;
	
	int8			cur_concentration_;
	int8			max_concentration_;
	int8			max_concentration_base_;
	int16			cur_attack_;
	int16			attack_base_;
	int16			cur_mitigation_;
	int16			max_mitigation_;
	int16			mitigation_base_;
	sint16			mitigation_modifier_;
	int16			avoidance_display_;
	float			cur_avoidance_;
	int16			base_avoidance_pct_;
	int16			avoidance_base_;
	int16			max_avoidance_;
	float			parry_;
	float			parry_base_;
	float			deflection_;
	int16			deflection_base_;
	float			block_;
	int16			block_base_;
	float			riposte_;
	float			riposte_base_;
	float			str_; //int16
	float			sta_; //int16
	float			agi_;//int16
	float			wis_;//int16
	float			intel_;//int16
	float			str_base_;//int16
	float			sta_base_;//int16
	float			agi_base_;//int16
	float			wis_base_;//int16
	float			intel_base_;//int16
	int16			heat_;
	int16			cold_;
	int16			magic_;
	int16			mental_;
	int16			divine_;
	int16			disease_;
	int16			poison_;
	int16			disease_base_;
	int16			cold_base_;
	int16			divine_base_;
	int16			magic_base_;
	int16			mental_base_;
	int16			heat_base_;
	int16			poison_base_;
	int16			elemental_base_;
	int16			noxious_base_;
	int16			arcane_base_;
	int32			coin_copper_;
	int32			coin_silver_;
	int32			coin_gold_;
	int32			coin_plat_;
	int32			bank_coin_copper_;
	int32			bank_coin_silver_;
	int32			bank_coin_gold_;
	int32			bank_coin_plat_;
	
	int32			status_points_;
	std::string		deity_;
	int32			weight_;
	int32			max_weight_;
	int8			tradeskill_class1_;
	int8			tradeskill_class2_;
	int8			tradeskill_class3_;
	int32			account_age_base_;
	int8			account_age_bonus_[19];
	int16			absorb_;
	int32			xp_;
	int32			xp_needed_;
	float			xp_debt_;
	int16			xp_yellow_;
	int16			xp_yellow_vitality_bar_;
	int16			xp_blue_vitality_bar_;
	int16			xp_blue_;
	int32			ts_xp_;
	int32			ts_xp_needed_;
	int16			tradeskill_exp_yellow_;
	int16			tradeskill_exp_blue_;
	int32			flags_;
	int32			flags2_;
	float			xp_vitality_;
	float			tradeskill_xp_vitality_;
	int16			mitigation_skill1_;
	int16			mitigation_skill2_;
	int16			mitigation_skill3_;
	int16			mitigation_pve_;
	int16			mitigation_pvp_;
	float			ability_modifier_;
	float			critical_mitigation_;
	float			block_chance_;
	float			uncontested_parry_;
	float			uncontested_block_;
	float			uncontested_dodge_;
	float			uncontested_riposte_;

	float			crit_chance_;
	float			crit_bonus_;
	float			potency_;
	float			hate_mod_;
	float			reuse_speed_;
	float			casting_speed_;
	float			recovery_speed_;
	float			spell_reuse_speed_;
	float			spell_multi_attack_;
	float			size_mod_;
	int8			ignore_size_mod_calc_;
	float			dps_;
	float           dps_multiplier_;
	float			attackspeed_;
	float			haste_;
	float			multi_attack_;
	float			flurry_;
	float			melee_ae_;
	float			strikethrough_;
	float			accuracy_;
	float			offensivespeed_;
	float			rain_;
	float			wind_;
	sint8			alignment_;

	int32			pet_id_;
	std::string		pet_name_;
	float			pet_health_pct_;
	float			pet_power_pct_;
	int8			pet_movement_;
	int8			pet_behavior_;

	int32          	vision_;
	int8			breathe_underwater_;
	std::string		biography_;
	float			drunk_;

	sint16			power_regen_;
	sint16			hp_regen_;

	int8			power_regen_override_;
	int8			hp_regen_override_;

	int8			water_type_;
	int8			flying_type_;

	int8			no_interrupt_;

	int8			interaction_flag_;
	int8			tag1_;
	int16			mood_;
	
	int32			range_last_attack_time_;
	int32			primary_last_attack_time_;
	int32			secondary_last_attack_time_;
	int16			primary_attack_delay_;
	int16			secondary_attack_delay_;
	int16			ranged_attack_delay_;
	int8			primary_weapon_type_;
	int8			secondary_weapon_type_;
	int8			ranged_weapon_type_;
	int32			primary_weapon_damage_low_;
	int32			primary_weapon_damage_high_;
	int32			secondary_weapon_damage_low_;
	int32			secondary_weapon_damage_high_;
	int32			ranged_weapon_damage_low_;
	int32			ranged_weapon_damage_high_;
	int8			wield_type_;
	int8			attack_type_;
	int16           primary_weapon_delay_;
	int16           secondary_weapon_delay_;
	int16           ranged_weapon_delay_;
	
	int8			override_primary_weapon_;
	int8			override_secondary_weapon_;
	int8			override_ranged_weapon_;
	
	int8			friendly_target_npc_;
	int32			last_claim_time_;
	
	int8			engaged_encounter_;
	int8			lockable_encounter_;
	
	int8			first_world_login_;
	int8			reload_player_spells_;
	
	int8			group_loot_method_;
	int8			group_loot_items_rarity_;
	int8			group_auto_split_;
	int8			group_default_yell_;
	int8			group_autolock_;
	int8			group_lock_method_;
	int8			group_solo_autolock_;
	int8			group_auto_loot_method_;
	
	int8			assist_auto_attack_;
	
	std::string		action_state_;
	std::string		combat_action_state_;
	
	float			max_spell_reduction_;
	int8			max_spell_reduction_override_;
	
	float			max_chase_distance_;
	// when PacketStruct is fixed for C++17 this should become a shared_mutex and handle read/write lock
	std::mutex		classMutex;
};

struct WardInfo {
	LuaSpell*	Spell;
	int32		BaseDamage;
	int32		DamageLeft;
	int8		WardType;
	int8		DamageType;
	bool		keepWard;
	int32		DamageAbsorptionPercentage;
	int32		DamageAbsorptionMaxHealthPercent;
	int32		RedirectDamagePercent;
	
	int32		LastRedirectDamage;
	int32		LastAbsorbedDamage;

	int32		HitCount;
	int32		MaxHitCount;

	bool		AbsorbAllDamage; // damage is always absorbed, usually spells based on hits, when we pass damage in AddWard as 0 this will be set to true
	
	bool		RoundTriggered;
	
	bool		DeleteWard; // removal after process CheckWard while loop
};

#define WARD_TYPE_ALL 0
#define WARD_TYPE_PHYSICAL 1
#define WARD_TYPE_MAGICAL 2

struct Proc {
	LuaSpell*	spell;
	Item*		item;
	float		chance;
	int32		spellid;
	int8		health_ratio;
	bool		below_health;
	bool		target_health;
	int8		damage_type;
	bool		extended_version;
	int32		initial_caster_entity_id;
};

#define PROC_TYPE_OFFENSIVE				1
#define PROC_TYPE_DEFENSIVE				2
#define PROC_TYPE_PHYSICAL_OFFENSIVE	3
#define PROC_TYPE_PHYSICAL_DEFENSIVE	4
#define PROC_TYPE_MAGICAL_OFFENSIVE		5
#define PROC_TYPE_MAGICAL_DEFENSIVE		6
#define PROC_TYPE_BLOCK					7
#define PROC_TYPE_PARRY					8
#define PROC_TYPE_RIPOSTE				9
#define PROC_TYPE_EVADE					10
#define PROC_TYPE_HEALING				11
#define PROC_TYPE_BENEFICIAL			12
#define PROC_TYPE_DEATH                 13
#define PROC_TYPE_KILL                  14
#define PROC_TYPE_DAMAGED               15
#define PROC_TYPE_DAMAGED_MELEE         16
#define PROC_TYPE_DAMAGED_MAGIC         17
#define PROC_TYPE_RANGED_ATTACK			18
#define PROC_TYPE_RANGED_DEFENSE		19

struct ThreatTransfer {
	int32		Target;
	float		Amount;
	LuaSpell*	Spell;
};

#define DET_TYPE_TRAUMA      1
#define DET_TYPE_ARCANE      2
#define DET_TYPE_NOXIOUS     3
#define DET_TYPE_ELEMENTAL   4
#define DET_TYPE_CURSE       5

#define DISPELL_TYPE_CURE    0
#define DISPELL_TYPE_DISPELL 1

#define CONTROL_EFFECT_TYPE_MEZ 1
#define CONTROL_EFFECT_TYPE_STIFLE 2
#define CONTROL_EFFECT_TYPE_DAZE 3
#define CONTROL_EFFECT_TYPE_STUN 4
#define CONTROL_EFFECT_TYPE_ROOT 5
#define CONTROL_EFFECT_TYPE_FEAR 6
#define CONTROL_EFFECT_TYPE_WALKUNDERWATER 7
#define CONTROL_EFFECT_TYPE_JUMPUNDERWATER 8
#define CONTROL_EFFECT_TYPE_INVIS 9
#define CONTROL_EFFECT_TYPE_STEALTH 10
#define CONTROL_EFFECT_TYPE_SNARE 11
#define CONTROL_EFFECT_TYPE_FLIGHT 12
#define CONTROL_EFFECT_TYPE_GLIDE 13
#define CONTROL_EFFECT_TYPE_SAFEFALL 14
#define CONTROL_MAX_EFFECTS 15 // always +1 to highest control effect

#define IMMUNITY_TYPE_MEZ 1
#define IMMUNITY_TYPE_STIFLE 2
#define IMMUNITY_TYPE_DAZE 3
#define IMMUNITY_TYPE_STUN 4
#define IMMUNITY_TYPE_ROOT 5
#define IMMUNITY_TYPE_FEAR 6
#define IMMUNITY_TYPE_AOE 7
#define IMMUNITY_TYPE_TAUNT 8
#define IMMUNITY_TYPE_RIPOSTE 9
#define IMMUNITY_TYPE_STRIKETHROUGH 10

//class Spell;
//class ZoneServer;

//The entity class is for NPCs and Players, spawns which are able to fight
class Entity : public Spawn{
public:
	Entity();
	virtual ~Entity();

	void DeleteSpellEffects(bool removeClient = false);
	void RemoveSpells(bool unfriendlyOnly = false);
	void MapInfoStruct();
	virtual float GetDodgeChance();
	virtual void AddMaintainedSpell(LuaSpell* spell);
	virtual void AddSpellEffect(LuaSpell* spell, int32 override_expire_time = 0);
	virtual void RemoveMaintainedSpell(LuaSpell* spell);
	virtual void RemoveSpellEffect(LuaSpell* spell);
	virtual void AddSkillBonus(int32 spell_id, int32 skill_id, float value);
	void AddDetrimentalSpell(LuaSpell* spell, int32 override_expire_timestamp = 0);
	DetrimentalEffects* GetDetrimentalEffect(int32 spell_id, Entity* caster);
	virtual MaintainedEffects* GetMaintainedSpell(int32 spell_id, bool on_char_load = false);
	void RemoveDetrimentalSpell(LuaSpell* spell);
	void	SetDeity(int8 new_deity){
			deity = new_deity;
	}
	int8	GetDeity(){ return deity; }
	EquipmentItemList* GetEquipmentList();
	EquipmentItemList* GetAppearanceEquipmentList();

	bool 	IsEntity(){ return true; }
	float 	CalculateSkillStatChance(char* skill, int16 item_stat, float max_cap = 0.0f, float modifier = 0.0f, bool add_to_skill = false);
	float 	CalculateSkillWithBonus(char* skillName, int16 item_stat, bool chance_skill_increase);
	float 	GetRuleSkillMaxBonus();
	void 	CalculateBonuses();
	float	CalculateLevelStatBonus(int16 stat_value);
	void 	CalculateApplyWeight();
	void 	SetRegenValues(int16 effective_level);
	float 	CalculateBonusMod();
	float 	CalculateDPSMultiplier();
	float 	CalculateCastingSpeedMod();
	
	InfoStruct* GetInfoStruct();

	int16	GetStr();
	int16	GetSta();
	int16	GetInt();
	int16	GetWis();
	int16	GetAgi();
	int16   GetPrimaryStat();

	int16	GetHeatResistance();
	int16	GetColdResistance();
	int16	GetMagicResistance();
	int16	GetMentalResistance();
	int16	GetDivineResistance();
	int16	GetDiseaseResistance();
	int16	GetPoisonResistance();

	int16	GetStrBase();
	int16	GetStaBase();
	int16	GetIntBase();
	int16	GetWisBase();
	int16	GetAgiBase();

	int16	GetHeatResistanceBase();
	int16	GetColdResistanceBase();
	int16	GetMagicResistanceBase();
	int16	GetMentalResistanceBase();
	int16	GetDivineResistanceBase();
	int16	GetDiseaseResistanceBase();
	int16	GetPoisonResistanceBase();

	int8	GetConcentrationCurrent();
	int8	GetConcentrationMax();

	sint8	GetAlignment();
	void	SetAlignment(sint8 new_value);

	bool	HasMoved(bool include_heading);
	void	SetHPRegen(int16 new_val);
	int16	GetHPRegen();
	void	DoRegenUpdate();
	MaintainedEffects* GetFreeMaintainedSpellSlot();
	SpellEffects* GetFreeSpellEffectSlot();
	SpellEffects* GetSpellEffect(int32 id, Entity* caster = 0, bool on_char_load = false);
	SpellEffects* GetSpellEffectBySpellType(int8 spell_type);
	SpellEffects* GetSpellEffectWithLinkedTimer(int32 id, int32 linked_timer = 0, sint32 type_group_spell_id = 0, Entity* caster = 0, bool notCaster = false);
	LuaSpell* HasLinkedTimerID(LuaSpell* spell, Spawn* target = nullptr,  bool stackWithOtherPlayers = true, bool checkNotCaster = false);

	//flags
	int32 GetFlags() { return info_struct.get_flags(); }
	int32	GetFlags2() { return info_struct.get_flags2(); }
	bool query_flags(int flag) {
			if (flag > 63) return false;
			if (flag < 32) return ((info_struct.get_flags() & (1 << flag))?true:false);
			return ((info_struct.get_flags2() & (1 << (flag - 32)))?true:false);
	}
	float	GetMaxSpeed();
	void	SetMaxSpeed(float val);
	//combat stuff:
	int32	GetRangeLastAttackTime();
	void	SetRangeLastAttackTime(int32 time);
	int16	GetRangeAttackDelay();
	int16   GetRangeWeaponDelay();
	void    SetRangeWeaponDelay(int16 new_delay);
	void    SetRangeAttackDelay(int16 new_delay);
	int32	GetPrimaryLastAttackTime();
	int16	GetPrimaryAttackDelay();
	void	SetPrimaryAttackDelay(int16 new_delay);
	void	SetPrimaryLastAttackTime(int32 new_time);
	void    SetPrimaryWeaponDelay(int16 new_delay);
	int32	GetSecondaryLastAttackTime();
	int16	GetSecondaryAttackDelay();
	void	SetSecondaryAttackDelay(int16 new_delay);
	void	SetSecondaryLastAttackTime(int32 new_time);
	void    SetSecondaryWeaponDelay(int16 new_delay);
	int32	GetPrimaryWeaponMinDamage();
	int32	GetPrimaryWeaponMaxDamage();
	int32	GetSecondaryWeaponMinDamage();
	int32	GetSecondaryWeaponMaxDamage();
	int32	GetRangedWeaponMinDamage();
	int32	GetRangedWeaponMaxDamage();
	int8	GetPrimaryWeaponType();
	int8	GetSecondaryWeaponType();
	int8	GetRangedWeaponType();
	int8	GetWieldType();
	int16   GetPrimaryWeaponDelay();
	int16   GetSecondaryWeaponDelay();
	bool	IsDualWield();
	bool	BehindTarget(Spawn* target);
	bool	FlankingTarget(Spawn* target);
	
	void	GetWeaponDamage(Item* item, int32* low_damage, int32* high_damage);
	void	ChangePrimaryWeapon();
	void	ChangeSecondaryWeapon();
	void	ChangeRangedWeapon();
	void	UpdateWeapons();
	int32	GetStrengthDamage();
	virtual Skill*	GetSkillByName(const char* name, bool check_update = false);
	virtual Skill*	GetSkillByID(int32 id, bool check_update = false);
	bool			AttackAllowed(Entity* target, float distance = 0, bool range_attack = false);
	Item*			GetAmmoFromSlot(bool is_ammo, bool is_thrown);
	bool			PrimaryWeaponReady();
	bool			SecondaryWeaponReady();
	bool			RangeWeaponReady();
	void			MeleeAttack(Spawn* victim, float distance, bool primary, bool multi_attack = false);
	bool			RangeAttack(Spawn* victim, float distance, Item* weapon, Item* ammo, bool multi_attack = false);
	bool			SpellAttack(Spawn* victim, float distance, LuaSpell* luaspell, int8 damage_type, int32 low_damage, int32 high_damage, int8 crit_mod = 0, bool no_calcs = false, int8 override_packet_type = 0, bool take_power = false);
	bool			ProcAttack(Spawn* victim, int8 damage_type, int32 low_damage, int32 high_damage, string name, string success_msg, string effect_msg);
	bool            SpellHeal(Spawn* target, float distance, LuaSpell* luaspell, string heal_type, int32 low_heal, int32 high_heal, int8 crit_mod = 0, bool no_calcs = false, string custom_spell_name="");
	int8			DetermineHit(Spawn* victim, int8 type, int8 damage_type, float ToHitBonus, bool is_caster_spell, LuaSpell* lua_spell = nullptr);
	float			GetDamageTypeResistPercentage(int8 damage_type);
	Skill*			GetSkillByWeaponType(int8 type, int8 damage_type, bool update);
	bool			DamageSpawn(Entity* victim, int8 type, int8 damage_type, int32 low_damage, int32 high_damage, const char* spell_name, int8 crit_mod = 0, bool is_tick = false, bool no_damage_calcs = false, bool ignore_attacker = false, bool take_power = false, LuaSpell* spell = 0, bool skip_check_wards = false);
	float			CalculateMitigation(int8 type = DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE, int8 damage_type = 0, int16 attacker_level = 0, bool for_pvp = false);
	void			AddHate(Entity* attacker, sint32 hate, bool ignore_pet_behavior = false);
	bool			CheckInterruptSpell(Entity* attacker);
	bool			CheckFizzleSpell(LuaSpell* spell);
	void			KillSpawn(Spawn* dead, int8 type = 0, int8 damage_type = 0, int16 kill_blow_type = 0);
	void			HandleDeathExperienceDebt(Spawn* killer);
	void            SetAttackDelay(bool primary = false, bool ranged = false);
	float           CalculateAttackSpeedMod();
	virtual void	ProcessCombat();

	bool	EngagedInCombat();
	virtual void	InCombat(bool val);

	bool	IsCasting();
	void	IsCasting(bool val);
	void SetMount(int16 mount_id, int8 red = 0xFF, int8 green = 0xFF, int8 blue = 0xFF, bool setUpdateFlags = true)
	{
		if (mount_id == 0) {
			EQ2_Color color;
			color.red = 0;
			color.green = 0;
			color.blue = 0;
			SetMountColor(&color);
			SetMountSaddleColor(&color);
		}
		else
		{
			EQ2_Color color;
			color.red = red;
			color.green = green;
			color.blue = blue;
			SetMountColor(&color);
			SetMountSaddleColor(&color);
		}
		SetInfo(&features.mount_model_type, mount_id, setUpdateFlags);
	}

	void SetEquipment(Item* item, int8 slot = 255);
	void SetEquipment(int8 slot, int16 type, int8 red, int8 green, int8 blue, int8 h_r, int8 h_g, int8 h_b){
		std::lock_guard<std::mutex> lk(MEquipment);
		if(slot >= NUM_SLOTS)
			return;
		
		SetInfo(&equipment.equip_id[slot], type);
		SetInfo(&equipment.color[slot].red, red);
		SetInfo(&equipment.color[slot].green, green);
		SetInfo(&equipment.color[slot].blue, blue);
		SetInfo(&equipment.highlight[slot].red, h_r);
		SetInfo(&equipment.highlight[slot].green, h_g);
		SetInfo(&equipment.highlight[slot].blue, h_b);
	}
	void SetHairType(int16 new_val, bool setUpdateFlags = true){
		SetInfo(&features.hair_type, new_val, setUpdateFlags);
	}
	void SetHairColor1(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.hair_color1, new_val, setUpdateFlags);
	}
	void SetHairColor2(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.hair_color2, new_val, setUpdateFlags);
	}
	void SetSogaHairColor1(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.soga_hair_color1, new_val, setUpdateFlags);
	}
	void SetSogaHairColor2(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.soga_hair_color2, new_val, setUpdateFlags);
	}
	void SetHairHighlightColor(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.hair_highlight_color, new_val, setUpdateFlags);
	}
	void SetSogaHairHighlightColor(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.soga_hair_highlight_color, new_val, setUpdateFlags);
	}
	void SetHairColor(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.hair_type_color, new_val, setUpdateFlags);
	}
	void SetSogaHairColor(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.soga_hair_type_color, new_val, setUpdateFlags);
	}
	void SetHairTypeHighlightColor(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.hair_type_highlight_color, new_val, setUpdateFlags);
	}
	void SetSogaHairTypeHighlightColor(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.soga_hair_type_highlight_color, new_val, setUpdateFlags);
	}
	void SetFacialHairType(int16 new_val, bool setUpdateFlags = true){
		SetInfo(&features.hair_face_type, new_val, setUpdateFlags);
	}
	void SetFacialHairColor(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.hair_face_color, new_val, setUpdateFlags);
	}
	void SetSogaFacialHairColor(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.soga_hair_face_color, new_val, setUpdateFlags);
	}
	void SetFacialHairHighlightColor(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.hair_face_highlight_color, new_val, setUpdateFlags);
	}
	void SetSogaFacialHairHighlightColor(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.soga_hair_face_highlight_color, new_val, setUpdateFlags);
	}
	void SetWingType(int16 new_val, bool setUpdateFlags = true){
		SetInfo(&features.wing_type, new_val, setUpdateFlags);
	}
	void SetWingColor1(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.wing_color1, new_val, setUpdateFlags);
	}
	void SetWingColor2(EQ2_Color new_val, bool setUpdateFlags = true){
		SetInfo(&features.wing_color2, new_val, setUpdateFlags);
	}
	void SetChestType(int16 new_val, bool setUpdateFlags = true){
		SetInfo(&features.chest_type, new_val, setUpdateFlags);
	}
	void SetLegsType(int16 new_val, bool setUpdateFlags = true){
		SetInfo(&features.legs_type, new_val, setUpdateFlags);
	}
	void SetSogaHairType(int16 new_val, bool setUpdateFlags = true){
		SetInfo(&features.soga_hair_type, new_val, setUpdateFlags);
	}	
	void SetSogaFacialHairType(int16 new_val, bool setUpdateFlags = true){
		SetInfo(&features.soga_hair_face_type, new_val, setUpdateFlags);
	}	
	void SetSogaChestType(int16 new_val, bool setUpdateFlags = true){
		SetInfo(&features.soga_chest_type, new_val, setUpdateFlags);
	}	
	void SetSogaLegType(int16 new_val, bool setUpdateFlags = true){
		SetInfo(&features.soga_legs_type, new_val, setUpdateFlags);
	}	
	void SetSkinColor(EQ2_Color color){
		SetInfo(&features.skin_color, color);
	}
	void SetSogaSkinColor(EQ2_Color color){
		SetInfo(&features.soga_skin_color, color);
	}
	void SetModelColor(EQ2_Color color){
		SetInfo(&features.model_color, color);
	}
	void SetSogaModelColor(EQ2_Color color){
		SetInfo(&features.soga_model_color, color);
	}
	void SetCombatVoice(int16 val, bool setUpdateFlags = true) { 
		SetInfo(&features.combat_voice, val, setUpdateFlags); 
	}
	void SetEmoteVoice(int16 val, bool setUpdateFlags = true) { 
		SetInfo(&features.emote_voice, val, setUpdateFlags); 
	}
	int16 GetCombatVoice(){ return features.combat_voice; }
	int16 GetEmoteVoice(){ return features.emote_voice; }
	int16 GetMount(){ return features.mount_model_type; }
	void SetMountSaddleColor(EQ2_Color* color){
		SetInfo(&features.mount_saddle_color, *color);
	}
	void SetMountColor(EQ2_Color* color){
		SetInfo(&features.mount_color, *color);
	}
	void SetEyeColor(EQ2_Color eye_color){
		SetInfo(&features.eye_color, eye_color);
	}
	void SetSogaEyeColor(EQ2_Color eye_color){
		SetInfo(&features.soga_eye_color, eye_color);
	}
	int16 GetHairType(){
		return features.hair_type;
	}
	int16 GetFacialHairType(){
		return features.hair_face_type;
	}
	int16 GetWingType(){
		return features.wing_type;
	}
	int16 GetChestType(){
		return features.chest_type;
	}
	int16 GetLegsType(){
		return features.legs_type;
	}
	int16 GetSogaHairType(){
		return features.soga_hair_type;
	}
	int16 GetSogaFacialHairType(){
		return features.soga_hair_face_type;
	}
	int16 GetSogaChestType(){
		return features.soga_chest_type;
	}
	int16 GetSogaLegType(){
		return features.soga_legs_type;
	}
	EQ2_Color* GetSkinColor(){
		return &features.skin_color;
	}
	EQ2_Color* GetModelColor(){
		return &features.model_color;
	}
	EQ2_Color* GetSogaModelColor(){
		return &features.soga_model_color;
	}
	EQ2_Color* GetEyeColor(){
		return &features.eye_color;
	}
	EQ2_Color* GetMountSaddleColor(){
		return &features.mount_saddle_color;
	}
	EQ2_Color* GetMountColor(){
		return &features.mount_color;
	}	
	// should only be accessed through MEquipment mutex
	EQ2_Equipment	equipment;
	CharFeatures	features;	

	void AddSpellBonus(LuaSpell* spell, int16 type, float value, int64 class_req =0, vector<int16> race_req = vector<int16>(), vector<int16> faction_req = vector<int16>());
	BonusValues* GetSpellBonus(int32 spell_id);
	vector<BonusValues*>* GetAllSpellBonuses(LuaSpell* spell);
	bool CheckSpellBonusRemoval(LuaSpell* spell, int16 type);
	void RemoveSpellBonus(const LuaSpell* spell, bool remove_all = false);
	void RemoveAllSpellBonuses();
	void CalculateSpellBonuses(ItemStatsValues* stats);
	void AddMezSpell(LuaSpell* spell);
	void RemoveMezSpell(LuaSpell* spell);
	void RemoveAllMezSpells();
	bool IsMezzed();
	void AddStifleSpell(LuaSpell* spell);
	void RemoveStifleSpell(LuaSpell* spell);
	bool IsStifled();
	void AddDazeSpell(LuaSpell* spell);
	void RemoveDazeSpell(LuaSpell* spell);
	bool IsDazed();
	void AddStunSpell(LuaSpell* spell);
	void RemoveStunSpell(LuaSpell* spell);
	bool IsStunned();
	bool IsMezzedOrStunned() {return IsMezzed() || IsStunned();}
	void AddRootSpell(LuaSpell* spell);
	void RemoveRootSpell(LuaSpell* spell);
	bool IsRooted();
	void AddFearSpell(LuaSpell* spell);
	void RemoveFearSpell(LuaSpell* spell);
	bool IsFeared();
	void AddSnareSpell(LuaSpell* spell);
	void RemoveSnareSpell(LuaSpell* spell);
	void SetSnareValue(LuaSpell* spell, float snare_val);
	bool IsSnared();
	float GetHighestSnare();

	bool HasControlEffect(int8 type);

	void HaltMovement();


	void SetCombatPet(Entity* pet) { this->pet = pet; }
	void SetCharmedPet(Entity* pet) { charmedPet = pet; }
	void SetDeityPet(Entity* pet) { deityPet = pet; }
	void SetCosmeticPet(Entity* pet) { cosmeticPet = pet; }
	Entity* GetPet() { return pet; }
	Entity* GetCharmedPet() { return charmedPet; }
	Entity* GetDeityPet() { return deityPet; }
	Entity* GetCosmeticPet() { return cosmeticPet; }
	/// <summary>Check to see if the entity has a combat pet</summary>
	/// <returns>True if the entity has a combat pet</returns>
	bool HasPet() { return (pet || charmedPet) ? true : false; }

	void HideDeityPet(bool val);
	void HideCosmeticPet(bool val);
	void DismissPet(Entity* pet, bool from_death = false, bool spawnListLocked = false);
	void DismissAllPets(bool from_death = false, bool spawnListLocked = false);

	void	SetOwner(Entity* owner) { if (owner) { this->owner = owner->GetID(); } else { owner = 0; } }
	Entity*	GetOwner();
	int8	GetPetType() { return m_petType; }
	void	SetPetType(int8 val) { m_petType = val; }
	void	SetPetSpellID(int32 val) { m_petSpellID = val; }
	int32	GetPetSpellID() { return m_petSpellID; }
	void	SetPetSpellTier(int8 val) { m_petSpellTier = val; }
	int8	GetPetSpellTier() { return m_petSpellTier; }
	bool IsDismissing() { return m_petDismissing; }
	void SetDismissing(bool val) { m_petDismissing = val; }

	/// <summary>Creates a loot chest to drop in the world</summary>
	/// <returns>Pointer to the chest</returns>
	NPC* DropChest();

	/// <summary>Add a ward to the entities ward list</summary>
	/// <param name='spellID'>Spell id of the ward to add</param>
	/// <param name='ward'>WardInfo* of the ward we are adding</parma>
	void AddWard(int32 spellID, WardInfo* ward);

	/// <summary>Gets ward info for the given spell id</summary>
	/// <param name='spellID'>The spell id of the ward we want to get</param>
	/// <returns>WardInfo for the given spell id</returns>
	WardInfo* GetWard(int32 spellID);

	/// <summary>Removes the ward with the given spell id</summary>
	/// <param name='spellID'>The spell id of the ward to remove</param>
	void RemoveWard(int32 spellID);
	void RemoveWard(LuaSpell* spell);

	/// <summary>Subtracts the given damage from the wards</summary>
	/// <param name='damage'>The damage to subtract from the wards</param>
	/// <returns>The amount of damage left after wards</returns>
	int32 CheckWards(Entity* attacker, int32 damage, int8 damage_type);

	map<int16, float> stats;

	/// <summary>Adds a proc to the list of current procs</summary>
	/// <param name='type'>The type of proc to add</param>
	/// <param name='chance'>The percent chance the proc has to go off</param>
	/// <param name='item'>The item the proc is coming from if any</param>
	/// <param name='spell'>The spell the proc is coming from if any</param>
	void AddProc(int8 type, float chance, Item* item = 0, LuaSpell* spell = 0, int8 damage_type = 0, int8 hp_ratio = 0, bool below_health = false, bool target_health = false, bool extended_version = false);

	/// <summary>Removes a proc from the list of current procs</summary>
	/// <param name='item'>Item the proc is from</param>
	/// <param name='spell'>Spell the proc is from</param>
	void RemoveProc(Item* item = 0, LuaSpell* spell = 0);

	/// <summary>Cycles through the proc list and executes them if they can go off</summary>
	/// <param name='type'>The proc type to check</param>
	/// <param name='target'>The target of the proc if it goes off</param>
	void CheckProcs(int8 type, Spawn* target);

	/// <summary>Clears the entire proc list</summary>
	void ClearProcs();

	float GetSpeed();
	float GetAirSpeed();
	float GetBaseSpeed() { return base_speed; }
	void SetSpeed(float val, bool override_ = false) { if ((base_speed == 0.0f && val > 0.0f) || override_) base_speed = val;  speed = val; }
	void SetSpeedMultiplier(float val) { speed_multiplier = val; }

	void SetThreatTransfer(ThreatTransfer* transfer);
	ThreatTransfer* GetThreatTransfer() { return m_threatTransfer; }
	int8 GetTraumaCount();
	int8 GetArcaneCount();
	int8 GetNoxiousCount();
	int8 GetElementalCount();
	int8 GetCurseCount();
	int8 GetDetTypeCount(int8 det_type);
	int8 GetDetCount();
	bool HasCurableDetrimentType(int8 det_type);
	Mutex* GetDetrimentMutex();
	Mutex* GetMaintainedMutex();
	Mutex* GetSpellEffectMutex();
	void ClearAllDetriments();
	void CureDetrimentByType(int8 cure_count, int8 det_type, string cure_name, Entity* caster, int8 cure_level = 0);
	void CureDetrimentByControlEffect(int8 cure_count, int8 det_type, string cure_name, Entity* caster, int8 cure_level = 0);
	vector<DetrimentalEffects>* GetDetrimentalSpellEffects();
	void RemoveEffectsFromLuaSpell(LuaSpell* spell);
	virtual void RemoveSkillBonus(int32 spell_id);

	virtual bool CanSeeInvis(Entity* target);
	void CancelAllStealth();
	bool IsStealthed();
	bool IsInvis();
	void AddInvisSpell(LuaSpell* spell);
	void AddStealthSpell(LuaSpell* spell);
	void RemoveStealthSpell(LuaSpell* spell);
	void RemoveInvisSpell(LuaSpell* spell);
	void AddWaterwalkSpell(LuaSpell* spell);
	void AddWaterjumpSpell(LuaSpell* spell);
	void RemoveWaterwalkSpell(LuaSpell* spell);
	void RemoveWaterjumpSpell(LuaSpell* spell);
	void AddAOEImmunity(LuaSpell* spell);
	bool IsAOEImmune();
	void RemoveAOEImmunity(LuaSpell* spell);
	void AddStunImmunity(LuaSpell* spell);
	void RemoveStunImmunity(LuaSpell* spell);
	bool IsStunImmune();
	void AddStifleImmunity(LuaSpell* spell);
	void RemoveStifleImmunity(LuaSpell* spell);
	bool IsStifleImmune();
	void AddMezImmunity(LuaSpell* spell);
	void RemoveMezImmunity(LuaSpell* spell);
	bool IsMezImmune();
	void AddRootImmunity(LuaSpell* spell);
	void RemoveRootImmunity(LuaSpell* spell);
	bool IsRootImmune();
	void AddFearImmunity(LuaSpell* spell);
	void RemoveFearImmunity(LuaSpell* spell);
	bool IsFearImmune();
	void AddDazeImmunity(LuaSpell* spell);
	void RemoveDazeImmunity(LuaSpell* spell);
	bool IsDazeImmune();
	void AddImmunity(LuaSpell* spell, int8 type);
	void RemoveImmunity(LuaSpell* spell, int8 type);
	bool IsImmune(int8 type);
	void AddFlightSpell(LuaSpell* spell);
	void RemoveFlightSpell(LuaSpell* spell);
	void AddSafefallSpell(LuaSpell* spell);
	void RemoveSafefallSpell(LuaSpell* spell);
	void AddGlideSpell(LuaSpell* spell);
	void RemoveGlideSpell(LuaSpell* spell);

	GroupMemberInfo* GetGroupMemberInfo() { return group_member_info; }
	void SetGroupMemberInfo(GroupMemberInfo* info) { group_member_info = info; }
	void UpdateGroupMemberInfo(bool inGroupMgrLock=false, bool groupMembersLocked=false);

	void CustomizeAppearance(PacketStruct* packet);

	Trade* trade;

	// Keep track of entities that hate this spawn.
	set<int32> HatedBy;
	std::mutex MHatedBy;

	bool IsAggroed() { 
			int32 size = 0;

			MHatedBy.lock();
			size = HatedBy.size();
			MHatedBy.unlock();

			return size > 0;
		}

	Mutex	MCommandMutex;

	bool HasSeeInvisSpell() { return hasSeeInvisSpell; }
	void SetSeeInvisSpell(bool val) { hasSeeInvisSpell = val; }

	bool HasSeeHideSpell() { return hasSeeHideSpell; }
	void SetSeeHideSpell(bool val) { hasSeeHideSpell = val; }

	void SetInfoStruct(InfoStruct* struct_) { info_struct.SetInfoStruct(struct_); }

	std::string GetInfoStructString(std::string field);
	int8 GetInfoStructInt8(std::string field);
	int16 GetInfoStructInt16(std::string field);
	int32 GetInfoStructInt32(std::string field);
	int64 GetInfoStructInt64(std::string field);
	sint8 GetInfoStructSInt8(std::string field);
	sint16 GetInfoStructSInt16(std::string field);
	sint32 GetInfoStructSInt32(std::string field);
	sint64 GetInfoStructSInt64(std::string field);
	int64 GetInfoStructUInt(std::string field);
	sint64 GetInfoStructSInt(std::string field);
	float GetInfoStructFloat(std::string field);


	bool SetInfoStructString(std::string field, std::string value);
	bool SetInfoStructUInt(std::string field, int64 value);
	bool SetInfoStructSInt(std::string field, sint64 value);
	bool SetInfoStructFloat(std::string field, float value);

	float CalculateSpellDamageReduction(float spellDamage, int16 competitorLevel);
	sint32 CalculateHateAmount(Spawn* target, sint32 amt);
	sint32 CalculateHealAmount(Spawn* target, sint32 amt, int8 crit_mod, bool* crit, bool skip_crit_mod = false);
	sint32 CalculateDamageAmount(Spawn* target, sint32 damage, int8 base_type, int8 damage_type, LuaSpell* spell);
	sint32 CalculateDamageAmount(Spawn* target, sint32 damage, int8 base_type, int8 damage_type, int8 spell_target_type);
	sint32 CalculateFormulaByStat(sint32 value, int16 stat);
	int32 CalculateFormulaByStat(int32 value, int16 stat);
	int32 CalculateFormulaBonus(int32 value, float percent_bonus);
	float CalculateSpellDamageReduction(float spellDamage, float resistancePercentage, int16 attackerLevel);
	
	float GetStat(int32 item_stat) {
		float item_chance_or_skill = 0.0f;
		MStats.lock();
		item_chance_or_skill = stats[item_stat];
		MStats.unlock();
		return item_chance_or_skill;
	}
	
	bool IsEngagedInEncounter(Spawn** res = nullptr);
	bool IsEngagedBySpawnID(int32 id);
	void SendControlEffectDetailsToClient(Client* client);
		
	std::string GetControlEffectName(int8 control_effect_type) {
		switch(control_effect_type) {
			case CONTROL_EFFECT_TYPE_MEZ: {
				return "Mesmerize";
				break;
			}
			case CONTROL_EFFECT_TYPE_STIFLE:{
				return "Stifle";
				break;
			}
			case CONTROL_EFFECT_TYPE_DAZE:{
				return "Daze";
				break;
			}
			case CONTROL_EFFECT_TYPE_STUN:{
				return "Stun";
				break;
			}
			case CONTROL_EFFECT_TYPE_ROOT:{
				return "Root";
				break;
			}
			case CONTROL_EFFECT_TYPE_FEAR:{
				return "Fear";
				break;
			}
			case CONTROL_EFFECT_TYPE_WALKUNDERWATER:{
				return "WalkUnderwater";
				break;
			}
			case CONTROL_EFFECT_TYPE_JUMPUNDERWATER:{
				return "JumpUnderwater";
				break;
			}
			case CONTROL_EFFECT_TYPE_INVIS:{
				return "Invisible";
				break;
			}
			case CONTROL_EFFECT_TYPE_STEALTH:{
				return "Stealth";
				break;
			}
			case CONTROL_EFFECT_TYPE_SNARE:{
				return "Snare";
				break;
			}
			case CONTROL_EFFECT_TYPE_FLIGHT:{
				return "Flight";
				break;
			}
			case CONTROL_EFFECT_TYPE_GLIDE:{
				return "Glide";
				break;
			}
			case CONTROL_EFFECT_TYPE_SAFEFALL:{
				return "SafeFall";
				break;
			}
			default: {
				return "Undefined";
				break;
			}
		}
	}
	
	void TerminateTrade();
	
	void CalculateMaxReduction();
	// when PacketStruct is fixed for C++17 this should become a shared_mutex and handle read/write lock
	std::mutex		MEquipment;
	std::mutex		MStats;

	Mutex   MMaintainedSpells;
	Mutex   MSpellEffects;
protected:
	bool	in_combat;
	int8	m_petType;
	int32	owner;
	// m_petSpellID holds the spell id used to create/control this pet
	int32	m_petSpellID;
	int8	m_petSpellTier;
	bool	m_petDismissing;
private:
	MutexList<BonusValues*> bonus_list;
	map<int8, MutexList<LuaSpell*>*> control_effects;
	map<int8, MutexList<LuaSpell*>*> immunities;
	float	max_speed;
	int8	deity;
	sint16	regen_hp_rate;
	sint16	regen_power_rate;
	float	last_x;
	float	last_y;
	float	last_z;
	float	last_heading;
	bool	casting;
	InfoStruct		info_struct;
	map<int8, int8> det_count_list;
	Mutex MDetriments;
	vector<DetrimentalEffects> detrimental_spell_effects;
	// Pointers for the 4 types of pets (Summon, Charm, Deity, Cosmetic)
	Entity*	pet;
	Entity* charmedPet;
	Entity* deityPet;
	Entity* cosmeticPet;

	// int32 = spell id, WardInfo* = pointer to ward info
	map<int32, WardInfo*> m_wardList;
	mutable std::shared_mutex MWardList;

	// int8 = type, vector<Proc*> = list of pointers to proc info
	map <int8, vector<Proc*> > m_procList;
	Mutex MProcList;

	/// <summary>Actually calls the lua script to cast the proc</summary>
	/// <param name='proc'>Proc to be cast</param>
	/// <param name='type'>Type of proc going off</type>
	/// <param name='target'>Target of the proc</param>
	bool CastProc(Proc* proc, int8 type, Spawn* target);

	float base_speed;
	float speed;
	float speed_multiplier;

	map<LuaSpell*, float> snare_values;

	ThreatTransfer* m_threatTransfer;

	GroupMemberInfo* group_member_info;

	bool hasSeeInvisSpell;
	bool hasSeeHideSpell;

	// GETs
	map<string, boost::function<float()> > get_float_funcs;
	map<string, boost::function<int64()> > get_int64_funcs;
	map<string, boost::function<int32()> > get_int32_funcs;
	map<string, boost::function<int16()> > get_int16_funcs;
	map<string, boost::function<int8()> > get_int8_funcs;

	map<string, boost::function<sint64()> > get_sint64_funcs;
	map<string, boost::function<sint32()> > get_sint32_funcs;
	map<string, boost::function<sint16()> > get_sint16_funcs;
	map<string, boost::function<sint8()> > get_sint8_funcs;
	
	map<string, boost::function<std::string()> > get_string_funcs;

	// SETs
	map<string, boost::function<void(float)> > set_float_funcs;
	map<string, boost::function<void(int64)> > set_int64_funcs;
	map<string, boost::function<void(int32)> > set_int32_funcs;
	map<string, boost::function<void(int16)> > set_int16_funcs;
	map<string, boost::function<void(int8)> > set_int8_funcs;

	map<string, boost::function<void(sint64)> > set_sint64_funcs;
	map<string, boost::function<void(sint32)> > set_sint32_funcs;
	map<string, boost::function<void(sint16)> > set_sint16_funcs;
	map<string, boost::function<void(sint8)> > set_sint8_funcs;
	
	map<string, boost::function<void(std::string)> > set_string_funcs;
};

#endif
