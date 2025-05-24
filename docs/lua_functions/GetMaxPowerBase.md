### Function: GetMaxPowerBase(spawn)

**Description:**
Gets the Spawn's base power.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** SInt32 base power of the spawn.

**Example:**

```lua
-- From Spells/Commoner/DualBreed.lua
function cast(Caster, Target)
    PowerBonus = math.ceil(GetMaxPowerBase(Caster) * 0.03)
    AddSpellBonus(Caster, 1, 2)
    AddSpellBonus(Caster, 501, PowerBonus)
end
```
