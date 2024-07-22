#include "Transmute.h"
#include "../common/MiscFunctions.h"
#include "../common/PacketStruct.h"
#include "client.h"
#include "Items/Items.h"
#include <vector>
#include <sstream>
#include "zoneserver.h"
#include "SpellProcess.h"
#include "../common/Log.h"
#include "WorldDatabase.h"

extern ConfigReader configReader;
extern MasterSpellList master_spell_list;

using namespace std;

int32 Transmute::CreateItemRequest(Client* client, Player* player) {
	PacketStruct* p = configReader.getStruct("WS_EqTargetItemCmd", client->GetVersion());
	if (!p) return 0;

	union {
		sint32 signed_request_id;
		int32 request_id;
	};

	do {
		signed_request_id = MakeRandomInt(-2147483648, 2147483647);
	} while (signed_request_id == 0);

	map<int32, Item*>* il = player->GetItemList();

	p->setDataByName("request_id", request_id);
	p->setDataByName("request_type", 1);
	p->setDataByName("unknownff", 0xff);
	
	vector<int32> transmutables;

	for (auto& itr : *il) {
		if (!itr.second) continue;

		if (ItemIsTransmutable(itr.second)) {
			transmutables.push_back(itr.first);
		}
	}

	p->setArrayLengthByName("item_array_size", transmutables.size());

	for (int i = 0; i < transmutables.size(); i++) {
		p->setArrayDataByName("item_id", transmutables[i], i);
	}

	client->QueuePacket(p->serialize());

	delete il;
	delete p;

	client->SetTransmuteID(request_id);

	return request_id;
}

bool Transmute::ItemIsTransmutable(Item* item) {
	//Item level > 0 AND Item is not LORE_EQUP, LORE, NO_VALUE etc AND item rarity is >= 5
		//(4 is treasured but the rarity used for journeyman spells)
	//I think flag 16384 is NO-TRANSMUTE but not positive
	const int32 disqualifyFlags = NO_ZONE | NO_VALUE | TEMPORARY | NO_DESTROY | NO_TRANSMUTE;
	const int32 disqualityFlags2 = ORNATE;
	if (item->generic_info.adventure_default_level > 0
		&& (item->generic_info.item_flags & disqualifyFlags) == 0
		&& (item->generic_info.item_flags2 & disqualityFlags2) == 0
		&& item->details.tier >= 5
		&& item->stack_count <= 1)
	{
		return true;
	}

	return false;
}

void Transmute::HandleItemResponse(Client* client, Player* player, int32 req_id, int32 item_id) {
	Item* item = player->item_list.GetItemFromUniqueID(item_id);
	if (!item) {
		client->SimpleMessage(CHANNEL_COLOR_RED, "Could not find the item you wish to transmute. Please try again.");
		return;
	}

	if (!ItemIsTransmutable(item)) {
		client->Message(CHANNEL_COLOR_RED, "%s is not transmutable.", item->name.c_str());
		return;
	}

	int32 item_level = item->generic_info.adventure_default_level;
	Skill* skill = player->GetSkillByName("Transmuting");

	int32 required_skill = (std::max<int32>(item_level, 5) - 5) * 5;
	sint32 item_stat_bonus = player->GetStat(ITEM_STAT_TRANSMUTING);
	if (!skill || (skill->current_val+item_stat_bonus) < required_skill) {
		client->Message(CHANNEL_COLOR_RED, "You need at least %u Transmuting skill to transmute the %s."
			" You have %u Transmuting skill.", required_skill, item->name.c_str(), skill ? (skill->current_val+item_stat_bonus) : 0);
		return;
	}

	client->SetTransmuteID(item_id);
	SendConfirmRequest(client, req_id, item);
}

void Transmute::SendConfirmRequest(Client* client, int32 req_id, Item* item) {
	PacketStruct* p = configReader.getStruct("WS_ChoiceWindow", client->GetVersion());
	if (!p) {
		client->SimpleMessage(CHANNEL_COLOR_RED, "Struct error for transmutation. Let a dev know.");
		return;
	}

	ostringstream ss;
	ss << "Are you sure you want to transmute the " << item->name << '?';
	p->setMediumStringByName("text", ss.str().c_str());
	p->setMediumStringByName("accept_text", "OK");
	
	ss.str("");
	ss << "targetitem " << req_id << ' ' << item->details.unique_id;
	string cancel_command = ss.str();
	ss << " 1";
	string accept_command = ss.str();

	p->setMediumStringByName("accept_command", accept_command.c_str());
	p->setMediumStringByName("cancel_text", "Cancel");
	p->setMediumStringByName("cancel_command", cancel_command.c_str());

	client->QueuePacket(p->serialize());
	delete p;
}

