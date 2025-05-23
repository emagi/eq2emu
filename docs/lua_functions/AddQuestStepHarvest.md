### Function: AddQuestStepHarvest(quest, step, description, quantity, percentage, str_taskgroup, icon, harvested_item_id)

**Description:**
Adds a quest step to the player's journal requiring a harvest related action be performed.  The result expected item can be specified as a number of last arguments crafted_item_id of the item to harvest.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `step` (int32) - Integer value `step`.
- `description` (int32) - Integer value `description`.
- `quantity` (int32) - Integer value `quantity`.
- `percentage` (int32) - Integer value `percentage`.
- `str_taskgroup` (string) - String `str_taskgroup`.
- `icon` (int32) - Integer value `icon`.
- `harvested_item_id` (int32) - Integer value `harvested_item_id`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Antonica/meteoric_hoop.lua
function Step2Complete(Quest, QuestGiver, Player)
	UpdateQuestStepDescription(Quest, 2, "I've applied the ink and made the chunk a lot darker.")

		AddQuestStepHarvest(Quest, 3, "I need to harvest some Sandwashed Rock to make a hoop.", 1, 100, "I need to harvest a lot of Sandwashed Rock to make a hoop.", 1186, 121172)
	AddQuestStepCompleteAction(Quest, 3, "Step3Complete")
end
```
