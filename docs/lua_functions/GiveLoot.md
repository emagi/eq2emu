### Function: GiveLoot(entity, player, coins, item_id)

**Description:**
Provides the player pending loot items from the entity(Spawn) with item_id that allows multiple comma delimited entries.  The coins will be added to the entities lootable coin.

**Parameters:**
- `entity` (Spawn) - Spawn object representing `entity`.
- `player` (Spawn) - Spawn object representing `player`.
- `coins` (uint32) - Integer value `coins`.
- `item_id` (uint32) - Integer value `item_id`.

**Returns:** None.

**Example:**

```lua
-- From Quests/FarJourneyFreeport/TasksaboardtheFarJourney.lua
		GiveLoot(chest, Player, 0, 185427)
		GiveLoot(chest, Player, 0, 20902)
		GiveLoot(chest, Player, 0, 15354)
```
