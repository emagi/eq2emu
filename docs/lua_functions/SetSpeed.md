### Function: SetSpeed(spawn, value)

**Description:**
Set's the speed of the spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `value` (int32) - Integer value `value`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Darklight/awellspringcub.lua
function casted_on(NPC, Caster, SpellName)
    if SpellName == "Leash" then
	    if GetQuestStep(Caster, DrawUponWellsprings) == 1 then
			if GetTempVariable(Caster, "cub") == nil then
				SetTempVariable(Caster, "cub", NPC)
				SpawnSet(NPC, "attackable", "0")
				SpawnSet(NPC, "show_level", "0")
				SetFollowTarget(NPC, Caster)
				SetSpeed(NPC, 9)
				ToggleFollow(NPC)
			end
```
