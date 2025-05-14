Function: GetDeityPet(Spawn)

Description: Retrieves the deity pet entity belonging to the specified player, if the deity pet is currently summoned.

Parameters:

    Spawn: Spawn – The player whose deity pet to get.

Returns: Spawn – The deity pet spawn if it is currently active, or nil if no deity pet is out.

Example:

-- Example usage (check for deity pet presence)
if GetDeityPet(Player) ~= nil then
    SendMessage(Player, "Your deity companion watches over you.", "white")
end