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
#ifndef __EQ2_NPC__
#define __EQ2_NPC__
#include <atomic>
#include "Entity.h"
#include "MutexMap.h"

#define AI_STRATEGY_BALANCED	1
#define AI_STRATEGY_OFFENSIVE	2
#define AI_STRATEGY_DEFENSIVE	3


// Randomize Appearances
#define RANDOMIZE_GENDER			1
#define RANDOMIZE_RACE				2
#define RANDOMIZE_MODEL_TYPE		4

// Randomize appearance id (spawn_npcs table values)
#define RANDOMIZE_FACIAL_HAIR_TYPE	8	// was RANDOMIZE_FACIAL_HAIR
#define RANDOMIZE_HAIR_TYPE			16	// was RANDOMIZE_HAIR
//#define RANDOMIZE_LEGS_TYPE			32  // spare!
#define RANDOMIZE_WING_TYPE			64

// Randomize parameters (npc_appearances, sInt values)
#define RANDOMIZE_CHEEK_TYPE		128
#define RANDOMIZE_CHIN_TYPE			256
#define RANDOMIZE_EAR_TYPE			512
#define RANDOMIZE_EYE_BROW_TYPE		1024
#define RANDOMIZE_EYE_TYPE			2048
#define RANDOMIZE_LIP_TYPE			4096
#define RANDOMIZE_NOSE_TYPE			8192

// Randomize colors/hues (npc_appearances, RGB values)
#define RANDOMIZE_EYE_COLOR					16384
#define RANDOMIZE_HAIR_COLOR1				32768
#define RANDOMIZE_HAIR_COLOR2				65536
#define RANDOMIZE_HAIR_HIGHLIGHT			131072
#define RANDOMIZE_HAIR_FACE_COLOR			262144 // was RANDOMIZE_FACIAL_HAIR_COLOR
#define RANDOMIZE_HAIR_FACE_HIGHLIGHT_COLOR	524288
#define RANDOMIZE_HAIR_TYPE_COLOR			1048576 // was RANDOMIZE_HAIR_COLOR
#define RANDOMIZE_HAIR_TYPE_HIGHLIGHT_COLOR	2097152
#define RANDOMIZE_SKIN_COLOR				4194304
#define RANDOMIZE_WING_COLOR1				8388608
#define RANDOMIZE_WING_COLOR2				16777216

// All Flags On: 33554431

#define PET_TYPE_COMBAT		1
#define PET_TYPE_CHARMED	2
#define PET_TYPE_DEITY		3
#define PET_TYPE_COSMETIC	4
#define PET_TYPE_DUMBFIRE	5

enum CAST_TYPE {
	CAST_ON_SPAWN=0,
	CAST_ON_AGGRO=1,
	MAX_CAST_TYPES=2
};
class Brain;

class NPCSpell {
public:
	NPCSpell() {
		
	}
	
	NPCSpell(NPCSpell* inherit) {
			list_id = inherit->list_id;
			spell_id = inherit->spell_id;
			tier = inherit->tier;
			cast_on_spawn = inherit->cast_on_spawn;
			cast_on_initial_aggro = inherit->cast_on_initial_aggro;
			required_hp_ratio = inherit->required_hp_ratio;
	}
	
	int32 	list_id;
	int32 	spell_id;
	int8 	tier;
	bool	cast_on_spawn;
	bool	cast_on_initial_aggro;
	sint8	required_hp_ratio;
};

