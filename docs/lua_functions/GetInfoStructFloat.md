### Function: GetInfoStructFloat(spawn, field)

**Description:**
Retrieves a floating-point field from a spawn’s info data. Could be used for precise position, speed multipliers, etc. if stored there.  See https://github.com/emagi/eq2emu/blob/main/docs/data_types/info_struct.md for a full list of options.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `field` (string) - String `field`.

**Returns:** Float – The value of that field.

**Example:**

```lua
-- From SpawnScripts/Generic/SpiritShard.lua
function recovershard(NPC, Spawn)
	local charid = GetShardCharID(NPC)
	FindSpellVisualByID
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
