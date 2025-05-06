### Function: AddItem(Spawn, ItemID, Quantity)

**Description:**
Add an Item to a Spawn/Player with the defined Item ID and Quantity.

**Parameters:**
- `Spawn`: Spawn - The spawn or entity involved.
- `ItemID`: UInt32 - Item id to assign to Spawn.
- `Quantity`: UInt32 - Quantity of item to assign to Spawn (default 1).

**Returns:** Boolean: True if successfully assigned the item, otherwise return's False.

**Example:**

```lua
-- Example usage: Assigns Spawn the ItemID 1696 (a fishman scale amulet) with Quantity of 1.
AddItem(Spawn, 1696)
```
