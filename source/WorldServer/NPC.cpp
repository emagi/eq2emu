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
#include "NPC.h"
#include "WorldDatabase.h"
#include <math.h>
#include "client.h"
#include "World.h"
#include "races.h"
#include "../common/Log.h"
#include "NPC_AI.h"
#include "Appearances.h"
#include "SpellProcess.h"
#include "Skills.h"
#include "Rules/Rules.h"

extern MasterSpellList master_spell_list;
extern ConfigReader configReader;
extern WorldDatabase database;
extern World world;
extern Races races;
extern Appearance master_appearance_list;
extern MasterSkillList master_skill_list;
extern RuleManager rule_manager;

NPC::NPC(){	
	Initialize();
	if (GetMaxSpeed() > 0)
		SetSpeed(GetMaxSpeed());
}

NPC::NPC(NPC* old_npc){
	Initialize();
	if(old_npc){
		if(old_npc->GetSizeOffset() > 0){
			int8 offset = old_npc->GetSizeOffset()+1;
			sint32 tmp_size = old_npc->size + (rand()%offset - rand()%offset);
			if(tmp_size < 0)
				tmp_size = 1;
			else if(tmp_size >= 0xFFFF)
				tmp_size = 0xFFFF;
			size = (int16)tmp_size;
		}
		else
			size = old_npc->size;
		SetCollector(old_npc->IsCollector());
		SetMerchantID(old_npc->GetMerchantID());
		SetMerchantType(old_npc->GetMerchantType());
		SetMerchantLevelRange(old_npc->GetMerchantMinLevel(), old_npc->GetMerchantMaxLevel());
		SetPrimaryCommands(&old_npc->primary_command_list);
		SetSecondaryCommands(&old_npc->secondary_command_list);
		appearance_id = old_npc->appearance_id;
		database_id = old_npc->database_id;
		primary_command_list_id = old_npc->primary_command_list_id;
		secondary_command_list_id = old_npc->secondary_command_list_id;
		this->SetInfoStruct(old_npc->GetInfoStruct());
		//memcpy(GetInfoStruct(), old_npc->GetInfoStruct(), sizeof(InfoStruct));
		memcpy(&appearance, &old_npc->appearance, sizeof(AppearanceData));
		memcpy(&features, &old_npc->features, sizeof(CharFeatures));
		memcpy(&equipment, &old_npc->equipment, sizeof(EQ2_Equipment));
		if(appearance.min_level < appearance.max_level)
			SetLevel(appearance.min_level + rand()%((appearance.max_level - appearance.min_level)+1));
		target = 0;
		SetTotalHPBase(old_npc->GetTotalHPBase());
		SetTotalHPBaseInstance(old_npc->GetTotalHPBase());
		SetTotalPowerBase(old_npc->GetTotalPowerBase());	
		SetTotalPowerBaseInstance(old_npc->GetTotalPowerBase());	
		faction_id = old_npc->faction_id;
		movement_interrupted = false;
		old_npc->SetQuestsRequired(this);
		SetTransporterID(old_npc->GetTransporterID());
		SetAIStrategy(old_npc->GetAIStrategy());
		SetPrimarySpellList(old_npc->GetPrimarySpellList());
		SetSecondarySpellList(old_npc->GetSecondarySpellList());
		SetPrimarySkillList(old_npc->GetPrimarySkillList());
		SetSecondarySkillList(old_npc->GetSecondarySkillList());
		SetEquipmentListID(old_npc->GetEquipmentListID());
		SetAggroRadius(old_npc->GetBaseAggroRadius());
		SetCastPercentage(old_npc->GetCastPercentage());
		SetRandomize(old_npc->GetRandomize());	
		if(appearance.randomize > 0)
			Randomize(this, appearance.randomize);
		CalculateBonuses();	
		SetHP(GetTotalHP());
		SetPower(GetTotalPower());
		UpdateWeapons();
		SetSoundsDisabled(old_npc->IsSoundsDisabled());
		SetFlyingCreature();
		SetWaterCreature();
		SetOmittedByDBFlag(old_npc->IsOmittedByDBFlag());
		SetLootTier(old_npc->GetLootTier());
		SetLootDropType(old_npc->GetLootDropType());
		has_spells = old_npc->HasSpells();
		SetScaredByStrongPlayers(old_npc->IsScaredByStrongPlayers());
	}
}

NPC::~NPC(){
	ResetMovement();
	if(skills){
		map<string, Skill*>::iterator skill_itr;
		for(skill_itr = skills->begin(); skill_itr != skills->end(); skill_itr++){
			safe_delete(skill_itr->second);	
		}
		safe_delete(skills);
	}
	if(spells) {
		vector<NPCSpell*>::iterator itr;
		for(itr = spells->begin(); itr != spells->end(); itr++){
			safe_delete((*itr));
		}
		spells->clear();
	}
	safe_delete(spells);
	MutexMap<int32, SkillBonus*>::iterator sb_itr = skill_bonus_list.begin();
	while (sb_itr.Next())
		RemoveSkillBonus(sb_itr.first);
	safe_delete(runback);
	safe_delete(m_brain);
}

