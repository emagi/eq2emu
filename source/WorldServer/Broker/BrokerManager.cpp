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

#include "BrokerManager.h"
#include "../Items/Items.h"
#include "../../common/Log.h"
#include "../WorldDatabase.h"
#include "../World.h"
#include "../Web/PeerManager.h"
#include <cstdlib>

extern WorldDatabase database;
extern ZoneList zone_list;
extern PeerManager peer_manager;

BrokerManager::BrokerManager() {
	
}

void BrokerManager::AddSeller(int32 cid,
							  const std::string& name,
							  int32 hid,
							  bool enabled,
							  bool invFlag)
{
	
	int64 prevSession = 0, prevTotal = 0;
	{
		std::shared_lock lock(mtx_);
		auto sit = players_.find(cid);
		if (sit != players_.end()) {
			prevSession = sit->second.coin_session;
			prevTotal   = sit->second.coin_total;
		}
	}

	SellerInfo info{cid, name, hid, enabled, invFlag, prevSession, prevTotal};
	{
		std::unique_lock lock(mtx_);
		players_[cid] = info;
	}

	SavePlayerToDB(info);
	peer_manager.sendPeersAddSeller(cid, hid, name, enabled, invFlag);
}

void BrokerManager::LoadSeller(int32 cid, const std::string& name,
							   int32 hid, bool enabled, bool invFlag,
							   int64 coin_session, int64 coin_total) 
{
	SellerInfo info{cid, name, hid, enabled, invFlag, coin_session, coin_total};
	{
		std::unique_lock lock(mtx_);
		players_[cid] = info;
	}
}

int64 BrokerManager::ResetSellerSessionCoins(int32 cid) {
	database.ClearSellerSession(cid);
	
	int64 session_coin = 0;
	{
		std::unique_lock lock(mtx_);
		auto it = players_.find(cid);
		if (it == players_.end()) return 0;
		session_coin = it->second.coin_session;
		it->second.coin_total += it->second.coin_session;
		it->second.coin_session = 0;
	}
	
	return session_coin;
}

void BrokerManager::AddSellerSessionCoins(int32 cid, uint64 session) {
	database.AddToSellerSession(cid, session);
	{
		std::unique_lock lock(mtx_);
		auto it = players_.find(cid);
		if (it == players_.end()) return;
		it->second.coin_session += session;
	}
}

void BrokerManager::RemoveSeller(int32 cid, bool peerCacheOnly) 
{
	{
		std::unique_lock lock(mtx_);
		players_.erase(cid);
		active_items_by_char_.erase(cid);
		inactive_items_by_char_.erase(cid);
	}
	if(!peerCacheOnly)
		peer_manager.sendPeersRemoveSeller(cid);
}

void BrokerManager::AddItem(const SaleItem& item, bool peerCacheOnly) 
{
	{
		std::unique_lock lock(mtx_);
		auto& a = active_items_by_char_[item.character_id];
		auto& i = inactive_items_by_char_[item.character_id];
		a.erase(item.unique_id);
		i.erase(item.unique_id);
		if (item.for_sale)  a[item.unique_id] = item;
		else                i[item.unique_id] = item;
	}
	SaveItemToDB(item);
	if(!peerCacheOnly)
		peer_manager.sendPeersAddItemSale(item.character_id, item.house_id, item.item_id, item.unique_id, item.cost_copper, item.inv_slot_id, 
											item.slot_id, item.count, item.from_inventory, item.for_sale, item.creator);
}

void BrokerManager::LoadItem(const SaleItem& item) 
{
	std::unique_lock lock(mtx_);
	if (item.for_sale)
		active_items_by_char_[item.character_id][item.unique_id] = item;
	else
		inactive_items_by_char_[item.character_id][item.unique_id] = item;
}

