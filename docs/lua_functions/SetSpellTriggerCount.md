### Function: SetSpellTriggerCount(trigger_count, cancel_after_triggers)

**Description:**
Set's the spell trigger count in a spell script, cancel_after_triggers set to 1 will remove spell after.

**Parameters:**
- `trigger_count` (uint16) - uint16 value `trigger_count`.
- `cancel_after_triggers` (uint8) - uint8 value `cancel_after_triggers`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/ArcaneEnlightenment.lua
function cast(Caster, Target, Power, Triggers)
    MaxPow = GetMaxPower(Caster)
    PowHeal = math.floor(MaxPow * 0.2)
    SpellHeal("Power", PowHeal, PowHeal, Caster)
    
    AddProc(Caster, 15, 50)
    SetSpellTriggerCount(Triggers, 1)
end
```
