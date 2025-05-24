### Function: Gate(spell)

**Description:**
Gate's the current Spawn to their bind point.

**Parameters:** None.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/CircleElders/GMHelper.lua
function BindPointOption(NPC, Spawn)
    Despawn(NPC)
    Gate(Spawn)
end
```
