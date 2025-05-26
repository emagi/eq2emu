### Function: ResetIllusion(spawn)

**Description:**
Remove the current illusion from a spawn, the spawn parameter is optional, inside spell script resets all spell targets illusion.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/GenericGenderDisguise.lua
function remove(Caster, Target)
	ResetIllusion(Target)
end
```
