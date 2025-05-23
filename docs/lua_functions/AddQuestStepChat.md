### Function: AddQuestStepChat(quest, step, description, quantity, str_taskgroup, icon, npc_id)

**Description:**
Adds a displayable chat step in the players quest journal to point them towards an NPC their next action to Complete the step.  The npc_id will need to match the destination NPC's spawn id if you select them and do /spawn details or refer to the spawn table in the database.  This supports multiple npc_id's where only one must match.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `step` (int32) - Integer value `step`.
- `description` (int32) - Integer value `description`.
- `quantity` (int32) - Integer value `quantity`.
- `str_taskgroup` (string) - String `str_taskgroup`.
- `icon` (int32) - Integer value `icon`.
- `npc_id` (int32) - Integer value `npc_id`.

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
