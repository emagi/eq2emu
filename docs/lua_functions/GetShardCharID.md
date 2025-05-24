### Function: GetShardCharID(ShardID)

**Description:**
Given a spirit shard’s ID, returns the character ID of the player who owns that shard.

**Parameters:**
- `npc` (Spawn) - Spawn object representing `npc`.

**Returns:** UInt32 char id of the shards player.

**Example:**

```lua
-- From SpawnScripts/Generic/SpiritShard.lua
function spawn(NPC)
	local DebtToRemovePct = GetRuleFlagFloat("R_Combat", "ShardDebtRecoveryPercent")
		
	if GetRuleFlagBool("R_Combat", "ShardRecoveryByRadius") == true then
		SetPlayerProximityFunction(NPC, 10.0, "recovershard")
	end
```
