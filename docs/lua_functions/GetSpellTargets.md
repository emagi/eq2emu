### Function: GetSpellTargets(spell)

**Description:**
Obtain all the Spell Targets of the Spell.  The `spell` field is optional, it will otherwise be in a Spell Script which defaults to the current spell's initial target.

**Parameters:**
- `spell` (Spell) - Spell object representing `spell`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/UnholyFear.lua
function cast(Caster, Target)
    local targets = GetSpellTargets()
	for k,v in ipairs(targets) do
        if GetLevel(v) < GetLevel(Caster) then
            PlayAnimationString(v, "cringe", nil, true)
        end
```
