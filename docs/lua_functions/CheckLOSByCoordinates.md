### Function: CheckLOSByCoordinates(Origin, X, Y, Z)

**Description:** Checks line-of-sight from a spawn to a specific point in the world coordinates. Useful for verifying a location’s visibility (for example, whether a ground target spell can reach a point).

**Parameters:**

`Origin`: Spawn – The entity from which to check LOS.
`X`: Float – X coordinate of the target point.
`Y`: Float – Y coordinate of the target point.
`Z`: Float – Z coordinate of the target point.

**Returns:** Boolean – true if the line from Origin to (X,Y,Z) is clear; false if it’s obstructed.

**Example:**

```lua
-- Example usage (check if a spot is reachable by ranged attack)
if CheckLOSByCoordinates(Player, targetX, targetY, targetZ) then
    LaunchProjectile(Player, targetX, targetY, targetZ)
end
```