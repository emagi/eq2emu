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

#include <assert.h>
#include "../../common/debug.h"
#include "../../common/Log.h"
#include "../../common/database.h"
#include "Rules.h"

extern RuleManager rule_manager;

Rule::Rule() {
	category = 0;
	type = 0;
	strncpy(value, "", sizeof(value));
	strncpy(combined, "NONE", sizeof(combined));
}

Rule::Rule(int32 category, int32 type, const char *value, const char *combined) {
	this->category = category;
	this->type = type;
	strncpy(this->value, value, sizeof(this->value));
	strncpy(this->combined, combined, sizeof(this->combined));
}

Rule::Rule (Rule *rule_in) {
	category = rule_in->GetCategory();
	type = rule_in->GetType();
	strncpy(value, rule_in->GetValue(), sizeof(value));
	strncpy(combined, rule_in->GetCombined(), sizeof(combined));
}

Rule::~Rule() {
}

RuleSet::RuleSet() {
	id = 0;
	memset(name, 0, sizeof(name));
	m_rules.SetName("RuleSet::rules");
}

RuleSet::RuleSet(RuleSet *in_rule_set) {
	assert(in_rule_set);

	map<int32, map<int32, Rule *> > * in_rules = in_rule_set->GetRules();
	map<int32, map<int32, Rule *> >::iterator itr;
	map<int32, Rule *>::iterator itr2;
	Rule * rule;
		
	m_rules.SetName("RuleSet::rules");
	id = in_rule_set->GetID();
	strncpy(name, in_rule_set->GetName(), sizeof(name));
	for (itr = in_rules->begin(); itr != in_rules->end(); itr++) {
		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++) {
			rule = itr2->second;
			rules[rule->GetCategory()][rule->GetType()] = new Rule(rule);
		}
	}
}

RuleSet::~RuleSet() {
	ClearRules();
}

void RuleSet::CopyRulesInto(RuleSet *in_rule_set) {
	assert(in_rule_set);

	map<int32, map<int32, Rule *> > * in_rules = in_rule_set->GetRules();
	map<int32, map<int32, Rule *> >::iterator itr;
	map<int32, Rule *>::iterator itr2;
	Rule * rule;
	
	ClearRules();
	m_rules.writelock(__FUNCTION__, __LINE__);
	for (itr = in_rules->begin(); itr != in_rules->end(); itr++) {
		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++) {
			rule = itr2->second;
			rules[rule->GetCategory()][rule->GetType()] = new Rule(rule);
		}
	}
	m_rules.releasewritelock(__FUNCTION__, __LINE__);
}

void RuleSet::AddRule(Rule *rule) {
	int32 category, type;

	assert(rule);

	category = rule->GetCategory();
	type = rule->GetType();
	m_rules.writelock(__FUNCTION__, __LINE__);
	if (rules[category].count(type) == 0)
		rules[category][type] = rule;
	else
		rules[category][type]->SetValue(rule->GetValue());
	m_rules.releasewritelock(__FUNCTION__, __LINE__);
}

Rule * RuleSet::GetRule(int32 category, int32 type) {
	Rule *ret = 0;

	m_rules.readlock(__FUNCTION__, __LINE__);
	if (rules[category].count(type) > 0)
		ret = rules[category][type];
	m_rules.releasereadlock(__FUNCTION__, __LINE__);

	if (!ret)
		ret = rule_manager.GetBlankRule();

	LogWrite(RULESYS__DEBUG, 5, "Rules", "Rule: %s, Value: %s", ret->GetCombined(), ret->GetValue());
	return ret;
}

Rule * RuleSet::GetRule(const char *category, const char *type) {
	map<int32, map<int32, Rule *> >::iterator itr;
	map<int32, Rule *>::iterator itr2;
	char combined[256];
	Rule *ret = 0;

	snprintf(combined, sizeof(combined), "%s:%s", category, type);
	// Zero terminate ([max - 1] = 0) to prevent a warning/error
	combined[255] = 0;

	m_rules.readlock(__FUNCTION__, __LINE__);
	for (itr = rules.begin(); itr != rules.end(); itr++) {
		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++) {
			if (!strcmp(itr2->second->GetCombined(), combined)) {
				ret = itr2->second;
				break;
			}
		}
	}
	m_rules.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

