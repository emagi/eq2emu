### Function: AddQuestPrereqTradeskillClass(quest, class_id)

**Description:**
CanReceiveQuest requires the Player to be of the tradeskill class with class_id.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `class_id` (uint32) - Integer value `class_id`.

**Returns:** None.

**Example:**

```lua
-- Require the Player to have a tradeskill class of 46 (Craftsman)
function Init(Quest)
    AddQuestPrereqTradeskillClass(Quest, 46)
end
```
