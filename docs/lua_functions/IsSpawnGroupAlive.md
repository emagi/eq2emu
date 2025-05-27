### Function: IsSpawnGroupAlive(zone, group_id)

**Description:**

Return's true if there is an alive spawn within the spawn group_id specified in the Zone.

**Parameters:**
- `zone` (Zone) - Zone object representing `zone`.
- `group_id` (uint32) - Integer value `group_id`.

**Returns:** Return's true if there is an alive spawn within the spawn group_id specified in the Zone.  Otherwise false.

**Example:**

```lua
-- From SpawnScripts/ThunderingSteppes/AntelopeHerd1.lua
function SpawnCheck(NPC)
    local zone = GetZone(NPC)
    
    if  IsSpawnGroupAlive(zone, GroupID) == true then
        AddTimer(NPC, 6000, "SpawnCheck")
    else
        Despawn(GetSpawnByLocationID(zone, 133793500))
    end
```
