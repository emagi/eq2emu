### Function: QuestIsComplete(player, quest_id)

**Description:**
Identifies if the quest has been completed or not based on the quest_id.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Baubbleshire/helping_some_friends.lua
function step2_complete_talkedToDrundo(Quest, QuestGiver, Player)
	UpdateQuestStepDescription(Quest, 2, "I have given Drundo the walnut pie.")
	
	if QuestIsComplete(Player, HELPING_SOME_FRIENDS) then
		pranks_given(Quest, QuestGiver, Player)
	end
```
