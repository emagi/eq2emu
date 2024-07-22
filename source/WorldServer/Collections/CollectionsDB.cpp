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
#include "Collections.h"

extern MasterCollectionList master_collection_list;


void WorldDatabase::LoadCollections() 
{
	Collection *collection;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int32 cItems_total = 0;
	int32 cItems_rewards = 0;

	res = query.RunQuery2(Q_SELECT,	"SELECT `id`,`collection_name`,`collection_category`,`level`\n"
									"FROM `collections`");
	if (res) 
	{
		while ((row = mysql_fetch_row(res))) 
		{
			collection = new Collection();
			collection->SetID(atoul(row[0]));
			collection->SetName(row[1]);
			collection->SetCategory(row[2]);
			collection->SetLevel(atoi(row[3]));

			LogWrite(COLLECTION__DEBUG, 5, "Collect", "\tLoading Collection: '%s' (%u)", collection->GetName(),collection->GetID());

			if (!master_collection_list.AddCollection(collection)) 
			{
				LogWrite(COLLECTION__ERROR, 0, "Collect", "Error adding collection '%s' - duplicate ID: %u", collection->GetName(),collection->GetID()); 
				safe_delete(collection);
				continue;
			}

			cItems_total += LoadCollectionItems(collection);
			cItems_rewards += LoadCollectionRewards(collection);
		}
	}
	LogWrite(COLLECTION__DEBUG, 0, "Collect", "\tLoaded %u collections", master_collection_list.Size());
	LogWrite(COLLECTION__DEBUG, 0, "Collect", "\tLoaded %u collection items", cItems_total);
	LogWrite(COLLECTION__DEBUG, 0, "Collect", "\tLoaded %u collection rewards", cItems_rewards);
}

int32 WorldDatabase::LoadCollectionItems(Collection *collection) 
{
	struct CollectionItem *collection_item;
	Item *item;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int32 total = 0;

	assert(collection);

	res = query.RunQuery2(Q_SELECT,	"SELECT `item_id`,`item_index`\n"
									"FROM `collection_details`\n"
									"WHERE `collection_id`=%u\n"
									"ORDER BY `item_index` ASC",
									collection->GetID());

	if (res) 
	{
		while ((row = mysql_fetch_row(res))) 
		{
			if ((item = master_item_list.GetItem(atoul(row[0])))) 
			{
				collection_item = new struct CollectionItem;
				collection_item->item = atoul(row[0]);
				collection_item->index = atoi(row[1]);
				collection_item->found = 0;
				LogWrite(COLLECTION__DEBUG, 5, "Collect", "\tLoading Collection Item: (%u)", atoul(row[0])); //LogWrite(COLLECTION__DEBUG, 5, "Collect", "\tLoading Collection Item: '%s' (%u)", master_item_list.GetItem(collection_item->item)->name.c_str(), atoul(row[0]));
				collection->AddCollectionItem(collection_item);
				total++;
			}
		}
	}
	if(query.GetErrorNumber())
		LogWrite(COLLECTION__ERROR, 0, "Collect", "Error Loading Collection Items, Query: %s, Error: %s", query.GetQuery(), query.GetError());

	return total;
}

int32 WorldDatabase::LoadCollectionRewards(Collection *collection) 
{
	struct CollectionRewardItem *reward_item;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int32 total = 0;

	assert(collection);

	res = query.RunQuery2(Q_SELECT,	"SELECT `reward_type`,`reward_value`,`reward_quantity`\n"
									"FROM `collection_rewards`\n"
									"WHERE `collection_id`=%u",
									collection->GetID());

	if (res) 
	{
		while ((row = mysql_fetch_row(res))) 
		{
			LogWrite(COLLECTION__DEBUG, 5, "Collect", "\tLoading Collection Reward: Type: %s, Val: %s, Qty: %u", row[0], row[1], atoi(row[2]));

			if (!strcasecmp(row[0], "Item")) 
			{
				reward_item = new struct CollectionRewardItem;
				reward_item->item = master_item_list.GetItem(atoul(row[1]));
				reward_item->quantity = atoi(row[2]);
				collection->AddRewardItem(reward_item);
				total++;
			}
			else if (!strcasecmp(row[0], "Selectable")) 
			{
				reward_item = new struct CollectionRewardItem;
				reward_item->item = master_item_list.GetItem(atoul(row[1]));
				reward_item->quantity = atoi(row[2]);
				collection->AddSelectableRewardItem(reward_item);
				total++;
			}
			else if (!strcasecmp(row[0], "Coin")) 
			{
				collection->SetRewardCoin(atoi64(row[1]));
				total++;
			}
			else if (!strcasecmp(row[0], "XP")) 
			{
				collection->SetRewardXP(atoi64(row[1]));
				total++;
			}
			else
				LogWrite(COLLECTION__ERROR, 0, "Collect", "Error adding collection reward to collection '%s'. Unknown reward type '%s'", collection->GetName(), row[0]);
		}
	}

	if(query.GetErrorNumber())
		LogWrite(COLLECTION__ERROR, 0, "Collect", "Error Loading Collection Rewards, Query: %s, Error: %s", query.GetQuery(), query.GetError());

	return total;
}

