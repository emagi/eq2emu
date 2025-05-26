### Function: MakeRandomFloat(min, max)

**Description:**
Make a random float between min and max.

**Parameters:**
- `min` (float) - Float value `min`.
- `max` (float) - Float value `max`.

**Returns:** Float value between min and max.

**Example:**

```lua
-- From Spells/Commoner/VerdantTrinity.lua
function cast(Caster, Target, Val1, Val2)
	Percentage = MakeRandomFloat(Val1, Val2)
	SpellHealPct("Heal", Percentage, false, false, Target, 2)
end
```