void NPC::Initialize(){
	ai_strategy = 0;
	attack_type = 0;
	movement_index = 0;
	resume_movement = true;
	movement_start_time = 0;
	spawn_type = 2;
	movement_interrupted = false;
	attack_resume_needed = false;
	MMovementLoop.SetName("NPC::MMovementLoop");
	last_movement_update = Timer::GetCurrentTime2();
	aggro_radius = 0.0f;
	base_aggro_radius = 0.0f;
	skills = 0;
	spells = 0;
	runback = 0;
	m_brain = new ::Brain(this);
	MBrain.SetName("NPC::m_brain");
	m_runningBack = false;
	m_runbackHeadingDir1 = m_runbackHeadingDir2 = 0;
	following = false;
	SetFollowTarget(0);
	m_petDismissing = false;
	m_ShardID = 0;
	m_ShardCharID = 0;
	m_ShardCreatedTimestamp = 0;
	m_call_runback = false;
	has_spells = false;
	cast_on_aggro_completed = false;
}

EQ2Packet* NPC::serialize(Player* player, int16 version){
	return spawn_serialize(player, version);
}

void NPC::SetSkills(map<string, Skill*>* in_skills){
	if (skills) {
		map<string, Skill*>::iterator skill_itr;
		for(skill_itr = skills->begin(); skill_itr != skills->end(); skill_itr++){
			safe_delete(skill_itr->second);	
		}

		safe_delete(skills);
	}

	skills = in_skills;
}

void NPC::SetSpells(vector<NPCSpell*>* in_spells){
	for(int i=0;i<CAST_TYPE::MAX_CAST_TYPES;i++) {
		cast_on_spells[i].clear();
	}
	
	if(spells) {
		vector<NPCSpell*>::iterator itr;
		for(itr = spells->begin(); itr != spells->end(); itr++){
			safe_delete((*itr));
		}
	}
	safe_delete(spells);

	spells = in_spells;
	
	if(spells && spells->size() > 0) {
		has_spells = true;
		
		vector<NPCSpell*>::iterator itr;
		for(itr = spells->begin(); itr != spells->end();){
			if((*itr)->cast_on_spawn) {
				cast_on_spells[CAST_TYPE::CAST_ON_SPAWN].push_back((*itr));
				itr = spells->erase(itr); // we don't keep on the master list
				continue;
			}
			if((*itr)->cast_on_initial_aggro) {
				cast_on_spells[CAST_TYPE::CAST_ON_AGGRO].push_back((*itr));
				itr = spells->erase(itr); // we don't keep on the master list
				continue;
			}
			
			// didn't hit a continue case, iterate
			itr++;
		}
	}
	else {
		has_spells = false;
	}
}

void NPC::SetRunbackLocation(float x, float y, float z, int32 gridid, bool set_hp_runback){
	safe_delete(runback);
	runback = new MovementLocation;
	runback->x = x;
	runback->y = y;
	runback->z = z;
	runback->gridid = gridid;
	runback->stage = 0;
	runback->reset_hp_on_runback = set_hp_runback;
}

MovementLocation* NPC::GetRunbackLocation(){
	return runback;
}

float NPC::GetRunbackDistance(){
	if(!runback)
		return 0;
	return GetDistance(runback->x, runback->y, runback->z);
}

void NPC::Runback(float distance, bool stopFollowing){
	if(!runback)
		return;
	
	if ( distance == 0.0f )
		distance = GetRunbackDistance(); // gotta make sure its true, lua doesn't send the distance
	
	if(stopFollowing)
		following = false;
	
	if (!m_runningBack)
	{
		ClearRunningLocations();
		GetZone()->movementMgr->StopNavigation((Entity*)this);
	}

	m_runningBack = true;
	SetSpeed(GetMaxSpeed()*2);

	if (CheckLoS(glm::vec3(runback->x, runback->z, runback->y + 1.0f), glm::vec3(GetX(), GetZ(), GetY() + 1.0f)))
	{
		FaceTarget(runback->x, runback->z);
		ClearRunningLocations();
		GetZone()->movementMgr->DisruptNavigation((Entity*)this);
		if (GetRunbackLocation()->gridid > 0)
			SetLocation(GetRunbackLocation()->gridid);
		AddRunningLocation(runback->x, runback->y, runback->z, GetSpeed(), 0, true, true, "", true);
	}
	else
		GetZone()->movementMgr->NavigateTo((Entity*)this, runback->x, runback->y, runback->z);

	//AddRunningLocation(runback->x, runback->y, runback->z, GetSpeed(), 0, false);	
	last_movement_update = Timer::GetCurrentTime2();
}

void NPC::ClearRunback(){
	safe_delete(runback);
	m_runningBack = false;
	m_runbackHeadingDir1 = m_runbackHeadingDir2 = 0;
	resume_movement = true;
	NeedsToResumeMovement(false);
}

void NPC::StartRunback(bool reset_hp_on_runback)
{
	if(GetRunbackLocation())
		return;

	SetRunbackLocation(GetX(), GetY(), GetZ(), GetLocation(), reset_hp_on_runback);
	m_runbackHeadingDir1 = appearance.pos.Dir1;
	m_runbackHeadingDir2 = appearance.pos.Dir2;
}

