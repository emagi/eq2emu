### Function: SetWisBase(spawn, value)

**Description:**
Set's the Spawn's base wisdom to the value provided.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `value` (int32) - Integer value `value`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Generic/CombatModule.lua
function regen(NPC, Spawn)
    
    -- In-combat health regeneration
    SetInfoStructUInt(NPC, "hp_regen_override", 1)  -- Set to  0 to disable and allow the server to set the regen rate.
    SetInfoStructSInt(NPC, "hp_regen", 0)           -- Set Regen Amount. Default 0
    
    -- In-combat power regeneration
    SetInfoStructUInt(NPC, "pw_regen_override", 1)  -- Set to  0 to disable and allow the server to set the regen rate.
    SetInfoStructSInt(NPC, "pw_regen", 0)           -- Set Regen Amount. Default 0
    
end
```
