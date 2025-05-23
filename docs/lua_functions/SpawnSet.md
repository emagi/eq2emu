### Function: SpawnSet(spawn, variable, value, no_update, temporary_flag, index)

**Description:**
Set a spawn setting on the specified spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `variable` (int32) - Integer value `variable`.
- `value` (int32) - Integer value `value`.
- `no_update` (int32) - Integer value `no_update`.
- `temporary_flag` (int32) - Integer value `temporary_flag`.
- `index` (int32) - Integer value `index`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/pirateskull.lua
  function FeedGuurok(Item, Player)
    zone = GetZone(Player)
  Guurok = GetSpawnByLocationID(zone, 433001)  
  if GetTempVariable(Guurok, "FeedCounter") == "0" then
 SpawnSet(Guurok, "size", "100")
 SetTempVariable(Guurok, "FeedCounter", 1)
 elseif GetTempVariable(Guurok, "FeedCounter") == "1" then
 SpawnSet(Guurok, "size", "105")
  SetTempVariable(Guurok, "FeedCounter", 2)
 elseif GetTempVariable(Guurok, "FeedCounter") == "2" then
 SetTempVariable(Guurok, "FeedCounter", 3)
 SpawnSet(Guurok, "size", "110")
elseif GetTempVariable(Guurok, "FeedCounter") == "3" then
   SetTempVariable(Guurok, "FeedCounter", 4)
   SpawnSet(Guurok, "size", "115")
elseif GetTempVariable(Guurok, "FeedCounter") == "4" then 
   SetTempVariable(Guurok, "FeedCounter", 5)
   SpawnSet(Guurok, "size", "120")
elseif GetTempVariable(Guurok, "FeedCounter") == "5" then 
   SetTempVariable(Guurok, "FeedCounter", 6)
   SpawnSet(Guurok, "size", "125")
elseif GetTempVariable(Guurok, "FeedCounter") == "6" then
   SetTempVariable(Guurok, "FeedCounter", 666)
   SpawnSet(Guurok, "size", "130")
   SpawnSet(Guurok, "visual_state", "3632")
    SpawnSet(Guurok, "show_level", "1")
     SpawnSet(Guurok, "attackable", "1")
     SpawnSet(Guurok, "faction", "1")
 end
```
