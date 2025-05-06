# File: `Bot.h`

## Classes

- `TradeItemInfo`
- `Bot`

## Functions

- `bool IsBot() { return true; }`
- `void GiveItem(int32 item_id);`
- `void GiveItem(Item* item);`
- `void RemoveItem(Item* item);`
- `void TradeItemAdded(Item* item);`
- `void AddItemToTrade(int8 slot);`
- `bool CheckTradeItems(map<int8, TradeItemInfo>* list);`
- `void FinishTrade();`
- `void GetNewSpells();`
- `void SetCombatTarget(int32 target) { combat_target = target; }`
- `void MessageGroup(string msg);`
- `void SetRecast(Spell* spell, int32 time);`
- `bool ShouldMelee();`
- `void SetMainTank(Entity* tank) { main_tank = tank; }`
- `void Camp(bool immediate=false);`
- `void ChangeLevel(int16 old_level, int16 new_level);`
- `bool IsCamping() { return camping; }`
- `bool IsImmediateCamp() { return immediate_camp; }`
- `void Begin_Camp();`
- `bool CanEquipItem(Item* item);`
- `bool IsSpellReady(Spell* spell);`
- `int8 GetHealThreshold();`

## Notable Comments

- // First int32 = spell id (change to timer id later), second int32 is time the spell is available to cast again
