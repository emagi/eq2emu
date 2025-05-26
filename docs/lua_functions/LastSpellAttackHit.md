### Function: LastSpellAttackHit()

**Description:**
Return's true if the last spell attack was a successful hit.

**Parameters:** None.

**Returns:** True if the last hit was successful  otherwise false.

**Example:**

```lua
-- From Spells/AA/NullifyingStaff.lua
function cast(Caster, Target, DmgType, MinVal, MaxVal, CombatMit, Arcane)
	SpellDamage(Target, DmgType, MinVal, MaxVal)
--if LastSpellAttackHit() then
--AddSpellBonus(Target, 0, CombatMit)
--end
		if LastSpellAttackHit() then
			AddSpellBonus(Target, 203, Arcane)
				end
```
