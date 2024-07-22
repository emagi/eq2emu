#include "Bot.h"
#include "BotBrain.h"
#include "../Trade.h"
#include "../../common/Log.h"
#include "../WorldDatabase.h"
#include "../classes.h"
#include "../SpellProcess.h"

extern WorldDatabase database;
extern MasterSpellList master_spell_list;
extern World world;
extern Classes classes;

Bot::Bot() : NPC() {
	SetBrain(new BotBrain(this));
	BotID = 0;
	ShowHelm = true;
	ShowCloak = true;
	CanTaunt = false;
	combat_target = 0;
	main_tank = 0;

	//AddPrimaryEntityCommand("hail", 10000, "hail", "", 0, 0);
	AddSecondaryEntityCommand("invite bot", 10000, "invite", "", 0, 0);
	AddSecondaryEntityCommand("bot inventory", 10000, "bot inv list", "", 0, 0);

	InfoStruct* info = GetInfoStruct();
	info->set_str_base(50);
	info->set_sta_base(20);
	info->set_wis_base(20);
	info->set_intel_base(20);
	info->set_agi_base(20);
	
	camping = false;
	immediate_camp = false;
}

Bot::~Bot() {
}

void Bot::GiveItem(int32 item_id) {
	Item* master_item = master_item_list.GetItem(item_id);
	Item* item = 0;
	if (master_item)
		item = new Item(master_item);
	if (item) {
		int8 slot = GetEquipmentList()->GetFreeSlot(item);
		if (slot != 255) {
			GetEquipmentList()->AddItem(slot, item);
			SetEquipment(item, slot);
			database.SaveBotItem(BotID, item_id, slot);
			if (slot == 0) {
				ChangePrimaryWeapon();
				if (IsBot())
					LogWrite(PLAYER__ERROR, 0, "Bot", "Changing bot primary weapon.");
			}

			CalculateBonuses();
		}
	}
}

void Bot::GiveItem(Item* item) {
	if (item) {
		int8 slot = GetEquipmentList()->GetFreeSlot(item);
		if (slot != 255) {
			GetEquipmentList()->AddItem(slot, item);
			SetEquipment(item, slot);
			database.SaveBotItem(BotID, item->details.item_id, slot);
			if (slot == 0) {
				ChangePrimaryWeapon();
				if (IsBot())
					LogWrite(PLAYER__ERROR, 0, "Bot", "Changing bot primary weapon.");
			}

			CalculateBonuses();
		}
	}
}

void Bot::RemoveItem(Item* item) {
	int8 slot = GetEquipmentList()->GetSlotByItem(item);
	if (slot != 255) {
		GetEquipmentList()->RemoveItem(slot, true);
		SetEquipment(0, slot);
	}
}

void Bot::TradeItemAdded(Item* item) {
	int8 slot = GetEquipmentList()->GetFreeSlot(item);
	if (slot == 255 && item->slot_data.size() > 0) {
		slot = item->slot_data[0];
		AddItemToTrade(slot);
	}
}

void Bot::AddItemToTrade(int8 slot) {
	Item* item = GetEquipmentList()->GetItem(slot);
	if (trading_slots.count(slot) == 0 && item && trade) {
		trade->AddItemToTrade(this, item, 1, 255);
		trading_slots.insert(slot);
	}
}

bool Bot::CheckTradeItems(map<int8, TradeItemInfo>* list) {
	if (!list) {
		LogWrite(PLAYER__ERROR, 0, "Bot", "CheckTradeItems did not recieve a valid list of items");
		return false;
	}

	bool ret = true;
	map<int8, TradeItemInfo>::iterator itr;
	for (itr = list->begin(); itr != list->end(); itr++) {
		Item* item = itr->second.item;
		if (item) {
			if (!CanEquipItem(item)) {
				// No slots means not equipable so reject the trade
				ret = false;
				break;
			}
		}
	}

	return ret;
}

void Bot::FinishTrade() {
	trading_slots.clear();
}

bool Bot::CanEquipItem(Item* item) {
	if (item) {
		if (item->IsArmor() || item->IsWeapon() || item->IsFood() || item->IsRanged() || item->IsShield() || item->IsBauble() || item->IsAmmo() || item->IsThrown()) {
			int16 override_level = item->GetOverrideLevel(GetAdventureClass(), GetTradeskillClass());
			if (override_level > 0 && override_level <= GetLevel()) {
				LogWrite(PLAYER__ERROR, 0, "Bot", "Passed in override_level check");
				return true;
			}
			if (item->CheckClass(GetAdventureClass(), GetTradeskillClass()))
				if (item->CheckLevel(GetAdventureClass(), GetTradeskillClass(), GetLevel())) {
					LogWrite(PLAYER__ERROR, 0, "Bot", "Passed in normal check");
					return true;
				}
		}
	}
	return false;
}

