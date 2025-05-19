Function: InLava(Spawn)

Description: Checks if the specified spawn is in lava. Similar to InWater, but specifically for lava volumes which might cause damage.

Parameters:

    Spawn: Spawn – The entity to check.

Returns: Boolean – true if the spawn is currently in lava; false otherwise.

Example:

-- Example usage (tell the player they are standing in lava)
if InLava(Player) then
    SendMessage(Player, "Feels toasty here..", "red")
end