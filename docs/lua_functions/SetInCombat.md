### Function: SetInCombat(spawn, val)

**Description:**
Set's if the Spawn is in combat.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `val` (bool) - Boolean flag `val`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/BrawlersDojo/afirstcircleadept.lua
function aggro(NPC,Spawn)
    if GetTempVariable(NPC,"Reset")== nil then
    else
        ClearHate(NPC, Spawn)
        SetInCombat(Spawn, false)
        SetInCombat(NPC, false)
        ClearEncounter(NPC)
        SetTarget(Spawn,nil)
    end
```
