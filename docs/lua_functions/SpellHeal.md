### Function: SpellHeal(heal_type, min_heal, max_heal, target, crit_mod, no_calcs, custom_spell_name)

**Description:**
Heal or Increase Power of a Target inside of a spell script.  heal_type is "heal" or "power".

**Parameters:**
- `heal_type` (string) - String value `heal_type`.
- `min_heal` (int32) - Integer value `min_heal`.
- `max_heal` (int32) - Integer value `max_heal`.
- `target` (Spawn) - Spawn object representing `target`.
- `crit_mod` (int32) - Integer value `crit_mod`.
- `no_calcs` (int32) - Integer value `no_calcs`.
- `custom_spell_name` (string) - String `custom_spell_name`.

**Returns:** None.

**Example:**

```lua
-- From Spells/AA/AncestralChanneling.lua
function cast(Caster, Target, HealMin, HealMax, HoTMin, HoTMax)
SpellHeal("Heal", HealMin, HealMax)
end
```
