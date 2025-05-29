### Function: CloseDoor(DoorSpawn, DisableCloseSound)

**Description:** Closes an open door spawn, playing its closing animation and sound.

**Parameters:**

`DoorSpawn`: Spawn – The door object to close.
`DisableCloseSound`: Boolean - Default is false, when set to true, no door close sound will be made.

**Returns:** None.

**Example:**

```lua
-- Example usage (close the door after some time or event)
CloseDoor(CastleGate)
```