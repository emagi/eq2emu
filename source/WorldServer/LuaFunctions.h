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
#ifndef LUA_FUNCTIONS_H
#define LUA_FUNCTIONS_H

#include <lua.hpp>
#include <vector>
#include <string>
#include <map>
using namespace std;

vector<string> ParseString(string strVal, char delim=',');
vector<unsigned int> ParseStringToInt32(string strVal, char delim=',');
map<string, signed int> ParseStringMap(string strVal, char delim=',');
map<unsigned int, unsigned short> ParseIntMap(string strVal, char delim = ',');
map<unsigned int, signed int> ParseSInt32Map(string strVal, char delim = ',');


//Sets
int EQ2Emu_lua_SetCurrentHP(lua_State* state);
int EQ2Emu_lua_SetMaxHP(lua_State* state);
int EQ2Emu_lua_SetMaxHPBase(lua_State* state);
int EQ2Emu_lua_SetCurrentPower(lua_State* state);
int EQ2Emu_lua_SetMaxPower(lua_State* state);
int EQ2Emu_lua_SetMaxPowerBase(lua_State* state);
int EQ2Emu_lua_ModifyMaxHP(lua_State* state);
int EQ2Emu_lua_ModifyMaxPower(lua_State* state);
int EQ2Emu_lua_SetHeading(lua_State* state);
int EQ2Emu_lua_SetModelType(lua_State* state);
int EQ2Emu_lua_SetAdventureClass(lua_State* state);
int EQ2Emu_lua_SetTradeskillClass(lua_State* state);
int EQ2Emu_lua_SetMount(lua_State* state);
int EQ2Emu_lua_SetMountColor(lua_State* state);
int EQ2Emu_lua_SetSpeed(lua_State* state);
int EQ2Emu_lua_SetPosition(lua_State* state);
int EQ2Emu_lua_AddSpellBonus(lua_State* state);
int EQ2Emu_lua_RemoveSpellBonus(lua_State* state);
int EQ2Emu_lua_AddSkillBonus(lua_State* state);
int EQ2Emu_lua_RemoveSkillBonus(lua_State* state);
int EQ2Emu_lua_AddControlEffect(lua_State* state);
int EQ2Emu_lua_RemoveControlEffect(lua_State* state);
int EQ2Emu_lua_HasControlEffect(lua_State* state);

int EQ2Emu_lua_GetBaseAggroRadius(lua_State* state);
int EQ2Emu_lua_GetAggroRadius(lua_State* state);
int EQ2Emu_lua_SetAggroRadius(lua_State* state);

int EQ2Emu_lua_SetDeity(lua_State* state);
int EQ2Emu_lua_GetDeity(lua_State* state);

int EQ2Emu_lua_SetInt(lua_State* state);
int EQ2Emu_lua_SetWis(lua_State* state);
int EQ2Emu_lua_SetSta(lua_State* state);
int EQ2Emu_lua_SetStr(lua_State* state);
int EQ2Emu_lua_SetAgi(lua_State* state);
int EQ2Emu_lua_SetIntBase(lua_State* state);
int EQ2Emu_lua_SetWisBase(lua_State* state);
int EQ2Emu_lua_SetStaBase(lua_State* state);
int EQ2Emu_lua_SetStrBase(lua_State* state);
int EQ2Emu_lua_SetAgiBase(lua_State* state);
int EQ2Emu_lua_SetLootCoin(lua_State* state);
int EQ2Emu_lua_HasCoin(lua_State* state);
int EQ2Emu_lua_SetQuestYellow(lua_State* state);

