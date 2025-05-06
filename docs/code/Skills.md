# File: `Skills.h`

## Classes

- `SkillBonusValue`
- `SkillBonus`
- `Skill`
- `MasterSkillList`
- `PlayerSkillList`

## Functions

- `int			CheckDisarmSkill(int16 targetLevel, int8 chest_difficulty=0);`
- `void AddSkill(Skill* skill);`
- `int16 GetSkillCount();`
- `void RemoveSkill(Skill* skill);`
- `void AddSkill(Skill* new_skill);`
- `bool CheckSkillIncrease(Skill* skill);`
- `bool HasSkill(int32 skill_id);`
- `void IncreaseSkill(Skill* skill, int16 amount);`
- `void IncreaseSkill(int32 skill_id, int16 amount);`
- `void DecreaseSkill(Skill* skill, int16 amount);`
- `void DecreaseSkill(int32 skill_id, int16 amount);`
- `void SetSkill(Skill* skill, int16 value, bool send_update = true);`
- `void SetSkill(int32 skill_id, int16 value, bool send_update = true);`
- `void IncreaseSkillCap(Skill* skill, int16 amount);`
- `void IncreaseSkillCap(int32 skill_id, int16 amount);`
- `void DecreaseSkillCap(Skill* skill, int16 amount);`
- `void DecreaseSkillCap(int32 skill_id, int16 amount);`
- `void SetSkillCap(Skill* skill, int16 value);`
- `void SetSkillCap(int32 skill_id, int16 value);`
- `void IncreaseAllSkillCaps(int16 value);`
- `void IncreaseSkillCapsByType(int8 type, int16 value);`
- `void SetSkillCapsByType(int8 type, int16 value);`
- `void SetSkillValuesByType(int8 type, int16 value, bool send_update = true);`
- `void AddSkillUpdateNeeded(Skill* skill);`
- `void AddSkillBonus(int32 spell_id, int32 skill_id, float value);`
- `void RemoveSkillBonus(int32 spell_id);`
- `int16 CalculateSkillValue(int32 skill_id, int16 current_val);`
- `int16 CalculateSkillMaxValue(int32 skill_id, int16 max_val);`
- `bool HasSkillUpdates();`
- `void ResetPackets();`

## Notable Comments

- /*
- */
- //the following update the current_value to the max_value as soon as the max_value is updated
- /* Each SkillBonus is comprised of multiple possible skill bonus values.  This is so one spell can modify
