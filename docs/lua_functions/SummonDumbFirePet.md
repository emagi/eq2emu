### Function: SummonDumbFirePet(spawn, target, pet_id, x, y, z)

**Description:**
Summon a dumbfire pet with the pet_id (spawn database id).

**Parameters:**
- `spawn` (Spawn) - Spawn object reference `spawn`.
- `target` (Spawn) - Spawn object representing `target`.
- `pet_id` (uint32) - Integer value `pet_id`.
- `x` (float) - Float value `x`.
- `y` (float) - Float value `y`.
- `z` (float) - Float value `z`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Mage/Enchanter/Coercer/Puppetmaster.lua
function cast(Caster, Target, PetID)
    local x = GetX(Caster)
    local y = GetY(Caster)
    local z = GetZ(Caster)
local count = 0;
while (count < 4) do
    local pet = SummonDumbFirePet(Caster, Target, PetID, x, y, z)
    if pet ~= nil then
  CopySpawnAppearance(pet, Target)
        SpawnSet(pet, "size", "6")
    end
```
