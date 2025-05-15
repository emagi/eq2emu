Function: OpenDoor(DoorSpawn)

Description: Opens a door spawn in the world. This will play the door’s open animation and typically allow passage. The door remains open until closed by script or by its own auto-close timer if any.

Parameters:

    DoorSpawn: Spawn – The door object to open (must be a door interactive spawn).

Returns: None.

Example:

-- Example usage (open a secret door when puzzle is solved)
OpenDoor(SecretDoor)