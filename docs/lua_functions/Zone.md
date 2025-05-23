### Function: Zone(zone, player, x, y, z, heading)

**Description:**
Zone the player to the destination zone,x,y,z,heading.

**Parameters:**
- `zone` (Zone) - Zone object representing `zone`.
- `player` (Spawn) - Spawn object representing `player`.
- `x` (int32) - Integer value `x`.
- `y` (int32) - Integer value `y`.
- `z` (int32) - Integer value `z`.
- `heading` (int32) - Integer value `heading`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/BetaVoucher.lua
function examined(Item, Spawn)
    Zone(GetZone(1), Spawn, 48.28, -1.08, 44.05, 221.37)
end
```
