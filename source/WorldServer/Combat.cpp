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
#include "Combat.h"
#include "client.h"
#include "../common/ConfigReader.h"
#include "classes.h"
#include "../common/debug.h"
#include "../common/Log.h"
#include "zoneserver.h"
#include "Skills.h"
#include "classes.h"
#include "World.h"
#include "LuaInterface.h"
#include "Rules/Rules.h"
#include "SpellProcess.h"
#include "World.h"
#include <math.h>

extern Classes classes;
extern ConfigReader configReader;
extern MasterSkillList master_skill_list;
extern RuleManager rule_manager;
extern LuaInterface* lua_interface;
extern World world;

/* ******************************************************************************

DamageSpawn() - Damage equation
MeleeAttack() - Melee auto attacks
RangeAttack() - Range auto attacks
DetermineHit() - ToHit chance as well as defender parry / dodge / block / riposte
CheckInterruptSpell() - Interrupt equations


No mitigation equations yet

****************************************************************************** */

/*			New Combat code				*/

bool Entity::PrimaryWeaponReady() {
	//Can only be ready if no ranged timer
	if (GetPrimaryLastAttackTime() == 0 || (Timer::GetCurrentTime2() >= (GetPrimaryLastAttackTime() + GetPrimaryAttackDelay()))) {
		if (GetRangeLastAttackTime() == 0 || Timer::GetCurrentTime2() >= (GetRangeLastAttackTime() + GetRangeAttackDelay()))
			return true;
	}

	return false;
}

bool Entity::SecondaryWeaponReady() {
	//Can only be ready if no ranged timer
	// if(IsDualWield() && (GetPrimaryLastAttackTime()
	if (IsDualWield() && (GetSecondaryLastAttackTime() == 0 || (Timer::GetCurrentTime2() >= (GetSecondaryLastAttackTime() + GetSecondaryAttackDelay())))) {
		if(GetRangeLastAttackTime() == 0  || Timer::GetCurrentTime2() >= (GetRangeLastAttackTime() + GetRangeAttackDelay()))
			return true;
	}

	return false;
}

bool Entity::RangeWeaponReady() {
	//Ranged can only be ready if no other attack timers are active
	if(GetRangeLastAttackTime() == 0 || (Timer::GetCurrentTime2() >= (GetRangeLastAttackTime() + GetRangeAttackDelay()))) {
		if((GetPrimaryLastAttackTime() == 0 || (Timer::GetCurrentTime2() >= (GetPrimaryLastAttackTime() + GetPrimaryAttackDelay()))) && (GetSecondaryLastAttackTime() == 0  || Timer::GetCurrentTime2() >= (GetSecondaryLastAttackTime() + GetSecondaryAttackDelay()))){
			if(!IsPlayer() || ((Player*)this)->GetRangeAttack()) {
				return true;
			}
		}
	}

	return false;
}

bool Entity::AttackAllowed(Entity* target, float distance, bool range_attack) {
	Entity* attacker = this;
	Client* client = 0;
	if(!target || IsMezzedOrStunned() || IsDazed()) {
		LogWrite(COMBAT__DEBUG, 3, "AttackAllowed", "Failed to attack: no target, mezzed, stunned or dazed");
		return false;
	}
	if (IsPlayer())
		client = ((Player*)this)->GetClient();

	if (IsPet())
		attacker = ((NPC*)this)->GetOwner();
	if (target->IsNPC() && ((NPC*)target)->IsPet()){
		if (((NPC*)target)->GetOwner())
			target = ((NPC*)target)->GetOwner();
	}

	if((IsBot() || (client || (attacker && (attacker->IsPlayer() || attacker->IsBot())))) && !target->IsPlayer() && !target->GetAttackable()) {
		return false;
	}

	if (attacker == target) {
		LogWrite(COMBAT__DEBUG, 3, "AttackAllowed", "Failed to attack: attacker tried to attack himself or his pet.");
		return false;
	}

	if (IsPlayer() && target->GetAttackable() == 0) {
		LogWrite(COMBAT__DEBUG, 3, "AttackAllowed", "Failed to attack: target is not attackable");
		return false;
	}

	if (IsPlayer() && target->IsBot()) {
		LogWrite(COMBAT__DEBUG, 3, "AttackAllowed", "Failed to attack: players are not allowed to attack bots");
		return false;
	}
	
	if(rule_manager.GetGlobalRule(R_Combat, LockedEncounterNoAttack)->GetBool()) {
		if(target->IsNPC() && (target->GetLockedNoLoot() == ENCOUNTER_STATE_LOCKED || target->GetLockedNoLoot() == ENCOUNTER_STATE_OVERMATCHED) && 
			!attacker->IsEngagedBySpawnID(target->GetID())) {
			return false;
		}
		else if(IsNPC() && (GetLockedNoLoot() == ENCOUNTER_STATE_LOCKED || GetLockedNoLoot() == ENCOUNTER_STATE_OVERMATCHED) && 
			!target->IsEngagedBySpawnID(GetID())) {
			return false;
		}
	}

	if (attacker->IsPlayer() && target->IsPlayer())
	{
		bool pvp_allowed = rule_manager.GetGlobalRule(R_PVP, AllowPVP)->GetBool();
		if (!pvp_allowed) {
			LogWrite(COMBAT__DEBUG, 3, "AttackAllowed", "Failed to attack: pvp is not allowed");
			return false;
		}
		else
		{
			sint32 pvpLevelRange = rule_manager.GetGlobalRule(R_PVP, LevelRange)->GetSInt32();
			int32 attackerLevel = attacker->GetLevel();
			int32 defenderLevel = target->GetLevel();
			if ((sint32)abs((sint32)attackerLevel - (sint32)defenderLevel) > pvpLevelRange)
			{
				LogWrite(COMBAT__DEBUG, 3, "AttackAllowed", "Failed to attack: pvp range of %i exceeded abs(%i-%i).", pvpLevelRange, attackerLevel, defenderLevel);
				return false;
			}
		}
	}

	if (target->GetHP() <= 0) {
		LogWrite(COMBAT__DEBUG, 3, "AttackAllowed", "Failed to attack: target is dead");
		return false;
	}

	if(range_attack && distance != 0) {
		Item* weapon = 0;
		Item* ammo = 0;
		if(attacker->IsPlayer()) {
				weapon = ((Player*)attacker)->GetEquipmentList()->GetItem(EQ2_RANGE_SLOT);
				ammo = GetAmmoFromSlot(true, true);
		}
		if(weapon && weapon->IsRanged() && ammo && ammo->IsAmmo() && ammo->IsThrown()) {
			// Distance is less then min weapon range
			if(distance < weapon->ranged_info->range_low) {
				if (client)
					client->SimpleMessage(CHANNEL_GENERAL_COMBAT, "Your target is too close! Move back!");
				LogWrite(COMBAT__DEBUG, 3, "AttackAllowed", "Failed to attack: range attack, target to close");
				return false;
			}
			// Distance is greater then max weapon range
			if  (distance > (weapon->ranged_info->range_high + ammo->thrown_info->range)) {
				if (client)
					client->SimpleMessage(CHANNEL_GENERAL_COMBAT, "Your target is too far away! Move closer!");
				LogWrite(COMBAT__DEBUG, 3, "AttackAllowed", "Failed to attack: range attack, target is to far");
				return false;
			}
		}
	}
	else if (distance != 0) {
		if(distance >= rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat()) {
			LogWrite(COMBAT__DEBUG, 3, "AttackAllowed", "Failed to attack: distance is beyond melee range");
			return false;
		}
	}
	
	if((target->IsPrivateSpawn() && !target->AllowedAccess(this)) || (IsPrivateSpawn() && AllowedAccess(target))) {
			LogWrite(COMBAT__DEBUG, 3, "AttackAllowed", "Access to spawn disallowed.");
		return false;
	}
	
	return true;
}

Item* Entity::GetAmmoFromSlot(bool is_ammo, bool is_thrown) {
	Item* ammo = GetEquipmentList()->GetItem(EQ2_AMMO_SLOT);
	if(ammo && ammo->IsBag() && IsPlayer()) {
		Item* ammo_bag = ammo;
		vector<Item*>* items = ((Player*)this)->GetPlayerItemList()->GetItemsInBag(ammo_bag);
		
		vector<Item*>::iterator itr;
		int16 i = 0;
		Item* tmp_bag_item = 0;
		for (itr = items->begin(); itr != items->end(); itr++) {
			tmp_bag_item = *itr;
			if (tmp_bag_item) {
				if(is_ammo && !tmp_bag_item->IsAmmo())
					continue;
				if(is_thrown && !tmp_bag_item->IsThrown())
					continue;
				ammo = tmp_bag_item;
				break;
			}
		}
	}
	
	if(ammo && is_ammo && !ammo->IsAmmo())
		ammo = nullptr;
	if(ammo && is_thrown && !ammo->IsThrown())
		ammo = nullptr;
	
	return ammo;
}

void Entity::MeleeAttack(Spawn* victim, float distance, bool primary, bool multi_attack) {
	if(!victim)
		return;

	int8 damage_type = 0;
	int32 min_damage = 0;
	int32 max_damage = 0;
	if(primary) {
		damage_type = GetPrimaryWeaponType();
		min_damage = GetPrimaryWeaponMinDamage();
		max_damage = GetPrimaryWeaponMaxDamage();
	}
	else {
		damage_type = GetSecondaryWeaponType();
		min_damage = GetSecondaryWeaponMinDamage();
		max_damage = GetSecondaryWeaponMaxDamage();
	}
	if (IsStealthed() || IsInvis())
		CancelAllStealth();
	
	int8 hit_result = DetermineHit(victim, DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE, damage_type, 0, false);
	
	if(victim->IsEntity()) {
		CheckEncounterState((Entity*)victim);
	}
	
	if(hit_result == DAMAGE_PACKET_RESULT_SUCCESSFUL){		
		/*if(GetAdventureClass() == MONK){
			max_damage*=3;
			crit_chance = GetLevel()/4+5;
		}
		else if(GetAdventureClass() == BRUISER){
			min_damage = GetLevel();
			max_damage*=3;
			crit_chance = GetLevel()/3+5;
		}
		if(rand()%100 <=crit_chance){
			max_damage*= 2;
			DamageSpawn((Entity*)victim, DAMAGE_PACKET_TYPE_SIMPLE_CRIT_DMG, damage_type, min_damage, max_damage, 0);
		}
		else*/
			DamageSpawn((Entity*)victim, DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE, damage_type, min_damage, max_damage, 0);
			if (!multi_attack) {
				CheckProcs(PROC_TYPE_OFFENSIVE, victim);
				CheckProcs(PROC_TYPE_PHYSICAL_OFFENSIVE, victim);
			}
	}
	else{

		GetZone()->SendDamagePacket(this, victim, DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE, hit_result, damage_type, 0, 0);
		if(hit_result == DAMAGE_PACKET_RESULT_RIPOSTE && victim->IsEntity())
			((Entity*)victim)->MeleeAttack(this, distance, true);
	}

	//Multi Attack roll
	if(!multi_attack){
		float multi_attack = info_struct.get_multi_attack();
		if(multi_attack > 0){
			float chance = multi_attack;
			if (multi_attack > 100){
				int8 automatic_multi = (int8)floor((float)(multi_attack / 100));
				chance = (multi_attack - (floor((float) ((multi_attack / 100) * 100))));
				while(automatic_multi > 0){
					MeleeAttack(victim, 100, primary, true);
					automatic_multi--;
				}
			}
			if (MakeRandomFloat(0, 100) <= chance)
				MeleeAttack(victim, 100, primary, true);
		}
	}

	//Apply attack speed mods
	if(!multi_attack)
		SetAttackDelay(primary);
	
	if(victim->IsNPC() && victim->EngagedInCombat() == false) {
		((NPC*)victim)->AddHate(this, 50);
	}

	if (victim->IsEntity() && victim->GetHP() > 0 && ((Entity*)victim)->HasPet()) {
		Entity* pet = 0;
		bool AddHate = false;
		if (victim->IsPlayer()) {
			if (((Player*)victim)->GetInfoStruct()->get_pet_behavior() & 1)
				AddHate = true;
		}
		else
			AddHate = true;
		
		if (AddHate) {
			pet = ((Entity*)victim)->GetPet();
			if (pet)
				pet->AddHate(this, 1);
			pet = ((Entity*)victim)->GetCharmedPet();
			if (pet)
				pet->AddHate(this, 1);
		}
	}
}