//Gets
int EQ2Emu_lua_GetLevel(lua_State* state);
int EQ2Emu_lua_GetDifficulty(lua_State* state);
int EQ2Emu_lua_GetCurrentHP(lua_State* state);
int EQ2Emu_lua_GetMaxHP(lua_State* state);
int EQ2Emu_lua_GetMaxHPBase(lua_State* state);
int EQ2Emu_lua_GetCurrentPower(lua_State* state);
int EQ2Emu_lua_GetName(lua_State* state);
int EQ2Emu_lua_GetMaxPower(lua_State* state);
int EQ2Emu_lua_GetMaxPowerBase(lua_State* state);
int EQ2Emu_lua_GetDistance(lua_State* state);
int EQ2Emu_lua_GetX(lua_State* state);
int EQ2Emu_lua_GetY(lua_State* state);
int EQ2Emu_lua_GetZ(lua_State* state);
int EQ2Emu_lua_GetHeading(lua_State* state);
int EQ2Emu_lua_GetModelType(lua_State* state);
int EQ2Emu_lua_GetRace(lua_State* state);
int EQ2Emu_lua_GetRaceName(lua_State* state);
int EQ2Emu_lua_GetMount(lua_State* state);
int EQ2Emu_lua_GetClass(lua_State* state);
int EQ2Emu_lua_GetClassName(lua_State* state);
int EQ2Emu_lua_GetArchetypeName(lua_State* state);
int EQ2Emu_lua_GetSpeed(lua_State* state);
int EQ2Emu_lua_HasMoved(lua_State* state);
int EQ2Emu_lua_GetInt(lua_State* state);
int EQ2Emu_lua_GetWis(lua_State* state);
int EQ2Emu_lua_GetSta(lua_State* state);
int EQ2Emu_lua_GetStr(lua_State* state);
int EQ2Emu_lua_GetAgi(lua_State* state);
int EQ2Emu_lua_GetIntBase(lua_State* state);
int EQ2Emu_lua_GetWisBase(lua_State* state);
int EQ2Emu_lua_GetStaBase(lua_State* state);
int EQ2Emu_lua_GetStrBase(lua_State* state);
int EQ2Emu_lua_GetAgiBase(lua_State* state);
int EQ2Emu_lua_GetLootCoin(lua_State* state);
int EQ2Emu_lua_GetSpawn(lua_State* state);
int EQ2Emu_lua_GetSpawnFromList(lua_State* state);
int EQ2Emu_lua_GetSpawnListSize(lua_State* state);
int EQ2Emu_lua_CreateSpawnList(lua_State* state);
int EQ2Emu_lua_AddSpawnToSpawnList(lua_State* state);
int EQ2Emu_lua_RemoveSpawnFromSpawnList(lua_State* state);
int EQ2Emu_lua_GetSpawnListBySpawnID(lua_State* state); 
int EQ2Emu_lua_GetSpawnListByRailID(lua_State* state); 
int EQ2Emu_lua_GetPassengerSpawnList(lua_State* state); 
int EQ2Emu_lua_GetVariableValue(lua_State* state);
int EQ2Emu_lua_GetCoinMessage(lua_State* state);
int EQ2Emu_lua_GetSpawnByGroupID(lua_State* state);
int EQ2Emu_lua_GetSpawnByLocationID(lua_State* state);
int EQ2Emu_lua_GetID(lua_State* state);
int EQ2Emu_lua_GetSpawnID(lua_State* state);
int EQ2Emu_lua_GetSpawnGroupID(lua_State* state);
int EQ2Emu_lua_SetSpawnGroupID(lua_State* state);
int EQ2Emu_lua_AddSpawnToGroup(lua_State* state);
int EQ2Emu_lua_GetSpawnLocationID(lua_State* state);
int EQ2Emu_lua_GetSpawnLocationPlacementID(lua_State* state);
int EQ2Emu_lua_GetFactionAmount(lua_State* state);
int EQ2Emu_lua_SetFactionID(lua_State* state);
int EQ2Emu_lua_GetFactionID(lua_State* state);
int EQ2Emu_lua_ChangeFaction(lua_State* state);
int EQ2Emu_lua_GetGender(lua_State* state);
int EQ2Emu_lua_GetTarget(lua_State* state);
int EQ2Emu_lua_HasFreeSlot(lua_State* state);
int EQ2Emu_lua_HasItemEquipped(lua_State* state);
int EQ2Emu_lua_GetEquippedItemBySlot(lua_State* state);
int EQ2Emu_lua_GetEquippedItemByID(lua_State* state);
int EQ2Emu_lua_SetEquippedItemByID(lua_State* state);
int EQ2Emu_lua_SetEquippedItem(lua_State* state);
int EQ2Emu_lua_UnequipSlot(lua_State* state);
int EQ2Emu_lua_SetEquipment(lua_State* state);
int EQ2Emu_lua_GetItemByID(lua_State* state);
int EQ2Emu_lua_GetItemType(lua_State* state);
int EQ2Emu_lua_GetItemEffectType(lua_State* state);
int EQ2Emu_lua_GetSpellName(lua_State* state);

//Misc
int EQ2Emu_lua_SetAttackable(lua_State* state);
int EQ2Emu_lua_SendStateCommand(lua_State* state);
int EQ2Emu_lua_SpawnSet(lua_State* state);
int EQ2Emu_lua_KillSpawn(lua_State* state); 
int EQ2Emu_lua_KillSpawnByDistance(lua_State* state);
int EQ2Emu_lua_SpawnSetByDistance(lua_State* state);
int EQ2Emu_lua_SetRequiredQuest(lua_State* state);
int EQ2Emu_lua_SetRequiredHistory(lua_State* state);
int EQ2Emu_lua_Despawn(lua_State* state);
int EQ2Emu_lua_ChangeHandIcon(lua_State* state);
int EQ2Emu_lua_SetVisualFlag(lua_State* state);
int EQ2Emu_lua_SetInfoFlag(lua_State* state);
int EQ2Emu_lua_AddHate(lua_State* state);
int EQ2Emu_lua_GetZone(lua_State* state);
int EQ2Emu_lua_GetZoneName(lua_State* state);
int EQ2Emu_lua_GetZoneID(lua_State* state);
int EQ2Emu_lua_Zone(lua_State* state);
int EQ2Emu_lua_ModifyPower(lua_State* state);
int EQ2Emu_lua_ModifyHP(lua_State* state);
int EQ2Emu_lua_ModifyTotalPower(lua_State* state);
int EQ2Emu_lua_ModifyTotalHP(lua_State* state);
int EQ2Emu_lua_SpellHeal(lua_State* state);
int EQ2Emu_lua_SpellHealPct(lua_State* state);