void BrokerManager::SetSaleStatus(int32 cid, int64 uid, bool for_sale) 
{
	std::optional<SaleItem> toUpdate;
	{
		std::unique_lock lock(mtx_);
		auto& a = active_items_by_char_[cid];
		auto& i = inactive_items_by_char_[cid];

		if (for_sale) {
			LogWrite(PLAYER__ERROR, 5, "Broker",
			  "--SetSaleStatus: %u (%u), for_sale=%u",
			  cid, uid, for_sale
			);
			if (auto it = i.find(uid); it != i.end()) {
			LogWrite(PLAYER__ERROR, 5, "Broker",
			  "--SetSaleStatusFuckOff: %u (%u), for_sale=%u",
			  cid, uid, for_sale
			);
				SaleItem copy = it->second;
				copy.for_sale = true;
				i.erase(it);
				a[uid] = copy;
				toUpdate = copy;
			}
		} else {
			LogWrite(PLAYER__ERROR, 5, "Broker",
			  "--SetSaleStatusInactive: %u (%u), for_sale=%u",
			  cid, uid, for_sale
			);
			if (auto it = a.find(uid); it != a.end()) {
			LogWrite(PLAYER__ERROR, 5, "Broker",
			  "--SetSaleStatusFuckyou: %u (%u), for_sale=%u",
			  cid, uid, for_sale
			);
				SaleItem copy = it->second;
				copy.for_sale = false;
				a.erase(it);
				i[uid] = copy;
				toUpdate = copy;
			}
		}
	}
	if (toUpdate) {
		SaveItemToDB(*toUpdate);
		peer_manager.sendPeersAddItemSale(toUpdate->character_id, toUpdate->house_id, toUpdate->item_id, toUpdate->unique_id, toUpdate->cost_copper, toUpdate->inv_slot_id, 
											toUpdate->slot_id, toUpdate->count, toUpdate->from_inventory, toUpdate->for_sale, toUpdate->creator);
	}
}

bool BrokerManager::IsItemListed(int32 cid, int64 uid) {
	std::shared_lock lock(mtx_);
	auto& active_map = active_items_by_char_[cid];
	auto& inactive_map = inactive_items_by_char_[cid];

	auto it = inactive_map.find(uid);
	if (it != inactive_map.end()) {
		return true;
	}
	
	auto it2 = active_map.find(uid);
	if (it2 != active_map.end()) {
		return true;
	}
	return false;
}
		
void BrokerManager::SetSalePrice(int32 cid, int64 uid, int64 price) 
{
	std::optional<SaleItem> toUpdate;
	{
		std::unique_lock lock(mtx_);
		if (auto ait = active_items_by_char_.find(cid); ait != active_items_by_char_.end()) {
			if (auto it = ait->second.find(uid); it != ait->second.end()) {
				
			LogWrite(PLAYER__ERROR, 5, "Broker",
			  "--SetSalePriceActive: %u (%u), cost=%u",
			  cid, uid, price
			);
				it->second.cost_copper = price;
				toUpdate = it->second;
			}
		}
		if (auto iit = inactive_items_by_char_.find(cid); iit != inactive_items_by_char_.end()) {
			if (auto it = iit->second.find(uid); it != iit->second.end()) {
			LogWrite(PLAYER__ERROR, 5, "Broker",
			  "--SetSalePriceInactive: %u (%u), cost=%u",
			  cid, uid, price
			);
				it->second.cost_copper = price;
				toUpdate = it->second;
			}
		}
	}
	if (toUpdate) {
		SaveItemToDB(*toUpdate);
		peer_manager.sendPeersAddItemSale(toUpdate->character_id, toUpdate->house_id, toUpdate->item_id, toUpdate->unique_id, toUpdate->cost_copper, toUpdate->inv_slot_id, 
											toUpdate->slot_id, toUpdate->count, toUpdate->from_inventory, toUpdate->for_sale, toUpdate->creator);
	}
}

void BrokerManager::RemoveItem(int32 cid, int64 uid, int16 qty, bool shouldDelete) 
{
	bool didDelete = false;
	std::optional<SaleItem> snapshot;
	{
		std::unique_lock lock(mtx_);
		if (auto ait = active_items_by_char_.find(cid); ait != active_items_by_char_.end()) {
			auto& m = ait->second;
			if (auto it = m.find(uid); it != m.end()) {
				if(shouldDelete)
					qty = it->second.count;
				
				it->second.count -= qty;
				if (it->second.count <= 0) {
					didDelete = true;
					snapshot = it->second;
					m.erase(it);
				} else {
					snapshot = it->second;
				}
				if (m.empty())
					active_items_by_char_.erase(ait);
			}
		}
	}
	if (didDelete || shouldDelete) {
		DeleteItemFromDB(cid, uid);
		peer_manager.sendPeersRemoveItemSale(cid, uid);
	}
	else if (snapshot) {
		SaveItemToDB(*snapshot);
		peer_manager.sendPeersAddItemSale(snapshot->character_id, snapshot->house_id, snapshot->item_id, snapshot->unique_id, snapshot->cost_copper, snapshot->inv_slot_id, 
											snapshot->slot_id, snapshot->count, snapshot->from_inventory, snapshot->for_sale, snapshot->creator);
	}
}

