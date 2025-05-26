### Function: IsOnAutoMount(player)

**Description:**
Return's true if Player is on a auto-mount (like griffon).

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** True if Player is on auto mount, otherwise false.

**Example:**

```lua
-- From ZoneScripts/Antonica.lua
function GriffonTower(Zone, Spawn)
	if IsPlayer(Spawn) and IsOnAutoMount(Spawn) then
		EndAutoMount(Spawn)
	end
```
