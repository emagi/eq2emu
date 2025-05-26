### Function: SummonCosmeticPet(spawn, pet_id)

**Description:**
Summon a cosmetic pet with the pet_id (spawn database id).

**Parameters:**
- `spawn` (Spawn) - Spawn object reference `spawn`.
- `pet_id` (uint32) - Integer value `pet_id`.

**Returns:** None.

**Example:**

```lua
-- From Spells/AA/SummonAnimatedTome.lua
function cast(Caster, Target, CritChance)
    AddSpellBonus(Target, 656, CritChance)
    SummonCosmeticPet(Caster, 150084)
end
```
