### Function: SummonPet(spawn, pet_id, max_level)

**Description:**
Summon a pet with the pet_id (spawn database id).

**Parameters:**
- `spawn` (Spawn) - Spawn object reference `spawn`.
- `pet_id` (uint32) - Integer value `pet_id`.
- `max_level` (uint8) - Integer value `max_level`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/MoleratTest.lua
function cast(Caster, Target)
	SummonPet(Caster, 2780089)
CallCliffdiverHawk(NPC, Spawn)
	local CliffdiverHawk = GetSpawn(NPC, 2780089)

	if CliffdiverHawk ~= nil then
		AddTimer(CliffdiverHawk, 5000, "FlyToMoleRat", 1, Spawn)
	end
```