void Entity::RangeAttack(Spawn* victim, float distance, Item* weapon, Item* ammo, bool multi_attack) {
	if(!victim)
		return;

	if(weapon && weapon->IsRanged() && ammo && ammo->IsAmmo() && ammo->IsThrown()) {
		if(weapon->ranged_info->range_low <= distance && (weapon->ranged_info->range_high + ammo->thrown_info->range) >= distance) {
			int8 hit_result = DetermineHit(victim, DAMAGE_PACKET_TYPE_RANGE_DAMAGE, ammo->thrown_info->damage_type, ammo->thrown_info->hit_bonus, false);
			if(hit_result == DAMAGE_PACKET_RESULT_SUCCESSFUL) {
				DamageSpawn((Entity*)victim, DAMAGE_PACKET_TYPE_RANGE_DAMAGE, ammo->thrown_info->damage_type, weapon->ranged_info->weapon_info.damage_low3, weapon->ranged_info->weapon_info.damage_high3+ammo->thrown_info->damage_modifier, 0);
				if (!multi_attack) {
					CheckProcs(PROC_TYPE_OFFENSIVE, victim);
					CheckProcs(PROC_TYPE_PHYSICAL_OFFENSIVE, victim);
				}
			}
			else
				GetZone()->SendDamagePacket(this, victim, DAMAGE_PACKET_TYPE_RANGE_DAMAGE, hit_result, ammo->thrown_info->damage_type, 0, 0);

			// If is a player subtract ammo
			if (IsPlayer()) {
				if (ammo->details.count > 1) {
					ammo->details.count -= 1;
					ammo->save_needed = true;
				}
				else {
					if(ammo->details.inv_slot_id >= 6) {
						((Player*)this)->equipment_list.RemoveItem(ammo->details.slot_id, false);
						((Player*)this)->item_list.DestroyItem(ammo->details.index);	
					}
					else {
						((Player*)this)->equipment_list.RemoveItem(ammo->details.slot_id, true);
					}
				}
				
				Client* client = ((Player*)this)->GetClient();
				if(client) {
					EQ2Packet* outapp = ((Player*)this)->GetEquipmentList()->serialize(client->GetVersion(), (Player*)this);
					if(outapp)
						client->QueuePacket(outapp);
					
					if(ammo->details.inv_slot_id > 6) {
						EQ2Packet* outapp = client->GetPlayer()->SendInventoryUpdate(client->GetVersion());
						if (outapp)
							client->QueuePacket(outapp);
					}
				}
			}

			if(victim->IsNPC() && victim->EngagedInCombat() == false) {
				((NPC*)victim)->AddHate(this, 50);
			}

			if (victim->IsEntity() && victim->GetHP() > 0 && ((Entity*)victim)->HasPet()) {
				Entity* pet = 0;
				bool AddHate = false;
				if (victim->IsPlayer()) {
					if (((Player*)victim)->GetInfoStruct()->get_pet_behavior() & 1)
						AddHate = true;
				}
				else
					AddHate = true;

				if (AddHate) {
					pet = ((Entity*)victim)->GetPet();
					if (pet)
						pet->AddHate(this, 1);
					pet = ((Entity*)victim)->GetCharmedPet();
					if (pet)
						pet->AddHate(this, 1);
				}
			}
			// Check Ranged attack proc
			CheckProcs(PROC_TYPE_RANGED_ATTACK, victim);

			// Check Ranged defence proc
			if (victim->IsEntity())
				((Entity*)victim)->CheckProcs(PROC_TYPE_RANGED_DEFENSE, this);

			SetRangeLastAttackTime(Timer::GetCurrentTime2());
		}
	}
	//Multi Attack roll
	if(!multi_attack){
		float multi_attack = info_struct.get_multi_attack();
		if(multi_attack > 0){
			float chance = multi_attack;
			if (multi_attack > 100){
				int8 automatic_multi = (int8)floor((float)(multi_attack / 100));
				chance = (multi_attack - (floor((float)(multi_attack / 100) * 100)));
				while(automatic_multi > 0){
					RangeAttack(victim, 100, weapon, ammo, true);
					automatic_multi--;
				}
			}
			if (MakeRandomFloat(0, 100) <= chance)
				RangeAttack(victim, 100, weapon, ammo, true);
		}
	}

	//Apply attack speed mods
	if(!multi_attack)
		SetAttackDelay(false, true);
}

bool Entity::SpellAttack(Spawn* victim, float distance, LuaSpell* luaspell, int8 damage_type, int32 low_damage, int32 high_damage, int8 crit_mod, bool no_calcs, int8 override_packet_type, bool take_power){
	if(!victim || !luaspell || !luaspell->spell)
		return false;

	Spell* spell = luaspell->spell;
	Skill* skill = nullptr;
	int8 packet_type = DAMAGE_PACKET_TYPE_SPELL_DAMAGE;
	
	if(override_packet_type) {
		packet_type = override_packet_type;
	}
	
	int8 hit_result = 0;
	bool is_tick = false; // if spell is already active, this is a tick
	if (GetZone()->GetSpellProcess()->GetActiveSpells()->count(luaspell)){
		hit_result = DAMAGE_PACKET_RESULT_SUCCESSFUL;
		is_tick = true;
	}
	else if(spell->GetSpellData()->type == SPELL_BOOK_TYPE_COMBAT_ART)
		hit_result = DetermineHit(victim, packet_type, damage_type, 0, false, luaspell);
	else
		hit_result = DetermineHit(victim, packet_type, damage_type, 0, true, luaspell);
	
	if(victim->IsEntity()) {
		CheckEncounterState((Entity*)victim);
	}
	bool successful_hit = true;
	if(hit_result == DAMAGE_PACKET_RESULT_SUCCESSFUL) {
		luaspell->last_spellattack_hit = true;
		//If this spell is a tick and has already crit, force the tick to crit
		if(is_tick){
			if(luaspell->crit)
				crit_mod = 1;
			else
				crit_mod = 2;
		}
		DamageSpawn((Entity*)victim, packet_type, damage_type, low_damage, high_damage, spell->GetName(), crit_mod, is_tick, no_calcs, false, take_power, luaspell);
		
		CheckProcs(PROC_TYPE_OFFENSIVE, victim);
		CheckProcs(PROC_TYPE_MAGICAL_OFFENSIVE, victim);

		if(spell->GetSpellData()->success_message.length() > 0){
			Client* client = nullptr;
			if(IsPlayer())
				client = ((Player*)this)->GetClient();
			if(client){
				string success_message = spell->GetSpellData()->success_message;
				if(success_message.find("%t") < 0xFFFFFFFF)
					success_message.replace(success_message.find("%t"), 2, victim->GetName());
				client->Message(CHANNEL_YOU_CAST, success_message.c_str());
				//commented out the following line as it was causing a duplicate message EmemJR 5/4/2019
				//GetZone()->SendDamagePacket(this, victim, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, hit_result, damage_type, 0, spell->GetName()); 
			}
		}
		if(spell->GetSpellData()->effect_message.length() > 0){
			string effect_message = spell->GetSpellData()->effect_message;
			if(effect_message.find("%t") < 0xFFFFFFFF)
				effect_message.replace(effect_message.find("%t"), 2, victim->GetName());
			GetZone()->SimpleMessage(CHANNEL_SPELLS, effect_message.c_str(), victim, 50);
		}
	}
	else {
		successful_hit = false;
		if(hit_result == DAMAGE_PACKET_RESULT_RESIST)
			luaspell->resisted = true;
		if(victim->IsNPC())
			((NPC*)victim)->AddHate(this, 5);
		luaspell->last_spellattack_hit = false;
		GetZone()->SendDamagePacket(this, victim, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, hit_result, damage_type, 0, spell->GetName());
	}
	if(EngagedInCombat() == false)
	{
		LogWrite(MISC__TODO, 1, "TODO", "//It would probably be better to add a column to the spells table for 'starts autoattack'\nfile: %s, func: %s, Line: %i", __FILE__, __FUNCTION__, __LINE__);
		int8 class1_ = GetInfoStruct()->get_class1();
		if(class1_ == COMMONER ||
			class1_ == FIGHTER ||
			class1_ == WARRIOR ||
			class1_ == GUARDIAN ||
			class1_ == BERSERKER ||
			class1_ == BRAWLER ||
			class1_ == MONK ||
			class1_ == BRUISER ||
			class1_ == CRUSADER ||
			class1_ == SHADOWKNIGHT ||
			class1_ == PALADIN ||
			class1_ == SCOUT ||
			class1_ == ROGUE ||
			class1_ == SWASHBUCKLER ||
			class1_ == BRIGAND ||
			class1_ == BARD ||
			class1_ == TROUBADOR ||
			class1_ == DIRGE ||
			class1_ == PREDATOR ||
			class1_ == RANGER ||
			class1_ == ASSASSIN ||
			class1_ == ANIMALIST ||
			class1_ == BEASTLORD ||
			class1_ == SHAPER || 
			class1_ == CHANNELER) //note: it would probably be better to add a column to the spells table for "starts autoattack".
		{
			if (victim->IsNPC())
				((NPC*)victim)->AddHate(this, 5);
			else {
				Client* client = 0;
				if(IsPlayer())
					client = ((Player*)this)->GetClient();
				if(client) {
					client->GetPlayer()->InCombat(true, client->GetPlayer()->GetRangeAttack());
				}
				InCombat(true);
			}
		}
	}

	if (victim->IsEntity() && victim->GetHP() > 0 && ((Entity*)victim)->HasPet()) {
		Entity* pet = 0;
		bool AddHate = false;
		if (victim->IsPlayer()) {
			if (((Player*)victim)->GetInfoStruct()->get_pet_behavior() & 1)
				AddHate = true;
		}
		else
			AddHate = true;

		if (AddHate) {
			pet = ((Entity*)victim)->GetPet();
			if (pet)
				pet->AddHate(this, 1);
			pet = ((Entity*)victim)->GetCharmedPet();
			if (pet)
				pet->AddHate(this, 1);
		}
	}

	return successful_hit;
}

