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
#include "Loot.h"
#include "../client.h"
#include "../../common/ConfigReader.h"
#include "../classes.h"
#include "../../common/debug.h"
#include "../zoneserver.h"
#include "../Skills.h"
#include "../classes.h"
#include "../World.h"
#include "../LuaInterface.h"
#include "../../common/Log.h"
#include "../Entity.h"
#include "../Rules/Rules.h"

extern Classes classes;
extern ConfigReader configReader;
extern MasterSkillList master_skill_list;
extern RuleManager rule_manager;

// If we want to transfer functions to this file then we should just do it, but for now we don't need all this commented code here

NPC* Entity::DropChest() {
	// Check to see if treasure chests are disabled in the rules
	if (rule_manager.GetGlobalRule(R_World, TreasureChestDisabled)->GetBool())
		return 0;
	
	if(GetChestDropTime()) {
		return 0; // this is a chest!  It doesn't drop itself!
	}

	NPC* chest = 0;

	chest = new NPC();
	chest->SetAttackable(0);
	chest->SetShowLevel(0);
	chest->SetShowName(1);
	chest->SetTargetable(1);
	chest->SetLevel(GetLevel());
	chest->SetChestDropTime();
	chest->SetTotalHP(100);
	chest->SetHP(100);
	chest->SetAlive(false);
	// Set the brain to a blank brain so it does nothing
	chest->SetBrain(new BlankBrain(chest));
	// Set the x, y, z, heading, location (grid id) to that of the dead spawn
	chest->SetZone(GetZone());
	// heading needs to be GetHeading() - 180 so the chest faces the proper way
	chest->SetHeading(GetHeading() - 180);
	// Set the primary command to loot and the secondary to disarm
	chest->AddPrimaryEntityCommand("loot", rule_manager.GetGlobalRule(R_Loot, LootRadius)->GetFloat(), "loot", "", 0, 0);
	chest->AddSecondaryEntityCommand("Disarm", rule_manager.GetGlobalRule(R_Loot, LootRadius)->GetFloat(), "Disarm", "", 0, 0);
	// 32 = loot icon for the mouse
	chest->SetIcon(32);
	// 1 = show the right click menu
	chest->SetShowCommandIcon(1);
	chest->SetLootMethod(this->GetLootMethod(), this->GetLootRarity(), this->GetLootGroupID());
	chest->SetLootName(this->GetName());
	int8 highest_tier = 0;
	vector<Item*>::iterator itr;	
	for (itr = ((Spawn*)this)->GetLootItems()->begin(); itr != ((Spawn*)this)->GetLootItems()->end(); ) {
		if ((*itr)->details.tier >= ITEM_TAG_COMMON && !(*itr)->IsBodyDrop()) {
			if ((*itr)->details.tier > highest_tier)
				highest_tier = (*itr)->details.tier;

			// Add the item to the chest
			chest->AddLootItem((*itr)->details.item_id, (*itr)->details.count);
			// Remove the item from the corpse
			itr = ((Spawn*)this)->GetLootItems()->erase(itr);
		}
		else
			itr++;
	}

	/*4034 = small chest | 5864 = treasure chest | 5865 = ornate treasure chest | 4015 = exquisite chest*/
	if (highest_tier >= ITEM_TAG_FABLED) {
		chest->SetModelType(4015); 
		chest->SetName("Exquisite Chest");
	}
	else if (highest_tier >= ITEM_TAG_LEGENDARY) {
		chest->SetModelType(5865);
		chest->SetName("Ornate Chest");
	}
	else if (highest_tier >= ITEM_TAG_TREASURED) {
		chest->SetModelType(5864);
		chest->SetName("Treasure Chest");
	}
	else if (highest_tier >= ITEM_TAG_COMMON) {
		chest->SetModelType(4034);
		chest->SetName("Small Chest");
	}
	else {
		safe_delete(chest);	
		chest = nullptr;
	}

	if (chest) {
		chest->SetID(Spawn::NextID());
		chest->SetShowHandIcon(1);
		chest->SetLocation(GetLocation());
		chest->SetX(GetX());
		chest->SetZ(GetZ());
		((Entity*)chest)->GetInfoStruct()->set_flying_type(false);
		chest->is_flying_creature = false;
		if(GetMap()) {
			auto loc = glm::vec3(GetX(), GetZ(), GetY());
			float new_z = FindBestZ(loc, nullptr);
			chest->appearance.pos.Y = new_z; // don't use SetY here can cause a loop
		}
		else {
			chest->appearance.pos.Y = GetY();
		}
	}

	return chest;
}