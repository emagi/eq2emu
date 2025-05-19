Function: GiveExp(Spawn, Amount)

Description: Awards a certain amount of experience points to the specified player. This can be used to grant quest rewards or bonus experience outside the normal combat exp flow.

Parameters:

    Spawn: Spawn – The player to receive experience.

    Amount: Int32 – The amount of experience points to award.

Returns: None.

Example:

-- Example usage (give experience upon quest completion)
GiveExp(Player, 10000)