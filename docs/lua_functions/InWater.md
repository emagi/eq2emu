Function: InWater(Spawn)

Description: Determines if the given spawn is currently submerged in water.

Parameters:

    Spawn: Spawn – The entity to check.

Returns: Boolean – true if the spawn is in water; false if not.

Example:

-- Example usage (If NPC is in water make them glow)
if InWater(NPC) then
    SpawnSet(NPC, "visual_state", 2103)
else
    SpawnSet(NPC, "visual_state", 0)
end