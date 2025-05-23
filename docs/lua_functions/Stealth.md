### Function: Stealth(type, spawn)

**Description:**
Type of 1 is stealth, 2 is invis.  If Spawn is specified will only apply to Spawn, otherwise applied to all spell targets in a Spell Script.

**Parameters:**
- `type` (int8) - Integer value `type`.
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Camouflage.lua
function cast(Caster, Target)
    Stealth(1)
end
```
