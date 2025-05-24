### Function: GetX(spawn)

**Description:**
Current X position of the spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Float X of the spawn.

**Example:**

```lua
-- From ItemScripts/BardCertificationPapers.lua
function Class(Item, Player)
    conversation = CreateConversation()
    if CanReceiveQuest(Player,Quest) then
    AddConversationOption(conversation, "[Turn in these papers for gear]","QuestStart")
    end
```
