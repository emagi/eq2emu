### Function: HasSpellEffect(spawn, spellID, tier)

**Description:**
Return's true if the spawn has an active spell effect with the spellid and tier specified.  If tier is set to 0 all tiers apply.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `spellID` (uint32) - Integer value `spellID`.
- `tier` (uint8) - Integer value `tier`.

**Returns:** True if spell effect is active on the spawn with spellid and tier (if tier is 0 then all tiers apply).  Otherwise false.

**Example:**

```lua
-- From ItemScripts/aCourierCostume.lua
function unequipped(Item, Player)
  if HasSpellEffect(Player, 5459, 1) then
    CastSpell(Player, 5459, 1)
end
```
