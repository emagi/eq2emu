### Function: GetRuleFlagInt32(category, name)

**Description:**
Gets the unsigned integer rule flag from the world based on the category and name.

**Parameters:**
- `category` (string) - String `category`.
- `name` (string) - String `name`.

**Returns:** UInt32 value of the rule category and name.

**Example:**

```lua
-- From SpawnScripts/FarJourneyFreeport/CaptainVarlos.lua
function zone_to_isle(NPC, player)
	serverType = GetRuleFlagInt32("R_World", "StartingZoneRuleFlag")
	-- if no server type is set (default of 0 wildcard) or odd number means bit 1 is set
	if serverType == 0 or (serverType % 2) == 1 then
		-- DoF alignment, 0 = evil (Outpost of Overlord), 1 = good (Queens Colony)
		alignment = GetAlignment(player)
		if GetClass(player) == 0 then -- isle of refuge (Commoners are sent here automatically)
          ZoneRef = GetZone("IsleRefuge1")
            Zone(ZoneRef,player)  
		
		elseif alignment == 0 then
			Zone(GetZone(278), player) -- outpost of overlord
		else
			Zone(GetZone(253), player) -- queens colony
		end
```
