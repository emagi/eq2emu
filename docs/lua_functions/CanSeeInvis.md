Function: CanSeeInvis(Player, Target)

Description: Checks if the given NPC or player can see invisible entities. Some NPCs have see-invis or see-stealth abilities, which this would indicate.

Parameters:

    Player/Entity: Spawn – The Player/Entity to check for see-invisibility capability.
    Target: Spawn – The entity to check if Player can see them.

Returns: Boolean – true if this spawn can detect invisible targets; false if not.

Example:

-- Example usage (NPC will attack stealthed players only if it can see invis)
if IsPlayer(Target) or CanSeeInvis(NPC, Target) then
    Attack(NPC, Target)
end