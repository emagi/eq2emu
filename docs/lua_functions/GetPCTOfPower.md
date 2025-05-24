### Function: GetPCTOfPower(spawn, pct)

**Description:**
Get the Spawn's amount of power in UInt32 value using a float percentage.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `pct` (float) - float value `pct`.

**Returns:** UInt32 representation of the power against the percentage provided for the Spawn.

**Example:**

```lua
-- From Spells/Commoner/BlessingofFaith.lua
function cast(Caster, Target, pctHeal)
SpellHeal("Power", GetPCTOfPower(Caster, pctHeal))
end
```
