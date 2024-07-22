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
#include <assert.h>
#include "../../common/debug.h"
#include "../../common/Log.h"
#include "../../common/database.h"
#include "Recipe.h"
#include "../../common/ConfigReader.h"
#include "../Items/Items.h"
#include "../World.h"

extern ConfigReader configReader;
extern MasterItemList master_item_list;
extern World world;


Recipe::Recipe() {
	id = 0;
	book_id = 0;
	memset(name, 0, sizeof(name));
	memset(book_name, 0, sizeof(book_name));
	memset(book, 0, sizeof(book));
	memset(device, 0, sizeof(device));
	level = 0;
	tier = 0;
	icon = 0;
	skill = 0;
	technique = 0;
	knowledge = 0;
	classes = 0;
	unknown2 = 0;
	unknown3 = 0;
	unknown4 = 0;
}

Recipe::~Recipe() {
	map<int8, RecipeProducts*>::iterator itr;
	for (itr = products.begin(); itr != products.end(); itr++)
		safe_delete(itr->second);
}

Recipe::Recipe(Recipe *in){
	assert(in);
	id = in->GetID();
	soe_id = in->GetSoeID();
	book_id = in->GetBookID();
	strncpy(name, in->GetName(), sizeof(name));
	strncpy(description, in->GetDescription(), sizeof(description));
	strncpy(book_name, in->GetBookName(), sizeof(book_name));
	strncpy(book, in->GetBook(), sizeof(book));
	strncpy(device, in->GetDevice(), sizeof(device));
	
	level = in->GetLevel();
	tier = in->GetTier();
	icon = in->GetIcon();
	skill = in->GetSkill();
	technique = in->GetTechnique();
	knowledge = in->GetKnowledge();
	device_sub_type = in->GetDevice_Sub_Type();
	classes = in->GetClasses();
	unknown1 = in->GetUnknown1();
	unknown2 = in->GetUnknown2();
	unknown3 = in->GetUnknown3();
	unknown4 = in->GetUnknown4();
	
	product_item_id = in->GetProductID();
	strncpy(product_name, in->product_name, sizeof(product_name));
	product_qty = in->GetProductQuantity();
	
	strncpy(primary_build_comp_title, in->primary_build_comp_title, sizeof(primary_build_comp_title));
	strncpy(build1_comp_title, in->build1_comp_title, sizeof(build1_comp_title));
	strncpy(build2_comp_title, in->build2_comp_title, sizeof(build2_comp_title));
	strncpy(build3_comp_title, in->build3_comp_title, sizeof(build3_comp_title));
	strncpy(build4_comp_title, in->build4_comp_title, sizeof(build4_comp_title));
	
	strncpy(fuel_comp_title, in->fuel_comp_title, sizeof(fuel_comp_title));
	build1_comp_qty = in->GetBuild1ComponentQuantity();
	build2_comp_qty = in->GetBuild2ComponentQuantity();
	build3_comp_qty = in->GetBuild3ComponentQuantity();
	build4_comp_qty = in->GetBuild4ComponentQuantity();
	fuel_comp_qty = in->GetFuelComponentQuantity();
	primary_comp_qty = in->GetPrimaryComponentQuantity();
	highestStage = in->GetHighestStage();
	
	std::map<int8, RecipeProducts*>::iterator itr;
	for (itr = in->products.begin(); itr != in->products.end(); itr++) {
		RecipeProducts* rp = new RecipeProducts;
		rp->product_id = itr->second->product_id;
		rp->byproduct_id = itr->second->byproduct_id;
		rp->product_qty = itr->second->product_qty;
		rp->byproduct_qty = itr->second->byproduct_qty;
		products.insert(make_pair(itr->first, rp));
	}
	
	std::map<int8, vector<int32>>::iterator itr2;
	for (itr2 = in->components.begin(); itr2 != in->components.end(); itr2++) {
		std::vector<int32> recipe_component;
		 std::copy(itr2->second.begin(), itr2->second.end(),
              std::back_inserter(recipe_component));
		components.insert(make_pair(itr2->first, recipe_component));
	}
}

