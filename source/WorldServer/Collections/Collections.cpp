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
#include "Collections.h"

#include "../../common/Log.h"
#include <assert.h>

extern MasterCollectionList master_collection_list;

Collection::Collection() {
	id = 0;
	memset(name, 0, sizeof(name));
	memset(category, 0, sizeof(category));
	level = 0;
	reward_coin = 0;
	reward_xp = 0;
	completed = false;
	save_needed = false;
}

Collection::Collection(Collection *in) {
	vector<struct CollectionItem *> *collection_items_in;
	vector<struct CollectionRewardItem *> *reward_items_in;
	vector<struct CollectionItem *>::iterator itr;
	vector<struct CollectionRewardItem *>::iterator itr2;
	struct CollectionItem *collection_item;
	struct CollectionRewardItem *reward_item;

	assert(in);

	id = in->GetID();
	strncpy(name, in->GetName(), sizeof(name));
	strncpy(category, in->GetCategory(), sizeof(category));
	level = in->GetLevel();
	reward_coin = in->GetRewardCoin();
	reward_xp = in->GetRewardXP();
	completed = in->GetCompleted();
	save_needed = in->GetSaveNeeded();

	collection_items_in = in->GetCollectionItems();
	for (itr = collection_items_in->begin(); itr != collection_items_in->end(); itr++) {
		collection_item = new struct CollectionItem;
		collection_item->item = (*itr)->item;
		collection_item->index = (*itr)->index;
		collection_item->found = (*itr)->found;
		collection_items.push_back(collection_item);
	}

	reward_items_in = in->GetRewardItems();
	for (itr2 = reward_items_in->begin(); itr2 != reward_items_in->end(); itr2++) {
		reward_item = new struct CollectionRewardItem;
		reward_item->item = (*itr2)->item;
		reward_item->quantity = (*itr2)->quantity;
		reward_items.push_back(reward_item);
	}

	reward_items_in = in->GetSelectableRewardItems();
	for (itr2 = reward_items_in->begin(); itr2 != reward_items_in->end(); itr2++) {
		reward_item = new struct CollectionRewardItem;
		reward_item->item = (*itr2)->item;
		reward_item->quantity = (*itr2)->quantity;
		selectable_reward_items.push_back(reward_item);
	}
}

Collection::~Collection() {
	vector<struct CollectionItem *>::iterator itr;
	vector<struct CollectionRewardItem *>::iterator itr2;

	for (itr = collection_items.begin(); itr != collection_items.end(); itr++)
		safe_delete(*itr);
	for (itr2 = reward_items.begin(); itr2 != reward_items.end(); itr2++)
		safe_delete(*itr2);
	for (itr2 = selectable_reward_items.begin(); itr2 != selectable_reward_items.end(); itr2++)
		safe_delete(*itr2);
}

void Collection::AddCollectionItem(struct CollectionItem *collection_item) {
	assert(collection_item);

	collection_items.push_back(collection_item);
}

void Collection::AddRewardItem(struct CollectionRewardItem *reward_item) {
	assert(reward_item);

	reward_items.push_back(reward_item);
}

void Collection::AddSelectableRewardItem(struct CollectionRewardItem *reward_item) {
	assert(reward_item);

	selectable_reward_items.push_back(reward_item);
}

bool Collection::NeedsItem(Item *item) {
	vector<struct CollectionItem *>::iterator itr;
	struct CollectionItem *collection_item;
	
	assert(item);

	if (completed)
		return false;

	for (itr = collection_items.begin(); itr != collection_items.end(); itr++) {
		collection_item = *itr;
		if (collection_item->item == item->details.item_id) {
			if (collection_item->found)
				return false;
			else
				return true;
		}
	}

	/* item is not required by this collection at all */
	return false;
}

struct CollectionItem * Collection::GetCollectionItemByItemID(int32 item_id) {
	vector<struct CollectionItem *>::iterator itr;
	struct CollectionItem *collection_item;

	for (itr = collection_items.begin(); itr != collection_items.end(); itr++) {
		collection_item = *itr;
		if (collection_item->item == item_id)
			return collection_item;
	}

	return 0;
}

bool Collection::GetIsReadyToTurnIn() {
	vector<struct CollectionItem *>::iterator itr;

	if (completed)
		return false;

	for (itr = collection_items.begin(); itr != collection_items.end(); itr++) {
		if (!(*itr)->found)
			return false;
	}

	return true;
}

