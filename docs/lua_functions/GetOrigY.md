### Function: GetOrigY(Spawn)

**Description:**
Returns the original Y coordinate (vertical position) of the spawn’s starting point in the zone.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Float – The Y position of the spawn’s original location.

**Example:**

```lua
-- From SpawnScripts/OutpostOverlord/acliffdiverhawk.lua
function ReturnHome(NPC)

    local x = GetOrigX(NPC)
    local y = GetORigY(NPC)
    local z = GetOrigZ(NPC)

    if IsInCombat(NPC) == false then
        MoveToLocation(NPC, x, y, z, 5)
    end
```
