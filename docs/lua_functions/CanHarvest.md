### Function: CanHarvest(Player, GroundSpawn)

**Description:** Determines if the specified spawn (which should be a player) is currently able to harvest resources (i.e., not in combat and has the required skill/tool). This function checks general conditions for harvesting.

**Parameters:**

`Player`: Spawn – The player to check.
`GroundSpawn`: Spawn – The groundspawn to apply the check on.

**Returns:** Boolean – true if the player can harvest at the moment; false if something prevents harvesting (lacking skill).

**Example:**

```lua
-- Example usage (before starting an auto-harvest routine, verify the player can harvest)
if CanHarvest(Player, GroundSpawn) then
    Harvest(Player, GroundSpawn)
end
```