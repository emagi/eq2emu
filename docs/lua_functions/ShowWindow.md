### Function: ShowWindow(player, window, show)

**Description:**
Shows a UI window, window represents the ui/default directory eq2ui_x_y.xml, in other words eq2ui_inventory_inventory.xml will be Inventory.Inventory.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `window` (string) - String `window`.
- `show` (uint8) - Integer value `show`.

**Returns:** None.

**Example:**

```lua
-- From Quests/FarJourneyFreeport/TasksaboardtheFarJourney.lua
	ShowWindow(Player, "Inventory.Inventory", 0)
```
