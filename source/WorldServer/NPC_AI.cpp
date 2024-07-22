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

#include "NPC_AI.h"
#include "Combat.h"
#include "zoneserver.h"
#include "Spells.h"
#include "../common/Log.h"
#include "LuaInterface.h"
#include "World.h"
#include "Rules/Rules.h"

extern RuleManager rule_manager;

extern LuaInterface* lua_interface;
extern World world;

/*  The NEW AI code  */

Brain::Brain(NPC* npc) {
	// Set the npc this brain will controll
	m_body = npc;
	// Set the default time between calls to think to 250 miliseconds (1/4 a second)
	m_tick = 250;
	m_lastTick = Timer::GetCurrentTime2();
	m_spellRecovery = 0;
	m_playerInEncounter = false;
	// Set up the mutex for the hate list
	MHateList.SetName("Brain::m_hatelist");
	// Set up the mutex for the encounter list
	MEncounter.SetName("Brain::m_encounter");
}

Brain::~Brain() {
	
}

void Brain::Think() {
	if (m_body->IsPet() && m_body->GetOwner() && m_body->GetOwner()->IsPlayer()) {
		Player* player = (Player*)m_body->GetOwner();
		if(player->GetInfoStruct()->get_pet_id() == 0) {
			player->GetInfoStruct()->set_pet_id(player->GetIDWithPlayerSpawn(m_body));
			player->SetCharSheetChanged(true);
		}
	}
	// Get the entity this NPC hates the most,
	// GetMostHated() will handle dead spawns so no need to check the health in this function
	Entity* target = GetMostHated();

	// If mezzed, stunned or feared we can't do anything so skip
	if (!m_body->IsMezzedOrStunned()) {
		// Not mezzed or stunned

		// Get the distance to the runback location
		float run_back_distance = m_body->GetRunbackDistance();

		if (target) {
			LogWrite(NPC_AI__DEBUG, 7, "NPC_AI", "%s has %s targeted.", m_body->GetName(), target->GetName());
			// NPC has an entity that it hates
			// Set the NPC's target to the most hated entity if it is not already.
			if (m_body->GetTarget() != target) {
				m_body->SetTarget(target);				
			}
			m_body->FaceTarget(target, false);
			// target needs to be set before in combat is engaged

			// If the NPC is not in combat then put them in combat
			if (!m_body->EngagedInCombat()) {
				m_body->ClearRunningLocations();
				m_body->InCombat(true);
				m_body->cast_on_aggro_completed = false;
				m_body->GetZone()->CallSpawnScript(m_body, SPAWN_SCRIPT_AGGRO, target);
			}

			bool breakWaterPursuit = false;
			if (m_body->IsWaterCreature() && !m_body->IsFlyingCreature() && !target->InWater())
				breakWaterPursuit = true;
			// Check to see if the NPC has exceeded the max chase distance
			if (run_back_distance > MAX_CHASE_DISTANCE || breakWaterPursuit) {
				LogWrite(NPC_AI__DEBUG, 7, "NPC_AI", "Run back distance is greater then max chase distance, run_back_distance = %f", run_back_distance);
				// Over the max chase distance, Check to see if the target is is a client
				if (target->IsPlayer() && ((Player*)target)->GetClient())
				{
					// Target is a client so send encounter break messages
					if (m_body->HasSpawnGroup())
						((Player*)target)->GetClient()->SimpleMessage(CHANNEL_NARRATIVE, "This encounter will no longer give encounter rewards.");
					else
						((Player*)target)->GetClient()->Message(CHANNEL_NARRATIVE, "%s is no longer worth any experience or treasure.", m_body->GetName());
				}
				
				// Clear the hate list for this NPC
				ClearHate();
				// Clear the encounter list
				ClearEncounter();
			}
			else {
				// Still within max chase distance lets to the combat stuff now

				float distance = m_body->GetDistance(target);

				if(!m_body->IsCasting() && (!HasRecovered() || !ProcessSpell(target, distance))) {
					LogWrite(NPC_AI__DEBUG, 7, "NPC_AI", "%s is attempting melee on %s.", m_body->GetName(), target->GetName());
					m_body->FaceTarget(target, false);
					ProcessMelee(target, distance);
				}
			}
		}
		else {
			// Nothing in the hate list
			bool wasInCombat = m_body->EngagedInCombat();
			// Check to see if the NPC is still flagged as in combat for some reason
			if (m_body->EngagedInCombat()) {
				// If it is set the combat flag to false
				m_body->InCombat(false);

				// Do not set a players pet to full health once they stop combat
				if (!m_body->IsPet() || (m_body->IsPet() && m_body->GetOwner() && !m_body->GetOwner()->IsPlayer()))
					m_body->SetHP(m_body->GetTotalHP());
			}

			CheckBuffs();

			// If run back distance is greater then 0 then run back
			if(!m_body->EngagedInCombat() && !m_body->IsPauseMovementTimerActive())
			{
				if (run_back_distance > 1 || (m_body->m_call_runback && !m_body->following)) {
					m_body->SetLockedNoLoot(ENCOUNTER_STATE_BROKEN);
					m_body->UpdateEncounterState(ENCOUNTER_STATE_BROKEN);
					m_body->GetZone()->AddChangedSpawn(m_body);
					m_body->Runback(run_back_distance);
					m_body->m_call_runback = false;
				}
				else if (m_body->GetRunbackLocation())
				{
					switch(m_body->GetRunbackLocation()->stage)
					{
						case 0:
							m_body->GetZone()->movementMgr->StopNavigation((Entity*)m_body);
							m_body->ClearRunningLocations();
							m_body->SetX(m_body->GetRunbackLocation()->x,false);
							m_body->SetZ(m_body->GetRunbackLocation()->z,false);
							m_body->SetY(m_body->GetRunbackLocation()->y,false);
							m_body->CalculateRunningLocation(true);
							m_body->GetRunbackLocation()->stage = 1;
							
							m_body->GetZone()->AddChangedSpawn(m_body);
							break;
						case 6: // artificially 1500ms per 250ms Think() call
							if (m_body->GetRunbackLocation()->gridid > 0)
								m_body->SetLocation(m_body->GetRunbackLocation()->gridid);
								if(m_body->GetTempActionState() == 0)
									m_body->SetTempActionState(-1);

							m_body->SetHeading(m_body->m_runbackHeadingDir1,m_body->m_runbackHeadingDir2,false);

							if(m_body->GetRunbackLocation()->reset_hp_on_runback)
								m_body->SetHP(m_body->GetTotalHP());

							m_body->ClearRunback();
							
							if(m_body->GetLockedNoLoot() != ENCOUNTER_STATE_AVAILABLE && m_body->Alive()) {
								m_body->SetLockedNoLoot(ENCOUNTER_STATE_AVAILABLE);
								m_body->UpdateEncounterState(ENCOUNTER_STATE_AVAILABLE);
							}

							m_body->GetZone()->AddChangedSpawn(m_body);
						break;
						default: // captures case 1 up to case 5 to turn around / reset hp
                                                        m_body->GetRunbackLocation()->stage++; // artificially delay
							break;

					}
				}
			}
			// If encounter size is greater then 0 then clear it
			if (GetEncounterSize() > 0)
				ClearEncounter();
		}
	}
}

