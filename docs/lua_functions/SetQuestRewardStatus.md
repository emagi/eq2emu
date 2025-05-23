### Function: SetQuestRewardStatus(quest, status, min_status_earned, max_status_earned)

**Description:**
Set's the quest reward status.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `status` (int32) - Integer value `status`.
- `min_status_earned` (int32) - Integer value `min_status_earned`.
- `max_status_earned` (int32) - Integer value `max_status_earned`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Antonica/these_boots_were_made_for.lua
function Init(Quest)
	AddQuestStepHarvest(Quest, 1, "Harvest 100 iron clusters in Antonica for Hwal, making sure to save them to give to him.", 100, 100, "Hwal needs me to gather the raw materials for the weapons he's planning to make for the sentries.", 1085,  8395)
		AddQuestStepHarvest(Quest, 2, "Harvest 100 severed maple in Antonica for Hwal, making sure to save the wood to give to him.", 100, 100, "Hwal needs me to gather the raw materials for the weapons he's planning to make for the sentries.", 824,  12101)
	AddQuestStepCompleteAction(Quest, 1, "Step1Complete")
	AddQuestStepCompleteAction(Quest, 2, "Step2Complete")
	SetQuestFeatherColor(Quest, 1) -- PURPLE FOR HERITAGE QUESTS
	SetQuestRewardStatus(Quest, 29666)
end
```
