### Function: OfferQuest(npc, player, quest_id, forced)

**Description:**
NPC Offers the quest window or auto accept of the quest to the Player.

**Parameters:**
- `npc` (Spawn) - Spawn object representing `npc`.
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.
- `forced` (int32) - Integer value `forced`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/abadlypolishedsteelkey.lua
function examined(Item, Player)
if not HasQuest(Player, Polishedsteelkey) then
OfferQuest(nil, Player, Polishedsteelkey)
  end
```
