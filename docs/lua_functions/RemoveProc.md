### Function: RemoveProc(spawn, item)

**Description:**
Remove proc that was applied in a spell script or item script.  Will apply to spawn unless item is not specified, then all spell targets are applicable.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `item` (Item) - Item object representing `item`.

**Returns:** None.

**Example:**

```lua
-- From Spells/BastensRunesofSurety.lua
function remove(Caster, Target)
    RemoveSpellBonus()
    RemoveProc()
end
```
