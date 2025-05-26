### Function: IsHeroic(spawn)

**Description:**
Return's true if Spawn is heroic.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** True if Spawn is heroic otherwise False.

**Example:**

```lua
-- From SpawnScripts/Antonica/adankfurgnoll.lua
function waypoints(NPC)
    if IsHeroic(NPC) == false then
        RandomMovement(NPC, Spawn, 8, -8, 2, 8, 15)
    end
```
