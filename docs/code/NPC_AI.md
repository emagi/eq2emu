# File: `NPC_AI.h`

## Classes

- `Brain`
- `CombatPetBrain`
- `NonCombatPetBrain`
- `BlankBrain`
- `LuaBrain`
- `DumbFirePetBrain`

## Functions

- `int16 Tick() { return m_tick; }`
- `void SetTick(int16 time) { m_tick = time; }`
- `int32 LastTick() { return m_lastTick; }`
- `void SetLastTick(int32 time) { m_lastTick = time; }`
- `sint32 GetHate(Entity* entity);`
- `void ClearHate();`
- `void ClearHate(Entity* entity);`
- `sint8 GetHatePercentage(Entity* entity);`
- `void SendHateList(Client* client);`
- `bool BrainCastSpell(Spell* spell, Spawn* cast_on, bool calculate_run_loc = true);`
- `bool CheckBuffs();`
- `void ProcessMelee(Entity* target, float distance);`
- `void AddToEncounter(Entity* entity);`
- `bool CheckLootAllowed(Entity* entity);`
- `int8 GetEncounterSize();`
- `void ClearEncounter();`
- `void SendEncounterList(Client* client);`
- `bool PlayerInEncounter() { return m_playerInEncounter; }`
- `bool IsPlayerInEncounter(int32 char_id);`
- `bool IsEntityInEncounter(int32 id, bool skip_read_lock = false);`
- `int32 CountPlayerBotInEncounter();`
- `bool AddToEncounter(int32 id);`
- `bool HasRecovered();`
- `void MoveCloser(Spawn* target);`
- `void Think();`
- `void Think();`
- `void Think();`
- `void Think();`
- `void Think();`
- `void AddHate(Entity* entity, sint32 hate);`

## Notable Comments

- /*
- */
- /// <summary>The main loop for the brain.  This will do all the AI work</summary>
- /* Timer related functions */
- /// <summary>Gets the time between calls to Think()</summary>
- /// <returns>Time in miliseconds between calls to Think()</returns>
- /// <summary>Sets the time between calls to Think()</summary>
- /// <param name="time">Time in miliseconds</param>
- /// <summary>Gets the timestamp of the last call to Think()</summary>
- /// <returns>Timestamp of the last call to Think()</returns>
