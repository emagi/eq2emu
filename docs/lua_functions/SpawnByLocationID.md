### Function: SpawnByLocationID(zone, location_id)

**Description:**
Spawn by location_id in the current Zone.

**Parameters:**
- `zone` (Zone) - Zone object representing `zone`.
- `location_id` (uint32) - Integer value `location_id`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/RyGorrExplosiveMiningBarrel.lua
function used(Item, Player)
	if GetQuestStep(Player, RyGorrOperations) == 1 then
		if GetZoneID(GetZone(Player)) == 470 then
			local X = GetX(Player)
			local Y = GetY(Player)
			local Z = GetZ(Player)
			if X > -20.27 and X < -10.27 then
				if Y < -60 then
					if Z > 150.07 and Z < 160.07 then
						local barrel = SpawnByLocationID(GetZone(Player), 33980)
						AddSpawnAccess(barrel, Player)
						SetTempVariable(barrel, "player", Player)
						RemoveItem(Player, 47881)
					else
						SendMessage(Player, "You cannot place the Ry'Gorr Explosive Mining Barrel here.", "yellow")
					end
```
