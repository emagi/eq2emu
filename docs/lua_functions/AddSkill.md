Function: AddSkill(Spawn, SkillID, CurrentVal, MaxVal, MoreToAdd)

Description: Grants a new skill to the specified player. If the player does not already have the skill in their skill list, this will add it (usually at base level).

Parameters:

    Spawn: Spawn – The player to grant the skill to.
    SkillID: Int32 – The ID of the skill to add.
    CurrentVal: Int16 – The current value the player will have in the skill.
    MaxVal: Int16 – The max skill the player can receive with the current skill.
    MoreToAdd: Boolean – When set to true, we skip sending the skill packet, expecting to send yet another skill with AddSkill.

Returns: If successfully adding the skill we will return true, otherwise we return false noting the player already has the skill.

Example:
-- For SkillID refer to the skills table https://github.com/emagi/eq2emu/blob/main/docs/data_types/skills.md
-- Example usage (teach the player the “Gnollish” language skill)
AddSkill(Player, GNOLLISH_LANGUAGE_SKILL_ID)