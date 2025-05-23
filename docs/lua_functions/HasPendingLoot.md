### Function: HasPendingLoot(entity, player)

**Description:**
Identifies if the Player has pending loot with the targetted Entity (NPC).

**Parameters:**
- `entity` (int32) - Integer value `entity`.
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/FarJourneyFreeport/atreasurechest.lua
function open(NPC, Player)
    SendStateCommand(NPC, 399)
    AddTimer(NPC, 2000, "finished_open_animation")
    if HasPendingLoot(NPC, Player) then        
        ShowLootWindow(Player, NPC)
        DisplayText(Player, 12, "There appears to be something inside this box.")
        InstructionWindow(Player, -1.0, "This screen shows the contents of the box you just opened. Left click on the loot all button to take the items from the box.", "voiceover/english/narrator/boat_06p_tutorial02/narrator_013_f0780e49.mp3", 1581263773, 1569244108, "tutorial_stage_17", "Left click on the loot all button.", "server")
        SetTutorialStep(player, 16)
    else
        DisplayText(Player, 12, "This box is empty.")
        ChangeHandIcon(NPC, 0)
        SpawnSet(NPC, "targetable", 0)
    end
```
