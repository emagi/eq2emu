### Function: SpawnGroupByID(zone, group_id, custom_level)

**Description:**
Spawn by group_id in the current zone, custom_level can be provided to hard set all spawns to the same level.

**Parameters:**
- `zone` (Zone) - Zone object representing `zone`.
- `group_id` (uint32) - Integer value `group_id`.
- `custom_level` (uint32) - Integer value `custom_level`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/IsleRefuge1/GoblinSaboteurFirepit.lua
function InRange(NPC,Spawn)
    if GetQuestStep(Spawn,saboteur)==1 then
        SetStepComplete(Spawn,saboteur,1)
    end
```