int EQ2Emu_lua_AddItem(lua_State* state);
int EQ2Emu_lua_SummonItem(lua_State* state);
int EQ2Emu_lua_RemoveItem(lua_State* state);
int EQ2Emu_lua_HasItem(lua_State* state);
int EQ2Emu_lua_Spawn(lua_State* state);
int EQ2Emu_lua_AddSpawnAccess(lua_State* state);
int EQ2Emu_lua_CastSpell(lua_State* state);
int EQ2Emu_lua_SpellDamage(lua_State* state);
int EQ2Emu_lua_SpellDamageExt(lua_State* state);
int EQ2Emu_lua_FaceTarget(lua_State* state);
int EQ2Emu_lua_MoveToLocation(lua_State* state);
int EQ2Emu_lua_ClearRunningLocations(lua_State* state);
int EQ2Emu_lua_Say(lua_State* state);
int EQ2Emu_lua_Shout(lua_State* state);
int EQ2Emu_lua_SayOOC(lua_State* state);
int EQ2Emu_lua_Emote(lua_State* state);
int EQ2Emu_lua_IsPlayer(lua_State* state);
int EQ2Emu_lua_GetCharacterID(lua_State* state);
int EQ2Emu_lua_MovementLoopAdd(lua_State* state);
int EQ2Emu_lua_GetCurrentZoneSafeLocation(lua_State* state);
int EQ2Emu_lua_PlayFlavor(lua_State* state);
int EQ2Emu_lua_PlayFlavorID(lua_State* state);
int EQ2Emu_lua_PlaySound(lua_State* state);
int EQ2Emu_lua_PlayVoice(lua_State* state);
int EQ2Emu_lua_PlayAnimation(lua_State* state);
int EQ2Emu_lua_PlayAnimationString(lua_State* state);
int EQ2Emu_lua_AddLootItem(lua_State* state);
int EQ2Emu_lua_HasLootItem(lua_State* state);
int EQ2Emu_lua_RemoveLootItem(lua_State* state);
int EQ2Emu_lua_AddLootCoin(lua_State* state);
int EQ2Emu_lua_GiveLoot(lua_State* state);
int EQ2Emu_lua_HasPendingLoot(lua_State* state);
int EQ2Emu_lua_HasPendingLootItem(lua_State* state);
int EQ2Emu_lua_CreateConversation(lua_State* state);
int EQ2Emu_lua_AddConversationOption(lua_State* state);
int EQ2Emu_lua_StartConversation(lua_State* state);
int EQ2Emu_lua_StartDialogConversation(lua_State* state);
//int EQ2Emu_lua_StartItemConversation(lua_State* state);
int EQ2Emu_lua_CloseConversation(lua_State* state);
int EQ2Emu_lua_CloseItemConversation(lua_State* state);
int EQ2Emu_lua_SetPlayerProximityFunction(lua_State* state);
int EQ2Emu_lua_SetLocationProximityFunction(lua_State* state);
int EQ2Emu_lua_IsBindAllowed(lua_State* state);
int EQ2Emu_lua_IsGateAllowed(lua_State* state);
int EQ2Emu_lua_Bind(lua_State* state);
int EQ2Emu_lua_Gate(lua_State* state);
int EQ2Emu_lua_IsAlive(lua_State* state);
int EQ2Emu_lua_IsSpawnGroupAlive(lua_State* state);
int EQ2Emu_lua_IsInCombat(lua_State* state);
int EQ2Emu_lua_SendMessage(lua_State* state);
int EQ2Emu_lua_SendPopUpMessage(lua_State* state);
int EQ2Emu_lua_SetServerControlFlag(lua_State* state);
int EQ2Emu_lua_ToggleTracking(lua_State* state);
int EQ2Emu_lua_AddPrimaryEntityCommand(lua_State* state);
int EQ2Emu_lua_AddSpellBookEntry(lua_State* state);
int EQ2Emu_lua_DeleteSpellBook(lua_State* state);
int EQ2Emu_lua_RemoveSpellBookEntry(lua_State* state);
int EQ2Emu_lua_SendNewAdventureSpells(lua_State* state);
int EQ2Emu_lua_SendNewTradeskillSpells(lua_State* state);
int EQ2Emu_lua_HasSpell(lua_State* state);
int EQ2Emu_lua_Attack(lua_State* state);
int EQ2Emu_lua_ApplySpellVisual(lua_State* state);
int EQ2Emu_lua_Interrupt(lua_State* state);
int EQ2Emu_lua_Stealth(lua_State* state);
int EQ2Emu_lua_IsStealthed(lua_State* state);
int EQ2Emu_lua_IsInvis(lua_State* state);
int EQ2Emu_lua_AddSpawnIDAccess(lua_State* state);
int EQ2Emu_lua_RemoveSpawnIDAccess(lua_State* state);
int EQ2Emu_lua_HasRecipeBook(lua_State* state);
int EQ2Emu_lua_SpawnMove(lua_State* state);
int EQ2Emu_lua_AddTransportSpawn(lua_State* state);
int EQ2Emu_lua_IsTransportSpawn(lua_State* state);
int	EQ2Emu_lua_PerformCameraShake(lua_State* state);

