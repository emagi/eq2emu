### Function: GetTradeskillClassName(spawn)

**Description:**
Gets the tradeskill class name of the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** String name of the Tradeskill Class.

**Example:**

```lua
-- From SpawnScripts/Generic/GenericCraftingTrainer.lua
function Chat5(NPC, Spawn)
	FaceTarget(NPC, Spawn)
	conversation = CreateConversation()

	AddConversationOption(conversation, "My name is " .. GetName(Spawn) .. ".", "Send" .. GetTradeskillClassName(Spawn) .. "Choice")
	StartConversation(conversation, NPC, Spawn, "I'm glad that you continued on as a " .. GetTradeskillClassName(Spawn) .. " and came back to advance your skills.  I can certify you in your chosen trade specialty. I need your name before I can start your paperwork.")
end
```
