Function: GetOrigX(Spawn)

Description: Returns the original X coordinate of the specified spawn’s spawn point (where it was initially placed in the zone).

Parameters:

    Spawn: Spawn – The entity to query.

Returns: Float – The X position of the spawn’s original location.

Example:

-- Example usage (see how far an NPC has moved from its spawn point on X axis)
local deltaX = math.abs(GetX(NPC) - GetOrigX(NPC))