void Bot::MessageGroup(string msg) {
	GroupMemberInfo* gmi = GetGroupMemberInfo();
	if (gmi)
		world.GetGroupManager()->GroupChatMessage(gmi->group_id, this, 0, msg.c_str());
}

void Bot::GetNewSpells() {
	vector<Spell*> spells;
	vector<Spell*>* spells1 = master_spell_list.GetSpellListByAdventureClass(GetAdventureClass(), GetLevel(), 1);
	vector<Spell*>* spells2 = master_spell_list.GetSpellListByAdventureClass(classes.GetBaseClass(GetAdventureClass()), GetLevel(), 1);
	vector<Spell*>* spells3 = master_spell_list.GetSpellListByAdventureClass(classes.GetSecondaryBaseClass(GetAdventureClass()), GetLevel(), 1);

	spells.insert(spells.end(), spells1->begin(), spells1->end());
	spells.insert(spells.end(), spells2->begin(), spells2->end());
	spells.insert(spells.end(), spells3->begin(), spells3->end());

	vector<Spell*>::iterator itr;
	map<int32, int8>* spell_list = 0;
	for (itr = spells.begin(); itr != spells.end(); itr++) {
		switch ((*itr)->GetSpellData()->spell_type) {
		case SPELL_TYPE_DD:
			spell_list = &dd_spells;
			break;
		case SPELL_TYPE_DOT:
			spell_list = &dot_spells;
			break;
		case SPELL_TYPE_HEAL:
			spell_list = &heal_spells;
			break;
		case SPELL_TYPE_HOT_WARD:
			spell_list = &hot_ward_spells;
			break;
		case SPELL_TYPE_DEBUFF:
			spell_list = &debuff_spells;
			break;
		case SPELL_TYPE_BUFF:
			spell_list = &buff_spells;
			break;
		case SPELL_TYPE_COMBATBUFF:
			spell_list = &combat_buff_spells;
			break;
		case SPELL_TYPE_TAUNT:
			spell_list = &taunt_spells;
			break;
		case SPELL_TYPE_DETAUNT:
			spell_list = &detaunt_spells;
			break;
		case SPELL_TYPE_REZ:
			LogWrite(PLAYER__ERROR, 0, "Bot", "Adding rez spell.");
			spell_list = &rez_spells;
			break;
		case SPELL_TYPE_CURE:
			spell_list = &cure_spells;
			break;
		default:
			spell_list = 0;
			break;
		}
		if (spell_list && spell_list->count((*itr)->GetSpellID()) == 0)
			(*spell_list)[(*itr)->GetSpellID()] = 1;
	}

	safe_delete(spells1);
	safe_delete(spells2);
	safe_delete(spells3);
}

Entity* Bot::GetCombatTarget() {
	Spawn* target = GetZone()->GetSpawnByID(combat_target);
	if (target && target->IsEntity())
		return (Entity*)target;

	combat_target = 0;
	return 0;
}

Spell* Bot::SelectSpellToCast(float distance) {
	Spell* spell = 0;
	map<int32, int8>::iterator itr;
	
	// Heal
	spell = GetHealSpell();
	if (spell)
		return spell;

	// Taunt
	spell = GetTauntSpell();
	if (spell)
		return spell;

	// Detaunt 
	spell = GetDetauntSpell();
	if (spell)
		return spell;

	// Hot/Ward
	spell = GetHoTWardSpell();
	if (spell)
		return spell;

	// Debuff
	spell = GetDebuffSpell();
	if (spell)
		return spell;

	// Combat Buff
	spell = GetCombatBuffSpell();
	if (spell)
		return spell;

	// DoT
	spell = GetDoTSpell();
	if (spell)
		return spell;

	// DD
	spell = GetDDSpell();
	if (spell)
		return spell;

	return 0;
}

void Bot::SetRecast(Spell* spell, int32 time) {
	recast_times[spell->GetSpellID()] = time;
}

bool Bot::IsSpellReady(Spell* spell) {
	if (recast_times.count(spell->GetSpellID()) > 0) {
		if (recast_times[spell->GetSpellID()] > Timer::GetCurrentTime2())
			return false;
	}

	return true;
}

