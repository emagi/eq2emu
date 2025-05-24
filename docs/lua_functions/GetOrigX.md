### Function: GetOrigX(Spawn)

**Description:**
Returns the original X coordinate of the specified spawn’s spawn point (where it was initially placed in the zone).

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Float – The X position of the spawn’s original location.

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
