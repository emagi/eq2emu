### Function: SetPlayerProximityFunction(spawn, distance, in_range_function, leaving_range_function)

**Description:**
The Spawn (NPC) will be notified of a Player entering in range or leaving range.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `distance` (int32) - Distance `distance`.
- `in_range_function` (int32) - Distance `in_range_function`.
- `leaving_range_function` (int32) - Distance `leaving_range_function`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Antonica/adistressedmerchant.lua
function spawn(NPC)
	SetPlayerProximityFunction(NPC, 15, "InRange")
end
```
