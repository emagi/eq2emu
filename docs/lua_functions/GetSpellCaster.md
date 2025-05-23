### Function: GetSpellCaster(spell)

**Description:**
Obtain the Spawn 'Caster' of the Spell.  The `spell` field is optional, it will otherwise be in a Spell Script which defaults to getting the current spell caster.

**Parameters:**
- `spell` (Spell) - Spell object representing `spell`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Priest/Cleric/Inquisitor/ContriteGrace.lua
function proc(Caster, Target, Type, MinValHeal, MaxValHeal, MinValDamage, MaxValDamage)
    local initial_caster = GetSpellCaster()
	if initial_caster ~= nil and Type == 15 then
	    MinValHeal = CalculateRateValue(initial_caster, Target, GetSpellRequiredLevel(initial_caster), GetCasterSpellLevel(), 3.75, MinValHeal)
	    MaxValHeal = CalculateRateValue(initial_caster, Target, GetSpellRequiredLevel(initial_caster), GetCasterSpellLevel(), 3.75, MaxValHeal)
	    MinValDamage = CalculateRateValue(initial_caster, Target, GetSpellRequiredLevel(initial_caster), GetCasterSpellLevel(), 1.25, MinValDamage)
	    MaxValDamage = CalculateRateValue(initial_caster, Target, GetSpellRequiredLevel(initial_caster), GetCasterSpellLevel(), 1.25, MaxValDamage)

	    SpellHeal("heal", MinValHeal, MaxValHeal, Caster, 0, 0, "Atoning Faith")
        ProcDamage(initial_caster, Target, "Atoning Faith", 7, MinValDamage, MaxValDamage)
	    RemoveTriggerFromSpell(1)
	end
```
