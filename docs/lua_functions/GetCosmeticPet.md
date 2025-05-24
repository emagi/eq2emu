### Function: GetCosmeticPet(Spawn)

**Description:**
Returns the cosmetic pet (fun pet) entity of the given player if one is currently summoned.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Spawn – The cosmetic pet spawn if active, or nil if the player has no cosmetic pet out.

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
