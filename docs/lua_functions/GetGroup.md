### Function: GetGroup(Spawn)

**Description:**
Returns an array of Spawns that the given spawn (player or NPC) belongs to. This can be used to iterate over group members or to perform group-wide actions.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** An array of Spawn objects that represents the spawn’s group. If the spawn is not in a group, this may return nil.

**Example:**

```lua
-- From SpawnScripts/Generic/AlexaLockets.lua -- v is the Spawn object reference, k is the position in the array.
function hailed(NPC, Spawn)
	if GetTempVariable(NPC, "talking") ~= "true" then
		StartDialogLoop(NPC, Spawn)
		local player_group = GetGroup(Spawn)
		if player_group ~= nil then
			for k,v in ipairs(player_group) do
				SetPlayerHistory(v, HISTORY.NEK_CASTLE_LIBRARY_ACCESS, 1)
			end
```
