### Function: AddQuestStepCompleteAction(quest, step, action)

**Description:**
Sets the function to call when the quest step is completed.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `step` (int32) - Integer value `step`.
- `action` (int32) - Integer value `action`.

**Returns:** None.

**Example:**

```lua
-- From Quests/AlphaTest/stowaway_antonica.lua
function Init(Quest)
	AddQuestStepZoneLoc(Quest, 1, "I must ride to Antonica...", 12, "The Shady Swashbuckler provided me passage to Antonica. He will have the paperwork I need when I get there.", 2285, 395.79, -38.56, 809.38,12)
	AddQuestStepCompleteAction(Quest, 1, "Step1Complete")
end

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
