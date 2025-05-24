### Function: GetPlayerHistory(Player, HistoryID)

**Description:**
Retrieves the value of a specific player history flag. This tells if a player has a certain historical event or choice recorded (and what value it is).

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `event_id` (uint32) - Integer value `event_id`.

**Returns:** Value of the historical value of SetPlayerHistory.

**Example:**

```lua
-- From SpawnScripts/Antonica/CaptainBeltho.lua
function hailed(NPC, Spawn)
    SetPlayerHistory(Spawn, 8, 0)
    if GetPlayerHistory(Spawn, 8) == nil then
    Say(Spawn, "ur player history is nil")
    elseif GetPlayerHistory(Spawn, 8) then
    Say(Spawn, "ur player history is not nil")
    end
```
