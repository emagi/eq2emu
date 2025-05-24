### Function: GetID(spawn)

**Description:**
Gets the current ID of the Spawn for the Zone.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 ID of the Spawn for it's current instance in the zone.

**Example:**

```lua
-- From Spells/Commoner/DetectGood.lua
function cast(Caster, Target)
    local targets = GetGroup(Caster)
    if targets ~= nil then
        for k,v in ipairs(targets) do
            if GetAlignment(Target) == 1 and GetID(Target) == GetID(v) then
                ApplySpellVisual(v,136)
            end
```
