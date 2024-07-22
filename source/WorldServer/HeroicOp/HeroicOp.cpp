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

#include "HeroicOp.h"
#include "../../common/Log.h"
#include "../Rules/Rules.h"

extern MasterHeroicOPList master_ho_list;
extern RuleManager rule_manager;

HeroicOP::HeroicOP() {
	m_complete = 0;
	m_currentStage = 0;
	m_wheel = 0;
	m_target = 0;
	m_startTime = 0;
	m_totalTime = 0;
	m_shifted = false;
	for (int8 i = 0; i < 6; i++)
		countered[i] = 0;
}

HeroicOP::~HeroicOP() {
	starters.clear();
}

void HeroicOP::SetWheel(HeroicOPWheel* val) {
	if (!m_wheel)
		m_wheel = val;
	else
		LogWrite(SPELL__ERROR, 0, "HO", "Attempted to set the wheel on a heroic op with a wheel already set");
}

void HeroicOP::SetTarget(int32 val) {
	m_target = val;
}

bool HeroicOP::UpdateHeroicOP(int16 icon) {
	bool ret = false;
	vector<HeroicOPStarter*>::iterator itr;
	vector<HeroicOPStarter*> temp;
	HeroicOPStarter* starter = 0;

	LogWrite(SPELL__DEBUG, 0, "HO", "Current Stage %u, wheel exists: %u, looking for icon %u", m_currentStage, m_wheel ? 1 : 0, icon);
	// If no wheel is set we are dealing with a starter chain still.
	if (!m_wheel) {
		// Loop through the starter chains
		for (itr = starters.begin(); itr != starters.end(); itr++) {
			starter = *itr;
			// See if the icon matches the ability at our current stage, if not add it to a list to be removed
			if (starter->abilities[m_currentStage] == icon)
				ret = true;
			else
				temp.push_back(*itr);
		}
		
		if (ret) {
			// ret = true so we had a match, first thing to do is remove those that didn't match
			vector<HeroicOPStarter*>::iterator remove_itr;
			for (remove_itr = temp.begin(); remove_itr != temp.end(); remove_itr++)
			{
				std::vector<HeroicOPStarter*>::iterator it = std::find(starters.begin(), starters.end(), *remove_itr);
				starters.erase(it);
			}

			// now advance the current stage
			m_currentStage++;
			
			// Temp pointer to hold the completed chain, if any
			HeroicOPStarter* complete_starter = 0;

			// now loop through those that are left and check the next stage abilities for a 0xFFFF
			for (itr = starters.begin(); itr != starters.end(); itr++) {
				starter = *itr;
				// Found one that is 0xFFFF, means the starter chain is done, get a wheel and reset the stage to 0
				if ((starter->abilities[m_currentStage] == 0xFFFF)) {
					LogWrite(SPELL__DEBUG, 0, "HO", "Current Stage %u, starter reset (new stage 0)", m_currentStage);
					// reset the stage
					ResetStage();
					// geth the wheel
					m_wheel = master_ho_list.GetWheel(starter);
					// store the starter chain that is completed
					complete_starter = starter;
					// set the start time to now
					SetStartTime(Timer::GetCurrentTime2());
					// set the total time to complete the real to was the admin set in rules (default 10.0f)
					SetTotalTime(rule_manager.GetGlobalRule(R_Zone, HOTime)->GetFloat());
					// We set a wheel so we are done, kill the loop
					break;
				}
			}

			// Check to see if the completed start chain pointer was set
			if (complete_starter) {
				LogWrite(SPELL__DEBUG, 0, "HO", "Current Stage %u, complete_starter set", m_currentStage);
				// clear the starter list
				starters.clear();
				// add the completed starter back in, we do this in case we need this starter again we can just do starters.at(0), for example shifting the wheel
				starters.push_back(complete_starter);
			}
		}
	}
	else {
		LogWrite(SPELL__DEBUG, 0, "HO", "Current Stage %u, wheel order: %u", m_currentStage, m_wheel->order);
		// Wheel was set so we need to check the order it needs to be completed in.
		if (m_wheel->order == 0) {
			// No order

			// Flag used to see if we can shift the wheel
			bool can_shift = true;
			// Check the icons and flag the ability as countered if there is a match
			for (int8 i = 0; i < 6; i++) {
				if (countered[i] == 1) {
					// progress made on this wheel so we can't shift it
					can_shift = false;
				}
				if (m_wheel->abilities[i] == icon) {
					countered[i] = 1;
					ret = true;
				}
			}

			if (ret) {
				// As we found a match lets loop through to see if we completed the ho
				bool finished = true;
				for (int8 i = 0; i < 6; i++) {
					// if the ability is not 0xFFFF and countered is 0 then we still have more to go
					if (m_wheel->abilities[i] != 0xFFFF && countered[i] == 0) {
						finished = false;
						break;
					}
				}

				// is we finished the ho set the complete flag
				if (finished)
					SetComplete(2);
			}

			if (!ret && can_shift && m_wheel->shift_icon == icon) {
				// can shift, icon matched shift icon, and no progress made
				ret = ShiftWheel();
			}
		}
		else {
			// In order

			// Check to see if we can shift the wheel
			if (countered[0] == 0 && icon == m_wheel->shift_icon) {
				// Can only shift the icon if nothing has completed yet (countered[0] = 0)
				ret = ShiftWheel();
			}
			// Check the current stage and compare it to the icon
			else if (m_wheel->abilities[m_currentStage] == icon) {
				// Is a match so flag this stage as done
				countered[m_currentStage] = 1;
				// Advance the stage
				m_currentStage++;
				// Set the return value to true
				ret = true;
				// Check the next stage, if it is over 6 or equal to 0xFFFF flag the HO as complete
				if (m_currentStage > 6 || m_wheel->abilities[m_currentStage] == 0xFFFF)
					SetComplete(2);
			}
		}
	}

	return ret;
}

