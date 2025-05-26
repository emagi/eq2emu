### Function: RemoveImmunitySpell(type, spawn)

**Description:**
Remove an immunity spell from the Spawn.  See immunity types https://github.com/emagi/eq2emu/blob/main/docs/data_types/immunity_types.md

**Parameters:**
- `type` (uint8) - UInt8 value `type`.
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From Spells/AA/AdvanceWarning.lua
function remove(Caster, Target)
    RemoveImmunitySpell(7, Target)
end
```
