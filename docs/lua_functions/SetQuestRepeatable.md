### Function: SetQuestRepeatable(quest)

**Description:**
Set's if the quest is repeatable.


**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Antonica/a_brass_key.lua
function Init(Quest)
	SetQuestFeatherColor(Quest, 3)
	SetQuestRepeatable(Quest)
	AddQuestStep(Quest, 1, "From the looks of the key and the tag, it would be safe to assume that this key can be used at a lighthouse of sorts.", 1, 100, "I need to find out what lighthouse this key goes to. The trek maybe dangerous, so I should take other hearty adventurers with me.  From the looks of the key and the tag, it would be safe to assume that this key can be used at a lighthouse of sorts.", 2176)
	AddQuestStepCompleteAction(Quest, 1, "QuestComplete")
end
```