//Quest Stuff
int EQ2Emu_lua_SetStepComplete(lua_State* state);
int EQ2Emu_lua_AddStepProgress(lua_State* state);
int EQ2Emu_lua_GetTaskGroupStep(lua_State* state);
int EQ2Emu_lua_QuestStepIsComplete(lua_State* state);
int EQ2Emu_lua_GetQuestStep(lua_State* state);
int EQ2Emu_lua_RegisterQuest(lua_State* state);
int EQ2Emu_lua_OfferQuest(lua_State* state);
int EQ2Emu_lua_SetQuestPrereqLevel(lua_State* state);
int EQ2Emu_lua_AddQuestPrereqQuest(lua_State* state);
int EQ2Emu_lua_AddQuestPrereqItem(lua_State* state);
int EQ2Emu_lua_AddQuestPrereqFaction(lua_State* state);
int EQ2Emu_lua_AddQuestPrereqClass(lua_State* state);
int EQ2Emu_lua_AddQuestPrereqRace(lua_State* state);
int EQ2Emu_lua_AddQuestPrereqModelType(lua_State* state);
int EQ2Emu_lua_AddQuestPrereqTradeskillLevel(lua_State* state);
int EQ2Emu_lua_AddQuestPrereqTradeskillClass(lua_State* state);
int EQ2Emu_lua_HasQuestRewardItem(lua_State* state);
int EQ2Emu_lua_AddQuestRewardItem(lua_State* state);
int EQ2Emu_lua_AddQuestSelectableRewardItem(lua_State* state);
int EQ2Emu_lua_AddQuestRewardCoin(lua_State* state);
int EQ2Emu_lua_AddQuestRewardFaction(lua_State* state);
int EQ2Emu_lua_SetQuestRewardStatus(lua_State* state);
int EQ2Emu_lua_SetStatusTmpReward(lua_State* state);
int EQ2Emu_lua_SetCoinTmpReward(lua_State* state);
int EQ2Emu_lua_SetQuestRewardComment(lua_State* state);
int EQ2Emu_lua_SetQuestRewardExp(lua_State* state);
int EQ2Emu_lua_AddQuestStep(lua_State* state);
int EQ2Emu_lua_AddQuestStepKillLogic(lua_State* state);
int EQ2Emu_lua_AddQuestStepKill(lua_State* state);
int EQ2Emu_lua_AddQuestStepKillByRace(lua_State* state);
int EQ2Emu_lua_AddQuestStepChat(lua_State* state);
int EQ2Emu_lua_AddQuestStepObtainItem(lua_State* state);
int EQ2Emu_lua_AddQuestStepZoneLoc(lua_State* state);
int EQ2Emu_lua_AddQuestStepLocation(lua_State* state);
int EQ2Emu_lua_AddQuestStepLoc(lua_State* state);
int EQ2Emu_lua_AddQuestStepSpell(lua_State* state);
int EQ2Emu_lua_AddQuestStepCraft(lua_State* state);
int EQ2Emu_lua_AddQuestStepHarvest(lua_State* state);
int EQ2Emu_lua_AddQuestStepCompleteAction(lua_State* state);
int EQ2Emu_lua_AddQuestStepProgressAction(lua_State* state);
int EQ2Emu_lua_SetQuestCompleteAction(lua_State* state);
int EQ2Emu_lua_GiveQuestReward(lua_State* state);
int EQ2Emu_lua_UpdateQuestTaskGroupDescription(lua_State* state);
int EQ2Emu_lua_UpdateQuestStepDescription(lua_State* state);
int EQ2Emu_lua_UpdateQuestDescription(lua_State* state);
int EQ2Emu_lua_UpdateQuestZone(lua_State* state);
int EQ2Emu_lua_SetCompletedDescription(lua_State* state);
int EQ2Emu_lua_ProvidesQuest(lua_State* state);
int EQ2Emu_lua_HasQuest(lua_State* state);
int EQ2Emu_lua_HasCompletedQuest(lua_State* state);
int EQ2Emu_lua_QuestIsComplete(lua_State* state);
int EQ2Emu_lua_QuestReturnNPC(lua_State* state);
int EQ2Emu_lua_GetQuest(lua_State* state);
int EQ2Emu_lua_AddTimer(lua_State* state);
int EQ2Emu_lua_StopTimer(lua_State* state);
int EQ2Emu_lua_Harvest(lua_State* state);
int EQ2Emu_lua_SetCompleteFlag(lua_State* state);
int EQ2Emu_lua_CanReceiveQuest(lua_State* state);

int EQ2Emu_lua_HasCollectionsToHandIn(lua_State *state);
int EQ2Emu_lua_HandInCollections(lua_State *state);

int EQ2Emu_lua_UseWidget(lua_State* state);
int EQ2Emu_lua_SummonPet(lua_State* state);
int EQ2Emu_lua_Charm(lua_State* state);

int EQ2Emu_lua_SetSpellList(lua_State* state);
int EQ2Emu_lua_GetPet(lua_State* state);
int EQ2Emu_lua_GetGroup(lua_State* state);

int EQ2Emu_lua_CreateOptionWindow(lua_State* state);
int EQ2Emu_lua_AddOptionWindowOption(lua_State* state);
int EQ2Emu_lua_SendOptionWindow(lua_State* state);

int EQ2Emu_lua_GetTradeskillClass(lua_State* state);
int EQ2Emu_lua_GetTradeskillLevel(lua_State* state);
int EQ2Emu_lua_GetTradeskillClassName(lua_State* state);
int EQ2Emu_lua_SetTradeskillLevel(lua_State* state);

