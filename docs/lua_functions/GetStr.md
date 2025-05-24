### Function: GetStr(spawn)

**Description:**
Gets the strength of the spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 value of the spawn's current strength.

**Example:**

```lua
-- From SpawnScripts/ADecrepitCrypt/KyrinSteelbone.lua
function spawn(NPC, Spawn)
    Named(NPC, Spawn)
    dmgMod = GetStr(NPC)/10
    SetInfoStructUInt(NPC, "override_primary_weapon", 1)        
    SetInfoStructUInt(NPC, "primary_weapon_damage_low", math.floor(24 + dmgMod)) 
    SetInfoStructUInt(NPC, "primary_weapon_damage_high", math.floor(42 + dmgMod))
end
```