Spell* Bot::GetDDSpell() {
	if (dd_spells.size() == 0)
		return 0;

	Spell* spell = 0;
	map<int32, int8>::iterator itr;
	for (itr = dd_spells.begin(); itr != dd_spells.end(); itr++) {
		spell = master_spell_list.GetSpell(itr->first, itr->second);
		if (spell && IsSpellReady(spell)) {
			return spell;
		}
	}

	return 0;
}

Spell* Bot::GetHealSpell() {
	if (heal_spells.size() == 0)
		return 0;
	
	// Get an available heal spell
	Spell* spell = 0;
	map<int32, int8>::iterator itr;
	for (itr = heal_spells.begin(); itr != heal_spells.end(); itr++) {
		spell = master_spell_list.GetSpell(itr->first, itr->second);
		if (spell && IsSpellReady(spell)) {
			break;
		}
	}

	// No heal available, return out
	if (!spell)
		return 0;


	// There was a heal spell so find a group member that needs healing
	int8 threshold = GetHealThreshold();
	GroupMemberInfo* gmi = GetGroupMemberInfo();
	PlayerGroup* group = world.GetGroupManager()->GetGroup(gmi->group_id);
	if (group)
	{
		group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
		deque<GroupMemberInfo*>* members = group->GetMembers();
		for (int8 i = 0; i < members->size(); i++) {
			Entity* member = members->at(i)->member;
			if(!member)
				continue;
			
			if (!member->Alive())
				continue;

			int8 percent = (int8)(((float)member->GetHP() / member->GetTotalHP()) * 100);
			if (percent <= threshold) {
				if (spell) {
					SetTarget(member);
					group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
					return spell;
				}
			}
		}
		group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
	}


	return 0;
}

Spell* Bot::GetTauntSpell() {
	if (taunt_spells.size() == 0)
		return 0;

	// If not the main tank and taunts are turned off return out
	if (main_tank != this && !CanTaunt)
		return 0;

	Spell* spell = 0;
	map<int32, int8>::iterator itr;
	for (itr = taunt_spells.begin(); itr != taunt_spells.end(); itr++) {
		spell = master_spell_list.GetSpell(itr->first, itr->second);
		if (spell && IsSpellReady(spell)) {
			return spell;
		}
	}

	return 0;
}

Spell* Bot::GetDetauntSpell() {
	if (detaunt_spells.size() == 0)
		return 0;

	if (!GetTarget() || !GetTarget()->IsNPC())
		return 0;

	NPC* target = (NPC*)GetTarget();
	Entity* hated = target->Brain()->GetMostHated();
	if (hated == this) {
		Spell* spell = 0;
		map<int32, int8>::iterator itr;
		for (itr = detaunt_spells.begin(); itr != detaunt_spells.end(); itr++) {
			spell = master_spell_list.GetSpell(itr->first, itr->second);
			if (spell && IsSpellReady(spell)) {
				return spell;
			}
		}
	}

	return 0;
}

Spell* Bot::GetHoTWardSpell() {
	if (hot_ward_spells.size() == 0)
		return 0;

	// Get an available spell
	Spell* spell = 0;
	map<int32, int8>::iterator itr;
	for (itr = hot_ward_spells.begin(); itr != hot_ward_spells.end(); itr++) {
		spell = master_spell_list.GetSpell(itr->first, itr->second);
		if (spell && IsSpellReady(spell)) {
			break;
		}
	}

	// No spell available, return out
	if (!spell)
		return 0;

	// There was a spell so find a group member that needs healing
	int8 threshold = GetHealThreshold();
	GroupMemberInfo* gmi = GetGroupMemberInfo();
	PlayerGroup* group = world.GetGroupManager()->GetGroup(gmi->group_id);
	if (group)
	{
		group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
		deque<GroupMemberInfo*>* members = group->GetMembers();

		for (int8 i = 0; i < members->size(); i++) {
			Entity* member = members->at(i)->member;
			
			if(!member)
				continue;
			
			int8 percent = 0;
			if (member->GetHP() > 0)
				percent = (int8)(((float)member->GetHP() / member->GetTotalHP()) * 100);

			if (percent <= 99 && percent > threshold) {
				if (spell) {
					SetTarget(member);
					group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
					return spell;
				}
			}
		}
		group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
	}

	return 0;
}
Spell* Bot::GetDebuffSpell() {
	if (debuff_spells.size() == 0)
		return 0;

	Spell* spell = 0;
	map<int32, int8>::iterator itr;
	for (itr = debuff_spells.begin(); itr != debuff_spells.end(); itr++) {
		spell = master_spell_list.GetSpell(itr->first, itr->second);
		if (spell && IsSpellReady(spell)) {
			// If target already has this effect on them then continue to the next spell
			if (((Entity*)GetTarget())->GetSpellEffect(itr->first))
				continue;

			return spell;
		}
	}

	return 0;
}

