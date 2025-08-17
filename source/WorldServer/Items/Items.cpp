/*  
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2005 - 2026  EQ2EMulator Development Team (http://www.eq2emu.com formerly http://www.eq2emulator.net)

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

#include "Items.h"
#include "../Spells.h"
#include "../Quests.h"
#include "../Player.h"
#include "../classes.h"
#include "math.h"
#include "../World.h"
#include "../LuaInterface.h"
#include "../../common/Log.h"
#include "../Entity.h"
#include "../Recipes/Recipe.h"
#include <algorithm>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "../Rules/Rules.h"
#include "../WorldDatabase.h"
#include "../Broker/BrokerManager.h"

extern World world;
extern MasterSpellList master_spell_list;
extern MasterQuestList master_quest_list;
extern MasterRecipeList master_recipe_list;
extern ConfigReader configReader;
extern LuaInterface* lua_interface;
extern RuleManager rule_manager;
extern Classes classes;
extern MasterItemList master_item_list;
extern WorldDatabase database;
extern BrokerManager broker;

MasterItemList::MasterItemList(){
	AddMappedItemStat(ITEM_STAT_ADORNING, std::string("adorning"));
	AddMappedItemStat(ITEM_STAT_AGGRESSION, std::string("aggression"));
	AddMappedItemStat(ITEM_STAT_ARTIFICING, std::string("artificing"));
	AddMappedItemStat(ITEM_STAT_ARTISTRY, std::string("artistry"));
	AddMappedItemStat(ITEM_STAT_CHEMISTRY, std::string("chemistry"));
	AddMappedItemStat(ITEM_STAT_CRUSHING, std::string("crushing"));
	AddMappedItemStat(ITEM_STAT_DEFENSE, std::string("defense"));
	AddMappedItemStat(ITEM_STAT_DEFLECTION, std::string("deflection"));
	AddMappedItemStat(ITEM_STAT_DISRUPTION, std::string("disruption"));
	AddMappedItemStat(ITEM_STAT_FISHING, std::string("fishing"));
	AddMappedItemStat(ITEM_STAT_FLETCHING, std::string("fletching"));
	AddMappedItemStat(ITEM_STAT_FOCUS, std::string("focus"));
	AddMappedItemStat(ITEM_STAT_FORESTING, std::string("foresting"));
	AddMappedItemStat(ITEM_STAT_GATHERING, std::string("gathering"));
	AddMappedItemStat(ITEM_STAT_METAL_SHAPING, std::string("metal shaping"));
	AddMappedItemStat(ITEM_STAT_METALWORKING, std::string("metalworking"));
	AddMappedItemStat(ITEM_STAT_MINING, std::string("mining"));
	AddMappedItemStat(ITEM_STAT_MINISTRATION, std::string("ministration"));
	AddMappedItemStat(ITEM_STAT_ORDINATION, std::string("ordination"));
	AddMappedItemStat(ITEM_STAT_ADORNING, std::string("adorning"));
	AddMappedItemStat(ITEM_STAT_PARRY, std::string("parry"));
	AddMappedItemStat(ITEM_STAT_PIERCING, std::string("piercing"));
	AddMappedItemStat(ITEM_STAT_RANGED, std::string("ranged"));
	AddMappedItemStat(ITEM_STAT_SAFE_FALL, std::string("safe fall"));
	AddMappedItemStat(ITEM_STAT_SCRIBING, std::string("scribing"));
	AddMappedItemStat(ITEM_STAT_SCULPTING, std::string("sculpting"));
	AddMappedItemStat(ITEM_STAT_SLASHING, std::string("slashing"));
	AddMappedItemStat(ITEM_STAT_SUBJUGATION, std::string("subjugation"));
	AddMappedItemStat(ITEM_STAT_SWIMMING, std::string("swimming"));
	AddMappedItemStat(ITEM_STAT_TAILORING, std::string("tailoring"));
	AddMappedItemStat(ITEM_STAT_TINKERING, std::string("tinkering"));
	AddMappedItemStat(ITEM_STAT_TRANSMUTING, std::string("transmuting"));
	AddMappedItemStat(ITEM_STAT_TRAPPING, std::string("trapping"));
	AddMappedItemStat(ITEM_STAT_WEAPON_SKILLS, std::string("weapon skills"));
	AddMappedItemStat(ITEM_STAT_POWER_COST_REDUCTION, std::string("power cost reduction"));
	AddMappedItemStat(ITEM_STAT_SPELL_AVOIDANCE, std::string("spell avoidance"));
}

void MasterItemList::AddMappedItemStat(int32 id, std::string lower_case_name)
{
	mappedItemStatsStrings[lower_case_name] = id;
	mappedItemStatTypeIDs[id] = lower_case_name;
}

MasterItemList::~MasterItemList(){
	RemoveAll();
	
	map<VersionRange*, map<int64,int64>>::iterator itr;
	for (itr = broker_item_map.begin(); itr != broker_item_map.end(); itr++)
	{
		VersionRange* range = itr->first;
		delete range;
	}

	broker_item_map.clear();
}


void MasterItemList::AddBrokerItemMapRange(int32 min_version, int32 max_version,
	int64 client_bitmask, int64 server_bitmask)
{
	map<VersionRange*, map<int64,int64>>::iterator itr = FindBrokerItemMapVersionRange(min_version, max_version);
	if (itr != broker_item_map.end()) {
		itr->second.insert(make_pair(client_bitmask, server_bitmask));
		return;
	}

	VersionRange* range = new VersionRange(min_version, max_version);
	broker_item_map[range][client_bitmask] = server_bitmask;
}

map<VersionRange*, map<int64,int64>>::iterator MasterItemList::FindBrokerItemMapVersionRange(int32 min_version, int32 max_version)
{
	map<VersionRange*, map<int64,int64>>::iterator itr;
	for (itr = broker_item_map.begin(); itr != broker_item_map.end(); itr++)
	{
		VersionRange* range = itr->first;
		// if min and max version are both in range
		if (range->GetMinVersion() <= min_version && max_version <= range->GetMaxVersion())
			return itr;
		// if the min version is in range, but max range is 0
		else if (range->GetMinVersion() <= min_version && range->GetMaxVersion() == 0)
			return itr;
		// if min version is 0 and max_version has a cap
		else if (range->GetMinVersion() == 0 && max_version <= range->GetMaxVersion())
			return itr;
	}

	return broker_item_map.end();
}

map<VersionRange*, map<int64,int64>>::iterator MasterItemList::FindBrokerItemMapByVersion(int32 version)
{
	map<VersionRange*, map<int64,int64>>::iterator enditr = broker_item_map.end();
	map<VersionRange*, map<int64,int64>>::iterator itr;
	for (itr = broker_item_map.begin(); itr != broker_item_map.end(); itr++)
	{
		VersionRange* range = itr->first;
		// if min and max version are both in range
		if(range->GetMinVersion() == 0 && range->GetMaxVersion() == 0)
			enditr = itr;
		else if (version >= range->GetMinVersion() && version <= range->GetMaxVersion())
			return itr;
	}

	return enditr;
}

bool MasterItemList::ShouldAddItemBrokerType(Item* item, int64 itype) {
	bool should_add = false;
	switch(itype){
		case ITEM_BROKER_TYPE_ADORNMENT:{
			if(item->IsAdornment())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_AMMO:{
			if(item->IsAmmo())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_ATTUNEABLE:{
			if(item->CheckFlag(ATTUNEABLE))
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_BAG:{
			if(item->IsBag())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_BAUBLE:{
			if(item->IsBauble())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_BOOK:{
			if(item->IsBook())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_CHAINARMOR:{
			if(item->IsChainArmor())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_CLOAK:{
			if(item->IsCloak())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_CLOTHARMOR:{
			if(item->IsClothArmor())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_COLLECTABLE:{
			if(item->IsCollectable())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_CRUSHWEAPON:{
			if(item->IsCrushWeapon() && (item->weapon_info->wield_type == ITEM_WIELD_TYPE_DUAL || item->weapon_info->wield_type == ITEM_WIELD_TYPE_SINGLE))
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_DRINK:{
			if(item->IsFoodDrink())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_FOOD:{
			if(item->IsFoodFood())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_HOUSEITEM:{
			if(item->IsHouseItem() || item->IsHouseContainer())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_JEWELRY:{
			if(item->IsJewelry())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_LEATHERARMOR:{
			if(item->IsLeatherArmor())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_LORE:{
			if(item->CheckFlag(LORE))
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_MISC:{
			if(item->IsMisc())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_PIERCEWEAPON:{
			if(item->IsPierceWeapon() && (item->weapon_info->wield_type == ITEM_WIELD_TYPE_DUAL || item->weapon_info->wield_type == ITEM_WIELD_TYPE_SINGLE))
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_PLATEARMOR:{
			if(item->IsPlateArmor())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_POISON:{
			if(item->IsPoison())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_POTION:{
			if(item->IsPotion())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_RECIPEBOOK:{
			if(item->IsRecipeBook())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_SALESDISPLAY:{
			if(item->IsSalesDisplay())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_SHIELD:{
			if(item->IsShield())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_SLASHWEAPON:{
			if(item->IsSlashWeapon() && (item->weapon_info->wield_type == ITEM_WIELD_TYPE_DUAL || item->weapon_info->wield_type == ITEM_WIELD_TYPE_SINGLE))
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_SPELLSCROLL:{
			if(item->IsSpellScroll())
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_TINKERED:{
			if(item->tinkered == 1)
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_TRADESKILL:{
			if(item->crafted == 1)
				should_add = true;
			break;
		}
		case ITEM_BROKER_TYPE_2H_CRUSH:{
			should_add = item->IsWeapon() && item->weapon_info->wield_type == ITEM_WIELD_TYPE_TWO_HAND && item->generic_info.skill_req1 == SKILL_ID_STAFF;
			break;
		}
		case ITEM_BROKER_TYPE_2H_PIERCE:{
			should_add = item->IsWeapon() && item->weapon_info->wield_type == ITEM_WIELD_TYPE_TWO_HAND && item->generic_info.skill_req1 == SKILL_ID_GREATSPEAR;
			break;
		}
		case ITEM_BROKER_TYPE_2H_SLASH:{
			should_add = item->IsWeapon() && item->weapon_info->wield_type == ITEM_WIELD_TYPE_TWO_HAND && item->generic_info.skill_req1 == SKILL_ID_GREATSWORD;
			break;
		}
	}
	return should_add;
}
bool MasterItemList::ShouldAddItemBrokerSlot(Item* item, int64 ltype) {
	bool should_add = false;

	switch(ltype){
		case ITEM_BROKER_SLOT_AMMO:{
			should_add = item->HasSlot(EQ2_AMMO_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_CHARM:{
			should_add = item->HasSlot(EQ2_CHARM_SLOT_1, EQ2_CHARM_SLOT_2);
			break;
		}
		case ITEM_BROKER_SLOT_CHEST:{
			should_add = item->HasSlot(EQ2_CHEST_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_CLOAK:{
			should_add = item->HasSlot(EQ2_CLOAK_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_DRINK:{
			should_add = item->HasSlot(EQ2_DRINK_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_EARS:{
			should_add = item->HasSlot(EQ2_EARS_SLOT_1, EQ2_EARS_SLOT_2);
			break;
		}
		case ITEM_BROKER_SLOT_FEET:{
			should_add = item->HasSlot(EQ2_FEET_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_FOOD:{
			should_add = item->HasSlot(EQ2_FOOD_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_FOREARMS:{
			should_add = item->HasSlot(EQ2_FOREARMS_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_HANDS:{
			should_add = item->HasSlot(EQ2_HANDS_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_HEAD:{
			should_add = item->HasSlot(EQ2_HEAD_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_LEGS:{
			should_add = item->HasSlot(EQ2_LEGS_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_NECK:{
			should_add = item->HasSlot(EQ2_NECK_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_PRIMARY:{
			should_add = item->HasSlot(EQ2_PRIMARY_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_PRIMARY_2H:{
			should_add = item->HasSlot(EQ2_PRIMARY_SLOT) && item->IsWeapon() && item->weapon_info->wield_type == ITEM_WIELD_TYPE_TWO_HAND;
			break;
		}
		case ITEM_BROKER_SLOT_RANGE_WEAPON:{
			should_add = item->HasSlot(EQ2_RANGE_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_RING:{
			should_add = item->HasSlot(EQ2_LRING_SLOT, EQ2_RRING_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_SECONDARY:{
			should_add = item->HasSlot(EQ2_SECONDARY_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_SHOULDERS:{
			should_add = item->HasSlot(EQ2_SHOULDERS_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_WAIST:{
			should_add = item->HasSlot(EQ2_WAIST_SLOT);
			break;
		}
		case ITEM_BROKER_SLOT_WRIST:{
			should_add = item->HasSlot(EQ2_LWRIST_SLOT, EQ2_RWRIST_SLOT);
			break;
		}
	}
	
	return should_add;
}

bool MasterItemList::ShouldAddItemBrokerStat(Item* item, int64 btype) {
	bool should_add = false;
	bool stat_found = false;
	switch(btype){
		case ITEM_BROKER_STAT_TYPE_NONE:{
			if (item->item_stats.size() == 0)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_DEF:{
			stat_found = item->HasStat(ITEM_STAT_DEFENSE, GetItemStatNameByID(ITEM_STAT_DEFENSE));
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_STR:{
			stat_found = item->HasStat(ITEM_STAT_STR);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_STA:{
			stat_found = item->HasStat(ITEM_STAT_STA);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_AGI:{
			stat_found = item->HasStat(ITEM_STAT_AGI);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_WIS:{
			stat_found = item->HasStat(ITEM_STAT_WIS);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_INT:{
			stat_found = item->HasStat(ITEM_STAT_INT);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_HEALTH:{
			stat_found = item->HasStat(ITEM_STAT_HEALTH);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_POWER:{
			stat_found = item->HasStat(ITEM_STAT_POWER);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_HEAT:{
			stat_found = item->HasStat(ITEM_STAT_VS_HEAT);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_COLD:{
			stat_found = item->HasStat(ITEM_STAT_VS_COLD);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_MAGIC:{
			stat_found = item->HasStat(ITEM_STAT_VS_MAGIC);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_MENTAL:{
			stat_found = item->HasStat(ITEM_STAT_VS_MENTAL);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_DIVINE:{
			stat_found = item->HasStat(ITEM_STAT_VS_DIVINE);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_POISON:{
			stat_found = item->HasStat(ITEM_STAT_VS_POISON);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_DISEASE:{
			stat_found = item->HasStat(ITEM_STAT_VS_DISEASE);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_CRUSH:{
			stat_found = item->HasStat(ITEM_STAT_DMG_CRUSH);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_SLASH:{
			stat_found = item->HasStat(ITEM_STAT_DMG_SLASH);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_PIERCE:{
			stat_found = item->HasStat(ITEM_STAT_DMG_PIERCE);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_CRITICAL: {
			stat_found = item->HasStat(ITEM_STAT_CRITICALMITIGATION);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_DBL_ATTACK:{
			stat_found = item->HasStat(ITEM_STAT_MULTIATTACKCHANCE);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_ABILITY_MOD:{
			stat_found = item->HasStat(ITEM_STAT_ABILITY_MODIFIER);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_POTENCY:{
			stat_found = item->HasStat(ITEM_STAT_POTENCY);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_AEAUTOATTACK:{
			stat_found = item->HasStat(ITEM_STAT_AEAUTOATTACKCHANCE);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_ATTACKSPEED:{
			stat_found = item->HasStat(ITEM_STAT_ATTACKSPEED);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_BLOCKCHANCE:{
			stat_found = item->HasStat(ITEM_STAT_EXTRASHIELDBLOCKCHANCE);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_CASTINGSPEED:{
			stat_found = item->HasStat(ITEM_STAT_ABILITYCASTINGSPEED);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_CRITBONUS:{
			stat_found = item->HasStat(ITEM_STAT_CRITBONUS);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_CRITCHANCE:{
			stat_found = item->HasStat(ITEM_STAT_MELEECRITCHANCE);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_DPS:{
			stat_found = item->HasStat(ITEM_STAT_DPS);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_FLURRYCHANCE:{
			stat_found = item->HasStat(ITEM_STAT_FLURRY);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_HATEGAIN:{
			stat_found = item->HasStat(ITEM_STAT_HATEGAINMOD);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_MITIGATION:{
			stat_found = item->HasStat(ITEM_STAT_ARMORMITIGATIONINCREASE);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_MULTI_ATTACK:{
			stat_found = item->HasStat(ITEM_STAT_MULTIATTACKCHANCE);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_RECOVERY:{
			stat_found = item->HasStat(ITEM_STAT_ABILITYRECOVERYSPEED);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_REUSE_SPEED:{
			stat_found = item->HasStat(ITEM_STAT_ABILITYREUSESPEED);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_SPELL_WPNDMG:{
			stat_found = item->HasStat(ITEM_STAT_SPELLWEAPONDAMAGEBONUS);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_STRIKETHROUGH:{
			stat_found = item->HasStat(ITEM_STAT_STRIKETHROUGH);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_TOUGHNESS:{
			stat_found = item->HasStat(ITEM_STAT_PVPTOUGHNESS);
			if (stat_found)
				should_add = true;
			break;
		}
		case ITEM_BROKER_STAT_TYPE_WEAPONDMG:{
			stat_found = item->HasStat(ITEM_STAT_WEAPONDAMAGEBONUS);
			if (stat_found)
				should_add = true;
			break;
		}
		default: {
			LogWrite(ITEM__DEBUG, 0, "Item", "Unknown item broker stat type %u", btype);
			LogWrite(ITEM__DEBUG, 0, "Item", "If you have a client before the new expansion this may be the reason.  Please be patient while we update items to support the new client.", btype);
			break;
		}
	}
	
	return should_add;
}

vector<Item*>* MasterItemList::GetItems(string name, int64 itype, int64 ltype, int64 btype, int64 minprice, int64 maxprice, int8 minskill, int8 maxskill, string seller, string adornment, int8 mintier, int8 maxtier, int16 minlevel, int16 maxlevel, sint8 itemclass){
	vector<Item*>* ret = new vector<Item*>;
	map<int32,Item*>::iterator iter;
	Item* item = 0;
	const char* chkname = 0;
	//const char* chkseller = 0;
	//const char* chkadornment = 0;
	if(name.length() > 0)
		chkname = name.c_str();
	//if(seller.length() > 0)
	//	chkseller = seller.c_str();
	//if(adornment.length() > 0)
	//	chkadornment = adornment.c_str();
	LogWrite(ITEM__WARNING, 0, "Item", "Get Items: %s (itype: %llu, ltype: %llu, btype: %llu, minskill: %u, maxskill: %u, mintier: %u, maxtier: %u, minlevel: %u, maxlevel: %u itemclass %i)", name.c_str(), itype, ltype, btype, minskill, maxskill, mintier, maxtier, minlevel, maxlevel, itemclass);
	bool should_add = true;
	for(iter = items.begin();iter != items.end(); iter++){
		item = iter->second;
		if(item){
			if(itype != ITEM_BROKER_TYPE_ANY && itype != ITEM_BROKER_TYPE_ANY64BIT){
				should_add = ShouldAddItemBrokerType(item, itype);
				if(!should_add)
					continue;
			}
			if(ltype != ITEM_BROKER_SLOT_ANY){
				should_add = ShouldAddItemBrokerSlot(item, ltype);
				if(!should_add)
					continue;
			}

			if(btype != 0xFFFFFFFF){
				vector<ItemStat*>::iterator itr;
				should_add = ShouldAddItemBrokerStat(item, btype);
				if (!should_add)
					continue;
			}
			
			if(itemclass > 0){
				int64 tmpVal = ((int64)2) << (itemclass-1);
				should_add = (item->generic_info.adventure_classes & tmpVal);			
				if(!should_add && !(item->generic_info.tradeskill_classes & tmpVal))
					continue;
			}
			if(chkname && item->lowername.find(chkname) >= 0xFFFFFFFF)
				continue;
			if(item->generic_info.adventure_default_level == 0 && item->generic_info.tradeskill_default_level == 0 && minlevel > 0 && maxlevel > 0){
				if(item->details.recommended_level < minlevel)
					continue;
				if(item->details.recommended_level > maxlevel)
					continue;
			}
			else{
				if(minlevel > 0 && ((item->generic_info.adventure_default_level == 0 && item->generic_info.tradeskill_default_level == 0) || (item->generic_info.adventure_default_level > 0 && item->generic_info.adventure_default_level < minlevel) || (item->generic_info.tradeskill_default_level > 0 && item->generic_info.tradeskill_default_level < minlevel)))
					continue;
				if(maxlevel > 0 && ((item->generic_info.adventure_default_level > 0 && item->generic_info.adventure_default_level > maxlevel) || (item->generic_info.tradeskill_default_level > 0 && item->generic_info.tradeskill_default_level > maxlevel)))
					continue;
			}
			// mintier of 1 is 'ANY'
			if(mintier > 1 && item->details.tier < mintier)
				continue;
			if(maxtier > 0 && item->details.tier > maxtier)
				continue;

			/* these skill values are not fields provided in the UI beyond CLASSIC
			** They are also not in line with skill_min, they provide a scale of 0-6, obselete is 0, 6 is red (cannot be used)
			if(minskill > 0 && item->generic_info.skill_min < minskill)
				continue;
			if(maxskill > 0 && item->generic_info.skill_min > maxskill)
				continue;
			*/
			ret->push_back(item);
		}
	}
	return ret;
}

vector<Item*>* MasterItemList::GetItems(map<string, string> criteria, Client* client_to_map){
	string name, seller, adornment;
	int64 itype = ITEM_BROKER_TYPE_ANY64BIT;
	int64 ltype = ITEM_BROKER_TYPE_ANY64BIT;
	int64 btype = ITEM_BROKER_TYPE_ANY64BIT;
	int64 minprice = 0;
	int64 maxprice = 0;
	int8 minskill = 0;
	int8 maxskill = 0;
	int8 mintier = 0;
	int8 maxtier = 0;
	int16 minlevel = 0;
	int16 maxlevel = 0;
	sint8 itemclass = 0;
	int32 itemID = 0;
	if (criteria.count("ITEM") > 0)
	{
		if (IsNumber(criteria["ITEM"].c_str()))
		{
			itemID = atoul(criteria["ITEM"].c_str());
			Item* itm = GetItem(itemID);
			vector<Item*>* ret = new vector<Item*>;
			if (itm)
				ret->push_back(itm);
			return ret;
		}
		else
			name = criteria["ITEM"];
	}
	if(criteria.count("MINSKILL") > 0)
		minskill = (int8)ParseIntValue(criteria["MINSKILL"]);
	if(criteria.count("MAXSKILL") > 0)
		maxskill = (int8)ParseIntValue(criteria["MAXSKILL"]);
	if(criteria.count("MINTIER") > 0)
		mintier = (int8)ParseIntValue(criteria["MINTIER"]);
	if(criteria.count("MAXTIER") > 0)
		maxtier = (int8)ParseIntValue(criteria["MAXTIER"]);
	if(criteria.count("MINLEVEL") > 0)
		minlevel = (int16)ParseIntValue(criteria["MINLEVEL"]);
	if(criteria.count("MAXLEVEL") > 0)
		maxlevel = (int16)ParseIntValue(criteria["MAXLEVEL"]);
	if(criteria.count("ITYPE") > 0)
		itype = ParseLongLongValue(criteria["ITYPE"]);
	if(criteria.count("LTYPE") > 0)
		ltype = ParseLongLongValue(criteria["LTYPE"]);
	if(criteria.count("BTYPE") > 0)
		btype = ParseLongLongValue(criteria["BTYPE"]);
	if(criteria.count("SKILLNAME") > 0)
		itemclass = world.GetClassID(criteria["SKILLNAME"].c_str());
	
	if(client_to_map) {
		map<VersionRange*, map<int64,int64>>::iterator itr = FindBrokerItemMapByVersion(client_to_map->GetVersion());
		if(itr != broker_item_map.end() && itr->second.find(btype) != itr->second.end()) {
			LogWrite(ITEM__DEBUG, 0, "Item", "Found broker mapping, btype %u becomes %llu", btype, itr->second[btype]);
			btype = itr->second[btype];
		}
	}
	if(client_to_map && client_to_map->IsGMStoreSearch()) {
		return GetItems(name, itype, ltype, btype, minprice, maxprice, minskill, maxskill, seller, adornment, mintier, maxtier, minlevel, maxlevel, itemclass);
	}
	else {
		return broker.GetItems(name, itype, ltype, btype, minprice, maxprice, minskill, maxskill, seller, adornment, mintier, maxtier, minlevel, maxlevel, itemclass);
	}
}

