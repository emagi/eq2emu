### Function: FaceTarget(spawn, target, reset_action_state)

**Description:**
The spawn will be forced to face the target.  The reset_action_state is true by default, unless otherwise specified as false, then the FaceTarget may not be honored if the Spawn is doing another activity.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `target` (Spawn) - Spawn object representing `target`.
- `reset_action_state` (bool) - Boolean flag `reset_action_state`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/pirateskull.lua
function PlaceSkull(Item, Player)
  zone = GetZone(Player)
  Guurok = GetSpawnByLocationID(zone, 433001)
  local distancecheck = GetDistance(Guurok, Player)
  if distancecheck > 8  then
   RemoveItem(Player, 10399)
   SendMessage(Player, "The skull crumbles to dust on the ground.", "yellow")
   CloseItemConversation(Item, Player)
  elseif distancecheck < 8 then
   FeedGuurok(Item, Player)
   SendMessage(Player, "The Guurok snatches the skull as you place it on the ground.", "yellow")
    FaceTarget(NPC, Player)
    PlayFlavor(Guurok, "", "", "attack", 0, 0)
    CloseItemConversation(Item, Player)
   RemoveItem(Player, 10399)
   end
```
