### Function: RemoveThreatTransfer(spawn)

**Description:**
Remove a threat transfer currently assigned to the Spawn, only inside a Spell Script.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Fighter/Crusader/Paladin/Amends.lua
function remove(Caster, Target)
    RemoveThreatTransfer(Caster)
end
```
