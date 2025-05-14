Function: CheckLOS(Origin, Target)

Description: Checks line-of-sight between two spawns. Returns true if Origin can “see” Target (no significant obstacles in between), false if line of sight is blocked.

Parameters:

    Origin: Spawn – The entity from whose perspective to check line of sight.

    Target: Spawn – The entity to check if visible.

Returns: Boolean – true if there is line-of-sight; false if something blocks the view between origin and target.

Example:

-- Example usage (sniper NPC only shoots if it has line of sight to the player)
if CheckLOS(SniperNPC, Player) then
    CastSpell(SniperNPC, SNIPER_SHOT_ID, 1, Player)
end