int64 MasterItemList::NextUniqueID(){ 
	return database.LoadNextUniqueItemID(); 
}

bool MasterItemList::IsBag(int32 item_id){
	Item* item = GetItem(item_id);
	if(item && item->details.num_slots > 0)
		return true;
	else
		return false;
}


Item* MasterItemList::GetItem(int32 id){
	Item* item = 0;
	if(items.count(id) > 0)
		item = items[id];
	return item;
}

Item* MasterItemList::GetItemByName(const char* name) {
	Item* item = 0;
	map<int32, Item*>::iterator itr;
	for (itr = items.begin(); itr != items.end(); itr++) {
		Item* current_item = itr->second;
		if (::ToLower(string(current_item->name.c_str())) == ::ToLower(string(name))) {
			item = current_item;
			break;
		}
	}
	return item;
}

ItemStatsValues* MasterItemList::CalculateItemBonuses(int32 item_id, Entity* entity){
	return CalculateItemBonuses(items[item_id], entity);
}

ItemStatsValues* MasterItemList::CalculateItemBonuses(Item* item, Entity* entity, ItemStatsValues* values){
	if(item){
		if(!values){
			values = new ItemStatsValues;
			memset(values, 0, sizeof(ItemStatsValues));
		}
		for(int32 i=0;i<item->item_stats.size();i++){
			ItemStat* stat = item->item_stats[i];
				int multiplier = 100;
				if(stat->stat_subtype > 99)
					multiplier = 1000;
			
			int32 id = 0;
			sint32 value = stat->value;
			if(stat->stat_type != 1)
					id = stat->stat_type*multiplier + stat->stat_subtype;
			else
			{
				int32 tmp_id = master_item_list.GetItemStatIDByName(::ToLower(stat->stat_name));
				if(tmp_id != 0xFFFFFFFF)
				{
					id = tmp_id;
					value = stat->stat_subtype;		
				}
				else
					id = stat->stat_type*multiplier + stat->stat_subtype;
			}

			if(entity->IsPlayer()) {
				int32 effective_level = entity->GetInfoStructUInt("effective_level");
				if(effective_level && effective_level < entity->GetLevel() && item->details.recommended_level > effective_level)
				{
					int32 diff = item->details.recommended_level - effective_level;
					float tmpValue = (float)value;
					value = (sint32)(float)(tmpValue / (1.0f + ((float)diff * rule_manager.GetZoneRule(entity->GetZoneID(), R_Player, MentorItemDecayRate)->GetFloat())));
				}
			}
			
			world.AddBonuses(item, values, id, value, entity);
		}
		return values;
	}
	return 0;
}

void MasterItemList::RemoveAll(){
	map<int32,Item*>::iterator iter;
	for(iter = items.begin();iter != items.end(); iter++){
		safe_delete(iter->second);
	}
	items.clear();
	if(lua_interface)
		lua_interface->DestroyItemScripts();
}

void MasterItemList::AddItem(Item* item){
	map<int32, Item*>::iterator iter;
	if((iter = items.find(item->details.item_id)) != items.end()) {
		Item* tmpItem = items[item->details.item_id];
		items.erase(iter);
		safe_delete(tmpItem);
	}
	items[item->details.item_id] = item;
}

Item::Item(){
	seller_char_id = 0;
	seller_house_id = 0;
	is_search_store_item = false;
	is_search_in_inventory = false;
	item_script = "";
	broker_price = 0;
	sell_price = 0;
	sell_status = 0;
	max_sell_value = 0;
	save_needed = true;
	needs_deletion = false;
	weapon_info = 0;
	ranged_info = 0;
	adornment_info = 0;
	bag_info = 0;
	food_info = 0;
	bauble_info = 0;
	thrown_info = 0;
	skill_info = 0;
	recipebook_info = 0;
	itemset_info = 0;
	armor_info = 0;
	book_info = 0;
	book_info_pages = 0;
	houseitem_info = 0;
	housecontainer_info = 0;
	memset(&details, 0, sizeof(ItemCore));
	memset(&generic_info, 0, sizeof(Generic_Info));
	generic_info.condition = 100;
	no_buy_back = false;
	no_sale = false;
	created = std::time(nullptr);
	effect_type = NO_EFFECT_TYPE;
	book_language = 0;
}

Item::Item(Item* in_item){
	seller_char_id = 0;
	seller_house_id = 0;
	is_search_store_item = false;
	is_search_in_inventory = false;
	needs_deletion = false;
	broker_price = 0;
	sell_price = in_item->sell_price;
	sell_status = in_item->sell_status;
	max_sell_value = in_item->max_sell_value;
	save_needed = true;
	SetItem(in_item);
	details.unique_id = master_item_list.NextUniqueID();
	if (IsBag())
		details.bag_id = details.unique_id;
	generic_info.condition = 100;
	spell_id = in_item->spell_id;
	spell_tier = in_item->spell_tier;
	no_buy_back = in_item->no_buy_back;
	no_sale = in_item->no_sale;
	created = in_item->created;
	grouped_char_ids.insert(in_item->grouped_char_ids.begin(), in_item->grouped_char_ids.end());
	effect_type = in_item->effect_type;
	book_language = in_item->book_language;
	details.lock_flags = 0;
	details.item_locked = false;
}

Item::Item(Item* in_item, int64 unique_id, std::string in_creator, std::string in_seller_name, int32 in_seller_char_id, int64 in_broker_price, int16 count, int64 in_seller_house_id, bool search_in_inventory){
	is_search_store_item = true;
	broker_price = in_broker_price;
	needs_deletion = false;
	sell_price = in_item->sell_price;
	sell_status = in_item->sell_status;
	max_sell_value = in_item->max_sell_value;
	save_needed = false;
	SetItem(in_item);
	details.unique_id = unique_id;
	if (IsBag())
		details.bag_id = details.unique_id;
	generic_info.condition = 100;
	spell_id = in_item->spell_id;
	spell_tier = in_item->spell_tier;
	no_buy_back = in_item->no_buy_back;
	no_sale = in_item->no_sale;
	created = in_item->created;
	grouped_char_ids.insert(in_item->grouped_char_ids.begin(), in_item->grouped_char_ids.end());
	effect_type = in_item->effect_type;
	book_language = in_item->book_language;
	creator = in_creator;
	seller_name = in_seller_name;
	seller_char_id = in_seller_char_id;
	details.count = count;
	seller_house_id = in_seller_house_id;
	details.lock_flags = 0;
	details.item_locked = false;
	is_search_in_inventory = search_in_inventory;
}

Item::~Item(){
	for(int32 i=0;i<item_stats.size();i++)
		safe_delete(item_stats[i]);
	for(int32 i=0;i<item_string_stats.size();i++)
		safe_delete(item_string_stats[i]);
	for(int32 i=0;i<item_level_overrides.size();i++)
		safe_delete(item_level_overrides[i]);
	for(int32 i=0;i<item_effects.size();i++)
		safe_delete(item_effects[i]);
	for (int32 i = 0; i < book_pages.size(); i++)
		safe_delete(book_pages[i]);
	safe_delete(weapon_info);
	safe_delete(ranged_info);
	safe_delete(adornment_info);
	safe_delete(bag_info);
	safe_delete(food_info);
	safe_delete(bauble_info);
	safe_delete(thrown_info);
	safe_delete(armor_info);
	safe_delete(skill_info);
	safe_delete(recipebook_info);
	safe_delete(itemset_info);
	safe_delete(book_info);
	safe_delete(houseitem_info);
	safe_delete(housecontainer_info);
	DeleteItemSets();
}
void Item::SetItem(Item* old_item){
	if(old_item->GetItemScript())
		SetItemScript(old_item->GetItemScript());
	name = old_item->name;
	lowername = old_item->lowername;
	description = old_item->description;
	memcpy(&generic_info, &old_item->generic_info, sizeof(Generic_Info));
	weapon_info = 0;
	ranged_info = 0;
	adornment_info = 0;
	adorn0 = 0;
	adorn1 = 0;
	adorn2 = 0;
	bag_info = 0;
	food_info = 0;
	bauble_info = 0;
	thrown_info = 0;
	skill_info = 0;
	recipebook_info = 0;
	itemset_info = 0;
	armor_info = 0;
	book_info = 0;
	book_info_pages = 0;
	houseitem_info = 0;
	housecontainer_info = 0;
	stack_count = old_item->stack_count;
	generic_info.skill_req1 = old_item->generic_info.skill_req1;
	generic_info.skill_req2 = old_item->generic_info.skill_req2;
	memcpy(&details, &old_item->details, sizeof(ItemCore));
	weapon_type = old_item->GetWeaponType();
	switch(old_item->generic_info.item_type){
		case ITEM_TYPE_WEAPON:{
			weapon_info = new Weapon_Info;
			memcpy(weapon_info, old_item->weapon_info, sizeof(Weapon_Info));
			break;
		}
		case ITEM_TYPE_RANGED:{
			ranged_info = new Ranged_Info;
			memcpy(ranged_info, old_item->ranged_info, sizeof(Ranged_Info));
			break;
		}
		case ITEM_TYPE_SHIELD:
		case ITEM_TYPE_ARMOR:{
			armor_info = new Armor_Info;
			memcpy(armor_info, old_item->armor_info, sizeof(Armor_Info));
			break;
		}
		case ITEM_TYPE_BAG:{
			bag_info = new Bag_Info;
			memcpy(bag_info, old_item->bag_info, sizeof(Bag_Info));
			break;
		}
		case ITEM_TYPE_FOOD:{
			food_info = new Food_Info;
			memcpy(food_info, old_item->food_info, sizeof(Food_Info));
			break;
		}
		case ITEM_TYPE_BAUBLE:{
			bauble_info = new Bauble_Info;
			memcpy(bauble_info, old_item->bauble_info, sizeof(Bauble_Info));
			break;
		}
		case ITEM_TYPE_SKILL:{
			skill_info = new Skill_Info;
			memcpy(skill_info, old_item->skill_info, sizeof(Skill_Info));
			break;
		}
		case ITEM_TYPE_THROWN:{
			thrown_info = new Thrown_Info;
			memcpy(thrown_info, old_item->thrown_info, sizeof(Thrown_Info));
			break;
		}
		case ITEM_TYPE_BOOK:{
			book_info = new Book_Info;
			book_info->language = old_item->book_info->language;
			book_info->author.data = old_item->book_info->author.data;
			book_info->author.size = old_item->book_info->author.size;
			book_info->title.data = old_item->book_info->title.data;
			book_info->title.size = old_item->book_info->title.size;
			
			break;
		}
		case ITEM_TYPE_HOUSE:{
			houseitem_info = new HouseItem_Info;
			memcpy(houseitem_info, old_item->houseitem_info, sizeof(HouseItem_Info));
			break;
		}
		case ITEM_TYPE_RECIPE:{
			// Recipe Book
			recipebook_info = new RecipeBook_Info;
			if (old_item->recipebook_info) {
				recipebook_info->recipe_id = old_item->recipebook_info->recipe_id;
				recipebook_info->uses = old_item->recipebook_info->uses;
				for (int32 i = 0; i < old_item->recipebook_info->recipes.size(); i++)
					recipebook_info->recipes.push_back(old_item->recipebook_info->recipes.at(i));
			}
			break;
		}
		
		case ITEM_TYPE_ADORNMENT:{
			adornment_info = new Adornment_Info;
			memcpy(adornment_info, old_item->adornment_info, sizeof(Adornment_Info));
			break;
		}
		case ITEM_TYPE_HOUSE_CONTAINER:{
			houseitem_info = new HouseItem_Info;
			memset(houseitem_info, 0, sizeof(HouseItem_Info));
			bag_info = new Bag_Info;
			memset(bag_info, 0, sizeof(Bag_Info));
			
			if(old_item->bag_info)
				memcpy(bag_info, old_item->bag_info, sizeof(Bag_Info));
			
			if(old_item->houseitem_info) {
				memcpy(houseitem_info, old_item->houseitem_info, sizeof(HouseItem_Info));
			}
			
			// House Containers
			housecontainer_info = new HouseContainer_Info;
			if (old_item->housecontainer_info) {
				housecontainer_info->broker_commission = old_item->housecontainer_info->broker_commission;
				housecontainer_info->fence_commission = old_item->housecontainer_info->fence_commission;
				housecontainer_info->allowed_types = old_item->housecontainer_info->allowed_types;
				housecontainer_info->num_slots = old_item->housecontainer_info->num_slots;
			}
			break;
		}
	}
	creator = old_item->creator;
	adornment = old_item->adornment;
	DeleteItemSets();
	for (int32 i = 0; i<old_item->item_sets.size(); i++){
		ItemSet* set = old_item->item_sets[i];
		if (set){
			ItemSet* set2 = new ItemSet;
			set2->item_id = set->item_id;
			set2->item_crc = set->item_crc;
			set2->item_icon = set->item_icon;
			set2->item_stack_size = set->item_stack_size;
			set2->item_list_color = set->item_list_color;
			item_sets.push_back(set2);
		}
	}
	item_stats.clear();
	for(int32 i=0;i<old_item->item_stats.size();i++){
		ItemStat* stat = old_item->item_stats[i];
		if(stat){
			ItemStat* stat2 = new ItemStat;
			stat2->stat_name = stat->stat_name;
			stat2->stat_type = stat->stat_type;
			stat2->stat_subtype = stat->stat_subtype;
			stat2->value = stat->value;
			stat2->stat_type_combined = stat->stat_type_combined;
			item_stats.push_back(stat2);
		}
	}
	item_string_stats.clear();
	for(int32 i=0;i<old_item->item_string_stats.size();i++){
		ItemStatString* stat = old_item->item_string_stats[i];
		if(stat){
			ItemStatString* stat2 = new ItemStatString;
			stat2->stat_string.data = stat->stat_string.data;
			stat2->stat_string.size = stat->stat_string.size;
			item_string_stats.push_back(stat2);
		}
	}
	item_level_overrides.clear();
	for(int32 i=0;i<old_item->item_level_overrides.size();i++){
		ItemLevelOverride* item_override = old_item->item_level_overrides[i];
		if(item_override){
			ItemLevelOverride* item_override2 = new ItemLevelOverride;
			memcpy(item_override2, item_override, sizeof(ItemLevelOverride));
			item_level_overrides.push_back(item_override2);
		}
	}
	item_effects.clear();
	for(int32 i=0;i<old_item->item_effects.size();i++){
		ItemEffect* effect = old_item->item_effects[i];
		if(effect){
			ItemEffect* effect_2 = new ItemEffect;
			effect_2->effect = effect->effect;
			effect_2->percentage = effect->percentage;
			effect_2->subbulletflag = effect->subbulletflag;
			item_effects.push_back(effect_2);
		}
	}
	book_pages.clear();
	for (int32 i = 0; i < old_item->book_pages.size(); i++) {
		BookPage* bookpage = old_item->book_pages[i];
		if (bookpage) {
			BookPage* bookpage_2 = new BookPage;
			bookpage_2->page = bookpage->page;
			bookpage_2->page_text.data = bookpage->page_text.data;
			bookpage_2->page_text.size = bookpage->page_text.size;
			bookpage_2->valign = bookpage->valign;
			bookpage_2->halign = bookpage->halign;
			



			book_pages.push_back(bookpage_2);
		}
	}
	slot_data.clear();
	slot_data = old_item->slot_data;
	spell_id = old_item->spell_id;
	spell_tier = old_item->spell_tier;
	book_language = old_item->book_language;
}

bool Item::CheckArchetypeAdvSubclass(int8 adventure_class, map<int8, int16>* adv_class_levels) {
	if (adventure_class > FIGHTER && adventure_class < ANIMALIST) {
		int8 check = adventure_class % 10;
		if (check == 2 || check == 5 || check == 8) {
			int64 adv_classes = 0;
			int16 level = 0;
			for (int i = adventure_class + 1; i < adventure_class + 3; i++) {				
				if (adv_class_levels) { //need to match levels
					if (level == 0) {
						if (adv_class_levels->count(i) > 0)
							level = adv_class_levels->at(i);
						else
							return false;
					}
					else{
						if (adv_class_levels->count(i) > 0 && adv_class_levels->at(i) != level)
							return false;
					}
				}
				else {
					adv_classes = ((int64)2) << (i - 1);
					if (!(generic_info.adventure_classes & adv_classes))
						return false;
				}
			}
			return true;
		}
	}
	return false;
}

bool Item::CheckArchetypeAdvClass(int8 adventure_class, map<int8, int16>* adv_class_levels) {
	if (adventure_class == 1 || adventure_class == 11 || adventure_class == 21 || adventure_class == 31) {
		//if the class is an archetype class and the subclasses have access, then allow
		if (CheckArchetypeAdvSubclass(adventure_class + 1, adv_class_levels) && CheckArchetypeAdvSubclass(adventure_class + 4, adv_class_levels) && CheckArchetypeAdvSubclass(adventure_class + 7, adv_class_levels)) {
			if (adv_class_levels) {
				int16 level = 0;
				for (int i = adventure_class + 1; i <= adventure_class + 7; i += 3) {
					if (adv_class_levels->count(i+1) == 0 || adv_class_levels->count(i + 2) == 0)
						return false;
					if(level == 0)
						level = adv_class_levels->at(i+1);
					if (adv_class_levels->at(i+1) != level) //already verified the classes, just need to verify the subclasses have the same levels
						return false;
				}
				
			}
			return true;
		}
	}
	else if (CheckArchetypeAdvSubclass(adventure_class, adv_class_levels)) {//check archetype subclass		
		return true;
	}
	return false;
}

bool Item::CheckClass(int8 adventure_class, int8 tradeskill_class) {
	int64 adv_classes = ((int64)2) << (adventure_class - 1);
	int64 ts_classes = ((int64)2) << (tradeskill_class - 1);
	if( ((generic_info.adventure_classes & adv_classes) || generic_info.adventure_classes == 0) && ((generic_info.tradeskill_classes & ts_classes) || generic_info.tradeskill_classes == 0) )
		return true;
	//check arechtype classes as last resort
	return CheckArchetypeAdvClass(adventure_class);
}

bool Item::CheckLevel(int8 adventure_class, int8 tradeskill_class, int16 level) {
	if ((level >= generic_info.adventure_default_level && adventure_class < 255) && (level >= generic_info.tradeskill_default_level && tradeskill_class < 255))
		return true;
	return false;
}

void Item::AddStat(ItemStat* in_stat){
	item_stats.push_back(in_stat);
}

bool Item::HasStat(uint32 statID, std::string statNameLower)
{
	vector<ItemStat*>::iterator itr;
	for (itr = item_stats.begin(); itr != item_stats.end(); itr++) {
		if (statID > 99 && statID < 200 && 
		(*itr)->stat_type == 1 && ::ToLower((*itr)->stat_name) == statNameLower) {
			return true;
			break;
		}
		else if((*itr)->stat_type_combined == statID && (statNameLower.length() < 1 || 
			(::ToLower((*itr)->stat_name) == statNameLower))) {
			return true;
			break;
		}
	}

	return false;
}

void Item::DeleteItemSets()
{
	for (int32 i = 0; i < item_sets.size(); i++){
		ItemSet* set = item_sets[i];
		safe_delete(set);
	}
		
	item_sets.clear();
}

void Item::AddSet(ItemSet* in_set){
	item_sets.push_back(in_set);
}
void Item::AddStatString(ItemStatString* in_stat){
	item_string_stats.push_back(in_stat);
}

bool Item::IsNormal(){ 
	return generic_info.item_type == ITEM_TYPE_NORMAL; 
}

bool Item::IsWeapon(){ 
	return generic_info.item_type == ITEM_TYPE_WEAPON; 
}

bool Item::IsDualWieldAble(Client* client, Item* item, int8 slot) {

	if (!item || !client || slot < 0) {
		LogWrite(ITEM__DEBUG, 0, "Items", "Error in IsDualWieldAble. No Item, Client, or slot Passed");
		return 0;
	}

	Player* player = client->GetPlayer();
	int8 base_class = classes.GetBaseClass(player->GetAdventureClass());

	//map out classes that can dw vs those that cant (did it this way so its easier to expand should we need to add classes later
	int8 can_dw;
	switch ((int)base_class) {
	case 1:
		can_dw = 1;
		break;
	case 5:
		can_dw = 1;
		break;
	case 31:
		can_dw = 1;
		break;
	case 35:
		can_dw = 1;
		break;
	case 41:
		can_dw = 1;
		break;

	default :
		can_dw = 0;
	}

	//if mage, item is dw, and they are trying to put offhand. Not sure this will ever happen but figured I should cover it.
	if (base_class == 21 && item->weapon_info->wield_type == ITEM_WIELD_TYPE_DUAL && slot == 1) {
		return 0;
	}

	//if the item is main hand (single) and they are trying to put in in offhand.
	//exceptions are classes 1, 5, 31, 35, 42 (fighter/brawler/rogue/bard/beastlord)
	if (item->weapon_info->wield_type == ITEM_WIELD_TYPE_SINGLE && slot == 1 && can_dw != 1) {
		return 0;
	}
//assume its safe if the above 2 if's arent hit.
return 1;
}

bool Item::IsArmor(){ 
	return generic_info.item_type == ITEM_TYPE_ARMOR || generic_info.item_type == ITEM_TYPE_SHIELD; 
}

bool Item::IsRanged(){ 
	return generic_info.item_type == ITEM_TYPE_RANGED; 
}

bool Item::IsBag(){ 
	return generic_info.item_type == ITEM_TYPE_BAG || generic_info.item_type == ITEM_TYPE_HOUSE_CONTAINER; 
}

bool Item::IsFood(){ 
	return generic_info.item_type == ITEM_TYPE_FOOD; 
}

bool Item::IsBauble(){ 
	return generic_info.item_type == ITEM_TYPE_BAUBLE; 
}

bool Item::IsSkill(){
	return generic_info.item_type == ITEM_TYPE_SKILL; 
}

bool Item::IsHouseItem(){
	return generic_info.item_type == ITEM_TYPE_HOUSE; 
}

bool Item::IsHouseContainer(){
	return generic_info.item_type == ITEM_TYPE_HOUSE_CONTAINER; 
}

bool Item::IsShield(){
	return generic_info.item_type == ITEM_TYPE_SHIELD; 
}

bool Item::IsAdornment(){
	return generic_info.item_type == ITEM_TYPE_ADORNMENT && !CheckFlag2(ORNATE);
}

bool Item::IsAmmo(){
	return HasSlot(EQ2_AMMO_SLOT);
}

bool Item::HasAdorn0(){
	if (adorn0 > 0)
		return true;

	return false;
}

bool Item::HasAdorn1(){
	if (adorn1 > 0)
		return true;

	return false;
}

bool Item::HasAdorn2(){
	if (adorn2 > 0)
		return true;

	return false;
}




bool Item::IsBook(){
	return generic_info.item_type == ITEM_TYPE_BOOK; 
}

bool Item::IsChainArmor(){
	return generic_info.item_type == ITEM_TYPE_ARMOR && (generic_info.skill_req1 == 2246237129UL || generic_info.skill_req2 == 2246237129UL);
}

bool Item::IsClothArmor(){
	return generic_info.item_type == ITEM_TYPE_ARMOR && (generic_info.skill_req1 == 3539032716UL || generic_info.skill_req2 == 3539032716UL);
}

