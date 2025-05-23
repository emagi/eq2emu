### Function: AddQuestStepSpell(quest, step, description, quantity, percentage, str_taskgroup, icon, cast_spell_id)

**Description:**
Adds a quest step to the Player's journal requiring that they use a spell id supplied by cast_spell_id.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `step` (int32) - Integer value `step`.
- `description` (int32) - Integer value `description`.
- `quantity` (int32) - Integer value `quantity`.
- `percentage` (int32) - Integer value `percentage`.
- `str_taskgroup` (string) - String `str_taskgroup`.
- `icon` (int32) - Integer value `icon`.

**Returns:** None.

**Example:**

```lua
-- From Quests/BeggarsCourt/dirty_work.lua
function Step2_Complete_Listened(Quest, QuestGiver, Player)
	UpdateQuestStepDescription(Quest, 2, "I have learned the location that the meeting will take place in.")
	UpdateQuestTaskGroupDescription(Quest, 2, "I have learned the location that the meeting will take place in.")
	
	AddQuestStepSpell(Quest, 3, "I must poison the cups in the room in the southeastern area of the Beggar's Court, east of the inn.", 1, 100, "I need to poison the cups at the meeting place.", 0, A_MUG_TO_POISON)
	AddQuestStepCompleteAction(Quest, 3, "Step3_Complete_CupsPoisoned")
end
```
