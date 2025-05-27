### Function: IsStealthed(spawn)

**Description:**

Return's true if the spawn is stealthed, otherwise false.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Return's true if the spawn is stealthed, otherwise false.

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