bool Item::IsCollectable(){
	return generic_info.collectable == 1;
}

bool Item::HasSlot(int8 slot, int8 slot2){
	for(int32 i=0;i<slot_data.size();i++){
		if(slot_data[i] == EQ2_LRING_SLOT && (slot == EQ2_RRING_SLOT || slot2 == EQ2_RRING_SLOT))
			return true;
		else if(slot_data[i] == EQ2_LWRIST_SLOT && (slot == EQ2_RWRIST_SLOT || slot2 == EQ2_RWRIST_SLOT))
			return true;
		else if(slot_data[i] == EQ2_EARS_SLOT_1 && (slot == EQ2_EARS_SLOT_2 || slot2 == EQ2_EARS_SLOT_2))
			return true;
		else if(slot_data[i] == EQ2_CHARM_SLOT_1 && (slot == EQ2_CHARM_SLOT_2 || slot2 == EQ2_CHARM_SLOT_2))
			return true;
		else if(slot_data[i] == slot || slot_data[i] == slot2)
			return true;
	}
	return false;
}

bool Item::IsCloak(){
	return HasSlot(EQ2_CLOAK_SLOT);
}

bool Item::IsCrushWeapon(){
	return generic_info.item_type == ITEM_TYPE_WEAPON && weapon_type == 1;
}

bool Item::IsFoodFood(){
	return generic_info.item_type == ITEM_TYPE_FOOD && food_info && food_info->type == 1;
}

bool Item::IsFoodDrink(){
	return generic_info.item_type == ITEM_TYPE_FOOD && food_info && food_info->type == 0;
}

bool Item::IsJewelry(){
	if(generic_info.item_type != ITEM_TYPE_ARMOR || (generic_info.skill_req1 != 2072844078 && generic_info.skill_req2 != 2072844078))
		return false;
	for(int32 i=0;i<slot_data.size();i++){
		if(slot_data[i] == EQ2_LRING_SLOT || slot_data[i] == EQ2_RRING_SLOT || slot_data[i] == EQ2_EARS_SLOT_1 || slot_data[i] == EQ2_EARS_SLOT_2 || slot_data[i] == EQ2_NECK_SLOT || slot_data[i] == EQ2_LWRIST_SLOT || slot_data[i] == EQ2_RWRIST_SLOT)
			return true;
	}
	return false;
}

bool Item::IsLeatherArmor(){
	return generic_info.item_type == ITEM_TYPE_ARMOR && (generic_info.skill_req1 == 2897193374UL || generic_info.skill_req2 == 2897193374UL);
}

bool Item::IsMisc(){
	return IsNormal();
}

bool Item::IsPierceWeapon(){
	return generic_info.item_type == ITEM_TYPE_WEAPON && weapon_type == 2;
}

bool Item::IsPlateArmor(){
	return generic_info.item_type == ITEM_TYPE_ARMOR && (generic_info.skill_req1 == 241174330UL || generic_info.skill_req2 == 241174330UL);
}

