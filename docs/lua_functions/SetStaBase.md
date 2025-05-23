### Function: SetStaBase(spawn, value)

**Description:**
Set's the Spawn's base stamina to the value provided.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `value` (int32) - Integer value `value`.

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
