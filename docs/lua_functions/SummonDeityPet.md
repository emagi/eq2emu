### Function: SummonDeityPet(spawn, pet_id)

**Description:**
Summon a deity pet with the pet_id (spawn database id).

**Parameters:**
- `spawn` (Spawn) - Spawn object reference `spawn`.
- `pet_id` (uint32) - Integer value `pet_id`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/PeacefulVisage.lua
function cast(Caster, Target, PetID, Stats)
	Adjusted = GetLevel(Caster) / Stats
		SummonDeityPet(Caster, PetID, Stats)
		Say(Caster, "deity checks needed and formula need to be fine tuned")
		AddSpellBonus(Target, 0, Adjusted)
		AddSpellBonus(Target, 1, Adjusted)
		AddSpellBonus(Target, 2, Adjusted)
		AddSpellBonus(Target, 3, Adjusted)
		AddSpellBonus(Target, 4, Adjusted)
end
```
