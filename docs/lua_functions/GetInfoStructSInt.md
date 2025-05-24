### Function: GetInfoStructSInt(spawn, field)

**Description:**
Gets a signed integer field from the spawn’s info struct. Similar to GetInfoStructUInt but for fields that can be negative.  See https://github.com/emagi/eq2emu/blob/main/docs/data_types/info_struct.md for a full list of options.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `field` (string) - String `field`.

**Returns:** SInt32 – The value of that field.

**Example:**

```lua
-- From Spells/BattleRest.lua
function cast(Caster, Target)
    CurrentRegen = GetInfoStructSInt(Caster, "hp_regen")
    AddSpellBonus(Caster, 600, math.ceil(CurrentRegen * 0.05))
    AddSpellBonus(Caster, 0, 2)
end
```
