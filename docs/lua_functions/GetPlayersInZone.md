Function: GetPlayersInZone(Zone)

Description: Retrieves a list of all player spawns currently present in the specified zone.

Parameters:

    Zone: Zone – The zone object or context to search in.

Returns: Table – A list (array) of player Spawn objects in that zone.

Example:

-- Example usage (announce a message to all players in the zone)
for _, player in ipairs(GetPlayersInZone(GetZone(NPC))) do
    SendMessage(player, "The dragon roars in the distance!", "red")
end