### Function: HasLanguage(player, language_id)

**Description:**
Return's true if the player has the language_id specified.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `language_id` (uint32) - Integer value `language_id`.

**Returns:** True if the language_id has been obtained by the player, otherwise false.

**Example:**

```lua
-- From ItemScripts/acarvedorcaxe.lua
function Dialog1(Item,Player)
    conversation = CreateConversation()
    if CanReceiveQuest(Player,AnAxesRevenge) then
    AddConversationOption(conversation, "[Run your fingers over the markings]", "Dialog2")
    end
```
