### Function: SetAlignment(spawn, alignment)

**Description:**
Sets the alignment of the Spawn to the value.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `alignment` (int32) - Integer value `alignment`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/ForgeryFreeportCitizenshipPapers.lua
if GetRace(Player) == 0 or GetRace(Player) == 3 or GetRace(Player) == 5 or GetRace(Player) == 6 or GetRace(Player) == 9 or GetRace(Player) == 11 or GetRace(Player) == 20 then
SetAlignment(Player, 2)
end   
```
