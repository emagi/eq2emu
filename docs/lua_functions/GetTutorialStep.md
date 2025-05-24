### Function: GetTutorialStep(player)

**Description:**
Get the current tutorial step for the player.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** UInt32 step of the tutorial progress.

**Example:**

```lua
-- From SpawnScripts/FarJourneyFreeport/Vim.lua
function hailed(NPC, player)
    if HasQuest(player,524) and GetQuestStep(player,524)==5 and HasItem(player,12565,1) then
        SetStepComplete(player,524,5)
    end
```
