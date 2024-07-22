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

#include "Traits.h"
#include "../../common/ConfigReader.h"
#include "../../common/Log.h"
#include "../Spells.h"
#include "../WorldDatabase.h"
#include "../client.h"
#include "../Rules/Rules.h"
#include <boost/assign.hpp>

#include <map>

extern ConfigReader configReader;
extern MasterSpellList master_spell_list;
extern WorldDatabase database;
extern RuleManager rule_manager;

using namespace boost::assign;

MasterTraitList::MasterTraitList(){
	MMasterTraitList.SetName("MasterTraitList::TraitList");
}

MasterTraitList::~MasterTraitList(){
	DestroyTraits();
}

void MasterTraitList::AddTrait(TraitData* data){
	MMasterTraitList.writelock(__FUNCTION__, __LINE__);
	TraitList.push_back(data);
	MMasterTraitList.releasewritelock(__FUNCTION__, __LINE__);
}

int MasterTraitList::Size(){
	return TraitList.size();
}

bool MasterTraitList::GenerateTraitLists(Client* client, map <int8, map <int8, vector<TraitData*> > >* sortedTraitList, map <int8, vector<TraitData*> >* classTraining,
										map <int8, vector<TraitData*> >* raceTraits, map <int8, vector<TraitData*> >* innateRaceTraits, map <int8, vector<TraitData*> >* focusEffects, int16 max_level, int8 trait_group)
{
	if (!client) {
		LogWrite(SPELL__ERROR, 0, "Traits", "GetTraitListPacket called with an invalid client");
		return false;
	}
	// Sort the Data
	if (Size() == 0 || !sortedTraitList || !classTraining || !raceTraits || !innateRaceTraits || !focusEffects)
		return false;
	
	map <int8, map <int8, vector<TraitData*> > >::iterator itr;
	map <int8, vector<TraitData*> >::iterator itr2;
	vector<TraitData*>::iterator itr3;

	MMasterTraitList.readlock(__FUNCTION__, __LINE__);
	
	for (int i=0; i < Size(); i++) {
		if(max_level > 0 && TraitList[i]->level > max_level) {
			continue; // skip per max level requirement
		}
		if(trait_group != 255 && trait_group != TraitList[i]->group) {
			continue;
		}
		
		// Sort Character Traits
		if (TraitList[i]->classReq == 255 && TraitList[i]->raceReq == 255 && !TraitList[i]->isFocusEffect && TraitList[i]->isTrait) {
			itr = sortedTraitList->lower_bound(TraitList[i]->group);
			if (itr != sortedTraitList->end() && !(sortedTraitList->key_comp()(TraitList[i]->group, itr->first))) {

				itr2 = (itr->second).lower_bound(TraitList[i]->level);
				if (itr2 != (itr->second).end() && !((itr->second).key_comp()(TraitList[i]->level, itr2->first))) {
					((itr->second)[itr2->first]).push_back(TraitList[i]);
					LogWrite(SPELL__INFO, 0, "Traits", "Added Trait: %u Tier %i", TraitList[i]->spellID, TraitList[i]->tier);
				}
				else {
					vector<TraitData*> tempVec;
					tempVec.push_back(TraitList[i]);
					(itr->second).insert(make_pair(TraitList[i]->level, tempVec));
					LogWrite(SPELL__INFO, 0, "Traits", "Added Trait: %u Tier %i", TraitList[i]->spellID, TraitList[i]->tier);
				}
			}
			else {
				map <int8, vector<TraitData*> > tempMap;
				vector <TraitData*> tempVec;
				tempVec.push_back(TraitList[i]);
				tempMap.insert(make_pair(TraitList[i]->level, tempVec));
				sortedTraitList->insert(make_pair(TraitList[i]->group, tempMap));
				LogWrite(SPELL__INFO, 0, "Traits", "Added Trait: %u Tier %i", TraitList[i]->spellID, TraitList[i]->tier);
			}
		}
		
		// Sort Class Training
		if (TraitList[i]->classReq == client->GetPlayer()->GetAdventureClass() && TraitList[i]->isTraining) {
			itr2 = classTraining->lower_bound(TraitList[i]->level);
			if (itr2 != classTraining->end() && !(classTraining->key_comp()(TraitList[i]->level, itr2->first))) {
				(itr2->second).push_back(TraitList[i]);
			}
			else {
				vector<TraitData*> tempVec;
				tempVec.push_back(TraitList[i]);
				classTraining->insert(make_pair(TraitList[i]->level, tempVec));
			}
		}
		
		// Sort Racial Abilities
		if (TraitList[i]->raceReq == client->GetPlayer()->GetRace() && !TraitList[i]->isInate && !TraitList[i]->isTraining) {
			itr2 = raceTraits->lower_bound(TraitList[i]->group);
			if (itr2 != raceTraits->end() && !(raceTraits->key_comp()(TraitList[i]->group, itr2->first))) {
				(itr2->second).push_back(TraitList[i]);
			}
			else {
				vector<TraitData*> tempVec;
				tempVec.push_back(TraitList[i]);
				raceTraits->insert(make_pair(TraitList[i]->group, tempVec));
			}
		}
		
		// Sort Innate Racial Abilities
		if (TraitList[i]->raceReq == client->GetPlayer()->GetRace() && TraitList[i]->isInate) {
			itr2 = innateRaceTraits->lower_bound(TraitList[i]->group);
			if (itr2 != innateRaceTraits->end() && !(innateRaceTraits->key_comp()(TraitList[i]->group, itr2->first))) {
				(itr2->second).push_back(TraitList[i]);
			}
			else {
				vector<TraitData*> tempVec;
				tempVec.push_back(TraitList[i]);
				innateRaceTraits->insert(make_pair(TraitList[i]->group, tempVec));
			}
		}
		
		// Sort Focus Effects
		if ((TraitList[i]->classReq == client->GetPlayer()->GetAdventureClass() || TraitList[i]->classReq == 255) && TraitList[i]->isFocusEffect) {
			int8 j = 0;
			itr2 = focusEffects->lower_bound(TraitList[i]->group);
			if (itr2 != focusEffects->end() && !(focusEffects->key_comp()(TraitList[i]->group, itr2->first))) {
				
				(itr2->second).push_back(TraitList[i]);
				//LogWrite(SPELL__INFO, 0, "Traits", "Added Focus Effect: %u Tier: %i", TraitList[i]->spellID, TraitList[i]->tier);
				j++;
			}
			else {
				vector<TraitData*> tempVec;
				tempVec.push_back(TraitList[i]);
				focusEffects->insert(make_pair(TraitList[i]->group, tempVec));
				//LogWrite(SPELL__INFO, 0, "Traits", "Added Focus Effect: %u Tier %i", TraitList[i]->spellID, TraitList[i]->tier);
			}
		}
	}

	MMasterTraitList.releasereadlock(__FUNCTION__, __LINE__);
	
	return true;
}

