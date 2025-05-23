### Function: AddQuestPrereqFaction(quest, faction_id, min, max)

**Description:**
CanReceiveQuest will fail if the Player does not have the faction_id within the min or max values.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `faction_id` (uint32) - Integer value `faction_id`.
- `min` (int32) - Integer value `min`.
- `max` (int32) - Integer value `max`.

**Returns:** None.

**Example:**

```lua
-- Require faction id 123 with 50 - 100000 faction to allow the quest.
function Init(Quest)
    AddQuestPrereqFaction(Quest, 123, 50, 100000)
end
```
