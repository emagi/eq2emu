### Function: GiveLoot(entity, player, coins, item_id)

**Description:**
Give the specified Player coin and item_id (multiple can be provided comma delimited) as pending loot for their inventory.

**Parameters:**
- `entity` (int32) - Integer value `entity`.
- `player` (Spawn) - Spawn object representing `player`.
- `coins` (int32) - Integer value `coins`.
- `item_id` (int32) - Integer value `item_id`.

**Returns:** None.

**Example:**

```lua
-- From Quests/FarJourneyFreeport/TasksaboardtheFarJourney.lua
function CurrentStep(Quest, QuestGiver, Player)
	if GetQuestStepProgress(Player, 524,2) == 0 and GetQuestStep(Player, 524) == 2 then
		i = 1
		spawns = GetSpawnListBySpawnID(Player, 270010)
		repeat
			spawn = GetSpawnFromList(spawns, i-1)
			if spawn then
				ChangeHandIcon(spawn, 1)
				AddPrimaryEntityCommand(nil, spawn)
				SpawnSet(NPC, "targetable", 1, true, true)
			end
```
