### Function: GetZoneID(zone)

**Description:**
Gets the zone id of the zone object.

**Parameters:**
- `zone` (Zone) - Zone object representing `zone`.

**Returns:** UInt32 zone id of the zone object.

**Example:**

```lua
-- From ItemScripts/AquaticResearchNotebook.lua
function examined(Item, Player)
  local zone = GetZone(Player)
if GetZoneID(zone)~= 325 then
    	SendMessage(Player, "The notebook is a research manual for aquatic creatures living on the Isle of Refuge.  It won't do you any good now that you have left the isle.")
else
if not HasQuest(Player,5757) and not HasCompletedQuest(Player, 5757) then
 OfferQuest(nil,Player,5757)
end
```
