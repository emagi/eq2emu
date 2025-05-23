### Function: RemoveSpellBonus(spawn)

**Description:**
Remove's any spell bonuses from AddSpellBonus.  If inside a spell Script RemoveSpellBonus() will remove all spell targets bonuses, RemoveSpellBonus(Target) removes only that Spawn's spell bonuses.

**Parameters:**
- `spawn` (Spawn) - Spawn object reference `spawn`.

**Returns:** None.

**Example:**

```lua
-- From Spells/AA/AbilityAptitude.lua
function remove(Caster, Target)
    RemoveSpellBonus(Target)
end
```