void RuleSet::ClearRules() {
	map<int32, map<int32, Rule *> >::iterator itr;
	map<int32, Rule *>::iterator itr2;

	m_rules.writelock(__FUNCTION__, __LINE__);
	for (itr = rules.begin(); itr != rules.end(); itr++) {
		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++)
			safe_delete(itr2->second);
	}
	rules.clear();
	m_rules.releasewritelock(__FUNCTION__, __LINE__);
}

RuleManager::RuleManager() {
	m_rule_sets.SetName("RuleManager::rule_sets");
	m_global_rule_set.SetName("RuleManager::global_rule_set");
	m_zone_rule_sets.SetName("RuleManager::zone_rule_sets");

	Init();
}

RuleManager::~RuleManager() {
	Flush();
}

void RuleManager::Init()
{
#define RULE_INIT(category, type, value) rules[category][type] = new Rule(category, type, value, #category ":" #type)

	/* CLIENT */
	RULE_INIT(R_Client, ShowWelcomeScreen, "0");
	RULE_INIT(R_Client, GroupSpellsTimer, "1000");
	RULE_INIT(R_Client, QuestQueueTimer, "50"); // in milliseconds

	/* FACTION */
	RULE_INIT(R_Faction, AllowFactionBasedCombat, "1");

	/* GUILD */
	RULE_INIT(R_Guild, MaxLevel, "50");
	RULE_INIT(R_Guild, MaxPlayers, "-1");

	/* PLAYER */
	RULE_INIT(R_Player, MaxLevel, "50");
	RULE_INIT(R_Player, MaxLevelOverrideStatus, "100");
	RULE_INIT(R_Player, VitalityAmount, ".5");
	RULE_INIT(R_Player, VitalityFrequency, "3600");
	RULE_INIT(R_Player, XPMultiplier, "1.0");
	RULE_INIT(R_Player, TSXPMultiplier, "1.0");
	RULE_INIT(R_Player, MaxAA, "320");
	RULE_INIT(R_Player, MaxClassAA, "100");
	RULE_INIT(R_Player, MaxSubclassAA, "100");
	RULE_INIT(R_Player, MaxShadowsAA, "70");
	RULE_INIT(R_Player, MaxHeroicAA, "50");
	RULE_INIT(R_Player, MaxTradeskillAA, "40");
	RULE_INIT(R_Player, MaxPrestigeAA, "25");
	RULE_INIT(R_Player, MaxTradeskillPrestigeAA, "25");
	RULE_INIT(R_Player, MinLastNameLevel, "20");
	RULE_INIT(R_Player, MaxLastNameLength, "20");
	RULE_INIT(R_Player, MinLastNameLength, "4");
	RULE_INIT(R_Player, DisableHouseAlignmentRequirement, "1");
	RULE_INIT(R_Player, MentorItemDecayRate, ".05"); // 5% per level lost when mentoring
	RULE_INIT(R_Player, TemporaryItemLogoutTime, "1800.0"); // time in seconds (double) for temporary item to decay after being logged out for a period of time, 30 min is the default
	RULE_INIT(R_Player, HeirloomItemShareExpiration, "172800.0"); // 2 days ('48 hours') in seconds
	RULE_INIT(R_Player, SwimmingSkillMinSpeed, "20"); 
	RULE_INIT(R_Player, SwimmingSkillMaxSpeed, "200");
	RULE_INIT(R_Player, SwimmingSkillMinBreathLength, "30");
	RULE_INIT(R_Player, SwimmingSkillMaxBreathLength, "1000");
	RULE_INIT(R_Player, AutoSkillUpBaseSkills, "0"); // when set to 1 we auto skill to max value on levelling up for armor,shield,class,weapon skills
	RULE_INIT(R_Player, MaxWeightStrengthMultiplier, "2.0"); // multiplier for strength to add to max weight, eg 25 str * 2.0 = 50 max weight + base weight
	RULE_INIT(R_Player, BaseWeight, "50"); // base weight per class, added to max weight with the strength multiplier
	RULE_INIT(R_Player, WeightPercentImpact, "0.01"); // overweight in stone speed impact (.01 = 1% per 1 stone)
	RULE_INIT(R_Player, WeightPercentCap, "0.95"); // cap total impact for being overweight (.95 = 95%)
	RULE_INIT(R_Player, CoinWeightPerStone, "40.0"); // coin weight per stone, 40.0 = 40 coins per 1 stone (per DoF client hover over)
	RULE_INIT(R_Player, WeightInflictsSpeed, "1"); // whether weight will inflict speed, 1 = on, 0 = off
	RULE_INIT(R_Player, LevelMasterySkillMultiplier, "5"); // multiplier for adventure level / recommended level when applying mastery damage to determine if they are in mastery range
	RULE_INIT(R_Player, TraitTieringSelection, "1"); // when set to true limited to single trait per group, otherwise you can freely select from any group
	RULE_INIT(R_Player, ClassicTraitLevelTable, "1"); // uses built in table based on Prima Guide, see Traits.cpp for more, otherwise  uses the levels below
	RULE_INIT(R_Player, TraitFocusSelectLevel, "9"); // x levels to receive new trait of focus, eg level/rule, level 10, rule value 5, 10/5 = 2 focus traits available at level 10
	RULE_INIT(R_Player, TraitTrainingSelectLevel, "10"); // x levels to receive new trait of focus
	RULE_INIT(R_Player, TraitRaceSelectLevel, "10"); // x levels to receive new trait of focus
	RULE_INIT(R_Player, TraitCharacterSelectLevel, "10"); // x levels to receive new trait of focus
	RULE_INIT(R_Player, StartHPBase, "40");
	RULE_INIT(R_Player, StartPowerBase, "45");
	RULE_INIT(R_Player, StartHPLevelMod, "2.0");
	RULE_INIT(R_Player, StartPowerLevelMod, "2.1");
	RULE_INIT(R_Player, AllowPlayerEquipCombat, "1");
	RULE_INIT(R_Player, MaxTargetCommandDistance, "50.0"); // max distance allowed for /target command when target name is not in group
	RULE_INIT(R_Player, MinSkillMultiplierValue, "30"); // min skill we use as a multiplier to note the max skill allowed by the node
	RULE_INIT(R_Player, HarvestSkillUpMultiplier, "2.0"); /* multiplier for node to take the "min skill" max and use a multiplier to offset the max skill allowed to skill up on node.
														  ** Eg. 50 min skill on node, 50*1.5=75, no one with higher than 75 skill gets a skill up
														  */
	/* PVP */
	RULE_INIT(R_PVP, AllowPVP, "0");
	RULE_INIT(R_PVP, LevelRange, "4");
	RULE_INIT(R_PVP, InvisPlayerDiscoveryRange, "20"); // value > 0 sets radius inner to see, = 0 means always seen, -1 = never seen
	RULE_INIT(R_PVP, PVPMitigationModByLevel, "25"); // gives a bonus to mitigation for PVP combat to offset the percentage level * mod (default 25)

	/* COMBAT */
	RULE_INIT(R_Combat, MaxCombatRange, "4.0");
	RULE_INIT(R_Combat, DeathExperienceDebt, "50.00"); // divide by 100, 50/100 = .5% debt per pve death
	RULE_INIT(R_Combat, PVPDeathExperienceDebt, "25.00"); // divide by 100, 25/100 = .25% debt per pvp death
	RULE_INIT(R_Combat, GroupExperienceDebt, "0"); // set to 1 means we will share debt between the group
	RULE_INIT(R_Combat, ExperienceToDebt, "50.00"); // percentage of xp earned to debt vs obtained xp 50/100 = 50% to debt
	RULE_INIT(R_Combat, ExperienceDebtRecoveryPercent, "5.00"); // recovery percentage per period of time, 5/100 = 5% recovered (so if .5% debt, .5*.05 = .025, .5-.025=.475% debt left)
	RULE_INIT(R_Combat, ExperienceDebtRecoveryPeriod, "600"); // every 10 minutes (x*60 seconds) recover ExperienceDebtRecoveryPercent
	RULE_INIT(R_Combat, EnableSpiritShards, "1");
	RULE_INIT(R_Combat, SpiritShardSpawnScript, "SpawnScripts/Generic/SpiritShard.lua");
	RULE_INIT(R_Combat, ShardDebtRecoveryPercent, "25.00"); // recovered percentage of debt upon obtainig shard, 25/100 means 25%.  If there is .5 DeathExperienceDebt, .5*25% = .125,  .5 - .125 = .375
	RULE_INIT(R_Combat, ShardRecoveryByRadius, "1"); // allow shards to auto pick up by radius, not requiring to click/right click the shard
	RULE_INIT(R_Combat, ShardLifetime, "86400"); // default: 86400 seconds (one day)
	RULE_INIT(R_Combat, EffectiveMitigationCapLevel, "80"); // level multiplier for max effective cap, level * 80 (default)
	RULE_INIT(R_Combat, CalculatedMitigationCapLevel, "100"); // The cap to calculate your mitigation from is [level*100].
	RULE_INIT(R_Combat, MitigationLevelEffectivenessMax, "1.5"); // ratio victim level / attacker level for max effectiveness, when victim is higher level cap can reach 1.5
	RULE_INIT(R_Combat, MitigationLevelEffectivenessMin, ".5"); // ratio victim level / attacker level for min effectiveness
	RULE_INIT(R_Combat, MaxMitigationAllowed, ".75"); // percentage max mitigation allowed, eg. 75% of damage can be mitigated max in PVE
	RULE_INIT(R_Combat, MaxMitigationAllowedPVP, ".75"); // percentage max mitigation allowed, eg. 75% of damage can be mitigated max in PVP
	RULE_INIT(R_Combat, StrengthNPC, "10"); // divider for strength NPC only str/x = additional dmg to low/high dmg
	RULE_INIT(R_Combat, StrengthOther, "25"); // divider for strength other than NPC str/x = additional dmg to low/high dmg
	RULE_INIT(R_Combat, MaxSkillBonusByLevel, "1.5"); // Level * 1.5 = max bonus skill allowed
	RULE_INIT(R_Combat, LockedEncounterNoAttack, "1"); // when set to 1, players/group members not part of the encounter cannot attack until /yell

	/* SPAWN */
	RULE_INIT(R_Spawn, SpeedMultiplier, "300"); // note: this value was 1280 until 6/1/2009, then was 600 til Sep 2009, when it became 300...?
	RULE_INIT(R_Spawn, ClassicRegen, "0");
	RULE_INIT(R_Spawn, HailMovementPause, "5000"); // time in milliseconds the spawn is paused on hail
	RULE_INIT(R_Spawn, HailDistance, "5"); // max distance to hail a spawn/npc
	RULE_INIT(R_Spawn, UseHardCodeWaterModelType, "1"); // uses alternate method of setting water type by model type (hardcoded) versus relying on just DB
	RULE_INIT(R_Spawn, UseHardCodeFlyingModelType, "1"); // uses alternate method of setting flying type by model type (hardcoded) versus relying on just DB

	/* TIMER */

	/* UI */
	RULE_INIT(R_UI, MaxWhoResults, "20");
	RULE_INIT(R_UI, MaxWhoOverrideStatus, "200");

	/* WORLD */
	RULE_INIT(R_World, DefaultStartingZoneID, "1");
	RULE_INIT(R_World, EnablePOIDiscovery, "0");
	RULE_INIT(R_World, GamblingTokenItemID, "2");
	RULE_INIT(R_World, GuildAutoJoin, "0");
	RULE_INIT(R_World, GuildAutoJoinID, "1");
	RULE_INIT(R_World, GuildAutoJoinDefaultRankID, "7");
	RULE_INIT(R_World, MaxPlayers, "-1");
	RULE_INIT(R_World, MaxPlayersOverrideStatus, "100");
	RULE_INIT(R_World, ServerLocked, "0");
	RULE_INIT(R_World, ServerLockedOverrideStatus, "10");
	RULE_INIT(R_World, SyncZonesWithLogin, "1");
	RULE_INIT(R_World, SyncEquipWithLogin, "1");
	RULE_INIT(R_World, UseBannedIPsTable, "0");
	RULE_INIT(R_World, LinkDeadTimer, "120000");					// default: 2 minutes
	RULE_INIT(R_World, RemoveDisconnectedClientsTimer, "30000");	// default: 30 seconds
	RULE_INIT(R_World, PlayerCampTimer, "20");						// default: 20 seconds
	RULE_INIT(R_World, GMCampTimer, "1");							// default: 1 second
	RULE_INIT(R_World, AutoAdminPlayers, "0");						// default: No
	RULE_INIT(R_World, AutoAdminGMs, "0");							// default: No
	RULE_INIT(R_World, AutoAdminStatusValue, "10");					// default: 10 (CSR)
	RULE_INIT(R_World, DuskTime, "20:00");							// default: 8pm
	RULE_INIT(R_World, DawnTime, "8:00");							// default: 8am
	RULE_INIT(R_World, ThreadedLoad, "0");							// default: no threaded loading
	RULE_INIT(R_World, TradeskillSuccessChance, "87.0");			// default: 87% chance of success while crafting
	RULE_INIT(R_World, TradeskillCritSuccessChance, "2.0");			// default: 2% chance of critical success while crafting
	RULE_INIT(R_World, TradeskillFailChance, "10.0");				// default: 10% chance of failure while crafting
	RULE_INIT(R_World, TradeskillCritFailChance, "1.0");			// default: 1% chance of critical failure while crafting
	RULE_INIT(R_World, TradeskillEventChance, "15.0");				// default: 15% chance of a tradeskill event while crafting
	RULE_INIT(R_World, EditorURL, "www.eq2emulator.net");			// default: www.eq2emulator.net
	RULE_INIT(R_World, EditorIncludeID, "0");						// default: 0 (0 = disabled, 1 = enabled)
	RULE_INIT(R_World, EditorOfficialServer, "0");					// default: 0 (0 = disabled, 1 = enabled)
	RULE_INIT(R_World, SavePaperdollImage, "1");					// default: true
	RULE_INIT(R_World, SaveHeadshotImage, "1");						// default: true
	RULE_INIT(R_World, SendPaperdollImagesToLogin, "1");			// default: true
	RULE_INIT(R_World, TreasureChestDisabled, "0");					// default: false
	RULE_INIT(R_World, StartingZoneLanguages, "0");					// default: 0 (0 = Live Like, 1 = Starting City Based)
	RULE_INIT(R_World, StartingZoneRuleFlag, "0");					// default: 0 - match any options available, just based on version/other fields (will not force qc/outpost)
																	// 1 - force split zones on alignment/deity despite client selection (queens colony/overlord outpost)
																	// 4 - send to 'new' starting zones, won't support old clients
																	// 8 - (isle of refuge)
	RULE_INIT(R_World, EnforceRacialAlignment, "1");
	RULE_INIT(R_World, MemoryCacheZoneMaps, "0");					// 0 disables caching the zone maps in memory, too many individual/unique zones entered may cause a lot of memory build up
	RULE_INIT(R_World, AutoLockEncounter, "0");						// When set to 0 we require player to attack to lock the encounter, otherwise if 1 then npc can auto lock encounter
	RULE_INIT(R_World, DisplayItemTiers, "1");						// Display item tiers when set to 1, otherwise do not
	RULE_INIT(R_World, LoreAndLegendAccept, "0"); 					// default: 0 - L&L quests accepted only through starter books. 1 - L&L quests can be started by examining bodyparts.

	//INSERT INTO `ruleset_details`(`id`, `ruleset_id`, `rule_category`, `rule_type`, `rule_value`, `description`) VALUES (NULL, '1', 'R_World', '', '', '')

	/* ZONE */
	RULE_INIT(R_Zone, MaxPlayers, "100");
	RULE_INIT(R_Zone, MinZoneLevelOverrideStatus, "1");
	RULE_INIT(R_Zone, MinZoneAccessOverrideStatus, "100");
	RULE_INIT(R_Zone, WeatherEnabled, "1");							// default: 1 (0 = disabled, 1 = enabled)
	RULE_INIT(R_Zone, WeatherType, "0");							// default: 1 (0 = normal, 1 = dynamic, 2 = random, 3 = chaotic)
	RULE_INIT(R_Zone, MinWeatherSeverity, "0.0");					// default: 0.0 or no weather
	RULE_INIT(R_Zone, MaxWeatherSeverity, "1.0");					// default: 1.0 or hard rain (range 0.0 - 1.0, rain starts at 0.75)
	RULE_INIT(R_Zone, WeatherChangeFrequency, "300");				// default: 5 minutes
	RULE_INIT(R_Zone, WeatherChangePerInterval, "0.02");			// default: 0.02 (slight changes)
	RULE_INIT(R_Zone, WeatherChangeChance, "20");					// default: 20% (in whole percents)
	RULE_INIT(R_Zone, WeatherDynamicMaxOffset, "0.08");				// default: 0.08 - dynamic weather changes can only change this max amount
	RULE_INIT(R_Zone, SpawnUpdateTimer, "50");						// default: 50ms - how often to check for spawn update sends
	RULE_INIT(R_Zone, CheckAttackNPC, "2000");                      // default: 2 seconds, how often to for NPCs to attack eachother
	RULE_INIT(R_Zone, CheckAttackPlayer, "2000");                   // default: 2 seconds, how often to check for NPCs to attack players
	RULE_INIT(R_Zone, HOTime, "10.0");								// default: 10 seconds, time to complete the HO wheel before it expires

	/* ZONE TIMERS */
	RULE_INIT(R_Zone, RegenTimer, "6000");
	RULE_INIT(R_Zone, ClientSaveTimer, "60000");
	RULE_INIT(R_Zone, ShutdownDelayTimer, "120000");
	RULE_INIT(R_Zone, WeatherTimer, "60000");						// default: 1 minute
	RULE_INIT(R_Zone, SpawnDeleteTimer, "30000");					// default: 30 seconds, how long a spawn pointer is held onto after being removed from the world before deleting it
	RULE_INIT(R_Zone, UseMapUnderworldCoords, "1"); // use maps lowest Y coordinate to establish underworld markers
	RULE_INIT(R_Zone, MapUnderworldCoordOffset, "-200.0"); // adds (or in the case of negative value subtracts) so that the underworld marker is lower when map is using its lowest Y coordinate
	
	RULE_INIT(R_Zone, SharedZoneMaxPlayers, "30"); // max players in a shared zone (non instanced) before splitting to another zone, city_zone flagged are exempt
	
	RULE_INIT(R_Loot, LootRadius, "5.0");
	RULE_INIT(R_Loot, AutoDisarmChest, "1");
	RULE_INIT(R_Loot, ChestTriggerRadiusGroup, "10.0"); // radius at which chest will trigger against group members
	RULE_INIT(R_Loot, ChestUnlockedTimeDrop, "1200"); // time in seconds, 20 minutes by default, triggers only if AllowChestUnlockByDropTime is 1
	RULE_INIT(R_Loot, AllowChestUnlockByDropTime, "1"); // when set to 1 we will start a countdown timer to allow anyone to loot once ChestUnlockedTimeDrop elapsed
	RULE_INIT(R_Loot, ChestUnlockedTimeTrap, "600"); // time in seconds, 10 minutes by default
	RULE_INIT(R_Loot, AllowChestUnlockByTrapTime, "1"); // when set to 1 we will allow unlocking the chest to all players after the trap is triggered (or chest is open) and period ChestUnlockedTimeTrap elapsed
	RULE_INIT(R_Loot, SkipLootGrayMob, "1");
	RULE_INIT(R_Loot, LootDistributionTime, "120"); // time in seconds that we allow the group to determine their loot decision (lotto/need/greed/decline).

	RULE_INIT(R_Spells, NoInterruptBaseChance, "50");
	RULE_INIT(R_Spells, EnableFizzleSpells, "1"); // enables/disables the 'fizzling' of spells based on can_fizzle in the spells table.  This also enables increasing specialized skills for classes based on spells/abilities.
	RULE_INIT(R_Spells, DefaultFizzleChance, "10.0"); // default percentage x / 100, eg 10% is 10.0
	RULE_INIT(R_Spells, FizzleMaxSkill, "1.2"); // 1.0 is 100%, 1.2 is 120%, so you get 120% your max skill against a spell, no fizzle
	RULE_INIT(R_Spells, FizzleDefaultSkill, ".2"); // offset against MaxSkill to average out to 100%, default of .2f so we don't go over the threshold if no skill
	RULE_INIT(R_Spells, EnableCrossZoneGroupBuffs, "0"); // enables/disables allowing cross zone group buffs
	RULE_INIT(R_Spells, EnableCrossZoneTargetBuffs, "0"); // enables/disables allowing cross zone target buffs
	RULE_INIT(R_Spells, PlayerSpellSaveStateWaitInterval, "100"); // time in milliseconds we wait before performing a save when the spell save trigger is activated, allows additional actions to take place until the cap is hit
	RULE_INIT(R_Spells, PlayerSpellSaveStateCap, "1000"); // sets a maximum wait time before we queue a spell state save to the DB, given a lot can go on in a short period with players especially in combat, maybe good to have this at a higher interval.
	RULE_INIT(R_Spells, RequirePreviousTierScribe, "0"); // requires step up apprentice -> apprentice (handcrafted?) -> journeyman (handcrafted?) -> adept -> expert -> master
	RULE_INIT(R_Spells, CureSpellID, "110003"); // Base Cure spell that was used after they removed cure types
	RULE_INIT(R_Spells, CureCurseSpellID, "110004"); // Curse Spell ID in the spells database
	RULE_INIT(R_Spells, CureNoxiousSpellID, "110005"); // Noxious/Poison Spell ID in the spells database
	RULE_INIT(R_Spells, CureMagicSpellID, "210006"); // Magic/Elemental Spell ID in the spells database
	RULE_INIT(R_Spells, CureTraumaSpellID, "0"); // Trauma/Mental Spell ID in the spells database
	RULE_INIT(R_Spells, CureArcaneSpellID, "0"); // Arcane/Heat Spell ID in the spells database
	RULE_INIT(R_Spells, MinistrationSkillID, "366253016"); // ministration skill id used to map power reduction rule MinistrationPowerReductionMax
	RULE_INIT(R_Spells, MinistrationPowerReductionMax, "15.0"); // max percentage of power reduction for spells with ministration mastery skill (default is 15.0 for 15%)
	RULE_INIT(R_Spells, MinistrationPowerReductionSkill, "25"); // divides by integer value to establish how much skill req for higher power reduction
	RULE_INIT(R_Spells, MasterSkillReduceSpellResist, "25"); // divides by integer value to establish how much skill bonus for reducing spell resistance on target
	
	RULE_INIT(R_Expansion, GlobalExpansionFlag, "0");
	RULE_INIT(R_Expansion, GlobalHolidayFlag, "0");

	RULE_INIT(R_World, DatabaseVersion, "0");

	//devn00b
	RULE_INIT(R_Discord, DiscordEnabled, "0"); //Enable/Disable built in discord bot.
	RULE_INIT(R_Discord, DiscordWebhookURL, "None"); //Webhook url used for server -> discord messages.
	RULE_INIT(R_Discord, DiscordBotToken, "None"); //Bot token used to connect to discord and provides discord -> server messages.
	RULE_INIT(R_Discord, DiscordChannel, "Discord"); // in-game channel used for server -> discord messages.
	RULE_INIT(R_Discord, DiscordListenChan, "0"); // Discord ChannelID used for discord->server messages.
#undef RULE_INIT
}

