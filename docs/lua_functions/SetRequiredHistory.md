### Function: SetRequiredHistory(spawn, event_id, value1, value2, flag_override)

**Description:**
Set required history to interact with the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `event_id` (uint32) - Integer value `event_id`.
- `value1` (int32) - Integer value `value1`.
- `value2` (int32) - Integer value `value2`.
- `flag_override` (int32) - Integer value `flag_override`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Nektropos1/BasementSecretDoor.lua
function spawn(NPC)
	SetRequiredHistory(NPC, HISTORY.NEK_CASTLE_BASEMENT_ACCESS, 1)
end
```
