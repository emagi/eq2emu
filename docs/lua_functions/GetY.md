### Function: GetY(spawn)

**Description:**
Current Y position of the spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Float Y of the spawn.

**Example:**

```lua
-- From ItemScripts/BardCertificationPapers.lua
function Class(Item, Player)
    conversation = CreateConversation()
    if CanReceiveQuest(Player,Quest) then
    AddConversationOption(conversation, "[Turn in these papers for gear]","QuestStart")
    end
```