bool MasterTraitList::IdentifyNextTrait(Client* client, map <int8, vector<TraitData*> >* traitList, vector<TraitData*>* collectTraits, vector<TraitData*>* tieredTraits, std::map<int32, int8>* previousMatchedSpells, bool omitFoundMatches) {
	bool found_spell_match = false;
	bool match = false;
	bool tiered_selection = rule_manager.GetGlobalRule(R_Player, TraitTieringSelection)->GetBool();
	int8 group_to_apply = 255;
	int8 count = 0;
	map <int8, vector<TraitData*> >::iterator itr2;
	vector<TraitData*>::iterator itr3;
	
				for (itr2 = traitList->begin(); itr2 != traitList->end(); itr2++) {
					//LogWrite(SPELL__INFO, 0, "AA", "Character Traits Size...%i ", traits_size);
					for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++) {
						if(tiered_selection) {
							if(found_spell_match && (*itr3)->group == group_to_apply) {
								continue; // skip!
							}
							else if((*itr3)->group != group_to_apply) {
								if(group_to_apply != 255 && !found_spell_match) {
									// found match
									LogWrite(SPELL__INFO, 0, "Traits", "Found match to group id %u", group_to_apply);
									match = true;
									break;
								}
								else {
									LogWrite(SPELL__INFO, 0, "Traits", "Try match to group... spell id %u, group id %u", (*itr3)->spellID, (*itr3)->group);
									found_spell_match = false;
									group_to_apply = (*itr3)->group;
									count = 0;
									if(!omitFoundMatches)
										tieredTraits->clear();
								}
							}
						}
					count++;
					
					std::map<int32,int8>::iterator spell_itr = previousMatchedSpells->find((*itr3)->spellID);
					
					if(spell_itr != previousMatchedSpells->end() && (*itr3)->group > spell_itr->second) {
						continue;
					}
					if(!IsPlayerAllowedTrait(client, (*itr3))) {
						LogWrite(SPELL__INFO, 0, "Traits", "We are not allowed any more spells from this type/group... spell id %u, group id %u", (*itr3)->spellID, (*itr3)->group);
						found_spell_match = true;
					}
					else if (client->GetPlayer()->HasSpell((*itr3)->spellID, (*itr3)->tier)) {
						LogWrite(SPELL__INFO, 0, "Traits", "Found existing spell match to group... spell id %u, group id %u", (*itr3)->spellID, (*itr3)->group);
						if(!omitFoundMatches)
							found_spell_match = true;
						previousMatchedSpells->insert(std::make_pair((*itr3)->spellID,(*itr3)->group));
					}
					else {
						tieredTraits->push_back((*itr3));
						collectTraits->push_back((*itr3));
					}
				}
				
				if(match)
					break;
			}
			
			
		if(!match && group_to_apply != 255 && !found_spell_match) {
			// found match
			match = true;
		}
		else if(!tiered_selection && collectTraits->size() > 0) {
			match = true;
		}
	return match;
}

