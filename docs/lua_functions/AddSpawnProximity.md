### Function: AddSpawnProximity(Spawn, SpawnValue, SpawnType, Radius, EnterFunction, LeaveFunction)

**Description:** Used on NPC's/Objects/Widgets to track other NPC's/Objects/Widgets entering proximity.  SpawnValue is the database id or location id based on the SpawnType.

**Parameters:**

`Spawn`: Spawn – The central entity defining the proximity area.
`SpawnValue`: UInt32 – 
`SpawnType`: UInt8 – SPAWNPROXIMITY_DATABASE_ID = 0, SPAWNPROXIMITY_LOCATION_ID = 1
`Radius`: Float – The radius (in meters) of the proximity zone around the spawn.
`EnterFunction`: String – The name of the function to call when a player enters the radius.
`LeaveFunction`: String – The name of the function to call when a player leaves the radius.

**Returns:** None.

**Example:**


```lua
-- Example script taken from SpawnScripts/TimorousDeep/aHaoaeranpoacher.lua (2630018)
-- Poacher attacks Crabs when In Range, using the v = SpawnValue, SpawnType = 1 (Location ID), Radius = 5.
local crablist = { 35182, 34566, 34575, 34752, 34873, 35006, 35182, 35355, 35470, 35506, 35527, 35535, 35544, 35550, 35551, 35554, 35555, 35581, 35635, 35698, 35768, 35818, 35848, 35867, 35889, 35918, 35943, 35948, 35951, 35960, 35971, 35981 }; -- array with crabs location ID's
function prespawn(NPC)
	for k, v in ipairs(crablist) do
		AddSpawnProximity(NPC, v,  1, 5, "InRange", "OutRange")
	end
end

function InRange(NPC)
local crab = GetSpawn(NPC, 2630018)
	if crab ~= nil then
		Attack(NPC, crab)
	end
end
   

function OutRange(NPC)
 -- do whatever out of range
end
```