sint32 Brain::GetHate(Entity* entity) {
	// We will use this variable to return the value, default to 0
	sint32 ret = 0;

	// Lock the hate list, not altering it so do a read lock
	MHateList.readlock(__FUNCTION__, __LINE__);

	// First check to see if the given entity is even in the hate list
	if (m_hatelist.count(entity->GetID()) > 0)
		// Entity in the hate list so get the hate value for the entity
		ret = m_hatelist[entity->GetID()];

	// Unlock the hate list
	MHateList.releasereadlock(__FUNCTION__, __LINE__);

	// return the hate
	return ret;
}

void Brain::AddHate(Entity* entity, sint32 hate) {
	// do not aggro when running back, despite taking damage
	if (m_body->IsNPC() && ((NPC*)m_body)->m_runningBack)
		return;
	else if (m_body->IsPet() && m_body->IsEntity() && ((Entity*)m_body)->GetOwner() == entity)
		return;

	if(m_body->IsImmune(IMMUNITY_TYPE_TAUNT))
	{
		LogWrite(NPC_AI__DEBUG, 7, "NPC_AI", "%s is immune to taunt from entity %s.", m_body->GetName(), entity ? entity->GetName() : "(null)");
		if(entity && entity->IsPlayer())
			((Player*)entity)->GetClient()->GetCurrentZone()->SendDamagePacket((Spawn*)entity, (Spawn*)m_body, DAMAGE_PACKET_TYPE_RANGE_SPELL_DMG, DAMAGE_PACKET_RESULT_IMMUNE, 0, 0, "Hate");
		return;
	}
	
	// Lock the hate list, we are altering the list so use write lock
	MHateList.writelock(__FUNCTION__, __LINE__);

	if (m_hatelist.count(entity->GetID()) > 0) {
		m_hatelist[entity->GetID()] += hate;
		// take into consideration that 0 or negative hate is not valid, we need to properly reset the value
		if(m_hatelist[entity->GetID()] < 1) {
			m_hatelist[entity->GetID()] = 1;
		}
	}
	else
		m_hatelist.insert(std::pair<int32, sint32>(entity->GetID(), hate));

	entity->MHatedBy.lock();
	if (entity->HatedBy.count(m_body->GetID()) == 0)
		entity->HatedBy.insert(m_body->GetID());
	entity->MHatedBy.unlock();

	// Unlock the list
	bool ownerExistsAddHate = false;
	
	if(entity->IsPet() && entity->GetOwner()) {
		map<int32, sint32>::iterator itr = m_hatelist.find(entity->GetOwner()->GetID());
		if(itr == m_hatelist.end()) {
			ownerExistsAddHate = true;
		}
	}
	MHateList.releasewritelock(__FUNCTION__, __LINE__);
	if(ownerExistsAddHate) {
		AddHate(entity->GetOwner(), 0);
	}
}

