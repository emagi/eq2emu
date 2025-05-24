### Function: GetSta(spawn)

**Description:**
Gets the current stamina of the spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 value of the stamina for the spawn.

**Example:**

```lua
-- From SpawnScripts/Antonica/Anguis.lua
function spawn(NPC)
    dmgMod = math.floor(GetStr(NPC)/10)
    HPRegenMod = math.floor(GetSta(NPC)/10)
    PwRegenMod = math.floor(GetStr(NPC)/10)
    SetInfoStructUInt(NPC, "override_primary_weapon", 1)        
    SetInfoStructUInt(NPC, "primary_weapon_damage_low", math.floor(205 + dmgMod)) 
    SetInfoStructUInt(NPC, "primary_weapon_damage_high", math.floor(355 + dmgMod))
    SetInfoStructUInt(NPC, "hp_regen_override", 1)  
    SetInfoStructSInt(NPC, "hp_regen", 125 + HPRegenMod)           
    SetInfoStructUInt(NPC, "pw_regen_override", 1)  
    SetInfoStructSInt(NPC, "pw_regen", 75 + PwRegenMod) 

end
```
