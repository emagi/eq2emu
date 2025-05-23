### Function: SetPosition(spawn, x, y, z, heading)

**Description:**
Set's the current position of the Spawn to the x,y,z,heading provided.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `x` (int32) - Integer value `x`.
- `y` (int32) - Integer value `y`.
- `z` (int32) - Integer value `z`.
- `heading` (int32) - Integer value `heading`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Commonlands/ExecutionerSelindi.lua
function LeaveRange(NPC,Spawn)
if HasCompletedQuest(Spawn,5890) and CanReceiveQuest(Spawn,5891) then
    SetPosition(Spawn,-1344.42, -69.53, 333.57, 218.64)
    PlayFlavor(NPC,"","Hey! I need to speak with you!","beckon",0,0,Spawn)
end
```
