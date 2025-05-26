### Function: SetSpawnAnimation(spawn, anim_id, leeway)

**Description:**
Set's the spawn animation of the spawn, leeway adds a delay in milliseconds.  See appearance id's at https://wiki.eq2emu.com/ReferenceLists for animation ids.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `anim_id` (uint32) - Integer value `anim_id`.
- `leeway` (uint16) - Integer value `leeway`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Antonica/anearthcrawler.lua
function spawn(NPC, Spawn)
    NPCModule(NPC, Spawn)
    SetSpawnAnimation(NPC, 13016)
end
```
