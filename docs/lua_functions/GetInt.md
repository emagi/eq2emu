### Function: GetInt(spawn)

**Description:**
Gets the intelligence of the current Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Gets the UInt32 value of the Spawn's current intelligence.

**Example:**

```lua
-- From Spells/Commoner/BlightoftheMorning.lua
function proc(Caster, Target, Type, DmgType, MinVal, MaxVal)
    Spell = GetSpell(2550441)
    DmgBonus = math.floor(GetInt(Caster)/10)
    MinDmg = MinVal + DmgBonus
    MaxDmg = MaxVal + DmgBonus
    
    if Type == 3 then
		SetSpellDataIndex(Spell, 0, DmgType)
		SetSpellDataIndex(Spell, 1, MinDmg)
		SetSpellDataIndex(Spell, 2, MaxDmg)
			CastCustomSpell(Spell, Caster, Target)
			RemoveTriggerFromSpell(1)
	end
```
