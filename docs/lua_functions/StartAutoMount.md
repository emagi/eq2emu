### Function: StartAutoMount(player, path)

**Description:**
Start the auto mount for the Player on a path id.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `path` (uint32) - Integer value `path`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Antonica/AGriffonTamer.lua
function QeynosToSteppes(NPC, Spawn)
	StartAutoMount(Spawn, 84)
	SetMount(Spawn, 225)
end
```
