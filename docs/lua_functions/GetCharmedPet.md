### Function: GetCharmedPet(Spawn)

**Description:**
Returns the NPC that the given player or NPC has charmed, if any. When a player charms an NPC (through a spell), that NPC becomes a pet under their control — this function retrieves it.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Example:**

```lua
-- From Spells/Mage/Enchanter/Charm.lua
function remove(Caster, Target)
    local pet = GetCharmedPet(Caster)
    if pet ~= nil then
        RemoveSpellBonus(pet)
        DismissPet(pet)
    end
```
