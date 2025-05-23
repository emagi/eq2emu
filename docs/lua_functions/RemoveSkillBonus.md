### Function: RemoveSkillBonus(spawn)

**Description:**
Remove's any skill bonuses added to the Spawn (if outside a spells script) or all spell targets of a spell from AddSkillBonus (if inside a spell script).

**Parameters:**
- `spawn` (Spawn) - Spawn object reference `spawn`.

**Returns:** None.

**Example:**

```lua
-- From Spells/AA/BattlemagesFervor.lua
function remove(Caster, Target)
    RemoveSkillBonus(Target)
end
```
