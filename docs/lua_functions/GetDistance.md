### Function: GetDistance(spawn, spawn2, include_radius)

**Description:**
Gets the distance between the two spawns, spawn and spawn2.  The include_radius is optional when set to true will use the collision radius of the spawn to reduce distance of the spawns.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `spawn2` (Spawn) - Spawn object representing `spawn2`.
- `include_radius` (uint8) - Distance `include_radius`.
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
