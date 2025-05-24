### Function: GetPCTOfHP(spawn, pct)

**Description:**
Get the Spawn's amount of hp in UInt32 value using a float percentage.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `pct` (float) - Floating point value `pct`.

**Returns:** UInt32 representation of the hp against the percentage provided for the Spawn.

**Example:**

```lua
-- From Spells/Fighter/Brawler/Monk/Mendpct.lua
function cast(Caster, Target, CureLvls, MinVal, MaxVal)
-- Dispels 7 levels of noxious hostile effects on target
CureByType(CureLvls, 3);  
--Heals target for 8.1 - 9.9% of max health
SpellHeal("Heal", GetPCTOfHP(Caster, MinVal), GetPCTOfHP(Caster, MaxVal),0 , 2, 1)

 
end
```
