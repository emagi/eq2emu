### Function: IsInvulnerable(spawn)

**Description:**
Return's true if spawn is set to invulnerable.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** True if Spawn invulnerable otherwise false.

**Example:**

```lua
-- From RegionScripts/exp04_dun_chardok/char_p10_crossbridge_pit01_region.lua
function EnterRegion(Zone, Spawn, RegionType)
    -- RegionType 2 is 'lava' or 'death' regions, RegionType 1 is water

    local invul = IsInvulnerable(Spawn)
    if invul == true then
        return 0
    end
```
