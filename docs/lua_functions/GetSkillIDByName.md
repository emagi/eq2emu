### Function: GetSkillIDByName(name)

**Description:**
Get the skill id by its name.

**Parameters:**
- `name` (string) - String `name`.

**Returns:** UInt32 skill id of the skill name.

**Example:**

```lua
-- From Spells/AA/BattlemagesFervor.lua
function cast(Caster, Target, SkillAmt)
    AddSkillBonus(Target, GetSkillIDByName("Focus"), SkillAmt)
    AddSkillBonus(Target, GetSkillIDByName("Disruption"), SkillAmt)
    AddSkillBonus(Target, GetSkillIDByName("Subjugation"), SkillAmt)
    AddSkillBonus(Target, GetSkillIDByName("Ordination"), SkillAmt)
    Say(Caster, "need formula")
end
```
