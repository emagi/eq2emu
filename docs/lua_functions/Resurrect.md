### Function: Resurrect(hp_perc, power_perc, send_window, target, heal_name, crit_mod, no_calcs, revive_spell_id, revive_spell_tier)

**Description:**
Resurrect the spell targets.

**Parameters:**
- `hp_perc` (float) - Float value `hp_perc`.
- `power_perc` (float) - Float value `power_perc`.
- `send_window` (uint32) - Integer value `send_window`.
- `target` (Spawn) - Spawn object representing `target`.
- `heal_name` (string) - String `heal_name`.
- `crit_mod` (uint32) - Integer value `crit_mod`.
- `no_calcs` (uint32) - Integer value `no_calcs`.
- `revive_spell_id` (uint32) - Integer value `revive_spell_id`.
- `revive_spell_tier` (uint32) - Integer value `revive_spell_tier`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/FavorofthePhoenix.lua
function cast(Caster, Target)
Resurrect(15, 15, 1)
    Say(Caster, "Summoning Sickness not implemented.")
end
```
