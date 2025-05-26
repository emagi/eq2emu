### Function: RemoveInvis(spawn)

**Description:**
Remove invisible from the spawn.  Spawn is optional, otherwise in a spell script applies to all spell targets.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Caves/acuriousrock.lua
function casted_on(NPC, Spawn, Message)
    if Message == "smash" then
    SetAccessToEntityCommand(Spawn,NPC,"smash", 0)
    SpawnSet(NPC, "show_command_icon", 0)
    SpawnSet(NPC, "display_hand_icon", 0)    
    if IsStealthed(Spawn)  then
--    RemoveStealth(NPC,Spawn)
    end
```
