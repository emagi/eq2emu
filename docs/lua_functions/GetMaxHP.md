### Function: GetMaxHP(spawn)

**Description:**
Gets the maximum (total) hp of the spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** SInt32 of the max hp for the spawn.

**Example:**

```lua
-- From ItemScripts/cadavers_dram.lua
function used(Item, Player)
	if GetQuestStep(Player, BecomingOrcbane) == 1 then
		local target = GetTarget(Player)
		if GetSpawnID(target) == 4700105 then
			if GetHP(target) < GetMaxHP(target) * .20 then
				CastEntityCommand(Player, target, 1299, "cadaver's dram")
			else
				SendMessage(Player, "You must use this on a Ry'Gorr tunneler that is under 20 percent life.", "yellow")
			end
```
