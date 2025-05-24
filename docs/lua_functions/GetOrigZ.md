### Function: GetOrigZ(Spawn)

**Description:**
Returns the original Z coordinate of where the spawn was initially placed.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Float – The Z position of the spawn’s original location.

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
