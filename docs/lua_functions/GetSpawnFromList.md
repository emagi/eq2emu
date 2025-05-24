### Function: GetSpawnFromList(spawns, position)

**Description:**
Placeholder description.

**Parameters:**

- `spawns` (Table) - List of spawn object references `spawns`.
- `position` (uint32) - Integer value `position`.

**Returns:** Spawn at position.

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
