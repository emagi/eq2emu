### Function: GetFactionID(spawn)

**Description:**
Gets the faction_id of the current Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 faction_id of the Spawn.

**Example:**

```lua
-- From SpawnScripts/Caves/GuardBelaire.lua
function InRange(NPC, Spawn)
  if GetFactionAmount(Spawn,11)>=5000 then
    if GetLevel(Spawn) ==8 or GetLevel(Spawn)==9 then
    ClassCheck(NPC,Spawn)
    end
    end
    if GetFactionID(Spawn) ==1 then 
        Attack(NPC,Spawn)
    end
end
```