bool MasterTraitList::ChooseNextTrait(Client* client) {
	map <int8, map <int8, vector<TraitData*> > >* SortedTraitList = new map <int8, map <int8, vector<TraitData*> > >;
	map <int8, map <int8, vector<TraitData*> > >::iterator itr;
	bool tiered_selection = rule_manager.GetGlobalRule(R_Player, TraitTieringSelection)->GetBool();
	vector<TraitData*>::iterator itr3;

	map <int8, vector<TraitData*> >* ClassTraining = new map <int8, vector<TraitData*> >;
	map <int8, vector<TraitData*> >* RaceTraits = new map <int8, vector<TraitData*> >;
	map <int8, vector<TraitData*> >* InnateRaceTraits = new map <int8, vector<TraitData*> >;
	map <int8, vector<TraitData*> >* FocusEffects = new map <int8, vector<TraitData*> >;
		vector<TraitData*>* collectTraits = new vector<TraitData*>;
		vector<TraitData*>* tieredTraits = new vector<TraitData*>;
		std::map<int32, int8>* previousMatchedSpells = new std::map<int32, int8>;
	bool match = false;
	if(GenerateTraitLists(client, SortedTraitList, ClassTraining, RaceTraits, InnateRaceTraits, FocusEffects, client->GetPlayer()->GetLevel())) {
		
		vector<TraitData*>* endTraits;
		if(!match || !tiered_selection) {
			match = IdentifyNextTrait(client, ClassTraining, collectTraits, tieredTraits, previousMatchedSpells);
		}
		if(!match || !tiered_selection) {
			match = IdentifyNextTrait(client, RaceTraits, collectTraits, tieredTraits, previousMatchedSpells, true);
			
			bool overrideMatch = IdentifyNextTrait(client, InnateRaceTraits, collectTraits, tieredTraits, previousMatchedSpells, true);
			
			if(!match && overrideMatch)
				match = true;
		}
		if(!match || !tiered_selection) {
			match = IdentifyNextTrait(client, FocusEffects, collectTraits, tieredTraits, previousMatchedSpells);
		}
		
		if(!tiered_selection && collectTraits->size() > 0) {
			endTraits = collectTraits;
		}
		else if (match) {
			endTraits = tieredTraits;
		}
		if(match) {
			PacketStruct* packet = configReader.getStruct("WS_QuestRewardPackMsg", client->GetVersion());
			// 0=enemy mastery, 1=specialized training,2=character trait, 3=racial tradition
			int8 packet_type = 0;
			int8 item_count = 0;
			packet->setSubstructArrayLengthByName("reward_data", "num_select_rewards", endTraits->size());
			for (itr3 = endTraits->begin(); itr3 != endTraits->end(); itr3++) {
					
					if((*itr3)->item_id) {
						//LogWrite(SPELL__INFO, 0, "Traits", "Item %u to be sent", (*itr3)->item_id);
						Item* item = master_item_list.GetItem((*itr3)->item_id);
						if(item) {
							//LogWrite(SPELL__INFO, 0, "Traits", "Item found %s to be sent", item->name.c_str());
							packet->setArrayDataByName("select_reward_id", (*itr3)->item_id, item_count);
							packet->setItemArrayDataByName("select_item", item, client->GetPlayer(), item_count, 0, client->GetClientItemPacketOffset());
							item_count++;
						}
					}
					
								
					// Sort Character Traits
					if ((*itr3)->classReq == 255 && (*itr3)->raceReq == 255 && (*itr3)->isTrait) {
						packet_type = 2;
					}
					// Sort Class Training
					else if ((*itr3)->classReq == client->GetPlayer()->GetAdventureClass() && (*itr3)->isTraining) {
						packet_type = 1;
					}
					// Sort Racial Abilities
					else if ((*itr3)->raceReq == client->GetPlayer()->GetRace() && !(*itr3)->isInate && !(*itr3)->isTraining) {
						packet_type = 3;
					}
					// Sort Innate Racial Abilities
					else if ((*itr3)->raceReq == client->GetPlayer()->GetRace() && (*itr3)->isInate) {
						packet_type = 3;
					}

					//LogWrite(SPELL__INFO, 0, "Traits", "Sending trait %u", (*itr3)->spellID);
			}
			packet->setSubstructDataByName("reward_data", "unknown1", packet_type);
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}
	
	safe_delete(SortedTraitList);
	safe_delete(ClassTraining);
	safe_delete(RaceTraits);
	safe_delete(InnateRaceTraits);
	safe_delete(FocusEffects);
	
	safe_delete(collectTraits);
	safe_delete(tieredTraits);
	safe_delete(previousMatchedSpells);
	return match;
}

int16 MasterTraitList::GetSpellCount(Client* client, map <int8, vector<TraitData*> >* traits, bool onlyCharTraits) {
	if(!traits)
		return 0;
	
	int16 count = 0;
	map <int8, vector<TraitData*> >::iterator itr2;
	vector<TraitData*>::iterator itr3;
	for (itr2 = traits->begin(); itr2 != traits->end(); itr2++) {
		for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++) {
			if (client->GetPlayer()->HasSpell((*itr3)->spellID, (*itr3)->tier))  {
				if(!onlyCharTraits || (onlyCharTraits && (*itr3)->classReq == 255 && (*itr3)->raceReq == 255 && (*itr3)->isTrait)) {
					count++;
				}
			}
		}
	}
	
	return count;
}

