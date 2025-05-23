### Function: AddSpawnAccess(Spawn, Player)

**Description:**
Allows `Player` access to `Spawn` visually and such as for restricting to Quest only Player's.

**Parameters:**
- `Spawn` (Spawn) - Spawn object representing `Spawn`.
- `Player` (Spawn) - Spawn object value `Player`.

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
