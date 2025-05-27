### Function: CloseConversation(npc, player)

**Description:**

Closes a Player's active conversation with the NPC.

**Parameters:**
- `npc` (Spawn) - Spawn object representing `npc`.
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** None.

**Example:**

```lua
-- From Quests/CastleviewHamlet/the_lost_book_of_arbos.lua
function Accepted(Quest, QuestGiver, Player)
 	PlayFlavor(QuestGiver, "", "", "thanks", 0,0 , Player)
 	CloseConversation(QuestGiver,Player)
end
```
