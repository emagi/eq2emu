### Function: GetInfoStructUInt(spawn, field)

**Description:**
Retrieves an unsigned integer field from a spawn’s info struct. This can include things like level, model type, gender, etc.  See https://github.com/emagi/eq2emu/blob/main/docs/data_types/info_struct.md for a full list of options.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `field` (string) - String `field`.

**Returns:** Int32 – The value of the field.

**Example:**

```lua
-- From Spells/Fighter/Crusader/PowerCleave.lua
function precast(Caster, Target)
    local wield_type = GetInfoStructUInt(Caster, "wield_type")
    if wield_type ~= 4 then
        SendMessage(Caster, "Must have a two-handed weapon equipped", "yellow")
        return false, 70
        end
```
