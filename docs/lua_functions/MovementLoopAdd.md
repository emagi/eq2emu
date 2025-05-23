### Function: MovementLoopAdd(spawn, x, y, z, speed, delay, function, heading, exclude_heading, use_nav_path)

**Description:**
Adds a waypoint for the NPC to reach in order of the supplied waypoints.  Calls function when arriving at the waypoint destination.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `x` (int32) - Integer value `x`.
- `y` (int32) - Integer value `y`.
- `z` (int32) - Integer value `z`.
- `speed` (int32) - Integer value `speed`.
- `delay` (int32) - Integer value `delay`.
- `function` (int32) - Integer value `function`.
- `heading` (int32) - Integer value `heading`.
- `exclude_heading` (int32) - Integer value `exclude_heading`.
- `use_nav_path` (int32) - Integer value `use_nav_path`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/FarJourneyFreeport/agoblin.lua
function run_around_loop(NPC)
	MovementLoopAdd(NPC, -4.43, -2.07, 6.17, 5, 1)
	MovementLoopAdd(NPC, -4.43, -2.07, 6.17, 5, 3, "run_around_loop_pause1")
	MovementLoopAdd(NPC, -4.43, -2.07, 6.17, 5, 0)
	MovementLoopAdd(NPC, -5.23, -2.01, 0.39, 5, 1)
	MovementLoopAdd(NPC, -5.23, -2.01, 0.39, 5, 3, "run_around_loop_pause2")
	MovementLoopAdd(NPC, -5.23, -2.01, 0.39, 5, 0)
	MovementLoopAdd(NPC, -4.88, -2.06, 4.26, 5, 1)
	MovementLoopAdd(NPC, -4.88, -2.06, 4.26, 5, 3, "run_around_loop_pause3")
	MovementLoopAdd(NPC, -4.88, -2.06, 4.26, 5, 0)
	MovementLoopAdd(NPC, 3.94, -2.07, 0.66, 5, 1)
	MovementLoopAdd(NPC, 3.94, -2.07, 0.66, 5, 3, "run_around_loop_pause4")
	MovementLoopAdd(NPC, 3.94, -2.07, 0.66, 5, 0)
	MovementLoopAdd(NPC, 2.84, -2.07, -2.07, 5, 1)
	MovementLoopAdd(NPC, 2.84, -2.07, -2.07, 5, 3, "run_around_loop_pause5")
	MovementLoopAdd(NPC, 2.84, -2.07, -2.07, 5, 0)
	MovementLoopAdd(NPC, 3.41, -1.99, -7.42, 5, 1)
	MovementLoopAdd(NPC, 3.41, -1.99, -7.42, 5, 3, "run_around_loop_pause6")
	MovementLoopAdd(NPC, 3.41, -1.99, -7.42, 5, 0)
	MovementLoopAdd(NPC, -2.75, -2.02, -5.82, 5, 0)
	MovementLoopAdd(NPC, -2.63, 1.21, -18.11,5,1)
	MovementLoopAdd(NPC, -2.63, 1.21, -18.11,5,3,"run_around_loop_pause7")
	MovementLoopAdd(NPC, -2.63, 1.21, -18.11,3,0)
	MovementLoopAdd(NPC, -2.75, -2.02, -5.82, 5, 0)
end
```
