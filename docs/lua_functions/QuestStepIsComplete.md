### Function: QuestStepIsComplete(player, quest_id, step_id)

**Description:**
Returns if the Player has completed the step_id in the specified quest_id.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.
- `step_id` (uint32) - Integer value `step_id`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/aboarfiendhoof.lua
function examined(Item, Player)
if not HasQuest(Player, LoreAndLegendBoarfiend) and not HasCompletedQuest(Player, LoreAndLegendBoarfiend) then
OfferQuest(nil, Player, LoreAndLegendBoarfiend)
elseif not QuestStepIsComplete(Player, LoreAndLegendBoarfiend, 4) then
conversation = CreateConversation()    
AddConversationOption(conversation, "Begin to study...", "Step_Complete")
AddConversationOption(conversation, "No, put away", "CloseItemConversation")
StartDialogConversation(conversation, 2, Item, Player, "This item can be used to learn the secrets of the boarfiend. Do you wish to study it?")
end
```
