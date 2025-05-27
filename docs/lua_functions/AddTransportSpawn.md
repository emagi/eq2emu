### Function: AddTransportSpawn(spawn)

**Description:**

Add's the spawn as a transport spawn in the zone.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/ElddarGrove/TransportTreeLift.lua
function spawn(NPC)
--	AddTransportSpawn(NPC)
--	AddTimer(NPC, 15, "UseLift")
    AddMultiFloorLift(NPC)
end
```
