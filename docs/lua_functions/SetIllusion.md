### Function: SetIllusion(spawn, model)

**Description:**
Sets the illusion of the Spawn to the model provided.  See model ids by version on https://wiki.eq2emu.com/ReferenceLists

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `model` (uint16) - Integer value `model`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/GenericGenderDisguise.lua
function cast(Caster, Target, Male, Female)
	if GetGender(Spawn) == 1 then
        SetIllusion(Target, Male)
    else
        SetIllusion(Target, Female)
    end
```
