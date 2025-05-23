### Function: AddQuestPrereqTradeskillLevel(quest, level)

**Description:**
CanReceiveQuest requires the Player to be of the tradeskill level defined.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `level` (int32) - Integer value `level`.

**Returns:** None.

**Example:**

```lua
-- Require the Player to have a tradeskill level of 25
function Init(Quest)
    AddQuestPrereqTradeskillLevel(Quest, 25)
end
```