void HeroicOP::AddStarterChain(HeroicOPStarter* starter) {
	starters.push_back(starter);
}

bool HeroicOP::ShiftWheel() {
	// Can only shift once so if we already have return out
	if (HasShifted())
		return false;

	// Clear the wheel
	m_wheel = 0;

	// Get a new Wheel
	SetWheel(master_ho_list.GetWheel(starters.at(0)));

	// Set the ho as shifted
	m_shifted = true;

	return true;
}

MasterHeroicOPList::MasterHeroicOPList() {
}

MasterHeroicOPList::~MasterHeroicOPList() {
	map<int8, map<HeroicOPStarter*, vector<HeroicOPWheel*> > >::iterator itr;
	map<HeroicOPStarter*, vector<HeroicOPWheel*> >::iterator itr2;
	vector<HeroicOPWheel*>::iterator itr3;
	vector<HeroicOPStarter*> temp;
	vector<HeroicOPStarter*>::iterator itr4;

	// loop through the m_hoList to delete the pointers
	for (itr = m_hoList.begin(); itr != m_hoList.end(); itr++) {
		// loop through the second map of the m_hoList
		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++) {
			// loop through the vector of the second map and delete the pointers
			for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++)
				safe_delete(*itr3);

			// clear the vector
			itr2->second.clear();
			// put the starter in a temp list to delete later
			temp.push_back(itr2->first);
		}
		// clear the seond map
		itr->second.clear();
	}
	// clear the m_hoList
	m_hoList.clear();

	// Delete the starters
	for (itr4 = temp.begin(); itr4 != temp.end(); itr4++)
		safe_delete(*itr4);

	// clear the temp vector
	temp.clear();
}

void MasterHeroicOPList::AddStarter(int8 start_class, HeroicOPStarter* starter) {
	if (m_hoList.count(start_class) == 0 || m_hoList[start_class].count(starter) == 0) {
		m_hoList[start_class][starter]; // This adds the starter with out giving it a vector of wheels yet.
	}
}

void MasterHeroicOPList::AddWheel(int32 starter_id, HeroicOPWheel* wheel) {
	map<int8, map<HeroicOPStarter*, vector<HeroicOPWheel*> > >::iterator itr;
	map<HeroicOPStarter*, vector<HeroicOPWheel*> >::iterator itr2;
	bool found = false;

	// Loop through the list and add the wheel to the correct starter
	for (itr = m_hoList.begin(); itr != m_hoList.end(); itr++) {
		for (itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++) {
			if (itr2->first->id == starter_id) {
				// Found a match, add the wheel, set the flag to break the first loop, and break this loop
				itr2->second.push_back(wheel);
				found = true;
				break;
			}
		}
		// If we found a match break the first loop
		if (found)
			break;
	}

	// No match found give an error.
	if (!found)
		LogWrite(SPELL__DEBUG, 0, "HO", "Attempted to add a wheel to a starter (%u) that doesn't exsist", starter_id);
}

HeroicOP* MasterHeroicOPList::GetHeroicOP(int8 class_id) {
	if (m_hoList.count(class_id) == 0) {
		LogWrite(SPELL__ERROR, 0, "HO", "No HO's found for the given class (%i)", class_id);
		return 0;
	}

	map<HeroicOPStarter*, vector<HeroicOPWheel*> >::iterator itr;
	HeroicOP* ret = new HeroicOP();

	// Loop through the starters for this class and add them to the HO
	for (itr = m_hoList[class_id].begin(); itr != m_hoList[class_id].end(); itr++)
		ret->AddStarterChain(itr->first);
	
	return ret;
}

HeroicOPWheel* MasterHeroicOPList::GetWheel(HeroicOPStarter* starter) {
	if (!starter)
		return 0;

	if (m_hoList.count(starter->start_class) == 0) {
		LogWrite(SPELL__ERROR, 0, "HO", "Start class (%u) not found", starter->start_class);
		return 0;
	}
		
	if (m_hoList[starter->start_class].count(starter) == 0) {
		LogWrite(SPELL__ERROR, 0, "HO", "Wheel not found for the provided starter (%u)", starter->id);
		return 0;
	}

	int index = MakeRandomInt(0, m_hoList[starter->start_class][starter].size() - 1);

	if(index < m_hoList[starter->start_class][starter].size())
		return m_hoList[starter->start_class][starter].at(index);
	else
		LogWrite(SPELL__ERROR, 0, "HO", "Wheel index %u for heroic_ops starter ID %u NOT Found!! Wheel starter_class %u, wheel size: %u.  Wheels that match starter_link_id -> Starter 'id' missing.", index, starter->id, starter->start_class, m_hoList[starter->start_class][starter].size());
	
	return nullptr;
}

