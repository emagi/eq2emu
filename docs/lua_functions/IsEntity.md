### Function: IsEntity(spawn)

**Description:**
Return's true if the Spawn is an entity (Player/NPC/Bot).

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** True if the spawn is an entity, otherwise false.

**Example:**

```lua
-- From ItemScripts/LaserGoggles.lua
function used(Item, Player)
    local target = GetTarget(Player)
    if target ~= nil and IsEntity(target) then
        local encounter = GetEncounter(target)
        if encounter ~= nil then
            doDamage(Player, target, damage)
        else
            doDamage(Player, target, damage)
        end
```
