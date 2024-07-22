#pragma once

#include "../NPC.h"
#include <set>

struct TradeItemInfo;

class Bot : public NPC {
public:
	Bot();
	~Bot();

	int32 BotID;	// DB id
	int32 BotIndex;	// Bot id with its owner (player)
	bool IsBot() { return true; }

	void GiveItem(int32 item_id);
	void GiveItem(Item* item);
	void RemoveItem(Item* item);
	void TradeItemAdded(Item* item);
	void AddItemToTrade(int8 slot);
	bool CheckTradeItems(map<int8, TradeItemInfo>* list);
	void FinishTrade();
	void GetNewSpells();
	map<int32, int8>* GetBotSpells() { return &dd_spells; }

	bool ShowHelm;
	bool ShowCloak;
	bool CanTaunt;

	Entity* GetCombatTarget();
	void SetCombatTarget(int32 target) { combat_target = target; }

	Spell* SelectSpellToCast(float distance);

	void MessageGroup(string msg);

	void SetRecast(Spell* spell, int32 time);
	bool ShouldMelee();

	Spell* GetNextBuffSpell(Spawn* target = 0) { return GetBuffSpell(); }
	Spell* GetHealSpell();
	Spell* GetRezSpell();

	void SetMainTank(Entity* tank) { main_tank = tank; }

	void Camp(bool immediate=false);
	void ChangeLevel(int16 old_level, int16 new_level);

	bool IsCamping() { return camping; }
	bool IsImmediateCamp() { return immediate_camp; }
	void Begin_Camp();
private:
	bool CanEquipItem(Item* item);
	bool IsSpellReady(Spell* spell);

	
	Spell* GetTauntSpell();
	Spell* GetDetauntSpell();
	Spell* GetHoTWardSpell();
	Spell* GetDebuffSpell();
	Spell* GetCombatBuffSpell();
	Spell* GetDoTSpell();
	Spell* GetDDSpell();

	Spell* GetBuffSpell();
	Spell* GetCureSpell();
	

	int8 GetHealThreshold();

	set<int8> trading_slots;
	int32 combat_target;

	Entity* main_tank;

	map<int32, int8> bot_spells;
	map<int32, int8> dd_spells;
	map<int32, int8> dot_spells;
	map<int32, int8> heal_spells;
	map<int32, int8> hot_ward_spells;
	map<int32, int8> debuff_spells;
	map<int32, int8> buff_spells;
	map<int32, int8> combat_buff_spells;
	map<int32, int8> taunt_spells;
	map<int32, int8> detaunt_spells;
	map<int32, int8> rez_spells;
	map<int32, int8> cure_spells;

	// First int32 = spell id (change to timer id later), second int32 is time the spell is available to cast again
	map<int32, int32> recast_times;
	std::atomic<bool> camping;
	std::atomic<bool> immediate_camp;
	
};
