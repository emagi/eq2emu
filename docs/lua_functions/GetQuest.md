### Function: GetQuest(player, quest_id)

**Description:**
Gets the quest object reference for the player and quest_id specified if they have the quest.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.

**Returns:** Quest object reference in relation to the Player.

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
