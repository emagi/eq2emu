### Function: GetSpellTier()

**Description:**
Get the spell tier of the current spell, must be ran in a spell scfript.

**Parameters:** None.

**Returns:** UInt32 tier of the spell.

**Example:**

```lua
-- From Spells/Fighter/Brawler/Bruiser/Haymaker.lua
function cast(Caster, Target, DmgType, MinVal, MaxVal)
    Level = GetLevel(Caster)
    SpellLevel = 30
    Mastery = SpellLevel + 10

    if Level < Mastery then
        LvlBonus = Level - SpellLevel
        else LvlBonus = Mastery - SpellLevel
    end
```
