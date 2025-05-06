### Function: AddLootItem(Spawn, ItemID, Charges)

**Description:**
Add an item and its charges (quantity) to a Spawn's loot table for this instance.

**Parameters:**
- `Spawn`: Spawn - The spawn or entity to add the loot item.
- `ItemID`: int32 - ItemID from the items table.
- `Charges`: int16 - Amount of charges for the item (or quantity), default is 1.

**Returns:** None.

**Example:**

```lua
-- Example usage: Add to Spawn ItemID 1696 (a fishman scale amulet) with Quantity of 1.
AddLootItem(Spawn, 1696)
```
