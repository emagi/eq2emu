### Function: SendPopUpMessage(spawn, message, red, green, blue)

**Description:**
Sends a popup message to the Spawn(Player) with the message in the popup.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `message` (string) - String `message`.
- `red` (int32) - Integer value `red`.
- `green` (int32) - Integer value `green`.
- `blue` (int32) - Integer value `blue`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/BardCertificationPapers.lua
function QuestStart(Item,Player)
    OfferQuest(nil,Player,Quest)
    conversation = CreateConversation()
    AddConversationOption(conversation, "[Put the signed certificate away]","TaskDone")
    StartDialogConversation(conversation, 2, Item, Player, "The Shady Swashbuckler might have some gear I can use...")    
end
```
