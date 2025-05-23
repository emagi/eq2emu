### Function: AddQuestPrereqQuest(quest, quest_id)

**Description:**
CanReceiveQuest checks if the Player has previously completed the quest_id defined.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `quest_id` (uint32) - Integer value `quest_id`.

**Returns:** None.

**Example:**

```lua
-- From Quests/FrostfangSea/the_absent_effigy.lua
function Init(Quest)
	AddQuestRewardCoin(Quest, math.random(10,80), math.random(6,15), 0, 0)
    AddQuestPrereqQuest(Quest, LostFroglok) -- change quest step to obtain item 'an Effigy of Mithaniel' drop from frigid whirlstorms/ The Deadly Icewind
	AddQuestStepKill(Quest, 1, "I must kill frigid whirlstorms to find Splorpy's Effigy of Mithaniel.", 1, 75, "I should kill frigid whirlstorms around Gwenevyn's Cove to find Splorpy's Effigy of Mithaniel.", 1059, 4700054, 4700069)
	AddQuestStepCompleteAction(Quest, 1, "GotEffigy")
end
```
