### Function: GetRaceBaseType(Spawn)

**Description:**
Returns the base race category of the spawn, refer to the database race types tables.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Example:**

```lua
-- From Spells/Priest/Cleric/RadiantStrike.lua
function cast(Caster, Target, DmgType, MinVal, MaxVal)
    
    Level = GetLevel(Caster)
    SpellLevel = 11
    Mastery = SpellLevel + 10
    StatBonus = GetInt(Caster) / 10
    
    if Level < Mastery then
        LvlBonus = Level - SpellLevel
        else LvlBonus = Mastery - SpellLevel
    end

    DmgBonus = LvlBonus + StatBonus
    MaxDmg = math.floor(DmgBonus) * 2 + MaxVal
    MinDmg = math.floor(DmgBonus) * 2 + MaxVal
    
    SpellDamage(Target, DmgType, MinDmg, MaxDmg)
    
    if GetRaceBaseType(Target) == 333 then
        SpellDamage(Target, Dmgtype, MinDmg, MaxDmg)
    end

end
```
