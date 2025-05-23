### Function: SetInfoFlag(spawn)

**Description:**
Enables the info flag to trigger a spawn update.

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
