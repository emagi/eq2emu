### Function: SendOptionWindow(option_window, player, window_title, cancel_command)

**Description:**
Send a option window with the previous option window arguments.

**Parameters:**
- `option_window` (OptionWindow) - OptionWindow object representing `option_window`.
- `player` (Spawn) - Spawn object representing `player`.
- `window_title` (string) - String `window_title`.
- `cancel_command` (string) - String `cancel_command`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Generic/GenericCraftingTrainer.lua
function SendSecondaryChoice(NPC, Spawn)
	window = CreateOptionWindow()
    AddOptionWindowOption(window, "Craftsman", "Craftsmen become carpenters, provisioners, or woodworkers.  They make furniture and strong boxes, food, drink, bows, arrows, totems, wooden weapons, and wooden shields.", 1, 420, "select_craftsman")
    AddOptionWindowOption(window, "Outfitter", "Outfitters become armorers, tailors, or weaponsmiths.  They make plate and chainmail armor, heavy shields, cloth and leather armor, casual clothing, backpacks, hex dolls, and metal weapons.", 1, 411, "select_outfitter")
    AddOptionWindowOption(window, "Scholar", "Scholars become alchemists, jewelers, and sages.  They make spell and combat art upgrades for adventurers, potions, poisons, and jewelry.", 1, 396, "select_scholar")
    SendOptionWindow(window, Spawn, "Select A Profession")
end
```
