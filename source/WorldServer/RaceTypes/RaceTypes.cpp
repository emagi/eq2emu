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

#include "RaceTypes.h"
#include <string.h>

MasterRaceTypeList::MasterRaceTypeList() {

}

MasterRaceTypeList::~MasterRaceTypeList() {

}

bool MasterRaceTypeList::AddRaceType(int16 model_id, int16 race_type_id, const char* category, const char* subcategory, const char* modelname, bool allow_override) {
	if (m_raceList.count(model_id) == 0 || allow_override) {
		RaceTypeStructure rts;
		m_raceList[model_id].race_type_id = race_type_id;
		if(category != NULL) {
			strncpy(m_raceList[model_id].category, category, 64);
		} else {
			strcpy(m_raceList[model_id].category,"");
		}
		
		if(subcategory != NULL) {
			strncpy(m_raceList[model_id].subcategory, subcategory, 64);
		} else {
			strcpy(m_raceList[model_id].subcategory,"");
		}
		
		if(modelname != NULL) {
			strncpy(m_raceList[model_id].modelname, modelname, 64);
		} else {
			strcpy(m_raceList[model_id].modelname,"");
		}
		
		return true;
	}
	
	return false;
}

int16 MasterRaceTypeList::GetRaceType(int16 model_id) {
	int16 ret = 0;

	if (m_raceList.count(model_id) > 0)
		ret = m_raceList[model_id].race_type_id;

	return ret;
}

char* MasterRaceTypeList::GetRaceTypeCategory(int16 model_id) {
	if(m_raceList.count(model_id)  > 0 && strlen(m_raceList[model_id].category) > 0)
		return m_raceList[model_id].category;
	
	return "";
}

char* MasterRaceTypeList::GetRaceTypeSubCategory(int16 model_id) {
	if(m_raceList.count(model_id)  > 0 && strlen(m_raceList[model_id].subcategory) > 0)
		return m_raceList[model_id].subcategory;
	
	return "";
}


char* MasterRaceTypeList::GetRaceTypeModelName(int16 model_id) {
	if(m_raceList.count(model_id)  > 0 && strlen(m_raceList[model_id].modelname) > 0)
		return m_raceList[model_id].modelname;
	
	return "";
}

int16 MasterRaceTypeList::GetRaceBaseType(int16 model_id) {
	int16 ret = 0;

	if (m_raceList.count(model_id) == 0)
		return ret;

	int16 race = m_raceList[model_id].race_type_id;
	if (race >= DRAGONKIND && race < FAY)
		ret = DRAGONKIND;
	else if (race >= FAY && race < MAGICAL)
		ret = FAY;
	else if (race >= MAGICAL && race < MECHANIMAGICAL)
		ret = MAGICAL;
	else if (race >= MECHANIMAGICAL && race < NATURAL)
		ret = MECHANIMAGICAL;
	else if (race >= NATURAL && race < PLANAR)
		ret = NATURAL;
	else if (race >= PLANAR && race < PLANT)
		ret = PLANAR;
	else if (race >= PLANT && race < SENTIENT)
		ret = PLANT;
	else if (race >= SENTIENT && race < UNDEAD)
		ret = SENTIENT;
	else if (race >= UNDEAD && race < WERE)
		ret = UNDEAD;
	else if (race >= WERE)
		ret = WERE;

	return ret;
}