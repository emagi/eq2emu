### Function: CanReceiveQuest(spawn, quest_id)

**Description:**

Return's true if the Spawn can receive the quest by quest_id.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `quest_id` (uint32) - Integer value `quest_id`.

**Returns:** True if the quest_id can be accepted, otherwise false.

**Example:**

```lua
-- From ItemScripts/abrokenmusicbox.lua
function examined(Item, Player)
	if CanReceiveQuest(Player,RewardForAMissingMusicBox) then
    Dialog1(Item,Player)
    else
    conversation = CreateConversation()
    AddConversationOption(conversation, "[Keep the musicbox]")
    AddConversationOption(conversation, "[Destroy the musicbox]", "QuestFinish")
    StartDialogConversation(conversation, 2, Item, Player, "This floral designed music box is broken like one you found before. Perhaps it is the same one? Oh well.")
	end
```
