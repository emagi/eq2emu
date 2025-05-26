### Function: SetInfoStructSInt(spawn, field, value)

**Description:**
Sets the signed integer field to the value provided.  See https://github.com/emagi/eq2emu/blob/main/docs/data_types/info_struct.md for field types.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `field` (string) - String `field`.
- `value` (int64) - Integer value `value`.

**Returns:** None.

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
