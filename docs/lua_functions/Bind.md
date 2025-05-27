### Function: Bind(spawn, zone_id, x, y, z, h)

**Description:**

Bind the spawn to the zone id, x, y, z and heading specified.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `zone_id` (uint32) - Integer value `zone_id`.
- `x` (float) - Float value `x`.
- `y` (float) - Float value `y`.
- `z` (float) - Float value `z`.
- `h` (float) - Float value `h`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/OutpostOverlord/CaptainVarlos.lua
function LeaveIsland(NPC, Spawn)
    Race = GetRace(Spawn)
  
    Bind(Spawn, 559, -232.03, -56.06, 172.57, 360.0)
    -- Human / Kerra
    if Race == 9 or Race == 11 then
        AddSpellBookEntry(Spawn, 8057, 1)	
        ZoneRef = GetZone("Freeport")
        Zone(ZoneRef,Spawn)

      -- Ratonga / Gnome
    elseif Race == 5 or Race == 13 then
        AddSpellBookEntry(Spawn, 8057, 1)	
        ZoneRef = GetZone("Freeport")
        Zone(ZoneRef,Spawn)


      -- Half Elf
    elseif Race == 6 then
        AddSpellBookEntry(Spawn, 8057, 1)	
        ZoneRef = GetZone("Freeport")
        Zone(ZoneRef,Spawn)


   -- Orge / Troll
    elseif Race == 12 or Race == 14 then
        AddSpellBookEntry(Spawn, 8057, 1)	
        ZoneRef = GetZone("Freeport")
        Zone(ZoneRef,Spawn)
    
   -- Dark Elf / Iksar
    elseif Race == 1 or Race == 10 then
        AddSpellBookEntry(Spawn, 8057, 1)	
        ZoneRef = GetZone("Freeport")
        Zone(ZoneRef,Spawn)



    -- Erudite / Freeblood
    elseif Race == 3 or Race == 19 then
        AddSpellBookEntry(Spawn, 8057, 1)
        ZoneRef = GetZone("Freeport")
        Zone(ZoneRef,Spawn)


    -- Barbarian and Aerakyn
    elseif Race == 0 or Race == 20 then
        AddSpellBookEntry(Spawn, 8057, 1)
        ZoneRef = GetZone("Freeport")
        Zone(ZoneRef,Spawn)

    -- Arasai or Sarnak
    elseif Race == 17 or Race == 18 then
        AddSpellBookEntry(Spawn, 8057, 1)
        ZoneRef = GetZone("Freeport")
        Zone(ZoneRef,Spawn)

    -- Unknown
    else
        PlayFlavor(NPC, "", "Sorry, I cannot deal with someone of your race. Try visiting the boat on the other island!", "", 0, 0, Spawn)
        ZoneRef = GetZone("Qeynos")
        Zone(ZoneRef,Spawn)
    end
```
