Function: GetRaceBaseType(Spawn)

Description: Returns the base race category of the spawn, which may be similar to GetRaceType but often refers to an even broader grouping (like “Player” vs “NPC” races or base model types).

Parameters:

    Spawn: Spawn – The entity to check.

Returns: Int32 – An identifier for the base race category.

Example:

-- Example usage (check if a target is a player character or an NPC by base type)
if GetRaceBaseType(Target) == BASE_RACE_PLAYER then
    Say(NPC, "Greetings, adventurer.")
end