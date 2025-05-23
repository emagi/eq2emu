### Function: Say(spawn, message, player, dist, language)

**Description:**
Sends a Say message from the Spawn to the general area based on distance, or otherwise to a specific Player if specified, otherwise optional/nil.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `message` (string) - String `message`.
- `player` (Spawn) - Spawn object representing `player`.
- `dist` (int32) - Integer value `dist`.
- `language` (int32) - Integer value `language`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/DrawingRay.lua
function used(Item, Player)
    quest = GetQuest(Player, CAVES_CONSUL_BREE_QUEST_3)
    --Say(Player, "RAY HAS BEEN USED")
	if HasQuest(Player, CAVES_CONSUL_BREE_QUEST_3) then
		spawn = GetTarget(Player)
	--	Say(Player, "PLAYER HAS QUEST")
		if spawn ~= nil then
		--Say(Player, "SPAWN IS NOT NIL")
			-- river behemoth remains
			if GetSpawnID(spawn) == RIVER_BEHEMOTH_REMAINS_ID then
			    CastSpell(Player, 5104, 1)
				GiveQuestItem(quest, Player, "", RIVER_STONE_ID)
			--	Say(Player, "ITEM OBTAINED")
			else
				SendMessage(Player, "The Drawing Ray has no effect. Emma said it must be used on the remains of a river behemoth.")
			end
```
