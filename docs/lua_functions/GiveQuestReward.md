### Function: GiveQuestReward(quest, spawn)

**Description:**
Provides the previously assigned rewards for a quest via AddQuestReward.. functions and other related functions.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From Quests/AlphaTest/stowaway_antonica.lua
function  QuestCheck1(Quest, QuestGiver, Player)
    if QuestStepIsComplete(Player,5858,2) and QuestStepIsComplete(Player,5858,3)and QuestStepIsComplete(Player,5858,4)  then
	UpdateQuestTaskGroupDescription(Quest, 1, "I have found all I need to Fast-Track to Qeynos.")
	UpdateQuestDescription(Quest, "I have all the necessary parts for the Fast-Track passage to Qeynos. The ride was a bit cramped...")
	GiveQuestReward(Quest, Player)
end
```