bool NPC::PauseMovement(int32 period_of_time_ms)
{
	if(period_of_time_ms < 1)
		period_of_time_ms = 1;

	if(HasMovementLoop() || HasMovementLocations())
		StartRunback();
	
	RunToLocation(GetX(),GetY(),GetZ());
	
	pause_timer.Start(period_of_time_ms, true);

	return true;
}

bool NPC::IsPauseMovementTimerActive()
{
	if(pause_timer.Check())
	{
		pause_timer.Disable();
		m_call_runback = true;
	}
	
	return pause_timer.Enabled();
}

void NPC::InCombat(bool val){
	if (in_combat == val)
		return;

	if(in_combat == false && val && GetZone()){
		LogWrite(NPC__DEBUG, 3, "NPC", "'%s' engaged in combat with '%s'", this->GetName(), ( GetTarget() ) ? GetTarget()->GetName() : "Unknown" );
		GetZone()->CallSpawnScript(this, SPAWN_SCRIPT_ATTACKED, GetTarget());
		SetTempActionState(0); // disable action states in combat
	}
	if(!in_combat && val){
		// if not a pet and no current run back location set then set one to the current location
		bool hadRunback = (GetRunbackLocation() != nullptr);
		if(hadRunback) {
			pause_timer.Disable();
			if(!GetRunbackLocation()->reset_hp_on_runback) // if we aren't resetting hp it isn't a real reset point, just face target swings
				ClearRunback();
		}
		if(!IsPet()) {
			StartRunback(true);
		}
	}

	int8 ruleAutoLockEncounter = rule_manager.GetGlobalRule(R_World, AutoLockEncounter)->GetInt8();
	in_combat = val;
	if(val){
		LogWrite(NPC__DEBUG, 3, "NPC", "'%s' engaged in combat with '%s'", this->GetName(), ( GetTarget() ) ? GetTarget()->GetName() : "Unknown" );
		if(ruleAutoLockEncounter) {
			SetLockedNoLoot(ENCOUNTER_STATE_LOCKED);
		}
		AddIconValue(64);
		// In combat so lets set the NPC's speed to its max speed
		if (GetMaxSpeed() > 0)
			SetSpeed(GetMaxSpeed());
	}
	else{
		SetLockedNoLoot(ENCOUNTER_STATE_AVAILABLE);
		RemoveIconValue(64);
		if (GetHP() > 0){
			SetTempActionState(-1); //re-enable action states on exiting combat
			GetZone()->CallSpawnScript(this, SPAWN_SCRIPT_COMBAT_RESET);
			// Stop all HO's attached to this NPC
			GetZone()->GetSpellProcess()->KillHOBySpawnID(GetID());
		}
	}
	if(!MovementInterrupted() && val && GetSpeed() > 0 && movement_loop.size() > 0){
		CalculateRunningLocation(true);
	}
	MovementInterrupted(val);
}

bool NPC::HandleUse(Client* client, string type){
	if(!client || type.length() == 0 || (appearance.show_command_icon == 0 && appearance.display_hand_icon == 0))
		return false;
	EntityCommand* entity_command = FindEntityCommand(type);
	if (entity_command) {
		client->GetCurrentZone()->ProcessEntityCommand(entity_command, client->GetPlayer(), client->GetPlayer()->GetTarget());
		return true;
	}
	return false;
	/*Spell* spell = master_spell_list.GetSpellByName((char*)type.c_str());
	if(spell)
		client->GetCurrentZone()->ProcessSpell(spell, client->GetPlayer(), client->GetPlayer()->GetTarget());
	else if(GetSpawnScript())
		client->GetCurrentZone()->CallSpawnScript(this, SPAWN_SCRIPT_CUSTOM, client->GetPlayer(), (char*)type.c_str());
	else
		return false;
	return true;*/
}


bool NPC::CheckSameAppearance(string name, int16 id)
{
	// need to iterate through master_appearance_list finding if id contains name
	return true;
}


