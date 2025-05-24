### Function: GetArchetypeName(spawn)

**Description:**
Gets the base class of the Spawn and then determines the archetype name of the Spawn.  The C++ code calls GetClassNameCase which changes the case sensitivity in that only the first letter is upper case for the classname.  See https://github.com/emagi/eq2emu/blob/main/docs/data_types/classes.md and use the Display Name.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** The class name in string format.  As listed under Display Name in classes https://github.com/emagi/eq2emu/blob/main/docs/data_types/classes.md

**Example:**

```lua
-- From SpawnScripts/FrostfangSea/KnutOrcbane.lua
function hailed(NPC, Spawn)
	FaceTarget(NPC, Spawn)
	conversation = CreateConversation()
	
	if not HasCompletedQuest(Spawn, NothingWaste) then
		PlayFlavor(NPC, "", "There are some coldain that could use your help.  Speak with Dolur Axebeard or Belka Thunderheart at the Great Shelf.", "nod", 1689589577, 4560189, Spawn)
	elseif not HasCompletedQuest(Spawn, ImpishThreats) and not HasQuest(Spawn, ImpishThreats) then
		PlayFlavor(NPC, "knut_orcbane/halas/cragged_spine/knut_orcbane_004.mp3", "", "", 2960091072, 298935483, Spawn)
		AddConversationOption(conversation, "Thank you.", "Quest1Chat_1")
		AddConversationOption(conversation, "What cause is that?", "Quest1Chat_10")
		AddConversationOption(conversation, "Not me.  I'm just passing through.")
		local archetype = GetArchetypeName(Spawn)
		if archetype == 'Fighter' then
			archetype_message = "strong fighter"
		elseif archetype == 'Mage' then
			archetype_message = 'powerful mage'
		elseif archetype == 'Scout' then
			archetype_message = 'stealthy scout'
		else
			archetype_message = 'caring priest'
		end
```
