### Function: SpellDamageExt(target, type, min_damage, max_damage, crit_mod, no_calcs, override_packet_type, take_power, class_id)

**Description:**
Damage a Target inside of a Spell Script, offers power 'damage' support.

**Parameters:**
- `luaspell` (int32) - Integer value `luaspell`.
- `type` (int32) - Integer value `type`.
- `min_damage` (int32) - Integer value `min_damage`.
- `max_damage` (int32) - Integer value `max_damage`.
- `crit_mod` (int32) - Integer value `crit_mod`.
- `no_calcs` (int32) - Integer value `no_calcs`.
- `override_packet_type` (int32) - Integer value `override_packet_type`.
- `take_power` (int32) - Integer value `take_power`.
- `class_id` (uint32) - Integer value `class_id`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Scout/Bard/WaltsSingingBlade.lua
function damage(Caster, Target, DmgType, MinVal, MaxVal, MinPwr, MaxPwr)
    Level = GetLevel(Caster)
    SpellLevel = 15
    Mastery = SpellLevel + 10
    StrBonus = GetStr(Caster) / 10
    IntBonus = GetInt(Caster) / 10 
        
    if Level < Mastery then
        LvlBonus = Level - SpellLevel
        else LvlBonus = Mastery - SpellLevel
    end
```
