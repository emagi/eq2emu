### Function: GetPlayersInZone(Zone)

**Description:** Retrieves a list of all player spawns currently present in the specified zone.

**Parameters:**
- `zone` (Zone) - Zone object representing `zone`.

**Returns:** Table - Spawns array list of players in current zone.

**Example:**

```lua
-- From SpawnScripts/Classic_forest/TheBasaltWatcher.lua
function wakeup(NPC)
		local players = GetPlayersInZone(GetZone(NPC))              --zone callout and activation
		for index, player in pairs(players) do
		SendPopUpMessage(player, "Grinding stone can be heard as something ancient stirs in the ruins.", 255, 255, 0)
		SendMessage(player, "Grinding stone can be heard as something ancient stirs in the ruins.","yellow")
    end
```
