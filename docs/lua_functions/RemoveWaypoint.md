### Function: RemoveWaypoint(Player, WaypointIndex)

**Description:** Removes the guiding waypoint for the Player.  SendWaypoints should also be sent after.

**Parameters:**

`Player`: Spawn – The NPC whose waypoint path to modify.
`Name`: String - The name of the waypoint supplied originally when AddWaypoint was called.

**Returns:** None.

**Example:**

```lua
-- Example usage (remove a previously added waypoint)
RemoveWaypoint(Player, "PreviousWaypoint")
SendWaypoints(Player)
```