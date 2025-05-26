### Function: SetBrainTick(spawn, tick)

**Description:**
Sets the Spawn's brain tick rate in milliseconds.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `tick` (uint16) - Integer value `tick`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/GMHall/TrainingDummy.lua
function spawn(NPC)
	-- set the calls to the ai to 10 mins as there is no ai
	SetBrainTick(NPC, 600000)
	SetLuaBrain(NPC)
	
	-- give the spawn a crap load of hp so we can't one hit kill
	SetHP(NPC, 1000000)
end
```
