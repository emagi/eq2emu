### Function: GetDeityPet(spawn)

**Description:**
Retrieves the deity pet entity belonging to the specified player, if the deity pet is currently summoned.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Example:**

```lua
-- From Spells/Commoner/PeacefulVisage.lua
function remove(Caster, Target)
	pet = GetDeityPet(Caster)
		if pet ~= nil then
			DismissPet(pet)
				end
```
