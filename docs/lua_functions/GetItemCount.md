### Function: GetItemCount(item)

**Description:**
Get's the item count of the specified Item object.

**Parameters:**
- `item` (Item) - Item object representing `item`.

**Returns:** UInt32 value of the quantity/count available in the current item stack.

**Example:**

```lua
-- From ItemScripts/anaxeedge.lua
function obtained(Item, Player)
if HasQuest(Player, Gnasher) or HasCompletedQuest(Player, Gnasher) or GetItemCount(3560) > 1 then
RemoveItem(Player, 3560)
end
```
