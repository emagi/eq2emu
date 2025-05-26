### Function: SetFailureTimer(spawn)

**Description:**
Sets the failure timer on the Spawn for the current zone.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From ZoneScripts/BloodSkullValleyExcavationSite.lua
function player_entry(zone, player)
    SetFailureTimer(player)
end
```
