### Function: SetCompleteFlag(quest)

**Description:**
Set the quest flag as completed.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Darklight/ASolidifiedFront.lua
function QuestComplete(Quest, QuestGiver, Player)
	SetCompleteFlag(Quest)
	GiveQuestReward(Quest, Player)
end
```
