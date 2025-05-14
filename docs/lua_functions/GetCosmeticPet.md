Function: GetCosmeticPet(Spawn)

Description: Returns the cosmetic pet (fun pet) entity of the given player if one is currently summoned.

Parameters:

    Spawn: Spawn – The player whose cosmetic pet to retrieve.

Returns: Spawn – The cosmetic pet spawn if active, or nil if the player has no cosmetic pet out.

Example:

-- Example usage (pet-related quest checking if a particular pet is summoned)
local pet = GetCosmeticPet(Player)
if pet ~= nil and GetName(pet) == "Frostfell Elf" then
    Say(NPC, "I see you have a festive friend with you!")
end