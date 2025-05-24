### Function: DamageSpawn(attacker, victim, type, dmg_type, low_damage, high_damage, spell_name, crit_mod)

**Description:**
Damages the victim by the attacker.  `type` represents damage packet types listed here https://github.com/emagi/eq2emu/blob/main/docs/data_types/damage_packet_types.md converted from hex to decimal.

**Parameters:**
- `attacker` (Spawn) - Spawn object representing `attacker`.
- `victim` (Spawn) - Spawn object representing `victim`.
- `type` (uint8) - Integer value `type`.
- `dmg_type` (uint8) - Integer value `dmg_type`.
- `low_damage` (uint32) - Integer value `low_damage`.
- `high_damage` (uint32) - Integer value `high_damage`.
- `spell_name` (string) - String `spell_name`.
- `crit_mod` (uint8) - Integer value `crit_mod`.

**Returns:** None.

**Example:**

```lua
-- From RegionScripts/exp04_dun_droga_nurga/naj_lavaregion_damage.lua
function TakeLavaDamage(Spawn)
    local invul = IsInvulnerable(Spawn)
    if invul == true then
        return 0
    end

	local hp = GetHP(Spawn)
    local level = GetLevel(Spawn)
    local damageToTake = level * 25
	-- if we don't have enough HP make them die to pain and suffering not self
	if hp <= damageToTake then
		KillSpawn(Spawn, null, 1)
	else
		DamageSpawn(Spawn, Spawn, 192, 3, damageToTake, damageToTake, "Lava Burn", 0, 0, 1, 1)
	end
end
```
