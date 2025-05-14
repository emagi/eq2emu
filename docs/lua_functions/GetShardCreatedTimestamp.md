Function: GetShardCreatedTimestamp(ShardID)

Description: Returns the Unix timestamp (or similar) of when the spirit shard was created (i.e., the time of the player’s death that generated it).

Parameters:

    ShardID: Int32 – The spirit shard’s ID.

Returns: Int64 – The creation timestamp of the shard.

Example:

-- Example usage (calculate how old a shard is for some mechanic)
local shardTime = GetShardCreatedTimestamp(shardID)
local ageSeconds = os.time() - shardTime