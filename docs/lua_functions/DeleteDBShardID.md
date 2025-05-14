Function: DeleteDBShardID(ShardID)

Description: Removes the database record for a given spirit shard, effectively deleting the shard (often after it’s been collected or expired).

Parameters:

    ShardID: Int32 – The ID of the shard to delete.

Returns: Boolean – true if a shard record was found and deleted; false if not.

Example:

-- Example usage (clean up a shard after player retrieves it)
if DeleteDBShardID(shardID) then
    SendMessage(Player, "You feel whole again as your spirit shard dissipates.", "white")
end