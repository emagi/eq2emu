### Function: SetVision(spawn, vision)

**Description:**
Set's the vision mode for the Player.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `vision` (uint32) - Integer value `vision`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/TotenoftheEnduringSpirit.lua
function cast(Caster, Target)
	if (GetLevel(Caster) >= 80) 
		then    
	AddControlEffect(Caster, 7)
	BreatheUnderwater(Caster, true)
	SetVision(Caster, 4)
	end
```
