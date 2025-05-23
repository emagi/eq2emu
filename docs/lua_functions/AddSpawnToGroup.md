### Function: AddSpawnToGroup(spawn, new_group_id)

**Description:**
Combines Spawn into the specified new_group_id which will link the Spawn's together in an encounter and they will engage and assist each other as a group.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `new_group_id` (uint32) - Integer value `new_group_id`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/IsleRefuge1/GoblinSaboteurFirepit.lua
function Grouping(NPC,Spawn)
        local zone = GetZone(NPC)
            Gob1 = GetSpawnByLocationID(zone,133776460)
            Gob2 = GetSpawnByLocationID(zone,133776463)
            Gob3 = GetSpawnByLocationID(zone,133776464)
            Gob4 = GetSpawnByLocationID(zone,133776458)
            Gob5 = GetSpawnByLocationID(zone,133776459)
            AddSpawnToGroup(Gob1,1051222)            
            AddSpawnToGroup(Gob2,1051222)            
            AddSpawnToGroup(Gob3,"1051222")            
            AddSpawnToGroup(Gob4,"1051222")            
            AddSpawnToGroup(Gob5,"1051222") 
end
```