void Brain::ClearHate() {
	// Lock the hate list, we are altering the list so use a write lock
	MHateList.writelock(__FUNCTION__, __LINE__);

	map<int32, sint32>::iterator itr;
	for (itr = m_hatelist.begin(); itr != m_hatelist.end(); itr++) {
		Spawn* spawn = m_body->GetZone()->GetSpawnByID(itr->first);
		if (spawn && spawn->IsEntity())
		{
			((Entity*)spawn)->MHatedBy.lock();
			((Entity*)spawn)->HatedBy.erase(m_body->GetID());
			((Entity*)spawn)->MHatedBy.unlock();
		}
	}

	// Clear the list
	m_hatelist.clear();
	// Unlock the hate list
	MHateList.releasewritelock(__FUNCTION__, __LINE__);
}

void Brain::ClearHate(Entity* entity) {
	// Lock the hate list, we could potentially modify the list so use write lock
	MHateList.writelock(__FUNCTION__, __LINE__);

	// Check to see if the given entity is in the hate list
	if (m_hatelist.count(entity->GetID()) > 0)
		// Erase the entity from the hate list
		m_hatelist.erase(entity->GetID());

	entity->MHatedBy.lock();
	entity->HatedBy.erase(m_body->GetID());
	entity->MHatedBy.unlock();

	// Unlock the hate list
	MHateList.releasewritelock(__FUNCTION__, __LINE__);
}

Entity* Brain::GetMostHated() {
	map<int32, sint32>::iterator itr;
	int32 ret = 0;
	sint32 hate = 0;

	// Lock the hate list, not going to alter it so use a read lock
	MHateList.readlock(__FUNCTION__, __LINE__);

	if (m_hatelist.size() > 0) {
		// Loop through the list looking for the entity that this NPC hates the most
		for(itr = m_hatelist.begin(); itr != m_hatelist.end(); itr++) {
			// Compare the hate value for the current iteration to our stored highest value
			if(itr->second > hate) {
				// New high value store the entity
				ret = itr->first;
				// Store the value to compare with the rest of the entities
				hate = itr->second;
			}
		}
	}
	// Unlock the list
	MHateList.releasereadlock(__FUNCTION__, __LINE__);
	Entity* hated = (Entity*)GetBody()->GetZone()->GetSpawnByID(ret);
	// Check the reult to see if it is still alive
	if(hated && hated->GetHP() <= 0) {
		// Entity we got was dead so remove it from the list
		ClearHate(hated);
		// Call this function again now that we removed the dead entity
		hated = GetMostHated();
	}

	// Return our result
	return hated;
}

