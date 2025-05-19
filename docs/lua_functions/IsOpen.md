Function: IsOpen(DoorSpawn)

Description: Checks whether a given door spawn is currently open.

Parameters:

    DoorSpawn: Spawn – The door object to check.

Returns: Boolean – true if the door is currently open; false if closed.

Example:

-- Example usage (guard reacts if the town gate is open past curfew)
if IsOpen(TownGate) then
    Say(GuardNPC, "Close the gates for the night!")
end