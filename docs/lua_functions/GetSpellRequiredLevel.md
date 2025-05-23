### Function: GetSpellRequiredLevel(spell)

**Description:**
Obtain the Required Level of the Spell, must be used inside a Spell Script.

**Parameters:**
- None

**Returns:** None.

**Example:**

```lua
-- From Spells/Dregs.lua
function cast(Caster, Target, BonusAmt)
    -- Allows target to breathe under water
    BreatheUnderwater(Target, true)
    
    BonusAmt = CalculateRateValue(Caster, Target, GetSpellRequiredLevel(Caster), GetLevel(Caster), 1.0, BonusAmt)

	AddSpellBonus(Target, 201, BonusAmt)
end
```