void NPC::Randomize(NPC* npc, int32 flags) 
{
	int8 random = 0;
	int8 min_val = 0;
	int8 max_val = 255;

	/* We need to check if gender is going to be randomized first because if the race is going to be
	 * randomized, we need to know its gender so we can choose the proper model.
	 * We also need to make sure the model gets set properly if the player chooses to randomize the gender
	 * and not the race. */
	int8 old_gender = npc->GetGender();

	if (flags & RANDOMIZE_GENDER) 
	{
		random = MakeRandomInt(1, 2);
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Gender: %i", random);
		npc->SetGender(random);
	}

	if ((flags & RANDOMIZE_RACE) || (flags & RANDOMIZE_GENDER && old_gender != npc->GetGender()) || (flags & RANDOMIZE_MODEL_TYPE)) 
	{
		string race_string = "";
		int8 gender = npc->GetGender();

		if (gender == 1 || gender == 2) 
		{

			if (flags & RANDOMIZE_RACE) 
			{
				if(npc->GetAlignment() == 1)			// Good
					random = races.GetRaceNameGood();
				else if(npc->GetAlignment() < 0)		// Evil
					random = races.GetRaceNameEvil();
				else									// All
					random = MakeRandomInt(0, 20);
				LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Race: %s (%i)", races.GetRaceNameCase(random), random);
				npc->SetRace(random);
			}

			switch (npc->GetRace()) 
			{
				case BARBARIAN:
					// JA: If randomize has "4" (model) and race=0, barbarians show up in place of the real models.
					// Started working on a solution (CheckSameAppearance) but cannot get it to work yet.
					// Solution for now is to not randomize models for race=0. Set to race=255, or turn off model randomize
					race_string = "/barbarian/barbarian";
					break;
				case DARK_ELF:
					race_string = "/darkelf/darkelf";
					break;
				case DWARF:
					race_string = "/dwarf/dwarf";
					break;
				case ERUDITE:
					race_string = "/erudite/erudite";
					break;
				case FROGLOK:
					race_string = "/froglok/froglok";
					break;
				case GNOME:
					race_string = "/gnome/gnome";
					break;
				case HALF_ELF:
					race_string = "/halfelf/halfelf";
					break;
				case HALFLING:
					race_string = "/halfling/halfling";
					break;
				case HIGH_ELF:
					race_string = "/highelf/highelf";
					break;
				case HUMAN:
					race_string = "/human/human";
					break;
				case IKSAR:
					race_string = "/iksar/iksar";
					break;
				case KERRA:
					race_string = "/kerra/kerra";
					break;
				case OGRE:
					race_string = "/ogre/ogre";
					break;
				case RATONGA:
					race_string = "/ratonga/ratonga";
					break;
				case TROLL:
					race_string = "/troll/troll";
					break;
				case WOOD_ELF:
					race_string = "/woodelf/woodelf";
					break;
				case FAE:
					race_string = "/fae/fae_light";
					break;
				case ARASAI:
					race_string = "/fae/fae_dark";
					break;
				case SARNAK:
					gender == 1 ? race_string = "01/sarnak_male/sarnak" : race_string = "01/sarnak_female/sarnak";
					break;
				case VAMPIRE:
					race_string = "/vampire/vampire";
					break;
				case AERAKYN:
					race_string = "/aerakyn/aerakyn";
					break;
			}

			if (race_string.length() > 0) 
			{
				string gender_string;

				gender == 1 ? gender_string = "male" : gender_string = "female";

				vector<int16>* id_list = database.GetAppearanceIDsLikeName("ec/pc" + race_string + "_" + gender_string);

				if (id_list) 
				{
					int32 index = MakeRandomInt(0, id_list->size() - 1);
					npc->SetModelType(id_list->at(index));
					npc->SetSogaModelType(id_list->at(index));
					LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Model Type: %i", npc->GetModelType());
					int16 wing_type = 0;

					if (npc->GetRace() == FAE) 
					{
						vector<int16>* id_list_wings = database.GetAppearanceIDsLikeName("ec/pc/fae_wings/fae_wing");
						if (id_list_wings) {
							wing_type = id_list_wings->at(MakeRandomInt(0, id_list_wings->size() - 1));
							safe_delete(id_list_wings);
						}
					}
					else if (npc->GetRace() == ARASAI) 
					{
						vector<int16>* id_list_wings = database.GetAppearanceIDsLikeName("ec/pc/fae_wings/fae_d_wing");
						if (id_list_wings) {
							wing_type = id_list_wings->at(MakeRandomInt(0, id_list_wings->size() - 1));
							safe_delete(id_list_wings);
						}
					}
					else if (npc->GetRace() == AERAKYN)
					{
						vector<int16>* id_list_wings = database.GetAppearanceIDsLikeName("ec/pc/aerakyn/aerakyn_male_wings");
						if (id_list_wings) {
							wing_type = id_list_wings->at(MakeRandomInt(0, id_list_wings->size() - 1));
							safe_delete(id_list_wings);
						}
					}
					if (wing_type > 0) 
					{
						EQ2_Color color1;
						EQ2_Color color2;
						color1.red = MakeRandomInt(0, 255);
						color1.green = MakeRandomInt(0, 255);
						color1.blue = MakeRandomInt(0, 255);
						color2.red = MakeRandomInt(0, 255);
						color2.green = MakeRandomInt(0, 255);
						color2.blue = MakeRandomInt(0, 255);
						npc->SetWingColor1(color1);
						npc->SetWingColor2(color2);
					}
					npc->SetWingType(wing_type);
					safe_delete(id_list);
				}
			}
		}
	}

	if (flags & RANDOMIZE_FACIAL_HAIR_TYPE) {
		vector<int16>* id_list = database.GetAppearanceIDsLikeName("accessories/hair/face");
		if (id_list) {
			int32 index = MakeRandomInt(0, id_list->size() - 1);
			npc->SetFacialHairType(id_list->at(index));
			npc->SetSogaFacialHairType(id_list->at(index));
			LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Facial Hair: %i", npc->GetFacialHairType());
			safe_delete(id_list);
		}
	}
	if (flags & RANDOMIZE_HAIR_TYPE) {
		vector<int16>* id_list = database.GetAppearanceIDsLikeName("accessories/hair/hair");
		if (id_list) {
			int32 index = MakeRandomInt(0, id_list->size() - 1);
			npc->SetHairType(id_list->at(index));
			npc->SetSogaHairType(id_list->at(index));
			LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Hair: %i", npc->GetHairType());
			safe_delete(id_list);
		}
	}
	if (flags & RANDOMIZE_WING_TYPE) {
		int16 wing_type = 0;
		if (npc->GetRace() == FAE) {
			vector<int16>* id_list_wings = database.GetAppearanceIDsLikeName("ec/pc/fae_wings/fae_wing");
			if (id_list_wings) {
				wing_type = id_list_wings->at(MakeRandomInt(0, id_list_wings->size() - 1));
				safe_delete(id_list_wings);
			}
		}
		else if (npc->GetRace() == ARASAI) {
			vector<int16>* id_list_wings = database.GetAppearanceIDsLikeName("ec/pc/fae_wings/fae_d_wing");
			if (id_list_wings) {
				wing_type = id_list_wings->at(MakeRandomInt(0, id_list_wings->size() - 1));
				safe_delete(id_list_wings);
			}
		}
		else if (npc->GetRace() == AERAKYN)
		{
			vector<int16>* id_list_wings = database.GetAppearanceIDsLikeName("ec/pc/aerakyn/aerakyn_male_wings");
			if (id_list_wings) {
				wing_type = id_list_wings->at(MakeRandomInt(0, id_list_wings->size() - 1));
				safe_delete(id_list_wings);
			}
		}
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Wing Type: %i", wing_type);
		npc->SetWingType(wing_type);
	}

	if (flags & RANDOMIZE_CHEEK_TYPE) {
		for(int i=0;i<3;i++) {
			random = MakeRandomFloat(-100, 100);
			LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Cheek[%i]: %i", i, random);
			npc->features.cheek_type[i] = random;
		}
	}
	if (flags & RANDOMIZE_CHIN_TYPE) {
		for(int i=0;i<3;i++) {
			random = MakeRandomFloat(-100, 100);
			LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Chin[%i]: %i", i, random);
			npc->features.chin_type[i] = MakeRandomFloat(-100, 100);
		}
	}
	if (flags & RANDOMIZE_EAR_TYPE) {
		for(int i=0;i<3;i++) {
			random = MakeRandomFloat(-100, 100);
			LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Ear[%i]: %i", i, random);
			npc->features.ear_type[i] = MakeRandomFloat(-100, 100);
		}
	}
	if (flags & RANDOMIZE_EYE_BROW_TYPE) {
		for(int i=0;i<3;i++) {
			random = MakeRandomFloat(-100, 100);
			LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Eyebrow[%i]: %i", i, random);
			npc->features.eye_brow_type[i] = MakeRandomFloat(-100, 100);
		}
	}
	if (flags & RANDOMIZE_EYE_TYPE) {
		for(int i=0;i<3;i++) {
			random = MakeRandomFloat(-100, 100);
			LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Eye[%i]: %i", i, random);
			npc->features.eye_type[i] = MakeRandomFloat(-100, 100);
		}
	}
	if (flags & RANDOMIZE_LIP_TYPE) {
		for(int i=0;i<3;i++) {
			random = MakeRandomFloat(-100, 100);
			LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Lip[%i]: %i", i, random);
			npc->features.lip_type[i] = MakeRandomFloat(-100, 100);
		}
	}
	if (flags & RANDOMIZE_NOSE_TYPE) {
		for(int i=0;i<3;i++) {
			random = MakeRandomFloat(-100, 100);
			LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Nose[%i]: %i", i, random);
			npc->features.nose_type[i] = MakeRandomFloat(-100, 100);
		}
	}

	/* Randomize Colors */
	random = MakeRandomInt(0, 255);
	if(random > 30) {
		min_val = random - MakeRandomInt(0, 30);
		max_val = random + MakeRandomInt(0, 30);
	}
	if(max_val > 255)
		max_val = 255;

	LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Color Ranges, random: %i, min: %i, max: %i", random, min_val, max_val);

	if (flags & RANDOMIZE_EYE_COLOR) {
		npc->features.eye_color.red = MakeRandomInt(min_val, max_val);
		npc->features.eye_color.green = MakeRandomInt(min_val, max_val);
		npc->features.eye_color.blue = MakeRandomInt(min_val, max_val);
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Eye Color - R: %i, G: %i, B: %i", npc->features.eye_color.red, npc->features.eye_color.green, npc->features.eye_color.blue);
	}
	if (flags & RANDOMIZE_HAIR_COLOR1) {
		npc->features.hair_color1.red = MakeRandomInt(min_val, max_val);
		npc->features.hair_color1.green = MakeRandomInt(min_val, max_val);
		npc->features.hair_color1.blue = MakeRandomInt(min_val, max_val);
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Hair Color 1 - R: %i, G: %i, B: %i", npc->features.hair_color1.red, npc->features.hair_color1.green, npc->features.hair_color1.blue);
	}
	if (flags & RANDOMIZE_HAIR_COLOR2) {
		npc->features.hair_color2.red = MakeRandomInt(min_val, max_val);
		npc->features.hair_color2.green = MakeRandomInt(min_val, max_val);
		npc->features.hair_color2.blue = MakeRandomInt(min_val, max_val);
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Hair Color 2 - R: %i, G: %i, B: %i", npc->features.hair_color2.red, npc->features.hair_color2.green, npc->features.hair_color2.blue);
	}
	if (flags & RANDOMIZE_HAIR_HIGHLIGHT) {
		npc->features.hair_highlight_color.red = MakeRandomInt(min_val, max_val);
		npc->features.hair_highlight_color.green = MakeRandomInt(min_val, max_val);
		npc->features.hair_highlight_color.blue = MakeRandomInt(min_val, max_val);
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Hair Highlight - R: %i, G: %i, B: %i", npc->features.hair_highlight_color.red, npc->features.hair_highlight_color.green, npc->features.hair_highlight_color.blue);
	}
	if (flags & RANDOMIZE_HAIR_FACE_COLOR) {
		EQ2_Color color1;
		color1.red = MakeRandomInt(min_val, max_val);
		color1.green = MakeRandomInt(min_val, max_val);
		color1.blue = MakeRandomInt(min_val, max_val);
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Facial Hair Color - R: %i, G: %i, B: %i", color1.red, color1.green, color1.blue);
		npc->SetFacialHairColor(color1);
	}
	if (flags & RANDOMIZE_HAIR_FACE_HIGHLIGHT_COLOR) {
		EQ2_Color color1;
		color1.red = MakeRandomInt(min_val, max_val);
		color1.green = MakeRandomInt(min_val, max_val);
		color1.blue = MakeRandomInt(min_val, max_val);
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Facial Hair Highlight - R: %i, G: %i, B: %i", color1.red, color1.green, color1.blue);
		npc->SetFacialHairHighlightColor(color1);
	}
	if (flags & RANDOMIZE_HAIR_TYPE_COLOR) {
		EQ2_Color color1;
		color1.red = MakeRandomInt(min_val, max_val);
		color1.green = MakeRandomInt(min_val, max_val);
		color1.blue = MakeRandomInt(min_val, max_val);
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Hair Type Color - R: %i, G: %i, B: %i", color1.red, color1.green, color1.blue);
		npc->SetHairColor(color1);
	}
	if (flags & RANDOMIZE_HAIR_TYPE_HIGHLIGHT_COLOR) {
		EQ2_Color color1;
		color1.red = MakeRandomInt(min_val, max_val);
		color1.green = MakeRandomInt(min_val, max_val);
		color1.blue = MakeRandomInt(min_val, max_val);
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Hair Type Highlight - R: %i, G: %i, B: %i", color1.red, color1.green, color1.blue);
		npc->SetHairTypeHighlightColor(color1);
	}
	if (flags & RANDOMIZE_SKIN_COLOR) {
		npc->features.skin_color.red = MakeRandomInt(min_val, max_val);
		npc->features.skin_color.green = MakeRandomInt(min_val, max_val);
		npc->features.skin_color.blue = MakeRandomInt(min_val, max_val);
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Skin Color - R: %i, G: %i, B: %i", npc->features.eye_color.red, npc->features.eye_color.green, npc->features.eye_color.blue);
	}
	if (flags & RANDOMIZE_WING_COLOR1) {
		EQ2_Color color1;
		color1.red = MakeRandomInt(min_val, max_val);
		color1.green = MakeRandomInt(min_val, max_val);
		color1.blue = MakeRandomInt(min_val, max_val);
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Wing Color 1 - R: %i, G: %i, B: %i", color1.red, color1.green, color1.blue);
		npc->SetWingColor1(color1);
	}
	if (flags & RANDOMIZE_WING_COLOR2) {
		EQ2_Color color1;
		color1.red = MakeRandomInt(min_val, max_val);
		color1.green = MakeRandomInt(min_val, max_val);
		color1.blue = MakeRandomInt(min_val, max_val);
		LogWrite(NPC__DEBUG, 5, "NPCs", "Randomizing Wing Color 2 - R: %i, G: %i, B: %i", color1.red, color1.green, color1.blue);
		npc->SetWingColor2(color1);
	}
}

