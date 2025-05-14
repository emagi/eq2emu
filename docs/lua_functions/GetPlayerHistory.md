Function: GetPlayerHistory(Player, HistoryID)

Description: Retrieves the value of a specific player history flag. This tells if a player has a certain historical event or choice recorded (and what value it is).

Parameters:

    Player: Spawn – The player to check.

    HistoryID: Int32 – The history flag ID to retrieve.

Returns: Int32 – The value of that history entry for the player (commonly 0 or 1, but could be other integers if used as counters).

Example:

-- Example usage (branch dialog if player has a specific history flag)
if GetPlayerHistory(Player, ALLIED_WITH_GNOLLS_HISTORY_ID) == 1 then
    Say(NPC, "Welcome, friend of the Gnolls!")
else
    Say(NPC, "Stranger, tread carefully...")
end