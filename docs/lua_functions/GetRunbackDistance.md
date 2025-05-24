### Function: GetRunbackDistance(spawn)

**Description:**
Gets the distance from the runback point for the current Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Float distance from the runback point.

**Example:**

```lua
-- From SpawnScripts/Generic/NPCModule.lua
function IdleAggressive(NPC)
    if not IsInCombat(NPC) and GetRunbackDistance(NPC)<2 then
        local choice = MakeRandomInt(1,5)
        if choice == 1 then
            PlayFlavor(NPC,"","","scheme",0,0)
        elseif choice == 2 then
            PlayFlavor(NPC,"","","brandish",0,0)
        elseif choice == 3 then
            PlayFlavor(NPC,"","","tapfoot",0,0)
        elseif choice == 4 then
            PlayFlavor(NPC,"","","swear",0,0)
        elseif choice == 5 then
            PlayFlavor(NPC,"","","threaten",0,0)
        end
```