class NPC : public Entity {
public:
	NPC();
	NPC(NPC* old_npc);
	virtual ~NPC();
	void	Initialize();
	EQ2Packet* serialize(Player* player, int16 version);
	void	SetAppearanceID(int32 id){ appearance_id = id; }
	int32	GetAppearanceID(){ return appearance_id; }
	bool	IsNPC(){ return true; }
	void	StartRunback(bool reset_hp_on_runback = false);
	void	InCombat(bool val);
	bool	HandleUse(Client* client, string type);
	void	SetRandomize(int32 value) {appearance.randomize = value;}
	void	AddRandomize(sint32 value) {appearance.randomize += value;}
	int32	GetRandomize() {return appearance.randomize;}
	bool	CheckSameAppearance(string name, int16 id);
	void	Randomize(NPC* npc, int32 flags);
	Skill*	GetSkillByName(const char* name, bool check_update = false);
	Skill*	GetSkillByID(int32 id, bool check_update = false);
	int8	GetAttackType();
	void	SetAIStrategy(int8 strategy);
	int8	GetAIStrategy();
	void	SetPrimarySpellList(int32 id);
	int32	GetPrimarySpellList();
	void	SetSecondarySpellList(int32 id);
	int32	GetSecondarySpellList();
	void	SetPrimarySkillList(int32 id);
	int32	GetPrimarySkillList();
	void	SetSecondarySkillList(int32 id);
	int32	GetSecondarySkillList();
	void	SetEquipmentListID(int32 id);
	int32	GetEquipmentListID();
	Spell*	GetNextSpell(Spawn* target, float distance);
	virtual Spell*	GetNextBuffSpell(Spawn* target = 0);
	void	SetAggroRadius(float radius, bool overrideBaseValue = false);
	float	GetAggroRadius();
	float	GetBaseAggroRadius() { return base_aggro_radius; }
	void	SetCastPercentage(int8 percentage);
	int8	GetCastPercentage();
	void	SetSkills(map<string, Skill*>* in_skills);
	void	SetSpells(vector<NPCSpell*>* in_spells);
	void	SetRunbackLocation(float x, float y, float z, int32 gridid, bool set_hp_runback = false);
	MovementLocation* GetRunbackLocation();
	float	GetRunbackDistance();
	void	Runback(float distance=0.0f, bool stopFollowing = true);
	void	ClearRunback();
	
	virtual bool PauseMovement(int32 period_of_time_ms);
	virtual bool IsPauseMovementTimerActive();
	
	void	AddSkillBonus(int32 spell_id, int32 skill_id, float value);
	virtual void RemoveSkillBonus(int32 spell_id);
	virtual void SetZone(ZoneServer* zone, int32 version=0);

	void	SetMaxPetLevel(int8 val) { m_petMaxLevel = val; }
	int8	GetMaxPetLevel() { return m_petMaxLevel; }

	void ProcessCombat();

	/// <summary>Sets the brain this NPC should use</summary>
	/// <param name="brain">The brain this npc should use</param>
	void SetBrain(Brain* brain);
	/// <summary>Gets the current brain this NPC uses</summary>
	/// <returns>The Brain this NPC uses</returns>
	::Brain* Brain() { return m_brain; }
	bool m_runningBack;
	sint16 m_runbackHeadingDir1;
	sint16 m_runbackHeadingDir2;

	int32 GetShardID() { return m_ShardID; }
	void SetShardID(int32 shardid) { m_ShardID = shardid; }

	int32 GetShardCharID() { return m_ShardCharID; }
	void SetShardCharID(int32 charid) { m_ShardCharID = charid; }

	sint64 GetShardCreatedTimestamp() { return m_ShardCreatedTimestamp; }
	void SetShardCreatedTimestamp(sint64 timestamp) { m_ShardCreatedTimestamp = timestamp; }
	
	bool HasSpells() { return has_spells; }
	
	std::atomic<bool> m_call_runback;
	std::atomic<bool> cast_on_aggro_completed;
private:
	MovementLocation* runback;
	int8	cast_percentage;
	float	aggro_radius;
	float	base_aggro_radius;
	Spell*	GetNextSpell(float distance, int8 type);
	map<string, Skill*>* skills;
	vector<NPCSpell*>* spells;
	vector<NPCSpell*> cast_on_spells[CAST_TYPE::MAX_CAST_TYPES];
	int32	primary_spell_list;
	int32	secondary_spell_list;
	int32	primary_skill_list;
	int32	secondary_skill_list;
	int32	equipment_list_id;
	int8	attack_type;
	int8	ai_strategy;
	int32	appearance_id;
	int32	npc_id;
	MutexMap<int32, SkillBonus*> skill_bonus_list;
	int8	m_petMaxLevel;

	// Because I named the get function Brain() as well we need to use '::' to specify we are refering to
	// the brain class and not the function defined above
	::Brain*	m_brain;
	Mutex		MBrain;

	int32		m_ShardID;
	int32		m_ShardCharID;
	sint64		m_ShardCreatedTimestamp;
	bool		has_spells;
};
#endif

