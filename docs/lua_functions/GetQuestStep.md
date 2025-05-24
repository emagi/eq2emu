### Function: GetQuestStep(player, quest_id)

**Description:**
Gets the current quest step of the quest based on the quest_id.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.

**Returns:** UInt32 current step the player is on with the quest.

**Example:**

```lua
-- From ItemScripts/ABloodsabermeddlernote.lua
function decipher(Item, Player)
if GetQuestStep(Player, AnIntriguingEye) == 2 then
SetStepComplete(Player, AnIntriguingEye, 2)
end
```
