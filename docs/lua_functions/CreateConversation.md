### Function: CreateConversation(conversation)

**Description:**

Create a new conversation option for the Spawn to use in AddConversationOption.

**Parameters:**
- `conversation` (Conversation) - Conversation object representing `conversation`.

**Returns:** None.

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