MasterRecipeList::MasterRecipeList() {
	m_recipes.SetName("MasterRecipeList::recipes");
}

MasterRecipeList::~MasterRecipeList() {
	ClearRecipes();
}

bool MasterRecipeList::AddRecipe(Recipe *recipe) {
	bool ret = false;
	int32 id;

	assert(recipe);

	id = recipe->GetID();
	m_recipes.writelock(__FUNCTION__, __LINE__);
	if (recipes.count(id) == 0) {
		recipes[id] = recipe;
		recipes_crc[recipe->GetSoeID()] = recipe;
		ret = true;
	}
	m_recipes.releasewritelock(__FUNCTION__, __LINE__);

	return ret;
}

Recipe* MasterRecipeList::GetRecipe(int32 recipe_id) {
	Recipe *ret = 0;

	m_recipes.readlock(__FUNCTION__, __LINE__);
	if (recipes.count(recipe_id) > 0)
		ret = recipes[recipe_id];
	m_recipes.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

Recipe* MasterRecipeList::GetRecipeByCRC(int32 recipe_crc) {
	Recipe *ret = 0;

	m_recipes.readlock(__FUNCTION__, __LINE__);
	if (recipes_crc.count(recipe_crc) > 0)
		ret = recipes_crc[recipe_crc];
	m_recipes.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

Recipe* MasterRecipeList::GetRecipeByName(const char* name) {
	Recipe* ret = 0;
	map<int32, Recipe*>::iterator itr;

	m_recipes.readlock(__FUNCTION__, __LINE__);
	for (itr = recipes.begin(); itr != recipes.end(); itr++) {
		if (::ToLower(string(name)) == ::ToLower(string(itr->second->GetName()))) {
			ret = itr->second;
			break;
		}
	}
	m_recipes.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

void MasterRecipeList::ClearRecipes() {
	map<int32, Recipe *>::iterator itr;

	m_recipes.writelock(__FUNCTION__, __LINE__);
	for (itr = recipes.begin(); itr != recipes.end(); itr++)
		safe_delete(itr->second);
	recipes.clear();
	recipes_crc.clear();
	m_recipes.releasewritelock(__FUNCTION__, __LINE__);
}

int32 MasterRecipeList::Size() {
	int32 ret;

	m_recipes.readlock(__FUNCTION__, __LINE__);
	ret = (int32)recipes.size();
	m_recipes.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

vector<Recipe*> MasterRecipeList::GetRecipes(const char* book_name) {
	vector<Recipe*> ret;
	map<int32, Recipe *>::iterator itr;

	m_recipes.writelock(__FUNCTION__, __LINE__);
	for (itr = recipes.begin(); itr != recipes.end(); itr++) {
		if (::ToLower(string(book_name)) == ::ToLower(string(itr->second->GetBook())))
			ret.push_back(itr->second);
	}
	m_recipes.releasewritelock(__FUNCTION__, __LINE__);

	return ret;
}

PlayerRecipeList::PlayerRecipeList(){
}

PlayerRecipeList::~PlayerRecipeList(){
	ClearRecipes();
}

bool PlayerRecipeList::AddRecipe(Recipe *recipe){
    std::unique_lock lock(player_recipe_mutex);
	assert(recipe);

	if(recipes.count(recipe->GetID()) == 0){
		recipes[recipe->GetID()] = recipe;
		return true;
	}
	return false;
}

Recipe * PlayerRecipeList::GetRecipe(int32 recipe_id){
    std::shared_lock lock(player_recipe_mutex);
	if (recipes.count(recipe_id) > 0)
		return recipes[recipe_id];
	return 0;
}

void PlayerRecipeList::ClearRecipes(){
    std::unique_lock lock(player_recipe_mutex);
	map<int32, Recipe *>::iterator itr;

	for (itr = recipes.begin(); itr != recipes.end(); itr++)
		safe_delete(itr->second);
	recipes.clear();
}

bool PlayerRecipeList::RemoveRecipe(int32 recipe_id) {
    std::unique_lock lock(player_recipe_mutex);
	bool ret = false;
	if (recipes.count(recipe_id) > 0) {
		recipes.erase(recipe_id);
		ret = true;
	}
	return ret;
}


int32 PlayerRecipeList::Size() {
    std::unique_lock lock(player_recipe_mutex);
	return recipes.size();
}

MasterRecipeBookList::MasterRecipeBookList(){
	m_recipeBooks.SetName("MasterRecipeBookList::recipeBooks");
}

MasterRecipeBookList::~MasterRecipeBookList(){
	ClearRecipeBooks();
}

bool MasterRecipeBookList::AddRecipeBook(Recipe *recipe){
	bool ret = false;
	int32 id = 0;

	assert(recipe);

	id = recipe->GetBookID();
	m_recipeBooks.writelock(__FUNCTION__, __LINE__);
	if(recipeBooks.count(id) == 0){
		recipeBooks[id] = recipe;
		ret = true;
	}
	m_recipeBooks.releasewritelock(__FUNCTION__, __LINE__);
	return ret;
}

Recipe * MasterRecipeBookList::GetRecipeBooks(int32 recipebook_id){
	Recipe *ret = 0;

	m_recipeBooks.readlock(__FUNCTION__, __LINE__);
	if (recipeBooks.count(recipebook_id) > 0)
		ret = recipeBooks[recipebook_id];
	m_recipeBooks.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

void MasterRecipeBookList::ClearRecipeBooks(){
	map<int32, Recipe *>::iterator itr;

	m_recipeBooks.writelock(__FUNCTION__, __LINE__);
	for (itr = recipeBooks.begin(); itr != recipeBooks.end(); itr++)
		safe_delete(itr->second);
	recipeBooks.clear();
	m_recipeBooks.releasewritelock(__FUNCTION__, __LINE__);
}

int32 MasterRecipeBookList::Size(){
	int32 ret = 0;

	m_recipeBooks.readlock(__FUNCTION__, __LINE__);
	ret = (int32)recipeBooks.size();
	m_recipeBooks.releasereadlock(__FUNCTION__, __LINE__);

	return ret;
}

EQ2Packet* MasterRecipeList::GetRecipePacket(int32 recipe_id, Client* client, bool display, int8 packet_type){
	Recipe *recipe = GetRecipe(recipe_id);
	if(recipe){
		LogWrite(TRADESKILL__DEBUG, 5, "Recipes", "Recipe ID: %u Recipe Name: %s", recipe->GetID(), recipe->GetName());
		return recipe->SerializeRecipe(client, recipe, display, packet_type);
	}
	return 0;
}

PlayerRecipeBookList::PlayerRecipeBookList(){
}

PlayerRecipeBookList::~PlayerRecipeBookList(){
	ClearRecipeBooks();
}

bool PlayerRecipeBookList::AddRecipeBook(Recipe *recipe){
	assert(recipe);

	if(recipeBooks.count(recipe->GetBookID()) == 0){
		recipeBooks[recipe->GetBookID()] = recipe;
		return true;
	}
	return false;
}

Recipe * PlayerRecipeBookList::GetRecipeBook(int32 recipebook_id){
	if(recipeBooks.count(recipebook_id) > 0)
		return recipeBooks[recipebook_id];
	return 0;
}

bool PlayerRecipeBookList::HasRecipeBook(int32 book_id) {
	if (recipeBooks.count(book_id) > 0)
		return true;
	return false;
}

void PlayerRecipeBookList::ClearRecipeBooks(){
	map<int32, Recipe*>::iterator itr;

	for(itr = recipeBooks.begin(); itr != recipeBooks.end(); itr++)
		safe_delete(itr->second);
	recipeBooks.clear();
}

EQ2Packet * Recipe::SerializeRecipe(Client *client, Recipe *recipe, bool display, int8 packet_type, int8 subpacket_type, const char *struct_name){
	int16 version = 1;
	Item* item = 0;
	RecipeProducts* rp = 0;
	vector<int32>::iterator itr;
	vector<RecipeComp> comp_list;
	
	int8 i = 0;
	int32 firstID = 0;
	int32 primary_comp_id = 0;
	if(client)
		version = client->GetVersion();
	if(!struct_name)
		struct_name = "WS_ExamineRecipeInfo";
	PacketStruct *packet = configReader.getStruct(struct_name, version);
	if(display)
		packet->setSubstructDataByName("info_header", "show_name", 1);
	else
		packet->setSubstructDataByName("info_header", "show_popup", 1);
	
	if(client->GetVersion() <= 561) {
		packet->setSubstructDataByName("info_header", "packettype", 0x02);
	}
	else if(packet_type > 0)
		packet->setSubstructDataByName("info_header", "packettype", GetItemPacketType(packet->GetVersion()));
	else
		if(version == 1096)
			packet->setSubstructDataByName("info_header", "packettype",0x35FE);
		if (version == 1208)
			packet->setSubstructDataByName("info_header", "packettype", 0x45FE);
		if(version >= 57048)
			packet->setSubstructDataByName("info_header", "packettype",0x48FE);
	if(subpacket_type == 0)
		subpacket_type = 0x02;
	packet->setSubstructDataByName("info_header", "packetsubtype", subpacket_type);

	packet->setSubstructDataByName("recipe_info", "id", recipe->GetID());
	packet->setSubstructDataByName("recipe_info", "unknown", 3);
	packet->setSubstructDataByName("recipe_info", "level", recipe->GetLevel());
	packet->setSubstructDataByName("recipe_info", "technique", recipe->GetTechnique());
	packet->setSubstructDataByName("recipe_info", "skill_level", 50); //50
	packet->setSubstructDataByName("recipe_info", "knowledge", recipe->GetKnowledge());
	packet->setSubstructDataByName("recipe_info", "device", recipe->GetDevice());
	packet->setSubstructDataByName("recipe_info", "icon", recipe->GetIcon());
	packet->setSubstructDataByName("recipe_info", "unknown3", 1);
	packet->setSubstructDataByName("recipe_info", "adventure_id", 0xFF);
	packet->setSubstructDataByName("recipe_info", "tradeskill_id", client ? client->GetPlayer()->GetTradeskillClass() : 0);
	packet->setSubstructDataByName("recipe_info", "unknown4a", 100);
	packet->setSubstructDataByName("recipe_info", "unknown4aa", 1);
	packet->setSubstructDataByName("recipe_info", "unknown5a", 20);//level *10
	packet->setSubstructDataByName("recipe_info", "product_classes", recipe->GetClasses());
	int32 HS = 0;
	if (client->GetPlayer()->GetRecipeList()->GetRecipe(recipe->GetID()) == NULL)
		HS = 0;
	else 
		HS = client->GetPlayer()->GetRecipeList()->GetRecipe(recipe->GetID())->highestStage;

	 
	packet->setSubstructDataByName("recipe_info", "show_previous", HS);//     recipe->highestStage);
	

	rp = recipe->products[1];
	if (rp->product_id > 0) {
		item = 0;
		item = master_item_list.GetItem(rp->product_id);
		if (item) {
			packet->setSubstructDataByName("recipe_info", "previous1_icon", item->GetIcon(client->GetVersion()));
			packet->setSubstructDataByName("recipe_info", "previous1_name", "previous1_name");
			packet->setSubstructDataByName("recipe_info", "previous1_qty", recipe->products[1]->product_qty);
			packet->setSubstructDataByName("recipe_info", "previous1_item_id", recipe->products[1]->product_id);
			packet->setSubstructDataByName("recipe_info", "previous1_item_crc", -853046774);
			packet->setSubstructDataByName("recipe_info", "firstbar_icon", item->GetIcon(client->GetVersion()));
			packet->setSubstructDataByName("recipe_info", "firstbar_name", "firstbar_name");
			packet->setSubstructDataByName("recipe_info", "firstbar_qty", recipe->products[1]->product_qty);
			packet->setSubstructDataByName("recipe_info", "firstbar_item_id", recipe->products[2]->product_id);
			packet->setSubstructDataByName("recipe_info", "firstbar_item_crc", -853046774);
		}
	}
	rp = recipe->products[2];
	if (rp->product_id > 0) {
		item = 0;
		item = master_item_list.GetItem(rp->product_id);
		if (item) {
			packet->setSubstructDataByName("recipe_info", "previous2_icon", item->GetIcon(client->GetVersion()));
			packet->setSubstructDataByName("recipe_info", "previous2_name", "previous2_name");
			packet->setSubstructDataByName("recipe_info", "previous2_qty", recipe->products[2]->product_qty);
			packet->setSubstructDataByName("recipe_info", "previous2_item_id", recipe->products[2]->product_id);
			packet->setSubstructDataByName("recipe_info", "previous2_item_crc", -853046774);
			packet->setSubstructDataByName("recipe_info", "secondbar_icon", item->GetIcon(client->GetVersion()));
			packet->setSubstructDataByName("recipe_info", "secondbar_name", "secondbar_name");
			packet->setSubstructDataByName("recipe_info", "secondbar_qty", recipe->products[2]->product_qty);
			packet->setSubstructDataByName("recipe_info", "secondbar_item_id", recipe->products[2]->product_id);
			packet->setSubstructDataByName("recipe_info", "secondbar_item_crc", -853046774);
		}
	}
	rp = recipe->products[3];
	if (rp->product_id > 0) {
		item = 0;
		item = master_item_list.GetItem(rp->product_id);
		if (item) {
			packet->setSubstructDataByName("recipe_info", "previous3_icon", item->GetIcon(client->GetVersion()));
			packet->setSubstructDataByName("recipe_info", "previous3_name", "previous3_name");
			packet->setSubstructDataByName("recipe_info", "previous3_qty", recipe->products[3]->product_qty);
			packet->setSubstructDataByName("recipe_info", "previous3_item_id", recipe->products[3]->product_id);
			packet->setSubstructDataByName("recipe_info", "previous3_item_crc", -853046774);
			packet->setSubstructDataByName("recipe_info", "thirdbar_icon", item->GetIcon(client->GetVersion()));
			packet->setSubstructDataByName("recipe_info", "thirdbar_name", "thirdbar_name");
			packet->setSubstructDataByName("recipe_info", "thirdbar_qty", recipe->products[3]->product_qty);
			packet->setSubstructDataByName("recipe_info", "thirdbar_item_id", recipe->products[3]->product_id);
			packet->setSubstructDataByName("recipe_info", "thirdbar_item_crc", -2065846136);
		}
	}
	
	
	
	
	//item = master_item_list.GetItemByName(recipe->GetName());// TODO: MJ we should be getting item by item number in case of multiple items with same name
	item = master_item_list.GetItem(recipe->GetProductID());
	if(item) {
		packet->setSubstructDataByName("recipe_info", "product_icon", item->GetIcon(client->GetVersion())); //item->details.icon);
		packet->setSubstructDataByName("recipe_info", "product_name", item->name.c_str()); //item->name);
		packet->setSubstructDataByName("recipe_info", "product_qty", 1);
		packet->setSubstructDataByName("recipe_info", "product_item_id", item->details.item_id); //item->details.item_id);
		packet->setSubstructDataByName("recipe_info", "product_item_crc", 0); //item->details.item_crc);
	}

	rp = recipe->products[0];
	if (rp->byproduct_id > 0) {
		item = 0;
		item = master_item_list.GetItem(rp->byproduct_id);
		if (item) {
			packet->setSubstructDataByName("recipe_info", "byproduct_icon", item->GetIcon(client->GetVersion()));//11
			packet->setSubstructDataByName("recipe_info", "byproduct_id", item->details.item_id);
		}

	}
	rp = recipe->products[1];
	if (rp->product_id > 0) {
		item = 0;
		item = master_item_list.GetItem(rp->product_id);
		if (item) {
			packet->setSubstructDataByName("recipe_info", "byproduct_icon", item->GetIcon(client->GetVersion()));//11
			packet->setSubstructDataByName("recipe_info", "byproduct_id", item->details.item_id);
		}

	}
		
	item = 0;

	// Check to see if we have a primary component (slot = 0)
	vector<Item*> itemss;
	if (recipe->components.count(0) > 0) {
		if(client->GetVersion() <= 561) {
			packet->setSubstructDataByName("recipe_info", "primary_count", 1);
		}	
		
		int16 have_qty = 0;
		vector<int32> rc = recipe->components[0];
		for (itr = rc.begin(); itr != rc.end(); itr++, i++) {
			item = master_item_list.GetItem(*itr);
			packet->setSubstructDataByName("recipe_info", "primary_comp", recipe->primary_build_comp_title);
			packet->setSubstructDataByName("recipe_info", "primary_qty", recipe->GetPrimaryComponentQuantity());
			item = 0;
			itemss = client->GetPlayer()->item_list.GetAllItemsFromID((*itr));
			if (itemss.size() > 0) {
				int16 needed_qty = recipe->GetPrimaryComponentQuantity();
				for (int8 i = 0; i < itemss.size(); i++) {
					have_qty += itemss[i]->details.count;
				}
			}
		}
		packet->setSubstructDataByName("recipe_info", "primary_qty_avail", have_qty);
	}
	
	

	int8 total_build_components = recipe->GetTotalBuildComponents();

	int8 index = 0;
	int8 count = 0;
	if (total_build_components > 0) {
		packet->setSubstructArrayLengthByName("recipe_info", "num_comps", total_build_components);
		for (index = 0; index < 4; index++) {
			if (recipe->components.count(index + 1) == 0)
				continue;
			
			count++;
			vector<int32> rc = recipe->components[index + 1];
			int16 have_qty = 0;
			string comp_title;
			int8 comp_qty;
			for (itr = rc.begin(); itr != rc.end(); itr++, i++) {
				if (index == 0) {
					comp_title = recipe->build1_comp_title;
					comp_qty = recipe->build1_comp_qty;
				}
				else if (index == 1) {
					comp_title = recipe->build2_comp_title;
					comp_qty = recipe->build2_comp_qty;
				}
				else if (index == 2) {
					comp_title = recipe->build3_comp_title;
					comp_qty = recipe->build3_comp_qty;
				}
				else if (index == 3) {
					comp_title = recipe->build4_comp_title;
					comp_qty = recipe->build4_comp_qty;
				}
				itemss = client->GetPlayer()->item_list.GetAllItemsFromID((*itr));
				for (int8 j = 0; j < itemss.size(); j++) {
					have_qty += itemss[j]->details.count;
				}
			}
				packet->setArrayDataByName("build_comp", comp_title.c_str(), index);
				packet->setArrayDataByName("build_comp_qty", comp_qty, index);
				packet->setArrayDataByName("build_comp_qty_avail", have_qty, index);
		}
		
	}
	
	if(client->GetVersion() <= 561) {
		packet->setSubstructDataByName("recipe_info", "fuel_count", 1);
		packet->setSubstructDataByName("recipe_info", "fuel_comp", recipe->fuel_comp_title);
		packet->setSubstructDataByName("recipe_info", "fuel_comp_qty", recipe->fuel_comp_qty);
	}	
	// Check to see if we have a fuel component (slot = 5)
	else if (recipe->components.count(5) > 0) {
		vector<int32> rc = recipe->components[5];
		for (itr = rc.begin(); itr != rc.end(); itr++, i++) {
			item = master_item_list.GetItem(*itr);
			packet->setSubstructDataByName("recipe_info", "fuel_comp", recipe->fuel_comp_title);
			packet->setSubstructDataByName("recipe_info", "fuel_comp_qty", recipe->fuel_comp_qty);
			item = 0;
			itemss = client->GetPlayer()->item_list.GetAllItemsFromID((*itr));
			if (itemss.size() > 0) {
				int16 have_qty = 0;
				for (int8 i = 0; i < itemss.size(); i++) {
					have_qty += itemss[i]->details.count;
				}
				packet->setSubstructDataByName("recipe_info", "fuel_comp_qty_avail", have_qty);
				break;
			}
		}
	}
	packet->setSubstructDataByName("recipe_info", "build_comp_qty_avail_flag", 1);
	packet->setSubstructDataByName("recipe_info", "unknown6", 4, 0);
	packet->setSubstructDataByName("recipe_info", "min_product", 1);
	packet->setSubstructDataByName("recipe_info", "max_product", 1);
	packet->setSubstructDataByName("recipe_info", "available_flag", 4);
	packet->setSubstructDataByName("recipe_info", "not_commissionable", 1);
	packet->setSubstructDataByName("recipe_info", "recipe_name", recipe->GetName());
	packet->setSubstructDataByName("recipe_info", "recipe_description", recipe->GetDescription());
	//packet->PrintPacket();
	EQ2Packet* data = packet->serialize();
	EQ2Packet* app = new EQ2Packet(OP_ClientCmdMsg, data->pBuffer, data->size);
	safe_delete(packet);
	safe_delete(data);
	//DumpPacket(app);
	return app;
}

void Recipe::AddBuildComp(int32 itemID, int8 slot, bool preffered) {
	if (preffered)
		components[slot].insert(components[slot].begin(), itemID);
	else
		components[slot].push_back(itemID);
}

int8 Recipe::GetTotalBuildComponents() {
	int8 total_build_components = 0;
	for(int i=1;i<=4;i++) {
		if (components.count(i) > 0)
		total_build_components++;
	}
	return total_build_components;
}

bool Recipe::ProvidedAllRequiredComponents(Client* client, vector<Item*>* player_components, vector<pair<int32,int16>>* player_component_pair_qty) {
	vector<int32>::iterator itr;
	std::vector<pair<int32,int16>> player_comp_itr;
	
	// Check to see if we have a fuel component (slot = 5)
	bool matched = false;
	bool hasfuel = false;
	if (components.count(5) > 0) {
		vector<int32> rc = components[5];
		for (itr = rc.begin(); itr != rc.end(); itr++) {
			hasfuel = true;
			LogWrite(TRADESKILL__INFO, 5, "Recipes", "Recipe ID: %u Recipe Name: %s, item %s (%u), fuel quantity required %u", GetID(), GetName(), fuel_comp_title, (*itr), fuel_comp_qty);
			if(PlayerHasComponentByItemID(client, player_components, player_component_pair_qty, (*itr), fuel_comp_qty)) {
				matched = true;
				break;
			}
		}
	}
	if(hasfuel && !matched) {
		LogWrite(TRADESKILL__ERROR, 0, "Recipes", "Recipe ID: %u Recipe Name: %s, item %s (%u), lacking fuel quantity required %u", GetID(), GetName(), fuel_comp_title, (*itr), fuel_comp_qty);
		 return false;
	}
	
	for (int8 index = 0; index < GetTotalBuildComponents(); index++) {
		if (components.count(index + 1) == 0)
			continue;
		
		vector<int32> rc = components[index + 1];
		string comp_title;
		int8 comp_qty;
		matched = false;
		for (itr = rc.begin(); itr != rc.end(); itr++) {
				if (index == 0) {
					comp_title = build1_comp_title;
					comp_qty = build1_comp_qty;
				}
				else if (index == 1) {
					comp_title = build2_comp_title;
					comp_qty = build2_comp_qty;
				}
				else if (index == 2) {
					comp_title = build3_comp_title;
					comp_qty = build3_comp_qty;
				}
				else if (index == 3) {
					comp_title = build4_comp_title;
					comp_qty = build4_comp_qty;
				}
			LogWrite(TRADESKILL__INFO, 5, "Recipes", "Recipe ID: %u Recipe Name: %s, item %s (%u), index %u quantity required %u", GetID(), GetName(), comp_title.c_str(), (*itr), index, comp_qty);
			if(PlayerHasComponentByItemID(client, player_components, player_component_pair_qty, (*itr), comp_qty)) {
				matched = true;
				break;
			}
		}
		if(!matched) {
			return false;
		}
	}
	return true;
}

bool Recipe::PlayerHasComponentByItemID(Client* client, vector<Item*>* player_components, vector<pair<int32,int16>>* player_component_pair_qty, int32 item_id, int8 required_qty) {
	vector<Item*>::iterator itr;
	int16 have_qty = 0;
	for(itr = player_components->begin(); itr != player_components->end(); itr++) {
		LogWrite(TRADESKILL__DEBUG, 0, "Recipes", "PlayerHasComponentByItemID %u to match %u, qty %u, qtyreq: %u", (*itr)->details.item_id, item_id, (*itr)->details.count, required_qty);
		if((*itr) && (*itr)->details.item_id == item_id && (*itr)->details.count >= required_qty) {
			return true;
		}
	}
	
	vector<Item*> itemss = client->GetPlayer()->item_list.GetAllItemsFromID(item_id);
	if (itemss.size() > 0) {
		for (int8 i = 0; i < itemss.size(); i++) {
			have_qty += itemss[i]->details.count;
		}
	}
	
	int16 track_req_qty = required_qty;
	if(have_qty >= required_qty) {
		LogWrite(TRADESKILL__DEBUG, 0, "Recipes", "PlayerHasComponentByItemID OVERRIDE! Inventory has item id %u, more than required for quantity %u, have %u", item_id, required_qty, have_qty);
		have_qty = 0;
		for (int8 i = 0; i < itemss.size(); i++) {
			have_qty += itemss[i]->details.count;
			int8 cur_qty = itemss[i]->details.count;
			if(cur_qty > track_req_qty)
				cur_qty = track_req_qty;
			
			track_req_qty -= cur_qty;
			itemss[i]->details.item_locked = true;
			player_component_pair_qty->push_back({itemss[i]->details.unique_id, cur_qty});
			player_components->push_back(itemss[i]);
			if(have_qty >= required_qty)
				break;
		}
		return true;
	}
	
	return false;
}

int8 Recipe::GetItemRequiredQuantity(int32 item_id) {
	vector<int32>::iterator itr;
	int8 comp_qty = 0, qty = 0;
	for (int8 index = 0; index < GetTotalBuildComponents(); index++) {
		if (components.count(index + 1) == 0)
			continue;
		
		vector<int32> rc = components[index + 1];
		string comp_title;
		int8 comp_qty;
		bool matched = false;
		for (itr = rc.begin(); itr != rc.end(); itr++) {
				if (index == 0) {
					comp_qty = build1_comp_qty;
				}
				else if (index == 1) {
					comp_qty = build2_comp_qty;
				}
				else if (index == 2) {
					comp_qty = build3_comp_qty;
				}
				else if (index == 3) {
					comp_qty = build4_comp_qty;
				}
			if((*itr) == item_id)
				qty += comp_qty;
		}
	}
	return qty;
}