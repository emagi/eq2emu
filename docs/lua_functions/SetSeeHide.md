Function: SetSeeHide(Spawn, Enable)

Description: Toggles an NPC’s ability to see hidden/stealthed entities on or off. “Hide” usually refers to stealth as opposed to magical invisibility.

Parameters:

    Spawn: Spawn – The NPC in question.

    Enable: Boolean – true to grant see-stealth; false to remove it.

Returns: None.

Example:

-- Example usage (guards become alert and can see stealth during an alarm event)
SetSeeHide(GuardNPC, true)