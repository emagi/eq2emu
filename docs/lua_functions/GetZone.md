### Function: GetZone(zone_id|zone_name|spawn)

**Description:**
Gets the zone object by the zone_id

**Parameters:**
- `zone_id`, `zone_name`, `spawn` (uint32/string/spawn) - Integer/String/Spawn value `zone_id` or `zone_name` or `spawn`.

**Returns:** Gets or creates Zone object reference to zone_id or zone_name.  Otherwise if `spawn` is provided gets the current spawn's zone object.

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
