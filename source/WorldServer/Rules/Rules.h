/*
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

    This file is part of EQ2Emulator.

    EQ2Emulator is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EQ2Emulator is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EQ2Emulator.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef RULES_H_
#define RULES_H_

#include <string.h>
#include <map>
#include "../../common/Mutex.h"
#include "../../common/types.h"

using namespace std;

enum RuleCategory {
	R_Client,
	R_Faction,
	R_Guild,
	R_Player,
	R_PVP,
	R_Combat,
	R_Spawn,
	R_UI,
	R_World,
	R_Zone,
	R_Loot,
	R_Spells,
	R_Expansion,
	R_Discord
};

enum RuleType {
	/* CLIENT */
	ShowWelcomeScreen,

	/* FACTION */
	AllowFactionBasedCombat,

	/* GUILD */
	/* PLAYER */
	MaxLevel,
	MaxLevelOverrideStatus,
	MaxPlayers,
	MaxPlayersOverrideStatus,
	VitalityAmount,
	VitalityFrequency,
	MaxAA,
	MaxClassAA,
	MaxSubclassAA,
	MaxShadowsAA,
	MaxHeroicAA,
	MaxTradeskillAA,
	MaxPrestigeAA,
	MaxTradeskillPrestigeAA,
	MaxDragonAA,
	MinLastNameLevel,
	MaxLastNameLength,
	MinLastNameLength,
	DisableHouseAlignmentRequirement,
	MentorItemDecayRate,
	TemporaryItemLogoutTime,
	HeirloomItemShareExpiration,
	SwimmingSkillMinSpeed,
	SwimmingSkillMaxSpeed,
	SwimmingSkillMinBreathLength,
	SwimmingSkillMaxBreathLength,
	AutoSkillUpBaseSkills,
	MaxWeightStrengthMultiplier,
	BaseWeight,
	WeightPercentImpact,
	WeightPercentCap,
	CoinWeightPerStone,
	WeightInflictsSpeed,
	LevelMasterySkillMultiplier,
	TraitTieringSelection,
	ClassicTraitLevelTable,
	TraitFocusSelectLevel,
	TraitTrainingSelectLevel,
	TraitRaceSelectLevel,
	TraitCharacterSelectLevel,
	StartHPBase,
	StartPowerBase,
	StartHPLevelMod,
	StartPowerLevelMod,
	AllowPlayerEquipCombat,
	MaxTargetCommandDistance,
	MinSkillMultiplierValue,
	HarvestSkillUpMultiplier,

	/* PVP */
	AllowPVP,
	LevelRange,
	InvisPlayerDiscoveryRange,
	PVPMitigationModByLevel,

	/* COMBAT */
	MaxCombatRange,
	DeathExperienceDebt,
	GroupExperienceDebt,
	PVPDeathExperienceDebt,
	ExperienceToDebt,
	ExperienceDebtRecoveryPercent,
	ExperienceDebtRecoveryPeriod,
	EnableSpiritShards,
	SpiritShardSpawnScript,
	ShardDebtRecoveryPercent,
	ShardRecoveryByRadius,
	ShardLifetime,
	EffectiveMitigationCapLevel,
	CalculatedMitigationCapLevel,
	MitigationLevelEffectivenessMax,
	MitigationLevelEffectivenessMin,
	MaxMitigationAllowed,
	MaxMitigationAllowedPVP,
	StrengthNPC,
	StrengthOther,
	MaxSkillBonusByLevel,
	LockedEncounterNoAttack,

	/* SPAWN */
	SpeedMultiplier,
	ClassicRegen,
	HailMovementPause,
	HailDistance,
	UseHardCodeWaterModelType,
	UseHardCodeFlyingModelType,
	//SpeedRatio,

	/* UI */
	MaxWhoResults,
	MaxWhoOverrideStatus,

	/* WORLD */
	DefaultStartingZoneID,
	EnablePOIDiscovery,
	GamblingTokenItemID,
	GuildAutoJoin,
	GuildAutoJoinID,
	GuildAutoJoinDefaultRankID,
	ServerLocked,
	ServerLockedOverrideStatus,
	SyncZonesWithLogin,
	SyncEquipWithLogin,
	UseBannedIPsTable,
	LinkDeadTimer,
	RemoveDisconnectedClientsTimer,
	PlayerCampTimer,
	GMCampTimer,
	AutoAdminPlayers,
	AutoAdminGMs,
	AutoAdminStatusValue,
	DuskTime,
	DawnTime,
	ThreadedLoad,
	TradeskillSuccessChance,
	TradeskillCritSuccessChance,
	TradeskillFailChance,
	TradeskillCritFailChance,
	TradeskillEventChance,
	EditorURL,
	EditorIncludeID,
	EditorOfficialServer,
	GroupSpellsTimer,
	QuestQueueTimer,
	SavePaperdollImage,
	SaveHeadshotImage,
	SendPaperdollImagesToLogin,
	TreasureChestDisabled,
	StartingZoneLanguages,
	StartingZoneRuleFlag,
	EnforceRacialAlignment,
	MemoryCacheZoneMaps,
	AutoLockEncounter,
	DisplayItemTiers,
	LoreAndLegendAccept,

	/* ZONE */
	MinZoneLevelOverrideStatus,
	MinZoneAccessOverrideStatus,
	XPMultiplier,
	TSXPMultiplier,
	WeatherEnabled,
	WeatherType,
	MinWeatherSeverity,
	MaxWeatherSeverity,
	WeatherChangeFrequency,
	WeatherChangePerInterval,
	WeatherDynamicMaxOffset,
	WeatherChangeChance,
	SpawnUpdateTimer,
	CheckAttackPlayer,
	CheckAttackNPC,
	HOTime,
	UseMapUnderworldCoords,
	MapUnderworldCoordOffset,
	SharedZoneMaxPlayers,
	
	/* LOOT */
	LootRadius,
	AutoDisarmChest, // if enabled disarm only works if you right click and disarm, clicking and opening chest won't attempt auto disarm
	ChestTriggerRadiusGroup,
	ChestUnlockedTimeDrop,
	AllowChestUnlockByDropTime,
	ChestUnlockedTimeTrap,
	AllowChestUnlockByTrapTime,
	
	/* SPELLS */
	NoInterruptBaseChance,
	EnableFizzleSpells,
	DefaultFizzleChance,
	FizzleMaxSkill,
	FizzleDefaultSkill,
	EnableCrossZoneGroupBuffs,
	EnableCrossZoneTargetBuffs,
	PlayerSpellSaveStateWaitInterval,
	PlayerSpellSaveStateCap,
	RequirePreviousTierScribe,
	CureSpellID,
	CureCurseSpellID,
	CureNoxiousSpellID,
	CureMagicSpellID,
	CureTraumaSpellID,
	CureArcaneSpellID,
	MinistrationSkillID,
	MinistrationPowerReductionMax,
	MinistrationPowerReductionSkill,
	MasterSkillReduceSpellResist,

	/* ZONE TIMERS */
	RegenTimer,
	ClientSaveTimer,
	ShutdownDelayTimer,
	WeatherTimer,
	SpawnDeleteTimer,

	GlobalExpansionFlag,
	GlobalHolidayFlag,

	DatabaseVersion,

	SkipLootGrayMob,
	LootDistributionTime,
	DiscordEnabled,
	DiscordWebhookURL,
	DiscordBotToken,
	DiscordChannel,
	DiscordListenChan
};

