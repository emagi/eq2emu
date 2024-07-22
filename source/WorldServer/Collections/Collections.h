#ifndef COLLECTIONS_H_
#define COLLECTIONS_H_

#include "../../common/types.h"
#include "../../common/Mutex.h"
#include "../Items/Items.h"
#include <map>
#include <vector>

using namespace std;

struct CollectionItem {
	int32 item;
	int8 index;
	int8 found;
};

struct CollectionRewardItem {
	Item *item;
	int8 quantity;
};

class Collection {
public:
	Collection();
	Collection(Collection *in);
	virtual ~Collection();

	void SetID(int32 id) {this->id = id;}
	void SetName(const char *name) {strncpy(this->name, name, sizeof(this->name));}
	void SetCategory(const char *category) {strncpy(this->category, category, sizeof(this->category));}
	void SetLevel(int8 level) {this->level = level;}
	void SetCompleted(bool completed) {this->completed = completed;}
	void SetSaveNeeded(bool save_needed) {this->save_needed = save_needed;}
	void AddCollectionItem(struct CollectionItem *collection_item);
	void AddRewardItem(struct CollectionRewardItem *reward_item);
	void AddSelectableRewardItem(struct CollectionRewardItem *reward_item);
	void SetRewardCoin(int64 reward_coin) {this->reward_coin = reward_coin;}
	void SetRewardXP(int64 reward_xp) {this->reward_xp = reward_xp;}
	bool NeedsItem(Item *item);
	struct CollectionItem * GetCollectionItemByItemID(int32 item_id);

	int32 GetID() {return id;}
	const char * GetName() {return name;}
	const char * GetCategory() {return category;}
	int8 GetLevel() {return level;}
	bool GetIsReadyToTurnIn();
	bool GetCompleted() {return completed;}
	bool GetSaveNeeded() {return save_needed;}
	vector<struct CollectionItem *> * GetCollectionItems() {return &collection_items;}
	vector<struct CollectionRewardItem *> * GetRewardItems() {return &reward_items;}
	vector<struct CollectionRewardItem *> * GetSelectableRewardItems() {return &selectable_reward_items;}
	int64 GetRewardCoin() {return reward_coin;}
	int64 GetRewardXP() {return reward_xp;}

private:
	int32 id;
	char name[512];
	char category[512];
	int8 level;
	int64 reward_coin;
	int64 reward_xp;
	bool completed;
	bool save_needed;
	vector<struct CollectionItem *> collection_items;
	vector<struct CollectionRewardItem *> reward_items;
	vector<struct CollectionRewardItem *> selectable_reward_items;
};

class MasterCollectionList {
public:
	MasterCollectionList();
	virtual ~MasterCollectionList();

	bool AddCollection(Collection *collection);
	Collection * GetCollection(int32 collection_id);
	void ClearCollections();
	int32 Size();
	bool NeedsItem(Item *item);

	Mutex * GetMutex() {return &mutex_collections;}
	map<int32, Collection *> * GetCollections() {return &collections;}

private:
	Mutex mutex_collections;
	map<int32, Collection *> collections;
};

class PlayerCollectionList {
public:
	PlayerCollectionList();
	virtual ~PlayerCollectionList();

	bool AddCollection(Collection *collection);
	Collection * GetCollection(int32 collection_id);
	void ClearCollections();
	int32 Size();
	bool NeedsItem(Item *item);
	bool HasCollectionsToHandIn();

	map<int32, Collection *> * GetCollections() {return &collections;}

private:
	map<int32, Collection *> collections;
};

#endif