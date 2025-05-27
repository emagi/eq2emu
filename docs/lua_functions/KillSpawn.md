### Function: KillSpawn(dead, killer)

**Description:**

Trigger's the dead spawn to be killed.  The killer field is optional, but updates that the killer targetted the dead spawn.

**Parameters:**
- `dead` (Spawn) - Spawn object representing `dead`.
- `killer` (Spawn) - Spawn object representing `killer`.

**Returns:** None.

**Example:**

```lua
-- From RegionScripts/exp04_dun_chardok/char_p10_crossbridge_pit01_region.lua
function EnterRegion(Zone, Spawn, RegionType)
    -- RegionType 2 is 'lava' or 'death' regions, RegionType 1 is water

    local invul = IsInvulnerable(Spawn)
    if invul == true then
        return 0
    end

    if RegionType == 2 then
		    KillSpawn(Spawn, null, 1)
	    end
end

```
