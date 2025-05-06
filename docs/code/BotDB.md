# File: `BotDB.cpp`

## Classes

_None detected_

## Functions

- `int32 WorldDatabase::CreateNewBot(int32 char_id, string name, int8 race, int8 advClass, int8 gender, int16 model_id, int32& index) {`
- `void WorldDatabase::SaveBotAppearance(Bot* bot) {`
- `void WorldDatabase::SaveBotColors(int32 bot_id, const char* type, EQ2_Color color) {`
- `void WorldDatabase::SaveBotFloats(int32 bot_id, const char* type, float float1, float float2, float float3) {`
- `bool WorldDatabase::LoadBot(int32 char_id, int32 bot_index, Bot* bot) {`
- `void WorldDatabase::LoadBotAppearance(Bot* bot) {`
- `void WorldDatabase::SaveBotItem(int32 bot_id, int32 item_id, int8 slot) {`
- `void WorldDatabase::LoadBotEquipment(Bot* bot) {`
- `string WorldDatabase::GetBotList(int32 char_id) {`
- `void WorldDatabase::DeleteBot(int32 char_id, int32 bot_index) {`
- `void WorldDatabase::SetBotStartingItems(Bot* bot, int8 class_id, int8 race_id) {`

## Notable Comments

- //SaveBotColors(bot->BotID, "unknown_chest_color", );
- //SaveBotColors(bot->BotID, "unknown_legs_color", );
- //SaveBotColors(bot->BotID, "unknown9", );
- //SaveBotColors(bot->BotID, "soga_unknown_chest_color", );
- //SaveBotColors(bot->BotID, "soga_unknown_legs_color", );
- //SaveBotColors(bot->BotID, "soga_unknown13", );
