### Function: SetModelType(spawn, value)

**Description:**
Set's the Spawn's model type to the value supplied, see Model IDs in the Reference Lists https://wiki.eq2emu.com/ReferenceLists by client version for more information.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `value` (int32) - Integer value `value`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/ForestRuins/hiddentools.lua
function casted_on(NPC, Spawn, Message)
	if HasQuest(Spawn, CACHES_QUEST_ID) then
			if Message == "Spinkle nullification powder" then
				AddStepProgress(Spawn, FAVORS_QUEST_ID, 1, 1)
				SetModelType(NPC,5210)
			    	SpawnMob(NPC, toolGuard)
			    	Attack(toolGuard, Spawn)
				AddTimer(NPC,15000,Despawn)
			end
```
