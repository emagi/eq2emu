### Function: RemoveWard()

**Description:**
Remove Ward from spell targets, must be used in spell script.

**Parameters:** None.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/CarpetDjinnMaster.lua
function remove(Caster, Target)
    RemoveSpellBonus(Caster)
RemoveWard()
SetMount(Caster, 0)
  RemoveControlEffect(Caster, 12)
end
```
