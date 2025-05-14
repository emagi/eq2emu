Function: GetShardCharID(ShardID)

Description: Given a spirit shard’s ID, returns the character ID of the player who owns that shard.

Parameters:

    ShardID: Int32 – The shard’s unique ID.

Returns: Int32 – The character ID of the player to whom the shard belongs.

Example:

-- Example usage (identify which player a shard belongs to, perhaps for a shard recovery NPC)
local ownerCharID = GetShardCharID(shardSpawnID)