### Function: GetRandomSpawnByID(spawnref, spawn_id)

**Description:**
Gets a random spawn by spawn_id where spawnref is located (zone area).

**Parameters:**
- `spawnref` (Spawn) - Spawn object representing `spawnref`.
- `spawn_id` (uint32) - Integer value `spawn_id`.

**Returns:** Spawn object reference matching the spawn_id.

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
