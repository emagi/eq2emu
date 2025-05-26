### Function: Knockback(target_spawn, spawn, duration, vertical, horizontal, use_heading)

**Description:**
Triggers a knockback on the target_spawn with spawn being the originator.

**Parameters:**
- `target_spawn` (Spawn) - Spawn object representing `target_spawn`.
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `duration` (uint32) - Time value `duration` in seconds.
- `vertical` (float) - Float value `vertical`.
- `horizontal` (float) - Float value `horizontal`.
- `use_heading` (uint8) - Integer value `use_heading`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/houseofroachie/GnomishKnockbackDevice.lua
function examined(NPC, Spawn)
    Knockback(NPC, Spawn, 10)
end
```
