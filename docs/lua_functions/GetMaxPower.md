### Function: GetMaxPower(spawn)

**Description:**
Gets the Spawn's maximum (total) power.


**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** SInt32 maximum (total) power of the spawn.

**Example:**

```lua
-- From Spells/Centered.lua
function cast(Caster, Target)
    MaxPower = GetMaxPower(Caster)
    AddSpellBonus(Caster, 501, math.floor(MaxPower * 0.025))
end
```
