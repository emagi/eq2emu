### Function: RemoveCoin(Spawn, Amount)

**Description:**
Deducts the specified amount of coin (in copper) from the spawn’s money. If the spawn does not have enough, this could reduce them to zero or potentially go negative (though typically it will not allow negative).

**Parameters:**
- `Spawn`: Spawn - The player from whom to take coin.
- `Amount`: UInt32 - The amount in copper to remove.

**Returns:** Return's true if the Player has enough coin to remove, otherwise return's false.

**Example:**

```lua
-- Example usage (charge a fee of 2 silver, 50 copper i.e., 250 copper total)
RemoveCoin(Player, 250)
```