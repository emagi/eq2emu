# File: `SpellProcess.h`

## Classes

- `InterruptStruct`
- `CastTimer`
- `CastSpell`
- `RecastTimer`
- `SpellProcess`

## Functions

- `void RemoveCaster(Spawn* caster, bool lock_spell_process = true);`
- `void RemoveAllSpells(bool reload_spells = false);`
- `void Process();`
- `void Interrupted(Entity* caster, Spawn* interruptor, int16 error_code, bool cancel = false, bool from_movement = false);`
- `void ProcessSpell(ZoneServer* zone, Spell* spell, Entity* caster, Spawn* target = 0, bool lock = true, bool harvest_spell = false, LuaSpell* customSpell = 0, int16 custom_cast_time = 0, bool in_heroic_opp = false);`
- `void ProcessEntityCommand(ZoneServer* zone, EntityCommand* entity_command, Entity* caster, Spawn* target, bool lock = true, bool in_heroic_opp = false);`
- `bool TakePower(LuaSpell* spell, int32 custom_power_req = 0);`
- `bool CheckPower(LuaSpell* spell);`
- `bool TakeHP(LuaSpell* spell, int32 custom_hp_req = 0);`
- `bool CheckHP(LuaSpell* spell);`
- `bool CheckConcentration(LuaSpell* spell);`
- `bool CheckSavagery(LuaSpell* spell);`
- `bool TakeSavagery(LuaSpell* spell);`
- `bool CheckDissonance(LuaSpell* spell);`
- `bool AddDissonance(LuaSpell* spell);`
- `bool AddConcentration(LuaSpell* spell);`
- `bool CastProcessedSpell(LuaSpell* spell, bool passive = false, bool in_heroic_opp = false);`
- `bool CastProcessedEntityCommand(EntityCommand* entity_command, Client* client, Spawn* target, bool in_heroic_opp = false);`
- `void SendStartCast(LuaSpell* spell, Client* client);`
- `void SendFinishedCast(LuaSpell* spell, Client* client);`
- `void LockAllSpells(Client* client);`
- `void UnlockAllSpells(Client* client, Spell* exception = 0);`
- `void UnlockSpell(Client* client, Spell* spell);`
- `bool DeleteCasterSpell(Spawn* caster, Spell* spell, string reason = "");`
- `bool DeleteCasterSpell(LuaSpell* spell, string reason="", bool removing_all_spells = false, Spawn* remove_target = nullptr, bool zone_shutting_down = false, bool shared_lock_spell = true);`
- `void CheckInterrupt(InterruptStruct* interrupt);`
- `void RemoveSpellTimersFromSpawn(Spawn* spawn, bool remove_all = false, bool delete_recast = false, bool call_expire_function = true, bool lock_spell_process = false);`
- `void CheckRecast(Spell* spell, Entity* caster, float timer_override = 0, bool check_linked_timers = true);`
- `void AddSpellToQueue(Spell* spell, Entity* caster);`
- `void RemoveSpellFromQueue(Spell* spell, Entity* caster, bool send_update = true);`
- `void RemoveSpellFromQueue(Entity* caster, bool hostile_only = false);`
- `bool CheckSpellQueue(Spell* spell, Entity* caster);`
- `bool IsReady(Spell* spell, Entity* caster);`
- `void SendSpellBookUpdate(Client* client);`
- `bool CastPassives(Spell* spell, Entity* caster, bool remove = false);`
- `bool CastInstant(Spell* spell, Entity* caster, Entity* target, bool remove = false, bool passive=false);`
- `void AddSpellScriptTimer(SpellScriptTimer* timer);`
- `void RemoveSpellScriptTimer(SpellScriptTimer* timer, bool locked=false);`
- `void RemoveSpellScriptTimerBySpell(LuaSpell* spell, bool clearPendingDeletes=true);`
- `void CheckSpellScriptTimers();`
- `bool SpellScriptTimersHasSpell(LuaSpell* spell);`
- `std::string SpellScriptTimerCustomFunction(LuaSpell* spell);`
- `void ClearSpellScriptTimerList();`
- `void RemoveTargetFromSpell(LuaSpell* spell, Spawn* target, bool remove_caster = false);`
- `void CheckRemoveTargetFromSpell(LuaSpell* spell, bool allow_delete = true, bool removing_all_spells = false);`
- `void RemoveTargetList(LuaSpell* spell);`
- `bool AddHO(Client* client, HeroicOP* ho);`
- `bool AddHO(int32 group_id, HeroicOP* ho);`
- `void KillHOBySpawnID(int32 spawn_id);`
- `void AddSpellCancel(LuaSpell* spell);`
- `void DeleteSpell(LuaSpell* spell);`
- `void SpellCannotStack(ZoneServer* zone, Client* client, Entity* caster, LuaSpell* lua_spell, LuaSpell* conflictSpell);`
- `bool ProcessSpell(LuaSpell* spell, bool first_cast = true, const char* function = 0, SpellScriptTimer* timer = 0, bool all_targets = false);`
- `std::string ApplyLuaFunction(LuaSpell* spell, bool first_cast, const char* function, SpellScriptTimer* timer, Spawn* altTarget = 0);`
- `void AddActiveSpell(LuaSpell* spell);`
- `void DeleteActiveSpell(LuaSpell* spell, bool skipRemoveCurrent = false);`

## Notable Comments

- /*
- */
- /// <summary> Handles all spell casts for a zone, only 1 SpellProcess per zone </summary>
- /// Remove dead pointers for casters when the Spawn is deconstructed
- /// <summary>Remove all spells from the SpellProcess </summary>
- /// <summary>Main loop, handles everything (interupts, cast time, recast, ...) </summary>
- /// <summary>Interrupts the caster (creates the InterruptStruct and adds it to a list)</summary>
- /// <param name='caster'>Entity being interrupted</param>
- /// <param name='interruptor'>Spawn that interrupted the caster</param>
- /// <param name='error_code'>The error code</param>
