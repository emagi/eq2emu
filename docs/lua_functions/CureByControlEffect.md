### Function: CureByControlEffect(cure_count, cure_type, cure_name, cure_level, target)

**Description:**
Cures the control effects.  cure_count controls the amount of cures activated, cure_type will be the control effect type.  If in a spell script and no target it will apply to all spell targets.

**Parameters:**
- `cure_count` (uint8) - Integer value `cure_count`.
- `cure_type` (uint8) - Integer value `cure_type`.
- `cure_name` (string) - String `cure_name`.
- `cure_level` (uint8) - Integer value `cure_level`.
- `target` (Spawn) - Spawn object representing `target`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Priest/Cleric/Inquisitor/FerventFaith.lua
function cast(Caster, Target, Levels)
    CureByControlEffect(1, 1, "Cure", Levels)
    CureByControlEffect(1, 2, "Cure", Levels)
    CureByControlEffect(1, 3, "Cure", Levels)
    CureByControlEffect(1, 4, "Cure", Levels)
end
```