vector<int16> PersonalTraitLevelLimits = boost::assign::list_of(0)(8)(14)(22)(28)(36)(42)(46)(48);
vector<int16> TrainingTraitLevelLimits = boost::assign::list_of(0)(10)(20)(30)(40)(50);
vector<int16> RacialTraitLevelLimits = boost::assign::list_of(0)(18)(26)(34)(44);
vector<int16> CharacterTraitLevelLimits = boost::assign::list_of(0)(12)(16)(24)(32)(38);

/***
Every other level, beginning at Level 8,
you gain an additional advantage — a
Personal Trait, an Enemy Tactic, a Racial
Tradition or a Training ability. Each time
you reach an even-numbered level, you
can select another advantage from the
appropriate list. You don’t have to select
in order — you may take any of the avail-
able choices.
Level Advantage
8 Personal Trait (1st)
10 Training (1st)
12 Enemy Tactic (1st)
14 Personal Trait (2nd)
16 Enemy Tactic (2nd)
18 Racial Tradition (1st)
20 Training (2nd)
22 Personal Trait (3rd)
24 Enemy Tactic (3rd)
26 Racial Tradition (2nd)
28 Personal Trait (4th)
30 Training (3rd)
32 Enemy Tactic (4th)
34 Racial Tradition (3rd)
36 Personal Trait (5th)
38 Enemy Tactic (5th)
40 Training (4th)
42 Personal Trait (6th)
44 Racial Tradition (4th)
46 Personal Trait (7th)
48 Personal Trait (8th)
50 Training (5th)
***/

bool MasterTraitList::IsPlayerAllowedTrait(Client* client, TraitData* trait) {
	std::unique_lock(client->GetPlayer()->trait_mutex);
	map <int8, map <int8, vector<TraitData*> > >* SortedTraitList = client->GetPlayer()->SortedTraitList;
	map <int8, map <int8, vector<TraitData*> > >::iterator itr;
	map <int8, vector<TraitData*> >::iterator itr2;
	vector<TraitData*>::iterator itr3;
	bool use_classic_table = rule_manager.GetGlobalRule(R_Player, ClassicTraitLevelTable)->GetBool();

	map <int8, vector<TraitData*> >* ClassTraining = client->GetPlayer()->ClassTraining;
	map <int8, vector<TraitData*> >* RaceTraits = client->GetPlayer()->RaceTraits;
	map <int8, vector<TraitData*> >* InnateRaceTraits = client->GetPlayer()->InnateRaceTraits;
	map <int8, vector<TraitData*> >* FocusEffects = client->GetPlayer()->FocusEffects;
	if(client->GetPlayer()->need_trait_update) {
		SortedTraitList->clear();
		ClassTraining->clear();
		RaceTraits->clear();
		InnateRaceTraits->clear();
		FocusEffects->clear();
	}
	bool allowed_trait = false;
	if(!client->GetPlayer()->need_trait_update || GenerateTraitLists(client, SortedTraitList, ClassTraining, RaceTraits, InnateRaceTraits, FocusEffects, 0)) {
		client->GetPlayer()->need_trait_update = false;
		if(trait->isFocusEffect) {
			
			int32 trait_level = rule_manager.GetGlobalRule(R_Player, TraitFocusSelectLevel)->GetInt32();
			int16 num_available_selections = 0;
			if(trait_level > 0) {
				num_available_selections = client->GetPlayer()->GetLevel() / trait_level;
			}
			int16 total_used = GetSpellCount(client, FocusEffects);
			
			int16 classic_avail = 0;
			
			if(use_classic_table && PersonalTraitLevelLimits.size() > total_used+1) {
				int16 classic_level_req = PersonalTraitLevelLimits.at(total_used+1);
				if(client->GetPlayer()->GetLevel() >= classic_level_req)
					classic_avail = num_available_selections = total_used+1;
				else
					num_available_selections = 0;
			}
			else if(use_classic_table)
				num_available_selections = 0;
			
			LogWrite(SPELL__INFO, 9, "Traits", "%s FocusEffects used %u, available %u, classic available %u", client->GetPlayer()->GetName(), total_used, num_available_selections, classic_avail);
			if(total_used < num_available_selections) {
				allowed_trait = true;
			}
		}
		else if(trait->isTraining) {
			int32 trait_level = rule_manager.GetGlobalRule(R_Player, TraitTrainingSelectLevel)->GetInt32();
			int16 num_available_selections = 0;
			if(trait_level > 0) {
				num_available_selections = client->GetPlayer()->GetLevel() / trait_level;
			}
			int16 total_used = GetSpellCount(client, ClassTraining);
			
			int16 classic_avail = 0;
			
			if(use_classic_table && TrainingTraitLevelLimits.size() > total_used+1) {
				int16 classic_level_req = TrainingTraitLevelLimits.at(total_used+1);
				if(client->GetPlayer()->GetLevel() >= classic_level_req)
					classic_avail = num_available_selections = total_used+1;
				else
					num_available_selections = 0;
			}
			else if(use_classic_table)
				num_available_selections = 0;
			
			LogWrite(SPELL__INFO, 9, "Traits", "%s ClassTraining used %u, available %u,  classic available %u", client->GetPlayer()->GetName(), total_used, num_available_selections, classic_avail);
			if(total_used < num_available_selections) {
				allowed_trait = true;
			}
		}
		else {
				if(trait->raceReq == client->GetPlayer()->GetRace()) {
					int32 trait_level = rule_manager.GetGlobalRule(R_Player, TraitRaceSelectLevel)->GetInt32();
					int16 num_available_selections = 0;
					if(trait_level > 0) {
						num_available_selections = client->GetPlayer()->GetLevel() / trait_level;
					}
					int16 total_used = GetSpellCount(client, RaceTraits);
					int16 total_used2 = GetSpellCount(client, InnateRaceTraits);
					
					int16 classic_avail = 0;
					
					if(use_classic_table && RacialTraitLevelLimits.size() > total_used+total_used2+1) {
						int16 classic_level_req = RacialTraitLevelLimits.at(total_used+total_used2+1);
						if(client->GetPlayer()->GetLevel() >= classic_level_req)
							classic_avail = num_available_selections = total_used+total_used2+1;
						else
							num_available_selections = 0;
					}
					else if(use_classic_table)
						num_available_selections = 0;
			
					LogWrite(SPELL__INFO, 9, "Traits", "%s RaceTraits used %u, available %u, classic available %u", client->GetPlayer()->GetName(), total_used+total_used2, num_available_selections, classic_avail);
					if(total_used+total_used2 < num_available_selections) {
						allowed_trait = true;
					}
				}
				else { // character trait?
					int16 num_available_selections = 0;
					int32 trait_level = rule_manager.GetGlobalRule(R_Player, TraitCharacterSelectLevel)->GetInt32();
					if(trait_level > 0) {
						num_available_selections = client->GetPlayer()->GetLevel() / trait_level;
					}
					int16 total_used = 0;
					for (itr = SortedTraitList->begin(); itr != SortedTraitList->end(); itr++) {
						total_used += GetSpellCount(client, &itr->second, true);
					}
					
					int16 classic_avail = 0;
					
					if(use_classic_table && CharacterTraitLevelLimits.size() > total_used+1) {
						int16 classic_level_req = CharacterTraitLevelLimits.at(total_used+1);
						if(client->GetPlayer()->GetLevel() >= classic_level_req)
							classic_avail = num_available_selections = total_used+1;
						else
							num_available_selections = 0;
					}
					else if(use_classic_table)
						num_available_selections = 0;
					
					LogWrite(SPELL__INFO, 9, "Traits", "%s CharacterTraits used %u, available %u, classic available %u", client->GetPlayer()->GetName(), total_used, num_available_selections, classic_avail);
					if(total_used < num_available_selections) {
						allowed_trait = true;
					}
				}
			}
		}
	
	return allowed_trait;
}

