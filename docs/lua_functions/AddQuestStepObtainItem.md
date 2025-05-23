### Function: AddQuestStepObtainItem(quest, step, description, quantity, percentage, str_taskgroup, icon, obtain_item_id)

**Description:**
Adds a quest step to the player's journal requiring an item be obtained for the complete action to be performed.  Multiple obtain_item_id's can be passed with only one requiring to match.  The player must obtain the item either by looting, item purchase (such as merchant, broker), summon item.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `step` (int32) - Integer value `step`.
- `description` (int32) - Integer value `description`.
- `quantity` (int32) - Integer value `quantity`.
- `percentage` (int32) - Integer value `percentage`.
- `str_taskgroup` (string) - String `str_taskgroup`.
- `icon` (int32) - Integer value `icon`.
- `obtain_item_id` (int32) - Integer value `obtain_item_id`.

**Returns:** None.

**Example:**

```lua
-- From Quests/AlphaTest/stowaway_antonica.lua
function Step1Complete(Quest)
	UpdateQuestStepDescription(Quest, 1, "I have arrived in Antonica mostly intact.")
	AddQuestStepChat(Quest, 2, "I need to meet up with the Shady Swashbuckler near the lighthouse in Antonica.", 1, "The Shady Swashbuckler provided me passage to Antonica. He will have the paperwork I need when I get there.", 11, 121874)
	AddQuestStepObtainItem(Quest, 3, "I must fill out Qeynos Citizenship papers.", 1,100, "The Shady Swashbuckler provided me passage to Antonica. He will have the paperwork I need when I get there.", 75, 1001095)
	AddQuestStepObtainItem(Quest, 4, "I must fill out Class Certification paperwork.", 1,100, "The Shady Swashbuckler provided me passage to Antonica. He will have the paperwork I need when I get there.", 2183, 1001096,1001097,1001098,1001099,1001100,1001101,1001102,1001103,1001104,1001105,1001106,1001107)
	AddQuestStepCompleteAction(Quest, 2, "Step2Complete")
	AddQuestStepCompleteAction(Quest, 3, "Step3Complete")
	AddQuestStepCompleteAction(Quest, 4, "Step4Complete")
end
```
