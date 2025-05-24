### Function: GetShardCreatedTimestamp(ShardID)

**Description:**
Returns the Unix timestamp (or similar) of when the spirit shard was created (i.e., the time of the player’s death that generated it).

**Parameters:**
- `npc` (Spawn) - Spawn object representing `npc`.

**Returns:** UInt32 timestamp of the creation time of the shard.

**Example:**

```lua
-- From SpawnScripts/Generic/SpiritShard.lua
function CheckShardExpired(NPC)
	local timestamp = GetShardCreatedTimestamp(NPC)
	local dateTable = os.date("*t", timestamp)

	-- Generate time
	local creationTime = os.time{year=dateTable.year, month=dateTable.month, day=dateTable.day, hour=dateTable.hour, min=dateTable.min, sec=dateTable.sec}

	local currentUTCTime = os.time(os.date('!*t'))
	
	local resultDiff = currentUTCTime - creationTime;
	local shardLifeTime = GetRuleFlagFloat("R_Combat", "ShardLifetime")
	
	if shardLifeTime > 0 and resultDiff > shardLifeTime then
		local shardid = GetShardID(NPC)
		DeleteDBShardID(shardid) -- you could alternatively choose to not delete from DB, but for now it only holds XP debt recovery not items
		Despawn(NPC)
	end
```