bool Item::IsPoison(){
	LogWrite(MISC__TODO, 1, "TODO", "Separate poisons from potions\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);
	return IsBauble();
}

bool Item::IsPotion(){
	LogWrite(MISC__TODO, 1, "TODO", "Separate poisons from potions\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);
	return IsBauble();
}

bool Item::IsRecipeBook(){
	return generic_info.item_type == ITEM_TYPE_RECIPE; 
}

bool Item::IsSalesDisplay(){
	return generic_info.item_type == ITEM_TYPE_HOUSE_CONTAINER; 
}

bool Item::IsSlashWeapon(){
	return generic_info.item_type == ITEM_TYPE_WEAPON && weapon_type == 0;
}

bool Item::IsSpellScroll(){
	return IsSkill();
}
//item->tinkered.
/*
bool Item::IsTinkered(){
	LogWrite(MISC__TODO, 1, "TODO", "Item Is Tinkered\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);
	return false;
}
*/

bool Item::IsThrown(){
	return generic_info.item_type == ITEM_TYPE_THROWN;
}

bool Item::IsHarvest() {
	return generic_info.harvest == 1;
}

bool Item::IsBodyDrop() {
	return generic_info.body_drop == 1;
}
//item->crafted
/*bool Item::IsTradeskill(){
	LogWrite(MISC__TODO, 1, "TODO", "Item Is Crafted\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);
	return false;
}*/

void Item::SetItemType(int8 in_type){
	generic_info.item_type = in_type;
	if(IsArmor() && !armor_info){
		armor_info = new Armor_Info;
		memset(armor_info, 0, sizeof(Armor_Info));
	}
	else if (IsWeapon() && !weapon_info){
		weapon_info = new Weapon_Info;
		memset(weapon_info, 0, sizeof(Weapon_Info));
	}
	else if (IsAdornment() && !adornment_info){
		adornment_info = new Adornment_Info;
		memset(adornment_info, 0, sizeof(Adornment_Info));
	}
	else if(IsRanged() && !ranged_info){
		ranged_info = new Ranged_Info;
		memset(ranged_info, 0, sizeof(Ranged_Info));
	}
	else if(IsBag() && !IsHouseContainer() && !bag_info){
		bag_info = new Bag_Info;
		memset(bag_info, 0, sizeof(Bag_Info));
	}
	else if(IsFood() && !food_info){
		food_info = new Food_Info;
		memset(food_info, 0, sizeof(Food_Info));
	}
	else if(IsBauble() && !bauble_info){
		bauble_info = new Bauble_Info;
		memset(bauble_info, 0, sizeof(Bauble_Info));
	}
	else if(IsThrown() && !thrown_info){
		thrown_info = new Thrown_Info;
		memset(thrown_info, 0, sizeof(Thrown_Info));
	}
	else if(IsSkill() && !skill_info){
		skill_info = new Skill_Info;
		memset(skill_info, 0, sizeof(Skill_Info));
	}
	else if(IsRecipeBook() && !recipebook_info){
		recipebook_info = new RecipeBook_Info;
		recipebook_info->recipe_id = 0;
		recipebook_info->uses = 0;
	}
	else if(IsBook() && !book_info){
		book_info = new Book_Info;
		book_info->language = 0;
		book_info->author.size = 0;
		book_info->title.size = 0;
	}
	else if(IsHouseItem() && !IsHouseContainer() && !houseitem_info){
		houseitem_info = new HouseItem_Info;
		memset(houseitem_info, 0, sizeof(HouseItem_Info));
	}
	else if(IsHouseContainer() && !housecontainer_info){
		bag_info = new Bag_Info;
		memset(bag_info, 0, sizeof(Bag_Info));
		
		if(!houseitem_info) {
			houseitem_info = new HouseItem_Info;
			memset(houseitem_info, 0, sizeof(HouseItem_Info));
		}
		housecontainer_info = new HouseContainer_Info;
		housecontainer_info->allowed_types = 0;
		housecontainer_info->broker_commission = 0;
		housecontainer_info->fence_commission = 0;
		housecontainer_info->num_slots = 0;
	}
}
bool Item::CheckFlag2(int32 flag){
	int32 value = 0;
	int32 flag_val = generic_info.item_flags2;
	while (flag_val > 0){
		if (flag_val >= FLAGS2_32768) 
			value = FLAGS2_32768;
		else if (flag_val >= FREE_REFORGE)
			value = FREE_REFORGE;
		else if (flag_val >= BUILDING_BLOCK)
			value = BUILDING_BLOCK;
		else if (flag_val >= FLAGS2_4096)
			value = FLAGS2_4096;
		else if (flag_val >= HOUSE_LORE)
			value = HOUSE_LORE;
		else if (flag_val >= NO_EXPERIMENT)
			value = NO_EXPERIMENT;
		else if (flag_val >= INDESTRUCTABLE)
			value = INDESTRUCTABLE;
		else if (flag_val >= NO_SALVAGE)
			value = NO_SALVAGE;
		else if (flag_val >= REFINED)
			value = REFINED;
		else if (flag_val >= ETHERAL)
			value = ETHERAL;
		else if (flag_val >= NO_REPAIR)
			value = NO_REPAIR;
		else if (flag_val >= REFORGED)
			value = REFORGED;
		else if (flag_val >= UNLOCKED)
			value = UNLOCKED;
		else if (flag_val >= APPEARANCE_ONLY)
			value = APPEARANCE_ONLY;
		else if (flag_val >= HEIRLOOM)
			value = HEIRLOOM;
		else if (flag_val >= ORNATE)
			value = ORNATE;
		if (value == flag)
			return true;
		else
			flag_val -= value;
	}

	return false;
}

bool Item::CheckFlag(int32 flag){
	int32 value = 0;
	int32 flag_val = generic_info.item_flags;
	while(flag_val>0){
		if (flag_val >= CURSED) //change this
			value = CURSED;
		else if (flag_val >= NO_TRANSMUTE) //change this
			value = NO_TRANSMUTE;
		else if (flag_val >= LORE_EQUIP) //change this
			value = LORE_EQUIP;
		else if (flag_val >= STACK_LORE) //change this
			value = STACK_LORE;
		else if(flag_val >= EVIL_ONLY)
			value = EVIL_ONLY;
		else if(flag_val >= GOOD_ONLY)
			value = GOOD_ONLY;
		else if(flag_val >= CRAFTED)
			value = CRAFTED;
		else if(flag_val >= NO_DESTROY)
			value = NO_DESTROY;
		else if(flag_val >= NO_ZONE)
			value = NO_ZONE;
		else if(flag_val >= NO_VALUE)
			value = NO_VALUE;
		else if(flag_val >= NO_TRADE)
			value = NO_TRADE;
		else if(flag_val >= TEMPORARY)
			value = TEMPORARY;
		else if(flag_val >= LORE)
			value = LORE;
		else if(flag_val >= ARTIFACT)
			value = ARTIFACT;
		else if(flag_val >= ATTUNEABLE)
			value = ATTUNEABLE;
		else if(flag_val >= ATTUNED)
			value = ATTUNED;
		if(value == flag)
			return true;
		else
			flag_val -= value;
	}
	return false;
}

void Item::SetSlots(int32 slots){
	if(slots & PRIMARY_SLOT)
		AddSlot(EQ2_PRIMARY_SLOT);
	if(slots & SECONDARY_SLOT)
		AddSlot(EQ2_SECONDARY_SLOT);
	if(slots & HEAD_SLOT)
		AddSlot(EQ2_HEAD_SLOT);
	if(slots & CHEST_SLOT)
		AddSlot(EQ2_CHEST_SLOT);
	if(slots & SHOULDERS_SLOT)
		AddSlot(EQ2_SHOULDERS_SLOT);
	if(slots & FOREARMS_SLOT)
		AddSlot(EQ2_FOREARMS_SLOT);
	if(slots & HANDS_SLOT)
		AddSlot(EQ2_HANDS_SLOT);
	if(slots & LEGS_SLOT)
		AddSlot(EQ2_LEGS_SLOT);
	if(slots & FEET_SLOT)
		AddSlot(EQ2_FEET_SLOT);
	if(slots & LRING_SLOT)
		AddSlot(EQ2_LRING_SLOT);
	if(slots & RRING_SLOT)
		AddSlot(EQ2_RRING_SLOT);
	if(slots & EARS_SLOT_1)
		AddSlot(EQ2_EARS_SLOT_1);
	if(slots & EARS_SLOT_2)
		AddSlot(EQ2_EARS_SLOT_2);
	if(slots & NECK_SLOT)
		AddSlot(EQ2_NECK_SLOT);
	if(slots & LWRIST_SLOT)
		AddSlot(EQ2_LWRIST_SLOT);
	if(slots & RWRIST_SLOT)
		AddSlot(EQ2_RWRIST_SLOT);
	if(slots & RANGE_SLOT)
		AddSlot(EQ2_RANGE_SLOT);
	if(slots & AMMO_SLOT)
		AddSlot(EQ2_AMMO_SLOT);
	if(slots & WAIST_SLOT)
		AddSlot(EQ2_WAIST_SLOT);
	if(slots & CLOAK_SLOT)
		AddSlot(EQ2_CLOAK_SLOT);
	if(slots & CHARM_SLOT_1)
		AddSlot(EQ2_CHARM_SLOT_1);
	if(slots & CHARM_SLOT_2)
		AddSlot(EQ2_CHARM_SLOT_2);
	if(slots & FOOD_SLOT)
		AddSlot(EQ2_FOOD_SLOT);
	if(slots & DRINK_SLOT)
		AddSlot(EQ2_DRINK_SLOT);
	if(slots & TEXTURES_SLOT)
		AddSlot(EQ2_TEXTURES_SLOT);
}

void Item::AddStat(int8 type, int16 subtype, float value, int8 level, char* name){
	char item_stat_combined_string[8] = {0};
	if(name && strlen(name) > 0 && type != 1){
		ItemStatString* stat = new ItemStatString;
		stat->stat_string.data = string(name);
		stat->stat_string.size = stat->stat_string.data.length();
		AddStatString(stat);
	}
	else{
		ItemStat* stat = new ItemStat;
		if(name && strlen(name) > 0)
			stat->stat_name = string(name);
		stat->stat_type = type;
		stat->stat_subtype = subtype;
		stat->value = value;
		stat->level = level;
		snprintf(item_stat_combined_string, 7, "%u%02u", type, subtype);
		stat->stat_type_combined = atoi(item_stat_combined_string);
		AddStat(stat);
	}
}
void Item::AddSet(int32 item_id, int32 item_crc, int16 item_icon, int32 item_stack_size, int32 item_list_color, std::string name, int8 language){
	ItemSet* set = new ItemSet;
	set->item_id = item_id;
	set->item_icon = item_icon;
	set->item_crc = item_crc;
	set->item_stack_size = item_stack_size;
	set->item_list_color = item_list_color;
	set->name = string(name);
	set->language = language;
	
	AddSet(set);
}

int16 Item::GetOverrideLevel(int8 adventure_class, int8 tradeskill_class){
	int16 ret = 0;
	int8 tmp_class = 0;
	bool found_class = false;
	for(int32 i=0;i<item_level_overrides.size();i++){
		found_class = false;
		if(adventure_class != 255){
			tmp_class = item_level_overrides[i]->adventure_class;
			if(tmp_class == PRIEST && (adventure_class >= CLERIC && adventure_class <= DEFILER))
				found_class = true;
			else if(tmp_class == MAGE && (adventure_class >= SORCERER && adventure_class <= NECROMANCER))
				found_class = true;
			else if(tmp_class == SCOUT && (adventure_class >= ROGUE && adventure_class <= ASSASSIN))
				found_class = true;
			else if(tmp_class == adventure_class || tmp_class == COMMONER || (tmp_class == FIGHTER && (adventure_class >= WARRIOR && adventure_class <= PALADIN)))
				found_class = true;			
		}
		else if(tradeskill_class != 255){
			tmp_class = item_level_overrides[i]->tradeskill_class;
			if(tmp_class == CRAFTSMAN && (tradeskill_class >= PROVISIONER && adventure_class <= CARPENTER))
				found_class = true;
			else if(tmp_class == OUTFITTER && (tradeskill_class >= ARMORER && tradeskill_class <= TAILOR))
				found_class = true;
			else if(tmp_class == SCHOLAR && (tradeskill_class >= JEWELER && tradeskill_class <= ALCHEMIST))
				found_class = true;
			else if(tmp_class == tradeskill_class || tmp_class == ARTISAN)
				found_class = true;
		}
		if(found_class){
			ret = item_level_overrides[i]->level;
			break;
		}
	}
	return ret;
}

void Item::serialize(PacketStruct* packet, bool show_name, Player* player, int16 packet_type, int8 subtype, bool loot_item, bool inspect){
	int64 classes = 0;
	Client *client;
	int8 tmp_subtype = 0;
	if (!packet || !player)
		return;
	client = ((Player*)player)->GetClient();
	if (!client)
		return;
	if(creator.length() > 0){
		packet->setSubstructSubstructDataByName("header", "info", "creator_flag", 1);
		packet->setSubstructSubstructDataByName("header", "info", "creator", creator.c_str());
	}
	if(show_name)
		packet->setSubstructSubstructDataByName("header", "info_header", "show_name", show_name);
	
	if(packet_type == 0)
		packet->setSubstructSubstructDataByName("header", "info_header", "packettype", GetItemPacketType(packet->GetVersion()));
	else
		packet->setSubstructSubstructDataByName("header", "info_header", "packettype", packet_type);
	packet->setSubstructSubstructDataByName("header", "info_header", "packetsubtype", subtype);  // should be substype

	/*
0 red
1 orange
2 yellow
3 white
4 blue
5 green
6 grey
7 purple*/
	int32 color = 3;

	if(player)
	{
		int32 effective_level = player->GetInfoStructUInt("effective_level");
		if(effective_level && effective_level < player->GetLevel() && details.recommended_level > effective_level)
			color = 7;
	}

	packet->setSubstructDataByName("header_info", "footer_type", color);
	packet->setSubstructDataByName("header_info", "item_id", details.item_id);
	
	if (!loot_item)
		packet->setSubstructDataByName("header_info", "broker_item_id", details.item_id);
	else
		packet->setSubstructDataByName("header_info", "broker_item_id", 0xFFFFFFFFFFFFFFFF);

	if(details.unique_id == 0)
		packet->setSubstructDataByName("header_info", "unique_id", details.item_id);
	else
		packet->setSubstructDataByName("header_info", "unique_id", details.unique_id);
	packet->setSubstructDataByName("header_info", "icon", GetIcon(packet->GetVersion()));
	
	if(rule_manager.GetZoneRule(player->GetZoneID(), R_World, DisplayItemTiers)->GetBool()) {
		packet->setSubstructDataByName("header_info", "tier", details.tier);
	}
 	packet->setSubstructDataByName("header_info", "flags", generic_info.item_flags);
	packet->setSubstructDataByName("header_info", "flags2", generic_info.item_flags2);
	if(item_stats.size() > 0){
		//packet->setSubstructArrayLengthByName("header_info", "stat_count", item_stats.size());
		int8 dropstat = 0;
		int8 bluemod = 0;
		for (int32 i = 0; i < item_stats.size(); i++){
			ItemStat* stat = item_stats[i];

			if(!stat)
			{
				LogWrite(ITEM__ERROR, 0, "Item", "%s: %s (itemid: %u) Error Serializing Item: Invalid item in item_stats position %u", client->GetPlayer()->GetName(), this->name.c_str(), this->details.item_id, i);
				continue;
			}
			
			if (stat->stat_type == 9){
				bluemod += 1;
			}
			
			tmp_subtype = world.TranslateSlotSubTypeToClient(client, stat->stat_type, stat->stat_subtype);
			
			if (tmp_subtype == 255 ){
				dropstat += 1;
			}

		}
		packet->setSubstructArrayLengthByName("header_info", "stat_count", item_stats.size() - dropstat);
		dropstat = 0;
		for (int32 i = 0; i < item_stats.size(); i++) {
			ItemStat* stat = item_stats[i];
			tmp_subtype = world.TranslateSlotSubTypeToClient(client, stat->stat_type, stat->stat_subtype);
			int16 stat_type = stat->stat_type;
			
			float statValue = stat->value;
			if(player)
			{
				int32 effective_level = player->GetInfoStructUInt("effective_level");
				if(effective_level && effective_level < player->GetLevel() && details.recommended_level > effective_level)
				{
					int32 diff = details.recommended_level - effective_level;
					float tmpValue = (float)statValue;
					statValue = (sint32)(float)(tmpValue / (1.0f + ((float)diff * rule_manager.GetZoneRule(player->GetZoneID(), R_Player, MentorItemDecayRate)->GetFloat())));
				}
			}	
			
			bool valueSet = false;
			if (tmp_subtype == 255 ){
				
				dropstat += 1;
				//packet->setSubstructArrayLengthByName("header_info", "stat_count", item_stats.size()-dropstat);
			}
			else {
				packet->setArrayDataByName("stat_type", stat_type, i-dropstat);
				
				if(client->GetVersion() <= 561 && stat_type == 5) {
					valueSet = true;
					// DoF client has to be goofy about this junk, stat_subtype is the stat value, value is always "9" and we set the stat_name to the appropriate stat (but power=mana)
					packet->setArrayDataByName("stat_subtype", (sint16)statValue , i - dropstat);
					packet->setArrayDataByName("value", (sint16)9 , i - dropstat);
					switch(tmp_subtype) {
						case 0: {
							packet->setArrayDataByName("stat_name", "health", i - dropstat);
							break;
						}
						case 1: {
							packet->setArrayDataByName("stat_name", "mana", i - dropstat);
							break;
						}
						case 2: {
							packet->setArrayDataByName("stat_name", "concentration", i - dropstat);
							break;
						}
					}
				}
				else {
					packet->setArrayDataByName("stat_subtype", tmp_subtype, i-dropstat);
				}
			}
			if (stat->stat_name.length() > 0)
				packet->setArrayDataByName("stat_name", stat->stat_name.c_str(), i-dropstat);
			/* SF client */

			if(!valueSet) {
				if ((client->GetVersion() >= 63119) || client->GetVersion() == 61331) {
					if (stat->stat_type == 6){
						packet->setArrayDataByName("value", statValue , i - dropstat);//63119 or when diety started (this is actually the modified stat
						packet->setArrayDataByName("value2", stat->value, i - dropstat);//63119 temp will be replace by modified value (this is the unmodified stat
					}
					else	{
						packet->setArrayDataByName("value", (sint16)statValue , i - dropstat, 0U, true);
						packet->setArrayDataByName("value2", stat->value, i - dropstat);//63119 temp will be replace by modified value
					}
				}
				 else if (client->GetVersion() >= 1028) {
					if (stat->stat_type == 6){
						packet->setArrayDataByName("value", statValue , i - dropstat);//63119 or when diety started (this is actually the infused modified stat
						packet->setArrayDataByName("value2", stat->value, i - dropstat);//63119 temp will be replace by modified value (this is the unmodified stat
					}
					else {
						packet->setArrayDataByName("value", (sint16)statValue , i - dropstat, 0U, true);
						packet->setArrayDataByName("value2", stat->value, i - dropstat);//63119 temp will be replace by modified value
					}
					
				}
				else{
					packet->setArrayDataByName("value", (sint16)statValue , i - dropstat);
					packet->setArrayDataByName("value2", stat->value, i - dropstat);//63119 temp will be replace by modified value
				}
			}
		}
	}
	if (item_string_stats.size() > 0 && !loot_item){
		if ((client->GetVersion() >= 63119) || client->GetVersion() == 61331) {
			packet->setSubstructArrayLengthByName("header_info", "mod_count", item_string_stats.size());
			for (int32 i = 0; i < item_string_stats.size(); i++){
				ItemStatString* stat = item_string_stats[i];
				packet->setArrayDataByName("mod_string", &(stat->stat_string), i);
				packet->setArrayDataByName("mod_need", 0, i);
			}
		}

		else if (client->GetVersion() >= 1096) {
			packet->setSubstructArrayLengthByName("header_info", "stat_string_count", item_string_stats.size());
			for (int32 i = 0; i < item_string_stats.size(); i++){
				ItemStatString* stat = item_string_stats[i];
				packet->setArrayDataByName("stat_string", &(stat->stat_string), i);

			}
		}
	}
	if (item_sets.size() > 0){
		packet->setArrayLengthByName("num_pieces", item_sets.size());
		for (int32 i = 0; i < item_sets.size(); i++){
			ItemSet* set = item_sets[i];
			packet->setArrayDataByName("item_id", set->item_id, i);
			packet->setArrayDataByName("item_crc", set->item_crc, i);
			packet->setArrayDataByName("item_icon", set->item_icon, i);
			packet->setArrayDataByName("item_unknown1", set->item_stack_size, i);

			Item* item2 = master_item_list.GetItem(set->item_id);
			if (item2)
				packet->setArrayDataByName("item_name", item2->name.c_str(), i);

			packet->setArrayDataByName("item_unknown2", set->item_list_color, i);
			
		}
			
	
	}

		



	if(!loot_item && item_effects.size() > 0){
		packet->setSubstructArrayLengthByName("footer", "num_effects", item_effects.size());
		for(int32 i=0;i<item_effects.size();i++){
			ItemEffect* effect = item_effects[i];
			packet->setArrayDataByName("subbulletflag", effect->subbulletflag, i);
			packet->setArrayDataByName("effect", &(effect->effect), i);
			packet->setArrayDataByName("percentage", effect->percentage, i);
		}
	}

	if (packet->GetVersion() < 1096) {
		packet->setSubstructDataByName("header_info", "adornment_id", 0xFFFFFFFF); // Send no ID for now
		packet->setSubstructDataByName("header_info", "unknown3", 0xFFFFFFFF);
	}
	packet->setSubstructDataByName("header_info", "unknown21", 0x00000000);
	packet->setSubstructDataByName("header_info", "condition", generic_info.condition);
	packet->setSubstructDataByName("header_info", "weight", generic_info.weight);
	if (packet->GetVersion() <= 373) { //orig client only has one skill
		if (generic_info.skill_req1 == 0 || generic_info.skill_req1 == 0xFFFFFFFF) {
			if (generic_info.skill_req2 != 0 && generic_info.skill_req2 != 0xFFFFFFFF) {
				packet->setSubstructDataByName("header_info", "skill_req1", generic_info.skill_req2);
			}
			else {
				packet->setSubstructDataByName("header_info", "skill_req1", 0xFFFFFFFF);
			}
		}
		else {
			packet->setSubstructDataByName("header_info", "skill_req1", generic_info.skill_req1);
		}
	}
	else {
		if (generic_info.skill_req1 == 0)
			packet->setSubstructDataByName("header_info", "skill_req1", 0xFFFFFFFF);
		else
			packet->setSubstructDataByName("header_info", "skill_req1", generic_info.skill_req1);
		if (generic_info.skill_req2 == 0)
			packet->setSubstructDataByName("header_info", "skill_req2", 0xFFFFFFFF);
		else
			packet->setSubstructDataByName("header_info", "skill_req2", generic_info.skill_req2);
	}
	if(generic_info.skill_min != 0)
		packet->setSubstructDataByName("header_info", "skill_min", generic_info.skill_min);
	if (client->GetVersion() <= 373) {
		string flags;
		if (CheckFlag(NO_TRADE))
			flags += "NO-TRADE   ";
		if (CheckFlag(NO_VALUE))
			flags += "NO-VALUE   ";
		if(flags.length() > 0)
			packet->setSubstructDataByName("header_info", "flag_names", flags.c_str());
	}
	if (generic_info.adventure_classes > 0 || generic_info.tradeskill_classes > 0 || item_level_overrides.size() > 0) {
		//int64 classes = 0;
		int16 tmp_level = 0;
		map<int8, int16> adv_class_levels;
		map<int8, int16> tradeskill_class_levels;
		map<int8, int16>::iterator itr;
		int64 tmpVal = 0;
		int8 temp = ASSASSIN;
		// AoD + clients with beastlords
		if (packet->GetVersion() >= 1142)
			temp += 2;

		// Chaneler class, get a more accurate version
		if (packet->GetVersion() >= 60000)
			temp += 2;

		for (int32 i = 0; i <= temp; i++) {
			tmpVal = (int64)pow(2.0, (double)i);
			if ((generic_info.adventure_classes & tmpVal)) {
				//classes += 2 << (i - 1);
				classes += tmpVal;
				tmp_level = GetOverrideLevel(i, 255);
				if (tmp_level == 0)
					adv_class_levels[i] = generic_info.adventure_default_level;
				else
					adv_class_levels[i] = tmp_level;
			}
			if (tmpVal == 0) {
				if (packet->GetVersion() >= 60000)
					classes = 576379072454289112;
				else if (packet->GetVersion() >= 57048)
					classes = 6281081087704;
				else if (packet->GetVersion() >= 1142)
					classes = 144095080877876952;
				else
					classes = 36024082983773912;
			}
		}
		for (int i = ALCHEMIST + 1 - ARTISAN; i >= 0; i--) {
			//tmpVal = 2 << i;
			tmpVal = (int64)pow(2.0, (double)i);
			if ((generic_info.tradeskill_classes & tmpVal)) {
				classes += pow(2, (i + ARTISAN));
				//classes += 2 << (i+ARTISAN-1);
				tmp_level = GetOverrideLevel(i, 255);
				if (tmp_level == 0)
					tradeskill_class_levels[i] = generic_info.tradeskill_default_level;
				else
					tradeskill_class_levels[i] = tmp_level;
			}
		}
		if (client->GetVersion() <= 561) { //simplify display (if possible)
			map<int8, int16> new_adv_class_levels;
			for (int i = 1; i <= 31; i += 10) {
				bool add_archetype = CheckArchetypeAdvClass(i, &adv_class_levels);
				if (add_archetype) {
					new_adv_class_levels[i] = 0;	
				}
				else {
					for (int x = 1; x <= 7; x += 3) {
						if (CheckArchetypeAdvSubclass(i+x, &adv_class_levels)) {
							new_adv_class_levels[i+x] = 0;
						}
					}
				}
			}
			if (new_adv_class_levels.size() > 0) {
				int8 i = 0;
				for (itr = new_adv_class_levels.begin(); itr != new_adv_class_levels.end(); itr++) {
					i = itr->first;
					if ((i % 10) == 1) {
						int16 level = 0;
						for (int x = i; x < i+10; x++) {
							if (adv_class_levels.count(x) > 0) {
								if(level == 0)
									level = adv_class_levels.at(x);
								adv_class_levels.erase(x);
							}
						}
						adv_class_levels[i] = level;
					}
					else {
						int16 level = 0;
						for (int x = i+1; x < i + 3; x++) {
							if (adv_class_levels.count(x) > 0) {
								if (level == 0)
									level = adv_class_levels.at(x);
								adv_class_levels.erase(x);
							}
						}
						adv_class_levels[i] = level;
					}
				}
			}			
		}
		packet->setSubstructArrayLengthByName("header_info", "class_count", adv_class_levels.size() + tradeskill_class_levels.size());
		int i = 0;
		for (itr = adv_class_levels.begin(); itr != adv_class_levels.end(); itr++, i++) {
			packet->setArrayDataByName("adventure_class", itr->first, i);
			packet->setArrayDataByName("tradeskill_class", 255, i);
			packet->setArrayDataByName("level", itr->second * 10, i);
		}
		for (itr = tradeskill_class_levels.begin(); itr != tradeskill_class_levels.end(); itr++, i++) {
			packet->setArrayDataByName("adventure_class", 255, i);
			packet->setArrayDataByName("tradeskill_class", itr->first, i);
			packet->setArrayDataByName("level", itr->second * 10, i);
		}
		packet->setSubstructDataByName("footer", "required_classes", classes);
	}
	else {
		if (packet->GetVersion() >= 60000)
			classes = 576379072454289112;
		else if (packet->GetVersion() >= 57048)
			classes = 6281081087704;
		else if (packet->GetVersion() >= 1142)
			classes = 144095080877876952;
		else
			classes = 36024082983773912;
		packet->setSubstructDataByName("footer", "required_classes", classes);
	}

	// Is this a copy and paste error???

	
		packet->setSubstructDataByName("footer", "required_classes2", classes);
	
	{
		if (packet->GetVersion() >= 60000)
			classes = 576379072454289112;
		else if (packet->GetVersion() >= 57048)
			classes = 6281081087704;
		else if (packet->GetVersion() >= 1142)
			classes = 144095080877876952;
		else
			classes = 36024082983773912;
		
	}
	if (client->GetVersion() <= 373 && generic_info.adventure_default_level > 0) {
		packet->setSubstructDataByName("header_info", "skill_min", (generic_info.adventure_default_level-1)*5+1);
		packet->setSubstructDataByName("header_info", "skill_recommended", details.recommended_level * 5);
	}
	packet->setSubstructDataByName("footer", "recommended_level", details.recommended_level);
	if(generic_info.adventure_default_level > 0){
		packet->setSubstructDataByName("footer", "required_level", generic_info.adventure_default_level);
		packet->setSubstructDataByName("footer", "footer_unknown2", 0);// remove defualt
	}
	else{
		packet->setSubstructDataByName("footer", "required_level", generic_info.tradeskill_default_level * 10);		
		packet->setSubstructDataByName("footer", "footer_unknown2", 0);//remove default
	}
	if(slot_data.size() > 0){
		packet->setSubstructArrayLengthByName("header_info", "slot_count", slot_data.size());
		for(int32 i=0;i<slot_data.size();i++){
			int8 slot = slot_data[i];
			if (client->GetVersion() <= 373) {
				if (slot > EQ2_EARS_SLOT_1 && slot <= EQ2_WAIST_SLOT) //they added a second ear slot later, adjust for only 1 original slot
					slot -= 1;
				else if (slot == EQ2_FOOD_SLOT)
					slot = EQ2_ORIG_FOOD_SLOT;
				else if(slot == EQ2_DRINK_SLOT)
					slot = EQ2_ORIG_DRINK_SLOT;
			}
			else if (client->GetVersion() <= 561) {
				if (slot > EQ2_EARS_SLOT_1 && slot <= EQ2_WAIST_SLOT) //they added a second ear slot later, adjust for only 1 original slot
					slot -= 1;
				else if (slot == EQ2_FOOD_SLOT)
					slot = EQ2_DOF_FOOD_SLOT;
				else if (slot == EQ2_DRINK_SLOT)
					slot = EQ2_DOF_DRINK_SLOT;
				else if (slot == EQ2_CHARM_SLOT_1)
					slot = EQ2_DOF_CHARM_SLOT_1;
				else if (slot == EQ2_CHARM_SLOT_2)
					slot = EQ2_DOF_CHARM_SLOT_2;
			}
			packet->setArrayDataByName("slot", slot, i);
		}
	}
	if(!loot_item && !inspect){
		if (adornment_info)
			LogWrite(ITEM__DEBUG, 0, "Items", "\ttype: %i, Duration: %i, item_types_: %i, slot_type: %i", generic_info.item_type, adornment_info->duration, adornment_info->item_types, adornment_info->slot_type);

		int8 tmpType = generic_info.item_type;
		if (client->GetVersion() <= 373 && generic_info.item_type > ITEM_TYPE_RECIPE)
			tmpType = 0;
		else if(client->GetVersion() <= 561 && (generic_info.item_type > ITEM_TYPE_HOUSE || generic_info.item_type == ITEM_TYPE_BAUBLE))
			tmpType = 0;
		
		packet->setSubstructDataByName("header", "item_type", tmpType);
		switch(generic_info.item_type){
			case ITEM_TYPE_WEAPON:{
				if(weapon_info){
					if (client->GetVersion() < 373) {
						packet->setSubstructDataByName("details", "wield_type", weapon_info->wield_type);
						packet->setSubstructDataByName("details", "damage_low1", weapon_info->damage_low1);
						packet->setSubstructDataByName("details", "damage_high1", weapon_info->damage_high1);
						packet->setSubstructDataByName("details", "damage_low2", weapon_info->damage_low2);
						packet->setSubstructDataByName("details", "damage_high2", weapon_info->damage_high2);
						packet->setSubstructDataByName("details", "damage_type", weapon_type);
						packet->setSubstructDataByName("details", "delay", weapon_info->delay);
					}
					else {
						packet->setDataByName("wield_type", weapon_info->wield_type);
						packet->setDataByName("damage_low1", weapon_info->damage_low1);
						packet->setDataByName("damage_high1", weapon_info->damage_high1);
						packet->setDataByName("damage_low2", weapon_info->damage_low2);
						packet->setDataByName("damage_high2", weapon_info->damage_high2);
						packet->setDataByName("damage_low3", weapon_info->damage_low3);
						packet->setDataByName("damage_high3", weapon_info->damage_high3);
						packet->setDataByName("damage_type", weapon_type);
						packet->setDataByName("delay", weapon_info->delay);
						packet->setDataByName("rating", weapon_info->rating);
					}
				}
				break;
			}
			case ITEM_TYPE_RANGED:{
				if(ranged_info){
					if (client->GetVersion() < 373) {
						packet->setSubstructDataByName("details", "damage_low1", ranged_info->weapon_info.damage_low1);
						packet->setSubstructDataByName("details", "damage_high1", ranged_info->weapon_info.damage_high1);
						packet->setSubstructDataByName("details", "damage_low2", ranged_info->weapon_info.damage_low2);
						packet->setSubstructDataByName("details", "damage_high2", ranged_info->weapon_info.damage_high2);
						packet->setSubstructDataByName("details", "delay", ranged_info->weapon_info.delay);
						packet->setSubstructDataByName("details", "range_low", ranged_info->range_low);
						packet->setSubstructDataByName("details", "range_high", ranged_info->range_high);
					}
					else {
						packet->setDataByName("damage_low1", ranged_info->weapon_info.damage_low1);
						packet->setDataByName("damage_high1", ranged_info->weapon_info.damage_high1);
						packet->setDataByName("damage_low2", ranged_info->weapon_info.damage_low2);
						packet->setDataByName("damage_high2", ranged_info->weapon_info.damage_high2);
						packet->setDataByName("damage_low3", ranged_info->weapon_info.damage_low3);
						packet->setDataByName("damage_high3", ranged_info->weapon_info.damage_high3);
						packet->setDataByName("delay", ranged_info->weapon_info.delay);
						packet->setDataByName("range_low", ranged_info->range_low);
						packet->setDataByName("range_high", ranged_info->range_high);
						packet->setDataByName("rating", ranged_info->weapon_info.rating);
					}
				}
				break;
			}
			case ITEM_TYPE_SHIELD:
			case ITEM_TYPE_ARMOR:{
				if(armor_info){
					if (client->GetVersion() < 373) {
						packet->setSubstructDataByName("details", "mitigation_low", armor_info->mitigation_low);
						packet->setSubstructDataByName("details", "mitigation_high", armor_info->mitigation_high);
					}
					else {
						packet->setDataByName("mitigation_low", armor_info->mitigation_low);
						packet->setDataByName("mitigation_high", armor_info->mitigation_high);
					}
				}
				break;
			}
			case ITEM_TYPE_BAG:{
				if(bag_info){
					
					int8 max_slots = player->GetMaxBagSlots(client->GetVersion());
					if (bag_info->num_slots > max_slots)
						bag_info->num_slots = max_slots;
				
						int16 free_slots = bag_info->num_slots;
						if (player) {
							Item* bag = player->GetPlayerItemList()->GetItemFromUniqueID(details.unique_id, true);
							if (bag && bag->IsBag()) {
								vector<Item*>* bag_items = player->GetPlayerItemList()->GetItemsInBag(bag);
								if (bag_items->size() > bag->bag_info->num_slots) {
									free_slots = 0;
									packet->setArrayLengthByName("num_names", bag->bag_info->num_slots);
								}
								else {
									free_slots = bag->bag_info->num_slots - bag_items->size();
									packet->setArrayLengthByName("num_names", bag_items->size());
								}
								vector<Item*>::iterator itr;
								int16 i = 0;
								Item* tmp_bag_item = 0;
								for (itr = bag_items->begin(); itr != bag_items->end(); itr++) {
									tmp_bag_item = *itr;
									if (tmp_bag_item && tmp_bag_item->details.slot_id < bag->bag_info->num_slots) {
										packet->setArrayDataByName("item_name", tmp_bag_item->name.c_str(), i);
										i++;
									}
								}
								safe_delete(bag_items);
							}
						}
						packet->setDataByName("num_slots", bag_info->num_slots);
						packet->setDataByName("num_empty", free_slots);
						packet->setDataByName("weight_reduction", bag_info->weight_reduction);
						packet->setDataByName("item_score", 2);
						//packet->setDataByName("unknown5", 0x1e50a86f);
						//packet->setDataByName("unknown6", 0x2c17f61d);
						//1 armorer
						//2 weaponsmith
						//4 tailor
						//16 jeweler 
						//32 sage
						//64 alchemist
						//120 all scholars
						//250 all craftsman
						//int8 blah[] = {0x00,0x00,0x01,0x01,0xb6,0x01,0x01};
						//int8 blah[] = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
						int8 blah[] = { 0xd8,0x66,0x9b,0x6d,0xb6,0xfb,0x7f };
						for (int8 i = 0; i < sizeof(blah); i++)
							packet->setSubstructDataByName("footer", "footer_unknown_0", blah[i], 0, i);
				}
				break;
			}
			case ITEM_TYPE_FOOD:{
				if(food_info && client->GetVersion() >=374){
					packet->setDataByName("food_type", food_info->type);
					packet->setDataByName("level", food_info->level);
					packet->setDataByName("duration", food_info->duration);
				}
				break;
			}
			case ITEM_TYPE_SKILL:{
				//Spell Books
				if(skill_info->spell_id > 0){
					Spell* spell = master_spell_list.GetSpell(skill_info->spell_id, skill_info->spell_tier);
					if(spell){
						if(player && client->GetVersion() >= 374) {
							packet->setSubstructDataByName("header_info", "footer_type", 0);
							
							spell->SetPacketInformation(packet, client);
							if (player->HasSpell(skill_info->spell_id, skill_info->spell_tier, true)) {
								packet->setDataByName("scribed", 1);
							}

								if (packet->GetVersion() >= 927){
									if (player->HasSpell(skill_info->spell_id, skill_info->spell_tier, true)) {
										packet->setAddToPacketByName("scribed_better_version", 1);// need to confirm
									}
								}
								else
									packet->setAddToPacketByName("scribed_better_version", 0); //if not scribed
							
							// either don't require previous tier or check that we have the lower tier spells potentially
							int32 tier_up = player->GetTierUp(skill_info->spell_tier);
							if (!rule_manager.GetZoneRule(player->GetZoneID(), R_Spells, RequirePreviousTierScribe)->GetInt8() || player->HasSpell(skill_info->spell_id, tier_up, false, true))
								packet->setDataByName("require_previous", 1, 0);
							// membership required
							//packet->setDataByName("unknown_1188_2_MJ", 1, 1);
							
						}
						else {							
							spell->SetPacketInformation(packet, client);
						}
						//packet->setDataByName("unknown26", 0);
					}
				}
				break;
			}
			case ITEM_TYPE_BAUBLE:{
				if(bauble_info && client->GetVersion() >= 546){
					packet->setDataByName("cast", bauble_info->cast);
					packet->setDataByName("recovery", bauble_info->recovery);
					packet->setDataByName("duration", bauble_info->duration);
					packet->setDataByName("recast", bauble_info->recast);
					packet->setDataByName("display_slot_optional", bauble_info->display_slot_optional);
					packet->setDataByName("display_cast_time", bauble_info->display_cast_time);
					packet->setDataByName("display_bauble_type", bauble_info->display_bauble_type);
					packet->setDataByName("effect_radius", bauble_info->effect_radius);
					packet->setDataByName("max_aoe_targets", bauble_info->max_aoe_targets);
					packet->setDataByName("display_until_cancelled", bauble_info->display_until_cancelled);
					//packet->setDataByName("item_score", 1);
				}
				break;
			}
			case ITEM_TYPE_THROWN:{
				if(thrown_info && client->GetVersion() >= 374){
					packet->setDataByName("range", thrown_info->range);
					packet->setDataByName("damage_modifier", thrown_info->damage_modifier);
					packet->setDataByName("hit_bonus", thrown_info->hit_bonus);
					packet->setDataByName("damage_type", thrown_info->damage_type);
				}
				break;
			}
			case ITEM_TYPE_HOUSE:{
				if(houseitem_info && client->GetVersion() >= 374){
					packet->setDataByName("status_rent_reduction", houseitem_info->status_rent_reduction);
					packet->setDataByName("coin_rent_reduction", houseitem_info->coin_rent_reduction);
					packet->setDataByName("house_only", houseitem_info->house_only);
				}
				break;
			}
			case ITEM_TYPE_BOOK:{
				if(book_info && client->GetVersion() >= 374){
					packet->setDataByName("language", book_info->language);
					packet->setMediumStringByName("author", book_info->author.data.c_str());
					packet->setMediumStringByName("title", book_info->title.data.c_str());
				}
				if (packet->GetVersion() <= 1096) packet->setDataByName("item_type", 13);

				break;
			}
			case ITEM_TYPE_RECIPE:{
				// Recipe Books
				if(recipebook_info){
					packet->setArrayLengthByName("num_recipes", recipebook_info->recipes.size());
					for (int32 i = 0; i < recipebook_info->recipes.size(); i++) {
						Recipe* recipe = master_recipe_list.GetRecipeByCRC(recipebook_info->recipes.at(i));
						if (recipe) {
							packet->setArrayDataByName("recipe_name", recipe->GetName(), i);
							packet->setArrayDataByName("recipe_id", recipe->GetID(), i);
							packet->setArrayDataByName("recipe_icon", recipe->GetIcon(), i);
						}
					}
					packet->setDataByName("uses", recipebook_info->uses);
					if(player->GetRecipeBookList()->HasRecipeBook(recipebook_info->recipe_id))
						packet->setDataByName("scribed", 1);
					else
						packet->setDataByName("scribed", 0);
				}
				break;
			}
			case ITEM_TYPE_ADORNMENT:{
				//Adornements
				if (client->GetVersion() >= 374) {
					packet->setDataByName("item_types", adornment_info->item_types);
					packet->setDataByName("duration", adornment_info->duration); // need to calcualte for remaining duration
					packet->setDataByName("slot_type", adornment_info->slot_type);
					packet->setDataByName("footer_set_name", "test footer set name");
					packet->setArrayLengthByName("footer_set_bonus_list_count", 1);// list of the bonus items
					packet->setArrayDataByName("footer_set_bonus_items_needed", 2, 0);  //this is nember of items needed for granteing that  stat //name,value,array
					packet->setSubArrayLengthByName("footer_set_bonus_stats_count", 2, 0);//name,value,array,subarray
					packet->setSubArrayDataByName("set_stat_type", 5, 0, 0);
					packet->setSubArrayDataByName("set_stat_subtype", 1, 0, 0);
					packet->setSubArrayDataByName("set_value", 25000, 0, 0);
				}
						
			}			
			case ITEM_TYPE_HOUSE_CONTAINER:{
				if(houseitem_info && client->GetVersion() >= 374){
					packet->setDataByName("status_rent_reduction", houseitem_info->status_rent_reduction);
					packet->setDataByName("coin_rent_reduction", houseitem_info->coin_rent_reduction);
					packet->setDataByName("house_only", houseitem_info->house_only);
				}
				//House Containers
				if(housecontainer_info && client->GetVersion() >= 374){
					packet->setDataByName("allowed_types", housecontainer_info->allowed_types);
					packet->setDataByName("num_slots", housecontainer_info->num_slots);
					packet->setDataByName("broker_commission", housecontainer_info->broker_commission);
					packet->setDataByName("fence_commission", housecontainer_info->fence_commission);
				}
				if(bag_info){
					int8 max_slots = player->GetMaxBagSlots(client->GetVersion());
					if (bag_info->num_slots > max_slots)
						bag_info->num_slots = max_slots;
				
						int16 free_slots = bag_info->num_slots;
						if (player) {
							Item* bag = player->GetPlayerItemList()->GetItemFromUniqueID(details.unique_id, true);
							if (bag && bag->IsBag()) {
								vector<Item*>* bag_items = player->GetPlayerItemList()->GetItemsInBag(bag);
								if (bag_items->size() > bag->bag_info->num_slots) {
									free_slots = 0;
									packet->setArrayLengthByName("num_names", bag->bag_info->num_slots);
								}
								else {
									free_slots = bag->bag_info->num_slots - bag_items->size();
									packet->setArrayLengthByName("num_names", bag_items->size());
								}
								vector<Item*>::iterator itr;
								int16 i = 0;
								Item* tmp_bag_item = 0;
								for (itr = bag_items->begin(); itr != bag_items->end(); itr++) {
									tmp_bag_item = *itr;
									if (tmp_bag_item && tmp_bag_item->details.slot_id < bag->bag_info->num_slots) {
										packet->setArrayDataByName("item_name", tmp_bag_item->name.c_str(), i);
										i++;
									}
								}
								safe_delete(bag_items);
							}
						}
						packet->setDataByName("num_slots", bag_info->num_slots);
						packet->setDataByName("num_empty", free_slots);
						packet->setDataByName("weight_reduction", bag_info->weight_reduction);
						packet->setDataByName("item_score", 2);
						//packet->setDataByName("unknown5", 0x1e50a86f);
						//packet->setDataByName("unknown6", 0x2c17f61d);
						//1 armorer
						//2 weaponsmith
						//4 tailor
						//16 jeweler 
						//32 sage
						//64 alchemist
						//120 all scholars
						//250 all craftsman
						//int8 blah[] = {0x00,0x00,0x01,0x01,0xb6,0x01,0x01};
						//int8 blah[] = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
						int8 blah[] = { 0xd8,0x66,0x9b,0x6d,0xb6,0xfb,0x7f };
						for (int8 i = 0; i < sizeof(blah); i++)
							packet->setSubstructDataByName("footer", "footer_unknown_0", blah[i], 0, i);
				}
			}
		}
	}

	LogWrite(MISC__TODO, 1, "TODO", "Item Set information\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);
	if (IsBauble()) {
		packet->setSubstructDataByName("footer", "stack_size", stack_count);
	}
	else {
		packet->setSubstructDataByName("footer", "stack_size", stack_count);
	}
	packet->setSubstructDataByName("footer", "collectable", generic_info.collectable);
	
	
	
	
	
	packet->setSubstructDataByName("footer", "status_item", sell_status);
	

	LogWrite(MISC__TODO, 1, "TODO", "Set collection_needed information properly\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);

	packet->setSubstructDataByName("footer", "collection_needed", player->GetCollectionList()->NeedsItem(this) ? 1 : 0);

	if(generic_info.offers_quest_id > 0){
		Quest* quest = master_quest_list.GetQuest(generic_info.offers_quest_id, false);
		if(quest){
			packet->setSubstructDataByName("footer", "offers_quest", strlen(generic_info.offers_quest_name) ? generic_info.offers_quest_name : quest->GetName());
			packet->setSubstructDataByName("footer", "offers_quest_color", player->GetArrowColor(quest->GetQuestLevel()));
		}
	}
	if(generic_info.part_of_quest_id > 0){
		Quest* quest = master_quest_list.GetQuest(generic_info.part_of_quest_id, false);
		if(quest){
			packet->setSubstructDataByName("footer", "part_of_quest", strlen(generic_info.required_by_quest_name) ? generic_info.required_by_quest_name : quest->GetName());
			packet->setSubstructDataByName("footer", "part_of_quest_color", player->GetArrowColor(quest->GetQuestLevel()));
		}
	}
	if(generic_info.max_charges > 0){
		packet->setSubstructDataByName("footer", "charges", 1);
		packet->setSubstructDataByName("footer", "total_charges", generic_info.max_charges);
		packet->setSubstructDataByName("footer", "charges_left", details.count);
		packet->setSubstructDataByName("footer", "display_charges", generic_info.display_charges);
	}
	if ((packet->GetVersion() >= 63119) || packet->GetVersion() == 61331){
		if (sell_status > 0){

		}
	}
	//packet->setSubstructDataByName("footer", "status_item", 0);
	
	if (IsHarvest()){
		packet->setSubstructDataByName("footer", "crafting_flag", 1);


		
	}
	
	// Set these to 0 for now
	if(packet->GetVersion() >= 1188){
		packet->setSubstructDataByName("footer", "locked_flag", 0);
		packet->setSubstructDataByName("footer", "account_retricted", 0);
	}

	// Adorns, set all to FF for now
	if (packet->GetVersion() >= 1096) {// changed to 1096 for dov from 1188
		packet->setSubstructDataByName("footer", "adorn_slots", 0xFF, 0, 0);
		packet->setSubstructDataByName("footer", "adorn_slots", 0xFF, 0, 1);
		packet->setSubstructDataByName("footer", "adorn_slots", 0xFF, 0, 2);
		packet->setSubstructDataByName("footer", "adorn_slots", 0xFF, 0, 3);
		packet->setSubstructDataByName("footer", "adorn_slots", 0xFF, 0, 4);
		packet->setSubstructDataByName("footer", "adorn_slots", 0xFF, 0, 5);
		
	}
	if (packet->GetVersion() >= 1289) {// at some point after this there are 10 adornment slots all FF for now but will skip this if not needed for a version
		
		packet->setSubstructDataByName("footer", "adorn_slots", 0xFF, 0, 6);
		packet->setSubstructDataByName("footer", "adorn_slots", 0xFF, 0, 7);
		packet->setSubstructDataByName("footer", "adorn_slots", 0xFF, 0, 8);
		packet->setSubstructDataByName("footer", "adorn_slots", 0xFF, 0, 9);
		packet->setSubstructDataByName("footer", "adorn_slots", 0xFF, 0, 10);
	}


	packet->setSubstructDataByName("footer", "name", name.c_str());
	packet->setSubstructDataByName("footer", "description", description.c_str());

	LogWrite(ITEM__PACKET, 0, "Items", "Dump/Print Packet in func: %s, line: %i", __FUNCTION__, __LINE__);
#if EQDEBUG >= 9
	packet->PrintPacket();
#endif

}

PacketStruct* Item::PrepareItem(int16 version, bool merchant_item, bool loot_item, bool inspection){
	PacketStruct* packet = 0;

	if(loot_item && version > 561)
		packet = configReader.getStruct("WS_LootItemGeneric", version);
	else if(!inspection && loot_item && version <= 561) {
		packet = configReader.getStruct("WS_ItemGeneric", version);
		packet->AddFlag("loot");
	}
	else if(inspection && version <= 373) {
			packet = configReader.getStruct("WS_ItemInspect", version);
	} 
	else if(version <= 561 && (generic_info.item_type > ITEM_TYPE_HOUSE || generic_info.item_type == ITEM_TYPE_BAUBLE)) {
			packet = configReader.getStruct("WS_ItemGeneric", version);
	}
	else{
		int8 tmpType = generic_info.item_type;
		if (version <= 373 && generic_info.item_type > ITEM_TYPE_RECIPE)
			tmpType = 0;
		else if(version <= 561 && (generic_info.item_type > ITEM_TYPE_HOUSE || generic_info.item_type == ITEM_TYPE_BAUBLE))
			tmpType = 0;
		
		switch(tmpType){
			case ITEM_TYPE_WEAPON:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemWeapon", version);
				else
					packet = configReader.getStruct("WS_ItemWeapon", version);
				break;
			}
			case ITEM_TYPE_RANGED:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemRange", version);
				else
					packet = configReader.getStruct("WS_ItemRange", version);
				break;
			}
			case ITEM_TYPE_SHIELD:{
				if (merchant_item)
					packet = configReader.getStruct("WS_MerchantItemShield", version);
				else
					packet = configReader.getStruct("WS_ItemShield", version);
				break;
			}
			case ITEM_TYPE_ARMOR:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemArmor", version);
				else
					packet = configReader.getStruct("WS_ItemArmor", version);
				break;
			}
			case ITEM_TYPE_BAG:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemBag", version);
				else
					packet = configReader.getStruct("WS_ItemBag", version);
				break;
			}
			case ITEM_TYPE_BOOK:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemBook", version);
				else
					packet = configReader.getStruct("WS_ItemBook", version);
				break;
			}
			case ITEM_TYPE_SKILL:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemSkill", version);
				else
					packet = configReader.getStruct("WS_ItemSkill", version);
				break;
			}
			case ITEM_TYPE_RECIPE:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemRecipeBook", version);
				else
					packet = configReader.getStruct("WS_ItemRecipeBook", version);
				break;
			}
			case ITEM_TYPE_FOOD:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemFood", version);
				else
					packet = configReader.getStruct("WS_ItemFood", version);
				break;
			}
			case ITEM_TYPE_BAUBLE:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemBauble", version);
				else
					packet = configReader.getStruct("WS_ItemBauble", version);
				break;
			}
			case ITEM_TYPE_ITEMCRATE:{
				if (merchant_item)
					packet = configReader.getStruct("WS_MerchantItemSet", version);
				else
					packet = configReader.getStruct("WS_ItemSet", version);
				break;
			}
			case ITEM_TYPE_HOUSE:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemHouse", version);
				else
					packet = configReader.getStruct("WS_ItemHouse", version);
				break;
			}
			case ITEM_TYPE_THROWN:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemThrown", version);
				else
					packet = configReader.getStruct("WS_ItemThrown", version);
				break;
			}
			case ITEM_TYPE_HOUSE_CONTAINER:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemHouseContainer", version);
				else
					packet = configReader.getStruct("WS_ItemHouseContainer", version);
				break;
			}
			case ITEM_TYPE_ADORNMENT:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantAdornment", version);
				else
					packet = configReader.getStruct("WS_ItemAdornment", version);
				break;
			}
			default:{
				if(merchant_item)
					packet = configReader.getStruct("WS_MerchantItemGeneric", version);
				else
					packet = configReader.getStruct("WS_ItemGeneric", version);
			}
		}
		if (packet && loot_item)
			packet->AddFlag("loot");
	}
	if(!packet){
		LogWrite(ITEM__ERROR, 0, "Item", "Unhandled Item type: %i", (int)generic_info.item_type);
		return 0;
	}
	return packet;
}

