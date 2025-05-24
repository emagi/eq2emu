### Function: GetSpawnListBySpawnID(spawn, spawn_id)

**Description:**
Gets a spawn list by the spawn_id.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `spawn_id` (uint32) - Integer value `spawn_id`.

**Returns:** Table - spawn list based on the database spawn_id.

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
