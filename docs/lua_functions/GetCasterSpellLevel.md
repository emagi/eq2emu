### Function: GetCasterSpellLevel(spell)

**Description:**
Gets the Caster's spell level.  The `spell` field is optional for outside a spell script, if inside a spell script the current spell is assumed.

**Parameters:**
- `spell` (Spell) - Spell object representing `spell`.

**Returns:** The spell caster's level in UINT32 format.

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