EQ2Packet* MasterTraitList::GetTraitListPacket (Client* client)
{
	std::unique_lock(client->GetPlayer()->trait_mutex);
	
	if (!client) {
		LogWrite(SPELL__ERROR, 0, "Traits", "GetTraitListPacket called with an invalid client");
		return 0;
	}
	// Sort the Data
	if (Size() == 0)
		return NULL;

	map <int8, map <int8, vector<TraitData*> > >* SortedTraitList = client->GetPlayer()->SortedTraitList;
	map <int8, map <int8, vector<TraitData*> > >::iterator itr;
	map <int8, vector<TraitData*> >::iterator itr2;
	vector<TraitData*>::iterator itr3;

	map <int8, vector<TraitData*> >* ClassTraining = client->GetPlayer()->ClassTraining;
	map <int8, vector<TraitData*> >* RaceTraits = client->GetPlayer()->RaceTraits;
	map <int8, vector<TraitData*> >* InnateRaceTraits = client->GetPlayer()->InnateRaceTraits;
	map <int8, vector<TraitData*> >* FocusEffects = client->GetPlayer()->FocusEffects;
	
	if(client->GetPlayer()->need_trait_update) {
		SortedTraitList->clear();
		ClassTraining->clear();
		RaceTraits->clear();
		InnateRaceTraits->clear();
		FocusEffects->clear();
	}
	
	if(client->GetPlayer()->need_trait_update && !GenerateTraitLists(client, SortedTraitList, ClassTraining, RaceTraits, InnateRaceTraits, FocusEffects)) {
		return NULL;
	}
	
	client->GetPlayer()->need_trait_update = false;
	
	int16 version = 1;
	int8 count = 0;
	int8 index = 0;
	int8 num_traits = 0;
	int8 traits_size = 0;
	int8 num_focuseffects = 0;
	char sTrait [20];
	char temp [20];
	
	version = client->GetVersion();

	// Jabantiz: Get the value for num_traits in the struct (num_traits refers to the number of rows not the total number of traits)
	for (itr = SortedTraitList->begin(); itr != SortedTraitList->end(); itr++) {
		num_traits += (itr->second).size();
	}

	PacketStruct* packet = configReader.getStruct("WS_TraitsList", version);

	if (packet == NULL) {
		return NULL;
	}

	packet->setArrayLengthByName("num_traits", num_traits);

	for (itr = SortedTraitList->begin(); itr != SortedTraitList->end(); itr++) {

		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++, index++) {
			traits_size += (itr2->second).size();
			count = 0;
			Spell* tmp_spell = 0;
			packet->setArrayDataByName("trait_level", (*itr2).first, index);
			packet->setArrayDataByName("trait_line", 255, index);
			//LogWrite(SPELL__INFO, 0, "AA", "Character Traits Size...%i ", traits_size);
			for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, count++) {
				// Jabantiz: cant have more then 5 traits per line
				tmp_spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
				if(!tmp_spell) {
					LogWrite(SPELL__ERROR, 0, "Traits", "Could not find SpellID %u and Tier %i in Master Spell List (line: %i)", (*itr3)->spellID, (*itr3)->tier, __LINE__);
					continue;
				}
				if (count > 4)
					break;

				strcpy(sTrait, "trait");
				itoa(count, temp, 10);
				strcat(sTrait, temp);

				strcpy(temp, sTrait);
				strcat(sTrait, "_icon");
				if (tmp_spell)
					packet->setArrayDataByName(sTrait, tmp_spell->GetSpellIcon(), index);
				else
					LogWrite(SPELL__ERROR, 0, "Traits", "Could not find SpellID %u and Tier %i in Master Spell List (line: %i)", (*itr3)->spellID, (*itr3)->tier, __LINE__);

				strcpy(sTrait, temp);
				strcat(sTrait, "_icon2");
				packet->setArrayDataByName(sTrait, 65535, index);

				strcpy(sTrait, temp);
				strcat(sTrait, "_id");
				packet->setArrayDataByName(sTrait, (*itr3)->spellID, index);

				strcpy(sTrait, temp);
				strcat(sTrait, "_name");
				if (tmp_spell)
					packet->setArrayDataByName(sTrait, tmp_spell->GetName(), index);
				else
					LogWrite(SPELL__ERROR, 0, "Traits", "Could not find SpellID %u and Tier %i in Master Spell List (line: %i)", (*itr3)->spellID, (*itr3)->tier, __LINE__);

				strcpy(sTrait, temp);
				strcat(sTrait, "_unknown2");
				packet->setArrayDataByName(sTrait, 1, index);

				strcpy(sTrait, temp);
				strcat(sTrait, "_unknown");
				packet->setArrayDataByName(sTrait, 1, index);

				if (client->GetPlayer()->HasSpell((*itr3)->spellID, (*itr3)->tier)) 
					packet->setArrayDataByName("trait_line", count, index);
			}
			// Jabantiz: If less then 5 fill the rest of the line with FF FF FF FF FF FF FF FF FF FF FF FF
			while (count < 5) {
				strcpy(sTrait, "trait");
				itoa(count, temp, 10);
				strcat(sTrait, temp);
				strcpy(temp, sTrait);
				strcat(sTrait, "_icon");
				packet->setArrayDataByName(sTrait, 65535, index); // FF FF
				strcpy(sTrait, temp);
				strcat(sTrait, "_icon2");
				packet->setArrayDataByName(sTrait, 65535, index); // FF FF
				strcpy(sTrait, temp);
				strcat(sTrait, "_id");
				packet->setArrayDataByName(sTrait, 0xFFFFFFFF, index);
				strcpy(sTrait, temp);
				strcat(sTrait, "_unknown");
				packet->setArrayDataByName(sTrait, 0xFFFFFFFF, index);
				strcpy(sTrait, temp);
				strcat(sTrait, "_name");
				packet->setArrayDataByName(sTrait, "", index);
				count++;
			}
		}
	}
	
	// Class Training portion of the packet
	packet->setArrayLengthByName("num_trainings", ClassTraining->size());
	index = 0;
	for (itr2 = ClassTraining->begin(); itr2 != ClassTraining->end(); itr2++, index++) {
		count = 0;
		Spell* tmp_spell = 0;
		packet->setArrayDataByName("training_level", itr2->first, index);
		packet->setArrayDataByName("training_line", 255, index);

		for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, count++) {
			// Jabantiz: cant have more then 5 traits per line
			tmp_spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
			if(!tmp_spell) {
				LogWrite(SPELL__ERROR, 0, "Traits", "Could not find SpellID %u and Tier %i in Master Spell List (line: %i)", (*itr3)->spellID, (*itr3)->tier, __LINE__);
				continue;
			}
			if (count > 4)
				break;
			if (client->GetPlayer()->HasSpell((*itr3)->spellID, (*itr3)->tier)) {
				packet->setArrayDataByName("training_line", count, index);
			}
			strcpy(sTrait, "training");
			itoa(count, temp, 10);
			strcat(sTrait, temp);

			strcpy(temp, sTrait);
			strcat(sTrait, "_icon");

			if (tmp_spell)
				packet->setArrayDataByName(sTrait, tmp_spell->GetSpellIcon(), index);
			else
				LogWrite(SPELL__ERROR, 0, "Training", "Could not find SpellID %u and Tier %i in Master Spell List (line: %i)", (*itr3)->spellID, (*itr3)->tier, __LINE__);

			strcpy(sTrait, temp);
			strcat(sTrait, "_icon2");
			if (tmp_spell)
				packet->setArrayDataByName(sTrait, tmp_spell->GetSpellIconBackdrop(), index);
			else
				LogWrite(SPELL__ERROR, 0, "Training", "Could not find SpellID %u and Tier %i in Master Spell List (line: %i)", (*itr3)->spellID, (*itr3)->tier, __LINE__);

			strcpy(sTrait, temp);
			strcat(sTrait, "_id");
			packet->setArrayDataByName(sTrait, (*itr3)->spellID, index);

			strcpy(sTrait, temp);
			strcat(sTrait, "_unknown");
			packet->setArrayDataByName(sTrait,0xFFFFFFFF , index);

			strcpy(sTrait, temp);
			strcat(sTrait, "_unknown2");
			packet->setArrayDataByName(sTrait, 1, index);

			strcpy(sTrait, temp);
			strcat(sTrait, "_name");
			if (tmp_spell)
				packet->setArrayDataByName(sTrait, tmp_spell->GetName(), index);
			else
				LogWrite(SPELL__ERROR, 0, "Training", "Could not find SpellID %u and Tier %i in Master Spell List (line: %i)", (*itr3)->spellID, (*itr3)->tier, __LINE__);

			if (client->GetPlayer()->HasSpell((*itr3)->spellID, (*itr3)->tier))
				packet->setArrayDataByName("training_line", count, index);
		}
		// Jabantiz: If less then 5 fill the rest of the line with FF FF FF FF FF FF FF FF FF FF FF FF
		while (count < 5) {
			strcpy(sTrait, "training");
			itoa(count, temp, 10);
			strcat(sTrait, temp);
			strcpy(temp, sTrait);
			strcat(sTrait, "_icon");
			packet->setArrayDataByName(sTrait, 65535, index); // FF FF
			strcpy(sTrait, temp);
			strcat(sTrait, "_icon2");
			packet->setArrayDataByName(sTrait, 65535, index); // FF FF
			strcpy(sTrait, temp);
			strcat(sTrait, "_id");
			packet->setArrayDataByName(sTrait, 0xFFFFFFFF, index);
			strcpy(sTrait, temp);
			strcat(sTrait, "_unknown");
			packet->setArrayDataByName(sTrait, 0xFFFFFFFF, index);
			strcpy(sTrait, temp);
			strcat(sTrait, "_name");
			packet->setArrayDataByName(sTrait, "", index);
			count++;
		}
	}
	// Racial Traits
	packet->setArrayLengthByName("num_sections", RaceTraits->size());
	index = 0;
	string tempStr;
	int8 num_selections = 0;
	for (itr2 = RaceTraits->begin(); itr2 != RaceTraits->end(); itr2++, index++) {
		count = 0;
		Spell* tmp_spell = 0;
		switch (itr2->first)
		{
		case TRAITS_ATTRIBUTES:
			tempStr = "Attributes";
			break;
		case TRAITS_COMBAT:
			tempStr = "Combat";
			break;
		case TRAITS_NONCOMBAT:
			tempStr = "Noncombat";
			break;
		case TRAITS_POOLS:
			tempStr = "Pools";
			break;
		case TRAITS_RESIST:
			tempStr = "Resist";
			break;
		case TRAITS_TRADESKILL:
			tempStr = "Tradeskill";
			break;
		default:
			tempStr = "Unknown";
			break;
		}
		packet->setArrayDataByName("section_name", tempStr.c_str(), index);
		packet->setSubArrayLengthByName("num_traditions", (itr2->second).size(), index);

		for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, count++) {
			if (client->GetPlayer()->HasSpell((*itr3)->spellID, (*itr3)->tier))	{
				num_selections++;
				packet->setSubArrayDataByName("tradition_selected", 1, index, count);
			}
			else {
				packet->setSubArrayDataByName("tradition_selected", 0, index, count);
			}
			tmp_spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
			if (tmp_spell){
				packet->setSubArrayDataByName("tradition_icon", tmp_spell->GetSpellIcon(), index, count);
				packet->setSubArrayDataByName("tradition_icon2", tmp_spell->GetSpellIconBackdrop(), index, count);
				packet->setSubArrayDataByName("tradition_id", (*itr3)->spellID, index, count);
				packet->setSubArrayDataByName("tradition_name", tmp_spell->GetName(), index, count);
				packet->setSubArrayDataByName("tradition_unknown_58617_MJ1", 1, index, count);
			}
			else
				LogWrite(SPELL__ERROR, 0, "Traits", "Could not find SpellID %u and Tier %i in Master Spell List (line: %i)", (*itr3)->spellID, (*itr3)->tier, __LINE__);
		}
	}
	int8 num_available_selections = client->GetPlayer()->GetLevel() / 10;
	if (num_selections < num_available_selections)
		packet->setDataByName("allow_select", num_available_selections - num_selections);
	else
		packet->setDataByName("allow_select", 0);

	// Innate Racial Traits
	index = 0;

	// total number of Innate traits
	num_traits = 0;
	for (itr2 = InnateRaceTraits->begin(); itr2 != InnateRaceTraits->end(); itr2++) {
		num_traits += (itr2->second).size();
	}
	packet->setArrayLengthByName("num_abilities", num_traits);
	for (itr2 = InnateRaceTraits->begin(); itr2 != InnateRaceTraits->end(); itr2++) {
		for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, index++) {
			Spell* innate_spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
			if (innate_spell) {
				packet->setArrayDataByName("ability_icon", innate_spell->GetSpellIcon(), index);
				packet->setArrayDataByName("ability_icon2", innate_spell->GetSpellIconBackdrop(), index);
				packet->setArrayDataByName("ability_id", (*itr3)->spellID, index);
				packet->setArrayDataByName("ability_name", innate_spell->GetName(), index);
			}
			else
				LogWrite(SPELL__ERROR, 0, "Traits", "Could not find SpellID %u and Tier %i in Master Spell List (line: %i)", (*itr3)->spellID, (*itr3)->tier, __LINE__);
		}
	}

	if (client->GetVersion() >= 1188) {
		// total number of Focus Effects
		num_selections = 0;
		num_focuseffects = 0;
		index = 0;
		for (itr2 = FocusEffects->begin(); itr2 != FocusEffects->end(); itr2++) {
			num_focuseffects += (itr2->second).size();
		}
		packet->setArrayLengthByName("num_focuseffects", num_focuseffects);
		for (itr2 = FocusEffects->begin(); itr2 != FocusEffects->end(); itr2++) {
			for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, index++) {
				Spell* spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
				if (client->GetPlayer()->HasSpell((*itr3)->spellID, (*itr3)->tier)) {
					num_selections++;
					packet->setArrayDataByName("focus_selected", 1, index);
				}
				else {
					packet->setArrayDataByName("focus_selected", 0, index);
				}
				if (spell) {
					packet->setArrayDataByName("focus_unknown2", 1, index);
					packet->setArrayDataByName("focus_icon", spell->GetSpellIcon(), index);
					packet->setArrayDataByName("focus_icon2", spell->GetSpellIconBackdrop(), index);
					packet->setArrayDataByName("focus_spell_crc", (*itr3)->spellID, index);
					packet->setArrayDataByName("focus_name", spell->GetName(), index);
					packet->setArrayDataByName("focus_unknown_58617_MJ1", 1, index);
				}
				else
					LogWrite(SPELL__ERROR, 0, "Traits", "Could not find SpellID %u and Tier %i in Master Spell List (line: %i)", (*itr3)->spellID, (*itr3)->tier, __LINE__);
			}
		}
		num_available_selections = client->GetPlayer()->GetLevel() / 9;
		if (num_selections < num_available_selections)
			packet->setDataByName("focus_allow_select", num_available_selections - num_selections);
		else
			packet->setDataByName("focus_allow_select", 0);
	}
	LogWrite(SPELL__PACKET, 0, "Traits", "Dump/Print Packet in func: %s, line: %i", __FUNCTION__, __LINE__);
