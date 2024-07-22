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
#ifndef __NPC_AI_H__
#define __NPC_AI_H__
#include "NPC.h"
#include <vector>
#include <map>

using namespace std;

class Brain {
public:
	Brain(NPC* npc);
	virtual ~Brain();

	/// <summary>The main loop for the brain.  This will do all the AI work</summary>
	virtual void Think();

	/* Timer related functions */

	/// <summary>Gets the time between calls to Think()</summary>
	/// <returns>Time in miliseconds between calls to Think()</returns>
	int16 Tick() { return m_tick; }
	/// <summary>Sets the time between calls to Think()</summary>
	/// <param name="time">Time in miliseconds</param>
	void SetTick(int16 time) { m_tick = time; }
	/// <summary>Gets the timestamp of the last call to Think()</summary>
	/// <returns>Timestamp of the last call to Think()</returns>
	int32 LastTick() { return m_lastTick; }
	/// <summary>Sets the last tick to the given time</summary>
	/// <param name="time">The time to set the last tick to</param>
	void SetLastTick(int32 time) { m_lastTick = time; }

	/* Hate related functions */

	/// <summary>Gets the amount of hate this npc has towards the given entity</summary>
	/// <param name="entity">The entity to check</param>
	/// <returns>The amount of hate towards the given entity</returns>
	sint32 GetHate(Entity* entity);
	/// <summary>Add hate for the given entity to this NPC</summary>
	/// <param name="entity">The entity we are adding to this NPC's hate list</param>
	/// <param name="hate">The amount of hate to add</param>
	virtual void AddHate(Entity* entity, sint32 hate);
	/// <summary>Completely clears the hate list for this npc</summary>
	void ClearHate();
	/// <summary>Removes the given entity from this NPC's hate list</summary>
	/// <param name="entity">Entity to remove from this NPC's hate list</param>
	void ClearHate(Entity* entity);
	/// <summary>Get the entity this NPC hates the most</summary>
	/// <returns>The entity this NPC hates the most</returns>
	Entity* GetMostHated();
	/// <summary>Gets a percentage of hate owned by the given entity</summary>
	/// <param name="entity">Entity to get the percentage for</param>
	/// <returns>Percentage of hate as a sint8</returns>
	sint8 GetHatePercentage(Entity* entity);

	void SendHateList(Client* client);
	
	///<summary>Gets a list of all the entities in the hate list</summary>
	vector<Entity*>* GetHateList();

	/* Combat related functions */
	
	bool BrainCastSpell(Spell* spell, Spawn* cast_on, bool calculate_run_loc = true);
	
	/// <summary></summary>
	/// <param name=""></param>
	/// <param name=""></param>
	virtual bool ProcessSpell(Entity* target, float distance);
	/// <summary></summary>
	/// <returns>True if a buff starts casting</returns>
	bool CheckBuffs();
	
	/// <summary>Has the NPC make a melee attack</summary>
	/// <param name="target">The target to attack</param>
	/// <param name="distance">The current distance from the target</param>
	void ProcessMelee(Entity* target, float distance);

	/* Encounter related functions */

	/// <summary>Adds the given entity and its group and raid members to the encounter list</summary>
	/// <param name="entity">Entity we are adding to the encounter list</param>
	void AddToEncounter(Entity* entity);
	/// <summary>Checks to see if the given entity can loot the corpse</summary>
	/// <param name="entity">Entity trying to loot</param>
	/// <returns>True if the entity can loot</returns>
	bool CheckLootAllowed(Entity* entity);
	/// <summary>Gets the size of the encounter list</summary>
	/// <returns>The size of the list as an int8</returns>
	int8 GetEncounterSize();
	/// <summary>Clears the encounter list</summary>
	void ClearEncounter();
	
	void SendEncounterList(Client* client);
	
	/// <summary>Gets a copy of the encounter list</summary>
	/// <returns>A copy of the encounter list as a vector<Entity*>*</returns>
	vector<int32>* GetEncounter();
	/// <summary>Checks to see if a player is in the encounter</summary>
	/// <returns>True if the encounter list contains a player</returns>
	bool PlayerInEncounter() { return m_playerInEncounter; }
	bool IsPlayerInEncounter(int32 char_id);
	bool IsEntityInEncounter(int32 id, bool skip_read_lock = false);
	int32 CountPlayerBotInEncounter();
	bool AddToEncounter(int32 id);
	/* Helper functions*/

	/// <summary>Gets the NPC this brain controls</summary>
	/// <returns>The NPC this brain controls</returns>
	NPC* GetBody() { return m_body; }
	/// <summary>Checks to see if the NPC can cast</summary>
	/// <returns>True if the NPC can cast</returns>
	bool HasRecovered();
	/// <summary>Tells the NPC to move closer to the given target</summary>
	/// <param name="target">The target to move closer to</param>
	void MoveCloser(Spawn* target);

protected:
	// m_body = the npc this brain controls
	NPC*					m_body;
	// m_spellRecovery = time stamp for when the npc can cast again
	int32					m_spellRecovery;
private:
	// MHateList = mutex to lock and unlock the hate list
	Mutex					MHateList;
	// m_hatelist = the list that stores all the hate,
	// entity is the entity this npc hates and the int32 is the value for how much we hate the entity
	map<int32, sint32>		m_hatelist;
	// m_lastTick = the last time we ran this brain
	int32					m_lastTick;
	// m_tick = the amount of time between Think() calls in milliseconds
	int16					m_tick;
	// m_encounter = list of players (entities) that will get a reward (xp/loot) for killing this npc
	vector<int32>			m_encounter;
	map<int32, int32>		m_encounter_playerlist;

	// MEncounter = mutex to lock and unlock the encounter list
	Mutex					MEncounter;
	//m_playerInEncounter = true if a player is added to the encounter
	bool					m_playerInEncounter;
};

// Extension of the default brain for combat pets
class CombatPetBrain : public Brain {
public:
	CombatPetBrain(NPC* body);
	virtual ~CombatPetBrain();
	void Think();
};

class NonCombatPetBrain : public Brain {
public:
	NonCombatPetBrain(NPC* body);
	virtual ~NonCombatPetBrain();
	void Think();
};

class BlankBrain : public Brain {
public:
	BlankBrain(NPC* body);
	virtual ~BlankBrain();
	void Think();
};

class LuaBrain : public Brain {
public:
	LuaBrain(NPC* body);
	virtual ~LuaBrain();
	void Think();
};

class DumbFirePetBrain : public Brain {
public:
	DumbFirePetBrain(NPC* body, Entity* target, int32 expire_time);
	virtual ~DumbFirePetBrain();
	void Think();
	void AddHate(Entity* entity, sint32 hate);
private:
	int32 m_expireTime;
};
#endif
