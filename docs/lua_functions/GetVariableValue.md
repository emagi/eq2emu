### Function: GetVariableValue(variable_name)

**Description:**
Get the variable value by name of the server.

**Parameters:**
- `variable_name` (string) - String `variable_name`.

**Returns:** String value based on the name.

**Example:**

```lua
-- From SpawnScripts/Generic/aGigglegibberGoblinGamblinGameVendor.lua
function hailed(NPC, Spawn)
	FaceTarget(NPC, Spawn)
	conversation = CreateConversation()

	AddConversationOption(conversation, "Thanks.")
	StartConversation(conversation, NPC, Spawn, "The current jackpot is " .. GetCoinMessage(GetVariableValue("gambling_current_jackpot")) .. ".")

--[[
		PlayFlavor(NPC, "", "", "", 0, 0, Spawn)
	AddConversationOption(conversation, "How did a goblin get in here?  Don't you kill people?", "dlg_0_1")
	AddConversationOption(conversation, "I think I'd rather keep my money, thanks.")
	StartConversation(conversation, NPC, Spawn, "Buy ticket, you!  Only ten shiny coins! You give just ten shiny coins and maybe you get um... many shinier coins!")
	if convo==1 then
		PlayFlavor(NPC, "", "", "", 0, 0, Spawn)
		AddConversationOption(conversation, "How did a goblin get in here?  Don't you kill people?", "dlg_1_1")
		AddConversationOption(conversation, "What do you know about the disappearance of Lord Bowsprite?")
		AddConversationOption(conversation, "I think I'd rather keep my money, thanks.")
		StartConversation(conversation, NPC, Spawn, "Buy ticket, you!  Only ten shiny coins! You give just ten shiny coins and maybe you get um... many shinier coins!")
	end
```