int EQ2Emu_lua_SummonDeityPet(lua_State* state);
int EQ2Emu_lua_SummonCosmeticPet(lua_State* state);
int EQ2Emu_lua_DismissPet(lua_State* state);
int EQ2Emu_lua_GetCharmedPet(lua_State* state);
int EQ2Emu_lua_GetDeityPet(lua_State* state);
int EQ2Emu_lua_GetCosmeticPet(lua_State* state);

int EQ2Emu_lua_SetQuestFeatherColor(lua_State* state);
int EQ2Emu_lua_RemoveSpawnAccess(lua_State* state);
int EQ2Emu_lua_SpawnByLocationID(lua_State* state);
int EQ2Emu_lua_SpawnGroupByID(lua_State* state);
int EQ2Emu_lua_CastEntityCommand(lua_State* state);
int EQ2Emu_lua_SetLuaBrain(lua_State* state);
int EQ2Emu_lua_SetBrainTick(lua_State* state);
int EQ2Emu_lua_SetFollowTarget(lua_State* state);
int EQ2Emu_lua_GetFollowTarget(lua_State* state);
int EQ2Emu_lua_ToggleFollow(lua_State* state);
int EQ2Emu_lua_IsFollowing(lua_State* state);
int EQ2Emu_lua_SetTempVariable(lua_State* state);
int EQ2Emu_lua_GetTempVariable(lua_State* state);
int EQ2Emu_lua_GiveQuestItem(lua_State*state);
int EQ2Emu_lua_SetQuestRepeatable(lua_State* state);


int EQ2Emu_lua_AddWaypoint(lua_State* state);
int EQ2Emu_lua_RemoveWaypoint(lua_State* state);
int EQ2Emu_lua_SendWaypoints(lua_State* state);

int EQ2Emu_lua_AddWard(lua_State* state);
int EQ2Emu_lua_AddToWard(lua_State* state);
int EQ2Emu_lua_RemoveWard(lua_State* state);
int EQ2Emu_lua_GetWardAmountLeft(lua_State* state);
int EQ2Emu_lua_GetWardValue(lua_State* state);

//Combat AI related
int EQ2Emu_lua_SetTarget(lua_State* state);
int EQ2Emu_lua_IsPet(lua_State* state);
int EQ2Emu_lua_GetOwner(lua_State* state);
int EQ2Emu_lua_SetInCombat(lua_State* state);
int EQ2Emu_lua_CompareSpawns(lua_State* state);
int EQ2Emu_lua_ClearRunback(lua_State* state);
int EQ2Emu_lua_Runback(lua_State* state);
int EQ2Emu_lua_GetRunbackDistance(lua_State* state);
int EQ2Emu_lua_IsCasting(lua_State* state);
int EQ2Emu_lua_IsMezzed(lua_State* state);
int EQ2Emu_lua_IsStunned(lua_State* state);
int EQ2Emu_lua_IsMezzedOrStunned(lua_State* state);
int EQ2Emu_lua_ClearEncounter(lua_State* state);
int EQ2Emu_lua_ClearHate(lua_State* state);
int EQ2Emu_lua_GetMostHated(lua_State* state);
int EQ2Emu_lua_GetEncounterSize(lua_State* state);
int EQ2Emu_lua_HasRecovered(lua_State* state);
int EQ2Emu_lua_ProcessMelee(lua_State* state);
int EQ2Emu_lua_ProcessSpell(lua_State* state);
int EQ2Emu_lua_GetEncounter(lua_State* state);
int EQ2Emu_lua_GetHateList(lua_State* state);
int EQ2Emu_lua_HasGroup(lua_State* state);
int EQ2Emu_lua_HasSpellEffect(lua_State* state);

int EQ2Emu_lua_SetSuccessTimer(lua_State* state);
int EQ2Emu_lua_SetFailureTimer(lua_State* state);
int EQ2Emu_lua_IsGroundSpawn(lua_State* state);
int EQ2Emu_lua_CanHarvest(lua_State* state);

int EQ2Emu_lua_SummonDumbFirePet(lua_State* state);

int EQ2Emu_lua_GetSkillValue(lua_State* state);
int EQ2Emu_lua_GetSkillMaxValue(lua_State* state);
int EQ2Emu_lua_GetSkillName(lua_State* state);
int EQ2Emu_lua_SetSkillMaxValue(lua_State* state);
int EQ2Emu_lua_SetSkillValue(lua_State* state);
int EQ2Emu_lua_GetSkill(lua_State* state);
int EQ2Emu_lua_GetSkillIDByName(lua_State* state);
int EQ2Emu_lua_HasSkill(lua_State* state);
int EQ2Emu_lua_AddSkill(lua_State* state);
int EQ2Emu_lua_RemoveSkill(lua_State* state);
int EQ2Emu_lua_IncreaseSkillCapsByType(lua_State* state);

int EQ2Emu_lua_AddProc(lua_State* state);
int EQ2Emu_lua_AddProcExt(lua_State* state);
int EQ2Emu_lua_RemoveProc(lua_State* state);
int EQ2Emu_lua_Knockback(lua_State* state);

