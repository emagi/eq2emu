### Function: SendWaypoints(Player)

**Description:** Sends the Player the visual/guiding waypoints provided through AddWaypoint.

**Parameters:**

`Player`: Spawn – The Player to update their waypoints by sending the packet update.

**Returns:** None.

**Example:**

```lua
-- Add a waypoint to the client and send the waypoints
AddWaypoint(Player, "Lets go here!", 102.5, -12.3, 230.0)
SendWaypoints(Player)
```