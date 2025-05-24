### Function: GetEncounter(spawn)

**Description:**
Gets the encounter list of the Spawn(NPC).

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Table list of Spawn's in the encounter list

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
