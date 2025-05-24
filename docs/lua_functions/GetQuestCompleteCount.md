### Function: GetQuestCompleteCount(Spawn, QuestID)

**Description:**
Retrieves how many times the specified player (Spawn) has completed a particular quest (identified by QuestID). For non-repeatable quests this is usually 0 or 1; for repeatable quests it could be higher.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.

**Returns:** UInt32 value of how many times the quest was completed.

**Example:**

```lua
-- From Quests/TheSerpentSewer/fresh_samples.lua
function Accepted(Quest, QuestGiver, Player)
    if GetQuestCompleteCount(Player, Quest) == 0 then
	FaceTarget(QuestGiver, Player)
	local conversation = CreateConversation()
	PlayFlavor(QuestGiver, "voiceover/english/marcus_puer/fprt_sewer02/marcuspuer006.mp3", "", "", 2102514737, 183908223, Player)
	AddConversationOption(conversation, "Alright then. ")
	StartConversation(conversation, QuestGiver, Player, "Well I need samples from the creatures down here, of course! You can handle this small request, right?  Of course you can!  Splendid!  Now off with you, off with your adventuring.")
	elseif GetQuestCompleteCount(Player, QUest) > 0 then
	PlayFlavor(QuestGiver, "voiceover/english/marcus_puer/fprt_sewer02/marcuspuer007.mp3", "", "", 794343, 2060215246, Player)
local conversation = CreateConversation()
AddConversationOption(conversation, "I'm on it.")
StartConversation(conversation, QuestGiver, Player, "I need more of the same, really, just bits and pieces, bits and pieces of the creatures down here. Now hop to it.  Remember, they need to be fresh!  The fresher, the more potent, that's what mom always said.")
end
```