int EQ2Emu_lua_IsEpic(lua_State* state);
int EQ2Emu_lua_IsHeroic(lua_State* state);
int EQ2Emu_lua_ProcDamage(lua_State* state);
int EQ2Emu_lua_LastSpellAttackHit(lua_State* state);

int EQ2Emu_lua_IsBehind(lua_State* state);
int EQ2Emu_lua_IsFlanking(lua_State* state);
int EQ2Emu_lua_InFront(lua_State* state);
int EQ2Emu_lua_AddSpellTimer(lua_State* state);
int EQ2Emu_lua_SetItemCount(lua_State* state);
int EQ2Emu_lua_GetItemCount(lua_State* state);
int EQ2Emu_lua_Resurrect(lua_State* state);
int EQ2Emu_lua_BreatheUnderwater(lua_State* state);
int EQ2Emu_lua_BlurVision(lua_State* state);
int EQ2Emu_lua_SetVision(lua_State* state);
int EQ2Emu_lua_GetItemSkillReq(lua_State* state);
int EQ2Emu_lua_SetSpeedMultiplier(lua_State* state);
int EQ2Emu_lua_SetIllusion(lua_State* state);
int EQ2Emu_lua_ResetIllusion(lua_State* state);
int EQ2Emu_lua_AddThreatTransfer(lua_State* state);
int EQ2Emu_lua_RemoveThreatTransfer(lua_State* state);
int EQ2Emu_lua_CureByType(lua_State* state);
int EQ2Emu_lua_CureByControlEffect(lua_State* state);
int EQ2Emu_lua_AddSpawnSpellBonus(lua_State* state);
int EQ2Emu_lua_RemoveSpawnSpellBonus(lua_State* state);
int EQ2Emu_lua_CancelSpell(lua_State* state);
int EQ2Emu_lua_RemoveStealth(lua_State* state);
int EQ2Emu_lua_RemoveInvis(lua_State* state);
int EQ2Emu_lua_StartHeroicOpportunity(lua_State* state);
int EQ2Emu_lua_CopySpawnAppearance(lua_State* state);
int EQ2Emu_lua_RemoveTriggerFromSpell(lua_State* state);
int EQ2Emu_lua_GetSpellTriggerCount(lua_State* state);
int EQ2Emu_lua_SetSpellTriggerCount(lua_State* state);
int EQ2Emu_lua_HasSpellImmunity(lua_State* state);
int EQ2Emu_lua_AddImmunitySpell(lua_State* state);
int EQ2Emu_lua_RemoveImmunitySpell(lua_State* state);
int EQ2Emu_lua_SetSpellSnareValue(lua_State* state);
int EQ2Emu_lua_CheckRaceType(lua_State* state);
int EQ2Emu_lua_GetRaceType(lua_State* state);
int EQ2Emu_lua_GetRaceBaseType(lua_State* state);
int EQ2Emu_lua_GetQuestFlags(lua_State* state);
int EQ2Emu_lua_SetQuestFlags(lua_State* state);
int EQ2Emu_lua_SetQuestTimer(lua_State* state);
int EQ2Emu_lua_RemoveQuestStep(lua_State* state);
int EQ2Emu_lua_ResetQuestStep(lua_State* state);
int EQ2Emu_lua_SetQuestTimerComplete(lua_State* state);
int EQ2Emu_lua_AddQuestStepFailureAction(lua_State* state);
int EQ2Emu_lua_SetStepFailed(lua_State* state);
int EQ2Emu_lua_GetQuestCompleteCount(lua_State* state);
int EQ2Emu_lua_SetServerVariable(lua_State* state);
int EQ2Emu_lua_GetServerVariable(lua_State* state);
int EQ2Emu_lua_HasLanguage(lua_State* state);
int EQ2Emu_lua_AddLanguage(lua_State* state);
int EQ2Emu_lua_IsNight(lua_State* state);
int EQ2Emu_lua_AddMultiFloorLift(lua_State* state);
int EQ2Emu_lua_StartAutoMount(lua_State* state);
int EQ2Emu_lua_EndAutoMount(lua_State* state);
int EQ2Emu_lua_IsOnAutoMount(lua_State* state);
int EQ2Emu_lua_SetPlayerHistory(lua_State* state);
int EQ2Emu_lua_GetPlayerHistory(lua_State* state);
int EQ2Emu_lua_SetGridID(lua_State* state);
int EQ2Emu_lua_GetQuestStepProgress(lua_State* state);
int EQ2Emu_lua_SetPlayerLevel(lua_State* state);
int EQ2Emu_lua_AddCoin(lua_State* state);
int EQ2Emu_lua_RemoveCoin(lua_State* state);
int EQ2Emu_lua_GetPlayersInZone(lua_State* state);
int EQ2Emu_lua_SetSpawnAnimation(lua_State* state);
int EQ2Emu_lua_GetClientVersion(lua_State* state);
int EQ2Emu_lua_GetItemID(lua_State* state);
int EQ2Emu_lua_IsEntity(lua_State* state);
int EQ2Emu_lua_GetOrigX(lua_State* state);
int EQ2Emu_lua_GetOrigY(lua_State* state);
int EQ2Emu_lua_GetOrigZ(lua_State* state);
int EQ2Emu_lua_GetPCTOfHP(lua_State* state);
int EQ2Emu_lua_GetPCTOfPower(lua_State* state);
int EQ2Emu_lua_GetBoundZoneID(lua_State* state);
int EQ2Emu_lua_Evac(lua_State* state);
int EQ2Emu_lua_GetSpellTier(lua_State* state);
int EQ2Emu_lua_GetSpellID(lua_State* state);
int EQ2Emu_lua_StartTransmute(lua_State* state);
int EQ2Emu_lua_CompleteTransmute(lua_State* state);
int EQ2Emu_lua_ProcHate(lua_State* state);

