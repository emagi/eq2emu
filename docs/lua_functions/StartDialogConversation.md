### Function: StartDialogConversation(conversation, type, spawn, player, text, mp3, key1, key2, language, can_close)

**Description:**
Begin a conversation with the Player from the Source Spawn with the previously established conversation options.

**Parameters:**
- `conversation` (Conversation) - Conversation object representing `conversation`.
- `type` (int32) - Integer value `type`.
- `item` (Item) - Item object representing `item`.
- `player` (Spawn) - Spawn object representing `player`.
- `text` (string) - String `text`.
- `mp3` (int32) - Integer value `mp3`.
- `key1` (int32) - Integer value `key1`.
- `key2` (int32) - Integer value `key2`.
- `language` (int32) - Integer value `language`.
- `can_close` (bool) - Boolean flag `can_close`.

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