bool BrokerManager::BuyItem(Client* buyer, int32 seller_cid, int64 uid, int32 quantity) 
{
	Client* seller = zone_list.GetClientByCharID(seller_cid); // establish if seller is online
	
	if(buyer && buyer->GetCharacterID() == seller_cid) {
		buyer->Message(CHANNEL_COLOR_RED, "You cannot buy from yourself!");
		return false;
	}
	if (quantity <= 0 || !IsSaleEnabled(seller_cid) || !IsItemForSale(seller_cid, uid)) {
		if(buyer)
			buyer->Message(CHANNEL_COLOR_RED, "Quantity not provided (%u), sale is not enabled (sale enabled? %u) or item is not for sale (itemforsale? %u).", quantity, IsSaleEnabled(seller_cid), IsItemForSale(seller_cid, uid));
		return false;
	}
	int64 price = GetSalePrice(seller_cid, uid) * quantity;
	
	if(!buyer || !buyer->GetPlayer() || !buyer->GetPlayer()->RemoveCoins(price)) {
		if(buyer)
			buyer->Message(CHANNEL_COLOR_RED, "You do not have enough coin to purchase.");
		return false;
	}
	
	if (!database.RemoveBrokerItem(seller_cid, uid, quantity)) {
		buyer->GetPlayer()->AddCoins(price);
		buyer->Message(CHANNEL_COLOR_RED, "Failed to remove broker item from database.");
		return false;
	}
	
	Item* giveItem = nullptr;
	int16 result_count = 0;
	std::string creator;
	bool deleteItem = false;
	int16 quantity_left = 0;
	// 2) Mirror in-memory
	
	std::optional<SaleItem> toUpdate;
	{
		std::unique_lock lock(mtx_);
		if (auto sit = active_items_by_char_.find(seller_cid); sit != active_items_by_char_.end()) {
			auto& m = sit->second;
			if (auto it = m.find(uid); it != m.end()) {
				creator = it->second.creator;
				giveItem = master_item_list.GetItem(it->second.item_id);
				SaleItem copy = it->second;
				toUpdate = copy;
				if (it->second.count > quantity) {
					it->second.count -= quantity;
					quantity_left = it->second.count;
					result_count = quantity;
					if(seller && seller->GetPlayer()) {
						seller->GetPlayer()->item_list.SetVaultItemUniqueIDCount(seller, uid, it->second.count);
					}
				}
				else {
					result_count = it->second.count;
					if(seller && seller->GetPlayer()) {
						seller->GetPlayer()->item_list.RemoveVaultItemFromUniqueID(seller, uid);
					}
					m.erase(it);
					deleteItem = true;
				}
				
				if (m.empty())
					active_items_by_char_.erase(sit);
			}
		}
	}
	
	if(!giveItem) {
		buyer->GetPlayer()->AddCoins(price);
		buyer->Message(CHANNEL_COLOR_RED, 	"Failed to find item on broker memory.");
		if(toUpdate)
			AddItem(*toUpdate);
		return false;
	}
	Item* resultItem = new Item(giveItem);
	resultItem->details.count = result_count;
	resultItem->creator = creator;
	if (buyer->GetPlayer()->item_list.HasFreeSlot() || buyer->GetPlayer()->item_list.CanStack(resultItem, false)) {
		if(!buyer->AddItem(resultItem, nullptr, AddItemType::BUY_FROM_BROKER)) {
			buyer->GetPlayer()->AddCoins(price);
			safe_delete(resultItem);
			if(toUpdate)
				AddItem(*toUpdate);
			return false;
		}
	}
	else {
		buyer->Message(CHANNEL_COLOR_RED, 	"You have no free slot available.");
		buyer->GetPlayer()->AddCoins(price);
		safe_delete(resultItem);
		if(toUpdate)
			AddItem(*toUpdate);
		return false;
	}
	
	if(deleteItem) {
		DeleteItemFromDB(seller_cid, uid);
		DeleteCharacterItemFromDB(seller_cid, uid);
	}
	else if(quantity_left) {
		UpdateCharacterItemDB(seller_cid, uid, quantity_left);
	}
	
	AddSellerSessionCoins(seller_cid, price);
	LogSale(seller_cid, std::string(buyer->GetPlayer()->GetName()), std::string(resultItem->name), result_count, price);
	if(seller) {
		std::string logMsg = GetShopPurchaseMessage(buyer->GetPlayer()->GetName(), std::string(resultItem->name), result_count, price);
		auto seller_info = GetSellerInfo(seller_cid);
		seller->SendHouseSaleLog(logMsg, 0, seller_info ? seller_info->coin_total : 0, 0);
		seller->OpenShopWindow(nullptr);	
	}
	return true;
}

