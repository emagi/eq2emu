### Function: SetPlayerHistory(player, event_id, value, value2)

**Description:**
Set's the players history to the value provided.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `event_id` (uint32) - Integer value `event_id`.
- `value` (uint32) - Integer value `value`.
- `value2` (uint32) - Integer value `value2`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Antonica/hunters_manifest.lua
function Accepted(Quest, QuestGiver, Player)
SetPlayerHistory(Player, 8, 1)
end 
```
