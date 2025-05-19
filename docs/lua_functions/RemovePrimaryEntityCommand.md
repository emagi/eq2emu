Function: RemovePrimaryEntityCommand(Spawn, CommandString)

Description: Removes a previously added primary entity command from the specified spawn entirely. Players will no longer see that option when interacting with the spawn.

Parameters:

    Spawn: Spawn – The entity from which to remove the command.

    CommandString: String – The name of the command to remove.

Returns: None.

Example:

-- Example usage (Remove hail from command list)
RemovePrimaryEntityCommand(QuestNPC, "hail")