Function: GetOrigY(Spawn)

Description: Returns the original Y coordinate (vertical position) of the spawn’s starting point in the zone.

Parameters:

    Spawn: Spawn – The entity in question.

Returns: Float – The Y coordinate of its original spawn location.

Example:

-- Example usage (for debugging an NPC's elevation change)
print("NPC original Y: " .. GetOrigY(NPC) .. ", current Y: " .. GetY(NPC))