Function: ClearRunback(NPC)

Description: Stops the NPC from trying to return to its “runback” point (the spot it was originally at or a designated safe point). Often used when you want to stop an NPC from automatically fleeing back or resetting.

Parameters:

    NPC: Spawn – The NPC to clear runback for.

Returns: None.

Example:

-- Example usage (prevent an NPC from running back to spawn point after combat)
ClearRunback(NPC)