bool Entity::ProcAttack(Spawn* victim, int8 damage_type, int32 low_damage, int32 high_damage, string name, string success_msg, string effect_msg) {
	int8 hit_result = DetermineHit(victim, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, damage_type, 0, true);

	if (hit_result == DAMAGE_PACKET_RESULT_SUCCESSFUL) {
		DamageSpawn((Entity*)victim, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, damage_type, low_damage, high_damage, name.c_str());

		if (success_msg.length() > 0) {
			Client* client = 0;
			if(IsPlayer())
				client = ((Player*)this)->GetClient();
			if(client) {
				if(success_msg.find("%t") < 0xFFFFFFFF)
					success_msg.replace(success_msg.find("%t"), 2, victim->GetName());
				client->Message(CHANNEL_YOU_CAST, success_msg.c_str());
			}
		}
		if (effect_msg.length() > 0) {
			if(effect_msg.find("%t") < 0xFFFFFFFF)
				effect_msg.replace(effect_msg.find("%t"), 2, victim->GetName());
			GetZone()->SimpleMessage(CHANNEL_SPELLS, effect_msg.c_str(), victim, 50);
		}
	}
	else {
		if(victim->IsNPC())
			((NPC*)victim)->AddHate(this, 5);
		GetZone()->SendDamagePacket(this, victim, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, hit_result, damage_type, 0, name.c_str());
	}
	

	if (victim->IsEntity() && victim->GetHP() > 0 && ((Entity*)victim)->HasPet()) {
		Entity* pet = 0;
		bool AddHate = false;
		if (victim->IsPlayer()) {
			if (((Player*)victim)->GetInfoStruct()->get_pet_behavior() & 1)
				AddHate = true;
		}
		else
			AddHate = true;

		if (AddHate) {
			pet = ((Entity*)victim)->GetPet();
			if (pet)
				pet->AddHate(this, 1);
			pet = ((Entity*)victim)->GetCharmedPet();
			if (pet)
				pet->AddHate(this, 1);
		}
	}

	return true;
}

// this is used exclusively by LUA, heal_type is forced lower case via boost::lower(heal_type); in the LUA Functions used by this
bool Entity::SpellHeal(Spawn* target, float distance, LuaSpell* luaspell, string heal_type, int32 low_heal, int32 high_heal, int8 crit_mod, bool no_calcs, string custom_spell_name){
	 if(!target || !luaspell || !luaspell->spell || (target->IsPrivateSpawn() && !target->AllowedAccess(this)))
		return false;

	 if (!target->Alive())
		 return false;

	 if (target->GetHP() == target->GetTotalHP())
		 return true;

	 int32 heal_amt = 0;
	 bool crit = false;

	 if(high_heal < low_heal)
		 high_heal = low_heal;
	 if(high_heal == low_heal)
		 heal_amt = high_heal;
	 else
		 heal_amt = MakeRandomInt(low_heal, high_heal);

	 if(!no_calcs){
		// if spell is already active, this is a tick
		bool is_tick = GetZone()->GetSpellProcess()->GetActiveSpells()->count(luaspell);

		//if is a tick and the spell has crit, force crit, else disable
		if(is_tick){
			if(luaspell->crit)
				crit_mod = 1;
			else
				crit_mod = 2;
		}
		
		if (heal_amt > 0){
			if(target->IsEntity())
				heal_amt = (int32)CalculateHealAmount((Entity*)target, (sint32)heal_amt, crit_mod, &crit);
			else
				heal_amt = (int32)CalculateHealAmount(nullptr, (sint32)heal_amt, crit_mod, &crit);
		}
	}

	int16 type = 0;
	if (heal_type == "heal") {
		if(crit)
			type = HEAL_PACKET_TYPE_CRIT_HEAL;
		else
			type = HEAL_PACKET_TYPE_SIMPLE_HEAL;
		//apply heal
		
		if (target->GetHP() + (sint32)heal_amt > target->GetTotalHP())
			heal_amt = target->GetTotalHP() - target->GetHP();
		target->SetHP(target->GetHP() + heal_amt);

		/*
		if (target->GetHP() + (sint32)heal_amt > target->GetTotalHP())
			target->SetHP(target->GetTotalHP());
		else
			target->SetHP(target->GetHP() + heal_amt);
		*/
	}
	else if (heal_type == "power"){
		if(crit)
			type = HEAL_PACKET_TYPE_CRIT_MANA;
		else
			type = HEAL_PACKET_TYPE_SIMPLE_MANA;
		//give power
		if (target->GetPower() + (sint32)heal_amt > target->GetTotalPower())
			heal_amt = target->GetTotalPower() - target->GetPower();
		target->SetPower(GetPower() + heal_amt);

		/*
		if (target->GetPower() + (sint32)heal_amt > target->GetTotalPower())
			target->SetPower(target->GetTotalPower());
		else
			target->SetPower(GetPower() + heal_amt);
		*/
	}
	/*else if (heal_type == "Savagery"){
		if(crit)
			type = HEAL_PACKET_TYPE_CRIT_SAVAGERY;
		else
			type = HEAL_PACKET_TYPE_SAVAGERY;
	}
	else if (heal_type == "Repair"){
		if(crit)
			type = HEAL_PACKET_TYPE_CRIT_REPAIR;
		else
			type = HEAL_PACKET_TYPE_REPAIR;
	}*/
	else{ //default to heal if type cannot be determined
		if(crit)
			type = HEAL_PACKET_TYPE_CRIT_HEAL;
		else
			type = HEAL_PACKET_TYPE_SIMPLE_HEAL;

		if (target->GetHP() + (sint32)heal_amt > target->GetTotalHP())
			heal_amt = target->GetTotalHP() - target->GetHP();
		target->SetHP(target->GetHP() + heal_amt);
		/*
		if (target->GetHP() + (sint32)heal_amt > target->GetTotalHP())
			target->SetHP(target->GetTotalHP());
		else
			target->SetHP(target->GetHP() + heal_amt);
		*/
	}

	target->GetZone()->TriggerCharSheetTimer();
	if (heal_amt > 0)
		GetZone()->SendHealPacket(this, target, type, heal_amt, custom_spell_name.length() > 0 ? (char*)custom_spell_name.c_str() : luaspell->spell->GetName());
	CheckProcs(PROC_TYPE_HEALING, target);
	CheckProcs(PROC_TYPE_BENEFICIAL, target);

	if (target->IsEntity()) {
		int32 hate_amt = heal_amt / 2;
		set<int32>::iterator itr;
		((Entity*)target)->MHatedBy.lock();
		set<int32> hatedByCopy(((Entity*)target)->HatedBy);
		((Entity*)target)->MHatedBy.unlock();
		for (itr = hatedByCopy.begin(); itr != hatedByCopy.end(); itr++) {
			Spawn* spawn = GetZone()->GetSpawnByID(*itr);
			if (spawn && spawn->IsEntity() && target != this) {
				CheckEncounterState((Entity*)spawn);
				((Entity*)spawn)->AddHate(this, hate_amt);
			}
		}
	}

	return true;
}

