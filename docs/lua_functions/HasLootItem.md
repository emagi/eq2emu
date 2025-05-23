### Function: HasLootItem(spawn, item_id)

**Description:**
Identifies if the specified spawn has the item_id in it's loot drop table.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `item_id` (uint32) - Integer value `item_id`.

**Returns:** None.

**Example:**

```lua
-- From Quests/FarJourneyFreeport/TasksaboardtheFarJourney.lua
			rat = GetSpawnFromList(spawns, i-1)
			if rat then
				SetAttackable(rat, 1)
				if HasLootItem(rat, 11615) == false then
					AddLootItem(rat, 11615)	
				end
			end
```
