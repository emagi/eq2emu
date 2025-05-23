### Function: SpellDamage(target, type, min_damage, max_damage, crit_mod, no_calcs, class_id)

**Description:**
Damage a Target inside of a Spell Script.

**Parameters:**
- `luaspell` (int32) - Integer value `luaspell`.
- `type` (int32) - Integer value `type`.
- `min_damage` (int32) - Integer value `min_damage`.
- `max_damage` (int32) - Integer value `max_damage`.
- `crit_mod` (int32) - Integer value `crit_mod`.
- `no_calcs` (int32) - Integer value `no_calcs`.
- `class_id` (uint32) - Integer value `class_id`.

**Returns:** None.

**Example:**

```lua
-- From Spells/AA/AmbidexterousCasting.lua
function cast(Caster, Target, DmgType, MinVal, MaxVal)
    Interrupt(Caster, Target)
SpellDamage(Target, DmgType, MinVal, MaxVal)
end
```
