Function: CompareSpawns(SpawnA, SpawnB)

Description: Compares two spawn objects to determine if they refer to the same actual entity in the world.

Parameters:

    SpawnA: Spawn – The first spawn to compare.

    SpawnB: Spawn – The second spawn to compare.

Returns: Boolean – true if both SpawnA and SpawnB represent the same spawn (same entity); false if they are different.

Example:

-- Example usage (make sure a target hasn’t changed before proceeding)
if CompareSpawns(CurrentTarget, GetTarget(Player)) then
    -- proceed assuming target remains the same
end