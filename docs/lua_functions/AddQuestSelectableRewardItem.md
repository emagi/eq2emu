### Function: AddQuestSelectableRewardItem(quest, item_id, quantity)

**Description:**
Adds a selectable reward item for completion of the current Quest.  Triggered by calling the LUA Function `GiveQuestReward` on completion of the Quest.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `item_id` (uint32) - Integer value `item_id`.
- `quantity` (int32) - Integer value `quantity`.

**Returns:** None.

**Example:**

```lua
-- Made up example

local MARINER_STITCHED_BRACERS_ID = 164053
local MARINER_STITCHED_SHAWL_ID = 164058
local MARINER_STITCHED_SLIPPERS_ID = 164059

function Init(Quest)
	-- allows one item to be selected as an end reward of the quest
	AddQuestSelectableRewardItem(Quest, MARINER_STITCHED_BRACERS_ID)
	AddQuestSelectableRewardItem(Quest, MARINER_STITCHED_SHAWL_ID)
	AddQuestSelectableRewardItem(Quest, MARINER_STITCHED_SLIPPERS_ID)
end
```
