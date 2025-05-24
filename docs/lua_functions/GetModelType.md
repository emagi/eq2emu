### Function: GetModelType(spawn)

**Description:**
Get the model type of the current spawn, see https://wiki.eq2emu.com/ReferenceLists Game Models / Model IDs by client version for the ID list.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/SilverTweezers.lua
function used(Item, Player)
	local target = GetTarget(Player)
	if target ~= nil then
		local model = GetModelType(target)
		if model == 81 or model == 82 or model == 91 or model == 93 or model == 94 or model == 95 or model == 96 or model == 97 or model == 98 or model == 99 or model == 100 or model == 101 or model == 102 then
			CastSpell(target, 2550000, 1, Player)
		end
```
