### Function: GetWis(spawn)

**Description:**
Gets the wisdom of the current spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 of the Spawn's wisdom.

**Example:**

```lua
-- From SpawnScripts/Commonlands/adrakotaadept.lua
function spawn(NPC, Spawn)
    dmgMod = math.floor(GetStr(NPC)/10)
    HPRegenMod = math.floor(GetSta(NPC)/10)
    PwRegenMod = math.floor(GetWis(NPC)/10)
    SetInfoStructUInt(NPC, "override_primary_weapon", 1)        
    SetInfoStructUInt(NPC, "primary_weapon_damage_low", math.floor(45 + dmgMod)) 
    SetInfoStructUInt(NPC, "primary_weapon_damage_high", math.floor(85 + dmgMod))
    SetInfoStructUInt(NPC, "hp_regen_override", 1)  
    SetInfoStructSInt(NPC, "hp_regen", 50 + HPRegenMod)           
    SetInfoStructUInt(NPC, "pw_regen_override", 1)  
    SetInfoStructSInt(NPC, "pw_regen", 25 + PwRegenMod) 
end
```
