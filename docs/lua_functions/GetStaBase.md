### Function: GetStaBase(spawn)

**Description:**
Gets the base stamina of the spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 value base stamina of the spawn.

**Example:**

```lua
-- From Spells/Priest/Cleric/Inquisitor/SacredArmorWithStaBonus.lua
function cast(Caster, Target, MitAmt, StaAmt)
HealthMod = GetStaBase(Target) * StaAmt    
AddSpellBonus(Target, 200, MitAmt, 11, 21, 31)
    AddSpellBonus(Target, 500, HealthMod, 1)
end
```