int8 Entity::DetermineHit(Spawn* victim, int8 type, int8 damage_type, float ToHitBonus, bool is_caster_spell, LuaSpell* lua_spell){
	if(lua_spell) {
		lua_spell->is_damage_spell = true;
	}
	if(!victim) {
		return DAMAGE_PACKET_RESULT_MISS;
	}

	if(victim->GetInvulnerable()) {
		return DAMAGE_PACKET_RESULT_INVULNERABLE;
	}

	bool behind = false;
	
	// move this above the if statement to assure skill-ups on successful damage return
	Skill* skill = GetSkillByWeaponType(type, damage_type, true);
	
	// Monk added with Brawler to 360 degree support per KoS Prima Official eGuide Fighter: Monk, pg 138, denoted '360-Degree Avoidance!'
	if(!victim->IsEntity() || (!is_caster_spell && victim->GetAdventureClass() != BRAWLER && victim->GetAdventureClass() != MONK && (behind = BehindTarget(victim)))) {
		return DAMAGE_PACKET_RESULT_SUCCESSFUL;
	}

	float bonus = ToHitBonus;

	float skillAddedByWeapon = 0.0f;
	if(skill)
	{
		int16 skillID = master_item_list.GetItemStatIDByName(skill->name.data);
		if(skillID != 0xFFFFFFFF)
		{
			MStats.lock();
			skillAddedByWeapon = stats[skillID];
			if(!is_caster_spell) {
				float item_stat_weapon_skill = stats[ITEM_STAT_WEAPON_SKILLS];
				skillAddedByWeapon += item_stat_weapon_skill;
			}
			MStats.unlock();
			
			float max_bonus_skill = GetRuleSkillMaxBonus();
			if(skillAddedByWeapon > max_bonus_skill) {
				skillAddedByWeapon = max_bonus_skill;
			}
		}
	}
	
	if (skill)
		bonus += (skill->current_val+skillAddedByWeapon) / 25;
		
	if(is_caster_spell && lua_spell) {
		if(lua_spell->spell->GetSpellData()->resistibility > 0)
			bonus -= (1.0f - lua_spell->spell->GetSpellData()->resistibility)*100.0f;
	
		LogWrite(COMBAT__DEBUG, 9, "Combat", "SpellResist: resistibility %f, bonus %f", lua_spell->spell->GetSpellData()->resistibility, bonus);
		// Here we take into account Subjugation, Disruption and Ordination (debuffs)
		if(lua_spell->spell->GetSpellData()->mastery_skill) {
			int32 master_skill_reduce = rule_manager.GetGlobalRule(R_Spells, MasterSkillReduceSpellResist)->GetInt32();
			if(master_skill_reduce < 1)
				master_skill_reduce = 25;
			if(IsPlayer() && lua_spell->spell->GetSpellData()->spell_book_type == SPELL_BOOK_TYPE_TRADESKILL && 
			   !((Player*)this)->GetSkills()->HasSkill(lua_spell->spell->GetSpellData()->mastery_skill)) {
				((Player*)this)->AddSkill(lua_spell->spell->GetSpellData()->mastery_skill, 1, ((Player*)this)->GetLevel() * 5, true);
			}
			
			Skill* master_skill = GetSkillByID(lua_spell->spell->GetSpellData()->mastery_skill, true);
			if(master_skill && (lua_spell->spell->GetSpellData()->spell_book_type == SPELL_BOOK_TYPE_TRADESKILL || 
			  ((master_skill->name.data == "Subjugation" || master_skill->name.data == "Disruption" || master_skill->name.data == "Ordination" || master_skill->name.data == "Aggression")))) {
				float item_stat_bonus = 0.0f;
				int32 item_stat = master_item_list.GetItemStatIDByName(::ToLower(master_skill->name.data));
				if(item_stat != 0xFFFFFFFF) {
					item_stat_bonus = GetStat(item_stat);
				}
				bonus += (master_skill->current_val + item_stat_bonus) / master_skill_reduce;
				LogWrite(COMBAT__DEBUG, 9, "Combat", "SpellResistMasterySkill: skill %u, stat bonus %f, mastery skill reduce %u, bonus %f", master_skill->current_val, item_stat_bonus, master_skill_reduce, bonus);

			}
		}
		
	}
	
	if (victim->IsEntity())
		bonus -= ((Entity*)victim)->GetDamageTypeResistPercentage(damage_type);
	
	LogWrite(COMBAT__DEBUG, 9, "Combat", "DamageResistPercent: type %u, bonus %f", damage_type, bonus);


	Entity* entity_victim = (Entity*)victim;
	float chance = 80 + bonus; //80% base chance that the victim will get hit (plus bonus)
	sint16 roll_chance = 100;
	if(is_caster_spell) {
		chance += CalculateLevelStatBonus(GetWis());
	}
	if(skill)
		roll_chance -= skill->current_val / 10;

	LogWrite(COMBAT__DEBUG, 9, "Combat", "Chance: fchance %f, roll_chance %i", chance, roll_chance);

	if(!is_caster_spell){ // melee or range attack		
		skill = GetSkillByName("Offense", true); //add this skill for NPCs
		if(skill)
			roll_chance -= skill->current_val / 25;

		if(entity_victim->IsImmune(IMMUNITY_TYPE_RIPOSTE))
		return DAMAGE_PACKET_RESULT_RIPOSTE;

		// Avoidance Instructions: https://forums.daybreakgames.com/eq2/index.php?threads/avoidance-faq.482979/

		/*Parry: reads as parry in the avoidance tooltip, increased by items with +parry on them
		  Caps at 70%. For plate tanks, works in the front quadrant only, for brawlers this is 360 degrees.
		  A small % of parries will be ripostes, which not only avoid the attack but also damage your attacker
		*/

		skill = entity_victim->GetSkillByName("Parry", true);
		if(skill){
			float parryChance = entity_victim->GetInfoStruct()->get_parry();
			float chanceValue = (100.0f - parryChance);

			if(chanceValue < 10.0f) { // min default per https://eq2.fandom.com/wiki/Update:29
				chanceValue = 10.0f; // this becomes 5.0f at EoF
			}
			
			if(rand()%roll_chance >= chanceValue){ //successful parry
				/* You may only riposte things in the front quadrant.
				Riposte is based off of parry: a certain % of parries turn into ripostes.
				*/
				if(!behind && victim->InFrontSpawn((Spawn*)this, victim->GetX(), victim->GetZ())) { // if the attacker is not behind the victim, and the victim is facing the attacker (in front of spawn) then we can riposte
					float riposteChanceValue = parryChance / 7.0f; //  Riposte is based off of parry: a certain % of parries turn into ripostes. Unknown what the value is divided by, 7 to make it 10% even.
						if(rand()%100 <= riposteChanceValue) {
							entity_victim->CheckProcs(PROC_TYPE_RIPOSTE, this);
							return DAMAGE_PACKET_RESULT_RIPOSTE;
						}
				}
				entity_victim->CheckProcs(PROC_TYPE_PARRY, this);
				return DAMAGE_PACKET_RESULT_PARRY;
			}
		}

		skill = nullptr;

		
		float blockChance = 0.0f;
		if(victim->GetAdventureClass() == BRAWLER)
			skill = entity_victim->GetSkillByName("Deflection", true);

		blockChance = entity_victim->GetInfoStruct()->get_block();
		
		if(blockChance > 0.0f)
		{
			blockChance += (blockChance*(entity_victim->GetInfoStruct()->get_block_chance()/100.0f));
			float chanceValue = (100.0f - blockChance);
			\
			if(chanceValue < 30.0f) { // min default per https://eq2.fandom.com/wiki/Update:29
				chanceValue = 30.0f; // this becomes 25.0f at EoF
			}
			/* Non-brawlers may only block things in the front quadrant.
			Riposte is based off of parry: a certain % of parries turn into ripostes.
			*/
			float rnd = rand()%roll_chance;	
			if(rnd >= chanceValue){ //successful block
				if((victim->GetAdventureClass() == BRAWLER || victim->GetAdventureClass() == MONK) || (!behind && victim->InFrontSpawn((Spawn*)this, victim->GetX(), victim->GetZ()))) { // if the attacker is not behind the victim, and the victim is facing the attacker (in front of spawn) then we can block				
					entity_victim->CheckProcs(PROC_TYPE_BLOCK, this);
					return (victim->GetAdventureClass() == BRAWLER) ? DAMAGE_PACKET_RESULT_DEFLECT : DAMAGE_PACKET_RESULT_BLOCK;
				}
			}
		}

		skill = entity_victim->GetSkillByName("Defense", true);

		// calculated in Entity::CalculateBonuses
		float dodgeChance = entity_victim->GetInfoStruct()->get_avoidance_base();
		if(dodgeChance > 0.0f)
		{
			float chanceValue = (100.0f - dodgeChance);
			float rnd = rand()%roll_chance;	
			if(rnd >= chanceValue){ //successful dodge
				entity_victim->CheckProcs(PROC_TYPE_EVADE, this);
				return DAMAGE_PACKET_RESULT_DODGE;//successfully dodged
			}
		}
		if(rand() % roll_chance >= chance)
			return DAMAGE_PACKET_RESULT_MISS; //successfully avoided
	}
	else{
		float focus_skill_with_bonus = entity_victim->CalculateSkillWithBonus("Spell Avoidance", ITEM_STAT_SPELL_AVOIDANCE, true);
		int16 effective_level = entity_victim->GetInfoStruct()->get_effective_level() != 0 ? entity_victim->GetInfoStruct()->get_effective_level() : entity_victim->GetLevel();
		focus_skill_with_bonus += entity_victim->CalculateLevelStatBonus(entity_victim->GetWis());
		chance -= ((focus_skill_with_bonus)/10);
		LogWrite(COMBAT__DEBUG, 9, "Combat", "SpellAvoidChance: fchance %f, focus with skill bonus %f", chance, focus_skill_with_bonus);
		if(rand()%roll_chance >= chance) {
			return DAMAGE_PACKET_RESULT_RESIST; //successfully resisted	
		}
	}

	return DAMAGE_PACKET_RESULT_SUCCESSFUL;
}

float Entity::GetDamageTypeResistPercentage(int8 damage_type) {
	float ret = 1;

	switch(damage_type) {
	case DAMAGE_PACKET_DAMAGE_TYPE_CRUSH:
	case DAMAGE_PACKET_DAMAGE_TYPE_PIERCE:
	case DAMAGE_PACKET_DAMAGE_TYPE_SLASH: {
		Skill* skill = GetSkillByName("Defense", true);
		if(skill)
			ret += skill->current_val / 25;
		if(IsNPC())
			LogWrite(COMBAT__DEBUG, 3, "Combat", "DamageType: Crush/Pierce/Slash (%i)", damage_type, ret);
		break;
										  }
	case DAMAGE_PACKET_DAMAGE_TYPE_HEAT: {
		ret += GetInfoStruct()->get_heat() / 50;
		LogWrite(COMBAT__DEBUG, 3, "Combat", "DamageType: Heat (%i), Amt: %.2f", damage_type, ret);
		break;
										 }
	case DAMAGE_PACKET_DAMAGE_TYPE_COLD: {
		ret += GetInfoStruct()->get_cold() / 50;
		LogWrite(COMBAT__DEBUG, 3, "Combat", "DamageType: Cold (%i), Amt: %.2f", damage_type, ret);
		break;
										 }
	case DAMAGE_PACKET_DAMAGE_TYPE_MAGIC: {
		ret += GetInfoStruct()->get_magic() / 50;
		LogWrite(COMBAT__DEBUG, 3, "Combat", "DamageType: Magic (%i), Amt: %.2f", damage_type, ret);
		break;
										  }
	case DAMAGE_PACKET_DAMAGE_TYPE_MENTAL: {
		ret += GetInfoStruct()->get_mental() / 50;
		LogWrite(COMBAT__DEBUG, 3, "Combat", "DamageType: Mental (%i), Amt: %.2f", damage_type, ret);
		break;
										   }
	case DAMAGE_PACKET_DAMAGE_TYPE_DIVINE: {
		ret += GetInfoStruct()->get_divine() / 50;
		LogWrite(COMBAT__DEBUG, 3, "Combat", "DamageType: Divine (%i), Amt: %.2f", damage_type, ret);
		break;
										   }
	case DAMAGE_PACKET_DAMAGE_TYPE_DISEASE: {
		ret += GetInfoStruct()->get_disease() / 50;
		LogWrite(COMBAT__DEBUG, 3, "Combat", "DamageType: Disease (%i), Amt: %.2f", damage_type, ret);
		break;
											}
	case DAMAGE_PACKET_DAMAGE_TYPE_POISON: {
		ret += GetInfoStruct()->get_poison() / 50;
		LogWrite(COMBAT__DEBUG, 3, "Combat", "DamageType: Poison (%i), Amt: %.2f", damage_type, ret);
		break;
										   }
	case DAMAGE_PACKET_DAMAGE_TYPE_FOCUS: {
	   ret = 0.0f;
		break;
										  }
	}

	return ret;
}

