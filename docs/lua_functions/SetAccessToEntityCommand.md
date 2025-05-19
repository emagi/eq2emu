Function: SetAccessToEntityCommand(Spawn, Player, CommandString, Allowed)

Description: Controls whether a particular primary entity command (right-click option) is enabled for a given spawn. You can use this to enable or disable specific interactions dynamically.

Parameters:

    Spawn: Spawn – The entity whose command access to modify.
	
    Player: Spawn – The Player who will receive access to the command.

    CommandString: String – The name of the command (same as used in AddPrimaryEntityCommand).

    Allowed: UInt8 – `1` to allow players to use this command on the spawn; `0` to remove it.

Returns: Return's true if successfully adding access to the entity command.

Example:

-- Example usage (disable the 'Buy' option on a merchant after shop closes)
SetAccessToEntityCommand(MerchantNPC, "Buy", 0)