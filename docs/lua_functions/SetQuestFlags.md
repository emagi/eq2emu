### Function: SetQuestFlags(quest, flags)

**Description:**
Sets the quest flags for the quest object.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `flags` (uint32) - Integer value `flags`.

**Returns:** None.

**Example:**

```lua
-- From Quests/EnchantedLands/HelpingSarmaSingebellows.lua
function Accepted(Quest, QuestGiver, Player)
	
	if GetTempVariable(Player, "HelpingSarmaSingebellows") == "true" then
		PlayFlavor(NPC, "voiceover/english/sarma_singebellows/enchanted/sarma_singebellows002.mp3", "", "", 2943069626, 2445316031, Spawn)
		AddConversationOption(conversation, "I shall return when they are destroyed.")
		StartConversation(conversation, NPC, Spawn, "Excellent!  You worked hard to kill all of those goblins, but we need to make sure they don't regain their foothold.")
	else
		PlayFlavor(NPC, "voiceover/english/sarma_singebellows/enchanted/sarma_singebellows002.mp3", "", "", 2943069626, 2445316031, Spawn)
		AddConversationOption(conversation, "As you wish.")
		StartConversation(conversation, NPC, Spawn, "Excellent! Goblins are tainting the water and withering the trees at a watermill by a nearby lake.  I want you to destroy as many of them as you can!")
	end
```
