### Function: IsInCombat(spawn)

**Description:**

Return's true if the spawn is currently in combat.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Return's true if the spawn is currently in combat.

**Example:**

```lua
-- From ItemScripts/awellspringcubleash.lua
function used(Item, Player)
    target = GetTarget(Player)
	if GetName(target) == 'a wellspring cub' and GetTempVariable(Player, "cub") == nil then
		if not IsInCombat(target) then
			CastEntityCommand(Player, target, 1278, "Leash")
		end
```
