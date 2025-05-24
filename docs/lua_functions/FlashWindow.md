### Function: FlashWindow(player, window_name, flash_seconds)

**Description:**
Causes a UI window to flash (usually its icon or border) on the client to draw attention. For example, flashing the quest journal icon when a new quest is added.

**Parameters:**
- `player` (Spawn) – The player whose UI to affect.
- `window_name` (string) – The window or UI element name to flash.
- `flash_seconds`(float) - Flash time of the window in seconds.

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
