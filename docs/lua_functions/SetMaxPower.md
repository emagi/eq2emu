### Function: SetMaxPower(spawn, value)

**Description:**
Set's the Max Power of the Spawn (As a Spell Bonus) to the new value.

**Parameters:**
- `spawn` (Spawn) - Spawn object reference `spawn`.
- `value` (int32) - Integer value `value`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Commonlands/TerraThud.lua
function spawn(NPC, Spawn)
    local Level = GetLevel(NPC)

    if Level == 21 then
        SetMaxHP(NPC, 6885)
        ModifyHP(NPC, 6885)
        SetMaxPower(NPC, 1650)
        ModifyPower(NPC, 1650)
    elseif Level == 22 then
        SetMaxHP(NPC, 7500)
        ModifyHP(NPC, 7500)
        SetMaxPower(NPC, 1750)
        ModifyPower(NPC, 1750)
    end
```
