### Function: StopTimer(spawn, function)

**Description:**
Stops an active timer that is to call the `function`.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `function` (int32) - Integer value `function`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/The Deserted Mine/ShyzintheCoercer.lua
function spawnthirdwaveisalivefinal(NPC)
    local zone = GetZone(NPC)
	local firstwavegrp = GetGroup(GetSpawnGroupByID(zone, 4122))
    if not firstwavegrp == nil then
		SetTempVariable(NPC, "combat_notfinish", "0")
        for k,v in ipairs(firstwavegrp) do
			if IsAlive(GetSpawnByLocationID(zone, v)) then
				SetTempVariable(NPC, "combat_notfinish", "1")
				break
			end
```
