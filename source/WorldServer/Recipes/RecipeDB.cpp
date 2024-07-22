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
#include "Recipe.h"

extern MasterRecipeList master_recipe_list;
extern MasterRecipeBookList master_recipebook_list;
extern MasterItemList master_item_list;

void WorldDatabase::LoadRecipes() {
	DatabaseResult res;

	bool status = database_new.Select(&res, 
		"SELECT r.`id`,r.`soe_id`,r.`level`,r.`icon`,r.`skill_level`,r.`technique`,r.`knowledge`,r.`name`,r.`description`,i.`name` as `book`,r.`bench`,ipc.`adventure_classes`, "
		"r.`stage4_id`, r.`name`, r.`stage4_qty`, pcl.`name` as primary_comp_title,r.primary_comp_qty, fcl.name as `fuel_comp_title`, r.fuel_comp_qty, "
		"bc.`name` AS build_comp_title, bc.qty AS build_comp_qty, bc2.`name` AS build2_comp_title, bc2.qty AS build2_comp_qty, "
		"bc3.`name` AS build3_comp_title, bc3.qty AS build3_comp_qty, bc4.`name` AS build4_comp_title, bc4.qty AS build4_comp_qty,\n"
		"r.stage0_id, r.stage1_id, r.stage2_id, r.stage3_id, r.stage4_id, r.stage0_qty, r.stage1_qty, r.stage2_qty, r.stage3_qty, r.stage4_qty,\n"
		"r.stage0_byp_id, r.stage1_byp_id, r.stage2_byp_id, r.stage3_byp_id, r.stage4_byp_id, r.stage0_byp_qty, r.stage1_byp_qty, r.stage2_byp_qty, r.stage3_byp_qty, r.stage4_byp_qty\n"
		"FROM `recipe` r\n"
		"LEFT JOIN ((SELECT recipe_id, soe_recipe_crc FROM item_details_recipe_items GROUP BY soe_recipe_crc) as idri) ON idri.soe_recipe_crc = r.soe_id\n"
		"LEFT JOIN items i ON idri.recipe_id = i.id\n"
		"INNER JOIN items ipc ON r.stage4_id = ipc.id\n"
		"INNER JOIN recipe_comp_list pcl ON r.primary_comp_list = pcl.id\n"
		"INNER JOIN recipe_comp_list fcl ON r.fuel_comp_list = fcl.id\n"
		"LEFT JOIN (SELECT rsc.recipe_id, rsc.comp_list, rsc.`index`, rcl.`name`, rsc.qty FROM recipe_secondary_comp rsc INNER JOIN recipe_comp_list rcl ON rcl.id = rsc.comp_list WHERE `index` = 0) AS bc ON bc.recipe_id = r.id\n"
		"LEFT JOIN (SELECT rsc.recipe_id, rsc.comp_list, rsc.`index`, rcl.`name`, rsc.qty FROM recipe_secondary_comp rsc INNER JOIN recipe_comp_list rcl ON rcl.id = rsc.comp_list WHERE `index` = 1) AS bc2 ON bc2.recipe_id = r.id\n"
		"LEFT JOIN (SELECT rsc.recipe_id, rsc.comp_list, rsc.`index`, rcl.`name`, rsc.qty FROM recipe_secondary_comp rsc INNER JOIN recipe_comp_list rcl ON rcl.id = rsc.comp_list WHERE `index` = 2) AS bc3 ON bc3.recipe_id = r.id\n"
		"LEFT JOIN (SELECT rsc.recipe_id, rsc.comp_list, rsc.`index`, rcl.`name`, rsc.qty FROM recipe_secondary_comp rsc INNER JOIN recipe_comp_list rcl ON rcl.id = rsc.comp_list WHERE `index` = 3) AS bc4 ON bc4.recipe_id = r.id\n"
		"WHERE r.bHaveAllProducts");

	if (!status)
		return;

	while (res.Next()) {
		int32 i = 0;
		Recipe* recipe = new Recipe();
		recipe->SetID(res.GetInt32(i++));
		recipe->SetSoeID(res.GetInt32(i++));
		recipe->SetLevel(res.GetInt32(i++));
		recipe->SetTier(recipe->GetLevel() / 10 + 1);
		recipe->SetIcon(res.GetInt32(i++));
		recipe->SetSkill(res.GetInt32(i++));
		recipe->SetTechnique(res.GetInt32(i++));
		recipe->SetKnowledge(res.GetInt32(i++));
		recipe->SetName(res.GetString(i++));
		recipe->SetDescription(res.GetString(i++));
		recipe->SetBook(res.GetString(i++));

		//Convert the device string
		string device = res.GetString(i++);
		int32 deviceID = 0;
		int8 deviceSubType = 0;
		
		recipe->SetDevice(GetDeviceName(device).c_str());
		recipe->SetUnknown2(deviceID);
		recipe->SetDevice_Sub_Type(deviceSubType);
		recipe->SetClasses(res.GetInt64(i++));	
		recipe->SetUnknown3(0);
		recipe->SetUnknown4(0);

		LogWrite(TRADESKILL__DEBUG, 5, "Recipes", "Loading recipe: %s (%u)", recipe->GetName(), recipe->GetID());

		recipe->SetProductID(res.GetInt32(i++));
		recipe->SetProductName(res.GetString(i++));
		recipe->SetProductQuantity(res.GetInt8(i++));
		recipe->SetPrimaryComponentTitle(res.GetString(i++));
		recipe->SetPrimaryComponentQuantity(res.GetInt16(i++));
		recipe->SetFuelComponentTitle(res.GetString(i++));
		recipe->SetFuelComponentQuantity(res.GetInt16(i++));

		recipe->SetBuild1ComponentTitle(res.GetString(i++));
		recipe->SetBuild1ComponentQuantity(res.GetInt16(i++));
		recipe->SetBuild2ComponentTitle(res.GetString(i++));
		recipe->SetBuild2ComponentQuantity(res.GetInt16(i++));
		recipe->SetBuild3ComponentTitle(res.GetString(i++));
		recipe->SetBuild3ComponentQuantity(res.GetInt16(i++));
		recipe->SetBuild4ComponentTitle(res.GetString(i++));
		recipe->SetBuild4ComponentQuantity(res.GetInt16(i++));

		LogWrite(TRADESKILL__DEBUG, 7, "Recipes", "Loading recipe: %s (%u)", recipe->GetName(), recipe->GetID());

		if (!master_recipe_list.AddRecipe(recipe)) {
			LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error adding recipe '%s' - duplicate ID: %u", recipe->GetName(), recipe->GetID());
			delete recipe;
			continue;
		}

		//Products/By-Products
		for (int8 stage = 0; stage < 5; stage++) {
			RecipeProducts* rp = new RecipeProducts;
			rp->product_id = res.GetInt32(i);
			rp->product_qty = res.GetInt8(i + 5);
			rp->byproduct_id = res.GetInt32(i + 10);
			rp->byproduct_qty = res.GetInt8(i + 15);
			recipe->products[stage] = rp;
			i++;
		}
		//Advance i past all the product info
		//i += 15;
	}
	LoadRecipeComponents();

	LogWrite(TRADESKILL__DEBUG, 0, "Recipes", "\tLoaded %u recipes", master_recipe_list.Size());
}

