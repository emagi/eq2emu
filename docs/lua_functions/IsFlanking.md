### Function: IsFlanking(spawn, target)

**Description:**
Return's true if Spawn is flanking Target.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `target` (Spawn) - Spawn object representing `target`.

**Returns:** True if Spawn is flanking Target, otherwise false.

**Example:**

```lua
-- From Spells/CaskinsRingingSwipe.lua
function precast(Caster,Target)
	if not IsFlanking(Caster, Target) and not IsBehind(Caster, Target) then
        SendMessage(Caster, "Must be flanking or behind", "yellow")
        return false
	end
```