Spell* Bot::GetCombatBuffSpell() {
	return 0;
}

Spell* Bot::GetDoTSpell() {
	if (dot_spells.size() == 0)
		return 0;

	Spell* spell = 0;
	map<int32, int8>::iterator itr;
	for (itr = dot_spells.begin(); itr != dot_spells.end(); itr++) {
		spell = master_spell_list.GetSpell(itr->first, itr->second);
		if (spell && IsSpellReady(spell)) {
			// If target already has this effect on them then continue to the next spell
			if (((Entity*)GetTarget())->GetSpellEffect(itr->first))
				continue;

			return spell;
		}
	}

	return 0;
}

Spell* Bot::GetBuffSpell() {
	if (buff_spells.size() == 0)
		return 0;

	Spell* spell = 0;
	Entity* target = 0;
	map<int32, int8>::iterator itr;
	for (itr = buff_spells.begin(); itr != buff_spells.end(); itr++) {
		spell = master_spell_list.GetSpell(itr->first, itr->second);
		if (spell && IsSpellReady(spell)) {
			target = 0;

			if (spell->GetSpellData()->target_type == SPELL_TARGET_SELF)
				target = this;
			if (spell->GetSpellData()->target_type == SPELL_TARGET_GROUP_AE)
				target = this;
			if (spell->GetSpellData()->target_type == SPELL_TARGET_ENEMY && spell->GetSpellData()->friendly_spell == 1)
				target = (main_tank != NULL) ? main_tank : GetOwner();
			if (!target)
				continue;
			if (!target->Alive())
				continue;

			// If target already has this effect on them then continue to the next spell
			if (target->GetSpellEffect(itr->first))
				continue;

			SetTarget(target);
			return spell;
		}
	}

	return 0;
}

Spell* Bot::GetRezSpell() {
	if (rez_spells.size() == 0)
		return 0;

	GroupMemberInfo* gmi = GetGroupMemberInfo();
	if (!gmi)
		return 0;

	Entity* target = 0;
	PlayerGroup* group = world.GetGroupManager()->GetGroup(gmi->group_id);
	if (group)
	{
		group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
		deque<GroupMemberInfo*>* members = group->GetMembers();
		for (int8 i = 0; i < members->size(); i++) {
			Entity* member = members->at(i)->member;
			if (member && !member->Alive() && member->IsPlayer()) {
				PendingResurrection* rez = members->at(i)->client->GetCurrentRez();
				if (rez->active)
					continue;

				target = member;
				break;
			}
		}
		group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
	}

	if (!target)
		return 0;

	Spell* spell = 0;
	map<int32, int8>::iterator itr;
	for (itr = rez_spells.begin(); itr != rez_spells.end(); itr++) {
		spell = master_spell_list.GetSpell(itr->first, itr->second);
		if (spell && IsSpellReady(spell)) {
			SetTarget(target);
			return spell;
		}
	}

	return 0;
}

Spell* Bot::GetCureSpell() {
	return 0;
}

int8 Bot::GetHealThreshold() {
	int8 ret = 0;

	switch (GetAdventureClass()) {
	case PRIEST:
	case CLERIC:
	case TEMPLAR:
	case INQUISITOR:
	case DRUID:
	case WARDEN:
	case FURY:
	case SHAMAN:
	case MYSTIC:
	case DEFILER:
		ret = 70;
		break;
	default:
		ret = 30;
		break;
	}

	return ret;
}

bool Bot::ShouldMelee() {
	bool ret = true;

	switch (GetAdventureClass()) {
	case PRIEST:
	case CLERIC:
	case TEMPLAR:
	case INQUISITOR:
	case DRUID:
	case WARDEN:
	case FURY:
	case SHAMAN:
	case MYSTIC:
	case DEFILER:
	case MAGE:
	case SORCERER:
	case WIZARD:
	case WARLOCK:
	case ENCHANTER:
	case ILLUSIONIST:
	case COERCER:
	case SUMMONER:
	case CONJUROR:
	case NECROMANCER:
		ret = false;
		break;
	default:
		ret = true;
		break;
	}

	if (GetTarget() == GetOwner())
		ret = false;

	return ret;
}