void RuleManager::Flush(bool reinit)
{
	map<int32, map<int32, Rule*> >::iterator itr;
	map<int32, Rule*>::iterator itr2;

	for (itr = rules.begin(); itr != rules.end(); itr++) {
		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++)
			safe_delete(itr2->second);
	}

	rules.clear();

	ClearRuleSets();
	ClearZoneRuleSets();

	if (reinit)
		Init();
}

void RuleManager::LoadCodedDefaultsIntoRuleSet(RuleSet *rule_set) {
	map<int32, map<int32, Rule *> >::iterator itr;
	map<int32, Rule *>::iterator itr2;

	assert(rule_set);

	for (itr = rules.begin(); itr != rules.end(); itr++) {
		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++)
			rule_set->AddRule(new Rule(itr2->second));
	}
}

bool RuleManager::AddRuleSet(RuleSet *rule_set) {
	bool ret = false;
	int32 id;

	assert(rule_set);

	id = rule_set->GetID();
	m_rule_sets.writelock(__FUNCTION__, __LINE__);
	if (rule_sets.count(id) == 0) {
		rule_sets[id] = rule_set;
		ret = true;
	}
	m_rule_sets.releasewritelock(__FUNCTION__, __LINE__);

	return ret;
}

int32 RuleManager::GetNumRuleSets() {
	int32 ret;

	m_rule_sets.readlock(__FUNCTION__, __LINE__);
	ret = rule_sets.size();
	m_rule_sets.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

void RuleManager::ClearRuleSets() {
	map<int32, RuleSet *>::iterator itr;

	m_rule_sets.writelock(__FUNCTION__, __LINE__);
	for (itr = rule_sets.begin(); itr != rule_sets.end(); itr++)
		safe_delete(itr->second);
	rule_sets.clear();
	m_rule_sets.releasewritelock(__FUNCTION__, __LINE__);
}

bool RuleManager::SetGlobalRuleSet(int32 rule_set_id) {
	if (rule_sets.count(rule_set_id) == 0)
		return false;

	global_rule_set.CopyRulesInto(rule_sets[rule_set_id]);
	return true;
}

Rule * RuleManager::GetGlobalRule(int32 category, int32 type) {
	return global_rule_set.GetRule(category, type);
}

Rule * RuleManager::GetGlobalRule(const char* category, const char* type) {
	return global_rule_set.GetRule(category, type);
}

bool RuleManager::SetZoneRuleSet(int32 zone_id, int32 rule_set_id) {
	bool ret = true;
	RuleSet *rule_set;

	m_rule_sets.readlock(__FUNCTION__, __LINE__);
	if (rule_sets.count(rule_set_id) == 0)
		ret = false;

	rule_set = rule_sets[rule_set_id];
	if (ret) {
		m_zone_rule_sets.writelock(__FUNCTION__, __LINE__);
		zone_rule_sets[zone_id] = rule_set;
		m_zone_rule_sets.releasewritelock(__FUNCTION__, __LINE__);
	}
	m_rule_sets.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

Rule * RuleManager::GetZoneRule(int32 zone_id, int32 category, int32 type) {
	Rule *ret = 0;

	/* first try to get the zone rule */
	if(zone_id) {
		m_zone_rule_sets.readlock(__FUNCTION__, __LINE__);
		if (zone_rule_sets.count(zone_id) > 0)
			ret = zone_rule_sets[zone_id]->GetRule(category, type);
		m_zone_rule_sets.releasereadlock(__FUNCTION__, __LINE__);
	}
	if(!ret) {
		ret = GetGlobalRule(category, type);
	}
	
	return ret;
}

void RuleManager::ClearZoneRuleSets() {
	m_zone_rule_sets.writelock(__FUNCTION__, __LINE__);
	zone_rule_sets.clear();
	m_zone_rule_sets.releasewritelock(__FUNCTION__, __LINE__);
}