void WorldDatabase::LoadRecipeBooks(){
	Recipe *recipe;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;

	res = query.RunQuery2(Q_SELECT, "SELECT id, name, tradeskill_default_level FROM items WHERE item_type='Recipe'");
	if (res){
		while ((row = mysql_fetch_row(res))){
			recipe = new Recipe();
			recipe->SetBookID(atoul(row[0]));
			recipe->SetBookName(row[1]);
			recipe->SetLevel(atoi(row[2]));
			LogWrite(TRADESKILL__DEBUG, 5, "Recipes", "Loading Recipe Books: %s (%u)", recipe->GetBookName(), recipe->GetBookID());

			if (!master_recipebook_list.AddRecipeBook(recipe)){
				LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error adding Recipe Book '%s' - duplicate ID: %u", recipe->GetBookName(), recipe->GetBookID());
				safe_delete(recipe);
				continue;
			}
		}
	}
	LogWrite(TRADESKILL__DEBUG, 0, "Recipes", "\tLoaded %u Recipe Books ", master_recipebook_list.Size());
}

void WorldDatabase::LoadPlayerRecipes(Player *player){
	Recipe *recipe;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int16 total = 0;

	assert(player);

	res = query.RunQuery2(Q_SELECT, "SELECT recipe_id, highest_stage FROM character_recipes WHERE char_id = %u", player->GetCharacterID());
	if (res) {
		while ((row = mysql_fetch_row(res))){
			int32 recipe_id = atoul(row[0]);
			Recipe* master_recipe = master_recipe_list.GetRecipe(recipe_id);
			if(!master_recipe) {
				LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error adding recipe %u to player '%s' - duplicate ID", atoul(row[0]), player->GetName());
				continue;
			}
			recipe = new Recipe(master_recipe);
			recipe->SetHighestStage(atoi(row[1]));

			LogWrite(TRADESKILL__DEBUG, 5, "Recipes", "Loading recipe: %s (%u) for player: %s (%u)", recipe->GetName(), recipe->GetID(), player->GetName(), player->GetCharacterID());

			if (!player->GetRecipeList()->AddRecipe(recipe)){
				LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error adding recipe %u to player '%s' - duplicate ID", recipe->GetID(), player->GetName());
				safe_delete(recipe);
				continue;
			}
			total++;
		}
		LogWrite(TRADESKILL__DEBUG, 0, "Recipes", "Loaded %u recipes for player: %s (%u)", total, player->GetName(), player->GetCharacterID());
	}
}

