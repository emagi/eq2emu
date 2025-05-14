Function: GetCharmedPet(Spawn)

Description: Returns the NPC that the given player or NPC has charmed, if any. When a player charms an NPC (through a spell), that NPC becomes a pet under their control — this function retrieves it.

Parameters:

    Spawn: Spawn – The entity (usually a player) who may have a charmed pet.

Returns: Spawn – The charmed pet NPC if one exists, or nil if there is no active charmed pet.

Example:

-- Example usage (checking if player currently has a charmed creature)
local charmed = GetCharmedPet(Player)
if charmed ~= nil then
    Say(charmed, "I am under your control...") 
end