class Rule {
public:
	Rule();
	Rule(int32 category, int32 type, const char *value, const char *combined);
	Rule (Rule *rule_in);
	virtual ~Rule();

	void SetValue(const char *value) {strncpy(this->value, value, sizeof(this->value));}

	int32 GetCategory() {return category;}
	int32 GetType() {return type;}
	const char * GetValue() {return value;}
	const char * GetCombined() {return combined;}

	int8 GetInt8() {return (int8)atoul(value);}
	int16 GetInt16() {return (int16)atoul(value);}
	int32 GetInt32() {return (int32)atoul(value);}
	int64 GetInt64() {return (int64)atoi64(value);}
	sint8 GetSInt8() {return (sint8)atoi(value);}
	sint16 GetSInt16() {return (sint16)atoi(value);}
	sint32 GetSInt32() {return (sint32)atoi(value);}
	sint64 GetSInt64() {return (sint64)atoi64(value);}
	bool GetBool() {return atoul(value) > 0 ? true : false;}
	float GetFloat() {return atof(value);}
	char GetChar() {return value[0];}
	const char * GetString() {return value;}

private:
	int32 category;
	int32 type;
	char value[1024];
	char combined[2048];
};

class RuleSet {
public:
	RuleSet();
	RuleSet(RuleSet *in_rule_set);
	virtual ~RuleSet();

	void CopyRulesInto(RuleSet *in_rule_set);

	void SetID(int32 id) {this->id = id;}
	void SetName(const char *name) {strncpy(this->name, name, sizeof(this->name));}

	int32 GetID() {return id;}
	const char *GetName() {return name;}

	void AddRule(Rule *rule);
	Rule * GetRule(int32 category, int32 type);
	Rule * GetRule(const char *category, const char *type);
	void ClearRules();

	map<int32, map<int32, Rule *> > * GetRules() {return &rules;}

private:
	int32 id;
	char name[64];
	Mutex m_rules;
	map<int32, map<int32, Rule *> > rules;
};

class RuleManager {
public:
	RuleManager();
	virtual ~RuleManager();

	void Init();
	void Flush(bool reinit=false);

	void LoadCodedDefaultsIntoRuleSet(RuleSet *rule_set);

	bool AddRuleSet(RuleSet *rule_set);
	int32 GetNumRuleSets();
	void ClearRuleSets();

	Rule * GetBlankRule() {return &blank_rule;}

	bool SetGlobalRuleSet(int32 rule_set_id);
	Rule * GetGlobalRule(int32 category, int32 type);
	Rule * GetGlobalRule(const char* category, const char* type);

	bool SetZoneRuleSet(int32 zone_id, int32 rule_set_id);
	Rule * GetZoneRule(int32 zone_id, int32 category, int32 type);
	void ClearZoneRuleSets();

	RuleSet * GetGlobalRuleSet() {return &global_rule_set;}
	map<int32, map<int32, Rule *> > * GetRules() {return &rules;}

private:
	Mutex m_rule_sets;
	Mutex m_global_rule_set;
	Mutex m_zone_rule_sets;
	Rule blank_rule;						/* READ ONLY */
	map<int32, map<int32, Rule *> > rules;	/* all of the rules loaded with their defaults (FROM THE CODE). map<category, map<type, rule>> */
	map<int32, RuleSet *> rule_sets;		/* all of the possible rule sets from the database. map<rule set id, rule set> */
	RuleSet global_rule_set;				/* the global rule set, first fill it the defaults from the code, then over ride from the database */
	map<int32, RuleSet *> zone_rule_sets;	/* references to a zone's rule set. map<zone id, rule set> */
};

#endif