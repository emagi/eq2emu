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
#ifdef WIN32
	#include <WinSock2.h>
	#include <windows.h>
#endif
#include <mysql.h>
#include <assert.h>
#include "../../common/Log.h"
#include "../WorldDatabase.h"
#include "Tradeskills.h"

extern MasterTradeskillEventsList master_tradeskillevent_list;

void WorldDatabase::LoadTradeskillEvents() {
	TradeskillEvent* TSEvent = 0;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;

	res = query.RunQuery2(Q_SELECT,	"SELECT `name`,`icon`,`technique`,`success_progress`,`success_durability`,`success_hp`,`success_power`,`success_spell_id`,`success_item_id`,`fail_progress`,`fail_durability`,`fail_hp`, `fail_power`\n"
									"FROM `tradeskillevents`");
	if (res) {
		while ((row = mysql_fetch_row(res))) {
			TSEvent = new TradeskillEvent;

			strncpy(TSEvent->Name, row[0], sizeof(TSEvent->Name));
			TSEvent->Icon = atoi(row[1]);
			TSEvent->Technique = atoul(row[2]);
			TSEvent->SuccessProgress = atoi(row[3]);
			TSEvent->SuccessDurability = atoi(row[4]);
			TSEvent->SuccessHP = atoi(row[5]);
			TSEvent->SuccessPower = atoi(row[6]);
			TSEvent->SuccessSpellID = atoul(row[7]);
			TSEvent->SuccessItemID = atoul(row[8]);
			TSEvent->FailProgress = atoi(row[9]);
			TSEvent->FailDurability = atoi(row[10]);
			TSEvent->FailHP = atoi(row[11]);
			TSEvent->FailPower = atoi(row[12]);			

			LogWrite(TRADESKILL__DEBUG, 7, "Tradeskills", "Loading tradeskill event: %s", TSEvent->Name);
			master_tradeskillevent_list.AddEvent(TSEvent);
		}
	}

	LogWrite(TRADESKILL__DEBUG, 0, "Tradeskills", "\tLoaded %u tradeskill events", master_tradeskillevent_list.Size());
}