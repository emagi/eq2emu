Function: SendUpdateDefaultCommand(Spawn, Distance, CommandString, Player)

Description: Updates the default highlighted command for an entity, as seen on hover or target window. This is often used after changing accessible commands to ensure the correct default action (usually the first allowed command) is highlighted.

Parameters:

    Spawn: Spawn – The entity to update.

    Distance: Float – The maximum distance the command can be used.

    CommandString: String – The command to provide access.

    Player: Spawn – The player whom has access to the command (optional to specify a specific Player).

Returns: None.

Example:

-- Example usage (after toggling commands on an NPC, refresh its default action)
SendUpdateDefaultCommand(NPC, 10.0, "hail")