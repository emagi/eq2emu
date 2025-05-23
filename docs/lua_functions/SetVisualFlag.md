### Function: SetVisualFlag(spawn)

**Description:**
Set the visual flag on the Spawn to initiate a update on the Spawns visual data.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Antonica/HaddekVimki.lua
function spawn(NPC)
	ProvidesQuest(NPC, RunningOutOfBeer)   
	ProvidesQuest(NPC, OuchMyHead)   
	ProvidesQuest(NPC, RiseAndShineWine)   
	ProvidesQuest(NPC, PracticalJokeOnBlarton)   
     SetPlayerProximityFunction(NPC, 10, "InRange", Spawn) 
end
```
