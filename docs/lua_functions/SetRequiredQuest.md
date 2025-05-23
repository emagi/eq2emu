### Function: SetRequiredQuest(spawn, quest_id, quest_step, flag_override)

**Description:**
Set a required quest to access the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `quest_id` (uint32) - Integer value `quest_id`.
- `quest_step` (int32) - Integer value `quest_step`.
- `flag_override` (int32) - Integer value `flag_override`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Antonica/furlinedglovesdownhay.lua
function spawn(NPC)
	SetRequiredQuest(NPC, 5815, 3)
end
```
