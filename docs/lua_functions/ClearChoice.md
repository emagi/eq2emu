### Function: ClearChoice(spawn, commandToClear, clearDecline)

**Description:**

Clear's a choice previously set on the Player with a command.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `commandToClear` (string) - String `commandToClear`.
- `clearDecline` (uint8) - Integer value `clearDecline`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/DoF_design_path_script/AnimationSpeedScroll.lua
function DoSpellVisual(NPC,Spawn)
	ClearChoice(Spawn, "select")
	CreateChoiceWindow(NPC, Spawn, "Display Visual ID X, Visual ID Range X-Y, Visual ID String Wildcard, eg. heal", "OK", "select", "Cancel", "", 0, 1, 1, 14)
end
```