void BrokerManager::OnPeerRemoveItem(int32 cid, int64 uid) 
{
	std::unique_lock lock(mtx_);
	if (auto ait = active_items_by_char_.find(cid); ait != active_items_by_char_.end())
		ait->second.erase(uid);
	if (auto iit = inactive_items_by_char_.find(cid); iit != inactive_items_by_char_.end())
		iit->second.erase(uid);
}

bool BrokerManager::IsItemForSale(int32 cid, int64 uid) const 
{
	std::shared_lock lock(mtx_);
	if (auto pit = active_items_by_char_.find(cid); pit != active_items_by_char_.end())
		return pit->second.find(uid) != pit->second.end();
	return false;
}

int64 BrokerManager::GetSalePrice(int32 cid, int64 uid) 
{
	std::shared_lock lock(mtx_);
	if (auto pit = active_items_by_char_.find(cid); pit != active_items_by_char_.end()) {
		if (auto it = pit->second.find(uid); it != pit->second.end())
			return it->second.cost_copper;
	}
	if (auto pit2 = inactive_items_by_char_.find(cid); pit2 != inactive_items_by_char_.end()) {
		if (auto it = pit2->second.find(uid); it != pit2->second.end())
			return it->second.cost_copper;
	}
	return 0;
}

std::vector<SaleItem> BrokerManager::GetActiveForSaleItems(int32 cid) const 
{
	std::shared_lock lock(mtx_);
	std::vector<SaleItem> out;
	if (auto pit = active_items_by_char_.find(cid); pit != active_items_by_char_.end()) {
		for (auto const& kv : pit->second)
			out.push_back(kv.second);
	}
	return out;
}

std::optional<SaleItem> BrokerManager::GetActiveItem(int32 cid, int64 uid) const
{
	std::shared_lock lock(mtx_);
	auto pit = active_items_by_char_.find(cid);
	if (pit == active_items_by_char_.end())
		return std::nullopt;

	auto it = pit->second.find(uid);
	if (it == pit->second.end())
		return std::nullopt;

	return it->second;  // copy out the SaleItem
}

bool BrokerManager::IsSellingItems(int32 cid, bool vaultOnly) const
{
	// Grab shared lock for thread‐safe read
	std::shared_lock lock(mtx_);

	// Find this character’s active sales
	auto pit = active_items_by_char_.find(cid);
	if (pit == active_items_by_char_.end())
		return false;  // no items => not selling from vault

	// Scan through each SaleItem; if any has sell_from_inventory == false,
	// that means it’s coming from the vault
	for (auto const& kv : pit->second) {
		const SaleItem& item = kv.second;
		if ((item.for_sale && (!item.from_inventory || !vaultOnly)))
			return true;
	}

	return false;
}

std::vector<SaleItem> BrokerManager::GetInactiveItems(int32 cid) const 
{
	std::shared_lock lock(mtx_);
	std::vector<SaleItem> out;
	if (auto pit = inactive_items_by_char_.find(cid); pit != inactive_items_by_char_.end()) {
		for (auto const& kv : pit->second)
			out.push_back(kv.second);
	}
	return out;
}

std::vector<std::pair<int64,int32>>
BrokerManager::GetUniqueIDsAndCost(int32 cid) const 
{
	std::shared_lock lock(mtx_);
	std::vector<std::pair<int64,int32>> out;
	if (auto pit = active_items_by_char_.find(cid); pit != active_items_by_char_.end()) {
		for (auto const& kv : pit->second)
			out.emplace_back(kv.second.unique_id, kv.second.cost_copper);
	}
	return out;
}

