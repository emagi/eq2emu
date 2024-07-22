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

#include "../../common/Log.h"
#include "../WorldDatabase.h"
#include "../World.h"

extern World world;

void WorldDatabase::LoadLoot(ZoneServer* zone) 
{
	// First, clear previous loot tables...
	zone->ClearLootTables();

	DatabaseResult result;
	int32 count = 0;

	if (database_new.Select(&result, "SELECT id, name, mincoin, maxcoin, maxlootitems, lootdrop_probability, coin_probability FROM loottable")) {
		LogWrite(LOOT__DEBUG, 0, "Loot", "--Loading LootTables...");
		LootTable* table = 0;
		// Load loottable from DB
		while(result.Next()) {
			int32 id = result.GetInt32Str("id");
			
			table = new LootTable;
			table->name = result.GetStringStr("name");
			table->mincoin = result.GetInt32Str("mincoin");
			table->maxcoin = result.GetInt32Str("maxcoin");
			table->maxlootitems = result.GetInt16Str("maxlootitems");
			table->lootdrop_probability = result.GetFloatStr("lootdrop_probability");
			table->coin_probability = result.GetFloatStr("coin_probability");
			zone->AddLootTable(id, table);

			LogWrite(LOOT__DEBUG, 5, "Loot", "---Loading LootTable '%s' (id: %u)", table->name.c_str(), id);
			LogWrite(LOOT__DEBUG, 5, "Loot", "---min_coin: %u, max_coin: %u, max_items: %i, prob: %.2f, coin_prob: %.2f", table->mincoin, table->maxcoin, table->maxlootitems, table->lootdrop_probability, table->coin_probability);
			count++;
		}
		LogWrite(LOOT__DEBUG, 0, "Loot", "--Loaded %u loot table%s.", count, count == 1 ? "" : "s");
	}
	
	// Now, load Loot Drops for configured loot tables
	if (database_new.Select(&result, "SELECT loot_table_id, item_id, item_charges, equip_item, probability, no_drop_quest_completed FROM lootdrop")) {
		count = 0;
		LogWrite(LOOT__DEBUG, 0, "Loot", "--Loading LootDrops...");
		LootDrop* drop = 0;

		while(result.Next()) {
			int32 id = result.GetInt32Str("loot_table_id");
			drop = new LootDrop;
			drop->item_id = result.GetInt32Str("item_id");
			drop->item_charges = result.GetInt16Str("item_charges");
			drop->equip_item = (result.GetInt8Str("equip_item") == 1);
			drop->probability = result.GetFloatStr("probability");
			drop->no_drop_quest_completed_id = result.GetInt32Str("no_drop_quest_completed");
			zone->AddLootDrop(id, drop);

			LogWrite(LOOT__DEBUG, 5, "Loot", "---Loading LootDrop item_id %u (tableID: %u", drop->item_id, id);
			LogWrite(LOOT__DEBUG, 5, "Loot", "---charges: %i, equip_item: %i, prob: %.2f", drop->item_charges, drop->equip_item, drop->probability);
			count++;
		}
		LogWrite(LOOT__DEBUG, 0, "Loot", "--Loaded %u loot drop%s.", count, count == 1 ? "" : "s");
	}

	// Finally, load loot tables into spawns that are set to use these loot tables
	if (database_new.Select(&result, "SELECT spawn_id, loottable_id FROM spawn_loot")) {
		count = 0;
		LogWrite(LOOT__DEBUG, 0, "Loot", "--Assigning loot table(s) to spawn(s)...");

		while(result.Next()) {
			int32 spawn_id = result.GetInt32Str("spawn_id");
			int32 table_id = result.GetInt32Str("loottable_id");
			zone->AddSpawnLootList(spawn_id, table_id);
			LogWrite(LOOT__DEBUG, 5, "Loot", "---Adding loot table %u to spawn %u", table_id, spawn_id);
			count++;
		}
		LogWrite(LOOT__DEBUG, 0, "Loot", "--Loaded %u spawn loot list%s.", count, count == 1 ? "" : "s");
	}

	// Load global loot lists
	LoadGlobalLoot(zone);
}

