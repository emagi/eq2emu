Function: GetPet(Spawn)

Description: Retrieves the pet entity of the given spawn, if one exists. For players, this returns their current summoned combat pet (summoner or necromancer pet, etc.), or for NPCs, a charmed pet or warder.

Parameters:

    Spawn: Spawn – The owner whose pet we want to get.

Returns: Spawn – The pet entity of the owner, or nil if no pet is present.

Example:

-- Example usage (command a player's pet to attack if it exists)
local pet = GetPet(Player)
if pet ~= nil then
    Attack(pet, TargetNPC)
end