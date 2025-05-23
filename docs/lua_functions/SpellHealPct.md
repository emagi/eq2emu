### Function: SpellHealPct(heal_type, percentage, current_value, caster_value, target, crit_mod, no_calcs, custom_spell_name)

**Description:**
Heal or Increase Power percentage of a Target inside of a spell script.  heal_type is "heal" or "power".

**Parameters:**
- `heal_type` (string) - String value `heal_type`.
- `percentage` (int32) - Integer value `percentage`.
- `current_value` (int32) - Integer value `current_value`.
- `caster_value` (int32) - Integer value `caster_value`.
- `target` (Spawn) - Spawn object representing `target`.
- `crit_mod` (int32) - Integer value `crit_mod`.
- `no_calcs` (int32) - Integer value `no_calcs`.
- `custom_spell_name` (string) - String `custom_spell_name`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/HateSpores.lua
function cast(Caster, Target, Pwr)
    SpellHealPct("Power", Pwr, false, true, Target)
end
```
