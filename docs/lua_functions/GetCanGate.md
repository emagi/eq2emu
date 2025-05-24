### Function: GetCanGate(spawn)

**Description:**
Checks if the Spawn is allowed to use Gate spells in this zone or area.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Returns UINT32 1 if you can gate, otherwise 0.

**Example:**

```lua
-- From Spells/Commoner/CalltoHome.lua
function precast(Caster, Target)
    if GetBoundZoneID(Caster) == 0 then
        return false
    end

    if(GetCanGate(Caster) == 1)
    then
        return true   
    else
        SendMessage(Caster, "You cannot use Call to Home from this location.", "red")
        return false
    end

 return true
end
```
