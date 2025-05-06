### Function: AddCoin(Player, Amount)

**Description:**
Provide the Amount of coin provided in copper which will translate the value into copper, silver, gold, platinum respectively to the Player.

**Parameters:**
- `Player`: Spawn - The Spawn (Player) to provide the coin.
- `Amount`: UInt32 - Amount in copper to provide Player.

**Returns:** None.

**Example:**

```lua
-- Example usage, provide 5 copper to Player. 50 copper = 5 silver, 500 copper = 5 gold, 5000 copper = 5 platinum.
AddCoin(Player, 5)
```
