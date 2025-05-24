### Function: GetCharacterID(spawn)

**Description:**
Gets the character id of the current Spawn(Player).

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UINT32 value of Spawn's the character id.  0 if not a Player.

**Example:**

```lua
-- From SpawnScripts/Generic/SpiritShard.lua
function recovershard(NPC, Spawn)
	local charid = GetShardCharID(NPC)
	
	if GetCharacterID(Spawn) == charid then
		local DebtToRemovePct = GetRuleFlagFloat("R_Combat", "ShardDebtRecoveryPercent")
		local DeathXPDebt = GetRuleFlagFloat("R_Combat", "DeathExperienceDebt")
		
		local debt = GetInfoStructFloat(Spawn, "xp_debt")
		local DebtToRemove = (DebtToRemovePct/100.0)*(DeathXPDebt/100.0);
		if debt > DebtToRemove then
			SetInfoStructFloat(Spawn, "xp_debt", debt - DebtToRemove)
		else
			SetInfoStructFloat(Spawn, "xp_debt", 0.0)
		end
```