sint8 Brain::GetHatePercentage(Entity* entity) {
	float percentage = 0.0;
	MHateList.readlock(__FUNCTION__, __LINE__);
	if (entity && m_hatelist.count(entity->GetID()) > 0 && m_hatelist[entity->GetID()] > 0) {
		sint32 total_hate = 0;
		map<int32, sint32>::iterator itr;
		for (itr = m_hatelist.begin(); itr != m_hatelist.end(); itr++)
			total_hate += itr->second;
		percentage = m_hatelist[entity->GetID()] / total_hate;
	}
	MHateList.releasereadlock(__FUNCTION__, __LINE__);

	return (sint8)(percentage * 100);
}

void Brain::SendHateList(Client* client) {
	MHateList.readlock(__FUNCTION__, __LINE__);

	client->Message(CHANNEL_COLOR_YELLOW, "%s's HateList", m_body->GetName());
	client->Message(CHANNEL_COLOR_YELLOW, "-------------------");
	map<int32, sint32>::iterator itr;
	if (m_hatelist.size() > 0) {
		// Loop through the list looking for the entity that this NPC hates the most
		for(itr = m_hatelist.begin(); itr != m_hatelist.end(); itr++) {
			Entity* ent = (Entity*)GetBody()->GetZone()->GetSpawnByID(itr->first);
			// Compare the hate value for the current iteration to our stored highest value
			if(ent) {
				client->Message(CHANNEL_COLOR_YELLOW, "%s : %i", ent->GetName(), itr->second);
			}
			else {
				client->Message(CHANNEL_COLOR_YELLOW, "%u (cannot identity spawn id->entity) : %i", itr->first, itr->second);
			}
		}
	}
	client->Message(CHANNEL_COLOR_YELLOW, "-------------------");
	MHateList.releasereadlock(__FUNCTION__, __LINE__);
}

vector<Entity*>* Brain::GetHateList() {
	vector<Entity*>* ret = new vector<Entity*>;
	map<int32, sint32>::iterator itr;

	// Lock the list
	MHateList.readlock(__FUNCTION__, __LINE__);
	// Loop over the list storing the values into the new list
	for (itr = m_hatelist.begin(); itr != m_hatelist.end(); itr++) {
		Entity* ent = (Entity*)GetBody()->GetZone()->GetSpawnByID(itr->first);
		if (ent)
			ret->push_back(ent);
	}
	// Unlock the list
	MHateList.releasereadlock(__FUNCTION__, __LINE__);

	// Return the copy of the list
	return ret;
}

void Brain::MoveCloser(Spawn* target) {
	if (target && m_body->GetFollowTarget() != target)
		m_body->SetFollowTarget(target, rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat());

	if (m_body->GetFollowTarget() && !m_body->following) {
		m_body->CalculateRunningLocation(true);
		//m_body->ClearRunningLocations();
		m_body->following = true;
	}
}

bool Brain::ProcessSpell(Entity* target, float distance) {
	if(rand()%100 > m_body->GetCastPercentage() || m_body->IsStifled() || m_body->IsFeared())
		return false;
	Spell* spell = m_body->GetNextSpell(target, distance);
	if(spell){
		Spawn* spell_target = 0;
		if(spell->GetSpellData()->friendly_spell == 1){
			vector<Spawn*>* group = m_body->GetSpawnGroup();
			if(group && group->size() > 0){
				vector<Spawn*>::iterator itr;
				for(itr = group->begin(); itr != group->end(); itr++){
					if((!spell_target && (*itr)->GetHP() > 0 && (*itr)->GetHP() < (*itr)->GetTotalHP()) || (spell_target && (*itr)->GetHP() > 0 && spell_target->GetHP() > (*itr)->GetHP()))
						spell_target = *itr;
				}
			}
			if(!spell_target)
				spell_target = m_body;

			safe_delete(group);
		}
		else
			spell_target = target;
		
		BrainCastSpell(spell, spell_target, false);
		return true;
	}
	return false;
}

bool Brain::BrainCastSpell(Spell* spell, Spawn* cast_on, bool calculate_run_loc) {
	if (spell) {
		if(calculate_run_loc) {
			m_body->CalculateRunningLocation(true);
		}
		m_body->GetZone()->ProcessSpell(spell, m_body, cast_on);
		m_spellRecovery = (int32)(Timer::GetCurrentTime2() + (spell->GetSpellData()->cast_time * 10) + (spell->GetSpellData()->recovery * 10) + 2000);
		return true;
	}
	return false;
}

