### Function: SetTradeskillLevel(spawn, level)

**Description:**
Sets the tradeskill level on the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `level` (uint8) - Integer value `level`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Hallmark/cellar_cleanup.lua
function Step8Complete(Quest, QuestGiver, Player)
	UpdateQuestStepDescription(Quest, 8, "I spoke with Assistant Dreak.")
	UpdateQuestTaskGroupDescription(Quest, 2, "I told Assistant Dreak that the cellar is clean.")
    UpdateQuestZone(Quest,"Mizan's Cellar")
    if not HasItem(Player,20708,1) and GetTradeskillLevel(Player) <2  then
    SummonItem(Player,1030001,1)
    GiveQuestItem(Quest, Player, "", 20708,1001034,1001034,1001034,7391,7391,7391)
    end
```
