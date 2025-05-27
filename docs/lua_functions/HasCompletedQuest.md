### Function: HasCompletedQuest(player, quest_id)

**Description:**

Return's true if the player has completed the quest with quest_id.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.

**Returns:** True if the quest_id matches a completed quest for the Player, otherwise false.

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
