### Function: AddLootCoin(Spawn, Amount)

**Description:**
Adds coin to the Spawn's loot table for the current instance when they die.

**Parameters:**
- `Spawn`: Spawn - The spawn or entity to add loot coin.
- `Amount`: int32 - Amount in copper.

**Returns:** None.

**Example:**

```lua
-- Example usage: Add 50 copper to Spawn's loot table for when it dies.
AddLootCoin(Spawn, 50)
```
