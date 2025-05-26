### Function: UseWidget(widget)

**Description:**
Use or Trigger a widget/object, such as in the context of opening or closing a door.

**Parameters:**
- `widget` (Spawn) - Spawn object representing `widget`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Baubbleshire/BinkOakshire.lua
function Door1(NPC,Spawn)
    local door = GetSpawn(NPC, 2380074)
    UseWidget(door)
end
```
