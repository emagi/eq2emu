### Function: GetTarget(spawn)

**Description:**
Gets the current target of the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Spawn object reference.

**Example:**

```lua
-- From ItemScripts/AutomaticBook.lua
function obtained(Item, Spawn)    
    target = GetTarget(Spawn)
    if target ~= nil then
--        CastEntityCommand(Spawn, target, 1, "Scribe")
end
```
