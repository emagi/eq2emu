### Function: SpawnSetByDistance(spawn, max_distance, variable, value)

**Description:**
Set a spawn setting on one or more spawns in a radius of the source.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `max_distance` (int32) - Distance `max_distance`.
- `variable` (int32) - Integer value `variable`.
- `value` (int32) - Integer value `value`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/OutpostOverlord/evilgoblintent1.lua
function casted_on(NPC, Spawn, SpellName)
    if SpellName == "burn tent" then
        if CheckTent(Spawn, NPC) == true then
            if GetQuestStep(Spawn, TheFinalAssault) == 2 then
                SpawnSetByDistance(NPC, 15, "visual_state", 491)
                KillSpawnByDistance(NPC, 15, 0, 1)
                AddStepProgress(Spawn, TheFinalAssault, 2, 1)
                    BurnTent(Spawn, NPC)
            elseif GetQuestStep(Spawn, TheFinalAssault) == 3 then
                SetStepComlete(Spawn, TheFinalAssault, 2)
            else
            end
```
