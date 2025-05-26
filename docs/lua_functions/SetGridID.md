### Function: SetGridID(spawn, grid)

**Description:**
Set's the grid id of the spawn.  See /grid for more information on the grid id.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `grid` (uint32) - Integer value `grid`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/FallenGate/atormentedbattlemage_A.lua
function Change_Grid_A(NPC)
	 Say(NPC, "This is the Change_Grid_A function")
	 SetGridID(NPC, 3104458931)
end
```
