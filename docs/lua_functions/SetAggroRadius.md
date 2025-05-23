### Function: SetAggroRadius(spawn, distance, override_)

**Description:**
Sets the aggro radius of the Spawn(NPC).  The override_ flag will set the base aggro radius to the same value.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `distance` (int32) - Distance `distance`.
- `override_` (int32) - Integer value `override_`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Commonlands/aMilitiaGuard.lua
function spawn(NPC)
    NPCModule(NPC, Spawn)  
    SetAggroRadius(NPC, 20)
    AddTimer(NPC, 180000, "despawn", 1)
end
```
