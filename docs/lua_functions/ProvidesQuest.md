### Function: ProvidesQuest(npc, quest_id)

**Description:**
The NPC provides a quest offering to Player's.  This will support the feather flags for players.

**Parameters:**
- `npc` (Spawn) - Spawn object representing `npc`.
- `quest_id` (uint32) - Integer value `quest_id`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Antonica/AGriffonTamer.lua
function spawn(NPC)
	ProvidesQuest(NPC, GriffonEggs)
end
```