bool Brain::CheckBuffs() {
	if (!m_body->GetZone()->GetSpellProcess() || m_body->EngagedInCombat() || m_body->IsCasting() || m_body->IsMezzedOrStunned() || !m_body->Alive() || m_body->IsStifled() || !HasRecovered())
		return false;

	Spell* spell = m_body->GetNextBuffSpell(m_body);
	
	bool casted_on = false;
	if(!(casted_on = BrainCastSpell(spell, m_body)) && m_body->IsNPC() && ((NPC*)m_body)->HasSpells()) {
		Spawn* target = nullptr;
	
		vector<Spawn*>* group = m_body->GetSpawnGroup();
		if(group && group->size() > 0){
			vector<Spawn*>::iterator itr;
			for(itr = group->begin(); itr != group->end(); itr++){
				Spawn* spawn = (*itr);
				if(spawn->IsEntity() && spawn != m_body) {
					if(target) {
						Spell* spell = m_body->GetNextBuffSpell(spawn);
						SpellEffects* se = ((Entity*)spawn)->GetSpellEffect(spell->GetSpellData()->id);
						if(!se && BrainCastSpell(spell, spawn)) {
							casted_on = true;
							break;
						}
					}
				}
			}
		}
		safe_delete(group);
	}
	return casted_on;
}

void Brain::ProcessMelee(Entity* target, float distance) {
	if(distance > rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat())
		MoveCloser((Spawn*)target);
	else {
		if (target) {
			LogWrite(NPC_AI__DEBUG, 7, "NPC_AI", "%s is within melee range of %s.", m_body->GetName(), target->GetName());
			if (m_body->AttackAllowed(target)) {
				LogWrite(NPC_AI__DEBUG, 7, "NPC_AI", "%s is allowed to attack %s.", m_body->GetName(), target->GetName());
				if (m_body->PrimaryWeaponReady() && !m_body->IsDazed() && !m_body->IsFeared()) {
					LogWrite(NPC_AI__DEBUG, 7, "NPC_AI", "%s swings its primary weapon at %s.", m_body->GetName(), target->GetName());
					m_body->SetPrimaryLastAttackTime(Timer::GetCurrentTime2());
					m_body->MeleeAttack(target, distance, true);
					m_body->GetZone()->CallSpawnScript(m_body, SPAWN_SCRIPT_AUTO_ATTACK_TICK, target);
				}
				if (m_body->SecondaryWeaponReady() && !m_body->IsDazed()) {
					m_body->SetSecondaryLastAttackTime(Timer::GetCurrentTime2());
					m_body->MeleeAttack(target, distance, false);
				}
			}
		}
	}
}

bool Brain::HasRecovered() {
	if(m_spellRecovery > Timer::GetCurrentTime2())
		return false;
	
	m_spellRecovery = 0;
	return true;
}

void Brain::AddToEncounter(Entity* entity) {
	// If player pet then set the entity to the pets owner
	if (entity->IsPet() && entity->GetOwner() && !entity->IsBot()) {
		MEncounter.writelock(__FUNCTION__, __LINE__);
		bool success = AddToEncounter(entity->GetID());
		MEncounter.releasewritelock(__FUNCTION__, __LINE__);
		if(!success)
			return;
		entity = entity->GetOwner();
	}
	else if(entity->HasPet() && entity->GetPet()) {
		MEncounter.writelock(__FUNCTION__, __LINE__);
		bool success = AddToEncounter(entity->GetPet()->GetID());
		MEncounter.releasewritelock(__FUNCTION__, __LINE__);
		if(!success)
			return;
	}

	// If player or bot then get the group
	int32 group_id = 0;
	if (entity->IsPlayer() || entity->IsBot()) {
		m_playerInEncounter = true;
		if (entity->GetGroupMemberInfo())
			group_id = entity->GetGroupMemberInfo()->group_id;
	}

	// Insert the entity into the encounter list, if there is a group add all group members as well
	// TODO: add raid members
	MEncounter.writelock(__FUNCTION__, __LINE__);
	if (group_id > 0) {
		world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);

		deque<GroupMemberInfo*>::iterator itr;
		PlayerGroup* group = world.GetGroupManager()->GetGroup(group_id);
		if (group)
		{
			group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
			deque<GroupMemberInfo*>* members = group->GetMembers();
			for (itr = members->begin(); itr != members->end(); itr++) {
				if ((*itr)->member)
				{
					bool success = AddToEncounter((*itr)->member->GetID());
					if((*itr)->client && success) {
						m_encounter_playerlist.insert(make_pair((*itr)->client->GetPlayer()->GetCharacterID(), (*itr)->client->GetPlayer()->GetID()));
					}
				}
			}
			group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
		}

		world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
	}
	else {
		bool success = AddToEncounter(entity->GetID());
		if (success && entity->IsPlayer())
		{
			Player* plyr = (Player*)entity;
			m_encounter_playerlist.insert(make_pair(plyr->GetCharacterID(), entity->GetID()));
		}
	}
	MEncounter.releasewritelock(__FUNCTION__, __LINE__);
}

