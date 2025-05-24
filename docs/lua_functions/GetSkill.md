### Function: GetSkill(spawn, name)

**Description:**
Gets the skill object reference for a spawn by name.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `name` (string) - String `name`.

**Returns:** Skill object reference.

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
