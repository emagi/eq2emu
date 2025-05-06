# File: `ItemsDB.cpp`

## Classes

_None detected_

## Functions

- `void WorldDatabase::LoadDataFromRow(DatabaseResult* result, Item* item)`
- `int32 WorldDatabase::LoadSkillItems(int32 item_id)`
- `int32 WorldDatabase::LoadShields(int32 item_id)`
- `int32 WorldDatabase::LoadAdornments(int32 item_id)`
- `int32 WorldDatabase::LoadClassifications()`
- `int32 WorldDatabase::LoadBaubles(int32 item_id)`
- `int32 WorldDatabase::LoadBooks(int32 item_id)`
- `int32 WorldDatabase::LoadItemsets(int32 item_id)`
- `int32 WorldDatabase::LoadHouseItem(int32 item_id)`
- `int32 WorldDatabase::LoadRecipeBookItems(int32 item_id)`
- `int32 WorldDatabase::LoadHouseContainers(int32 item_id){`
- `int32 WorldDatabase::LoadArmor(int32 item_id)`
- `int32 WorldDatabase::LoadBags(int32 item_id)`
- `int32 WorldDatabase::LoadFoods(int32 item_id)`
- `int32 WorldDatabase::LoadRangeWeapons(int32 item_id)`
- `int32 WorldDatabase::LoadThrownWeapons(int32 item_id)`
- `int32 WorldDatabase::LoadWeapons(int32 item_id)`
- `int32 WorldDatabase::LoadItemAppearances(int32 item_id)`
- `int32 WorldDatabase::LoadItemEffects(int32 item_id)`
- `int32 WorldDatabase::LoadBookPages(int32 item_id)`
- `int32 WorldDatabase::LoadItemLevelOverride(int32 item_id)`
- `int32 WorldDatabase::LoadItemStats(int32 item_id)`
- `else if(row[4])`
- `int32 WorldDatabase::LoadItemModStrings(int32 item_id)`
- `void WorldDatabase::LoadBrokerItemStats()`
- `void WorldDatabase::ReloadItemList(int32 item_id)`
- `void WorldDatabase::LoadItemList(int32 item_id)`
- `int32 WorldDatabase::LoadNextUniqueItemID()`
- `return strtoul(row[0], NULL, 0);`
- `else if(!result)`
- `void WorldDatabase::SaveItems(Client* client)`
- `else if(item->save_needed)`
- `else if(item->save_needed) {`
- `else if(item->save_needed) {`
- `void WorldDatabase::SaveItem(int32 account_id, int32 char_id, Item* item, const char* type)`
- `void WorldDatabase::DeleteItem(int32 char_id, Item* item, const char* type)`
- `void WorldDatabase::LoadCharacterItemList(int32 account_id, int32 char_id, Player* player, int16 version)`
- `else if (strncasecmp(row[0], "APPEARANCE", 10) == 0)`

## Notable Comments

- /*
- */
- // handle new database class til all functions are converted
- // this is too much on top of already having the top level load item debug msg
- //	LogWrite(ITEM__DEBUG, 5, "Items", "\tSetting details for item ID: %i", result->GetInt32Str("id"));
- // add more Flags/Flags2 here
- //LogWrite(ITEM__DEBUG, 0, "Items", "\tItem Adornment for item_id: %u", id);
- //LogWrite(ITEM__DEBUG, 0, "Items", "\ttype: %i, Duration: %i, item_types_: %i, slot_type: %i", ITEM_TYPE_ADORNMENT, atoi(row[1]), atoi(row[2]), atoi(row[3]));
- //LogWrite(ITEM__DEBUG, 0, "Items", "\ttype: %i, Duration: %i, item_types_: %i, slot_type: %i",item->generic_info.item_type, item->adornment_info->duration, item->adornment_info->item_types, item->adornment_info->slot_type);
- //if (database_new.Select(&result, "SELECT id, itemset_item_id, item_id, item_icon,item_stack_size,item_list_color,language_type FROM item_details_itemset"))