void WorldDatabase::LoadGlobalLoot(ZoneServer* zone) {
	LogWrite(LOOT__INFO, 0, "Loot", "-Loading Global loot data...");
	DatabaseResult result;
	int32 count = 0;
	if (database_new.Select(&result, "SELECT type, loot_table, value1, value2, value3, value4 FROM loot_global")) {
		while(result.Next()) {
			const char* type = result.GetStringStr("type");
			int32 table_id = result.GetInt32Str("loot_table");
			if (strcmp(type, "Level") == 0) {
				GlobalLoot* loot = new GlobalLoot();
				loot->minLevel = result.GetInt8Str("value1");
				loot->maxLevel = result.GetInt8Str("value2");
				loot->table_id = table_id;
				loot->loot_tier = result.GetInt32Str("value4");

				if (loot->minLevel > loot->maxLevel)
					loot->maxLevel = loot->minLevel;

				zone->AddLevelLootList(loot);
				LogWrite(LOOT__DEBUG, 5, "Loot", "---Loading Level %i loot table (id: %u)", loot->minLevel, table_id);
				LogWrite(LOOT__DEBUG, 5, "Loot", "---minlevel: %i, maxlevel: %i", loot->minLevel, loot->maxLevel);
			}
			else if (strcmp(type, "Racial") == 0) {
				GlobalLoot* loot = new GlobalLoot();
				int16 race_id = result.GetInt16Str("value1");
				loot->minLevel = result.GetInt8Str("value2");
				loot->maxLevel = result.GetInt8Str("value3");
				loot->table_id = table_id;
				loot->loot_tier = result.GetInt32Str("value4");

				if (loot->minLevel > loot->maxLevel)
					loot->maxLevel = loot->minLevel;

				zone->AddRacialLootList(race_id, loot);
				LogWrite(LOOT__DEBUG, 5, "Loot", "---Loading Racial %i loot table (id: %u)", race_id, table_id);
				LogWrite(LOOT__DEBUG, 5, "Loot", "---minlevel: %i, maxlevel: %i", loot->minLevel, loot->maxLevel);
			}
			else if (strcmp(type, "Zone") == 0) {
				GlobalLoot* loot = new GlobalLoot();
				int32 zoneID = result.GetInt32Str("value1");
				loot->minLevel = result.GetInt8Str("value2");
				loot->maxLevel = result.GetInt8Str("value3");
				loot->table_id = table_id;
				loot->loot_tier = result.GetInt32Str("value4");

				if (loot->minLevel > loot->maxLevel)
					loot->maxLevel = loot->minLevel;
				
				zone->AddZoneLootList(zoneID, loot);
				LogWrite(LOOT__DEBUG, 5, "Loot", "---Loading Zone %i loot table (id: %u)", zoneID, table_id);
				LogWrite(LOOT__DEBUG, 5, "Loot", "---minlevel: %i, maxlevel: %i", loot->minLevel, loot->maxLevel);
			}
			count++;
		}

		LogWrite(LOOT__DEBUG, 0, "Loot", "--Loaded %u Global loot list%s.", count, count == 1 ? "" : "s");
	}
}

bool WorldDatabase::LoadSpawnLoot(ZoneServer* zone, Spawn* spawn)
{
	if (!spawn->GetDatabaseID())
		return false;

	DatabaseResult result;
	int32 count = 0;

	zone->ClearSpawnLootList(spawn->GetDatabaseID());
	// Finally, load loot tables into spawns that are set to use these loot tables
	if (database_new.Select(&result, "SELECT spawn_id, loottable_id FROM spawn_loot where spawn_id=%u",spawn->GetDatabaseID())) {
		count = 0;
		LogWrite(LOOT__DEBUG, 0, "Loot", "--Assigning loot table(s) to spawn(s)...");

		while (result.Next()) {
			int32 spawn_id = result.GetInt32Str("spawn_id");
			int32 table_id = result.GetInt32Str("loottable_id");
			zone->AddSpawnLootList(spawn_id, table_id);
			LogWrite(LOOT__DEBUG, 5, "Loot", "---Adding loot table %u to spawn %u", table_id, spawn_id);
			count++;
		}
		LogWrite(LOOT__DEBUG, 0, "Loot", "--Loaded %u spawn loot list%s.", count, count == 1 ? "" : "s");
		return true;
	}
	return false;
}

void WorldDatabase::AddLootTableToSpawn(Spawn* spawn, int32 loottable_id) {
	Query query;
	query.RunQuery2(Q_INSERT, "insert into spawn_loot set spawn_id=%u,loottable_id=%u", spawn->GetDatabaseID(), loottable_id);
}

bool WorldDatabase::RemoveSpawnLootTable(Spawn* spawn, int32 loottable_id) {
	Query query;
	if (loottable_id)
	{
		string delete_char = string("delete from spawn_loot where spawn_id=%i and loottable_id=%i");
		query.RunQuery2(Q_DELETE, delete_char.c_str(), spawn->GetDatabaseID(), loottable_id);
	}
	else
	{
		string delete_char = string("delete from spawn_loot where spawn_id=%i");
		query.RunQuery2(Q_DELETE, delete_char.c_str(), spawn->GetDatabaseID());
	}
	if (!query.GetAffectedRows())
	{
		//No error just in case ppl try doing stupid stuff
		return false;
	}

	return true;
}