vector<Item*>* BrokerManager::GetItems(
	const std::string& name,
	int64 itype,
	int64 ltype,
	int64 btype,
	int64 minprice,
	int64 maxprice,
	int8  minskill,
	int8  maxskill,
	const std::string& seller,
	const std::string& adornment,
	int8  mintier,
	int8  maxtier,
	int16 minlevel,
	int16 maxlevel,
	int8  itemclass
) const {
	vector<Item*>* ret = new vector<Item*>;
	string lower_name = ::ToLower(string(name.c_str()));
	std::shared_lock lock(mtx_);
	std::vector<SaleItem> out;
	for (auto const& char_pair : active_items_by_char_) {
		int32 cid = char_pair.first;
		auto pit_player = players_.find(cid);
		if (pit_player == players_.end())
			continue;
		bool allowInv = pit_player->second.sell_from_inventory;
		for (auto const& kv : char_pair.second) {
			auto const& itm = kv.second;
			
			LogWrite(PLAYER__ERROR, 5, "Broker",
			  "--GetItems: %u (selling: %u), allowinv: %u",
			  itm.unique_id, itm.for_sale, allowInv
			);
			if (!itm.for_sale) continue;
			if (!allowInv && itm.from_inventory) continue;
			Item* def = master_item_list.GetItem(itm.item_id);
			if (!def) continue;
			
			LogWrite(PLAYER__ERROR, 5, "Broker",
			  "--GetItems#1: %u (selling: %u), allowinv: %u",
			  itm.unique_id, itm.for_sale, allowInv
			);
			
			if (!name.empty() && def->lowername.find(lower_name) == std::string::npos) continue;
			if (itype!=ITEM_BROKER_TYPE_ANY && itype !=ITEM_BROKER_TYPE_ANY64BIT && !master_item_list.ShouldAddItemBrokerType(def, itype)) continue;
			if (ltype!=ITEM_BROKER_SLOT_ANY && !master_item_list.ShouldAddItemBrokerSlot(def, ltype)) continue;
			if (btype!=0xFFFFFFFF && !master_item_list.ShouldAddItemBrokerStat(def, btype)) continue;
			
			LogWrite(PLAYER__ERROR, 5, "Broker",
			  "--GetItems#2: %u (cost_copper: %u), seller: %s",
			  itm.unique_id, itm.cost_copper, seller.c_str()
			);
			
			//if (itm.cost_copper < minprice || itm.cost_copper > maxprice) continue;
			//if (!seller.empty() && pit_player->second.seller_name.find(seller)==std::string::npos) continue;
			/*if(mintier > 1 && def->details.tier < mintier)
				continue;
			if(maxtier > 0 && def->details.tier > maxtier)
				continue;*/
			
			LogWrite(PLAYER__ERROR, 5, "Broker",
			  "--GetItems#3: %u (selling: %u), allowinv: %u",
			  itm.unique_id, itm.for_sale, allowInv
			);
			
			if(def->generic_info.adventure_default_level == 0 && def->generic_info.tradeskill_default_level == 0 && minlevel > 0 && maxlevel > 0){
				if(def->details.recommended_level < minlevel)
					continue;
				if(def->details.recommended_level > maxlevel)
					continue;
			}
			else{
				if(minlevel > 0 && ((def->generic_info.adventure_default_level == 0 && def->generic_info.tradeskill_default_level == 0) || (def->generic_info.adventure_default_level > 0 && def->generic_info.adventure_default_level < minlevel) || (def->generic_info.tradeskill_default_level > 0 && def->generic_info.tradeskill_default_level < minlevel)))
					continue;
				if(maxlevel > 0 && ((def->generic_info.adventure_default_level > 0 && def->generic_info.adventure_default_level > maxlevel) || (def->generic_info.tradeskill_default_level > 0 && def->generic_info.tradeskill_default_level > maxlevel)))
					continue;
			}
			if (itemclass>0) {
				int64 bit = ((int64)2 << (itemclass-1));
				if (!(def->generic_info.adventure_classes & bit) &&
					!(def->generic_info.tradeskill_classes & bit))
					continue;
			}
			
			LogWrite(PLAYER__ERROR, 5, "Broker",
			  "--GetItemsPass: %u (selling: %u), allowinv: %u",
			  itm.unique_id, itm.for_sale, allowInv
			);
			ret->push_back(new Item(def, itm.unique_id, itm.creator, pit_player->second.seller_name, cid, itm.cost_copper, itm.count, pit_player->second.house_id, itm.from_inventory));
		}
	}
	return ret;
}

