### Function: Despawn(spawn, delay)

**Description:**
Despawns the defined spawn, delay is optional, if provided the Despawn will take place after a delay.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `delay` (uint32) - Integer value `delay`.

**Returns:** None.

**Example:**

```lua
-- From Quests/BigBend/gnomore_gnomesteaks.lua
function Step1Complete(Quest, QuestGiver, Player)
	UpdateQuestStepDescription(Quest, 1, "Looks like Ruzb is beyond salvation.")
	UpdateQuestTaskGroupDescription(Quest, 1, "Looks like Ruzb just couldn't keep away from the gnomesteaks. His loss.")
	
	local zone = GetZone(Player)
	local RuzbNPC = GetSpawnByLocationID(zone, 388762, false)
    Despawn(RuzbNPC)
    
    local Ruzb = GetSpawnByLocationID(zone, 133773787, false)
    local SpawnRuzb = SpawnByLocationID(zone, 133773787,false)

    AddQuestStepKill(Quest, 2, "I need to kill Ruzb!", 1, 100, "I need to kill Ruzb after he attacked me.", 91, 1340140)
	AddQuestStepCompleteAction(Quest, 2, "Step2Complete")
end
```