#if EQDEBUG >= 9
	packet->PrintPacket();
#endif
	EQ2Packet* data = packet->serialize();
	EQ2Packet* outapp = new EQ2Packet(OP_ClientCmdMsg, data->pBuffer, data->size);
	//DumpPacket(outapp);
	safe_delete(packet);
	safe_delete(data);
	
	return outapp;
}

// Jabantiz: Probably a better way to do this but can't think of it right now
TraitData* MasterTraitList::GetTrait(int32 spellID) {
	vector<TraitData*>::iterator itr;
	TraitData* data = NULL;

	MMasterTraitList.readlock(__FUNCTION__, __LINE__);
	for (itr = TraitList.begin(); itr != TraitList.end(); itr++) {
		if ((*itr)->spellID == spellID) {
			data = (*itr);
			break;
		}
	}
	MMasterTraitList.releasereadlock(__FUNCTION__, __LINE__);

	return data;
}

TraitData* MasterTraitList::GetTraitByItemID(int32 itemID) {
	vector<TraitData*>::iterator itr;
	TraitData* data = NULL;

	MMasterTraitList.readlock(__FUNCTION__, __LINE__);
	for (itr = TraitList.begin(); itr != TraitList.end(); itr++) {
		if ((*itr)->item_id == itemID) {
			data = (*itr);
			break;
		}
	}
	MMasterTraitList.releasereadlock(__FUNCTION__, __LINE__);

	return data;
}

void MasterTraitList::DestroyTraits(){
	MMasterTraitList.writelock(__FUNCTION__, __LINE__);
	vector<TraitData*>::iterator itr;
	for (itr = TraitList.begin(); itr != TraitList.end(); itr++)
		safe_delete((*itr));
	TraitList.clear();
	MMasterTraitList.releasewritelock(__FUNCTION__, __LINE__);
}