int EQ2Emu_lua_GiveExp(lua_State* state);
int EQ2Emu_lua_DisplayText(lua_State* state);
int EQ2Emu_lua_ShowLootWindow(lua_State* state);
int EQ2Emu_lua_GetRandomSpawnByID(lua_State* state);
int EQ2Emu_lua_AddPrimaryEntityCommandAllSpawns(lua_State* state);
int EQ2Emu_lua_InstructionWindow(lua_State* state);
int EQ2Emu_lua_InstructionWindowClose(lua_State* state);
int EQ2Emu_lua_InstructionWindowGoal(lua_State* state);
int EQ2Emu_lua_ShowWindow(lua_State* state);
int EQ2Emu_lua_FlashWindow(lua_State* state);
int EQ2Emu_lua_EnableGameEvent(lua_State* state);
int EQ2Emu_lua_GetTutorialStep(lua_State* state);
int EQ2Emu_lua_SetTutorialStep(lua_State* state);

int EQ2Emu_lua_CheckLOS(lua_State* state);
int EQ2Emu_lua_CheckLOSByCoordinates(lua_State* state);

int EQ2Emu_lua_SetZoneExpansionFlag(lua_State* state);
int EQ2Emu_lua_GetZoneExpansionFlag(lua_State* state);
int EQ2Emu_lua_SetZoneHolidayFlag(lua_State* state);
int EQ2Emu_lua_GetZoneHolidayFlag(lua_State* state);

int EQ2Emu_lua_SetCanBind(lua_State* state);
int EQ2Emu_lua_GetCanBind(lua_State* state);

int EQ2Emu_lua_GetCanGate(lua_State* state);
int EQ2Emu_lua_SetCanGate(lua_State* state);

int EQ2Emu_lua_GetCanEvac(lua_State* state);
int EQ2Emu_lua_SetCanEvac(lua_State* state);

int EQ2Emu_lua_AddSpawnProximity(lua_State* state);

int EQ2Emu_lua_CanSeeInvis(lua_State* state);
int EQ2Emu_lua_SetSeeInvis(lua_State* state);
int EQ2Emu_lua_SetSeeHide(lua_State* state);

int EQ2Emu_lua_SetAccessToEntityCommand(lua_State* state);
int EQ2Emu_lua_SetAccessToEntityCommandByCharID(lua_State* state);
int EQ2Emu_lua_RemovePrimaryEntityCommand(lua_State* state);
int EQ2Emu_lua_SendUpdateDefaultCommand(lua_State* state);


int EQ2Emu_lua_SendTransporters(lua_State* state);
int EQ2Emu_lua_SetTemporaryTransportID(lua_State* state);
int EQ2Emu_lua_GetTemporaryTransportID(lua_State* state);

int EQ2Emu_lua_GetAlignment(lua_State* state);
int EQ2Emu_lua_SetAlignment(lua_State* state);

int EQ2Emu_lua_GetSpell(lua_State* state);
int EQ2Emu_lua_GetSpellData(lua_State* state);
int EQ2Emu_lua_SetSpellData(lua_State* state);
int EQ2Emu_lua_CastCustomSpell(lua_State* state);

int EQ2Emu_lua_SetSpellDataIndex(lua_State* state);
int EQ2Emu_lua_GetSpellDataIndex(lua_State* state);

int EQ2Emu_lua_SetSpellDisplayEffect(lua_State* state);
int EQ2Emu_lua_GetSpellDisplayEffect(lua_State* state);

int EQ2Emu_lua_InWater(lua_State* state);
int EQ2Emu_lua_InLava(lua_State* state);

int EQ2Emu_lua_DamageSpawn(lua_State* state);

int EQ2Emu_lua_IsInvulnerable(lua_State* state);
int EQ2Emu_lua_SetInvulnerable(lua_State* state);

int EQ2Emu_lua_GetRuleFlagBool(lua_State* state);
int EQ2Emu_lua_GetRuleFlagInt32(lua_State* state);
int EQ2Emu_lua_GetRuleFlagFloat(lua_State* state);

int EQ2Emu_lua_GetAAInfo(lua_State* state);
int EQ2Emu_lua_SetAAInfo(lua_State* state);

int EQ2Emu_lua_AddMasterTitle(lua_State* state);
int EQ2Emu_lua_AddCharacterTitle(lua_State* state);
int EQ2Emu_lua_SetCharacterTitleSuffix(lua_State* state);
int EQ2Emu_lua_SetCharacterTitlePrefix(lua_State* state);
int EQ2Emu_lua_ResetCharacterTitleSuffix(lua_State* state);
int EQ2Emu_lua_ResetCharacterTitlePrefix(lua_State* state);

