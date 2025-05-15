Function: SetTarget(Originator, Target)

Description: Forces one spawn (Originator) to target another (Target). This can make an NPC switch targets mid-combat or cause a player’s target to change under certain conditions.

Parameters:

    Originator: Spawn – The entity whose target will be changed.

    Target: Spawn – The entity to set as the new target.

Returns: None.

Example:

-- Example usage (boss switches target to a healer)
SetTarget(BossNPC, HealerNPC)
SetInfoFlag(BossNPC) -- assures we send the changes out immediately versus waiting for process loop