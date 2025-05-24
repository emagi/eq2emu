### Function: GetQuestFlags(Quest)

**Description:**
Retrieves the bitwise flags set for a quest. These flags might denote various quest properties (such as hidden, completed, failed, etc.). This is more of an internal function for quest data management.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.

**Returns:** UInt32 set of quest flags

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
	
	SetTempVariable(Player, "HelpingSarmaSingebellows", nil)
	
	if GetQuestFlags(Quest) == 0 then
		local quantity = math.random(8, 12)
		local flags = 0
		
		if quantity == 8 then
			flags = flags + kill8
		elseif quantity == 9 then
			flags = flags + kill9
		elseif quantity == 10 then
			flags = flags + kill10
		elseif quantity == 11 then
			flags = flags + kill11
		elseif quantity == 12 then
			flags = flags + kill12
		end
		
		SetQuestFlags(Quest, flags)
		SetStep(Quest, Player, quantity)
		
	else -- need the else for /reload quest
		CheckBitMask(Quest, Player, GetQuestFlags(Quest))
	end
end

function hasflag(flags, flag)
	return flags % (2*flag) >= flag
end

function CheckBitMask(Quest, Player, Flags)
	local quantity = 0
	
	if hasflag(Flags, kill8) then
		quantity = 8
	elseif hasflag(Flags, kill9) then
		quantity = 9
	elseif hasflag(Flags, kill10) then
		quantity = 10
	elseif hasflag(Flags, kill11) then
		quantity = 11
	elseif hasflag(Flags, kill12) then
		quantity = 12
	end
	
	SetStep(Quest, Player, quantity)
end
```
