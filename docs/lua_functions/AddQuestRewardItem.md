### Function: AddQuestRewardItem(quest, item_id, quantity)

**Description:**
Adds a rewarded item for completion of the current Quest.  Triggered by calling the LUA Function `GiveQuestReward` on completion of the Quest.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `item_id` (uint32) - Integer value `item_id`.
- `quantity` (int32) - Integer value `quantity`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Peatbog/FarSeasDirectRequisitionPBG0162.lua

local MARINER_STITCHED_BRACERS_ID = 164053
local MARINER_STITCHED_SHAWL_ID = 164058
local MARINER_STITCHED_SLIPPERS_ID = 164059

function Init(Quest)
	local chance = math.random(1, 3)
	if chance == 1 then
		AddQuestRewardItem(Quest, MARINER_STITCHED_BRACERS_ID)
	elseif chance == 2 then
		AddQuestRewardItem(Quest, MARINER_STITCHED_SHAWL_ID)
	elseif chance == 3 then
		AddQuestRewardItem(Quest, MARINER_STITCHED_SLIPPERS_ID)
	end

	SetQuestFeatherColor(Quest, 3)
	SetQuestRepeatable(Quest)
	AddQuestStepKill(Quest, 1, "I must kill some bog slugs", 10, 100, "I must hunt down the creatures in the Peat Bog to fill the requisition.", 289, BOG_SLUG_ID)
	AddQuestStepCompleteAction(Quest, 1, "Step1Complete")
end
```
