### Function: SetMount(spawn, value)

**Description:**
Set's the Spawn's mount to the model type supplied as the value, see Model IDs in the Reference Lists https://wiki.eq2emu.com/ReferenceLists by client version for more information.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `value` (int32) - Integer value `value`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Antonica/AGriffonTamer.lua
function QeynosToOracle(NPC, Spawn)
	StartAutoMount(Spawn, 85)
	SetMount(Spawn, 225)
end
```
