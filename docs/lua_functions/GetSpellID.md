### Function: GetSpellID()

**Description:**
Gets the spell id of the current spell.

**Parameters:** None.

**Returns:** None.

**Example:**

```lua
-- From Spells/Mage/Sorcerer/Warlock/SkeletalGrasp.lua
function proc(Caster, Target, Type, Chance)
local Spell = GetSpellID()	
if Type == 15 and HasSpellEffect(Target, Spell) then
		RemoveControlEffect(Target, 5)
			end
```
