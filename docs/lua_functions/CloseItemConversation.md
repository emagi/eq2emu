### Function: CloseItemConversation(item, player)

**Description:**

Closes a conversation of the Player with an Item script.

**Parameters:**
- `item` (Item) - Item object representing `item`.
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/abixieeye.lua
function Step_Complete(Item, Player)
if HasItem(Player,1219,1) then
    SetStepComplete(Player, LoreAndLegendBixie, 4)
    CloseItemConversation(Item, Player)
    RemoveItem(Player, 1219)
end
```
