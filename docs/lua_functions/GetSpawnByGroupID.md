### Function: GetSpawnByGroupID(zone, group_id)

**Description:**
Gets the first spawn by its group_id.

**Parameters:**
- `zone` (Zone) - Zone object representing `zone`.
- `group_id` (uint32) - Integer value `group_id`.

**Returns:** Spawn object reference

**Example:**

```lua
-- From SpawnScripts/IsleRefuge1/GoblinSaboteurFirepit.lua
function InRange(NPC,Spawn)
    if GetQuestStep(Spawn,saboteur)==1 then
        SetStepComplete(Spawn,saboteur,1)
    end
```
