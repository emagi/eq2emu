### Function: HasSkill(player, skill_id)

**Description:**
Return's true if the player has the skill_id specified

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `skill_id` (uint32) - Integer value `skill_id`.

**Returns:** True if the player has the skill_id, otherwise false.

**Example:**

```lua
-- From ItemScripts/BardCertificationPapers.lua
function Class(Item, Player)
    conversation = CreateConversation()
    if CanReceiveQuest(Player,Quest) then
    AddConversationOption(conversation, "[Turn in these papers for gear]","QuestStart")
    end
```
