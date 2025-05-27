### Function: AddSpellBookEntry(player, spellid, tier, add_silently, add_to_hotbar)

**Description:**

Adds a spell book entry for the Player with the spellid and tier provided.  The add_silently (false by default), add_to_hotbar (true by default)

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `spellid` (uint32) - Integer value `spellid`.
- `tier` (uint16) - Integer value `tier`.
- `add_silently` (bool) - Boolean flag `add_silently`.
- `add_to_hotbar` (bool) - Boolean flag `add_to_hotbar`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/ForgeryFreeportCitizenshipPapers.lua
function Task3(Item,Player)
local Race = GetRace(Player)
    if Race == 11 then --Kerra
    if HasQuest(Player,LA_F) then
    SetStepComplete(Player,LA_F,14)
    end
```
