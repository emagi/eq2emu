### Function: GetPet(Spawn)

**Description:**
Retrieves the pet entity of the given spawn, if one exists. For players, this returns their current summoned combat pet (summoner or necromancer pet, etc.), or for NPCs, a charmed pet or warder.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Return:** Spawn object of the pet that the owner spawn currently has.

**Example:**

```lua
-- From Spells/CamtursEnergizingAura.lua
function cast(Caster, Target)
    level = GetLevel(Caster)
    Pet = GetPet(Caster)
    AddSpellBonus(Caster, 500, math.ceil(level * 2.75))
    AddSpellBonus(Caster, 501, math.ceil(level * 2.75))
    AddSpellBonus(Caster, 200, level * 5 + 99)
    CastSpell(Pet, 2550518)
end
```
