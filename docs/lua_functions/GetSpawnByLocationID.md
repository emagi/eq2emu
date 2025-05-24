### Function: GetSpawnByLocationID(zone, location_id)

**Description:**
Gets a spawn by its location id.

**Parameters:**
- `zone` (Zone) - Zone object representing `zone`.
- `location_id` (uint32) - Integer value `location_id`.

**Returns:** Spawn object reference.

**Example:**

```lua
-- From ItemScripts/Griz.lua
function GrizChat2_1(Item, Spawn)
	if GetQuestStep(Spawn, SometimesKnut) == 2 then
		SetStepComplete(Spawn, SometimesKnut, 2)
		AddSpawnAccess(GetSpawnByLocationID(Zone, 579551), Spawn)
	end
```
