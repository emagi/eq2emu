### Function: IsFollowing(spawn)

**Description:**
Return's true if the spawn following is enabled.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** True if the spawn following is enabled, otherwise false.

**Example:**

```lua
-- From SpawnScripts/BeggarsCourt/Ro.lua
function ResetFollow(NPC)    
    if IsFollowing(NPC) then
        SetTarget(NPC,nil)
        ToggleFollow(NPC)
        AttackTimer = false
end
```
