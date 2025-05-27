### Function: AddStepProgress(player, quest_id, step, progress)

**Description:**

Adds progress to the Player's quest based on the quest_id and step.  Setting the current progress value.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.
- `step` (uint32) - Integer value `step`.
- `progress` (uint32) - Integer value `progress`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/OrcSmugglerRequisition.lua
function examined(Item, Player)
if not HasQuest(Player, AnOrderOfOrcTongue) and not HasCompletedQuest(Player, AnOrderOfOrcTongue) then
OfferQuest(nil, Player, AnOrderOfOrcTongue)
elseif HasQuest(Player, AnOrderOfOrcTongue) then
AddStepProgress(Player, AnOrderOfOrcTongue, 1, 1)
RemoveItem(Player, 10202)
end
```
