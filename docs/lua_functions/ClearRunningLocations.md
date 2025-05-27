### Function: ClearRunningLocations(spawn)

**Description:**

Removes all the running locations on the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/FarJourneyFreeport/agoblin.lua
function run_around_loop_init_pause(NPC)
	ClearRunningLocations(NPC)
	AddTimer(NPC, 700, "run_around_loop_init_continue")
end
```
