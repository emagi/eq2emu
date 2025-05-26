### Function: SetTempVariable(spawn, var, val)

**Description:**
Set's a temporary variable on the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `var` (string) - String `var`.
- `val` (string) - String `val`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/BowlOfTerratrodderChuck.lua
function used(Item, Player)
	if HasQuest(Player, AMindOfMyOwn) then
		if GetZoneID(GetZone(Player)) == 108 then

		    RemoveItem(Player, TerratrodderChuck)
			local bucket = SpawnMob(GetZone(Player), 1081002, 1, GetX(Player), GetY(Player), GetZ(Player), GetHeading(Player))
			AddSpawnAccess(bucket, Player)
			SetTempVariable(bucket, "PlayerPointer", Player)
		end
```