Skill* Entity::GetSkillByWeaponType(int8 type, int8 damage_type, bool update) {
	if(type == DAMAGE_PACKET_TYPE_RANGE_DAMAGE) {
		return GetSkillByName("Ranged", update);
	}
	switch(damage_type) {
	case DAMAGE_PACKET_DAMAGE_TYPE_SLASH: // slash
		return GetSkillByName("Slashing", update);
	case DAMAGE_PACKET_DAMAGE_TYPE_CRUSH: // crush
		return GetSkillByName("Crushing", update);
	case DAMAGE_PACKET_DAMAGE_TYPE_PIERCE: // pierce
		return GetSkillByName("Piercing", update);
	case DAMAGE_PACKET_DAMAGE_TYPE_HEAT:
	case DAMAGE_PACKET_DAMAGE_TYPE_COLD:
	case DAMAGE_PACKET_DAMAGE_TYPE_MAGIC:
	case DAMAGE_PACKET_DAMAGE_TYPE_MENTAL:
	case DAMAGE_PACKET_DAMAGE_TYPE_DIVINE:
	case DAMAGE_PACKET_DAMAGE_TYPE_DISEASE:
	case DAMAGE_PACKET_DAMAGE_TYPE_POISON:
		return GetSkillByName("Disruption", update);
	case DAMAGE_PACKET_DAMAGE_TYPE_FOCUS:
		return GetSkillByName("Focus", update);
	}

	return 0;
}

bool Entity::DamageSpawn(Entity* victim, int8 type, int8 damage_type, int32 low_damage, int32 high_damage, const char* spell_name, int8 crit_mod, bool is_tick, bool no_calcs, bool ignore_attacker, bool take_power, LuaSpell* spell) {
	if(spell) {
		spell->is_damage_spell = true;
	}
	
	bool has_damaged = false;
	
	if(!victim || !victim->Alive() || victim->GetHP() == 0)
		return false;
	
	int8 hit_result = 0;
	int16 blow_type = 0;
	sint32 damage = 0;
	sint32 damage_before_crit = 0;
	bool crit = false;

	if(low_damage > high_damage)
		high_damage = low_damage;
	if(low_damage == high_damage)
		damage = low_damage;
	else
		 damage = MakeRandomInt(low_damage, high_damage);

	if(!no_calcs) {
		//this can be simplified by taking out the / 2, but I wanted the damage to be more consistent
		//damage = (rand()%((int)(high_damage/2-low_damage/2) + low_damage/2)) + (rand()%((int)(high_damage/2-low_damage/2) + low_damage/2)); 
		//damage = (rand()%((int)(high_damage-low_damage) + low_damage)) + (rand()%((int)(high_damage-low_damage) + low_damage)); 

		//DPS mod is only applied to auto attacks
		if (type == DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE || type == DAMAGE_PACKET_TYPE_RANGE_DAMAGE) {
			if(info_struct.get_dps_multiplier() != 0.0f) {
				damage *= (info_struct.get_dps_multiplier());
			}
		}
		//Potency and ability mod is only applied to spells/CAs
		else { 
			damage = CalculateDamageAmount(victim, damage, type, damage_type, spell);
		}

		if(!crit_mod || crit_mod == 1){
			//force crit if crit_mod == 1
			if(crit_mod == 1)
				crit = true;

			// Crit Roll
			else {
				victim->MStats.lock();
				float chance = max((float)0, (info_struct.get_crit_chance() - victim->stats[ITEM_STAT_CRITAVOIDANCE]));
				victim->MStats.unlock();
				if (MakeRandomFloat(0, 100) <= chance)
					crit = true;
			}
			if(crit){
				damage_before_crit = damage;
				//Apply total crit multiplier with crit bonus
				if(info_struct.get_crit_bonus() > 0)
					damage *= (1.3 + (info_struct.get_crit_bonus() / 100));
				else
					damage *= 1.3;

				// Change packet type to crit
				if (type == DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE)
					type = DAMAGE_PACKET_TYPE_SIMPLE_CRIT_DMG;
				else if (type == DAMAGE_PACKET_TYPE_SPELL_DAMAGE)
					type = DAMAGE_PACKET_TYPE_SPELL_CRIT_DMG;
			}
		}

		if(type == DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE || type == DAMAGE_PACKET_TYPE_SIMPLE_CRIT_DMG || type == DAMAGE_PACKET_TYPE_RANGE_DAMAGE) {			
			int16 effective_level_attacker = GetInfoStruct()->get_effective_level() != 0 ? GetInfoStruct()->get_effective_level() : GetLevel();
			float mit_percentage = CalculateMitigation(type, damage_type, effective_level_attacker, (IsPlayer() && victim->IsPlayer()));
			sint32 damage_to_reduce = (damage * mit_percentage);
			if(damage_to_reduce > damage)
				damage = 0;
			else
				damage -= damage_to_reduce;
			
			// if we reduce damage back below crit level then its no longer a crit, but we don't go below base damage
			if(type == DAMAGE_PACKET_TYPE_SIMPLE_CRIT_DMG && damage <= damage_before_crit) {
				damage = damage_before_crit;
				type = DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE;
			}
		}
	}

	bool useWards = false;

	if(damage <= 0){
		hit_result = DAMAGE_PACKET_RESULT_NO_DAMAGE;
		damage = 0;
		
		if(victim->IsNPC() && victim->GetHP() > 0)
			((Entity*)victim)->AddHate(this, damage);
	}
	else{
		hit_result = DAMAGE_PACKET_RESULT_SUCCESSFUL;
		sint32 return_damage = 0;
		if(GetZone()->CallSpawnScript(victim, SPAWN_SCRIPT_HEALTHCHANGED, this, 0, false, damage, &return_damage) && return_damage != 0)
		{
			// anything not 0 (no return) becomes considered 'immune' to the damage
			if(return_damage < 0) {
				damage = 0;
				hit_result = DAMAGE_PACKET_RESULT_NO_DAMAGE;
			}
			else if(return_damage != 0) {
				// otherwise we use what was given back to us (either less or greater)
				damage = return_damage;
			}
		}
		
		if(victim->IsNPC() && victim->GetHP() > 0)
			((Entity*)victim)->AddHate(this, damage);
		
		if(damage) {
			int32 prevDmg = damage;
			damage = victim->CheckWards(this, damage, damage_type);

			if (damage < (sint64)prevDmg)
				useWards = true;
			if(damage > 0 && spell) {
				has_damaged = true;
				spell->has_damaged = true;
			}
			if(take_power) {
				sint32 curPower = victim->GetPower();
				if(curPower < damage)
					curPower = 0;
				else
					curPower -= damage;
				victim->SetPower(curPower);
			}
			else {
				victim->TakeDamage(damage);
			}
			victim->CheckProcs(PROC_TYPE_DAMAGED, this);

			if (IsPlayer()) {
				switch (damage_type) {
				case DAMAGE_PACKET_DAMAGE_TYPE_SLASH:
				case DAMAGE_PACKET_DAMAGE_TYPE_CRUSH:
				case DAMAGE_PACKET_DAMAGE_TYPE_PIERCE:
					if (((Player*)this)->GetPlayerStatisticValue(STAT_PLAYER_HIGHEST_MELEE_HIT) < damage)
						((Player*)this)->UpdatePlayerStatistic(STAT_PLAYER_HIGHEST_MELEE_HIT, damage, true);
					victim->CheckProcs(PROC_TYPE_DAMAGED_MELEE, this);
					break;
				case DAMAGE_PACKET_DAMAGE_TYPE_HEAT:
				case DAMAGE_PACKET_DAMAGE_TYPE_COLD:
				case DAMAGE_PACKET_DAMAGE_TYPE_MAGIC:
				case DAMAGE_PACKET_DAMAGE_TYPE_MENTAL:
				case DAMAGE_PACKET_DAMAGE_TYPE_DIVINE:
				case DAMAGE_PACKET_DAMAGE_TYPE_DISEASE:
				case DAMAGE_PACKET_DAMAGE_TYPE_POISON:
					if (((Player*)this)->GetPlayerStatisticValue(STAT_PLAYER_HIGHEST_MAGIC_HIT) < damage)
						((Player*)this)->UpdatePlayerStatistic(STAT_PLAYER_HIGHEST_MAGIC_HIT, damage, true);
					victim->CheckProcs(PROC_TYPE_DAMAGED_MAGIC, this);
					break;
				}
			}
		}
	}

	Entity* attacker = nullptr;
	if(!ignore_attacker)
		attacker = this;
	if (damage > 0) {
		GetZone()->SendDamagePacket(attacker, victim, type, hit_result, damage_type, damage, spell_name);
		if (IsStealthed() || IsInvis())
			CancelAllStealth();

		if (victim->IsEntity())
			((Entity*)victim)->CheckInterruptSpell(this);
	}
	else if (useWards)
	{
		GetZone()->SendDamagePacket(attacker, victim, DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE, DAMAGE_PACKET_RESULT_NO_DAMAGE, damage_type, 0, spell_name);
	}

	if (victim->GetHP() <= 0)
		KillSpawn(victim, type, damage_type, blow_type);
	else {
		victim->CheckProcs(PROC_TYPE_DEFENSIVE, this);
		if (spell_name)
			victim->CheckProcs(PROC_TYPE_MAGICAL_DEFENSIVE, this);
		else
			victim->CheckProcs(PROC_TYPE_PHYSICAL_DEFENSIVE, this);
	}
	if(spell)
		spell->crit = crit;
	return has_damaged;
}

float Entity::CalculateMitigation(int8 type, int8 damage_type, int16 effective_level_attacker, bool for_pvp) {
	int16 effective_level_victim = GetInfoStruct()->get_effective_level() != 0 ? GetInfoStruct()->get_effective_level() : GetLevel();
	if(effective_level_attacker < 1 && effective_level_victim)
		effective_level_attacker = effective_level_victim;
	else
		effective_level_attacker = 1;

	int32 effective_mit_cap = effective_level_victim * rule_manager.GetGlobalRule(R_Combat, EffectiveMitigationCapLevel)->GetInt32();
	float max_mit = (float)GetInfoStruct()->get_max_mitigation();
	if(max_mit == 0.0f)
		max_mit = effective_level_victim * 100.0f;
	
	int32 mit_to_use = 0;
	switch(type) {
	
		case DAMAGE_PACKET_TYPE_SIMPLE_DAMAGE:
		case DAMAGE_PACKET_TYPE_RANGE_DAMAGE:
			mit_to_use = GetInfoStruct()->get_cur_mitigation();
		break;
		case DAMAGE_PACKET_TYPE_SIMPLE_CRIT_DMG:
			// since critical mitigation is a percentage we will reverse the mit value so we can add skill from specific types of weapons
			mit_to_use = (int32)((float)GetInfoStruct()->get_max_mitigation() * (float)(GetInfoStruct()->get_critical_mitigation()/100.0f));
		break;
		
	}
	
	switch(damage_type) {
			case DAMAGE_PACKET_DAMAGE_TYPE_SLASH:
				mit_to_use += GetInfoStruct()->get_mitigation_skill1(); // slash
			break;
			case DAMAGE_PACKET_DAMAGE_TYPE_PIERCE:
				mit_to_use += GetInfoStruct()->get_mitigation_skill2(); // pierce
			break;
			case DAMAGE_PACKET_DAMAGE_TYPE_CRUSH:
				mit_to_use += GetInfoStruct()->get_mitigation_skill3(); // crush
			break;
			case DAMAGE_PACKET_DAMAGE_TYPE_FOCUS:
				return 0.0f; // focus cannot be mitigated, just break out of this now
			break;
			default:
			// do nothing
			break;
	}
	
	if(for_pvp) {
		mit_to_use += effective_level_victim * rule_manager.GetGlobalRule(R_Combat, MitigationLevelEffectivenessMax)->GetInt32();
	}
	
	if(mit_to_use > effective_mit_cap) {
		mit_to_use = effective_mit_cap;
	}
	float level_diff = ((float)effective_level_victim / (float)effective_level_attacker);
	if(level_diff > rule_manager.GetGlobalRule(R_Combat, MitigationLevelEffectivenessMax)->GetFloat()) {
		level_diff = rule_manager.GetGlobalRule(R_Combat, MitigationLevelEffectivenessMax)->GetFloat();
	}
	else if(level_diff < rule_manager.GetGlobalRule(R_Combat, MitigationLevelEffectivenessMax)->GetFloat()) {
		level_diff = rule_manager.GetGlobalRule(R_Combat, MitigationLevelEffectivenessMin)->GetFloat();
	}
	float mit_percentage = ((float)mit_to_use / max_mit) * level_diff;
	
	if(!for_pvp && mit_percentage > rule_manager.GetGlobalRule(R_Combat, MaxMitigationAllowed)->GetFloat()) {
		mit_percentage = rule_manager.GetGlobalRule(R_Combat, MaxMitigationAllowed)->GetFloat();
	}
	else if(for_pvp && mit_percentage > rule_manager.GetGlobalRule(R_Combat, MaxMitigationAllowedPVP)->GetFloat()) {
		mit_percentage = rule_manager.GetGlobalRule(R_Combat, MaxMitigationAllowedPVP)->GetFloat();
	} 
	
	return mit_percentage;
}

