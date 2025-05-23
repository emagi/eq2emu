### Function: AddQuestPrereqRace(quest, race)

**Description:**
CanReceiveQuest requires the Player to be of a specific race ID.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `race` (int32) - Integer value `race`.

**Returns:** None.

**Example:**

```lua
-- Require the Player to have a race of 1 (Dark Elf) for the Quest
function Init(Quest)
    AddQuestPrereqRace(Quest, 1)
end
```
