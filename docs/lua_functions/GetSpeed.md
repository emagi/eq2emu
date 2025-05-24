### Function: GetSpeed(spawn)

**Description:**
Gets the current speed of the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Float value of the Spawn's speed.

**Example:**

```lua
-- From Spells/Mage/Summoner/Conjuror/Sleet.lua
function cast(Caster, Target, DDType, MinDDVal, MaxDDVal, SnareAmount, DispelChance)

	-- DD component
	if MaxDDVal ~= nil and MinDDVal < MaxDDVal then
		SpellDamage(Target, DDType, math.random(MinDDVal, MaxDDVal))
	else
		SpellDamage(Target, DDType, MinDDVal)
	end
```
