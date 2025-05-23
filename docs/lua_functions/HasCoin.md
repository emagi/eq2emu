### Function: HasCoin(Spawn, Value)

**Description:** Checks if a player (Spawn) has at least a certain amount of coin (Value in copper) on them.

**Parameters:**
- `Spawn`: Spawn – The player whose coin purse to check.
- `Value`: Int32 – The amount of coin (in copper units) to check for.

**Returns:** Boolean – true (1) if the player has at least that amount of coin; false (0) if they do not have enough money.

**Example:**

```lua
-- Example usage (charging a fee if the player can afford it)
if HasCoin(Player, 5000) then  -- 5000 copper = 50 silver
    RemoveCoin(Player, 5000)
    SendMessage(Player, "You pay 50 silver for the item.")
else
    SendMessage(Player, "You don't have enough coin.")
end
```