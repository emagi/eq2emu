### Function: ProcDamage(caster, target, name, dmg_type, low_damage, high_damage, success_msg, effect_msg)

**Description:**
Conduct proc damage against the target.

**Parameters:**
- `caster` (Spawn) - Spawn object representing `caster`.
- `target` (Spawn) - Spawn object representing `target`.
- `name` (string) - String `name`.
- `dmg_type` (uint8) - Integer value `dmg_type`.
- `low_damage` (uint32) - Integer value `low_damage`.
- `high_damage` (uint32) - Integer value `high_damage`.
- `success_msg` (string) - String `success_msg`.
- `effect_msg` (string) - String `effect_msg`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/LaserGoggles.lua
function doDamage(Player, Target, damage)
    local damage = math.floor(((GetHP(Target) / 100) * 50) + GetHP(Target))
    ProcDamage(Player, Target, " Dev AE Slay", 4, damage)
end
```
