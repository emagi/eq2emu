### Function: GetWardAmountLeft(spell)

**Description:**
Get the ward value amount left on the spell.

**Parameters:**
- `spell` (Spell) - Spell object representing `spell`.

**Returns:** UInt32 amount left on the ward.

**Example:**

```lua
-- From Spells/Priest/Shaman/EidolicWard.lua
function remove(Caster, Target)
    local heal = GetWardAmountLeft(Target)
    SpellHeal("Heal", heal, heal)
    RemoveWard(Caster)
end
```