void WorldDatabase::LoadPlayerCollections(Player *player) 
{
	Collection *collection;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;

	assert(player);

	res = query.RunQuery2(Q_SELECT,	"SELECT `collection_id`,`completed` FROM `character_collections` WHERE `char_id`=%u", player->GetCharacterID());
	if (res) 
	{
		while ((row = mysql_fetch_row(res))) 
		{
			collection = new Collection(master_collection_list.GetCollection(atoul(row[0])));
			collection->SetCompleted(atoi(row[1]));

			if (!player->GetCollectionList()->AddCollection(collection)) 
			{
				LogWrite(COLLECTION__ERROR, 0, "Collect", "Error adding collection %u to player '%s' - duplicate ID\n", collection->GetID(), player->GetName());
				safe_delete(collection);
				continue;
			}

			LoadPlayerCollectionItems(player, collection);
		}
	}
	if(query.GetErrorNumber())
		LogWrite(COLLECTION__ERROR, 0, "Collect", "Error Loading Character Collections, Query: %s, Error: %s", query.GetQuery(), query.GetError());
}

void WorldDatabase::LoadPlayerCollectionItems(Player *player, Collection *collection) 
{
	struct CollectionItem *collection_item;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;

	assert(player);
	assert(collection);

	res = query.RunQuery2(Q_SELECT,	"SELECT `collection_item_id`\n"
									"FROM `character_collection_items`\n"
									"WHERE `char_id`=%u\n"
									"AND `collection_id`=%u",
									player->GetCharacterID(), collection->GetID());

	if (res) 
	{
		while ((row = mysql_fetch_row(res))) 
		{
			if ((collection_item = collection->GetCollectionItemByItemID(atoul(row[0]))))
				collection_item->found = true;
			else
				LogWrite(COLLECTION__ERROR, 0, "Collect", "Error Loading character collection items. Item ID %u does not exist in collection %s", atoul(row[0]), collection->GetName());
		}
	}
	if(query.GetErrorNumber())
		LogWrite(COLLECTION__ERROR, 0, "Collect", "Error Loading Character Collection Items, Query: %s, Error: %s", query.GetQuery(), query.GetError());
}

void WorldDatabase::SavePlayerCollections(Client *client) 
{
	map<int32, Collection *> *collections;
	map<int32, Collection *>::iterator itr;
	Collection *collection;

	assert(client);

	collections = client->GetPlayer()->GetCollectionList()->GetCollections();
	for (itr = collections->begin(); itr != collections->end(); itr++) 
	{
		collection = itr->second;
		if (collection->GetSaveNeeded()) 
		{
			SavePlayerCollection(client, collection);
			SavePlayerCollectionItems(client, collection);
			collection->SetSaveNeeded(false);
		}
	}
}

void WorldDatabase::SavePlayerCollection(Client *client, Collection *collection) 
{
	Query query;

	assert(client);
	assert(collection);

	query.AddQueryAsync(client->GetCharacterID(), this, Q_UPDATE,	"INSERT INTO `character_collections` (`char_id`,`collection_id`,`completed`)\n"
								"VALUES (%u,%u,0)\n"
								"ON DUPLICATE KEY UPDATE `completed`=%i",
								client->GetPlayer()->GetCharacterID(), collection->GetID(),
								collection->GetCompleted() ? 1 : 0);
}

void WorldDatabase::SavePlayerCollectionItems(Client *client, Collection *collection) 
{
	vector<struct CollectionItem *> *collection_items;
	vector<struct CollectionItem *>::iterator itr;
	struct CollectionItem *collection_item;

	assert(client);
	assert(collection);

	collection_items = collection->GetCollectionItems();
	for (itr = collection_items->begin(); itr != collection_items->end(); itr++) 
	{
		collection_item = *itr;
		if (collection_item->found > 0)
			SavePlayerCollectionItem(client, collection, collection_item->item);
	}
}

void WorldDatabase::SavePlayerCollectionItem(Client *client, Collection *collection, int32 item_id) 
{
	Query query;

	assert(client);
	assert(collection);
	//assert(item);

	query.AddQueryAsync(client->GetCharacterID(), this, Q_INSERT,	"INSERT IGNORE INTO `character_collection_items` (`char_id`,`collection_id`,`collection_item_id`)\n"
								"VALUES (%u,%u,%u)",
								client->GetPlayer()->GetCharacterID(), collection->GetID(), item_id);
}

