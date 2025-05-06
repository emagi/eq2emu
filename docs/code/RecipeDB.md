# File: `RecipeDB.cpp`

## Classes

_None detected_

## Functions

- `void WorldDatabase::LoadRecipes() {`
- `void WorldDatabase::LoadRecipeBooks(){`
- `void WorldDatabase::LoadPlayerRecipes(Player *player){`
- `int32 WorldDatabase::LoadPlayerRecipeBooks(int32 char_id, Player *player) {`
- `void WorldDatabase::SavePlayerRecipeBook(Player* player, int32 recipebook_id){`
- `void WorldDatabase::SavePlayerRecipe(Player* player, int32 recipe_id) {`
- `void WorldDatabase::LoadRecipeComponents() {`
- `void WorldDatabase::UpdatePlayerRecipe(Player* player, int32 recipe_id, int8 highest_stage) {`

## Notable Comments

- /*
- */
- //Convert the device string
- //Products/By-Products
- //Advance i past all the product info
- //i += 15;
- //if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF)
- //LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error in SavePlayerRecipeBook query '%s' : %s", query.GetQuery(), query.GetError());
- //if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF)
- //LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error in SavePlayerRecipeBook query '%s' : %s", query.GetQuery(), query.GetError());
