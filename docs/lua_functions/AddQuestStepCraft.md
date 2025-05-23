### Function: AddQuestStepCraft(quest, step, description, quantity, percentage, str_taskgroup, icon, crafted_item_id)

**Description:**
Adds a quest step to the player's journal requiring a craft related action be performed.  The result expected item can be specified as a number of last arguments crafted_item_id.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `step` (int32) - Integer value `step`.
- `description` (int32) - Integer value `description`.
- `quantity` (int32) - Integer value `quantity`.
- `percentage` (int32) - Integer value `percentage`.
- `str_taskgroup` (string) - String `str_taskgroup`.
- `icon` (int32) - Integer value `icon`.
- `crafted_item_id` (int32) - Integer value `crafted_item_id`.

**Returns:** None.

**Example:**

```lua
-- From Quests/CityofFreeport/the_stein_of_moggok_it_can_be_rebuilt.lua
function Step7Complete(Quest, QuestGiver, Player)
	UpdateQuestStepDescription(Quest, 7, "I spoke with Rumdum.")
	UpdateQuestTaskGroupDescription(Quest, 4, "I spoke with Rumdum.")
    GiveQuestItem(Quest, Player, "I spoke with Rumdum." ,  13961, 14072, 31562)
	AddQuestStepCraft(Quest, 8, "I need to remake the Stein of Moggok.", 1, 100, "I need to remake the Stein of Moggok using Rumdum's family Recipe.", 11, 54775)
	AddQuestStepCompleteAction(Quest, 8, "QuestComplete")
end
```