Skill* NPC::GetSkillByName(const char* name, bool check_update){
	if(skills && skills->count(name) > 0){
		Skill* ret = (*skills)[name];
		if(ret && check_update && ret->current_val < ret->max_val && (rand()%100) >= 90)
			ret->current_val++;
		return ret;
	}
	return 0;
}

Skill* NPC::GetSkillByID(int32 id, bool check_update){
	Skill* skill = master_skill_list.GetSkill(id);

	if(skill && skills && skills->count(skill->name.data) > 0){
		Skill* ret = (*skills)[skill->name.data];
		if(ret && check_update && ret->current_val < ret->max_val && (rand()%100) >= 90)
			ret->current_val++;
		return ret;
	}
	return 0;
}

int8 NPC::GetAttackType(){
	return attack_type;
}

void NPC::SetAIStrategy(int8 strategy){
	ai_strategy = strategy;
}

int8 NPC::GetAIStrategy(){
	return ai_strategy;
}

void NPC::SetPrimarySpellList(int32 id){
	primary_spell_list = id;
}

int32 NPC::GetPrimarySpellList(){
	return primary_spell_list;
}

void NPC::SetSecondarySpellList(int32 id){
	secondary_spell_list = id;
}

int32 NPC::GetSecondarySpellList(){
	return secondary_spell_list;
}

