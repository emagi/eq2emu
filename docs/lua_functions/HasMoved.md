### Function: HasMoved(spawn)

**Description:**

Return's true if the position has moved (X/Y/Z) heading is not included since the last time HasMoved was called on the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** True if the HasMoved function was previously called on the Spawn and the x/y/z positions have changed since last being called. Otherwise false.


**Example:**

```lua
-- From SpawnScripts/Cache/abanditcook.lua
function Checking(NPC,Spawn)
    if GetDistance(NPC,Spawn) <=8 and HasMoved(Spawn) then
    Attack(NPC,Spawn)
    end
```
