### Function: GetSpawn(spawn, spawn_id)

**Description:**
Gets a spawn object by the spawn_id in the area.    The `spawn` value references the area to search.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `spawn_id` (uint32) - Integer value `spawn_id`.
**Returns:** None.

**Example:**

```lua
-- From ItemScripts/FroglokPondstoneEvil.lua
function used(Item, Player)
    local Cube = 331142
    local Spawn2 = GetSpawn(Player, Cube)
    if Spawn2 == nil then SendMessage(Player, "You must seek an ancient pond to use this item.", "Yellow") else
    local Distance = GetDistance(Player, Spawn2)
    if Distance > 50 then SendMessage(Player, "You must seek an ancient pond to use this item.", "Yellow")
    else CastSpell(Player, 2550399, 1)
        end
```
