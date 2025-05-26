### Function: InstructionWindowGoal(player, goal_num)

**Description:**
Set's the players instruction window goal number.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `goal_num` (uint8) - Quantity `goal_num`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/FarJourneyFreeport/CaptainVarlos.lua
function hailed_instructions(NPC, player)	
	if needs_selection_help then
		InstructionWindowGoal(player,0)	
		InstructionWindowClose(player)
		InstructionWindow(player, -1.0, "To respond to the Captain and other characters you will meet, left click on the response text.", "voiceover/english/narrator/boat_06p_tutorial02/narrator_006_7521b625.mp3", 3936228257, 1877316160, "tutorial_stage_8", "Left click on one of the response options.", "server")
		needs_selection_help = false
	end
```
