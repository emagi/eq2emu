Function: GetOrigZ(Spawn)

Description: Returns the original Z coordinate of where the spawn was initially placed.

Parameters:

    Spawn: Spawn – The entity to check.

Returns: Float – The Z position of the spawn’s original location.

Example:

-- Example usage (calculate how far NPC roamed from spawn point horizontally)
local distanceFromSpawn = math.sqrt((GetX(NPC)-GetOrigX(NPC))^2 + (GetZ(NPC)-GetOrigZ(NPC))^2)