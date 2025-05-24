### Function: DeleteSpellBook(player, type_selection)

**Description:**
Delete all spell entries in a spell book.  `type_selection` represents DELETE_TRADESKILLS = 1, DELETE_SPELLS = 2, DELETE_COMBAT_ART = 4, DELETE_ABILITY = 8, DELETE_NOT_SHOWN = 16.  The `type_selection` can be an added combination of the delete options, for example a value of 7 (1+2+4) would delete tradeskills, spells and combat arts.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `type_selection` (uint8) - Integer value `type_selection`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Generic/SubClassToCommoner.lua
function RemoveGear(NPC,player)
-- many other parts to the script function
-- this example deletes spells and combat arts.
DeleteSpellBook(player, 6)
end
```
