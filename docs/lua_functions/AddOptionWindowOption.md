### Function: AddOptionWindowOption(OptionWindow, OptionName, OptionDescription, OptionIconSheet, OptionIconID, OptionCommand, OptionConfirmTitle)

**Description:**
Adds a new option to the option window screen.

**Parameters:**
- `OptionWindow`: OptionWindow - OptionWindow object made with CreateOptionWindow().
- `OptionName`: string - String value.
- `OptionDescription`: string - String value.
- `OptionIconSheet`: int32 - Integer value.
- `OptionIconID`: int16 - Short integer value.
- `OptionCommand`: string - String value.
- `OptionConfirmTitle`: string - String value.

**Returns:** None.

**Notes:**
- Must call CreateOptionWindow to instantiate the OptionWindow before AddOptionWindowOption can be used.

**Example:**

```lua
-- Example usage: Create an option window to select a profession, Craftsman, Outfitter or Scholar, function outputs are select_*professionname*.
function SendOptionWindow(NPC, Spawn)
	window = CreateOptionWindow()
    AddOptionWindowOption(window, "Craftsman", "Craftsmen become carpenters, provisioners, or woodworkers.  They make furniture and strong boxes, food, drink, bows, arrows, totems, wooden weapons, and wooden shields.", 1, 420, "select_craftsman")
    AddOptionWindowOption(window, "Outfitter", "Outfitters become armorers, tailors, or weaponsmiths.  They make plate and chainmail armor, heavy shields, cloth and leather armor, casual clothing, backpacks, hex dolls, and metal weapons.", 1, 411, "select_outfitter")
    AddOptionWindowOption(window, "Scholar", "Scholars become alchemists, jewelers, and sages.  They make spell and combat art upgrades for adventurers, potions, poisons, and jewelry.", 1, 396, "select_scholar")
    SendOptionWindow(window, Spawn, "Select A Profession")
end

function select_craftsman(NPC, Spawn)
   SetTradeskillClass(Spawn, CRAFTSMAN)
   SetTradeskillLevel(Spawn, 10)
end

function select_outfitter(NPC, Spawn)
   SetTradeskillClass(Spawn, OUTFITTER)
   SetTradeskillLevel(Spawn, 10)
end

function select_scholar(NPC, Spawn)
   SetTradeskillClass(Spawn, SCHOLAR)
   SetTradeskillLevel(Spawn, 10)
end

```
