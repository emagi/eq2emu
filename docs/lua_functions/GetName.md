### Function: GetName(spawn)

**Description:**
Gets the string name of the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** string name of the specified Spawn.

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
