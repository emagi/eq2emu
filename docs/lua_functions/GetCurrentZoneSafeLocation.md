### Function: GetCurrentZoneSafeLocation(spawn)

**Description:**
Returns the X, Y, Z of the current zone safe location by using the supplied Spawn's zone.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** x,y,z as returned array.

**Example:**

```lua
function cast(Caster, Target)
    x,y,z = GetCurrentZoneSafeLocation(Caster)
    SetPosition(Caster, x, y, z)
end
```
