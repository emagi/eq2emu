### Function: ModifyPower(spawn, value)

**Description:**
Sets the Spawn's Power to the value if the value plus the current power is greater than the Spawn's Total Power.  If the current power plus the is less than the total power, then it will restore the current power up to the value. 


**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
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
