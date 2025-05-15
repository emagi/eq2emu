Function: SetPlayerLevel(Player, Level)

Description: Sets the player’s adventure level to the specified value. This is an administrative function (normally level changes by experience gain), allowing GM or script to directly change level.

Parameters:

    Player: Spawn – The player whose level to change.

    Level: Int32 – The new level to set.

Returns: None.

Example:

-- Example usage (GM tool leveling a player to 50)
SetPlayerLevel(Player, 50)