int32 WorldDatabase::LoadPlayerRecipeBooks(int32 char_id, Player *player) {
	assert(player);

	LogWrite(TRADESKILL__DEBUG, 0, "Recipes", "Loading Character Recipe Books for player '%s' ...", player->GetName());
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int32 count = 0;
	int32 old_id = 0;
	int32 new_id = 0;
	Recipe* recipe;

	res = query.RunQuery2(Q_SELECT, "SELECT recipebook_id FROM character_recipe_books WHERE char_id = %u", char_id);
	if (res && mysql_num_rows(res) > 0) {
		while (res && (row = mysql_fetch_row(res))){
			count++;
			new_id = atoul(row[0]);
			if(new_id == old_id)
				continue;

			Item* item = master_item_list.GetItem(new_id);
			if (!item)
				continue;

			recipe = new Recipe();
			recipe->SetBookID(new_id);
			recipe->SetBookName(item->name.c_str());

			LogWrite(TRADESKILL__DEBUG, 5, "Recipes", "Loading Recipe Books: %s (%u)", recipe->GetBookName(), recipe->GetBookID());

			if (!player->GetRecipeBookList()->AddRecipeBook(recipe)) {
				LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error adding player Recipe Book '%s' - duplicate ID: %u", recipe->GetBookName(), recipe->GetBookID());
				safe_delete(recipe);
				continue;
			}
			old_id = new_id;
		}
	}
	return count;
}

void WorldDatabase::SavePlayerRecipeBook(Player* player, int32 recipebook_id){
	Query query;
	query.AddQueryAsync(player->GetCharacterID(), this, Q_INSERT, "INSERT INTO character_recipe_books (char_id, recipebook_id) VALUES(%u, %u)", player->GetCharacterID(), recipebook_id);
	//if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF)
		//LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error in SavePlayerRecipeBook query '%s' : %s", query.GetQuery(), query.GetError());
}

void WorldDatabase::SavePlayerRecipe(Player* player, int32 recipe_id) {
	Query query;
	query.AddQueryAsync(player->GetCharacterID(), this, Q_INSERT, "INSERT INTO character_recipes (char_id, recipe_id) VALUES (%u, %u)", player->GetCharacterID(), recipe_id);
	//if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF)
		//LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error in SavePlayerRecipeBook query '%s' : %s", query.GetQuery(), query.GetError());
}

void WorldDatabase::LoadRecipeComponents() {
	DatabaseResult res;
	bool status = database_new.Select(&res,
		"SELECT r.id, pc.item_id AS primary_comp, fc.item_id AS fuel_comp, sc.item_id as secondary_comp, rsc.`index` + 1 AS slot\n"
		"FROM recipe r\n"
		"INNER JOIN (select comp_list, item_id FROM recipe_comp_list_item ) as pc ON r.primary_comp_list = pc.comp_list\n"
		"INNER JOIN (select comp_list, item_id FROM recipe_comp_list_item ) as fc ON r.fuel_comp_list = fc.comp_list\n"
		"LEFT JOIN recipe_secondary_comp rsc ON rsc.recipe_id = r.id\n"
		"LEFT JOIN (select comp_list, item_id FROM recipe_comp_list_item) as sc ON rsc.comp_list = sc.comp_list\n"
		"WHERE r.bHaveAllProducts\n"
		"ORDER BY r.id, rsc.`index` ASC");

	if (!status) {
		return;
	}

	Recipe* recipe = 0;
	int32 id = 0;
	while (res.Next()) {
		int32 tmp = res.GetInt32(0);
		if (id != tmp) {
			id = tmp;
			recipe = master_recipe_list.GetRecipe(id);

			if (!recipe) {
				continue;
			}
		
		}
		if (recipe && !res.IsNull(3)) {
			if (find(recipe->components[0].begin(), recipe->components[0].end(), res.GetInt32(1)) == recipe->components[0].end())
				recipe->AddBuildComp(res.GetInt32(1), 0);
			if (find(recipe->components[5].begin(), recipe->components[5].end(), res.GetInt32(2)) == recipe->components[5].end())
				recipe->AddBuildComp(res.GetInt32(2), 5);
			if (find(recipe->components[res.GetInt8(4)].begin(), recipe->components[res.GetInt8(4)].end(), res.GetInt32(3)) == recipe->components[res.GetInt8(4)].end())
				recipe->AddBuildComp(res.GetInt32(3), res.GetInt8(4));
		}
		//else
			//LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Error loading `recipe_build_comps`, Recipe ID: %u", id);
	}
}

void WorldDatabase::UpdatePlayerRecipe(Player* player, int32 recipe_id, int8 highest_stage) {
	Query query;
	query.AddQueryAsync(player->GetCharacterID(), this, Q_UPDATE, "UPDATE `character_recipes` SET `highest_stage` = %i WHERE `char_id` = %u AND `recipe_id` = %u", highest_stage, player->GetCharacterID(), recipe_id);
}

/*
ALTER TABLE `character_recipes`
	ADD COLUMN `highest_stage` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER `recipe_id`;

*/