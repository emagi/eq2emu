### Function: GetSpawnLocationID(spawn)

**Description:**
Get a spawn location id.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 spawn location id of the Spawn.

**Example:**

```lua
-- From SpawnScripts/Antonica/aDarkpawyouth.lua
function spawn(NPC, Spawn)
    NPCModule(NPC, Spawn)
    if GetSpawnLocationID(NPC)==  133785089 or GetSpawnLocationID(NPC)==   133785090 then
        AddTimer(NPC,MakeRandomInt(1000,3500),"Run")
    else
        IdleBored(NPC)
    end
```
