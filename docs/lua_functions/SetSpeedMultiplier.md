### Function: SetSpeedMultiplier(spawn, val)

**Description:**
Set's the speed multiplier of the spell.  Must be in a spell script.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `val` (float) - Float value `val`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Nektropos1/ArchfiendIzzoroth.lua
function spawn(NPC)
	SetSpeedMultiplier(NPC, 0)
	AddTimer(NPC, 6000, "MakeAttackable")
end
```