EQ2Packet* Item::serialize(int16 version, bool show_name, Player* player, bool include_twice, int16 packet_type, int8 subtype, bool merchant_item, bool loot_item, bool inspect){
	PacketStruct* packet = PrepareItem(version, merchant_item, loot_item, inspect);
	if(!packet)
		return 0;
	if (version <= 561) {
		include_twice = false;
		packet_type = 0;
	}
	if(include_twice && IsBag() == false && IsBauble() == false && IsFood() == false)
		serialize(packet, show_name, player, packet_type, 0x80, loot_item, inspect);
	else
		serialize(packet, show_name, player, packet_type, 0, loot_item, inspect);
	if(merchant_item)
		packet->setSubstructDataByName("header_info", "unique_id", 0xFFFFFFFF);
	string* generic_string_data = packet->serializeString();

	//packet->PrintPacket();
	//LogWrite(ITEM__DEBUG, 9, "Items", "generic_string_data:");	
	//DumpPacket((uchar*)generic_string_data->c_str(), generic_string_data->length());

	int32 size = generic_string_data->length();
	if(include_twice && IsBag() == false && IsBauble() == false && IsFood() == false)
		size = (size*2)-13;
	uchar* out_data = new uchar[size+1];
	uchar* out_ptr = out_data;
	memcpy(out_ptr, (uchar*)generic_string_data->c_str(), generic_string_data->length());
	out_ptr += generic_string_data->length();
	if(include_twice && IsBag() == false && IsBauble() == false && IsFood() == false){
		memcpy(out_ptr, (uchar*)generic_string_data->c_str() + 13, generic_string_data->length() -13);
	}
	int32 size2 = size;
	if (version <= 373) {
		uchar* out_ptr2 = out_data;
		if (size2 >= 0xFF) {
			size2 -= 3;			
			out_ptr2[0] = 0xFF;
			out_ptr2 += sizeof(int8);
			memcpy(out_ptr2, &size2, sizeof(int16));
		}
		else {
			size2 -= 1;
			out_ptr2[0] = size2;
		}
	}
	else {
		size2 -= 4;
		memcpy(out_data, &size2, sizeof(int32));
	}
	EQ2Packet* outapp = new EQ2Packet(OP_ClientCmdMsg, out_data, size);
	//DumpPacket(outapp);
	safe_delete(packet);
	safe_delete_array(out_data);
	return outapp;
}

void Item::SetAppearance(ItemAppearance* appearance){
	SetAppearance(appearance->type, appearance->red, appearance->green, appearance->blue, appearance->highlight_red, appearance->highlight_green, appearance->highlight_blue);
}

void Item::SetAppearance(int16 type, int8 red, int8 green, int8 blue, int8 highlight_red, int8 highlight_green, int8 highlight_blue){
	generic_info.appearance_id = type;
	generic_info.appearance_red = red;
	generic_info.appearance_green = green;
	generic_info.appearance_blue = blue;
	generic_info.appearance_highlight_red = highlight_red;
	generic_info.appearance_highlight_green = highlight_green;
	generic_info.appearance_highlight_blue = highlight_blue;
}

void Item::AddEffect(string effect, int8 percentage, int8 subbulletflag){
	ItemEffect* item_effect = new ItemEffect;
	item_effect->subbulletflag = subbulletflag;
	item_effect->effect.data = effect;
	item_effect->effect.size = effect.length();
	item_effect->percentage = percentage;
	item_effects.push_back(item_effect);
}
void Item::AddBookPage(int8 page, string page_text, int8 valign, int8 halign) {
	 BookPage * bookpage = new BookPage;
	 bookpage->page = page;
	 bookpage->page_text.data = page_text;
	 bookpage->page_text.size = page_text.length();
	 bookpage->valign = valign;
	 bookpage->halign = halign;
	 book_pages.push_back(bookpage);
}
void Item::AddLevelOverride(ItemLevelOverride* level_override){
	AddLevelOverride(level_override->adventure_class, level_override->tradeskill_class, level_override->level);
}

void Item::AddLevelOverride(int8 adventure_class, int8 tradeskill_class, int16 level){
	ItemLevelOverride* item_override = new ItemLevelOverride;
	item_override->adventure_class = adventure_class;
	item_override->tradeskill_class = tradeskill_class;
	item_override->level = level;
	item_level_overrides.push_back(item_override);
}

void Item::AddSlot(int8 slot_id){
	slot_data.push_back(slot_id);
}

void Item::SetWeaponType(int8 type){
	weapon_type = type;
}

int8 Item::GetWeaponType(){
	return weapon_type;
}

int32 Item::GetMaxSellValue(){
	return max_sell_value;
}

void Item::SetMaxSellValue(int32 val){
	max_sell_value = val;
}

void Item::SetItemScript(string name){
	item_script = name;
}

const char* Item::GetItemScript(){
	if(item_script.length() > 0)
		return item_script.c_str();
	return 0;
}

int32 Item::CalculateRepairCost() {
	if (generic_info.condition == 100)
		return 0;
	float repair_cost = (float)generic_info.adventure_default_level * (10.0 - ((float)generic_info.condition * 0.1));
	if (details.tier == ITEM_TAG_LEGENDARY)
		repair_cost *= 4;
	else if (details.tier == ITEM_TAG_FABLED)
		repair_cost *= 8;
	else if (details.tier == ITEM_TAG_MYTHICAL)
		repair_cost *= 12;
	return (int32)repair_cost;
}

PlayerItemList::PlayerItemList(){
	packet_count = 0;
	xor_packet = 0;
	orig_packet = 0;
	max_saved_index = 0;
	MPlayerItems.SetName("PlayerItemList::MPlayerItems");
}

PlayerItemList::~PlayerItemList(){
	safe_delete_array(xor_packet);
	safe_delete_array(orig_packet);
	map<sint32, map<int8, map<int16, Item*>> >::iterator bag_iter;
	map<int16, Item*>::iterator itr;
	for(bag_iter = items.begin(); bag_iter != items.end(); bag_iter++){
		for(itr = bag_iter->second[0].begin(); itr != bag_iter->second[0].end(); itr++){
			safe_delete(itr->second);
		}
		for(itr = bag_iter->second[1].begin(); itr != bag_iter->second[1].end(); itr++){
			safe_delete(itr->second);
		}
		bag_iter->second.clear();
	}
	items.clear();
	while (!overflowItems.empty()){
		safe_delete(overflowItems.back());
		overflowItems.pop_back();
	}
}

