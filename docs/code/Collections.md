# File: `Collections.h`

## Classes

- `CollectionItem`
- `CollectionRewardItem`
- `Collection`
- `CollectionItem`
- `MasterCollectionList`
- `PlayerCollectionList`

## Functions

- `void SetID(int32 id) {this->id = id;}`
- `void SetName(const char *name) {strncpy(this->name, name, sizeof(this->name));}`
- `void SetCategory(const char *category) {strncpy(this->category, category, sizeof(this->category));}`
- `void SetLevel(int8 level) {this->level = level;}`
- `void SetCompleted(bool completed) {this->completed = completed;}`
- `void SetSaveNeeded(bool save_needed) {this->save_needed = save_needed;}`
- `void AddCollectionItem(struct CollectionItem *collection_item);`
- `void AddRewardItem(struct CollectionRewardItem *reward_item);`
- `void AddSelectableRewardItem(struct CollectionRewardItem *reward_item);`
- `void SetRewardCoin(int64 reward_coin) {this->reward_coin = reward_coin;}`
- `void SetRewardXP(int64 reward_xp) {this->reward_xp = reward_xp;}`
- `bool NeedsItem(Item *item);`
- `int32 GetID() {return id;}`
- `int8 GetLevel() {return level;}`
- `bool GetIsReadyToTurnIn();`
- `bool GetCompleted() {return completed;}`
- `bool GetSaveNeeded() {return save_needed;}`
- `int64 GetRewardCoin() {return reward_coin;}`
- `int64 GetRewardXP() {return reward_xp;}`
- `bool AddCollection(Collection *collection);`
- `void ClearCollections();`
- `int32 Size();`
- `bool NeedsItem(Item *item);`
- `bool AddCollection(Collection *collection);`
- `void ClearCollections();`
- `int32 Size();`
- `bool NeedsItem(Item *item);`
- `bool HasCollectionsToHandIn();`

## Notable Comments

_None detected_