void Transmute::HandleConfirmResponse(Client* client, Player* player, int32 item_id) {
	Item* item = player->item_list.GetItemFromUniqueID(item_id);
	if (!item) {
		client->SimpleMessage(CHANNEL_COLOR_RED, "Item no longer exists!");
		return;
	}

	client->SetTransmuteID(item_id);

	ZoneServer* zone = player->GetZone();
	if (!zone) return;

	const int32 transmute_item_spell = 5163;

	Spell* spell = master_spell_list.GetSpell(transmute_item_spell, 1);

	if (!spell) {
		LogWrite(SPELL__ERROR, 0, "Transmute", "Could not find the Transmute Item spell : %u", transmute_item_spell);
		return;
	}

	zone->GetSpellProcess()->ProcessSpell(zone, spell, player);
}

void Transmute::CompleteTransmutation(Client* client, Player* player) {
	int32 item_id = client->GetTransmuteID();
	Item* item = player->item_list.GetItemFromUniqueID(item_id);
	if (!item) {
		client->SimpleMessage(CHANNEL_COLOR_RED, "Item no longer exists!");
		return;
	}

	int32 common_mat_id = 0;
	int32 rare_mat_id = 0;
	
	//Figure out the transmutation tier for our loot roll
	int32 item_level = item->generic_info.adventure_default_level;
	vector<TransmutingTier>& tiers = GetTransmutingTiers();
	for (auto& itr : tiers) {
		if (itr.min_level <= item_level && itr.max_level >= item_level) {
			//This is the correct tier
			int32 tier = item->details.tier;

			if (tier >= ITEM_TAG_FABLED) {
				common_mat_id = itr.infusion_id;
				rare_mat_id = itr.mana_id;
			}
			else if (tier >= ITEM_TAG_LEGENDARY) {
				common_mat_id = itr.powder_id;
				rare_mat_id = itr.infusion_id;
			}
			else {
				common_mat_id = itr.fragment_id;
				rare_mat_id = itr.powder_id;
			}

			break;
		}
	}

	if (common_mat_id == 0 || rare_mat_id == 0) {
		client->SimpleMessage(CHANNEL_COLOR_RED, "Could not complete transmutation! Tell a dev!");
		return;
	}

	//Do the loot roll
	const int32 BOTH_ITEMS_CHANCE_PERCENT = 15;
	//The common/rare roll only applies if the both items roll fails
	const int32 COMMON_MAT_CHANCE_PERCENT = 75;
	const int32 RARE_MAT_CHANCE_PERCENT = 25;

	Item* item1 = nullptr;
	Item* item2 = nullptr;

	int32 roll = MakeRandomInt(1, 100);
	if (roll <= BOTH_ITEMS_CHANCE_PERCENT) {
		item1 = master_item_list.GetItem(rare_mat_id);
		if (item1) item1 = new Item(item1);

		item2 = master_item_list.GetItem(common_mat_id);
		if (item2) item2 = new Item(item2);
	}
	else if (roll <= COMMON_MAT_CHANCE_PERCENT) {
		item1 = master_item_list.GetItem(common_mat_id);
		if (item1) item1 = new Item(item1);
	}
	else { //rare mat roll
		item2 = master_item_list.GetItem(rare_mat_id);
		if (item2) item2 = new Item(item2);
	}

	client->Message(89, "You transmute %s and create: ", item->CreateItemLink(client->GetVersion(), false).c_str());

	player->item_list.RemoveItem(item, true);

	PacketStruct* packet = configReader.getStruct("WS_QuestComplete", client->GetVersion());
	if (packet) {
		packet->setDataByName("title", "Item Transmuted!");
	}

	if (item1) {
		item1->details.count = 1;
		client->Message(89, "     %s", item1->CreateItemLink(client->GetVersion(), false).c_str());
		bool itemDeleted = false;
		client->AddItem(item1, &itemDeleted);

		if (packet && !itemDeleted) {
			packet->setArrayDataByName("reward_id", item1->details.item_id, 0);
			if (client->GetVersion() < 860)
				packet->setItemArrayDataByName("item", item1, player, 0, 0, -1);
			else if (client->GetVersion() < 1193)
				packet->setItemArrayDataByName("item", item, player);
			else
				packet->setItemArrayDataByName("item", item1, player, 0, 0, 2);
		}
	}

	if (item2) {
		item2->details.count = 1;
		client->Message(89, "     %s", item2->CreateItemLink(client->GetVersion(), false).c_str());
		bool itemDeleted = false;
		client->AddItem(item2, &itemDeleted);

		if (packet && !itemDeleted) {
			int32 dataIndex = 1;
			if (!item1) {
				packet->setArrayLengthByName("num_rewards", 1);
				dataIndex = 0;
			}
			packet->setArrayDataByName("reward_id", item2->details.item_id, dataIndex);
			if (client->GetVersion() < 860)
				packet->setItemArrayDataByName("item", item2, player, dataIndex, 0, -1);
			else if (client->GetVersion() < 1193)
				packet->setItemArrayDataByName("item", item2, player, dataIndex);
			else
				packet->setItemArrayDataByName("item", item2, player, dataIndex, 0, 2);
		}
	}

	if (packet) {
		client->QueuePacket(packet->serialize());
		delete packet;
	}

	//Check if we need to apply a skill-up
	Skill* skill = player->GetSkillByName("Transmuting");
	if (!skill) {
		//Shouldn't happen, sanity check
		LogWrite(SKILL__ERROR, 0, "Skill", "Unable to find the transmuting skill for the player %s", player->GetName());
		return;
	}

	//Skill up roll
	int32 max_trans_level = skill->current_val / 5 + 5;
	sint32 level_dif = (sint32)max_trans_level - (sint32)item_level;
	if (level_dif > 10 || skill->current_val >= skill->max_val) {
		//No skill up possible
		LogWrite(SKILL__DEBUG, 7, "Skill", "Transmuting skill up not possible.  level_dif = %u, skill val = %u, skill max val = %u", level_dif, skill->current_val, skill->max_val);
		return;
	}
	
	//50% Base chance of a skillup at max item level, 20% overall decrease per level difference
	const int32 SKILLUP_PERCENT_CHANCE_MAX = 50;
	int32 required_roll = SKILLUP_PERCENT_CHANCE_MAX * (1.f - (item_level <= 5 ?  0.f : (level_dif * .2f)));
	roll = MakeRandomInt(1, 100);
	//LogWrite(SKILL__ERROR, 0, "Skill", "Skill up roll results, roll = %u, required_roll = %u", roll, required_roll);
	if (roll <= required_roll) {
		player->skill_list.IncreaseSkill(skill, 1);
	}
}

void WorldDatabase::LoadTransmuting() {
	DatabaseResult result;
	
	if (!database_new.Select(&result,
		"SELECT min_level, max_level, fragment, powder, infusion, mana FROM `transmuting`")) {
		LogWrite(DATABASE__ERROR, 0, "Transmuting", "Error loading transmuting data!");
		return;
	}

	Transmute::ProcessDBResult(result);
}

vector<Transmute::TransmutingTier>& Transmute::GetTransmutingTiers() {
	static vector<TransmutingTier> gTransmutingTiers;
	return gTransmutingTiers;
}

void Transmute::ProcessDBResult(DatabaseResult& result) {
	vector<TransmutingTier>& tiers = GetTransmutingTiers();
	tiers.clear();
	tiers.reserve(result.GetNumRows());

	while (result.Next()) {
		tiers.emplace_back();
		TransmutingTier& t = tiers.back();

		int32_t i = 0;
		t.min_level = result.GetInt32(i++);
		t.max_level = result.GetInt32(i++);
		t.fragment_id = result.GetInt32(i++);
		t.powder_id = result.GetInt32(i++);
		t.infusion_id = result.GetInt32(i++);
		t.mana_id = result.GetInt32(i++);
	}
}