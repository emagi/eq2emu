### Function: MoveToLocation(spawn, x, y, z, speed, lua_function, more_points, use_nav_path)

**Description:**
Tells NPC to move to the location specified in the x,y,z.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `x` (int32) - Integer value `x`.
- `y` (int32) - Integer value `y`.
- `z` (int32) - Integer value `z`.
- `speed` (int32) - Integer value `speed`.
- `lua_function` (int32) - Integer value `lua_function`.
- `more_points` (int32) - Integer value `more_points`.
- `use_nav_path` (int32) - Integer value `use_nav_path`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/BeggarsCourt/aBrotherhoodenforcer.lua
function spawn(NPC)
	if GetSpawnLocationID(NPC) == 403031 then
		MoveToLocation(NPC, -14.62, 2.25, -6.99, 3, "", true)
		MoveToLocation(NPC, -17.68, 3.00, -21.58, 3, "", false)
	end
```
