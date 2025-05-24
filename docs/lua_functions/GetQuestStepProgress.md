### Function: GetQuestStepProgress(player, quest_id, step_id)

**Description:**
Gets the current progress of the quest step for the player.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.
- `step_id` (uint32) - Integer value `step_id`.

**Returns:** UInt32 current progress of the step.

**Example:**

```lua
-- From ItemScripts/abixieeye.lua
function examined(Item, Player)
    local LnLAccept = GetRuleFlagFloat("R_World", "LoreAndLegendAccept")
if LnLAccept > 0 and not HasQuest(Player, LoreAndLegendBixie) and not HasCompletedQuest(Player, LoreAndLegendBixie) then
    OfferQuest(nil, Player, LoreAndLegendBixie)
else
    conversation = CreateConversation()    
if  HasQuest(Player, LoreAndLegendBixie) and  GetQuestStepProgress(Player, LoreAndLegendBixie, 4)==0 then
    AddConversationOption(conversation, "Begin to study...", "Step_Complete")
end
```
