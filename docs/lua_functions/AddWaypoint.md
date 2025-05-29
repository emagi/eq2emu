### Function: AddWaypoint(Player, Name, X, Y, Z)

**Description:** Adds a guiding waypoint to a Player with the Name for the description at X, Y, Z coordinates.  You can add multiple waypoints and then send waypoints.  Refer to RemoveWaypoint(Player, Name) to remove an existing waypoint.

**Parameters:**

`Player`: Spawn – The Player to which to provide the waypoint.
`Name`: String - The name / description of the waypoint.
`X`: Float – The X coordinate of the waypoint.
`Y`: Float – The Y coordinate (vertical) of the waypoint.
`Z`: Float – The Z coordinate of the waypoint.

**Returns:** None.

**Example:**

```lua
-- Add a waypoint to the client and send the waypoints
AddWaypoint(Player, "Lets go here!", 102.5, -12.3, 230.0)
SendWaypoints(Player)
```