map<int32, Item*>* PlayerItemList::GetAllItems(){
	map<int32, Item*>* ret = new map<int32, Item*>;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	ret->insert(indexed_items.begin(), indexed_items.end());
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

Item* PlayerItemList::GetItemFromIndex(int32 index){
	Item* ret = 0;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if(indexed_items.count(index) > 0)
		ret = indexed_items[index];
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

Item* PlayerItemList::GetItem(sint32 bag_slot, int16 slot, int8 appearance_type){
	Item* ret = 0;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if(items.count(bag_slot) > 0 && items[bag_slot][appearance_type].count(slot) > 0)
		ret = items[bag_slot][appearance_type][slot];
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

int32 PlayerItemList::SetMaxItemIndex() {
	int32 max_index = indexed_items.size();
	int32 new_index = 0;
	map<int32, Item*>::iterator itr;
	MPlayerItems.writelock(__FUNCTION__, __LINE__);
	for(itr = indexed_items.begin();itr != indexed_items.end(); itr++){
		if(itr->first > max_index) //just grab the highest index val for next loop
			max_index = itr->first;
	}
	max_saved_index = max_index;
	MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
	
	return max_index;
}

bool PlayerItemList::AddItem(Item* item){ //is called with a slot already set
	//quick check to verify item
	if(!item)
		return false;
	else{
		if(item->details.inv_slot_id != 0){
			Item* bag = GetItemFromUniqueID(item->details.inv_slot_id, true);
			if(bag && bag->IsBag()){
				if(item->details.slot_id > bag->details.num_slots){
					LogWrite(ITEM__ERROR, 0, "Item", "Error Adding Item: Invalid slot for item unique id: %u (%s - %i), InvSlotID: %u, slotid: %u, numslots: %u", item->details.unique_id, item->name.c_str(), 
					item->details.item_id, item->details.inv_slot_id, item->details.slot_id, bag->details.num_slots);
					lua_interface->SetLuaUserDataStale(item);
					safe_delete(item);
					return false;
				}
			}
		}
	}
	int32 max_index = indexed_items.size();
	int32 new_index = 0;
	map<int32, Item*>::iterator itr;
	MPlayerItems.writelock(__FUNCTION__, __LINE__);
	for(itr = indexed_items.begin();itr != indexed_items.end(); itr++){
		if(itr->first > max_index) //just grab the highest index val for next loop
			max_index = itr->first;
	}
	
	bool doNotOverrideIndex = false;
	int32 i=0;
	for(i=0;i<max_index;i++){
		if(!indexed_items[i]){
			new_index = i;
			LogWrite(ITEM__DEBUG, 0, "Item %s assigned to %u",item->name.c_str(), i);
			item->details.new_item = false;
			item->details.new_index = 0;
			doNotOverrideIndex = true;
			break;
		}
	}
	
	if(doNotOverrideIndex) {
		if(i < max_saved_index) {
		item->details.new_item = false;
		} else {
		item->details.new_item = true;
		}
	}
	
	// may break non DoF clients
	if(!doNotOverrideIndex && new_index == 0 && max_index > 0)
		new_index = max_index;

	indexed_items[new_index] = item;
	item->details.index = new_index;
	items[item->details.inv_slot_id][item->details.appearance_type][item->details.slot_id] = item;
	MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);

	return true;
}

Item* PlayerItemList::GetBag(int8 inventory_slot, bool lock){
	Item* bag = 0;
	if(lock)
		MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if(items.count(0) > 0 && items[0][BASE_EQUIPMENT].count(inventory_slot) > 0 && items[0][BASE_EQUIPMENT][inventory_slot]->IsBag())
		bag = items[0][BASE_EQUIPMENT][inventory_slot];
	if(lock)
		MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return bag;
}

Item* PlayerItemList::GetBankBag(int8 inventory_slot, bool lock){
	Item* bag = 0;
	if(lock)
		MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if(items.count(InventorySlotType::BANK) > 0 && items[InventorySlotType::BANK][BASE_EQUIPMENT].count(inventory_slot) > 0 && items[InventorySlotType::BANK][BASE_EQUIPMENT][inventory_slot]->IsBag())
		bag = items[InventorySlotType::BANK][BASE_EQUIPMENT][inventory_slot];
	if(lock)
		MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return bag;
}

int16 PlayerItemList::GetNumberOfFreeSlots(){
	int16 count = 0;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	for(int8 i=0;i<NUM_INV_SLOTS;i++){
		if(items.count(i) == 0)
			count++;
	}
	Item* bag = 0;
	for(int8 i=0;i<NUM_INV_SLOTS;i++){
		bag = GetBag(i, false);
		if(bag && bag->details.num_slots > 0){
			if(items.count(bag->details.bag_id) > 0){
				for(int16 x=0;x<bag->details.num_slots;x++){
					if(items[bag->details.bag_id][BASE_EQUIPMENT].count(x) == 0)
						count++;
				}
			}
			else
				count += bag->bag_info->num_slots; //if the bag hasnt been used yet, add all the free slots
		}
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return count;
}

bool PlayerItemList::HasFreeBagSlot(){
	bool ret = false;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if(items.count(0) > 0){
		for(int8 i=0;i<NUM_INV_SLOTS;i++){
			if(items[0][BASE_EQUIPMENT].count(i) == 0){
				ret = true;
				break;
			}
		}
	}
	else
		ret = true;
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

bool PlayerItemList::HasFreeSlot(){
	bool ret = false;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if(items.count(0) > 0){
		for(int8 i=0;i<NUM_INV_SLOTS;i++){
			if(items[0][BASE_EQUIPMENT].count(i) == 0){
				ret = true;
				break;
			}
		}
	}
	else
		ret = true;
	if(!ret){ //inventory slots full, check bags
		Item* bag = 0;
		for(int8 i=0;!ret && i<NUM_INV_SLOTS;i++){
			bag = GetBag(i, false);
			if(bag && bag->details.num_slots > 0){
				if(items.count(bag->details.bag_id) > 0){
					for(int16 x=0;x<bag->details.num_slots;x++){
						if(items[bag->details.bag_id][BASE_EQUIPMENT].count(x) == 0){
							ret = true;
							break;
						}
					}
				}
				else{ //if the bag hasnt been used yet, then all slots are free
					ret = true;
					break;
				}
			}
		}
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

bool PlayerItemList::GetFirstFreeBankSlot(sint32* bag_id, sint16* slot) {
	bool ret = false;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if (items.count(InventorySlotType::BANK) > 0) {
		for (int8 i = 0; i < NUM_BANK_SLOTS; i++) {
			if (items[InventorySlotType::BANK][BASE_EQUIPMENT].count(i) == 0) {
				*bag_id = InventorySlotType::BANK;
				*slot = i;
				ret = true;
				break;
			}
		}
	}
	else {
		*bag_id = InventorySlotType::BANK;
		*slot = 0;
		ret = true;
	}

	if(!ret) {
		// Inventory slots were full so check bags
		Item* bag = 0;
		for(int8 i = 0; !ret && i < NUM_BANK_SLOTS; i++) {
			// Check to see if the item in the inventory slot is a bag and it has slots
			bag = GetBankBag(i, false);
			if(bag && bag->details.num_slots > 0) {
				// Item was a bag so lets loop through the slots and try to find an empty one
				if(items.count(bag->details.bag_id) > 0) {
					for(int16 x = 0; x < bag->details.num_slots; x++) {
						if(items[bag->details.bag_id][BASE_EQUIPMENT].count(x) == 0) {
							// Found a free slot, get the bag id of this bag
							*bag_id = bag->details.bag_id;
							// Get the slot
							*slot = x;
							ret = true;
							break;
						}
					}
				}
				else {
					//if the bag hasnt been used yet, then all slots are free, so set the bag_id to this bag
					// and the slot to 0 (the first slot)
					*bag_id = bag->details.bag_id;
					*slot = 0;
					ret = true;
					break;
				}
			}
		}
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

bool PlayerItemList::GetFirstFreeSlot(sint32* bag_id, sint16* slot) {
	// Mostly copy and paste from the above function
	bool ret = false;
	// Try to place the item in the normal inventory slots first
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if(items.count(0) > 0){
		for(int8 i=0; i < NUM_INV_SLOTS; i++) {
			if(items[0][BASE_EQUIPMENT].count(i) == 0) {
				// Found an empty slot, store the slot id and set the return value
				*bag_id = 0;
				*slot = i;
				ret = true;
				break;
			}
		}
	}
	else {
		// no items in the players inventory, set it to the first slot
		*bag_id = 0;
		*slot = 0;
		ret = true;
	}

	if(!ret) {
		// Inventory slots were full so check bags
		Item* bag = 0;
		for(int8 i = 0; !ret && i < NUM_INV_SLOTS; i++) {
			// Check to see if the item in the inventory slot is a bag and it has slots
			bag = GetBag(i, false);
			if(bag && bag->details.num_slots > 0) {
				// Item was a bag so lets loop through the slots and try to find an empty one
				if(items.count(bag->details.bag_id) > 0) {
					for(int16 x = 0; x < bag->details.num_slots; x++) {
						if(items[bag->details.bag_id][BASE_EQUIPMENT].count(x) == 0) {
							// Found a free slot, get the bag id of this bag
							*bag_id = bag->details.bag_id;
							// Get the slot
							*slot = x;
							ret = true;
							break;
						}
					}
				}
				else {
					//if the bag hasnt been used yet, then all slots are free, so set the bag_id to this bag
					// and the slot to 0 (the first slot)
					*bag_id = bag->details.bag_id;
					*slot = 0;
					ret = true;
					break;
				}
			}
		}
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}
vector<Item*> PlayerItemList::GetAllItemsFromID(int32 id, bool include_bank, bool lock) {
	//first check for an exact count match
	map<sint32, map<int8, map<int16, Item*>> >::iterator itr;
	map<int16, Item*>::iterator slot_itr;
	vector<Item*> ret ;
	if (lock)
		MPlayerItems.readlock(__FUNCTION__, __LINE__);
	for (itr = items.begin(); itr != items.end(); itr++) {
		if (include_bank || (!include_bank && itr->first >= 0)) {
			for (int8 i = 0; i < MAX_EQUIPMENT; i++)
			{
				for (slot_itr = itr->second[i].begin(); slot_itr != itr->second[i].end(); slot_itr++) {
					if (slot_itr->second && slot_itr->second->details.item_id == id) {
						if (lock)
							MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
						ret.push_back(slot_itr->second);
					}
				}
				
			}
			
		}
	}
	return ret;
}
Item* PlayerItemList::CanStack(Item* item, bool include_bank){
	if(!item || item->stack_count < 2)
		return 0;
	
	Item* ret = 0;
	map<sint32, map<int8, map<int16, Item*>> >::iterator itr;
	map<int16, Item*>::iterator slot_itr;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	for(itr = items.begin(); itr != items.end(); itr++){
		if(include_bank || (!include_bank && itr->first >= 0)){
			for(slot_itr=itr->second[0].begin();slot_itr!=itr->second[0].end(); slot_itr++){
				if(slot_itr->second && slot_itr->second->details.item_id == item->details.item_id && (((slot_itr->second->details.count ? slot_itr->second->details.count : 1) + (item->details.count > 0 ? item->details.count : 1)) <= slot_itr->second->stack_count)){
					ret = slot_itr->second;
					break;
				}
			}
			for(slot_itr=itr->second[1].begin();slot_itr!=itr->second[1].end(); slot_itr++){
				if(slot_itr->second && slot_itr->second->details.item_id == item->details.item_id && (((slot_itr->second->details.count ? slot_itr->second->details.count : 1) + (item->details.count > 0 ? item->details.count : 1)) <= slot_itr->second->stack_count)){
					ret = slot_itr->second;
					break;
				}
			}
		}
		if(ret)
			break;
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

void PlayerItemList::Stack(Item* orig_item, Item* item){
	if(!orig_item || !item)
		return;
	orig_item->details.count += item->details.count;
	orig_item->save_needed = true;
}

bool PlayerItemList::AssignItemToFreeSlot(Item* item, bool inventory_only){
	if(item){
		Item* orig_item = CanStack(item, !inventory_only);
		
		if(inventory_only && !IsItemInSlotType(orig_item, InventorySlotType::BASE_INVENTORY)){
			orig_item = nullptr;
		}
		if(orig_item){
			Stack(orig_item, item);
			return true;
		}
		bool use_bag_freeslot = false;
		if(item->IsBag())
			use_bag_freeslot = HasFreeBagSlot();
		MPlayerItems.writelock(__FUNCTION__, __LINE__);
		if(!use_bag_freeslot){
			Item* bag = 0;
			for(int8 i=0;i<NUM_INV_SLOTS;i++){
				bag = GetBag(i, false);
				if(bag && !item->IsBag() && bag->details.num_slots > 0){
					for(int16 x=0;x<bag->details.num_slots;x++){
						if(items[bag->details.bag_id][BASE_EQUIPMENT].count(x) == 0){
							item->details.inv_slot_id = bag->details.bag_id;
							item->details.slot_id = x;
							item->details.new_item = true;
							item->details.new_index = 0;
							MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
							bool ret = AddItem(item);
							return ret;
						}
					}
				}
			}
		}
		//bags full, check inventory slots
		for(int8 i=0;i<NUM_INV_SLOTS;i++){
			if(items[0][BASE_EQUIPMENT].count(i) == 0){
				item->details.inv_slot_id = 0;
				item->details.slot_id = i;
				item->details.new_item = true;
				item->details.new_index = 0;
				MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
				bool ret = AddItem(item);
				return ret;
			}
		}
		MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
	}
	return false;
}


void PlayerItemList::RemoveItem(Item* item, bool delete_item, bool lock){
	if(lock)
		MPlayerItems.writelock(__FUNCTION__, __LINE__);
	if(items.count(item->details.inv_slot_id) > 0 && items[item->details.inv_slot_id][item->details.appearance_type].count(item->details.slot_id) > 0){
		items[item->details.inv_slot_id][item->details.appearance_type].erase(item->details.slot_id);
		indexed_items[item->details.index] = 0;
	}
	if(item->IsBag() && item->details.inv_slot_id == 0 && item->details.slot_id < NUM_INV_SLOTS && items.count(item->details.bag_id) > 0){
		map<int16, Item*>::iterator itr;
		for(itr = items[item->details.bag_id][item->details.appearance_type].begin(); itr != items[item->details.bag_id][item->details.appearance_type].end(); itr++){
			indexed_items[itr->second->details.index] = 0;
			if(delete_item){
				if(itr->second == item) {
					item = nullptr;
				}
				lua_interface->SetLuaUserDataStale(itr->second);
				safe_delete(itr->second);
			}
		}
		items.erase(item->details.bag_id);
	}
	if(item && delete_item){
		map<int32, Item*>::iterator itr = indexed_items.find(item->details.index);
		if(itr != indexed_items.end() && item == indexed_items[item->details.index])
			indexed_items[item->details.index] = 0;
		
		lua_interface->SetLuaUserDataStale(item);
		safe_delete(item);
	}
	if(lock)
		MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
}

void PlayerItemList::DestroyItem(int16 index){
	MPlayerItems.writelock(__FUNCTION__, __LINE__);
	Item* item = indexed_items[index];
	map<int16, Item*>::iterator itr;
	if(item && item->IsBag() && item->details.inv_slot_id == 0 && item->details.slot_id < NUM_INV_SLOTS && items.count((sint32)item->details.bag_id) > 0){ //inventory
		map<int16, Item*>* tmp_map = &(items[(sint32)item->details.bag_id][item->details.appearance_type]);
		for(itr = tmp_map->begin(); itr != tmp_map->end(); itr++){
			indexed_items[itr->second->details.index] = 0;
			if(itr->second != item){
				lua_interface->SetLuaUserDataStale(itr->second);
				safe_delete(itr->second);
			}
		}
		items.erase(item->details.bag_id);
	}
	if(item) {
		if(items.count(item->details.inv_slot_id) > 0 && items[item->details.inv_slot_id][item->details.appearance_type].count(item->details.slot_id) > 0)
			items[item->details.inv_slot_id][item->details.appearance_type].erase(item->details.slot_id);
		indexed_items[index] = 0;

		vector<Item*>::iterator itr = std::find(overflowItems.begin(), overflowItems.end(), item);
		if(itr != overflowItems.end()) {
			overflowItems.erase(itr);  // avoid a dead ptr
		}
		lua_interface->SetLuaUserDataStale(item);
		
		safe_delete(item);
	}
	MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
}

void PlayerItemList::MoveItem(Item* item, sint32 inv_slot, int16 slot, int8 appearance_type, bool erase_old){
	if(erase_old && items.count(item->details.inv_slot_id) > 0 && items[item->details.inv_slot_id][BASE_EQUIPMENT].count(item->details.slot_id))
		items[item->details.inv_slot_id][BASE_EQUIPMENT].erase(item->details.slot_id);
	items[inv_slot][BASE_EQUIPMENT][slot] = item;
	item->details.inv_slot_id = inv_slot;
	item->details.slot_id = slot;
	item->details.appearance_type = 0;
	item->save_needed = true;
}

void PlayerItemList::EraseItem(Item* item){
	if(items.count(item->details.inv_slot_id) > 0 && items[item->details.inv_slot_id][BASE_EQUIPMENT].count(item->details.slot_id))
		items[item->details.inv_slot_id][BASE_EQUIPMENT].erase(item->details.slot_id);
}

int16 PlayerItemList::GetNumberOfItems(){
	int16 ret = 0;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if(items.size() > 0){
		map<sint32, map<int8, map<int16, Item*>> >::iterator itr;
		sint32 bag_id = 0;
		for(itr = items.begin(); itr != items.end(); itr++){
			bag_id = itr->first;
			if(items[bag_id].count(0))
				ret += items[bag_id][BASE_EQUIPMENT].size();
		}
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

int32 PlayerItemList::GetWeight(){
	int32 ret = 0;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	for(int16 i = 0; i < indexed_items.size(); i++){
		Item* item = indexed_items[i];
		if (item) {
			if(!IsItemInSlotType(item, InventorySlotType::BANK, false) &&
			   !IsItemInSlotType(item, InventorySlotType::SHARED_BANK, false) &&
			   !IsItemInSlotType(item, InventorySlotType::HOUSE_VAULT, false))
				ret += item->generic_info.weight;
		}
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

bool PlayerItemList::IsItemInSlotType(Item* item, InventorySlotType type, bool lockItems) {
	if(!item)
		return false;
	
	bool matchType = (item->details.inv_slot_id == type);
	if(item->details.inv_slot_id > 0) {
		Item* bagItem = GetItemFromUniqueID(item->details.inv_slot_id, true, lockItems);
		if(bagItem && bagItem->details.inv_slot_id == type)
			matchType = true;
	}
	return matchType;
}

bool PlayerItemList::MoveItem(sint32 to_bag_id, int16 from_index, sint8 to, int8 appearance_type, int8 charges){
	MPlayerItems.writelock(__FUNCTION__, __LINE__);
	Item* item_from = indexed_items[from_index];
	Item* item_to = 0;
	if(item_from && !item_from->IsItemLocked()){
		if(to_bag_id > 0){  //bag item
			Item* bag = GetItemFromUniqueID(to_bag_id, true, false);
			if(bag && !bag->IsItemLocked() && bag->details.num_slots > to && (!item_from || !item_from->IsBag()))
				item_to = items[to_bag_id][BASE_EQUIPMENT][to];
			else{
				MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
				return false;
			}
		}
		else {
			item_to = items[to_bag_id][BASE_EQUIPMENT][to];
			if(item_to && item_to->IsBag() && item_from && item_from->IsBag()) {
				MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
				return false;
			}
		}
		
		if(item_to && item_to->IsItemLocked()) {
			MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
			return false;
		}
		if(charges > 0) {
			if (item_to && item_from->details.item_id == item_to->details.item_id){
				if(item_to->details.count > 0 && item_to->details.count < item_to->stack_count){
					int32 total_tmp_price = 0;
					if((item_to->details.count + item_from->details.count) <= item_to->stack_count){
						total_tmp_price = (item_to->GetMaxSellValue()*item_to->details.count) + (item_from->GetMaxSellValue()*item_from->details.count);
						item_to->details.count += item_from->details.count;
						indexed_items[from_index] = 0;
						items[item_from->details.inv_slot_id][BASE_EQUIPMENT].erase(item_from->details.slot_id);
						item_from->needs_deletion = true;
						item_to->save_needed = true;
					}
					else{
						int8 diff = item_to->stack_count - item_to->details.count;
						total_tmp_price = (item_to->GetMaxSellValue()*item_to->details.count) + (item_from->GetMaxSellValue()*diff);
						item_to->details.count = item_to->stack_count;
						item_from->details.count -= diff;
						item_to->save_needed = true;
					}
					item_to->SetMaxSellValue(total_tmp_price/item_to->details.count);
					MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
					return true;
				}
			}
			else {
				if (item_from->details.count == charges) {
					MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
					if (item_to) 
						MoveItem(item_to, item_from->details.inv_slot_id, item_from->details.slot_id, BASE_EQUIPMENT, true);

					MoveItem(item_from, to_bag_id, to, BASE_EQUIPMENT, item_to ? false:true);
				}
				else {
					MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
					if (item_to) {
						MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
						return false;
					}
					item_from->details.count -= charges;
					Item* new_item = new Item(master_item_list.GetItem(item_from->details.item_id));
					new_item->details.count = charges;
					new_item->details.slot_id = to;
					new_item->details.inv_slot_id = to_bag_id;
					new_item->details.appearance_type = 0;
					new_item->save_needed = true;
					AddItem(new_item);
					if (item_from->details.count == 0)
						RemoveItem(item_from);
				}
				return true;
			}
		}
		else if(item_to && item_to->IsBag() && item_to->details.num_slots > 0){
			// if item we are moving is a bag
			if (item_from->IsBag() && item_from->details.num_slots > 0) {
				for (int8 i = 0; i < item_from->details.num_slots; i++) {
					// if there is something in the bag return, can't put bags with items into other bags
					if (items[item_from->details.bag_id][BASE_EQUIPMENT].count(i) != 0) {
						MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
						return false;
					}
				}
			}
			if(items.count(item_to->details.bag_id) > 0){
				for(int8 i=0;i<item_to->details.num_slots;i++){
					if(items[item_to->details.bag_id][BASE_EQUIPMENT].count(i) == 0){
						MoveItem(item_from, item_to->details.bag_id, i, 0, true);
						MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
						return true;
					}
				}
			}
			else{
				MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
				MoveItem(item_from, item_to->details.bag_id, 0, BASE_EQUIPMENT, true);
				return true;
			}
		}
		
		bool canMove = true;
		if(item_to && item_to->IsItemLocked())
			canMove = false;
		
		MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);

		if (item_to && canMove) 
			MoveItem(item_to, item_from->details.inv_slot_id, item_from->details.slot_id, BASE_EQUIPMENT, true);

		if(canMove)
			MoveItem(item_from, to_bag_id, to, BASE_EQUIPMENT, item_to ? false:true);
		
		return canMove;
	}
	MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
	return false;
}

EQ2Packet* PlayerItemList::serialize(Player* player, int16 version){
	bool firstRun = false;
	if(version <= 561 && !packet_count) {
		firstRun = true;
	}
	EQ2Packet* app = 0;
	PacketStruct* packet = configReader.getStruct("WS_UpdateInventory",version);
	Item* item = 0;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
 	if(packet && indexed_items.size() > 0){
		int8 packet_size = 0;
		int16 size = indexed_items.size();
		
		if (!firstRun && overflowItems.size() > 0)
			size++;
		
		if(size > 20 && firstRun) {
			size = 20;
		}
		PacketStruct* packet2 = configReader.getStruct("Substruct_Item", version);
		packet_size = packet2->GetTotalPacketSize();
		safe_delete(packet2);
		packet->setArrayLengthByName("item_count", size);
		if(packet_count < size){
			if(!orig_packet){
				xor_packet = new uchar[packet_size * size];
				orig_packet = new uchar[packet_size * size];
				memset(xor_packet, 0, packet_size * size);
				memset(orig_packet, 0, packet_size * size);
			}
			else{
				uchar* tmp = new uchar[packet_size * size];
				memset(tmp, 0, packet_size * size);
				memcpy(tmp, orig_packet, packet_size * packet_count);
				safe_delete_array(orig_packet);
				orig_packet = tmp;
				safe_delete_array(xor_packet);
				xor_packet = new uchar[packet_size * size];
			}
		}
		
		packet_count = size;
		
		int16 new_index = 0;
		for(int16 i = 0; i < indexed_items.size(); i++){
			item = indexed_items[i];
			if(item && item->details.new_item)
				new_index++;
			
			if(item && firstRun && i > 19) {
				item->details.new_item = true;
				continue;
			}
			
			if (item && item->details.item_id > 0)
				AddItemToPacket(packet, player, item, i, false, new_index);
			
		}

		if (!firstRun && overflowItems.size() > 0) {
			// We have overflow items, lets get the first one
			item = overflowItems.at(0);
			// Lets make sure the item is valid
			if (item && item->details.item_id > 0) {
				// Set the slot to 6 as that is what overflow requires to work
				item->details.slot_id = 6;
				// now add it to the packet
				AddItemToPacket(packet, player, item, size - 1, true);
			}
		}

		LogWrite(ITEM__PACKET, 0, "Items", "Dump/Print Packet in func: %s, line: %i", __FUNCTION__, __LINE__);
#if EQDEBUG >= 9
		packet->PrintPacket();
#endif
		packet->setDataByName("equip_flag",0);
		app = packet->serializeCountPacket(version, 1, orig_packet, xor_packet);
		safe_delete(packet);
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return app;
}

int16 PlayerItemList::GetFirstNewItem() {
	int16 new_item_slot = 0;
	for(int16 i = 0; i < indexed_items.size(); i++){
		Item* item = indexed_items[i];
		if(item && item->details.new_item) {
			return i;
		}
	}
	return 0xFFFF;
}

int16 PlayerItemList::GetNewItemByIndex(int16 in_index) {
	int16 new_item_slot = 0;
	for(int16 i = 0; i < indexed_items.size(); i++){
		Item* item = indexed_items[i];
		if(item && item->details.new_item) {
			new_item_slot++;
			int16 actual_index = in_index - new_item_slot;
			// this isn't compiling right
			//printf("In index: %u new index %u actual %u and %u, new slot num %u\n", in_index, item->details.new_index, actual_index, i, new_item_slot);
			if(actual_index == i) {
				return i;
			}
		}
	}
	return 0xFFFF;
}

void PlayerItemList::AddItemToPacket(PacketStruct* packet, Player* player, Item* item, int16 i, bool overflow, int16 new_index){
	Client *client;
	if (!packet || !player)
		return;
	client = ((Player*)player)->GetClient();
	if (!client)
		return;
	
	int32 menu_data = 3;
	if(item->slot_data.size() > 0)
		menu_data -= ITEM_MENU_TYPE_GENERIC;

	if (item->details.num_slots > 0) {
		int8 max_slots = player->GetMaxBagSlots(client->GetVersion());
		if (item->details.num_slots > max_slots)
			item->details.num_slots = max_slots;
		
		menu_data += ITEM_MENU_TYPE_BAG;
		
		if (item->details.num_free_slots == item->details.num_slots)
			menu_data += ITEM_MENU_TYPE_EMPTY_BAG;
	}
	if (item->details.item_id == 21355) {
		//menu_data += ITEM_MENU_TYPE_GENERIC;
		//menu_data += ITEM_MENU_TYPE_EQUIP;
		menu_data += ITEM_MENU_TYPE_BOOK;
		//menu_data += ITEM_MENU_TYPE_BAG;
		//menu_data += ITEM_MENU_TYPE_HOUSE;
		//menu_data += ITEM_MENU_TYPE_TEST12;
		//menu_data += ITEM_MENU_TYPE_SCRIBE;
		//menu_data += ITEM_MENU_TYPE_TEST13;
		//menu_data += ITEM_MENU_TYPE_INVALID;
		//menu_data += ITEM_MENU_TYPE_TEST14;
		//menu_data += ITEM_MENU_TYPE_BROKEN;
	}
	if (item->details.item_id == 21356) {
		//menu_data += ITEM_MENU_TYPE_TEST15;
		menu_data += ITEM_MENU_TYPE_ATTUNED;
		menu_data += ITEM_MENU_TYPE_ATTUNEABLE;
		menu_data += ITEM_MENU_TYPE_BOOK;
		menu_data += ITEM_MENU_TYPE_DISPLAY_CHARGES;
		menu_data += ITEM_MENU_TYPE_TEST1;
		menu_data += ITEM_MENU_TYPE_NAMEPET;
		menu_data += ITEM_MENU_TYPE_MENTORED;
		menu_data += ITEM_MENU_TYPE_CONSUME;
		menu_data += ITEM_MENU_TYPE_USE;
	}
	if (item->details.item_id == 21357) {
		menu_data += ITEM_MENU_TYPE_CONSUME_OFF		;
		menu_data += ITEM_MENU_TYPE_TEST3			;
		menu_data += ITEM_MENU_TYPE_TEST4			;
		menu_data += ITEM_MENU_TYPE_TEST5			;
		menu_data += ITEM_MENU_TYPE_TEST6			;
		menu_data += ITEM_MENU_TYPE_TEST7			;
		menu_data += ITEM_MENU_TYPE_TEST8			;
		menu_data += ITEM_MENU_TYPE_TEST9			;
		menu_data += ITEM_MENU_TYPE_DAMAGED			;
		menu_data += ITEM_MENU_TYPE_BROKEN2			;
		menu_data += ITEM_MENU_TYPE_REDEEM	       ;
		menu_data += ITEM_MENU_TYPE_TEST10			;
		menu_data += ITEM_MENU_TYPE_UNPACK			;
	}
	if(item->IsSkill()){
		Spell* spell = master_spell_list.GetSpell(item->skill_info->spell_id, item->skill_info->spell_tier);
		if (spell && spell->ScribeAllowed(player))			
			menu_data += ITEM_MENU_TYPE_SCRIBE;
		else
			menu_data += ITEM_MENU_TYPE_INSUFFICIENT_KNOWLEDGE;
	}
	if(item->IsRecipeBook()){
		//TODO: Add check to allow scribe
		menu_data += ITEM_MENU_TYPE_SCRIBE;
	}
	if (item->generic_info.item_type == ITEM_TYPE_HOUSE || (item->generic_info.item_type == ITEM_TYPE_HOUSE_CONTAINER && item->details.inv_slot_id == InventorySlotType::HOUSE_VAULT)){ // containers must be in base house slot for placement
		menu_data += ITEM_MENU_TYPE_TEST1;
		menu_data += ITEM_MENU_TYPE_HOUSE;
	}
	if (item->generic_info.item_type == 18){
		menu_data += ITEM_MENU_TYPE_UNPACK;
		packet->setSubstructArrayDataByName("items", "unknown3", ITEM_MENU_TYPE2_UNPACK, 0, i);
	}

	if(item->generic_info.condition == 0)
		menu_data += ITEM_MENU_TYPE_BROKEN;
	if (client->GetVersion() <= 373){
		string flags;
		if (item->CheckFlag(NO_TRADE))
			flags += "NO-TRADE   ";
		if (item->CheckFlag(NO_VALUE))
			flags += "NO-VALUE   ";
		if (flags.length() > 0)
			packet->setSubstructArrayDataByName("items", "flag_names", flags.c_str(), 0, i);
	}

	if (item->CheckFlag(ATTUNED) || item->CheckFlag(NO_TRADE)) {
		if (client->GetVersion() <= 373)
			menu_data += ORIG_ITEM_MENU_TYPE_ATTUNED;
		else
			menu_data += ITEM_MENU_TYPE_ATTUNED;
	}
	else if (item->CheckFlag(ATTUNEABLE)) {
		if (client->GetVersion() <= 373)
			menu_data += ORIG_ITEM_MENU_TYPE_ATTUNEABLE;
		else
			menu_data += ITEM_MENU_TYPE_ATTUNEABLE;
	}
	if (item->generic_info.usable == 1)
		menu_data += ITEM_MENU_TYPE_USE;
	if (item->details.count > 0 && item->stack_count > 1) {
		if (client->GetVersion() <= 373)
			menu_data += ORIG_ITEM_MENU_TYPE_STACKABLE;
		else
			menu_data += ITEM_MENU_TYPE_DISPLAY_CHARGES;
	}
	if(item->IsFood()) {
		if (client->GetVersion() <= 373) {						
			if (item->IsFoodDrink())
				menu_data += ORIG_ITEM_MENU_TYPE_DRINK;
			else if(item->IsFoodFood())
				menu_data += ORIG_ITEM_MENU_TYPE_FOOD;
		}
	}
	if(item->IsItemLocked()) {
		menu_data += ITEM_MENU_TYPE_BROKEN; // broken is also used to lock item during crafting
	}
	// Added the if (overflow) so mouseover examines work properly
	if (overflow)
		packet->setSubstructArrayDataByName("items", "unique_id", item->details.item_id, 0, i);
	else
		packet->setSubstructArrayDataByName("items", "unique_id", item->details.unique_id, 0, i);
	packet->setSubstructArrayDataByName("items", "bag_id", item->details.bag_id, 0, i);
	packet->setSubstructArrayDataByName("items", "inv_slot_id", item->details.inv_slot_id, 0, i);
	packet->setSubstructArrayDataByName("items", "menu_type", menu_data, 0, i);
	if (overflow)
		packet->setSubstructArrayDataByName("items", "index", 0xFFFF, 0, i);
	else {
		if(packet->GetVersion() <= 561) {
				/* DoF client and earlier side automatically assigns indexes
				** we have to send 0xFF or else all index is set to 255 on client
				** and then examine inventory won't work */
				LogWrite(ITEM__DEBUG, 0, "%s Offset index %u bag id %u (new index %u, set index %u)",item->name.c_str(),i, item->details.bag_id, new_index, item->details.new_index);
				if(item->details.new_item) {
					item->details.new_index = new_index + i; // we have to offset in this way to get consistent indexes for the client to send back
					packet->setSubstructArrayDataByName("items", "index", 0xFF+item->details.new_index, 0, i);
				}
				else {
					packet->setSubstructArrayDataByName("items", "index", 0xFF, 0, i);
				}
		}
		else {
				packet->setSubstructArrayDataByName("items", "index", i, 0, i);
		}
	}
	item->details.index = i;
	
	packet->setSubstructArrayDataByName("items", "icon", item->GetIcon(client->GetVersion()), 0, i);
	packet->setSubstructArrayDataByName("items", "slot_id", item->details.slot_id, 0, i); // inventory doesn't convert slots
	if (client->GetVersion() <= 1208) {
		packet->setSubstructArrayDataByName("items", "count", (std::min)(item->details.count, (int16)255), 0, i);
	}
	else
		packet->setSubstructArrayDataByName("items", "count", item->details.count, 0, i);
	//packet->setSubstructArrayDataByName("items", "unknown4", 5, 0, i);
	// need item level
	packet->setSubstructArrayDataByName("items", "item_level", item->details.recommended_level , 0, i);
	
	
	if(rule_manager.GetZoneRule(player->GetZoneID(), R_World, DisplayItemTiers)->GetBool()) {
		packet->setSubstructArrayDataByName("items", "tier", item->details.tier, 0, i);
	}
	
	packet->setSubstructArrayDataByName("items", "num_slots", item->details.num_slots, 0, i);
	// need empty slots
	packet->setSubstructArrayDataByName("items", "item_id", item->details.item_id, 0, i);
	//need broker id
	packet->setSubstructArrayDataByName("items", "name", item->name.c_str(), 0, i);
	
}

bool PlayerItemList::AddOverflowItem(Item* item) {
	bool ret = false;
	MPlayerItems.writelock(__FUNCTION__, __LINE__);
	if (item && item->details.item_id > 0 && overflowItems.size() < 255) {
		item->details.slot_id = 6;
		item->details.inv_slot_id = InventorySlotType::OVERFLOW;
		overflowItems.push_back(item);
		ret = true;
	}
	MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
	return ret;
}

Item* PlayerItemList::GetOverflowItem() {
	if(overflowItems.empty()) {
		return nullptr;
	}
	
	return overflowItems.at(0);
}

void PlayerItemList::RemoveOverflowItem(Item* item) {
	MPlayerItems.writelock(__FUNCTION__, __LINE__);
	vector<Item*>::iterator itr = std::find(overflowItems.begin(), overflowItems.end(), item);
	if(itr != overflowItems.end()) {
		overflowItems.erase(itr);
	}
	MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
}

vector<Item*>* PlayerItemList::GetOverflowItemList() {
	vector<Item*>* ret = new vector<Item*>;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	vector<Item*>::iterator itr= ret->begin();
	ret->insert(itr, overflowItems.begin(), overflowItems.end());
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

bool PlayerItemList::HasItem(int32 id, bool include_bank){
	if(include_bank) {
		Item* item = GetItemFromID(id, 1, true, true);
		if(item)
			return true;
		else
			return false;
	}
	map<sint32, map<int8, map<int16, Item*>> >::iterator itr;
	map<int16, Item*>::iterator slot_itr;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	for(itr = items.begin(); itr != items.end(); itr++){
		if(itr->first >= 0){
			for(slot_itr=itr->second[BASE_EQUIPMENT].begin();slot_itr!=itr->second[BASE_EQUIPMENT].end(); slot_itr++){
				if(slot_itr->second && slot_itr->second->details.item_id == id){
					MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
					return true;
				}
			}
			for(slot_itr=itr->second[APPEARANCE_EQUIPMENT].begin();slot_itr!=itr->second[APPEARANCE_EQUIPMENT].end(); slot_itr++){
				if(slot_itr->second && slot_itr->second->details.item_id == id){
					MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
					return true;
				}
			}
		}
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return false;
}

bool PlayerItemList::SharedBankAddAllowed(Item* item){
	if(!item || (item->CheckFlag(NO_TRADE) && (item->CheckFlag2(HEIRLOOM) == 0)))
		return false;

	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if(item->IsBag() && items.count(item->details.bag_id) > 0){
		map<int16, Item*>::iterator itr;
		for(itr = items[item->details.bag_id][BASE_EQUIPMENT].begin(); itr != items[item->details.bag_id][BASE_EQUIPMENT].end(); itr++){
			if(itr->second->CheckFlag(NO_TRADE) && itr->second->CheckFlag2(HEIRLOOM) == 0){
				MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
				return false;
			}
		}
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return true;
}

vector<Item*>* PlayerItemList::GetItemsFromBagID(sint32 bag_id){
	vector<Item*>* ret = new vector<Item*>;
	if(items.count(bag_id) > 0){
		MPlayerItems.readlock(__FUNCTION__, __LINE__);
		map<int16, Item*>::iterator itr;
		map<int16, Item*>::iterator itr2;
		Item* item = 0;
		for(itr = items[bag_id][BASE_EQUIPMENT].begin(); itr != items[bag_id][BASE_EQUIPMENT].end(); itr++){
			item = itr->second;
			if(item)
				ret->push_back(item);
		}
		MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	}
	return ret;
}
int32 PlayerItemList::GetItemCountInBag(Item* bag){
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if(bag && bag->IsBag() && items.count(bag->details.bag_id) > 0){
		int32 bagitems = items.count(bag->details.bag_id);
		MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
		return bagitems;
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return 0;
}
vector<Item*>* PlayerItemList::GetItemsInBag(Item* bag){
	vector<Item*>* ret_items = new vector<Item*>;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if(bag && bag->IsBag() && items.count(bag->details.bag_id) > 0){
		map<int16, Item*>::iterator itr;
		for(itr = items[bag->details.bag_id][BASE_EQUIPMENT].begin(); itr != items[bag->details.bag_id][BASE_EQUIPMENT].end(); itr++){
			ret_items->push_back(itr->second);
		}
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret_items;
}

Item* PlayerItemList::GetItemFromID(int32 id, int8 count, bool include_bank, bool lock){
	//first check for an exact count match
	map<sint32, map<int8, map<int16, Item*>> >::iterator itr;
	map<int16, Item*>::iterator slot_itr;
	if(lock)
		MPlayerItems.readlock(__FUNCTION__, __LINE__);
	for(itr = items.begin(); itr != items.end(); itr++){
		if(include_bank || (!include_bank && itr->first >= 0)){
			for(int8 i=0;i<MAX_EQUIPMENT;i++)
			{
				for(slot_itr=itr->second[i].begin();slot_itr!=itr->second[i].end(); slot_itr++){
					if(slot_itr->second && slot_itr->second->details.item_id == id && (count == 0 || slot_itr->second->details.count == count)){
						if(lock)
							MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
						return slot_itr->second;
					}
				}
			}
		}
	}

	//couldn't find an exact match, look for closest
	Item* closest = 0;
	for(itr = items.begin(); itr != items.end(); itr++){
		if(include_bank || (!include_bank && itr->first >= 0)){
			for(int8 i=0;i<MAX_EQUIPMENT;i++)
			{
				for(slot_itr=itr->second[i].begin();slot_itr!=itr->second[i].end(); slot_itr++){
					if(slot_itr->second && slot_itr->second->details.item_id == id && slot_itr->second->details.count > count && (closest == 0 || slot_itr->second->details.count < closest->details.count))
						closest = slot_itr->second;
				}
			}
		}
	}
	if(lock)
		MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return closest;
}

sint32 PlayerItemList::GetAllStackCountItemFromID(int32 id, int8 count, bool include_bank, bool lock){
	sint32 stack_count = 0;
	//first check for an exact count match
	map<sint32, map<int8, map<int16, Item*>> >::iterator itr;
	map<int16, Item*>::iterator slot_itr;
	if(lock)
		MPlayerItems.readlock(__FUNCTION__, __LINE__);
	for(itr = items.begin(); itr != items.end(); itr++){
		if(include_bank || (!include_bank && itr->first >= 0)){
			for(int8 i=0;i<MAX_EQUIPMENT;i++)
			{
				for(slot_itr=itr->second[i].begin();slot_itr!=itr->second[i].end(); slot_itr++){
					if(slot_itr->second && slot_itr->second->details.item_id == id && (count == 0 || slot_itr->second->details.count == count)){
						stack_count += slot_itr->second->details.count;
					}
				}
			}
		}
	}

	//couldn't find an exact match, look for closest
	Item* closest = 0;
	for(itr = items.begin(); itr != items.end(); itr++){
		if(include_bank || (!include_bank && itr->first >= 0)){
			for(int8 i=0;i<MAX_EQUIPMENT;i++)
			{
				for(slot_itr=itr->second[i].begin();slot_itr!=itr->second[i].end(); slot_itr++){
					if(slot_itr->second && slot_itr->second->details.item_id == id && slot_itr->second->details.count > count && (closest == 0 || slot_itr->second->details.count < closest->details.count))
						stack_count += slot_itr->second->details.count;
				}
			}
		}
	}
	if(lock)
		MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return stack_count;
}

Item* PlayerItemList::GetItemFromUniqueID(int32 id, bool include_bank, bool lock){
	map<sint32, map<int8, map<int16, Item*>> >::iterator itr;
	map<int16, Item*>::iterator slot_itr;
	if(lock)
		MPlayerItems.readlock(__FUNCTION__, __LINE__);
	for(itr = items.begin(); itr != items.end(); itr++){
		if(include_bank || (!include_bank && itr->first >= 0)){
			for(slot_itr=itr->second[0].begin();slot_itr!=itr->second[0].end(); slot_itr++){
				if(slot_itr->second && slot_itr->second->details.unique_id == id){
					if(lock)
						MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
					return slot_itr->second;
				}
			}
			for(slot_itr=itr->second[1].begin();slot_itr!=itr->second[1].end(); slot_itr++){
				if(slot_itr->second && slot_itr->second->details.unique_id == id){
					if(lock)
						MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
					return slot_itr->second;
				}
			}
		}
	}
	if(lock)
		MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return 0;
}

void PlayerItemList::SetVaultItemLockUniqueID(Client* client, int64 id, bool state, bool lock) {
	if (lock) {
		MPlayerItems.readlock(__FUNCTION__, __LINE__);
	}
	sint32 inv_slot_id = 0;
	Item* item = client->GetPlayer()->item_list.GetVaultItemFromUniqueID(id, false);
	if(item) {
		bool bag_remains_locked = true;
		if(state && !item->TryLockItem(LockReason::LockReason_Shop)) {
			// this shouldn't happen, but if we have a conflict it might
			client->Message(CHANNEL_COLOR_RED, "Failed to lock item %u for vault.", id);
			return;
		}
		else if(!state && !item->TryUnlockItem(LockReason::LockReason_Shop)) {
			// still in use for another reason, we don't need to report to the user it will spam them
		}
		if(item->details.inv_slot_id) {
			Item* bagItem = client->GetPlayer()->item_list.GetVaultItemFromUniqueID(item->details.inv_slot_id, false);
			if(bagItem) {
				inv_slot_id = item->details.inv_slot_id;
				if(!state) {
					bag_remains_locked = false;
					if (auto bagIt = items.find(item->details.inv_slot_id);
						bagIt != items.end())
					{
						const auto& bagSlots = bagIt->second.at(BASE_EQUIPMENT);
						for (auto& [bagSlot, bagItem] : bagSlots) {
							if (bagItem && bagItem->IsItemLocked()) {
								bag_remains_locked = true;
								break;
							}
						}
					}
				}
				if(!bag_remains_locked) {
					bagItem->TryUnlockItem(LockReason::LockReason_Shop);
				}
				else {
					bagItem->TryLockItem(LockReason::LockReason_Shop);
				}
			}
		}
	}
	if (lock) {
		MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	}
	if(inv_slot_id) {
		client->GetPlayer()->UpdateInventory(item->details.inv_slot_id);
	}
	else {
		EQ2Packet* outapp = client->GetPlayer()->SendInventoryUpdate(client->GetVersion());
		client->QueuePacket(outapp);
	}
}

bool PlayerItemList::CanStoreSellItem(int64 unique_id, bool lock) {
	if (lock) {
		MPlayerItems.readlock(__FUNCTION__, __LINE__);
	}
	Item* item = GetVaultItemFromUniqueID(unique_id,  false);
	if(!item || (item->CheckFlag(NO_TRADE) && (item->CheckFlag2(HEIRLOOM) == 0))) {
		if(lock)
			MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
		return false;
	}
	if(item->CheckFlag(ATTUNED)) {
		if(lock)
			MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
		return false;
	}

	if(item->IsBag() && items.count(item->details.bag_id) > 0){
		map<int16, Item*>::iterator itr;
		for(itr = items[item->details.bag_id][BASE_EQUIPMENT].begin(); itr != items[item->details.bag_id][BASE_EQUIPMENT].end(); itr++){
			if(itr->second->CheckFlag(NO_TRADE) && itr->second->CheckFlag2(HEIRLOOM) == 0){
				if(lock)
					MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
				return false;
			}
			if(item->CheckFlag(ATTUNED)) {
				if(lock)
					MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
				return false;
			}
		}
	}
	if(lock) {
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	}
	return true;
}

void PlayerItemList::SetVaultItemUniqueIDCount(Client* client, int64 unique_id, int16 count, bool lock) {
	if (lock) {
		MPlayerItems.readlock(__FUNCTION__, __LINE__);
	}
	
	sint32 inv_slot_id = 0;
	bool countUpdated = false;
	Item* item = GetVaultItemFromUniqueID(unique_id, false);
	if(item) {
		item->details.count = count;
		item->save_needed = true;
		inv_slot_id = item->details.inv_slot_id;
		countUpdated = true;
	}
	if (lock) {
		MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	}
	
	if(client && client->GetPlayer() && countUpdated) {
		if(inv_slot_id) {
			client->GetPlayer()->UpdateInventory(inv_slot_id);
		}
		else {
			EQ2Packet* outapp = client->GetPlayer()->SendInventoryUpdate(client->GetVersion());
			client->QueuePacket(outapp);
		}
	}
}


void PlayerItemList::RemoveVaultItemFromUniqueID(Client* client, int64 unique_id, bool lock) {
	if (lock) {
		MPlayerItems.writelock(__FUNCTION__, __LINE__);
	}
	
	sint32 inv_slot_id = 0;
	bool foundItem = false;
	Item* item = GetVaultItemFromUniqueID(unique_id, false);
	if(item) {
		inv_slot_id = item->details.inv_slot_id;
		RemoveItem(item, true, false);
		foundItem = true;
	}
	if (lock) {
		MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
	}
	
	if(client && client->GetPlayer() && foundItem) {
		if(inv_slot_id) {
			client->GetPlayer()->UpdateInventory(inv_slot_id);
		}
		else {
			EQ2Packet* outapp = client->GetPlayer()->SendInventoryUpdate(client->GetVersion());
			client->QueuePacket(outapp);
		}
	}
}

Item* PlayerItemList::GetVaultItemFromUniqueID(int64 id, bool lock) {
	if (lock) {
		MPlayerItems.readlock(__FUNCTION__, __LINE__);
	}

	// 1) Check the house vault
	if (auto vaultIt = items.find(InventorySlotType::HOUSE_VAULT);
		vaultIt != items.end())
	{
		for (auto& [containerIdx, slotMap] : vaultIt->second) {
			for (auto& [slotID, itemPtr] : slotMap) {
				if(itemPtr) {
				LogWrite(PLAYER__ERROR, 0, "Vault",
				  "--GetVaultItem: %u (%s - %u) needs to match %u", slotID, itemPtr->name.c_str(), itemPtr->details.unique_id, id
				);
				}
				else {
					LogWrite(PLAYER__ERROR, 0, "Vault",
				  "--GetVaultItem: %u (??) needs to match %u", slotID, id
				);
				}
				if (itemPtr && itemPtr->details.unique_id == id) {
					if (lock) MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
					return itemPtr;
				}
				
				// If it's a bag, search its contents
				if (!itemPtr->IsBag()) 
					continue;

				if (auto bagIt = items.find(itemPtr->details.bag_id);
					bagIt != items.end())
				{
					const auto& bagSlots = bagIt->second.at(BASE_EQUIPMENT);
					for (auto& [bagSlot, bagItem] : bagSlots) {
						if (bagItem && bagItem->details.unique_id == id) {
							if (lock) MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
							return bagItem;
						}
					}
				}
			}
		}
	}

	// 2) Check base inventory slots and any bags inside them
	const auto& baseEquip = items
		.at(InventorySlotType::BASE_INVENTORY)
		.at(BASE_EQUIPMENT);

	for (int8 slotIdx = 0; slotIdx < NUM_INV_SLOTS; ++slotIdx) {
		auto it = baseEquip.find(slotIdx);
		if (it == baseEquip.end() || it->second == nullptr) 
			continue;

		Item* curr = it->second;
		// Direct match in base slot
		if (curr->details.unique_id == id) {
			if (lock) MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
			return curr;
		}

		// If it's a bag, search its contents
		if (!curr->IsBag()) 
			continue;

		if (auto bagIt = items.find(curr->details.bag_id);
			bagIt != items.end())
		{
			const auto& bagSlots = bagIt->second.at(BASE_EQUIPMENT);
			for (auto& [bagSlot, bagItem] : bagSlots) {
				if (bagItem && bagItem->details.unique_id == id) {
					if (lock) MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
					return bagItem;
				}
			}
		}
	}

	if (lock) {
		MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	}
	return nullptr;
}

bool PlayerItemList::HasFreeBankSlot() {
	bool ret = false;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	if (items[InventorySlotType::BANK][BASE_EQUIPMENT].size() < 12) //12 slots in the bank
		ret = true;
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

int8 PlayerItemList::FindFreeBankSlot() {
	int8 ret = 0;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	for (int8 i = 0; i < 12; i++) { //12 slots in the bank
		if (items[InventorySlotType::BANK][BASE_EQUIPMENT].count(i) == 0) {
			ret = i;
			break;
		}
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	return ret;
}

void PlayerItemList::PopulateHouseStoragePacket(Client* client, PacketStruct* packet, Item* item, int16 itemIdx, int8 storage_flags) {
	int64 cost = broker.GetSalePrice(client->GetPlayer()->GetCharacterID(), item->details.unique_id);
	bool sale = broker.IsItemForSale(client->GetPlayer()->GetCharacterID(), item->details.unique_id);
	bool isInv = !IsItemInSlotType(item, InventorySlotType::HOUSE_VAULT, false);
	client->AddItemSale(item->details.unique_id, item->details.item_id, cost, item->details.inv_slot_id, item->details.slot_id, item->details.count, isInv, sale, item->creator);
	
	if(broker.IsItemForSale(client->GetPlayer()->GetCharacterID(), item->details.unique_id))
		storage_flags += HouseStoreItemFlags::HOUSE_STORE_FOR_SALE;
	
	LogWrite(PLAYER__ERROR, 5, "Broker",
	  "--Sell broker item: %u (%s - %u), cost=%u",
	  client->GetPlayer()->GetCharacterID(), item->name.c_str(), item->details.unique_id, cost
	);
	
	packet->setArrayDataByName("your_item_name", item->name.c_str(), itemIdx);
	packet->setArrayDataByName("unique_id", item->details.item_id, itemIdx);
	packet->setArrayDataByName("unique_id2", item->details.unique_id, itemIdx);
	packet->setArrayDataByName("cost", cost, itemIdx);
	packet->setArrayDataByName("your_item_quantity", item->details.count, itemIdx);
	packet->setArrayDataByName("your_item_icon", item->GetIcon(packet->GetVersion()), itemIdx);
	packet->setArrayDataByName("storage_flags", storage_flags, itemIdx);
}

void PlayerItemList::GetVaultItems(Client* client, int32 spawn_id, int8 maxSlots, bool isSelling) {
	int8 ret = 0;
	int8 numItems = 0;
	MPlayerItems.readlock(__FUNCTION__, __LINE__);
	for (int8 i = 0; i < maxSlots; i++) {
		if (items[InventorySlotType::HOUSE_VAULT][BASE_EQUIPMENT].count(i) != 0) {
			Item* item = items[InventorySlotType::HOUSE_VAULT][BASE_EQUIPMENT][i];
			if(item) {
				if(!item->IsBag()) {
					numItems++;
				}
				else {
					bool bagHasItem = false;
					if(items.count(item->details.bag_id) > 0){
						map<int16, Item*>::iterator itr;
						for(itr = items[item->details.bag_id][BASE_EQUIPMENT].begin(); itr != items[item->details.bag_id][BASE_EQUIPMENT].end(); itr++){
							if(itr->second) {
								numItems++;
								bagHasItem = true;
							}
						}
					}
					if(!bagHasItem) {
						numItems++;
					}
				}
			}
		}
	}
	for (int8 i = 0; i < NUM_INV_SLOTS; i++) {
		if (items[InventorySlotType::BASE_INVENTORY][BASE_EQUIPMENT].count(i) != 0) {
			Item* item = items[InventorySlotType::BASE_INVENTORY][BASE_EQUIPMENT][i];
			if(item) {
				if(!item->IsBag()) {
					numItems++;
				}
				else {
					bool bagHasItem = false;
					if(items.count(item->details.bag_id) > 0){
						map<int16, Item*>::iterator itr;
						for(itr = items[item->details.bag_id][BASE_EQUIPMENT].begin(); itr != items[item->details.bag_id][BASE_EQUIPMENT].end(); itr++){
							if(itr->second) {
								numItems++;
								bagHasItem = true;
							}
						}
					}
					if(!bagHasItem) {
						numItems++;
					}
				}
			}
		}
	}

	PacketStruct* packet = configReader.getStruct("WS_HouseStorage", client->GetVersion());
	if (packet) {
		packet->setDataByName("spawn_id", spawn_id);
		packet->setDataByName("type", isSelling ? 6 : 4);
		packet->setArrayLengthByName("your_item_count", numItems);
		int16 itemIdx = 0;
		for (int8 i = 0; i < maxSlots; i++) {
			if (items[InventorySlotType::HOUSE_VAULT][BASE_EQUIPMENT].count(i) != 0) {
				Item* item = items[InventorySlotType::HOUSE_VAULT][BASE_EQUIPMENT][i];
				if(item) {
					if(!item->IsBag()) {
						PopulateHouseStoragePacket(client, packet, item, itemIdx, HouseStoreItemFlags::HOUSE_STORE_VAULT_TAB);
						itemIdx++;
					}
					else {
						bool bagHasItem = false;
						if(items.count(item->details.bag_id) > 0){
							map<int16, Item*>::iterator itr;
							for(itr = items[item->details.bag_id][BASE_EQUIPMENT].begin(); itr != items[item->details.bag_id][BASE_EQUIPMENT].end(); itr++){
								if(itr->second) {
									PopulateHouseStoragePacket(client, packet, itr->second, itemIdx, HouseStoreItemFlags::HOUSE_STORE_VAULT_TAB);
									bagHasItem = true;
									itemIdx++;
								}
							}
						}
						if(!bagHasItem) {
							PopulateHouseStoragePacket(client, packet, item, itemIdx, HouseStoreItemFlags::HOUSE_STORE_VAULT_TAB);
							itemIdx++;
						}
					}
				}
			}
		}
		
		for (int8 i = 0; i < NUM_INV_SLOTS; i++) {
			if (items[InventorySlotType::BASE_INVENTORY][BASE_EQUIPMENT].count(i) != 0) {
				Item* item = items[InventorySlotType::BASE_INVENTORY][BASE_EQUIPMENT][i];
				if(item) {
					if(!item->IsBag()) {
						PopulateHouseStoragePacket(client, packet, item, itemIdx, 0);
						itemIdx++;
					}
					else {
						bool bagHasItem = false;
						if(items.count(item->details.bag_id) > 0){
							map<int16, Item*>::iterator itr;
							for(itr = items[item->details.bag_id][BASE_EQUIPMENT].begin(); itr != items[item->details.bag_id][BASE_EQUIPMENT].end(); itr++){
								if(itr->second) {
									PopulateHouseStoragePacket(client, packet, itr->second, itemIdx, 0);
									bagHasItem = true;
									itemIdx++;
								}
							}
						}
						if(!bagHasItem) {
							PopulateHouseStoragePacket(client, packet, item, itemIdx, 0);
							itemIdx++;
						}
					}
				}
			}
		}
		EQ2Packet* outapp = packet->serialize();
		client->QueuePacket(outapp);
		safe_delete(packet);
	}
	MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
}

void PlayerItemList::ResetPackets() {
	MPlayerItems.writelock(__FUNCTION__, __LINE__);
	safe_delete_array(orig_packet);
	safe_delete_array(xor_packet);
	orig_packet = 0;
	xor_packet = 0;
	packet_count = 0;
	MPlayerItems.releasewritelock(__FUNCTION__, __LINE__);
}


int32 PlayerItemList::CheckSlotConflict(Item* item, bool check_lore_only, bool lock, int16* lore_stack_count) {
	bool is_lore = false;
	bool is_stack_lore = false;
	if(!(is_lore = item->CheckFlag(LORE)) && !(is_stack_lore = item->CheckFlag(STACK_LORE)) && check_lore_only) {
		return 0;
	}
	
	if(!check_lore_only && !is_lore && !is_stack_lore && !item->CheckFlag(LORE_EQUIP)) {
		return 0;
	}
	
	
	int32 conflict = 0;
	
	if(lock)
		MPlayerItems.readlock(__FUNCTION__, __LINE__);
	
	map<sint32, map<int8, map<int16, Item*>> >::iterator itr;
	map<int16, Item*>::iterator slot_itr;
	
	for(itr = items.begin(); itr != items.end(); itr++){
		for(slot_itr=itr->second[0].begin();slot_itr!=itr->second[0].end(); slot_itr++){
			if(slot_itr->second && slot_itr->second->details.item_id == item->details.item_id){
				if(lore_stack_count) {
					*lore_stack_count += slot_itr->second->details.count;
				}
				if(!is_stack_lore && slot_itr->second->CheckFlag(LORE)) {
					conflict = LORE;
					break;
				}
				else if(is_stack_lore && (*lore_stack_count + item->details.count) > slot_itr->second->stack_count) {
					conflict = STACK_LORE;
					break;
				}
				else if(!check_lore_only && slot_itr->second->CheckFlag(LORE_EQUIP)) {
					conflict = LORE_EQUIP;
					break;
				}
			}
		}
			
		if(conflict > 0)
			break;
		
		for(slot_itr=itr->second[1].begin();slot_itr!=itr->second[1].end(); slot_itr++){
			if(slot_itr->second && slot_itr->second->details.item_id == item->details.item_id){
				if(lore_stack_count) {
					*lore_stack_count += slot_itr->second->details.count;
				}
				if(!is_stack_lore && slot_itr->second->CheckFlag(LORE)) {
					conflict = LORE;
					break;
				}
				else if(is_stack_lore && (*lore_stack_count + item->details.count) > slot_itr->second->stack_count) {
					conflict = STACK_LORE;
					break;
				}
				else if(!check_lore_only && slot_itr->second->CheckFlag(LORE_EQUIP)) {
					conflict = LORE_EQUIP;
					break;
				}
			}
		}
		
		if(conflict > 0)
			break;
	}

	if(lock)
		MPlayerItems.releasereadlock(__FUNCTION__, __LINE__);
	
	return conflict;
}


EquipmentItemList::EquipmentItemList(){
	orig_packet = 0;
	xor_packet = 0;
	for(int8 i=0;i<NUM_SLOTS;i++)
		items[i] = 0;
	MEquipmentItems.SetName("EquipmentItemList::MEquipmentItems");
	AppearanceType = 0;
}

EquipmentItemList::EquipmentItemList(const EquipmentItemList& list){
	orig_packet = 0;
	xor_packet = 0;
	for(int8 i=0;i<NUM_SLOTS;i++)
		items[i] = 0;
	MEquipmentItems.SetName("EquipmentItemList::MEquipmentItems");
}

EquipmentItemList::~EquipmentItemList(){
	for(int8 i=0;i<NUM_SLOTS;i++)
		safe_delete(items[i]);
	safe_delete_array(orig_packet);
	safe_delete_array(xor_packet);
}

void EquipmentItemList::ResetPackets() {
	MEquipmentItems.lock();
	safe_delete_array(orig_packet);
	safe_delete_array(xor_packet);
	orig_packet = 0;
	xor_packet = 0;
	MEquipmentItems.unlock();
}

bool EquipmentItemList::AddItem(int8 slot, Item* item){
	if(item){
		MEquipmentItems.lock();
		Item* curItem = GetItem(slot);

		if (curItem) // existing item in slot
		{
			MEquipmentItems.unlock();
			LogWrite(ITEM__ERROR, 0, "Items", "%s: Error in AddItem, curItem %s in slot %u, cannot put %s in slot.", __FUNCTION__, curItem->name.c_str(), slot, item->name.c_str());
			return false;
		}
		
		SetItem(slot, item, true);
		if (item->details.unique_id == 0) {
			GetItem(slot)->details.unique_id = master_item_list.NextUniqueID();
			if (item->IsBag())
				item->details.bag_id = item->details.unique_id;
		}
		MEquipmentItems.unlock();
		return true;
	}
	return false;
}

int8 EquipmentItemList::GetNumberOfItems(){
	int8 ret = 0;
	MEquipmentItems.lock();
	for(int8 i=0;i<NUM_SLOTS;i++){
		if(items[i])
			ret++;
	}
	MEquipmentItems.unlock();
	return ret;
}

int32 EquipmentItemList::GetWeight(){
	int32 ret = 0;
	MEquipmentItems.lock();
	for(int8 i=0;i<NUM_SLOTS;i++){
		if(items[i]) {
			ret += items[i]->generic_info.weight;
		}
	}
	MEquipmentItems.unlock();
	return ret;
}

void EquipmentItemList::SetItem(int8 slot_id, Item* item, bool locked){
	if(!locked)
		MEquipmentItems.lock();
	item->details.bag_id = item->details.unique_id;
	if(!item->IsBag()) {
		item->details.inv_slot_id = 0;
	}
	else {
	item->details.equip_slot_id = slot_id;
	}
	
	if(!item->IsBag()) {
		item->details.slot_id = slot_id;
		item->details.index = slot_id;
	}
	item->details.appearance_type = GetAppearanceType();
	items[slot_id] = item;

	if(!locked)
		MEquipmentItems.unlock();
}

vector<Item*>* EquipmentItemList::GetAllEquippedItems(){
	vector<Item*>* ret = new vector<Item*>;
	MEquipmentItems.lock();
	for(int8 i=0;i<NUM_SLOTS;i++){
		if(items[i])
			ret->push_back(items[i]);
	}
	MEquipmentItems.unlock();
	return ret;
}

Item* EquipmentItemList::GetItem(int8 slot_id){
	return items[slot_id];
}

void EquipmentItemList::SendEquippedItems(Player* player){
	if(!player->GetClient()) {
		return;
	}
	
	for(int16 i=0;i<NUM_SLOTS;i++){
		Item* item = items[i];
		if(item && item->details.item_id > 0)
			player->GetClient()->QueuePacket(item->serialize(player->GetClient()->GetVersion(), false, player));
	}
}

EQ2Packet* EquipmentItemList::serialize(int16 version, Player* player){
	EQ2Packet* app = 0;
	Item* item = 0;
	PacketStruct* packet = configReader.getStruct("WS_UpdateInventory", version);
	MEquipmentItems.lock();
	if(packet){
		int8 packet_size = 0;
		PacketStruct* packet2 = configReader.getStruct("Substruct_Item", version);
		packet_size = packet2->GetTotalPacketSize();
		safe_delete(packet2);
		int8 num_slots = player->GetNumSlotsEquip(version);
		packet->setArrayLengthByName("item_count", num_slots);
		if(!orig_packet){
			xor_packet = new uchar[packet_size* num_slots];
			orig_packet = new uchar[packet_size* num_slots];
			memset(xor_packet, 0, packet_size* num_slots);
			memset(orig_packet, 0, packet_size* num_slots);
		}
		int32 menu_data = 3;
		int32 effective_level = player->GetInfoStructUInt("effective_level");

		int32 levelsLowered = (effective_level > 0 && effective_level < player->GetLevel()) ? player->GetLevel() - effective_level : 0;

		for(int16 i=0;i<num_slots;i++){
			// override the item slot we currently check as the client has different ordering, we need to match it
			int16 itemIdx = player->ConvertSlotFromClient(i, version);
			
			menu_data = 3;
			item = items[itemIdx];
			if(item && item->details.item_id > 0){
				if(item->slot_data.size() > 0)
					menu_data -= ITEM_MENU_TYPE_GENERIC;
				if (item->details.num_slots > 0) {
					int8 max_slots = player->GetMaxBagSlots(version);
					if (item->details.num_slots > max_slots)
						item->details.num_slots = max_slots;
					
					menu_data += ITEM_MENU_TYPE_BAG;
					
					if (item->details.num_free_slots == item->details.num_slots)
						menu_data += ITEM_MENU_TYPE_EMPTY_BAG;
				}
				if(item->IsSkill())
					menu_data += ITEM_MENU_TYPE_SCRIBE;
				if(item->generic_info.condition == 0)
					menu_data += ITEM_MENU_TYPE_BROKEN2;
				else if (item->generic_info.condition <= 20)
					menu_data += ITEM_MENU_TYPE_DAMAGED;
				if (item->CheckFlag(ATTUNED) || item->CheckFlag(NO_TRADE)) {
					if (version <= 373)
						menu_data += ORIG_ITEM_MENU_TYPE_ATTUNED;
					else
						menu_data += ITEM_MENU_TYPE_ATTUNED;
				}
				else if (item->CheckFlag(ATTUNEABLE)) {
					if (version <= 373)
						menu_data += ORIG_ITEM_MENU_TYPE_ATTUNEABLE;
					else
						menu_data += ITEM_MENU_TYPE_ATTUNEABLE;
				}
				if (item->generic_info.usable == 1)
					menu_data += ITEM_MENU_TYPE_USE;
				if (item->IsFood())
				{
					if (version <= 373) {						
						if (item->IsFoodDrink())
							menu_data += ORIG_ITEM_MENU_TYPE_DRINK;
						else
							menu_data += ORIG_ITEM_MENU_TYPE_FOOD;
					}
					else {
							menu_data += ITEM_MENU_TYPE_CONSUME;
						if (player && ((item->IsFoodFood() && player->get_character_flag(CF_FOOD_AUTO_CONSUME)) || (item->IsFoodDrink() && player->get_character_flag(CF_DRINK_AUTO_CONSUME))))
						{
							// needs all 3 to display 'auto consume' off option as well as set the yellowish tint in the background
							menu_data += ITEM_MENU_TYPE_CONSUME_OFF;
							menu_data += ORIG_ITEM_MENU_TYPE_DRINK;
							menu_data += ORIG_ITEM_MENU_TYPE_FOOD;
						}
					}
				}
				packet->setSubstructArrayDataByName("items", "unique_id", item->details.unique_id, 0, i);
				packet->setSubstructArrayDataByName("items", "bag_id", item->details.bag_id, 0, i);
				packet->setSubstructArrayDataByName("items", "inv_slot_id", item->details.inv_slot_id, 0, i);
				if (item->details.count > 0 && item->stack_count > 1) {
					if (version <= 373)
						menu_data += ORIG_ITEM_MENU_TYPE_STACKABLE;
					else
						menu_data += ITEM_MENU_TYPE_DISPLAY_CHARGES;
				}
				if(levelsLowered && item->details.recommended_level > effective_level)
					menu_data += ITEM_MENU_TYPE_MENTORED;
				packet->setSubstructArrayDataByName("items", "menu_type", menu_data, 0, i);
				packet->setSubstructArrayDataByName("items", "icon", item->GetIcon(version), 0, i);
				packet->setSubstructArrayDataByName("items", "slot_id", player->ConvertSlotToClient(item->details.equip_slot_id > 0 ? item->details.equip_slot_id : item->details.slot_id, version), 0, i);
				packet->setSubstructArrayDataByName("items", "count", item->details.count, 0, i);
				// item level needed here
				
				if(rule_manager.GetZoneRule(player->GetZoneID(), R_World, DisplayItemTiers)->GetBool()) {
					packet->setSubstructArrayDataByName("items", "tier", item->details.tier, 0, i);
				}
				packet->setSubstructArrayDataByName("items", "num_slots", item->details.num_slots, 0, i);
				//empty slots needed here
				packet->setSubstructArrayDataByName("items", "item_id", item->details.item_id, 0, i);
				//broker id needed here
				packet->setSubstructArrayDataByName("items", "name", item->name.c_str(), 0, i);
				
				//packet->setSubstructArrayDataByName("items", "unknown4", 10, 0, i);
				
				item->details.index = i;
			}
			packet->setSubstructArrayDataByName("items", "index", i, 0, i);
		}
		packet->setDataByName("equip_flag", GetAppearanceType() ? 2 : 1);
		app = packet->serializeCountPacket(version, 1, orig_packet, xor_packet);
		safe_delete(packet);
	}
	MEquipmentItems.unlock();
	return app;
}
ItemStatsValues* EquipmentItemList::CalculateEquipmentBonuses(Entity* entity){
	ItemStatsValues* stats = new ItemStatsValues;
	memset(stats, 0, sizeof(ItemStatsValues));
	entity->GetInfoStruct()->set_mitigation_base(0);
	MEquipmentItems.lock();
	for(int8 i=0;i<NUM_SLOTS;i++){
		if(items[i] && items[i]->details.item_id > 0){
			master_item_list.CalculateItemBonuses(items[i], entity, stats);
			if (items[i]->armor_info && !items[i]->IsShield())
				entity->GetInfoStruct()->add_mitigation_base(items[i]->armor_info->mitigation_high);
		}
	}
	MEquipmentItems.unlock();
	return stats;
}
bool EquipmentItemList::HasItem(int32 id){
	MEquipmentItems.lock();
	for(int8 i=0;i<NUM_SLOTS;i++){
		if(items[i] && items[i]->details.item_id == id){
			MEquipmentItems.unlock();
			return true;
		}
	}
	MEquipmentItems.unlock();
	return false;
}
void EquipmentItemList::RemoveItem(int8 slot, bool delete_item){
	if(slot < NUM_SLOTS){
		MEquipmentItems.lock();
		if(items[slot] && items[slot]->details.appearance_type)
			items[slot]->details.appearance_type = 0;
		
		if(delete_item){
			safe_delete(items[slot]);
		}
		items[slot] = 0;
		MEquipmentItems.unlock();
	}
}

Item* EquipmentItemList::GetItemFromUniqueID(int32 item_id){
	MEquipmentItems.lock();
	for(int8 i=0;i<NUM_SLOTS;i++){
		if(items[i] && items[i]->details.unique_id == item_id){
			MEquipmentItems.unlock();
			return items[i];
		}
	}
	MEquipmentItems.unlock();
	return 0;
}

Item* EquipmentItemList::GetItemFromItemID(int32 item_id) {
	Item* item = 0;
	MEquipmentItems.lock();
	for(int8 i = 0; i < NUM_SLOTS; i++) {
		if(items[i] && items[i]->details.item_id == item_id) {
			item = items[i];
			break;
		}
	}
	MEquipmentItems.unlock();
	return item;
}

bool EquipmentItemList::CanItemBeEquippedInSlot(Item* tmp, int8 slot){
	MEquipmentItems.lock();
	for(int8 i=0;tmp && i<tmp->slot_data.size();i++){
		if(tmp->slot_data[i] == slot){
			MEquipmentItems.unlock();
			return true;
		}
	}
	MEquipmentItems.unlock();
	return false;
}
bool EquipmentItemList::CheckEquipSlot(Item* tmp, int8 slot){
	MEquipmentItems.lock();
	for(int8 i=0;tmp && i<tmp->slot_data.size();i++){
		if(tmp->slot_data[i] == slot){
			Item* tmp_item = GetItem(tmp->slot_data[i]);
			if(!tmp_item || tmp_item->details.item_id == 0){
				if(slot == EQ2_SECONDARY_SLOT)
				{
					Item* primary = GetItem(EQ2_PRIMARY_SLOT);
					if(primary && primary->weapon_info->wield_type == ITEM_WIELD_TYPE_TWO_HAND)
						continue;
				}
				MEquipmentItems.unlock();
				return true;
			}
		}
	}
	MEquipmentItems.unlock();
	return false;
}

int8 EquipmentItemList::GetFreeSlot(Item* tmp, int8 slot_id, int16 version){
	int8 slot = 0;
	MEquipmentItems.lock();
	for(int8 i=0;tmp && i<tmp->slot_data.size();i++){
		slot = tmp->slot_data[i];
		if(slot_id == 255 || slot == slot_id){
			Item* tmp_item = GetItem(slot);
			if(!tmp_item || tmp_item->details.item_id == 0){
				if(slot == EQ2_SECONDARY_SLOT)
				{
					Item* primary = GetItem(EQ2_PRIMARY_SLOT);
					if(primary && primary->weapon_info->wield_type == ITEM_WIELD_TYPE_TWO_HAND)
						continue;
				}
				MEquipmentItems.unlock();
				return slot;
			}
			else if ( slot == EQ2_LRING_SLOT || slot == EQ2_EARS_SLOT_1 || slot == EQ2_LWRIST_SLOT || slot == EQ2_CHARM_SLOT_1)
			{
				if(version <= 561 && slot == EQ2_EARS_SLOT_1)
					continue;
			
				Item* rslot = GetItem(slot+1);
				if(!rslot)
				{
					MEquipmentItems.unlock();
					return slot+1;
				}
			}
		}
	}
	MEquipmentItems.unlock();
	return 255;
}

int32 EquipmentItemList::CheckSlotConflict(Item* item, bool check_lore_only, int16* lore_stack_count) {
	bool is_lore = false;
	bool is_stack_lore = false;
	if(!(is_lore = item->CheckFlag(LORE)) && !(is_stack_lore = item->CheckFlag(STACK_LORE)) && check_lore_only) {
		return 0;
	}
	
	if(!check_lore_only && !is_lore && !is_stack_lore && !item->CheckFlag(LORE_EQUIP)) {
		return 0;
	}
	
	int32 conflict = 0;
	MEquipmentItems.lock();
	for(int8 i=0;i<NUM_SLOTS;i++){
		if(items[i] && items[i]->details.item_id == item->details.item_id) {
			if(lore_stack_count)
				*lore_stack_count += items[i]->details.count;
			if(!is_stack_lore && items[i]->CheckFlag(LORE)) {
				conflict = LORE;
				break;
			}
			else if(is_stack_lore && (*lore_stack_count + item->details.count) > items[i]->stack_count) {
				conflict = STACK_LORE;
				break;
			}
			else if(!check_lore_only && items[i]->CheckFlag(LORE_EQUIP)) {
				conflict = LORE_EQUIP;
				break;
			}
		}
	}

	MEquipmentItems.unlock();
	return conflict;
}

int8 EquipmentItemList::GetSlotByItem(Item* item) {
	int8 slot = 255;
	for (int8 i = 0; i < NUM_SLOTS; i++) {
		if (items[i] && items[i] == item) {
			slot = i;
			break;
		}
	}
	return slot;
}

string Item::CreateItemLink(int16 client_Version, bool bUseUniqueID) {
	ostringstream ss;
	if(client_Version > 561)
		ss << "\\aITEM " << details.item_id << ' ' << (bUseUniqueID ? details.unique_id : 0) << ':' << name << "\\/a";
	else {
		if(bUseUniqueID)
			ss << "\\aITEM " << details.item_id << ' ' << details.unique_id << ':' << name << "\\/a";
		else
			ss << "\\aITEM " << details.item_id << ' ' << name << ':' << name << "\\/a";
	}
	return ss.str();
}

int16 Item::GetIcon(int16 version) {
	if(version <= 561 && details.classic_icon) {
		return details.classic_icon;
	}
	
	return details.icon;
}

bool Item::TryLockItem(LockReason reason) {
	std::unique_lock lock(item_lock_mtx_);
	// current flags
	auto cur = static_cast<LockReason>(details.lock_flags);

	// 0) If this reason is already applied, succeed immediately
	if ((cur & reason) == reason) {
		return true;
	}

	// 1) No lock held? allow any first‐lock
	if (cur == LockReason::LockReason_None) {
		details.lock_flags = static_cast<uint32>(reason);
		details.item_locked = true;
		return true;
	}

	// 2) Only House‐lock held, and we're adding Shop‐lock? allow
	if ((cur == LockReason::LockReason_House && reason == LockReason::LockReason_Shop) ||
		(cur == LockReason::LockReason_Shop && reason == LockReason::LockReason_House)) {
		details.lock_flags = static_cast<uint32>(cur | reason);
		// item_locked already true
		return true;
	}

	// 3) Anything else: reject
	return false;
}

bool Item::TryUnlockItem(LockReason reason) {
	std::unique_lock lock(item_lock_mtx_);
	LockReason cur = static_cast<LockReason>(details.lock_flags);
	if ((cur & reason) == reason) {
		details.lock_flags = int32(cur & ~reason);
		if (details.lock_flags == 0)
			details.item_locked = false;
		return true;
	}
	return false;
}

bool Item::IsItemLocked() {
	std::shared_lock lock(item_lock_mtx_);
	return details.lock_flags != 0;
}

bool Item::IsItemLockedFor(LockReason reason) {
	std::shared_lock lock(item_lock_mtx_);
	return (static_cast<LockReason>(details.lock_flags) & reason) == reason;
}

int32 MasterItemList::GetItemStatIDByName(std::string name)
{
	boost::to_lower(name);
	map<std::string, int32>::iterator itr = mappedItemStatsStrings.find(name.c_str());
	if(itr != mappedItemStatsStrings.end())
		return itr->second;

	return 0xFFFFFFFF;
}

std::string MasterItemList::GetItemStatNameByID(int32 id)
{
	map<int32, std::string>::iterator itr = mappedItemStatTypeIDs.find(id);
	if(itr != mappedItemStatTypeIDs.end())
		return itr->second;

	return std::string("");
}