void NPC::SetPrimarySkillList(int32 id){
	primary_skill_list = id;
}

int32 NPC::GetPrimarySkillList(){
	return primary_skill_list;
}

void NPC::SetSecondarySkillList(int32 id){
	secondary_skill_list = id;
}

int32 NPC::GetSecondarySkillList(){
	return secondary_skill_list;
}

void NPC::SetEquipmentListID(int32 id){
	equipment_list_id = id;
}

int32 NPC::GetEquipmentListID(){
	return equipment_list_id;
}

Spell* NPC::GetNextSpell(Spawn* target, float distance){
	if(!cast_on_aggro_completed) {
		Spell* ret = nullptr;
		Spell* tmpSpell = nullptr;
		vector<NPCSpell*>::iterator itr;
		Spawn* tmpTarget = target;
		for (itr = cast_on_spells[CAST_ON_AGGRO].begin(); itr != cast_on_spells[CAST_ON_AGGRO].end(); itr++) {
			tmpSpell = master_spell_list.GetSpell((*itr)->spell_id, (*itr)->tier);
			
			if(!tmpSpell)
				continue;
			if (tmpSpell->GetSpellData()->friendly_spell > 0) {
				tmpTarget = (Spawn*)this;
			}
			if (tmpSpell->GetSpellData()) {
				SpellEffects* effect = ((Entity*)tmpTarget)->GetSpellEffect(tmpSpell->GetSpellID());
				if (!effect) {
					ret = tmpSpell;
							
					if (tmpSpell->GetSpellData()->friendly_spell > 0) {
						tmpTarget = target;
					}
					break;
				}
			}
			if (tmpSpell->GetSpellData()->friendly_spell > 0) {
				tmpTarget = target;
			}
		}
		
		if(ret) {
			return ret;
		}
		else {
			cast_on_aggro_completed = true;
		}
	}
	
	int8 val = rand()%100;
	if(ai_strategy == AI_STRATEGY_OFFENSIVE){
		if(val >= 20)//80% chance to cast offensive spell if Offensive AI
			return GetNextSpell(distance, AI_STRATEGY_OFFENSIVE);
		return GetNextSpell(distance, AI_STRATEGY_DEFENSIVE);
	}
	else if(ai_strategy == AI_STRATEGY_DEFENSIVE){
		if(val >= 20)//80% chance to cast defensive spell if Defensive AI
			return GetNextSpell(distance, AI_STRATEGY_DEFENSIVE);
		return GetNextSpell(distance, AI_STRATEGY_OFFENSIVE);
	}
	return GetNextSpell(distance, AI_STRATEGY_BALANCED);
}

