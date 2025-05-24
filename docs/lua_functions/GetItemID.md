### Function: GetItemID(item)

**Description:**
Get Item ID of the specified Item object.

**Parameters:**
- `item` (Item) - Item object representing `item`.

**Returns:** UInt32 value of the item id.

**Example:**

```lua
-- From ItemScripts/Darkheart.lua
function used(Item, Player)
    local item_id = GetItemID(Item)
    CastSpell(Player, SPELLID, SPELL_TIERS[item_id])
end
```
