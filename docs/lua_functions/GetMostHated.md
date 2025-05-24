### Function: GetMostHated(NPC)

**Description:**
Retrieves the spawn that currently has the highest hate (aggro) on the specified NPC’s hate list. This is usually the NPC’s current primary target in combat.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Example:**

```lua
-- From SpawnScripts/A Meeting of the Minds/Borxx.lua
function borxxConvo5(NPC, Spawn)
    local overlord = GetSpawn(NPC, 5560003)
    local hated = GetMostHated(overlord)
    local braxx = GetSpawn(NPC, 5560004)
    local brixx = GetSpawn(NPC, 5560005)
    FaceTarget(NPC, overlord)   
    Say(NPC, "So be it.")
    Attack(NPC, hated)
    Attack(braxx, hated)
    Attack(brixx, hated)
end
```