Spell* NPC::GetNextSpell(float distance, int8 type){
	Spell* ret = 0;
	if(spells){
		if(distance < 0)
			distance = 0;
		Spell* tmpSpell = 0;
		vector<NPCSpell*>::iterator itr;
		for(itr = spells->begin(); itr != spells->end(); itr++){
			// if positive, then say the hp ratio must be GREATER than OR EQUAL TO
			if((*itr)->required_hp_ratio > 0 && (*itr)->required_hp_ratio < 101 && GetIntHPRatio() >= (*itr)->required_hp_ratio)
				continue;
			// if negative, then say the hp ratio must be LESS than OR EQUAL TO
			if((*itr)->required_hp_ratio < 0 && (*itr)->required_hp_ratio > -101 && (-(*itr)->required_hp_ratio) >= GetIntHPRatio())
				continue;
			tmpSpell = master_spell_list.GetSpell((*itr)->spell_id, (*itr)->tier);
			if(!tmpSpell || (type == AI_STRATEGY_OFFENSIVE && tmpSpell->GetSpellData()->friendly_spell > 0))
				continue;
			if (tmpSpell->GetSpellData()->cast_type == SPELL_CAST_TYPE_TOGGLE)
				continue;
			if(type == AI_STRATEGY_DEFENSIVE && tmpSpell->GetSpellData()->friendly_spell == 0)
				continue;
			if(distance <= tmpSpell->GetSpellData()->range && distance >= tmpSpell->GetSpellData()->min_range && GetPower() >= tmpSpell->GetPowerRequired(this)){
				ret = tmpSpell;
				if((rand()%100) >= 70) //30% chance to stop after finding the first match, this will give the appearance of the NPC randomly choosing a spell to cast
					break;
			}
		}
		if(!ret && type != AI_STRATEGY_BALANCED)
			ret = GetNextSpell(distance, AI_STRATEGY_BALANCED); //wasnt able to find a valid match, so find any spell that the NPC has
	}
	return ret;
}

