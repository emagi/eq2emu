### Function: SetMaxHP(spawn, value)

**Description:**
Set's the Max HP of the Spawn (As a Spell Bonus) to the new value.

**Parameters:**
- `spawn` (Spawn) - Spawn object reference `spawn`.
- `value` (int32) - Integer value `value`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/devpotion.lua
function examined(Item, Player)
SetMaxHP(Player, GetMaxHP * 3)
end
```
