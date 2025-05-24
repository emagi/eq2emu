### Function: GetItemType(item)

**Description:**
Gets the defined type of the item described in the database.  See https://github.com/emagi/eq2emu/blob/main/docs/data_types/item_types.md for item types.

**Parameters:**
- `item` (Item) - Item object representing `item`.

**Returns:** UInt32 value of the item type.

**Example:**

```lua
-- From Spells/Fighter/Crusader/UnyieldingAdvance.lua
function precast(Caster, Target)
    local item = GetEquippedItemBySlot(Caster, 1)
    if not item or GetItemType(item) ~= 4 then
        SendMessage(Caster, "Must have shield equipped", "yellow")
        return false, 70
    end
```
