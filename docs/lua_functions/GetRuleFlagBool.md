### Function: GetRuleFlagBool(category, name)

**Description:**
Gets the boolean rule flag from the world based on the category and name.

**Parameters:**
- `category` (string) - String `category`.
- `name` (string) - String `name`.

**Returns:** Boolean value true/false depending on the rule setting.

**Example:**

```lua
-- From SpawnScripts/Generic/SpiritShard.lua
function spawn(NPC)
	local DebtToRemovePct = GetRuleFlagFloat("R_Combat", "ShardDebtRecoveryPercent")
		
	if GetRuleFlagBool("R_Combat", "ShardRecoveryByRadius") == true then
		SetPlayerProximityFunction(NPC, 10.0, "recovershard")
	end
```
