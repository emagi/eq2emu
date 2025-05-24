### Function: GetTradeskillClass(spawn)

**Description:**
Gets the tradeskill class id of the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 tradeskill class id of the spawn.

**Example:**

```lua
-- From SpawnScripts/FarJourneyFreeport/CaptainVarlos.lua
function ClassSet(NPC,player)
	SetAdventureClass(player,0)
	SendMessage(player, "You are now a Commoner.")
    SendPopUpMessage(player, "You are now a Commoner.", 255, 255, 255)
    SetPlayerLevel(player,1)
if GetTradeskillClass(player)>0 then
    SetTradeskillClass(player,0)
end
```