void Entity::AddHate(Entity* attacker, sint32 hate) {
	if(!attacker || GetHP() <= 0 || attacker->GetHP() <= 0)
		return;

	if(IsInSpawnGroup(attacker))
		return; // can't aggro your own members
	
	// If a players pet and protect self is off
	if (IsPet() && ((NPC*)this)->GetOwner() && ((NPC*)this)->GetOwner()->IsPlayer() && ((((Player*)((NPC*)this)->GetOwner())->GetInfoStruct()->get_pet_behavior() & 2) == 0))
		return;

	hate = attacker->CalculateHateAmount(this, hate);
	
	if (IsNPC() && ((NPC*)this)->Brain()) {
		LogWrite(COMBAT__DEBUG, 3, "Combat", "Add NPC_AI Hate: Victim '%s', Attacker '%s', Hate: %i", GetName(), attacker->GetName(), hate);
		((NPC*)this)->Brain()->AddHate(attacker, hate);
		int8 loot_state = ((NPC*)this)->GetLockedNoLoot();
		// if encounter size is 0 then add the attacker to the encounter
		if ((loot_state != ENCOUNTER_STATE_AVAILABLE && loot_state != ENCOUNTER_STATE_BROKEN && ((NPC*)this)->Brain()->GetEncounterSize() == 0)) {
			((NPC*)this)->Brain()->AddToEncounter(attacker);
			AddTargetToEncounter(attacker);
		}
	}

	if (attacker->GetThreatTransfer() && hate > 0) {
		Spawn* transfer_target = (Entity*)GetZone()->GetSpawnByID(attacker->GetThreatTransfer()->Target);
		if (transfer_target && transfer_target->IsEntity()) {
			sint32 transfered_hate = hate * (attacker->GetThreatTransfer()->Amount / 100);
			hate -= transfered_hate;
			this->AddHate((Entity*)transfer_target, transfered_hate);
		}
	}

	// If pet is adding hate add some to the pets owner as well
	if (attacker->IsNPC() && ((NPC*)attacker)->IsPet())
		AddHate(((NPC*)attacker)->GetOwner(), 1);

	// If player and player has a pet and protect master is set add hate to the pet
	if (IsPlayer() && HasPet() && (((Player*)this)->GetInfoStruct()->get_pet_behavior() & 1)) {
		// If we have a combat pet add hate to it
		if (((Player*)this)->GetPet())
			AddHate(((Player*)this)->GetPet(), 1);
		if (((Player*)this)->GetCharmedPet())
			AddHate(((Player*)this)->GetCharmedPet(), 1);
	}

	// If this spawn has a spawn group then add the attacker to the hate list of the other
	// group members if not already in their list
	if (HasSpawnGroup()) {
		vector<Spawn*>* group = GetSpawnGroup();
		vector<Spawn*>::iterator itr;
		for (itr = group->begin(); itr != group->end(); itr++) {
			if (!(*itr)->IsNPC())
				continue;
			NPC* spawn = (NPC*)(*itr);
			if (spawn->Brain()->GetHate(attacker) == 0)
				spawn->Brain()->AddHate(attacker, 1);
		}
		safe_delete(group);
	}
}

bool Entity::CheckFizzleSpell(LuaSpell* spell) {
	if(!spell || !rule_manager.GetGlobalRule(R_Spells, EnableFizzleSpells)->GetInt8()
	|| spell->spell->GetSpellData()->can_fizzle == false)
		return false;

	float fizzleMaxSkill = rule_manager.GetGlobalRule(R_Spells, FizzleMaxSkill)->GetFloat();
	float baseFizzle = rule_manager.GetGlobalRule(R_Spells, DefaultFizzleChance)->GetFloat()/100.0f; // 10%
	float skillObtained = rule_manager.GetGlobalRule(R_Spells, FizzleDefaultSkill)->GetFloat(); // default of .2f so we don't go over the threshold if no skill
	Skill* skill = GetSkillByID(spell->spell->GetSpellData()->mastery_skill, false);
	if(skill && spell->spell->GetSpellData()->min_class_skill_req > 0)
	{
		float skillObtained = skill->current_val / spell->spell->GetSpellData()->min_class_skill_req;
		if(skillObtained > fizzleMaxSkill) // 120% over the skill value
		{
			skillObtained = fizzleMaxSkill;
		}
			
		baseFizzle = (fizzleMaxSkill - skillObtained) * baseFizzle;

		float totalSuccessChance = 1.0f - baseFizzle;

		float randResult = MakeRandomFloat(0.0f, 1.0f);
		if(randResult > totalSuccessChance)
			return true;
	}

	return false;
}

bool Entity::CheckInterruptSpell(Entity* attacker) {
	if(!IsCasting())
		return false;

	Spell* spell = GetZone()->GetSpell(this);
	if(!spell || spell->GetSpellData()->interruptable == 0)
		return false;

	if(GetInfoStruct()->get_no_interrupt())
		return false;
		
	//originally base of 30 percent chance to continue casting if attacked
	//modified to 50% and added global rule, 30% was too small at starting levels
	int8 percent = rule_manager.GetGlobalRule(R_Spells, NoInterruptBaseChance)->GetInt32();

	float focus_skill_with_bonus = CalculateSkillWithBonus("Focus", ITEM_STAT_FOCUS, true);

	percent += ((1 + focus_skill_with_bonus)/6);
	
	if(MakeRandomInt(1, 100) > percent) {
		LogWrite(COMBAT__DEBUG, 0, "Combat", "'%s' interrupted spell for '%s': %i%%", attacker->GetName(), GetName(), percent);
		GetZone()->Interrupted(this, attacker, SPELL_ERROR_INTERRUPTED);
		return true;
	}

	LogWrite(COMBAT__DEBUG, 0, "Combat", "'%s' failed to interrupt spell for '%s': %i%%", attacker->GetName(), GetName(), percent);
	return false;
}

void Entity::KillSpawn(Spawn* dead, int8 type, int8 damage_type, int16 kill_blow_type) {
	if(!dead)
		return;

	if (IsPlayer()) { // old clients do not support WS_EnterCombat, uses activity_status in spawn info
		Client* client = ((Player*)this)->GetClient();
		if(client && client->GetVersion() > 561) {
			PacketStruct* packet = configReader.getStruct("WS_EnterCombat", client->GetVersion());
			if (packet) {
				client->QueuePacket(packet->serialize());
			}
			safe_delete(packet);
		}

		((Player*)this)->InCombat(false);
	}

	if (IsPlayer() && dead->IsEntity())
		GetZone()->GetSpellProcess()->KillHOBySpawnID(dead->GetID());

	/* just for sake of not knowing if we are in a read lock, write lock, or no lock
	**  say spawnlist is locked (DismissPet arg 3 true), which means RemoveSpawn will remove the id from the spawn_list outside of the potential lock
	*/
	if (dead->IsPet() && ((NPC*)dead)->GetOwner())
		((NPC*)dead)->GetOwner()->DismissPet((NPC*)dead, true, true);
	else if (dead->IsEntity()) {
		// remove all pets for this entity
		((Entity*)dead)->DismissAllPets(false, true);
	}

	if (IsCasting())
		GetZone()->Interrupted(this, dead, SPELL_ERROR_NOT_ALIVE);

	LogWrite(COMBAT__DEBUG, 3, "Combat", "Killing '%s'", dead->GetName());

	// Kill movement for the dead npc so the corpse doesn't move
	GetZone()->movementMgr->StopNavigation((Entity*)dead);
	dead->ClearRunningLocations();
	dead->CalculateRunningLocation(true);

	GetZone()->KillSpawn(true, dead, this, true, type, damage_type, kill_blow_type);
}

void Entity::HandleDeathExperienceDebt(Spawn* killer)
{
	if(!IsPlayer())
		return;

	float ruleDebt = 0.0f;
	
	if(killer && killer->IsPlayer())
		ruleDebt = rule_manager.GetGlobalRule(R_Combat, PVPDeathExperienceDebt)->GetFloat()/100.0f;
	else
		ruleDebt = rule_manager.GetGlobalRule(R_Combat, DeathExperienceDebt)->GetFloat()/100.0f;

	if(ruleDebt > 0.0f)
	{
		bool groupDebt = rule_manager.GetGlobalRule(R_Combat, GroupExperienceDebt)->GetBool();
		if(groupDebt && ((Player*)this)->GetGroupMemberInfo())
		{
			world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);
			PlayerGroup* group = world.GetGroupManager()->GetGroup(((Player*)this)->GetGroupMemberInfo()->group_id);
			if (group)
			{
				group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
				deque<GroupMemberInfo*>* members = group->GetMembers();

				int32 size = members->size();
				float xpDebtPerMember = ruleDebt/(float)size;
				deque<GroupMemberInfo*>::iterator itr;
				for (itr = members->begin(); itr != members->end(); itr++) {
					GroupMemberInfo* gmi = *itr;
					if (gmi->client && gmi->client->GetPlayer()) {
						gmi->client->GetPlayer()->GetInfoStruct()->set_xp_debt(gmi->client->GetPlayer()->GetInfoStruct()->get_xp_debt()+xpDebtPerMember);
						gmi->client->GetPlayer()->SetCharSheetChanged(true);
					}
				}
				group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
			}
			world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
		}
		else
		{
			((Player*)this)->GetInfoStruct()->set_xp_debt(((Player*)this)->GetInfoStruct()->get_xp_debt()+ruleDebt);
			((Player*)this)->SetCharSheetChanged(true);
		}
	}
}

