### Function: AddQuestRewardCoin(quest, copper, silver, gold, plat)

**Description:**
Upon completion of the quest the Player will receive the copper, silver, gold, platinum described.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `copper` (int32) - Integer value `copper`.
- `silver` (int32) - Integer value `silver`.
- `gold` (int32) - Integer value `gold`.
- `plat` (int32) - Integer value `plat`.

**Returns:** None.

**Example:**

```lua
-- From Quests/EnchantedLands/Drodo'sGoodies.lua
function Init(Quest)
	AddQuestRewardCoin(Quest, math.random(10,95), math.random(39,49), math.random(1,1), 0)
	AddQuestStepKill(Quest, 1, "I must hunt grove badgers.", 1, 40, "I must hunt the critters near the granary in Enchanted Lands. They should have Drodo's goodies.", 2299, 390041)
	AddQuestStepKill(Quest, 2, "I must hunt lancer wasps.", 1, 40, "I must hunt the critters near the granary in Enchanted Lands. They should have Drodo's goodies.", 1229, 390092)
	AddQuestStepKill(Quest, 3, "I must hunt klakrok drones.", 1, 40, "I must hunt the critters near the granary in Enchanted Lands. They should have Drodo's goodies.", 1225, 390069)
	AddQuestStepKill(Quest, 4, "I must hunt briarpaw cubs.", 1, 40, "I must hunt the critters near the granary in Enchanted Lands. They should have Drodo's goodies.", 928, 390104)
	AddQuestStepCompleteAction(Quest, 1, "deck")
	AddQuestStepCompleteAction(Quest, 2, "comb")
	AddQuestStepCompleteAction(Quest, 3, "dice")
	AddQuestStepCompleteAction(Quest, 4, "box")
end
```