std::string BrokerManager::GetSellerName(int32 cid) const {
	std::shared_lock lock(mtx_);
	auto it = players_.find(cid);
	return (it != players_.end()) ? it->second.seller_name : std::string();
}

bool BrokerManager::IsSaleEnabled(int32 cid) const {
	std::shared_lock lock(mtx_);
	auto it = players_.find(cid);
	return (it != players_.end()) ? it->second.sale_enabled : false;
}

bool BrokerManager::CanSellFromInventory(int32 cid) const {
	std::shared_lock lock(mtx_);
	auto it = players_.find(cid);
	return (it != players_.end()) ? it->second.sell_from_inventory : false;
}

int32 BrokerManager::GetHouseID(int32 cid) const {
	std::shared_lock lock(mtx_);
	auto it = players_.find(cid);
	return (it != players_.end()) ? it->second.house_id : -1;
}

std::optional<SellerInfo> BrokerManager::GetSellerInfo(int32 cid) const {
	std::shared_lock lock(mtx_);
	auto it = players_.find(cid);
	if (it == players_.end())
		return std::nullopt;
	return it->second;
}

void BrokerManager::SavePlayerToDB(const SellerInfo& p) {
	Query q;
	std::ostringstream oss;
	oss << "INSERT INTO broker_sellers "
		   "(character_id,seller_name,house_id,sale_enabled,sell_from_inventory,coin_session,coin_total) "
		   "VALUES("
		<< p.character_id << ",'"
		<< EscapeSQLString(p.seller_name)    << "',"
		<< p.house_id       << ","
		<< (p.sale_enabled ? 1 : 0) << ","
		<< (p.sell_from_inventory ? 1 : 0) << ","
		<< p.coin_session << ","
		<< p.coin_total
		<< ") ON DUPLICATE KEY UPDATE "
		   "seller_name=VALUES(seller_name),"
		   "house_id=VALUES(house_id),"
		   "sale_enabled=VALUES(sale_enabled),"
		   "sell_from_inventory=VALUES(sell_from_inventory),"
			"coin_session=VALUES(coin_session), "
			"coin_total=VALUES(coin_total)";

	std::string sql = oss.str();
	
	q.AddQueryAsync(p.character_id, &database, Q_INSERT,
	  sql.c_str()
	);
}
#include <sstream>

void BrokerManager::SaveItemToDB(const SaleItem& i) {
	// 2) Build full SQL
	std::ostringstream oss;
	oss
	  << "INSERT INTO broker_items "
		 "(unique_id,character_id,house_id,item_id,cost_copper,for_sale,"
		 "inv_slot_id,slot_id,`count`,from_inventory,creator) "
		 "VALUES("
	  << i.unique_id                << ","
	  << i.character_id             << ","
	  << i.house_id                 << ","
	  << i.item_id                  << ","
	  << i.cost_copper              << ","
	  << (i.for_sale ? 1 : 0)       << ","
	  << i.inv_slot_id              << ","
	  << i.slot_id                  << ","
	  << i.count                    << ","
	  << (i.from_inventory ? 1 : 0) << ",'"
	  << EscapeSQLString(i.creator)                 << "') "
		 "ON DUPLICATE KEY UPDATE "
		 "house_id=VALUES(house_id),"
		 "item_id=VALUES(item_id),"
		 "cost_copper=VALUES(cost_copper),"
		 "for_sale=VALUES(for_sale),"
		 "inv_slot_id=VALUES(inv_slot_id),"
		 "slot_id=VALUES(slot_id),"
		 "`count`=VALUES(`count`),"
		 "from_inventory=VALUES(from_inventory),"
		 "creator=VALUES(creator)";

	std::string sql = oss.str();
	Query q;
	q.AddQueryAsync(i.character_id, &database, Q_INSERT, sql.c_str());
}

void BrokerManager::UpdateItemInDB(const SaleItem& i) {
	Query q;
	std::ostringstream oss;
	oss << "UPDATE broker_items SET "
		<< "house_id="      << i.house_id
		<< ",item_id="      << i.item_id
		<< ",cost_copper="  << i.cost_copper
		<< ",for_sale="     << (i.for_sale ? 1 : 0)
		<< ",inv_slot_id="  << i.inv_slot_id
		<< ",slot_id="      << i.slot_id
		<< ",count="        << i.count
		<< ",from_inventory=" << (i.from_inventory ? 1 : 0)
		<< ",creator='"     << EscapeSQLString(i.creator) << "' "
		<< "WHERE unique_id="     << i.unique_id
		<< " AND character_id="   << i.character_id;

	std::string sql = oss.str();

	q.AddQueryAsync(i.character_id, &database, Q_UPDATE,
		sql.c_str()
	);
}

