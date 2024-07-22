#include "../WorldDatabase.h"
#include "../World.h"

extern World world;

void WorldDatabase::LoadHouseZones() {
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT * FROM `houses`");

	if (result && mysql_num_rows(result) > 0) {
		while ((row = mysql_fetch_row(result))) {
			world.AddHouseZone(atoul(row[0]), row[1], atoi64(row[2]), atoul(row[3]), atoi64(row[4]), atoul(row[5]), atoi(row[6]), atoi(row[7]), atoi(row[8]), atoul(row[9]), atoul(row[10]), atof(row[11]), atof(row[12]), atof(row[13]), atof(row[14]));
		}
	}
}

int64 WorldDatabase::AddPlayerHouse(int32 char_id, int32 house_id, int32 instance_id, int32 upkeep_due) {
	Query query;
	string insert = string("INSERT INTO character_houses (char_id, house_id, instance_id, upkeep_due) VALUES (%u, %u, %u, %u) ");
	query.RunQuery2(Q_INSERT, insert.c_str(), char_id, house_id, instance_id, upkeep_due);
	
	int64 unique_id = query.GetLastInsertedID();
	return unique_id;
}

void WorldDatabase::SetHouseUpkeepDue(int32 char_id, int32 house_id, int32 instance_id, int32 upkeep_due) {
	Query query;
	string update = string("UPDATE character_houses set upkeep_due=%u where char_id = %u and house_id = %u and instance_id = %u");
	query.RunQuery2(Q_UPDATE, update.c_str(), upkeep_due, char_id, house_id, instance_id);
}


void WorldDatabase::UpdateHouseEscrow(int32 house_id, int32 instance_id, int64 amount_coins, int32 amount_status) {
	Query query;
	string update = string("UPDATE character_houses set escrow_coins = %llu, escrow_status = %u where house_id = %u and instance_id = %u");
	query.RunQuery2(Q_UPDATE, update.c_str(), amount_coins, amount_status, house_id, instance_id);
}


void WorldDatabase::RemovePlayerHouse(int32 char_id, int32 house_id) {
}

void WorldDatabase::LoadPlayerHouses() {
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT h.id, h.char_id, h.house_id, h.instance_id, h.upkeep_due, h.escrow_coins, h.escrow_status, c.name FROM character_houses h, characters c WHERE h.char_id = c.id");

	if (result && mysql_num_rows(result) > 0) {
		while ((row = mysql_fetch_row(result))) {
			world.AddPlayerHouse(atoul(row[1]), atoul(row[2]), atoi64(row[0]), atoul(row[3]), atoul(row[4]), atoi64(row[5]), atoul(row[6]), row[7]);
		}
	}
}

void WorldDatabase::LoadDeposits(PlayerHouse* ph)
{
	if (!ph)
		return;
	ph->deposits.clear();
	ph->depositsMap.clear();

	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "select timestamp, amount, last_amount, status, last_status, name from character_house_deposits where house_id = %u and instance_id = %u order by timestamp asc limit 255", ph->house_id, ph->instance_id);

	if (result && mysql_num_rows(result) > 0) {
		while ((row = mysql_fetch_row(result))) {
			Deposit d;
			d.timestamp = atoul(row[0]);

			int64 outVal = strtoull(row[1], NULL, 0);
			d.amount = outVal;

			outVal = strtoull(row[2], NULL, 0);
			d.last_amount = outVal;

			d.status = atoul(row[3]);
			d.last_status = atoul(row[4]);

			d.name = string(row[5]);

			ph->deposits.push_back(d);
			ph->depositsMap.insert(make_pair(d.name, d));
		}
	}
}

void WorldDatabase::LoadHistory(PlayerHouse* ph)
{
	if (!ph)
		return;
	ph->history.clear();

	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "select timestamp, amount, status, reason, name, pos_flag from character_house_history where house_id = %u and instance_id = %u order by timestamp asc limit 255", ph->house_id, ph->instance_id);

	if (result && mysql_num_rows(result) > 0) {
		while ((row = mysql_fetch_row(result))) {
			HouseHistory h;
			h.timestamp = atoul(row[0]);

			int64 outVal = strtoull(row[1], NULL, 0);
			h.amount = outVal;

			h.status = atoul(row[2]);

			h.reason = string(row[3]);
			h.name = string(row[4]);

			h.pos_flag = atoul(row[5]);

			ph->history.push_back(h);
		}
	}
}


void WorldDatabase::AddHistory(PlayerHouse* house, char* name, char* reason, int32 timestamp, int64 amount, int32 status, int8 pos_flag)
{
	if (!house)
		return;

	HouseHistory h(Timer::GetUnixTimeStamp(), amount, string(name), string(reason), status, pos_flag);
	house->history.push_back(h);

	Query query;
	string insert = string("INSERT INTO character_house_history (timestamp, house_id, instance_id, name, amount, status, reason, pos_flag) VALUES (%u, %u, %u, '%s', %llu, %u, '%s', %u) ");
	query.RunQuery2(Q_INSERT, insert.c_str(), timestamp, house->house_id, house->instance_id, name, amount, status, reason, pos_flag);
}