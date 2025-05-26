### Function: InstructionWindow(player, duration, text, voice, voice_key1, voice_key2, signal, goal1, task1, goal2, task2, goal3, task3, goal4, task4)

**Description:**
Provides an instruction window for the player.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `duration` (float) - Float value `duration`.
- `text` (string) - String `text`.
- `voice` (string) - String `voice`.
- `voice_key1` (uint32) - Integer value `voice_key1`.
- `voice_key2` (uint32) - Integer value `voice_key2`.
- `signal` (string) - String `signal`.
- `goal1` (string) - String `goal1`.
- `task1` (string) - String `task1`.
- `goal2` (string) - String `goal2`.
- `task2` (string) - String `task2`.
- `goal3` (string) - String `goal3`.
- `task3` (string) - String `task3`.
- `goal4` (string) - String `goal4`.
- `task4` (string) - String `task4`.

**Returns:** None.

**Example:**

```lua
-- From Quests/FarJourneyFreeport/TasksaboardtheFarJourney.lua
	InstructionWindow(Player, -1.0, "The items are now in your inventory.", "voiceover/english/narrator/boat_06p_tutorial02/narrator_014_eaa89ef7.mp3", 361706387, 1106127199, "tutorial_stage_18", "", "continue")		
```
