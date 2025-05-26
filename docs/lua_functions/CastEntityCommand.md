### Function: CastEntityCommand(caster, target, id, command)

**Description:**
Calls an Entity Command through LUA versus the player triggering it themselves.

**Parameters:**
- `caster` (Spawn) - Spawn object representing `caster`.
- `target` (Spawn) - Spawn object representing `target`.
- `id` (uint32) - Integer value `id`.
- `command` (string) - String `command`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/AutomaticBook.lua
function obtained(Item, Spawn)    
    target = GetTarget(Spawn)
    if target ~= nil then
--        CastEntityCommand(Spawn, target, 1, "Scribe")
end
```
