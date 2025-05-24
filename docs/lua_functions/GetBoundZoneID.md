### Function: GetBoundZoneID(spawn)

**Description:**
Gets the zone id the Spawn(Player) is bound in.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Returns UINT32 of the Zone ID.

**Example:**

```lua
-- From Spells/Commoner/CalltoHome.lua
function precast(Caster, Target)
    if GetBoundZoneID(Caster) == 0 then
        return false
    end
```