void Bot::Camp(bool immediate) {
	// Copy from COMMAND_GROUP_LEAVE
	camping = true;
	immediate_camp = immediate;
}

void Bot::ChangeLevel(int16 old_level, int16 new_level) {
	if (new_level < 1)
		return;

	if (GetLevel() != new_level) {
		SetLevel(new_level);
		if (GetGroupMemberInfo()) {
			UpdateGroupMemberInfo();
			world.GetGroupManager()->SendGroupUpdate(GetGroupMemberInfo()->group_id);
		}
	}

	if (GetPet()) {
		NPC* pet = (NPC*)GetPet();
		if (pet->GetMaxPetLevel() == 0 || new_level <= pet->GetMaxPetLevel()) {
			pet->SetLevel(new_level);
			GetZone()->PlayAnimation(pet, 1753);
		}
	}

	// level up animation
	GetZone()->PlayAnimation(this, 1753);

	//player->GetSkills()->IncreaseAllSkillCaps(5 * (new_level - old_level));
	GetNewSpells();
	//SendNewSpells(player->GetAdventureClass());
	//SendNewSpells(classes.GetBaseClass(player->GetAdventureClass()));
	//SendNewSpells(classes.GetSecondaryBaseClass(player->GetAdventureClass()));

	GetInfoStruct()->set_level(new_level);
	UpdateWeapons();
	// GetPlayer()->SetLevel(new_level);

	LogWrite(MISC__TODO, 1, "TODO", "Get new HP/POWER/stat based on default values from DB\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);

	SetTotalHPBase(new_level*new_level * 2 + 40);
	SetTotalPowerBase((sint32)(new_level*new_level*2.1 + 45));
	CalculateBonuses();
	SetHP(GetTotalHP());
	SetPower(GetTotalPower());

	GetInfoStruct()->set_agi_base(new_level * 2 + 15);
	GetInfoStruct()->set_intel_base(new_level * 2 + 15);
	GetInfoStruct()->set_wis_base(new_level * 2 + 15);
	GetInfoStruct()->set_str_base(new_level * 2 + 15);
	GetInfoStruct()->set_sta_base(new_level * 2 + 15);
	GetInfoStruct()->set_cold_base((int16)(new_level*1.5 + 10));
	GetInfoStruct()->set_heat_base((int16)(new_level*1.5 + 10));
	GetInfoStruct()->set_disease_base((int16)(new_level*1.5 + 10));
	GetInfoStruct()->set_mental_base((int16)(new_level*1.5 + 10));
	GetInfoStruct()->set_magic_base((int16)(new_level*1.5 + 10));
	GetInfoStruct()->set_divine_base((int16)(new_level*1.5 + 10));
	GetInfoStruct()->set_poison_base((int16)(new_level*1.5 + 10));
	/*UpdateTimeStampFlag(LEVEL_UPDATE_FLAG);
	GetPlayer()->SetCharSheetChanged(true);

	Message(CHANNEL_COLOR_EXP, "You are now level %i!", new_level);
	LogWrite(WORLD__DEBUG, 0, "World", "Player: %s leveled from %u to %u", GetPlayer()->GetName(), old_level, new_level);
	GetPlayer()->GetSkills()->SetSkillCapsByType(1, 5 * new_level);
	GetPlayer()->GetSkills()->SetSkillCapsByType(3, 5 * new_level);
	GetPlayer()->GetSkills()->SetSkillCapsByType(6, 5 * new_level);
	GetPlayer()->GetSkills()->SetSkillCapsByType(13, 5 * new_level);
	*/
}

void Bot::Begin_Camp() {
	GroupMemberInfo* gmi = GetGroupMemberInfo();
	if (gmi) {
		int32 group_id = gmi->group_id;
		world.GetGroupManager()->RemoveGroupMember(group_id, this);
		if (!world.GetGroupManager()->IsGroupIDValid(group_id)) {
			// leader->Message(CHANNEL_COLOR_GROUP, "%s has left the group.", client->GetPlayer()->GetName());
		}
		else {
			world.GetGroupManager()->GroupMessage(group_id, "%s has left the group.", GetName());
		}
	}

	if(!immediate_camp)
	{
		GetZone()->PlayAnimation(this, 538);
		SetVisualState(540);
		GetZone()->Despawn(this, 5000);
	}

	if (!GetOwner())
		return;

	if (GetOwner()->IsPlayer())
		((Player*)GetOwner())->SpawnedBots.erase(BotIndex);
	
	camping = false;
	immediate_camp = true;
}