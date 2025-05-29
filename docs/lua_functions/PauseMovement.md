### Function: PauseMovement(Spawn, DurationMS)

**Description:** Pauses the given NPC’s movement for the specified duration (in milliseconds). The NPC will stop moving along waypoints or patrols, and then resume after the pause.

**Parameters:**

`Spawn`: Spawn – The NPC to pause.
`DurationMS`: Int32 – How long to pause movement, in milliseconds.

**Returns:** None.

**Example:**

```lua
-- Example usage (stop a guard for 5 seconds when hailed)
PauseMovement(GuardNPC, 5000)
```