void BrokerManager::DeleteItemFromDB(int32 cid, int64 uid) {
	Query q;
	q.AddQueryAsync(cid, &database, Q_DELETE,
		"DELETE FROM broker_items WHERE unique_id=%llu AND character_id=%u",
		uid, cid
	);
}

void BrokerManager::DeleteCharacterItemFromDB(int32 cid, int64 uid) {
	Query q;
	q.AddQueryAsync(cid, &database, Q_DELETE,
		"DELETE FROM character_items WHERE id=%llu AND char_id=%u",
		uid, cid
	);
}

void BrokerManager::UpdateCharacterItemDB(int32 cid, int64 uid, int16 count) {
	Query q;
	q.AddQueryAsync(cid, &database, Q_UPDATE,
		"UPDATE character_items set count=%u WHERE id=%llu AND char_id=%u",
		count, uid, cid
	);
}

void BrokerManager::DeletePlayerFromDB(int32 cid) {
	Query q;
	// delete from broker_sellers
	q.AddQueryAsync(cid, &database, Q_DELETE,
		"DELETE FROM broker_sellers WHERE character_id=%u", cid);
	// delete all their items
	q.AddQueryAsync(cid, &database, Q_DELETE,
		"DELETE FROM broker_items WHERE character_id=%u", cid);
}

bool BrokerManager::IsItemFromInventory(int32 cid, int64 uid) const {
	std::shared_lock lock(mtx_);

	// 1) Check active items for that character
	auto pit = active_items_by_char_.find(cid);
	if (pit != active_items_by_char_.end()) {
		auto it = pit->second.find(uid);
		if (it != pit->second.end())
			return it->second.from_inventory;
	}

	// 2) Otherwise check inactive items
	auto iit = inactive_items_by_char_.find(cid);
	if (iit != inactive_items_by_char_.end()) {
		auto it = iit->second.find(uid);
		if (it != iit->second.end())
			return it->second.from_inventory;
	}

	// 3) Not found → false
	return false;
}


void BrokerManager::LockActiveItemsForClient(Client* client) const {
	int32 cid = client->GetCharacterID();

	auto infoOpt = GetSellerInfo(cid);
	if (!infoOpt) 
		return;
	const SellerInfo& info = *infoOpt;

	{
		auto items = GetActiveForSaleItems(cid);
		for (auto const& itm : items) {
			if (!itm.for_sale) {
				client->GetPlayer()->item_list.SetVaultItemLockUniqueID(client, itm.unique_id, false, true);
				continue;
			}
			
			LogWrite(PLAYER__ERROR, 5, "Broker",
			  "--LockActiveItemsForClient: %u (selling: %u), allowinv: %u, sellfrominv: %u",
			  itm.unique_id, itm.for_sale, itm.from_inventory, info.sell_from_inventory
			);
			
			if (!info.sell_from_inventory && itm.from_inventory) {
				client->GetPlayer()->item_list.SetVaultItemLockUniqueID(client, itm.unique_id, false, true);
				continue;
			}

			client->GetPlayer()->item_list.SetVaultItemLockUniqueID(client, itm.unique_id, true, true);
		}
	}
}


void WorldDatabase::ClearSellerSession(int32 character_id) {
	Query q;
		std::ostringstream sql;
	sql << "UPDATE broker_sellers SET coin_session = 0"
		<< " WHERE character_id = " << character_id;

	std::string full = sql.str();
	q.AddQueryAsync(character_id, &database, Q_INSERT, full.c_str());
}

void WorldDatabase::AddToSellerSession(int32 character_id, int64 amount) {
	Query q;
		std::ostringstream sql;
	sql << "UPDATE broker_sellers SET coin_session = coin_session + "
		<< amount 
		<< " WHERE character_id = " << character_id;

	std::string full = sql.str();
	q.AddQueryAsync(character_id, &database, Q_INSERT, full.c_str());
}