void Entity::ProcessCombat() {
	// This is a virtual function so when a NPC calls this it will use the NPC::ProcessCombat() version
	// and a player will use the Player::ProcessCombat() version, leave this function blank.
}

void NPC::ProcessCombat() {
	MBrain.writelock(__FUNCTION__, __LINE__);
	// Check to see if it is time to call the AI again
	if (m_brain && GetHP() > 0 && Timer::GetCurrentTime2() >= (m_brain->LastTick() + m_brain->Tick())) {
		// Probably never want to use the following log, will spam the console for every NPC in a zone 4 times a second
		//LogWrite(NPC_AI__DEBUG, 9, "NPC_AI", "%s is thinking...", GetName());
		m_brain->Think();
		// Set the time for when the brain was last called
		m_brain->SetLastTick(Timer::GetCurrentTime2());
	}
	MBrain.releasewritelock(__FUNCTION__, __LINE__);
}

void Player::ProcessCombat() {
	// if not in combat OR casting a spell OR dazed  OR feared return out
	if (!EngagedInCombat() || IsCasting() || IsDazed() || IsFeared())
		return;

	//If no target delete combat_target and return out
	Spawn* Target = GetZone()->GetSpawnByID(target);
	if (!Target) {
		combat_target = 0;
		if (target > 0) {
			SetTarget(0);
		}
		return;
	}
	// If is not an entity return out
	if (!Target->IsEntity())
		return;

	// Reset combat target
	combat_target = 0;

	if (Target->HasTarget()) {
		if (Target->IsPlayer() || (Target->IsNPC() && Target->IsPet() && ((NPC*)Target)->GetOwner() && ((NPC*)Target)->GetOwner()->IsPlayer())){
			Spawn* secondary_target = Target->GetTarget();
			if (secondary_target->IsNPC() && secondary_target->appearance.attackable) {
				if (!secondary_target->IsPet() || (secondary_target->IsPet() && ((NPC*)secondary_target)->GetOwner() && ((NPC*)secondary_target)->GetOwner()->IsNPC())) {
					combat_target = secondary_target;
				}
			}
		}
	}
	
	// If combat_target wasn't set in the above if set it to the original target
	if (!combat_target)
		combat_target = Target;
	
	// this if may not be required as at the min combat_target will be Target, which we already check at the begining
	if(!combat_target)
		return;

	float distance = 0;
	distance = GetDistance(combat_target);

	// Check to see if we are doing ranged auto attacks if not check to see if we are in melee range
	if (GetRangeAttack()) {
		// We are doing ranged auto attacks
		
		//check to see if we can attack the target AND the ranged weapon is ready
		if(AttackAllowed((Entity*)combat_target, distance, true) && RangeWeaponReady()) {
			Item* weapon = 0;
			Item* ammo = 0;
			// Get the currently equiped weapon and ammo for the ranged attack
			weapon = GetEquipmentList()->GetItem(EQ2_RANGE_SLOT);
			ammo = GetAmmoFromSlot(true, true);
			LogWrite(COMBAT__DEBUG, 1, "Combat", "Weapon '%s', Ammo '%s'", ( weapon )? weapon->name.c_str() : "None", ( ammo ) ? ammo->name.c_str() : "None");

			// If weapon and ammo are both valid perform the ranged attack else send a message to the client
			if(weapon && ammo) {
				LogWrite(COMBAT__DEBUG, 1, "Combat", "Weapon: Primary, Fighter: '%s', Target: '%s', Distance: %.2f", GetName(), combat_target->GetName(), distance);
				RangeAttack(combat_target, distance, weapon, ammo);
			}
			else {
				Client* client = ((Player*)this)->GetClient();
				if (client) {
					// Need to get messages from live, made these up so the player knows what is wrong in game if weapon or ammo are not valid
					if (!ammo)
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Out of ammo.");
					if (!weapon)
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "No ranged weapon found.");
					
				}
			}
		}
	}
	else if(distance <= rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat()) {
		// We are doing melee auto attacks and are within range

		// Check to see if we can attack the target
		if(AttackAllowed((Entity*)combat_target)) {
			// Check to see if the primary melee weapon is ready
			if(PrimaryWeaponReady()) {
				// Set the time of the last melee attack with the primary weapon and perform the melee attack with primary weapon
				SetPrimaryLastAttackTime(Timer::GetCurrentTime2());
				MeleeAttack(combat_target, distance, true);
			}
			// Check to see if the secondary weapon is ready
			if(SecondaryWeaponReady()) {
				// set the time of the last melee attack with the secondary weapon and perform the melee attack with the secondary weapon
				SetSecondaryLastAttackTime(Timer::GetCurrentTime2());
				MeleeAttack(combat_target, distance, false);
			}
		}
	}
}

void Entity::SetAttackDelay(bool primary, bool ranged) {
	float mod = CalculateAttackSpeedMod();
	bool dual_wield = IsDualWield();

	//Note: Capping all attack speed increases at 125% normal speed (from function CalculateAttackSpeedMod())
	//Add 33% longer delay if dual wielding
	if(dual_wield && ! ranged) {
		if(primary)
			SetPrimaryAttackDelay((GetPrimaryWeaponDelay() * 1.33) / mod);
		else
			SetSecondaryAttackDelay((GetSecondaryWeaponDelay() * 1.33) / mod);
	}
	else {
		if(primary)
			SetPrimaryAttackDelay(GetPrimaryWeaponDelay() / mod);
		else if(ranged)
			SetRangeAttackDelay(GetRangeWeaponDelay() / mod);
		else
			SetSecondaryAttackDelay(GetSecondaryWeaponDelay() / mod);
	}
}

float Entity::CalculateAttackSpeedMod(){
	float aspeed = info_struct.get_attackspeed();
	
	if(aspeed > 0) {
		if (aspeed <= 100)
			return (aspeed / 100 + 1);
		else if (aspeed <= 200)
			return 2.25;
	}
	return 1;
}

void Entity::AddProc(int8 type, float chance, Item* item, LuaSpell* spell, int8 damage_type, int8 hp_ratio, bool below_health, bool target_health, bool extended_version) {
	if (type == 0) {
		LogWrite(COMBAT__ERROR, 0, "Proc", "Entity::AddProc called with an invalid type.");
		return;
	}

	if (!item && !spell) {
		LogWrite(COMBAT__ERROR, 0, "Proc", "Entity::AddProc must have a valid item or spell.");
		return;
	}

	MProcList.writelock(__FUNCTION__, __LINE__);
	Proc* proc = new Proc();
	proc->chance = chance;
	proc->item = item;
	proc->spell = spell;
	if(spell) {
		spell->has_proc = true;
	}
	if(spell && spell->spell) {
		proc->spellid = spell->spell->GetSpellID();
	}
	proc->health_ratio = hp_ratio;
	proc->below_health = below_health;
	proc->damage_type = damage_type;
	proc->target_health = target_health;
	proc->extended_version = extended_version;
	m_procList[type].push_back(proc);
	MProcList.releasewritelock(__FUNCTION__, __LINE__);
}

void Entity::RemoveProc(Item* item, LuaSpell* spell) {
	if (!item && !spell) {
		LogWrite(COMBAT__ERROR, 0, "Proc", "Entity::RemoveProc must have a valid item or spell.");
		return;
	}

	MProcList.writelock(__FUNCTION__, __LINE__);
	map<int8, vector<Proc*> >::iterator proc_itr;
	vector<Proc*>::iterator itr;
	for (proc_itr = m_procList.begin(); proc_itr != m_procList.end(); proc_itr++) {
		itr = proc_itr->second.begin();
		while (itr != proc_itr->second.end()) {
			Proc* proc = *itr;

			if ((item && proc->item == item) || (spell && proc->spell == spell)) {
				itr = proc_itr->second.erase(itr);
				safe_delete(proc);
			}
			else
				itr++;
		}
	}
	MProcList.releasewritelock(__FUNCTION__, __LINE__);
}

bool Entity::CastProc(Proc* proc, int8 type, Spawn* target) {
	lua_State* state = 0;
	bool item_proc = false;
	int8 num_args = 3;
	Mutex* mutex = 0;
	if (proc->spell) {
		state = proc->spell->state;
	}
	else if (proc->item) {
		state = lua_interface->GetItemScript(proc->item->GetItemScript(), true, true);
		item_proc = true;
	}
	
	if (!state) {
		LogWrite(COMBAT__ERROR, 0, "Proc", "No valid lua_State* found");
		return false;
	}
	
	if(item_proc) {
		mutex = lua_interface->GetItemScriptMutex(proc->item->GetItemScript());
	}
	
	if(mutex)
		mutex->readlock(__FUNCTION__, __LINE__);

	if(proc->extended_version) {
		lua_getglobal(state, "proc_ext");
	}
	else {
		lua_getglobal(state, "proc");
	}
	if (lua_isfunction(state, lua_gettop(state))) {
		if (item_proc) {
			num_args++;
			lua_interface->SetItemValue(state, proc->item);
		}

		lua_interface->SetSpawnValue(state, this);
		lua_interface->SetSpawnValue(state, target);
		lua_interface->SetInt32Value(state, type);
		
		if(proc->extended_version) {
			lua_interface->SetInt32Value(state, proc->damage_type);
		}

		/*
		Add spell data from db in case of a spell proc here...
		*/
		if (!item_proc) {
			// Append spell data to the param list
			vector<LUAData*>* data = proc->spell->spell->GetLUAData();
			for(int32 i = 0; i < data->size(); i++) {
				switch(data->at(i)->type) {
				case 0:{
					lua_interface->SetSInt32Value(proc->spell->state, data->at(i)->int_value);
					break;
					   }
				case 1:{
					lua_interface->SetFloatValue(proc->spell->state, data->at(i)->float_value);
					break;
					   }
				case 2:{
					lua_interface->SetBooleanValue(proc->spell->state, data->at(i)->bool_value);
					break;
					   }
				case 3:{
					lua_interface->SetStringValue(proc->spell->state, data->at(i)->string_value.c_str());
					break;
					   }
				default:{
					LogWrite(SPELL__ERROR, 0, "Spell", "Error: Unknown LUA Type '%i' in Entity::CastProc for Spell '%s'", (int)data->at(i)->type, proc->spell->spell->GetName());
					return false;
						}
				}
				num_args++;
			}
		}

		if (lua_pcall(state, num_args, 0, 0) != 0) {
			LogWrite(COMBAT__ERROR, 0, "Proc", "Unable to call the proc function for spell %i tier %i", proc->spell->spell->GetSpellID(), proc->spell->spell->GetSpellTier());
			lua_pop(state, 1);
			if(mutex)
				mutex->releasereadlock(__FUNCTION__, __LINE__);
			if(item_proc)
				lua_interface->UseItemScript(proc->item->GetItemScript(), state, false);
			return false;
		}
	}
	else if(state) {
		lua_pop(state, 1);
	}
	lua_interface->ResetFunctionStack(state);
	if(mutex)
		mutex->releasereadlock(__FUNCTION__);
	if(item_proc)
		lua_interface->UseItemScript(proc->item->GetItemScript(), state, false);
	return true;
}