int EQ2Emu_lua_GetInfoStructString(lua_State* state);
int EQ2Emu_lua_GetInfoStructUInt(lua_State* state);
int EQ2Emu_lua_GetInfoStructSInt(lua_State* state);
int EQ2Emu_lua_GetInfoStructFloat(lua_State* state);

int EQ2Emu_lua_SetInfoStructString(lua_State* state);
int EQ2Emu_lua_SetInfoStructUInt(lua_State* state);
int EQ2Emu_lua_SetInfoStructSInt(lua_State* state);
int EQ2Emu_lua_SetInfoStructFloat(lua_State* state);

int EQ2Emu_lua_SetCharSheetChanged(lua_State* state);

int EQ2Emu_lua_AddPlayerMail(lua_State* state);
int EQ2Emu_lua_AddPlayerMailByCharID(lua_State* state);

int EQ2Emu_lua_OpenDoor(lua_State* state);
int EQ2Emu_lua_CloseDoor(lua_State* state);
int EQ2Emu_lua_IsOpen(lua_State* state);

int EQ2Emu_lua_MakeRandomInt(lua_State* state);
int EQ2Emu_lua_MakeRandomFloat(lua_State* state);

int EQ2Emu_lua_AddIconValue(lua_State* state);
int EQ2Emu_lua_RemoveIconValue(lua_State* state);

int EQ2Emu_lua_GetShardID(lua_State* state);
int EQ2Emu_lua_GetShardCharID(lua_State* state);
int EQ2Emu_lua_GetShardCreatedTimestamp(lua_State* state);
int EQ2Emu_lua_DeleteDBShardID(lua_State* state);

int EQ2Emu_lua_PauseMovement(lua_State* state);
int EQ2Emu_lua_StopMovement(lua_State* state);

int EQ2Emu_lua_GetArrowColor(lua_State* state);
int EQ2Emu_lua_GetTSArrowColor(lua_State* state);

int EQ2Emu_lua_GetSpawnByRailID(lua_State* state);
int EQ2Emu_lua_SetRailID(lua_State* state);
int EQ2Emu_lua_IsZoneLoading(lua_State* state);
int EQ2Emu_lua_IsRunning(lua_State* state);

int EQ2Emu_lua_GetZoneLockoutTimer(lua_State* state);

int EQ2Emu_lua_SetWorldTime(lua_State* state);
int EQ2Emu_lua_GetWorldTimeYear(lua_State* state);
int EQ2Emu_lua_GetWorldTimeMonth(lua_State* state);
int EQ2Emu_lua_GetWorldTimeHour(lua_State* state);
int EQ2Emu_lua_GetWorldTimeMinute(lua_State* state);
int EQ2Emu_lua_SendTimeUpdate(lua_State* state);


int EQ2Emu_lua_SetLootTier(lua_State* state);
int EQ2Emu_lua_GetLootTier(lua_State* state);

int EQ2Emu_lua_SetLootDropType(lua_State* state);
int EQ2Emu_lua_GetLootDropType(lua_State* state);

int EQ2Emu_lua_DamageEquippedItems(lua_State* state);

int EQ2Emu_lua_CreateWidgetRegion(lua_State* state);
int EQ2Emu_lua_RemoveRegion(lua_State* state);

int EQ2Emu_lua_SetPlayerPOVGhost(lua_State* state);
int EQ2Emu_lua_SetCastOnAggroComplete(lua_State* state);
int EQ2Emu_lua_IsCastOnAggroComplete(lua_State* state);

int EQ2Emu_lua_AddRecipeBookToPlayer(lua_State* state);
int EQ2Emu_lua_RemoveRecipeFromPlayer(lua_State* state);

int EQ2Emu_lua_ReplaceWidgetFromClient(lua_State* state);
int EQ2Emu_lua_RemoveWidgetFromSpawnMap(lua_State* state);
int EQ2Emu_lua_RemoveWidgetFromZoneMap(lua_State* state);

int EQ2Emu_lua_SendHearCast(lua_State* state);

int EQ2Emu_lua_GetCharacterFlag(lua_State* state);
int EQ2Emu_lua_ToggleCharacterFlag(lua_State* state);

int EQ2Emu_lua_GetSpellInitialTarget(lua_State* state);
int EQ2Emu_lua_GetSpellCaster(lua_State* state);
int EQ2Emu_lua_GetCasterSpellLevel(lua_State* state);
int EQ2Emu_lua_GetSpellTargets(lua_State* state);

int EQ2Emu_lua_DespawnByLocationID(lua_State* state);

int EQ2Emu_lua_AddRespawn(lua_State* state);
int EQ2Emu_lua_CreatePersistedRespawn(lua_State* state);

int EQ2Emu_lua_CreateChoiceWindow(lua_State* state);
int EQ2Emu_lua_ClearChoice(lua_State* state);
int EQ2Emu_lua_GetChoiceSpawnID(lua_State* state);

int EQ2Emu_lua_GetZonePlayerMinLevel(lua_State* state);
int EQ2Emu_lua_GetZonePlayerMaxLevel(lua_State* state);
int EQ2Emu_lua_GetZonePlayerAvgLevel(lua_State* state);
int EQ2Emu_lua_GetZonePlayerFirstLevel(lua_State* state);

int EQ2Emu_lua_GetSpellRequiredLevel(lua_State* state);
#endif