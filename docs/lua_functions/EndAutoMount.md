### Function: EndAutoMount(Spawn)

**Description:**
Dismounts a player who was auto-mounted via StartAutoMount. Typically called at the end of an automated travel route or upon leaving the area where auto-mount is enforced.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** None.

**Example:**

```lua
-- From ZoneScripts/Antonica.lua
function GriffonTower(Zone, Spawn)
	if IsPlayer(Spawn) and IsOnAutoMount(Spawn) then
		EndAutoMount(Spawn)
	end
```
