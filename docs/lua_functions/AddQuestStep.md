### Function: AddQuestStep(quest, step, description, quantity, percentage, str_taskgroup, icon, usableitemid)

**Description:**
Adds a displayable step in the players quest journal to help them determine their next action to Complete the step.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `step` (int32) - Integer value `step`.
- `description` (int32) - Integer value `description`.
- `quantity` (int32) - Integer value `quantity`.
- `percentage` (int32) - Integer value `percentage`.
- `str_taskgroup` (string) - String `str_taskgroup`.
- `icon` (int32) - Integer value `icon`.
- `usableitemid` (uint32) - Integer value `usableitemid`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Antonica/ancient_kite_shield.lua
function Step1Complete(Quest, QuestGiver, Player)
	UpdateQuestStepDescription(Quest, 1, "I've rinsed a lot of dirt from the shield.")
    AddQuestStep(Quest,2,"I should inspect the shield again.",1, 100,"If I'm going to clean this shield and return to working condition, I'll need to put some effort into it.", 2268)
	AddQuestStepCompleteAction(Quest, 2, "Step2Complete")
end    
```
