### Function: SetStepComplete(player, quest_id, step)

**Description:**
Set the Quest step as complete.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.
- `step` (int32) - Integer value `step`.

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
