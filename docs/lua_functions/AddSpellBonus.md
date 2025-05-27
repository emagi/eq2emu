### Function: AddSpellBonus(Spawn, type, value)

**Description:**

Adds a spell bonus of the type specified.  The types are defined on https://github.com/emagi/eq2emu/blob/main/docs/data_types/item_stat_types.md

**Parameters:**
- `spawn` (Spawn) - Spawn object reference `spawn`.
- `type` (uint16) - Integer value `type`.
- `value` (float) - Float value `value`.

**Returns:** None.

**Example:**

```lua
-- From Spells/AA/AbilityAptitude.lua
function cast(Caster, Target, BonusAmt)
    AddSpellBonus(Target, 707, BonusAmt)
end
```
