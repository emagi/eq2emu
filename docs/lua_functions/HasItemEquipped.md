### Function: HasItemEquipped(player, item_id)

**Description:**
Return's if the player has an item equipped with the item_id.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `item_id` (uint32) - Integer value `item_id`.

**Returns:** Return's true if the item_id is equipped on the player, otherwise false.

**Example:**

```lua
-- From ItemScripts/Griz.lua
function GrizChat2_1(Item, Spawn)
	if GetQuestStep(Spawn, SometimesKnut) == 2 then
		SetStepComplete(Spawn, SometimesKnut, 2)
		AddSpawnAccess(GetSpawnByLocationID(Zone, 579551), Spawn)
	end
```