void Entity::CheckProcs(int8 type, Spawn* target) {
	if (type == 0) {
		LogWrite(COMBAT__ERROR, 0, "Proc", "Entity::CheckProcs called with an invalid type.");
		return;
	}

	vector<Proc*> tmpList;

	MProcList.readlock(__FUNCTION__, __LINE__);
	for (int8 i = 0; i < m_procList[type].size(); i++) {
		// roll per proc, not overall
		float roll = MakeRandomFloat(0, 100);
		Proc* proc = m_procList[type].at(i);
		if (roll <= proc->chance && 
			((!proc->extended_version || proc->health_ratio == 0) || 
			(proc->below_health && !proc->target_health && proc->health_ratio < (int8)GetIntHPRatio()) ||
			(!proc->below_health && !proc->target_health && proc->health_ratio >= (int8)GetIntHPRatio()) ||
			(target && proc->below_health && proc->target_health && proc->health_ratio < (int8)target->GetIntHPRatio()) ||
			(target && !proc->below_health && proc->target_health && proc->health_ratio >= (int8)target->GetIntHPRatio()))
			)
		{
			Proc* tmpProc = new Proc();
			tmpProc->chance = proc->chance;
			tmpProc->item = proc->item;
			tmpProc->spell = proc->spell;
			tmpProc->spellid = proc->spellid;
			tmpProc->damage_type = proc->damage_type;
			tmpProc->health_ratio = proc->health_ratio;
			tmpProc->below_health = proc->below_health;
			tmpProc->target_health = proc->target_health;
			tmpProc->extended_version = proc->extended_version;
			tmpList.push_back(tmpProc);
		}
	}
	MProcList.releasereadlock(__FUNCTION__, __LINE__);


	vector<Proc*>::iterator proc_itr;
	for (proc_itr = tmpList.begin(); proc_itr != tmpList.end();) {
		Proc* tmpProc = *proc_itr;
		CastProc(tmpProc, type, target);
		proc_itr++;
		safe_delete(tmpProc);
	}
}

void Entity::ClearProcs() {
	MProcList.writelock(__FUNCTION__, __LINE__);

	map<int8, vector<Proc*> >::iterator proc_itr;
	vector<Proc*>::iterator itr;
	for (proc_itr = m_procList.begin(); proc_itr != m_procList.end(); proc_itr++) {
		itr = proc_itr->second.begin();
		while (itr != proc_itr->second.end()) {
			safe_delete(*itr);
			itr = proc_itr->second.erase(itr);
		}
		proc_itr->second.clear();
	}
	m_procList.clear();

	MProcList.releasewritelock(__FUNCTION__, __LINE__);
}

sint32 Entity::CalculateHateAmount(Spawn* target, sint32 amt) {
	amt = CalculateFormulaByStat(amt, ITEM_STAT_TAUNT_AMOUNT);

	amt = CalculateFormulaByStat(amt, ITEM_STAT_TAUNT_AND_COMBAT_ART_DAMAGE);

	amt = CalculateFormulaByStat(amt, ITEM_STAT_ABILITY_MODIFIER);

	return amt;
}

sint32 Entity::CalculateHealAmount(Spawn* target, sint32 amt, int8 crit_mod, bool* crit, bool skip_crit_mod) {
	amt = CalculateFormulaByStat(amt, ITEM_STAT_HEAL_AMOUNT);
	
	amt = CalculateFormulaByStat(amt, ITEM_STAT_SPELL_AND_HEAL);

	//Potency Mod
	amt = CalculateFormulaByStat(amt, ITEM_STAT_POTENCY);
	
	//Ability Mod
	amt += (int32)min((int32)GetInfoStruct()->get_ability_modifier(), (int32)(amt / 2));

	if(!skip_crit_mod){
		if(!crit_mod || crit_mod == 1){
			if(crit_mod == 1) 
				*crit = true;
			else if(!*crit) {
				// Crit Roll
				float chance = (float)max((float)0, (float)GetInfoStruct()->get_crit_chance());
				*crit = (MakeRandomFloat(0, 100) <= chance); 
			}
			if(*crit){
				//Apply total crit multiplier with crit bonus
				amt *= ((GetInfoStruct()->get_crit_bonus() / 100) + 1.3);
			}
		}
	}

	return amt;
}

sint32 Entity::CalculateDamageAmount(Spawn* target, sint32 damage, int8 base_type, int8 damage_type, LuaSpell* spell) {
	return CalculateDamageAmount(target, damage, base_type, damage_type, (spell && spell->spell) ? spell->spell->GetSpellData()->target_type : 0);
}

sint32 Entity::CalculateDamageAmount(Spawn* target, sint32 damage, int8 base_type, int8 damage_type, int8 target_type) {
	if(damage_type == DAMAGE_PACKET_DAMAGE_TYPE_FOCUS) {
		return damage; // cannot avoid focus damage
	}
	
	// only spells may add spell damage item stat
	if(damage_type >= DAMAGE_PACKET_DAMAGE_TYPE_HEAT && damage_type <= DAMAGE_PACKET_DAMAGE_TYPE_POISON)
	{
		// https://forums.daybreakgames.com/eq2/index.php?threads/potency-and-ability-mod-what.4316/
		// Spell damage model assuming 100% crit chance:
		// (Spell base damage * Base damage modifier * Int bonus * Skill bonus * Potency + Ability modifier) * Crit bonus * Spell double attack
		/** Spell base damage: Get all master spells
			Base damage modifier: Very very rare. Available from Wizard aa Wisdom line(Brainstorm). Get it, cherish it.
			Int bonus: Past 1200 or so int, you will get a 10% increase in spell damage per 30% increase in int. Look at int tooltip to see the numerical value.
			Skill bonus: Past skill cap, 100 points skill is worth 2% increase in minimum spell damage, which translates to 1% overall increase. Looks at the skill that the spell uses, for wizards mostly disruption. Cap is 6.5*level.
			Potency: A straight damage modifier, the more you can get the better. No practical cap.
			Ability modifier: A straight damage modifier. Limited to 50% of the spell base damage, but for a wizard this usually has little consequence. However note that this is not affected by Potency.
			Crit bonus: A straight damage modifier, the more you can get the better. No practical cap. Wizards got 50% intrinsic Crit Bonus that does not show up in the stat window, just add 50 to stat value for calculations.
			Spell double cast: A straight damage modifier, the more you can get the better. You won't be able to get very much of this.
			Makes the spell cast twice with some limitations.
		**/
		damage = damage + damage * CalculateLevelStatBonus(GetInt()); // lvl 70 * 1200 int = 84000, log10f(84000) = 4.924 / 50.0f = 0.09848
		damage = CalculateFormulaByStat(damage, ITEM_STAT_SPELL_DAMAGE);
	}

	if(base_type == DAMAGE_PACKET_TYPE_SPELL_DAMAGE) {
		damage = CalculateFormulaByStat(damage, ITEM_STAT_SPELL_AND_COMBAT_ART_DAMAGE);
		
		if(target && target->IsEntity() && damage > 0) {
			int16 effective_level_attacker = GetInfoStruct()->get_effective_level() != 0 ? GetInfoStruct()->get_effective_level() : GetLevel();
			float resistBase = ((Entity*)target)->GetDamageTypeResistPercentage(damage_type);
			if(resistBase > 1.0f) {
				float dmgReduction = ((Entity*)target)->CalculateSpellDamageReduction((float)damage, resistBase - 1.0f, effective_level_attacker);
				float newDamage = static_cast<float>(damage) - dmgReduction;
				if(newDamage < 0.0f)
					newDamage = 0.0f;
				damage = static_cast<sint32>(std::floor(newDamage));
			}
		}
	}

	// combat abilities only bonus
	if(damage_type <= DAMAGE_PACKET_DAMAGE_TYPE_PIERCE) {
		damage = CalculateFormulaByStat(damage, ITEM_STAT_TAUNT_AND_COMBAT_ART_DAMAGE);
	}
			
	// Potency mod
	damage = CalculateFormulaByStat(damage, ITEM_STAT_POTENCY);

	int32 modifier = 2;
	if(target_type == SPELL_TARGET_GROUP_AE || target_type == SPELL_TARGET_RAID_AE || target_type == SPELL_TARGET_OTHER_GROUP_AE)
	{
		modifier = 3;
	}

	// Ability mod can only give up to half of damage after potency
	int32 mod = (int32)min(info_struct.get_ability_modifier(), (float)(damage / modifier));
	damage += mod;

	return damage;
}

sint32 Entity::CalculateFormulaByStat(sint32 value, int16 stat) {
	sint32 outValue = value;
	MStats.lock();
	std::map<int16, float>::iterator itr = stats.find(stat);
	if(itr != stats.end())
		outValue = (sint32)((float)value * ((itr->second / 100.0f) + 1.0f));
	MStats.unlock();

	return outValue;
}

int32 Entity::CalculateFormulaByStat(int32 value, int16 stat) {
	int32 outValue = value;
	MStats.lock();
	std::map<int16, float>::iterator itr = stats.find(stat);
	if(itr != stats.end())
		outValue = (int32)((float)value * ((itr->second / 100.0f) + 1.0f));
	MStats.unlock();

	return outValue;
}

int32 Entity::CalculateFormulaBonus(int32 value, float percent_bonus) {
	return (int32)((float)value * ((percent_bonus / 100.0f) + 1.0f));
}

float Entity::CalculateSpellDamageReduction(float spellDamage, float resistancePercentage, int16 attackerLevel) {
	int16 effective_level = GetInfoStruct()->get_effective_level() != 0 ? GetInfoStruct()->get_effective_level() : GetLevel();
	float levelDifference = std::abs(effective_level - attackerLevel);
	float logFactor = 1.0f / (1.0f + 0.1f * levelDifference); // Adjust the factor for smoother reduction

	// Calculate the actual damage reduction based on resistance percentage, logarithmic factor, and maximum reduction
	float reducedDamage = spellDamage * (1.0f - resistancePercentage) * logFactor * GetInfoStruct()->get_max_spell_reduction();
	return reducedDamage;
}