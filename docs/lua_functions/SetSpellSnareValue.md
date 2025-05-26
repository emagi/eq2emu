### Function: SetSpellSnareValue(snare, spawn)

**Description:**
Set's the spell snare value of the current spell script.

**Parameters:**
- `snare` (float) - Float value `snare`.
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/Blind.lua
function cast(Caster, Target, Snare)
    if not IsEpic(Target) then
        --Dazes the target
        AddControlEffect(Target, 3)
        BlurVision(Target, 1.0)
        SetSpellSnareValue(Target, Snare) 
        AddControlEffect(Target, 11)
    end
```
