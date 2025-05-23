### Function: RemoveControlEffect(spawn, type, only_remove_spawn)

**Description:**
Removes a control effect from the Spawn or from the spell targets of a spell.

**Parameters:**
- `Spawn` (Spawn) - Spawn object reference `spawn`.
- `type` (int32) - Integer value `type`.
- `only_remove_spawn` (int32) - Integer value `only_remove_spawn`.

**Returns:** None.

**Example:**

```lua
-- From Spells/AA/DazingBash.lua
function remove(Caster, Target)
	RemoveControlEffect(Target, 3)
end
```
