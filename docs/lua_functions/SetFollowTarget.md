### Function: SetFollowTarget(spawn, target, follow_distance)

**Description:**
Set's the spawns follow target and the optional follow_distance.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `target` (Spawn) - Spawn object representing `target`.
- `follow_distance` (uint32) - Distance `follow_distance`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/BeggarsCourt/arat.lua
function CatCheck(NPC,Spawn)
    if IsAlive(NPC) then
        AddTimer(NPC,MakeRandomInt(2000,4000),"CatCheck")

        local zone = GetZone(NPC)
        local Cat = GetSpawnByLocationID(zone,402996)
        if not IsInCombat(NPC) and Cat~= nil and not IsInCombat(Cat)then
        local Distance = GetDistance(NPC,Cat,1)
        if Distance <=5 then
            Attack(Cat,NPC)
            Attack(NPC,Cat)
            local x = GetX(Cat)
            local y = GetY(Cat)
            local z = GetZ(Cat)
            SetFollowTarget(Cat,NPC)
            SetFollowTarget(NPC,Cat)
            ToggleFollow(Cat)
            ToggleFollow(NPC)
            SetTarget(Cat,NPC)
            FaceTarget(Cat,NPC)
            FaceTarget(NPC,Cat)
            PlayFlavor(NPC,"","","attack",0,0)
            AddTimer(NPC,MakeRandomInt(2500,4500),"kill",1,Spawn)
      end
```
