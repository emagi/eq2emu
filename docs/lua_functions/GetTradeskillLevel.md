### Function: GetTradeskillLevel(spawn)

**Description:**
Gets the tradeskill level of the spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 tradeskill level of spawn.

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
