### Function: RemoveTriggerFromSpell(remove_count)

**Description:**
Remove a spell trigger from the spell, must be ran inside Spell Script.

**Parameters:**
- `remove_count` (uint16) - Quantity `remove_count`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/ArcaneEnlightenment.lua
function proc(Caster, Target, Type, Power, Triggers)
    CastSpell(Caster, 2550391, 1)
    RemoveTriggerFromSpell()
end 
```
