### Function: RemoveSpellBookEntry(player, spellid)

**Description:**
Removes the spellid from the Player's spell book.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `spellid` (uint32) - Integer value `spellid`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/BrawlerCertificationPapers.lua
function Class(Item, Player)
    conversation = CreateConversation()
    if CanReceiveQuest(Player,Quest) then
    AddConversationOption(conversation, "[Turn in these papers for gear]","QuestStart")
    end
```
