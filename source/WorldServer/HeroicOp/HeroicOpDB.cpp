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
#include "HeroicOp.h"

extern MasterHeroicOPList master_ho_list;

void WorldDatabase::LoadHOStarters() {
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `id`, `starter_class`, `starter_icon`, `ability1`, `ability2`, `ability3`, `ability4`, `ability5`, `ability6` FROM `heroic_ops` WHERE `ho_type`='Starter'");

	if (result && mysql_num_rows(result) > 0) {
		int32 count = 0;
		while ((row = mysql_fetch_row(result))) {
			HeroicOPStarter* starter = new HeroicOPStarter;
			starter->id = atoul(row[0]);
			starter->start_class = atoi(row[1]);
			starter->starter_icon = atoi(row[2]);
			starter->abilities[0] = atoi(row[3]);
			starter->abilities[1] = atoi(row[4]);
			starter->abilities[2] = atoi(row[5]);
			starter->abilities[3] = atoi(row[6]);
			starter->abilities[4] = atoi(row[7]);
			starter->abilities[5] = atoi(row[8]);
			master_ho_list.AddStarter(starter->start_class, starter);
			count++;
		}

		LogWrite(WORLD__INFO, 0, "World", "- Loaded %u starter chains", count);
	}
}

void WorldDatabase::LoadHOWheel() {
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `starter_link_id`, `chain_order`, `shift_icon`, `spell_id`, `chance`, `ability1`, `ability2`, `ability3`, `ability4`, `ability5`, `ability6` FROM `heroic_ops` WHERE `ho_type`='Wheel'");

	if (result && mysql_num_rows(result) > 0) {
		int32 count = 0;
		while ((row = mysql_fetch_row(result))) {
			HeroicOPWheel* wheel = new HeroicOPWheel;
			wheel->order = atoi(row[1]);
			wheel->shift_icon = atoi(row[2]);
			wheel->spell_id = atoul(row[3]);
			wheel->chance = atof(row[4]);
			wheel->abilities[0] = atoi(row[5]);
			wheel->abilities[1] = atoi(row[6]);
			wheel->abilities[2] = atoi(row[7]);
			wheel->abilities[3] = atoi(row[8]);
			wheel->abilities[4] = atoi(row[9]);
			wheel->abilities[5] = atoi(row[10]);

			master_ho_list.AddWheel(atoul(row[0]), wheel);
			count++;
		}

		LogWrite(WORLD__INFO, 0, "World", "- Loaded %u HO wheels", count);
	}
}
