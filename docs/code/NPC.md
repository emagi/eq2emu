# File: `NPC.h`

## Classes

- `Brain`
- `NPCSpell`
- `NPC`

## Functions

- `void	Initialize();`
- `void	SetAppearanceID(int32 id){ appearance_id = id; }`
- `int32	GetAppearanceID(){ return appearance_id; }`
- `bool	IsNPC(){ return true; }`
- `void	StartRunback(bool reset_hp_on_runback = false);`
- `void	InCombat(bool val);`
- `bool	HandleUse(Client* client, string type);`
- `void	SetRandomize(int32 value) {appearance.randomize = value;}`
- `void	AddRandomize(sint32 value) {appearance.randomize += value;}`
- `int32	GetRandomize() {return appearance.randomize;}`
- `bool	CheckSameAppearance(string name, int16 id);`
- `void	Randomize(NPC* npc, int32 flags);`
- `int8	GetAttackType();`
- `void	SetAIStrategy(int8 strategy);`
- `int8	GetAIStrategy();`
- `void	SetPrimarySpellList(int32 id);`
- `int32	GetPrimarySpellList();`
- `void	SetSecondarySpellList(int32 id);`
- `int32	GetSecondarySpellList();`
- `void	SetPrimarySkillList(int32 id);`
- `int32	GetPrimarySkillList();`
- `void	SetSecondarySkillList(int32 id);`
- `int32	GetSecondarySkillList();`
- `void	SetEquipmentListID(int32 id);`
- `int32	GetEquipmentListID();`
- `void	SetAggroRadius(float radius, bool overrideBaseValue = false);`
- `float	GetAggroRadius();`
- `float	GetBaseAggroRadius() { return base_aggro_radius; }`
- `void	SetCastPercentage(int8 percentage);`
- `int8	GetCastPercentage();`
- `void	SetSkills(map<string, Skill*>* in_skills);`
- `void	SetSpells(vector<NPCSpell*>* in_spells);`
- `void	SetRunbackLocation(float x, float y, float z, int32 gridid, bool set_hp_runback = false);`
- `float	GetRunbackDistance();`
- `void	Runback(float distance=0.0f, bool stopFollowing = true);`
- `void	ClearRunback();`
- `void	AddSkillBonus(int32 spell_id, int32 skill_id, float value);`
- `void	SetMaxPetLevel(int8 val) { m_petMaxLevel = val; }`
- `int8	GetMaxPetLevel() { return m_petMaxLevel; }`
- `void ProcessCombat();`
- `void SetBrain(Brain* brain);`
- `int32 GetShardID() { return m_ShardID; }`
- `void SetShardID(int32 shardid) { m_ShardID = shardid; }`
- `int32 GetShardCharID() { return m_ShardCharID; }`
- `void SetShardCharID(int32 charid) { m_ShardCharID = charid; }`
- `sint64 GetShardCreatedTimestamp() { return m_ShardCreatedTimestamp; }`
- `void SetShardCreatedTimestamp(sint64 timestamp) { m_ShardCreatedTimestamp = timestamp; }`
- `bool HasSpells() { return has_spells; }`

## Notable Comments

- /*
- */
- // Randomize Appearances
- // Randomize appearance id (spawn_npcs table values)
- //#define RANDOMIZE_LEGS_TYPE			32  // spare!
- // Randomize parameters (npc_appearances, sInt values)
- // Randomize colors/hues (npc_appearances, RGB values)
- // All Flags On: 33554431
- /// <summary>Sets the brain this NPC should use</summary>
- /// <param name="brain">The brain this npc should use</param>
