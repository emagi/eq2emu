### Function: SetMountColor(spawn, red, green, blue, red, green, blue)

**Description:**
Sets the mount's colors for the Spawn specified.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `red` (int32) - Integer value `red`.
- `green` (int32) - Integer value `green`.
- `blue` (int32) - Integer value `blue`.
- `red` (int32) - Integer value `red`.
- `green` (int32) - Integer value `green`.
- `blue` (int32) - Integer value `blue`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/BeggarsCourt/AdjunctCaptainRommuls.lua
function spawn(NPC)
	SetMount(NPC, 6831)
	SetMountColor(NPC, 1, 1, 1, 255, 1, 1)
	
	MoveToLocation(NPC, -8.13, 4.00, -42.68, 3, "", true)
	MoveToLocation(NPC, -14.43, 3.57, -27.02, 3, "", false)
end
```
