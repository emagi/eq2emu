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

#include "../WorldDatabase.h"
#include "../../common/Log.h"
#include "RaceTypes.h"

extern MasterRaceTypeList race_types_list;

void WorldDatabase::LoadRaceTypes() {
	DatabaseResult result;
	
	if(database_new.Select(&result, "SELECT `model_type`, `race_id`, `category`, `subcategory`, `model_name` FROM `race_types`")) {
		int32 count = 0;

		while (result.Next()) {
			int16 race_id = result.GetInt16Str("race_id");
			if (race_id > 0) {
				race_types_list.AddRaceType(result.GetInt16Str("model_type"), race_id, result.GetStringStr("category"), result.GetStringStr("subcategory"), result.GetStringStr("model_name"));
				count++;
			}
		}

		LogWrite(WORLD__INFO, 0, "World", "- Loaded %u Race Types", count);
	}
}