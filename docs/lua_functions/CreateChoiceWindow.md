### Function: CreateChoiceWindow(npc, spawn, windowTextPrompt, acceptText, acceptCommand, declineText, declineCommand, time, textBox, textBoxRequired, maxLength)

**Description:**

Create's a choice display window for the client to provide a accept/decline prompt window and input box support (for text field).

**Parameters:**
- `npc` (Spawn) - Spawn object representing `npc`.
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `windowTextPrompt` (string) - String `windowTextPrompt`.
- `acceptText` (string) - String `acceptText`.
- `acceptCommand` (string) - String `acceptCommand`.
- `declineText` (string) - String `declineText`.
- `declineCommand` (string) - String `declineCommand`.
- `time` (uint32) - Integer value `time`.
- `textBox` (uint8) - Integer value `textBox`.
- `textBoxRequired` (uint8) - Integer value `textBoxRequired`.
- `maxLength` (uint32) - Integer value `maxLength`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/DoF_design_path_script/AnimationSpeedScroll.lua
function DoSpellVisual(NPC,Spawn)
	ClearChoice(Spawn, "select")
	CreateChoiceWindow(NPC, Spawn, "Display Visual ID X, Visual ID Range X-Y, Visual ID String Wildcard, eg. heal", "OK", "select", "Cancel", "", 0, 1, 1, 14)
end
```