int64 WorldDatabase::GetSellerSession(int32 character_id) {
	Query query;
	MYSQL_RES* result = query.RunQuery2(
		Q_SELECT,
		"SELECT "
		  "coin_session "
		  "FROM broker_sellers "
		  "WHERE character_id = %d "
		  "LIMIT 1",
		character_id
	);

	if (result) {
		MYSQL_ROW row;
		if ((row = mysql_fetch_row(result))) {
			return strtoull(row[0], NULL, 0);
		}
	}
	return 0;
}

std::string BrokerManager::GetShopPurchaseMessage(const std::string& buyer_name, const std::string& item_desc, int16  quantity, int64 coin) {
	int64 platinum = static_cast<int64>(coin / 1000000);
	// 1) Break totalCopper into denominations
	int64 rem = coin % 1000000;
	int64 gold    = static_cast<int64>(rem / 10000);
	rem        %= 10000;
	int64 silver  = static_cast<int64>(rem / 100);
	int64 copper  = static_cast<int64>(rem % 100);
	
	// 1) Timestamp
	auto now = std::time(nullptr);
	std::tm tm;
	localtime_r(&now, &tm);
	char timebuf[64];
	std::strftime(timebuf, sizeof(timebuf),
				  "%B %d, %Y, %I:%M:%S %p", &tm);

	// 2) Build the sale line
	std::ostringstream msg;
	msg << timebuf
		<< " " << buyer_name << " buys ";
	if (quantity > 1)
		msg << quantity << " ";
	msg << item_desc
		<< " for ";

	// 3) Denominations
	std::vector<std::string> parts;
	if (platinum  > 0) parts.push_back(std::to_string(platinum)  + " Platinum");
	if (gold      > 0) parts.push_back(std::to_string(gold)      + " Gold");
	if (silver    > 0) parts.push_back(std::to_string(silver)    + " Silver");
	if (copper    > 0) parts.push_back(std::to_string(copper)    + " Copper");

	// If all are zero (unlikely), still print "0 Copper"
	if (parts.empty())
		parts.push_back("0 Copper");

	// Join with ", "
	for (size_t i = 0; i < parts.size(); ++i) {
		if (i) msg << ", ";
		msg << parts[i];
	}

	return msg.str();	
}

void BrokerManager::LogSale(int32  cid,
							const std::string& buyer_name,
							const std::string& item_desc,
							int16  quantity,
							int64 coin)
{
	std::string msg = GetShopPurchaseMessage(buyer_name, item_desc, quantity, coin);

	std::string esc = EscapeSQLString(msg.c_str());
	std::ostringstream sql;
	sql << "INSERT INTO broker_seller_log "
		   "(character_id,log_time,message) VALUES ("
		<< cid << ",NOW(),'"
		<< esc << "')";

	std::string full = sql.str();

	Query q;
	q.AddQueryAsync(cid, &database, Q_INSERT, full.c_str());
}

void BrokerManager::LogSaleMessage(int32  cid,
							const std::string& log_message)
{
	auto now = std::time(nullptr);
	std::tm tm;
	localtime_r(&now, &tm);
	char timebuf[64];
	std::strftime(timebuf, sizeof(timebuf),
				  "%B %d, %Y, %I:%M:%S %p", &tm);

	std::ostringstream msg;
	msg << timebuf
		<< " " << log_message;

	std::string esc = EscapeSQLString(msg.str());
	std::ostringstream sql;
	sql << "INSERT INTO broker_seller_log "
		   "(character_id,log_time,message) VALUES ("
		<< cid << ",NOW(),'"
		<< esc << "')";

	std::string full = sql.str();

	Query q;
	q.AddQueryAsync(cid, &database, Q_INSERT, full.c_str());
}

std::vector<SellerLog> BrokerManager::GetSellerLog(int32 cid) const {
	std::vector<SellerLog> out;

	Query query;
	MYSQL_RES* result = query.RunQuery2(
		Q_SELECT,
		"SELECT "
		  "log_id, "
		  "DATE_FORMAT(log_time, '%%M %%d, %%Y, %%r') as ts, "
		  "message "
		"FROM broker_seller_log "
		"WHERE character_id=%u "
		"ORDER BY log_time ASC",
		cid
	);

	if (result) {
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(result))) {
			SellerLog entry;
			entry.log_id     = row[0] ? atoll(row[0]) : 0;
			entry.timestamp  = row[1] ? row[1] : "";
			entry.message    = row[2] ? row[2] : "";
			out.push_back(std::move(entry));
		}
	}

	return out;
}