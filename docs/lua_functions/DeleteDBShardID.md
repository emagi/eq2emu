### Function: DeleteDBShardID(ShardID)

**Description:**
Removes the database record for a given spirit shard, effectively deleting the shard (often after it’s been collected or expired).

**Parameters:**
- `shardid` (uint32) - Integer value `shardid`.

**Returns:** None.

**Example:**

```lua
-- Example usage (clean up a shard after player retrieves it)
if DeleteDBShardID(shardID) then
    SendMessage(Player, "You feel whole again as your spirit shard dissipates.", "white")
end
```
