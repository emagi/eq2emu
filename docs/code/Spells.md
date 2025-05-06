# File: `Spells.h`

## Classes

- `LUAData`
- `SpellScriptTimer`
- `LevelArray`
- `SpellDisplayEffect`
- `SpellData`
- `Spell`
- `MasterSpellList`

## Functions

- `void AddSpellLevel(int8 adventure_class, int8 tradeskill_class, int16 level, float classic_spell_level);`
- `void AddSpellEffect(int8 percentage, int8 subbullet, string description);`
- `void AddSpellLuaData(int8 type, int int_value, int int_value2, float float_value, float float_value2, bool bool_value, string string_value,string string_value2, string helper);`
- `void AddSpellLuaDataInt(int value, int value2, string helper);`
- `void AddSpellLuaDataFloat(float value, float value2, string helper);`
- `void AddSpellLuaDataBool(bool value, string helper);`
- `void AddSpellLuaDataString(string value, string value2, string helper);`
- `int32 GetSpellID();`
- `sint16 TranslateClientSpellIcon(int16 version);`
- `void SetPacketInformation(PacketStruct* packet, Client* client = 0, bool display_tier = false);`
- `void SetAAPacketInformation(PacketStruct* packet, AltAdvanceData* data, Client* client = 0, bool display_tier = false);`
- `void AppendLevelInformation(PacketStruct* packet);`
- `int8 GetSpellTier();`
- `int32 GetSpellDuration();`
- `int16 GetSpellIcon();`
- `int16 GetSpellIconBackdrop();`
- `int16 GetSpellIconHeroicOp();`
- `int16 GetLevelRequired(Player* player);`
- `int16 GetHPRequired(Spawn* spawn);`
- `int16 GetPowerRequired(Spawn* spawn);`
- `int16 GetSavageryRequired(Spawn* spawn);`
- `int16 GetDissonanceRequired(Spawn* spawn);`
- `bool GetSpellData(lua_State* state, std::string field);`
- `bool SetSpellData(lua_State* state, std::string field, int8 fieldArg);`
- `bool ScribeAllowed(Player* player);`
- `bool IsHealSpell();`
- `bool IsBuffSpell();`
- `bool IsDamageSpell();`
- `bool IsControlSpell();`
- `bool IsOffenseSpell();`
- `bool IsCopiedSpell();`
- `void ModifyCastTime(Entity* caster);`
- `int32 CalculateRecastTimer(Entity* caster, float override_timer = 0.0f);`
- `bool CastWhileStunned();`
- `bool CastWhileMezzed();`
- `bool CastWhileStifled();`
- `bool CastWhileFeared();`
- `bool GetStayLocked() { return stay_locked; }`
- `void StayLocked(bool val) { stay_locked = val; }`
- `void DestroySpells();`
- `void Reload();`
- `void AddSpell(int32 id, int8 tier, Spell* spell);`
- `int16 GetSpellErrorValue(int16 version, int8 error_index);`
- `void AddSpellError(int16 version, int8 error_index, int16 error_value);`
- `int32 GetNewMaxSpellID() {`
- `int16 GetClosestVersion(int16 version);`

## Notable Comments

- /*
- */
- // Spell type is for AI so code knows what a spell is
- //vector<SpellDisplayEffect*> effects;
- /// <summary>Gets the correct spell error value for the given version</summary>
- /// <param name='version'>Client version</param>
- /// <param name='error_index'>ID of the error</param>
- /// <returns>The int16 value for the given error and version</returns>
- /// <summary>Adds a spell error to the list</summary>
- /// <param name='version'>Client version for the error</param>
