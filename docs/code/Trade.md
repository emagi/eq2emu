# File: `Trade.h`

## Classes

- `Item`
- `Entity`
- `TradeItemInfo`
- `Trade`

## Functions

- `int8 AddItemToTrade(Entity* character, Item* item, int8 quantity, int8 slot);`
- `void RemoveItemFromTrade(Entity* character, int8 slot);`
- `void AddCoinToTrade(Entity* character, int64 amount);`
- `void RemoveCoinFromTrade(Entity* character, int64 amount);`
- `bool SetTradeAccepted(Entity* character);`
- `bool HasAcceptedTrade(Entity* character);`
- `void CancelTrade(Entity* character);`
- `int8 CheckItem(Entity* trader, Item* item, Entity* other);`
- `int8 MaxSlots() { return trade_max_slots; }`
- `void Trader1ItemAdd(Item* item, int8 quantity, int8 slot);`
- `void Trader2ItemAdd(Item* item, int8 quantity, int8 slot);`
- `void CompleteTrade();`
- `void OpenTradeWindow();`
- `void SendTradePacket();`
- `void CalculateCoins(int64 val, int32& plat, int32& gold, int32& silver, int32& copper);`
- `int8 GetNextFreeSlot(Entity* character);`

## Notable Comments

_None detected_
