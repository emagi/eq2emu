### Function: IsGroundSpawn(spawn)

**Description:**
Return's true if the spawn is a ground spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** True if the spawn is the Ground Spawn, otherwise False.

**Example:**

```lua
-- From Spells/Commoner/harvest.lua
function precast(Caster, Target)
    if IsGroundSpawn(Target) then
      return CanHarvest(Caster, Target)
    end
```
