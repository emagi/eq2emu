### Function: SetLuaBrain(spawn)

**Description:**
Creates a new LUA Brain for the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

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
