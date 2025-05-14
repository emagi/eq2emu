Function: GetRaceType(Spawn)

Description: Retrieves the race type category ID of the specified spawn. Instead of the specific race (human, elf, etc.), this returns a broader category (e.g., humanoid, animal, etc.).

Parameters:

    Spawn: Spawn – The entity whose race type to get.

Returns: Int32 – The race type ID of the spawn (corresponding to categories in game data).

Example:

-- Example usage (determine if NPC is animal-type for a charm spell)
if GetRaceType(NPC) == RACE_TYPE_ANIMAL then
    SendMessage(Player, "This creature can be charmed by your spell.", "white")
end