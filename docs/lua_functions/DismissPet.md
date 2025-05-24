### Function: DismissPet(Spawn)

**Description:**
Dismisses (despawns) the specified player’s active pet. This works for combat pets, cosmetic pets, deity pets, etc., causing them to vanish as if the player dismissed them manually.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From Spells/AA/SummonAnimatedTome.lua
function remove(Caster, Target)
    RemoveSpellBonus(Target)
    pet = GetCosmeticPet(Caster)
    if pet ~= nil then
        DismissPet(pet)
    end
```
