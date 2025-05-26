### Function: SetInfoStructString(spawn, field, value)

**Description:**
Sets the string field to the value provided.  See https://github.com/emagi/eq2emu/blob/main/docs/data_types/info_struct.md for field types.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `field` (string) - String `field`.
- `value` (string) - String `value`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Antonica/anoverlandminer.lua
function spawn(NPC, Spawn)
    NPCModule(NPC, Spawn)
    dwarf(NPC)
    SetInfoStructString(NPC, "action_state", "mining_digging")
end
```
