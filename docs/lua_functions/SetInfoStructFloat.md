### Function: SetInfoStructFloat(spawn, field, value)

**Description:**
Sets the float field to the value provided.  See https://github.com/emagi/eq2emu/blob/main/docs/data_types/info_struct.md for field types.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `field` (string) - String `field`.
- `value` (float) - Float value `value`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Generic/CombatModule.lua
function attributes(NPC, Spawn)
    -- Calculate attributes
    if  level <= 4 then
        baseStat = 19 else
            baseStat = level + 15
    end
```
