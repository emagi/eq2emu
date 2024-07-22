#pragma once

#include "../common/types.h"
#include <map>

class Item;
class Entity;

struct TradeItemInfo {
	Item* item;
	int32 quantity;
};

class Trade {
public:
	Trade(Entity* trader1, Entity* trader2);
	~Trade();

	int8 AddItemToTrade(Entity* character, Item* item, int8 quantity, int8 slot);
	void RemoveItemFromTrade(Entity* character, int8 slot);
	void AddCoinToTrade(Entity* character, int64 amount);
	void RemoveCoinFromTrade(Entity* character, int64 amount);
	Entity* GetTradee(Entity* character);

	bool SetTradeAccepted(Entity* character);
	bool HasAcceptedTrade(Entity* character);
	void CancelTrade(Entity* character);

	int8 CheckItem(Entity* trader, Item* item, Entity* other);

	Item* GetTraderSlot(Entity* trader, int8 slot);
	Entity* GetTrader1() { return trader1; }
	Entity* GetTrader2() { return trader2; }
	
	int8 MaxSlots() { return trade_max_slots; }
private:


	void Trader1ItemAdd(Item* item, int8 quantity, int8 slot);
	void Trader2ItemAdd(Item* item, int8 quantity, int8 slot);
	void CompleteTrade();
	void OpenTradeWindow();
	void SendTradePacket();
	void CalculateCoins(int64 val, int32& plat, int32& gold, int32& silver, int32& copper);
	int8 GetNextFreeSlot(Entity* character);

	Entity* trader1;
	map<int8, TradeItemInfo> trader1_items;
	int64 trader1_coins;
	bool trader1_accepted;

	Entity* trader2;
	map<int8, TradeItemInfo> trader2_items;
	int64 trader2_coins;
	bool trader2_accepted;
	int32 trade_max_slots;
};