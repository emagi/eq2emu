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

#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <string>
#include <optional>
#include <shared_mutex>
#include "../../common/types.h"

// Key = (character_id, unique_id)
using Key = std::pair<int32, int64>;

class Item;
class Client;

struct SaleItem {
	int64 unique_id;
	int32 character_id;
	int32 house_id;
	int64 item_id;
	int64 cost_copper;
	bool    for_sale;
	sint32 inv_slot_id;
	int16 slot_id;
	int16 count;
	bool    from_inventory;
	string creator;
};

struct SellerInfo {
	int32     character_id;
	std::string seller_name;
	int64     house_id;
	bool        sale_enabled;
	bool        sell_from_inventory;
	int64		coin_session;
	int64		coin_total;
};

struct SellerLog {
    int64     log_id;      // auto_increment PK
    std::string timestamp;   // e.g. "October 20, 2005, 05:29:33 AM"
    std::string message;     // the human-readable text
};

class BrokerManager {
public:
	BrokerManager();
	
	//–– Player management
	void AddSeller(int32 cid,
				   const std::string& name,
				   int32 house_id,
				   bool sale_enabled,
				   bool sell_from_inventory);

	void LoadSeller(int32 cid,
					const std::string& name,
					int32 house_id,
					bool sale_enabled,
					bool sell_from_inventory,
					int64 coin_session,
					int64 coin_total);
	
	int64 ResetSellerSessionCoins(int32 cid);
	void AddSellerSessionCoins(int32 cid, uint64 session);
	void RemoveSeller(int32 character_id, bool peerCacheOnly = false);

	//–– Item management
	void AddItem(const SaleItem& item, bool peerCacheOnly = false);
	void LoadItem(const SaleItem& item);

	//–– Activate / deactivate sale flag
	void SetSaleStatus(int32 cid, int64 uid, bool for_sale);
	bool IsItemListed(int32 cid, int64 uid);
	void SetSalePrice(int32 cid, int64 uid, int64 price);
	int64 GetSalePrice(int32 cid, int64 uid);
	
	//–– Remove quantity
	void RemoveItem(int32 cid, int64 uid, int16 quantity = 1, bool shouldDelete = false);

	//–– Attempt to buy (atomic DB + in-memory + broadcast)
	bool BuyItem(Client* buyer, int32 seller_cid, int64 uid, int32 quantity);
	bool IsItemForSale(int32 seller_cid, int64 uid) const;
	
	//–– Called when a peer notifies that an item was sold/removed (in-memory only)
	void OnPeerRemoveItem(int32 character_id, int64 unique_id);

	//–– Queries
	std::vector<SaleItem> GetActiveForSaleItems(int32 cid) const;
	std::optional<SaleItem> GetActiveItem(int32 cid, int64 uid) const;
	bool IsSellingItems(int32 cid, bool vaultOnly = false) const;
	std::vector<SaleItem> GetInactiveItems(int32 cid) const;
	//–– Global search API (only active_items_)
	vector<Item*>* GetItems(
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
	) const;

	//–– UI helper: get (unique_id, cost) for active items
	std::vector<std::pair<int64,int32>> GetUniqueIDsAndCost(int32 cid) const;

	std::optional<SellerInfo> GetSellerInfo(int32 character_id) const;
	//–– Lookup seller name
	std::string GetSellerName(int32 cid) const;
	bool IsSaleEnabled(int32 cid) const;
	bool CanSellFromInventory(int32 cid) const;
	int32 GetHouseID(int32 cid) const;

	bool IsItemFromInventory(int32 cid, int64 uid) const;
	void LockActiveItemsForClient(Client* client) const;
	
	std::string GetShopPurchaseMessage(const std::string& buyer_name, const std::string& item_desc, int16  quantity, int64 coin);
	void LogSale(int32 character_id, const std::string& buyer_name,const std::string& item_desc, int16  quantity, int64 coin);
	void LogSaleMessage(int32 character_id,const std::string& log_message);
	std::vector<SellerLog> GetSellerLog(int32 character_id) const;
	static std::string EscapeSQLString(const std::string& s) {
		std::string out;
		out.reserve(s.size() * 2);
		for (char c : s) {
			if (c == '\'') out += "''";
			else            out += c;
		}
		return out;
	}
private:
	mutable std::shared_mutex mtx_;
	std::unordered_map<int32,SellerInfo>            players_;
    std::unordered_map<
        int32,
        std::unordered_map<int64_t, SaleItem>
    > active_items_by_char_;
    std::unordered_map<
        int32,
        std::unordered_map<int64_t, SaleItem>
    > inactive_items_by_char_;

	//–– DB sync (async writes)
	void SavePlayerToDB(const SellerInfo& p);
	void SaveItemToDB(const SaleItem& i);
	void UpdateItemInDB(const SaleItem& i);
	void DeleteItemFromDB(int32 character_id, int64 unique_id);
	void DeleteCharacterItemFromDB(int32 cid, int64 uid);
	void UpdateCharacterItemDB(int32 cid, int64 uid, int16 count);
	void DeletePlayerFromDB(int32 cid);
};