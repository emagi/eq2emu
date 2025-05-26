### Function: HasQuest(player, quest_id)

**Description:**
Return's true if the player has the quest_id active in their journal.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.

**Returns:** Return's true if the player has an active quest (not completed/pending).

**Example:**

```lua
-- From ItemScripts/abadlypolishedsteelkey.lua
function examined(Item, Player)
if not HasQuest(Player, Polishedsteelkey) then
OfferQuest(nil, Player, Polishedsteelkey)
  end
```
