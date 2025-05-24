### Function: GetTempVariable(spawn, var)

**Description:**
Get a temporary variable of the Spawn

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `var` (string) - String `var`.

**Returns:** Depending on the SetTempVariable type the return type will vary.

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