MasterCollectionList::MasterCollectionList() {
	mutex_collections.SetName("MasterCollectionList::collections");
}

MasterCollectionList::~MasterCollectionList() {
	ClearCollections();
}

bool MasterCollectionList::AddCollection(Collection *collection) {
	bool ret = false;

	assert(collection);

	mutex_collections.writelock(__FUNCTION__, __LINE__);
	if (collections.count(collection->GetID()) == 0) {
		collections[collection->GetID()] = collection;
		ret = true;
	}
	mutex_collections.releasewritelock(__FUNCTION__, __LINE__);

	return ret;
}

Collection * MasterCollectionList::GetCollection(int32 collection_id) {
	Collection *collection = 0;

	mutex_collections.readlock(__FUNCTION__, __LINE__);
	if (collections.count(collection_id) > 0)
		collection = collections[collection_id];
	mutex_collections.releasereadlock(__FUNCTION__, __LINE__);

	return collection;
}

void MasterCollectionList::ClearCollections() {
	map<int32, Collection *>::iterator itr;

	mutex_collections.writelock(__FUNCTION__, __LINE__);
	for (itr = collections.begin(); itr != collections.end(); itr++)
		safe_delete(itr->second);
	collections.clear();
	mutex_collections.releasewritelock(__FUNCTION__, __LINE__);
}

int32 MasterCollectionList::Size() {
	int32 size;

	mutex_collections.readlock(__FUNCTION__, __LINE__);
	size = collections.size();
	mutex_collections.releasereadlock(__FUNCTION__, __LINE__);

	return size;
}

bool MasterCollectionList::NeedsItem(Item *item) {
	map<int32, Collection *>::iterator itr;
	bool ret = false;

	assert(item);

	mutex_collections.readlock(__FUNCTION__, __LINE__);
	for (itr = collections.begin(); itr != collections.end(); itr++) {
		if (itr->second->NeedsItem(item)) {
			ret = true;
			break;
		}
	}
	mutex_collections.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

PlayerCollectionList::PlayerCollectionList() {
}

PlayerCollectionList::~PlayerCollectionList() {
	ClearCollections();
}

bool PlayerCollectionList::AddCollection(Collection *collection) {
	assert(collection);

	if (collections.count(collection->GetID()) == 0) {
		collections[collection->GetID()] = collection;
		return true;
	}

	return false;
}

Collection * PlayerCollectionList::GetCollection(int32 collection_id) {
	if (collections.count(collection_id) > 0)
		return collections[collection_id];

	return 0;
}

void PlayerCollectionList::ClearCollections() {
	map<int32, Collection *>::iterator itr;

	for (itr = collections.begin(); itr != collections.end(); itr++)
		safe_delete(itr->second);
	collections.clear();
}

int32 PlayerCollectionList::Size() {
	return collections.size();
}

bool PlayerCollectionList::NeedsItem(Item *item) {
	map<int32, Collection *> *master_collections;
	map<int32, Collection *>::iterator itr;
	Collection *collection;
	Mutex *master_mutex;
	bool ret = false;

	assert(item);

	for (itr = collections.begin(); itr != collections.end(); itr++) {
		if (itr->second->NeedsItem(item)) {
			ret = true;
			break;
		}
	}

	/* if the player doesnt have a collection that needs the item, check the master collection list to see if there's a collection
	 * in there that needs the item that the player does not have yet */
	if (!ret) {
		master_mutex = master_collection_list.GetMutex();
		master_collections = master_collection_list.GetCollections();

		master_mutex->readlock(__FUNCTION__, __LINE__);
		for (itr = master_collections->begin(); itr != master_collections->end(); itr++) {
			collection = itr->second;
			if (collection->NeedsItem(item) && !GetCollection(collection->GetID())) {
				ret = true;
				break;
			}
		}
		master_mutex->releasereadlock(__FUNCTION__, __LINE__);
	}

	return ret;
}

bool PlayerCollectionList::HasCollectionsToHandIn() {
	map<int32, Collection *>::iterator itr;

	for (itr = collections.begin(); itr != collections.end(); itr++) {
		if (itr->second->GetIsReadyToTurnIn())
			return true;
	}

	return false;
}