Spell* NPC::GetNextBuffSpell(Spawn* target) {
	if(!target) {
		target = (Spawn*)this;
	}
	
	Spell* ret = 0;
	
	if(!target->IsEntity()) {
		return ret;
	}
	
	if (spells && GetZone()->GetSpellProcess()) {
		Spell* tmpSpell = 0;
		vector<NPCSpell*>::iterator itr;
		for (itr = cast_on_spells[CAST_ON_SPAWN].begin(); itr != cast_on_spells[CAST_ON_SPAWN].end(); itr++) {
			tmpSpell = master_spell_list.GetSpell((*itr)->spell_id, (*itr)->tier);
			if (tmpSpell && tmpSpell->GetSpellData()) {
				SpellEffects* effect = ((Entity*)target)->GetSpellEffect(tmpSpell->GetSpellID());
				if (effect) {
					if (effect->tier < tmpSpell->GetSpellTier()) {
						ret = tmpSpell;
						break;
					}
				}
				else {
					ret = tmpSpell;
					break;
				}
			}
		}
		
		for (itr = spells->begin(); itr != spells->end(); itr++) {
			// if positive, then say the hp ratio must be GREATER than OR EQUAL TO
			if((*itr)->required_hp_ratio > 0 && (*itr)->required_hp_ratio < 101 && GetIntHPRatio() >= (*itr)->required_hp_ratio)
				continue;
			// if negative, then say the hp ratio must be LESS than OR EQUAL TO
			if((*itr)->required_hp_ratio < 0 && (*itr)->required_hp_ratio > -101 && (-(*itr)->required_hp_ratio) >= GetIntHPRatio())
				continue;
			tmpSpell = master_spell_list.GetSpell((*itr)->spell_id, (*itr)->tier);
			if (tmpSpell && tmpSpell->GetSpellData() && tmpSpell->GetSpellData()->cast_type == SPELL_CAST_TYPE_TOGGLE) {
				SpellEffects* effect = ((Entity*)target)->GetSpellEffect(tmpSpell->GetSpellID());
				if (effect) {
					if (effect->tier < tmpSpell->GetSpellTier()) {
						ret = tmpSpell;
						break;
					}
				}
				else {
					ret = tmpSpell;
					break;
				}
			}
		}
	}
	return ret;
}

void NPC::SetAggroRadius(float radius, bool overrideBaseValue){
	if (base_aggro_radius == 0.0f || overrideBaseValue)
		base_aggro_radius = radius;

	aggro_radius = radius;
}

float NPC::GetAggroRadius(){
	return aggro_radius;
}

void NPC::SetCastPercentage(int8 percentage){
	cast_percentage = percentage;
}

int8 NPC::GetCastPercentage(){
	return cast_percentage;
}

void NPC::AddSkillBonus(int32 spell_id, int32 skill_id, float value) {
	if (value != 0) {
		SkillBonus* sb;
		if (skill_bonus_list.count(spell_id) == 0) {
			sb = new SkillBonus;
			sb->spell_id = spell_id;
			skill_bonus_list.Put(spell_id, sb);
		}
		else
			sb = skill_bonus_list.Get(spell_id);
		if (sb->skills[skill_id] == 0) {
			SkillBonusValue* sbv = new SkillBonusValue;
			sbv->skill_id = skill_id;
			sbv->value = value;
			sb->skills[skill_id] = sbv;
			if (skills) {
				map<string, Skill*>::iterator itr;
				for (itr = skills->begin(); itr != skills->end(); itr++) {
					Skill* skill = itr->second;
					if (skill->skill_id == sbv->skill_id) {
						skill->current_val += (int16)sbv->value;
						skill->max_val += (int16)sbv->value;
					}
				}
			}
		}
	}
}
	
void NPC::RemoveSkillBonus(int32 spell_id) {
	if (skill_bonus_list.count(spell_id) > 0) {
		SkillBonus* sb = skill_bonus_list.Get(spell_id);
		skill_bonus_list.erase(spell_id);
		map<int32, SkillBonusValue*>::iterator itr;
		for (itr = sb->skills.begin(); itr != sb->skills.end(); itr++) {
			SkillBonusValue* sbv = itr->second;
			if (skills) {
				map<string, Skill*>::iterator skill_itr;
				for (skill_itr = skills->begin(); skill_itr != skills->end(); skill_itr++) {
					Skill* skill = skill_itr->second;
					if (sbv->skill_id == skill->skill_id) {
						skill->current_val -= (int16)sbv->value;
						skill->max_val -= (int16)sbv->value;
					}
				}
			}
			safe_delete(sbv);
		}
		safe_delete(sb);
	}
}

void NPC::SetBrain(::Brain* brain) {
	// Again, had to use the '::' to refer to the Brain class and not the function defined in the NPC class
	MBrain.writelock(__FUNCTION__, __LINE__);
	// Check to make sure the NPC the brain controls matches this npc
	if (brain && brain->GetBody() != this) {
		LogWrite(NPC_AI__ERROR, 0, "NPC_AI", "Brain body does not match the npc we tried to assign the brain to.");
		MBrain.releasewritelock(__FUNCTION__, __LINE__);
		return;
	}
	
	// Store the old brain in a temp pointer so we can delete it later
	::Brain* old_brain = m_brain;
	// Set the brain for this NPC to the new brain
	m_brain = brain;
	// Release the lock
	MBrain.releasewritelock(__FUNCTION__, __LINE__);
	// Delete the old brain
	safe_delete(old_brain);
}

void NPC::SetZone(ZoneServer* in_zone, int32 version) {
	Spawn::SetZone(in_zone, version);
	if (in_zone){
		GetZone()->SetNPCEquipment(this);
		SetSkills(GetZone()->GetNPCSkills(primary_skill_list, secondary_skill_list));
		SetSpells(world.GetNPCSpells(primary_spell_list, secondary_spell_list));
	}
}