bool Brain::CheckLootAllowed(Entity* entity) {
	bool ret = false;
	vector<int32>::iterator itr;

	if (m_body)
	{
		if ((m_body->GetLootMethod() != GroupLootMethod::METHOD_LOTTO && m_body->GetLootMethod() != GroupLootMethod::METHOD_NEED_BEFORE_GREED) && m_body->GetLooterSpawnID() > 0 && m_body->GetLooterSpawnID() != entity->GetID()) {
			LogWrite(LOOT__INFO, 0, "Loot", "%s: CheckLootAllowed failed, looter spawn id %u does not match received %s(%u)", GetBody()->GetName(), m_body->GetLooterSpawnID(), entity->GetName(), entity->GetID());
			return false;
		}
		if (rule_manager.GetGlobalRule(R_Loot, AllowChestUnlockByDropTime)->GetInt8()
			&& m_body->GetChestDropTime() > 0 && Timer::GetCurrentTime2() >= m_body->GetChestDropTime() + (rule_manager.GetGlobalRule(R_Loot, ChestUnlockedTimeDrop)->GetInt32() * 1000)) {
			return true;
		}
		if (rule_manager.GetGlobalRule(R_Loot, AllowChestUnlockByTrapTime)->GetInt8()
			&& m_body->GetTrapOpenedTime() > 0 && Timer::GetCurrentTime2() >= m_body->GetChestDropTime() + (rule_manager.GetGlobalRule(R_Loot, ChestUnlockedTimeTrap)->GetInt32() * 1000)) {
			return true;
		}
		if ((m_body->GetLootMethod() == GroupLootMethod::METHOD_LOTTO || m_body->GetLootMethod() == GroupLootMethod::METHOD_NEED_BEFORE_GREED) && m_body->HasSpawnLootWindowCompleted(entity->GetID())) {
			LogWrite(LOOT__INFO, 0, "Loot", "%s: CheckLootAllowed failed, looter %s(%u) has already completed their lotto selections.", GetBody()->GetName(), entity->GetName(), entity->GetID());
			return false;
		}
	}
	// Check the encounter list to see if the given entity is in it, if so return true.
	MEncounter.readlock(__FUNCTION__, __LINE__);
	if (entity->IsPlayer())
	{
		Player* plyr = (Player*)entity;

		map<int32, int32>::iterator itr = m_encounter_playerlist.find(plyr->GetCharacterID());
		if (itr != m_encounter_playerlist.end())
		{
			MEncounter.releasereadlock(__FUNCTION__, __LINE__);
			return true;
		}

		MEncounter.releasereadlock(__FUNCTION__, __LINE__);
		return false;
	}

	for (itr = m_encounter.begin(); itr != m_encounter.end(); itr++) {
		if ((*itr) == entity->GetID()) {
			// found the entity in the encounter list, set return value to true and break the loop
			ret = true;
			break;
		}
	}
	MEncounter.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

int8 Brain::GetEncounterSize() {
	int8 ret = 0;

	MEncounter.readlock(__FUNCTION__, __LINE__);
	ret = (int8)m_encounter.size();
	MEncounter.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

vector<int32>* Brain::GetEncounter() {
	vector<int32>* ret = new vector<int32>;
	vector<int32>::iterator itr;

	// Lock the list
	MEncounter.readlock(__FUNCTION__, __LINE__);
	// Loop over the list storing the values into the new list
	for (itr = m_encounter.begin(); itr != m_encounter.end(); itr++)
		ret->push_back(*itr);
	// Unlock the list
	MEncounter.releasereadlock(__FUNCTION__, __LINE__);

	// Return the copy of the list
	return ret;
}

bool Brain::IsPlayerInEncounter(int32 char_id) {
	bool ret = false;
	MEncounter.readlock(__FUNCTION__, __LINE__);
	std::map<int32,int32>::iterator itr = m_encounter_playerlist.find(char_id);
	if(itr != m_encounter_playerlist.end()) {
		ret = true;
	}
	MEncounter.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

bool Brain::IsEntityInEncounter(int32 id, bool skip_read_lock) {
	bool ret = false;
	if(!skip_read_lock) {
		MEncounter.readlock(__FUNCTION__, __LINE__);
	}
	vector<int32>::iterator itr;
	for (itr = m_encounter.begin(); itr != m_encounter.end(); itr++) {
		if ((*itr) == id) {
			// found the entity in the encounter list, set return value to true and break the loop
			ret = true;
			break;
		}
	}
	if(!skip_read_lock) {
		MEncounter.releasereadlock(__FUNCTION__, __LINE__);
	}
	return ret;
}

int32 Brain::CountPlayerBotInEncounter() {
	int32 count = 0;
	vector<int32>::iterator itr;
	MEncounter.readlock(__FUNCTION__, __LINE__);	
	for (itr = m_encounter.begin(); itr != m_encounter.end(); itr++) {
		Entity* ent = (Entity*)GetBody()->GetZone()->GetSpawnByID((*itr));
		if (ent && (ent->IsPlayer() || ent->IsBot())) {
			count++;
		}
	}
	MEncounter.releasereadlock(__FUNCTION__, __LINE__);	
	return count;
}

bool Brain::AddToEncounter(int32 id) {
	if(!IsEntityInEncounter(id, true)) {
		m_encounter.push_back(id);
		return true;
	}
	return false;
}

void Brain::ClearEncounter() {
	MEncounter.writelock(__FUNCTION__, __LINE__);
	if(m_body) {
		m_body->RemoveSpells(true);
	}
	m_encounter.clear();
	m_encounter_playerlist.clear();
	m_playerInEncounter = false;
	MEncounter.releasewritelock(__FUNCTION__, __LINE__);
}

void Brain::SendEncounterList(Client* client) {
	client->Message(CHANNEL_COLOR_YELLOW, "%s's EncounterList", m_body->GetName());
	client->Message(CHANNEL_COLOR_YELLOW, "-------------------");
	vector<int32>::iterator itr;

	// Check the encounter list to see if the given entity is in it, if so return true.
	MEncounter.readlock(__FUNCTION__, __LINE__);

	for (itr = m_encounter.begin(); itr != m_encounter.end(); itr++) {
		Entity* ent = (Entity*)GetBody()->GetZone()->GetSpawnByID((*itr));
			// Compare the hate value for the current iteration to our stored highest value
			if(ent) {
				client->Message(CHANNEL_COLOR_YELLOW, "%s", ent->GetName());
			}
			else {
				client->Message(CHANNEL_COLOR_YELLOW, "%u (cannot identity spawn id->entity)", (*itr));
			}
	}
	client->Message(CHANNEL_COLOR_YELLOW, "-------------------");
	MEncounter.releasereadlock(__FUNCTION__, __LINE__);	
}

/* Example of how to extend the default AI */


CombatPetBrain::CombatPetBrain(NPC* body) : Brain(body) {
	// Make sure to have the " : Brain(body)" so it calls the parent class constructor
	// to set up the AI properly
}

CombatPetBrain::~CombatPetBrain() {

}

void CombatPetBrain::Think() {
	// We are extending the base brain so make sure to call the parent Think() function.
	// If we want to override then we could remove Brain::Think()
	Brain::Think();

	// All this Brain does is make the pet follow its owner, the combat comes from the default brain

	if (GetBody()->EngagedInCombat() || !GetBody()->IsPet() || GetBody()->IsMezzedOrStunned())
		return;
	
	LogWrite(NPC_AI__DEBUG, 7, "NPC_AI", "Pet AI code called for %s", GetBody()->GetName());

	// If owner is a player and player has stay set then return out
	if (GetBody()->GetOwner() && GetBody()->GetOwner()->IsPlayer() && ((Player*)GetBody()->GetOwner())->GetInfoStruct()->get_pet_movement() == 1)
		return;

	// Set target to owner
	Entity* target = GetBody()->GetOwner();
	GetBody()->SetTarget(target);

	// Get distance from the owner
	float distance = GetBody()->GetDistance(target);

	// If out of melee range then move closer
	if (distance > rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat())
		MoveCloser((Spawn*)target);
}

/* Example of how to override the default AI */


NonCombatPetBrain::NonCombatPetBrain(NPC* body) : Brain(body) {
	// Make sure to have the " : Brain(body)" so it calls the parent class constructor
	// to set up the AI properly
}

NonCombatPetBrain::~NonCombatPetBrain() {

}

void NonCombatPetBrain::Think() {
	// All this Brain does is make the pet follow its owner

	if (!GetBody()->IsPet() || GetBody()->IsMezzedOrStunned())
		return;
	
	LogWrite(NPC_AI__DEBUG, 7, "NPC_AI", "Pet AI code called for %s", GetBody()->GetName());

	// Set target to owner
	Entity* target = GetBody()->GetOwner();
	GetBody()->SetTarget(target);

	// Get distance from the owner
	float distance = GetBody()->GetDistance(target);

	// If out of melee range then move closer
	if (distance > rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat())
		MoveCloser((Spawn*)target);
}

BlankBrain::BlankBrain(NPC* body) : Brain(body) {
	// Make sure to have the " : Brain(body)" so it calls the parent class constructor
	// to set up the AI properly
	SetTick(50000);
}

BlankBrain::~BlankBrain() {

}

void BlankBrain::Think() {
	
}

LuaBrain::LuaBrain(NPC* body) : Brain(body) {

}

LuaBrain::~LuaBrain() {
}

void LuaBrain::Think() {
	if (!lua_interface)
		return;

	const char* script = GetBody()->GetSpawnScript();
	if(script) {
		if (!lua_interface->RunSpawnScript(script, "Think", GetBody(), GetBody()->GetTarget())) {
			lua_interface->LogError("LUA LuaBrain error: was unable to call the Think function in the spawn script (%s)", script);
		}
	}
	else {
		LogWrite(NPC_AI__ERROR, 0, "NPC_AI", "Lua brain set on a spawn that doesn't have a script...");
	}
}

DumbFirePetBrain::DumbFirePetBrain(NPC* body, Entity* target, int32 expire_time) : Brain(body) {
	m_expireTime = Timer::GetCurrentTime2() + expire_time;
	AddHate(target, INT_MAX);
}

DumbFirePetBrain::~DumbFirePetBrain() {

}

void DumbFirePetBrain::AddHate(Entity* entity, sint32 hate) {
	if (!GetMostHated())
		Brain::AddHate(entity, hate);
}

void DumbFirePetBrain::Think() {

	Entity* target = GetMostHated();

	if (target) {
		if (!GetBody()->IsMezzedOrStunned()) {
			// Set the NPC's target to the most hated entity if it is not already.
			if (GetBody()->GetTarget() != target) {
				GetBody()->SetTarget(target);
				GetBody()->FaceTarget(target, false);
			}
			// target needs to be identified before combat setting

			// If the NPC is not in combat then put them in combat
			if (!GetBody()->EngagedInCombat()) {
				//GetBody()->ClearRunningLocations();
				GetBody()->CalculateRunningLocation(true);
				GetBody()->InCombat(true);
			}

			float distance = GetBody()->GetDistance(target);

			if(GetBody()->CheckLoS(target) && !GetBody()->IsCasting() && (!HasRecovered() || !ProcessSpell(target, distance))) {
				LogWrite(NPC_AI__DEBUG, 7, "NPC_AI", "%s is attempting melee on %s.", GetBody()->GetName(), target->GetName());
				GetBody()->FaceTarget(target, false);
				ProcessMelee(target, distance);
			}
		}
	}
	else {
		// No hated target or time expired, kill this mob
		if (GetBody()->GetHP() > 0) {
			GetBody()->KillSpawn(GetBody());
			LogWrite(NPC_AI__DEBUG, 7, "NPC AI", "Dumbfire being killed because there is no target.");
		}
	}

	if (Timer::GetCurrentTime2() > m_expireTime) {
		if (GetBody()->GetHP() > 0) {
			GetBody()->KillSpawn(GetBody());
			LogWrite(NPC_AI__DEBUG, 7, "NPC AI", "Dumbfire being killed because timer expired.");
		}
	 }
}
