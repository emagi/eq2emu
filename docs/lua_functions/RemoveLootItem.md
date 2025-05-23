### Function: RemoveLootItem(spawn, item_id)

**Description:**
Removes a loot item from the Spawn(NPC).

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `item_id` (uint32) - Integer value `item_id`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Antonica/adankfurdockwarden.lua
function death(NPC, Spawn)
if GetQuestStep(Spawn, QUEST_3) == 2 then
if not HasItem(Spawn, 7800) then
 AddLootItem(Spawn, 7800)
elseif HasItem(Spawn, 7800) then
RemoveLootItem(Spawn, 7800)
end
```
