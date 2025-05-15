Function: MakeRandomInt(Min, Max)

Description: Generates a random integer between the specified minimum and maximum values (inclusive).

Parameters:

    Min: Int32 – The minimum value.

    Max: Int32 – The maximum value.

Returns: Int32 – A random integer N where Min ≤ N ≤ Max.

Example:

-- Example usage (roll a random amount of coin to reward)
local reward = MakeRandomInt(100, 500)  -- between 100 and 500 copper
AddCoin(Player, reward)