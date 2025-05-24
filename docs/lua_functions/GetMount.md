### Function: GetMount(spawn)

**Description:**
Gets the spawn object that represents the mount for the Spawn provided.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Mount Spawn object of the Spawn specified.

**Example:**

```lua
-- From Spells/Commoner/AbyssalCarpet.lua
function precast(Caster)
 if GetMount(Caster) > 0 then
        return false
    end
```
