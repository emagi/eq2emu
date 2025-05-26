### Function: SetSkillMaxValue(skill, value)

**Description:**
Set's the maximum skill value for the skill object.

**Parameters:**
- `skill` (int32) - Integer value `skill`.
- `value` (uint16) - Integer value `value`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/DarkBargainers/SasitSoroth.lua
function MaxGathering(NPC, Spawn)
	local skill = GetSkill(Spawn, "Gathering")
    if skill ~= nil then
        SetSkillMaxValue(skill, 300)
        SetSkillValue(skill, 300)
    end
```
