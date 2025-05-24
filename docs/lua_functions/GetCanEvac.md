### Function: GetCanEvac(spawn)

**Description:**
Gets if the Spawn(Player) can evac in the current zone.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Returns UINT32 1 if you can evac, otherwise 0.

**Example:**

```lua
-- From Spells/Scout/Escape.lua
function precast(Caster, Target)
    if(GetCanEvac(Caster) == 1)
    then
        return true
    else
        SendMessage(Caster, "You cannot use evacuate spells in this zone.", "red")
        return false
    end
```
