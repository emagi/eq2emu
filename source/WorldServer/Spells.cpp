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
#include "Spells.h"
#include "../common/ConfigReader.h"
#include "WorldDatabase.h"
#include "../common/Log.h"
#include "Traits/Traits.h"
#include "AltAdvancement/AltAdvancement.h"
#include <cmath>
#include "LuaInterface.h"
#include "Rules/Rules.h"

#include <boost/regex.hpp>

extern ConfigReader configReader;
extern WorldDatabase database;
extern MasterTraitList master_trait_list;
extern MasterAAList master_aa_list;
extern MasterSpellList master_spell_list;
extern LuaInterface* lua_interface;
extern MasterSkillList master_skill_list;
extern RuleManager rule_manager;

Spell::Spell(){
	spell = new SpellData;
	heal_spell = false;
	buff_spell = false;
	damage_spell = false;
	control_spell = false;
	offense_spell = false;
	copied_spell = false;
}

Spell::Spell(Spell* host_spell, bool unique_spell)
{
	std::shared_lock lock(host_spell->MSpellInfo);
	copied_spell = true;

	spell = new SpellData;

	if (host_spell->GetSpellData())
	{
		if(!unique_spell) {
			spell->id = host_spell->GetSpellData()->id;
		}
		else {
			// try inheriting an existing custom spell id, otherwise obtain the new highest number on the spell list
			int32 tmpid = lua_interface->GetFreeCustomSpellID();
			if (tmpid)
				spell->id = tmpid;
			else
			{
				spell->id = master_spell_list.GetNewMaxSpellID();
			}
		}
		
		spell->inherited_spell_id = host_spell->GetSpellData()->inherited_spell_id;

		spell->affect_only_group_members = host_spell->GetSpellData()->affect_only_group_members;
		spell->call_frequency = host_spell->GetSpellData()->call_frequency;
		spell->can_effect_raid = host_spell->GetSpellData()->can_effect_raid;
		spell->casting_flags = host_spell->GetSpellData()->casting_flags;
		spell->cast_time = host_spell->GetSpellData()->cast_time;
		spell->orig_cast_time = host_spell->GetSpellData()->orig_cast_time;
		spell->cast_type = host_spell->GetSpellData()->cast_type;
		spell->cast_while_moving = host_spell->GetSpellData()->cast_while_moving;
		spell->class_skill = host_spell->GetSpellData()->class_skill;
		spell->min_class_skill_req = host_spell->GetSpellData()->min_class_skill_req;
		spell->control_effect_type = host_spell->GetSpellData()->control_effect_type;
		spell->description = EQ2_16BitString(host_spell->GetSpellData()->description);
		spell->det_type = host_spell->GetSpellData()->det_type;
		spell->display_spell_tier = host_spell->GetSpellData()->display_spell_tier;

		spell->dissonance_req = host_spell->GetSpellData()->dissonance_req;
		spell->dissonance_req_percent = host_spell->GetSpellData()->dissonance_req_percent;
		spell->dissonance_upkeep = host_spell->GetSpellData()->dissonance_upkeep;
		spell->duration1 = host_spell->GetSpellData()->duration1;
		spell->duration2 = host_spell->GetSpellData()->duration2;
		spell->duration_until_cancel = host_spell->GetSpellData()->duration_until_cancel;
		spell->effect_message = string(host_spell->GetSpellData()->effect_message);
		spell->fade_message = string(host_spell->GetSpellData()->fade_message);
		spell->fade_message_others = string(host_spell->GetSpellData()->fade_message_others);

		spell->friendly_spell = host_spell->GetSpellData()->friendly_spell;
		spell->group_spell = host_spell->GetSpellData()->group_spell;

		spell->hit_bonus = host_spell->GetSpellData()->hit_bonus;

		spell->hp_req = host_spell->GetSpellData()->hp_req;
		spell->hp_req_percent = host_spell->GetSpellData()->hp_req_percent;
		spell->hp_upkeep = host_spell->GetSpellData()->hp_upkeep;

		spell->icon = host_spell->GetSpellData()->icon;
		spell->icon_backdrop = host_spell->GetSpellData()->icon_backdrop;

		spell->icon_heroic_op = host_spell->GetSpellData()->icon_heroic_op;

		spell->incurable = host_spell->GetSpellData()->incurable;
		spell->interruptable = host_spell->GetSpellData()->interruptable;
		spell->is_aa = host_spell->GetSpellData()->is_aa;

		spell->is_active = host_spell->GetSpellData()->is_active;
		spell->linked_timer = host_spell->GetSpellData()->linked_timer;
		spell->lua_script = string(host_spell->GetSpellData()->lua_script);

		spell->mastery_skill = host_spell->GetSpellData()->mastery_skill;
		spell->max_aoe_targets = host_spell->GetSpellData()->max_aoe_targets;

		spell->min_range = host_spell->GetSpellData()->min_range;
		spell->name = EQ2_8BitString(host_spell->GetSpellData()->name);
		spell->not_maintained = host_spell->GetSpellData()->not_maintained;
		spell->num_levels = host_spell->GetSpellData()->num_levels;
		spell->persist_through_death = host_spell->GetSpellData()->persist_through_death;
		spell->power_by_level = host_spell->GetSpellData()->power_by_level;
		spell->power_req = host_spell->GetSpellData()->power_req;
		spell->power_req_percent = host_spell->GetSpellData()->power_req_percent;
		spell->power_upkeep = host_spell->GetSpellData()->power_upkeep;
		spell->radius = host_spell->GetSpellData()->radius;
		spell->range = host_spell->GetSpellData()->range;
		spell->recast = host_spell->GetSpellData()->recast;
		spell->recovery = host_spell->GetSpellData()->recovery;
		spell->req_concentration = host_spell->GetSpellData()->req_concentration;
		spell->resistibility = host_spell->GetSpellData()->resistibility;
		spell->savagery_req = host_spell->GetSpellData()->savagery_req;
		spell->savagery_req_percent = host_spell->GetSpellData()->savagery_req_percent;
		spell->savagery_upkeep = host_spell->GetSpellData()->savagery_upkeep;
		spell->savage_bar = host_spell->GetSpellData()->savage_bar;
		spell->savage_bar_slot = host_spell->GetSpellData()->savage_bar_slot;
		spell->soe_spell_crc = host_spell->GetSpellData()->soe_spell_crc;
		spell->spell_book_type = host_spell->GetSpellData()->spell_book_type;
		spell->spell_name_crc = host_spell->GetSpellData()->spell_name_crc;
		spell->spell_type = host_spell->GetSpellData()->spell_type;
		spell->spell_visual = host_spell->GetSpellData()->spell_visual;
		spell->success_message = string(host_spell->GetSpellData()->success_message);
		spell->target_type = host_spell->GetSpellData()->target_type;
		spell->tier = host_spell->GetSpellData()->tier;
		spell->ts_loc_index = host_spell->GetSpellData()->ts_loc_index;
		spell->type = host_spell->GetSpellData()->type;
		spell->type_group_spell_id = host_spell->GetSpellData()->type_group_spell_id;
		spell->can_fizzle = host_spell->GetSpellData()->can_fizzle;
	}

	heal_spell = host_spell->IsHealSpell();
	buff_spell = host_spell->IsBuffSpell();
	damage_spell = host_spell->IsDamageSpell();;
	control_spell = host_spell->IsControlSpell();
	offense_spell = host_spell->IsOffenseSpell();

	std::vector<LevelArray*>::iterator itr;
	for (itr = host_spell->levels.begin(); itr != host_spell->levels.end(); itr++)
	{
		LevelArray* lvlArray = *itr;
		AddSpellLevel(lvlArray->adventure_class, lvlArray->tradeskill_class, lvlArray->spell_level, lvlArray->classic_spell_level);
	}

	std::vector<SpellDisplayEffect*>::iterator sdeitr;
	for (sdeitr = host_spell->effects.begin(); sdeitr != host_spell->effects.end(); sdeitr++)
	{
		SpellDisplayEffect* sde = *sdeitr;
		AddSpellEffect(sde->percentage, sde->subbullet, sde->description);
	}

	vector<LUAData*>::iterator luaitr;
	for (luaitr = host_spell->lua_data.begin(); luaitr != host_spell->lua_data.end(); luaitr++) {
		LUAData* data = *luaitr;
		AddSpellLuaData(data->type, data->int_value, data->int_value2, data->float_value, data->float_value2, data->bool_value, string(data->string_value), string(data->string_value2), string(data->string_helper));
	}
}

Spell::Spell(SpellData* in_spell){
	spell = in_spell;
	heal_spell = false;
	buff_spell = false;
	damage_spell = false;
	control_spell = false;
	offense_spell = false;
	copied_spell = false;
}

Spell::~Spell(){
	vector<LevelArray*>::iterator itr1;
	for(itr1=levels.begin();itr1!=levels.end();itr1++) {
		safe_delete(*itr1);
	}
	vector<SpellDisplayEffect*>::iterator itr2;
	for(itr2=effects.begin();itr2!=effects.end();itr2++) {
		safe_delete(*itr2);
	}
	vector<LUAData*>::iterator itr3;
	for(itr3=lua_data.begin();itr3!=lua_data.end();itr3++) {
		safe_delete(*itr3);
	}
	safe_delete(spell);
}

void Spell::AddSpellLuaData(int8 type, int int_value, int int_value2, float float_value, float float_value2, bool bool_value, string string_value, string string_value2, string helper){
    std::unique_lock lock(MSpellInfo);
	LUAData* data = new LUAData;
	data->type = type;
	data->int_value = int_value;
	data->int_value2 = int_value2;
	data->float_value = float_value;
	data->float_value2 = float_value2;
	data->bool_value = bool_value;
	data->string_value = string_value;
	data->string_value2 = string_value2;
	data->string_helper = helper;
	data->needs_db_save = false;
	lua_data.push_back(data);
}

void Spell::AddSpellLuaDataInt(int value, int value2, string helper) {
	std::unique_lock lock(MSpellInfo);
	LUAData *data = new LUAData;

	data->type = 0;
	data->int_value = value;
	data->int_value2 = value2;
	data->float_value = 0;
	data->float_value2 = 0;
	data->bool_value = false;
	data->string_helper = helper;
	data->needs_db_save = false;

	lua_data.push_back(data);
}

void Spell::AddSpellLuaDataFloat(float value, float value2, string helper) {
	std::unique_lock lock(MSpellInfo);
	LUAData *data = new LUAData;

	data->type = 1;
	data->int_value = 0;
	data->int_value2 = 0;
	data->float_value = value;
	data->float_value2 = value2;
	data->bool_value = false;
	data->string_helper = helper;
	data->needs_db_save = false;

	lua_data.push_back(data);
}

void Spell::AddSpellLuaDataBool(bool value, string helper) {
	std::unique_lock lock(MSpellInfo);
	LUAData *data = new LUAData;

	data->type = 2;
	data->int_value = 0;
	data->float_value = 0;
	data->bool_value = value;
	data->string_helper = helper;
	data->needs_db_save = false;

	lua_data.push_back(data);
}

void Spell::AddSpellLuaDataString(string value, string value2,string helper) {
	std::unique_lock lock(MSpellInfo);
	LUAData *data = new LUAData;

	data->type = 3;
	data->int_value = 0;
	data->int_value2 = 0;
	data->float_value = 0;
	data->float_value2 = 0;
	data->bool_value = false;
	data->string_value = value;
	data->string_value2 = value2;
	data->string_helper = helper;
	data->needs_db_save = false;

	lua_data.push_back(data);
}

int16 Spell::GetLevelRequired(Player* player){
	int16 ret = 0xFFFF;
	if(!player)
		return ret;
	LevelArray* level = 0;
	vector<LevelArray*>::iterator itr;
	for(itr = levels.begin(); itr != levels.end(); itr++){
		level = *itr;
		if(level && level->adventure_class == player->GetAdventureClass()){
			if(rule_manager.GetGlobalRule(R_Spells, UseClassicSpellLevel)->GetInt8() && level->classic_spell_level > 0.0f)
				ret = std::floor(level->classic_spell_level);
			else
				ret = level->spell_level/10;
			break;
		}
	}
	return ret;
}

void Spell::SetAAPacketInformation(PacketStruct* packet, AltAdvanceData* data, Client* client, bool display_tier) {
	int8 current_tier = client->GetPlayer()->GetSpellTier(spell->id);
	Spell* next_spell;
	SpellData* spell2;
	if (data->maxRank > current_tier) {
		next_spell = master_spell_list.GetSpell(spell->id, current_tier + 1);
		spell2 = next_spell->GetSpellData();
	}
	SpellDisplayEffect* effect2;

	//next_spell->effects[1]->description;


	int xxx = 0;

	int16 hp_req = 0;
	int16 power_req = 0;

	if (current_tier > 0) {
		packet->setSubstructDataByName("spell_info", "current_id", spell->id);
		packet->setSubstructDataByName("spell_info", "current_icon", spell->icon);
		packet->setSubstructDataByName("spell_info", "current_icon2", spell->icon_heroic_op);	// fix struct element name eventually
		packet->setSubstructDataByName("spell_info", "current_icontype", spell->icon_backdrop);	// fix struct element name eventually

		if (packet->GetVersion() >= 63119) {
			packet->setSubstructDataByName("spell_info", "current_version", 0x04);
			packet->setSubstructDataByName("spell_info", "current_sub_version", 0x24);
		}
		else if (packet->GetVersion() >= 58617) {
			packet->setSubstructDataByName("spell_info", "current_version", 0x03);
			packet->setSubstructDataByName("spell_info", "current_sub_version", 0x131A);
		}
		else {
			packet->setSubstructDataByName("spell_info", "current_version", 0x00);
			packet->setSubstructDataByName("spell_info", "current_sub_version", 0xD9);
		}

		packet->setSubstructDataByName("spell_info", "current_type", spell->type);
		packet->setSubstructDataByName("spell_info", "unknown_MJ1d", 1); //63119 test
		packet->setSubstructDataByName("spell_info", "current_class_skill", spell->class_skill);
		packet->setSubstructDataByName("spell_info", "current_mastery_skill", spell->mastery_skill);
		packet->setSubstructDataByName("spell_info", "duration_flag", spell->duration_until_cancel);


		if (client && spell->type != 2) {
			sint8 spell_text_color = client->GetPlayer()->GetArrowColor(GetLevelRequired(client->GetPlayer()));
			if (spell_text_color != ARROW_COLOR_WHITE && spell_text_color != ARROW_COLOR_RED && spell_text_color != ARROW_COLOR_GRAY)
				spell_text_color = ARROW_COLOR_WHITE;
			spell_text_color -= 6;
			if (spell_text_color < 0)
				spell_text_color *= -1;
			packet->setSubstructDataByName("spell_info", "current_spell_text_color", (xxx == 1 ? 0xFFFFFFFF : spell_text_color));
		}
		else {
			packet->setSubstructDataByName("spell_info", "current_spell_text_color", (xxx == 1 ? 0xFFFFFFFF : 3));
		}
		packet->setSubstructDataByName("spell_info", "current_spell_text_color", (xxx == 1 ? 0xFFFFFFFF : 3));
		packet->setSubstructDataByName("spell_info", "current_tier", (spell->tier));

		if (spell->type != 2) {
			packet->setArrayLengthByName("current_num_levels", 0);
			for (int32 i = 0; i < levels.size(); i++) {
				// revisit when implementing AA and use AppendLevelInformation instead (this struct doesn't even exist yet for KoS)
				packet->setArrayDataByName("spell_info_aa_adventure_class", levels[i]->adventure_class, i);
				packet->setArrayDataByName("spell_info_aa_tradeskill_class", levels[i]->tradeskill_class, i);
				packet->setArrayDataByName("spell_info_aa_spell_level", levels[i]->spell_level, i);
			}
		}
		//packet->setSubstructDataByName("spell_info","unknown9", 20);

		if (client) {
			hp_req = GetHPRequired(client->GetPlayer());
			power_req = GetPowerRequired(client->GetPlayer());

			// might need version checks around these?
			if (client->GetVersion() >= 1193)
			{
				int16 savagery_req = GetSavageryRequired(client->GetPlayer()); // dunno why we need to do this
				packet->setSubstructDataByName("spell_info", "current_savagery_req", savagery_req);
				packet->setSubstructDataByName("spell_info", "current_savagery_upkeep", spell->savagery_upkeep);
			}
			if (client->GetVersion() >= 57048)
			{
				int16 dissonance_req = GetDissonanceRequired(client->GetPlayer()); // dunno why we need to do this
				packet->setSubstructDataByName("spell_info", "dissonance_req", dissonance_req);
				packet->setSubstructDataByName("spell_info", "dissonance_upkeep", spell->dissonance_upkeep);
			}
		}
		packet->setSubstructDataByName("spell_info", "current_health_req", hp_req);
		packet->setSubstructDataByName("spell_info", "current_health_upkeep", spell->hp_upkeep);
		packet->setSubstructDataByName("spell_info", "current_power_req", power_req);
		packet->setSubstructDataByName("spell_info", "current_power_upkeep", spell->power_upkeep);
		packet->setSubstructDataByName("spell_info", "current_req_concentration", spell->req_concentration);
		//unknown1 savagery???
		packet->setSubstructDataByName("spell_info", "current_cast_time", spell->cast_time);
		packet->setSubstructDataByName("spell_info", "current_recovery", spell->recovery);
		packet->setSubstructDataByName("spell_info", "current_recast", spell->recast);
		packet->setSubstructDataByName("spell_info", "current_radius", spell->radius);
		packet->setSubstructDataByName("spell_info", "current_max_aoe_targets", spell->max_aoe_targets);
		packet->setSubstructDataByName("spell_info", "current_friendly_spell", spell->friendly_spell);
		// rumber of reagents with array









		packet->setSubstructArrayLengthByName("spell_info", "current_num_effects", (xxx == 1 ? 0 : effects.size()));
		for (int32 i = 0; i < effects.size(); i++) {
			packet->setArrayDataByName("current_subbulletflag", effects[i]->subbullet, i);
			string effect_message;
			if (effects[i]->description.length() > 0) {
				effect_message = effects[i]->description;
				if (effect_message.find("%LM") < 0xFFFFFFFF) {
					int string_index = effect_message.find("%LM");
					int data_index = stoi(effect_message.substr(string_index + 3, 2));
					float value;
					if (lua_data[data_index]->type == 1)
						value = lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
					else
						value = lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
					string strValue = to_string(value);
					strValue.erase(strValue.find_last_not_of('0') + 1, std::string::npos);
					effect_message.replace(effect_message.find("%LM"), 5, strValue);
				}
				// Magic damage min
				if (effect_message.find("%DML") < 0xFFFFFFFF) {
					int string_index = effect_message.find("%DML");
					int data_index = stoi(effect_message.substr(string_index + 4, 2));
					float value;
					if (lua_data[data_index]->type == 1)
						value = lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
					else
						value = lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();

					value = client->GetPlayer()->CalculateDamageAmount(nullptr, value, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
					string damage = to_string((int)round(value));
					effect_message.replace(effect_message.find("%DML"), 6, damage);
				}
				// Magic damage max
				if (effect_message.find("%DMH") < 0xFFFFFFFF) {
					int string_index = effect_message.find("%DMH");
					int data_index = stoi(effect_message.substr(string_index + 4, 2));
					float value;
					if (lua_data[data_index]->type == 1)
						value = lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
					else
						value = lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
						
					value = client->GetPlayer()->CalculateDamageAmount(nullptr, value, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
					string damage = to_string((int)round(value));
					effect_message.replace(effect_message.find("%DMH"), 6, damage);
				}
				// level based Magic damage min
				if (effect_message.find("%LDML") < 0xFFFFFFFF) {
					int string_index = effect_message.find("%LDML");
					int data_index = stoi(effect_message.substr(string_index + 5, 2));
					float value;
					if (lua_data[data_index]->type == 1)
						value = lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
					else
						value = lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
						
					value = client->GetPlayer()->CalculateDamageAmount(nullptr, value, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
					string damage = to_string((int)round(value));
					effect_message.replace(effect_message.find("%LDML"), 7, damage);
				}
				// level based Magic damage max
				if (effect_message.find("%LDMH") < 0xFFFFFFFF) {
					int string_index = effect_message.find("%LDMH");
					int data_index = stoi(effect_message.substr(string_index + 5, 2));
					float value;
					if (lua_data[data_index]->type == 1)
						value = lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
					else
						value = lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
						
					value = client->GetPlayer()->CalculateDamageAmount(nullptr, value, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
					string damage = to_string((int)round(value));
					effect_message.replace(effect_message.find("%LDMH"), 7, damage);
				}
				//GetZone()->SimpleMessage(CHANNEL_COLOR_SPELL_EFFECT, effect_message.c_str(), victim, 50);
				packet->setArrayDataByName("current_effect", effect_message.c_str(), i);
			}
			packet->setArrayDataByName("current_percentage", effects[i]->percentage, i);
		}
		if (display_tier == true)
			packet->setSubstructDataByName("spell_info", "current_display_spell_tier", 1);// spell2->display_spell_tier);
		else
			packet->setSubstructDataByName("spell_info", "current_display_spell_tier", 1);// 0);

		packet->setSubstructDataByName("spell_info", "current_unknown_1", 1);// 0);
		//unkown1_1
		packet->setSubstructDataByName("spell_info", "current_minimum_range", spell->min_range);
		packet->setSubstructDataByName("spell_info", "current_range", spell->range);
		packet->setSubstructDataByName("spell_info", "current_duration_1", spell->duration1);
		packet->setSubstructDataByName("spell_info", "current_duration_2", spell->duration2);

		packet->setSubstructDataByName("spell_info", "current_duration_flag", spell->duration_until_cancel);
		packet->setSubstructDataByName("spell_info", "current_target", spell->target_type);





		packet->setSubstructDataByName("spell_info", "current_can_effect_raid", spell->can_effect_raid);
		packet->setSubstructDataByName("spell_info", "current_affect_only_group_members", spell->affect_only_group_members);
		packet->setSubstructDataByName("spell_info", "current_group_spell", spell->group_spell);
		packet->setSubstructDataByName("spell_info", "current_resistibility", spell->resistibility);
		packet->setSubstructDataByName("spell_info", "current_name", &(spell->name));
		packet->setSubstructDataByName("spell_info", "current_description", &(spell->description));

	}

	if (current_tier + 1 <= data->maxRank) {
		packet->setSubstructDataByName("spell_info", "next_id", spell2->id);
		packet->setSubstructDataByName("spell_info", "next_icon", spell2->icon);
		packet->setSubstructDataByName("spell_info", "next_icon2", spell2->icon_heroic_op);	// fix struct element name eventually
		packet->setSubstructDataByName("spell_info", "next_icontype", spell2->icon_backdrop);	// fix struct element name eventually

		if (packet->GetVersion() >= 63119) {
			packet->setSubstructDataByName("spell_info", "next_aa_spell_info2", "version", 0x04);
			packet->setSubstructDataByName("spell_info", "next_aa_spell_info2", "sub_version", 0x24);
		}
		else if (packet->GetVersion() >= 58617) {
			packet->setSubstructDataByName("spell_info", "next_version", 0x03);
			packet->setSubstructDataByName("spell_info", "next_sub_version", 0x131A);
		}
		else {
			packet->setSubstructDataByName("spell_info", "next_version", 0x00);
			packet->setSubstructDataByName("spell_info", "next_sub_version", 0xD9);
		}

		packet->setSubstructDataByName("spell_info", "next_type", spell2->type);
		packet->setSubstructDataByName("spell_info", "next_unknown_MJ1d", 1); //63119 test
		packet->setSubstructDataByName("spell_info", "next_class_skill", spell2->class_skill);
		packet->setSubstructDataByName("spell_info", "next_mastery_skill", spell2->mastery_skill);
		packet->setSubstructDataByName("spell_info", "next_duration_flag", spell2->duration_until_cancel);
		if (client && spell->type != 2) {
			sint8 spell_text_color = client->GetPlayer()->GetArrowColor(GetLevelRequired(client->GetPlayer()));
			if (spell_text_color != ARROW_COLOR_WHITE && spell_text_color != ARROW_COLOR_RED && spell_text_color != ARROW_COLOR_GRAY)
				spell_text_color = ARROW_COLOR_WHITE;
			spell_text_color -= 6;
			if (spell_text_color < 0)
				spell_text_color *= -1;
			packet->setSubstructDataByName("spell_info", "next_spell_text_color", spell_text_color);
		}
		else
			packet->setSubstructDataByName("spell_info", "next_spell_text_color", 3);
		if (spell->type != 2) {
			packet->setArrayLengthByName("num_levels", levels.size());
			for (int32 i = 0; i < levels.size(); i++) {
				packet->setArrayDataByName("spell_info_aa_adventure_class2", levels[i]->adventure_class, i);
				packet->setArrayDataByName("spell_info_aa_tradeskill_class2", levels[i]->tradeskill_class, i);
				packet->setArrayDataByName("spell_info_aa_spell_level2", levels[i]->spell_level, i);
			}
		}
		//packet->setSubstructDataByName("spell_info","unknown9", 20);
		hp_req = 0;
		power_req = 0;
		if (client) {
			hp_req = GetHPRequired(client->GetPlayer());
			power_req = GetPowerRequired(client->GetPlayer());

			// might need version checks around these?
			if (client->GetVersion() >= 1193)
			{
				int16 savagery_req = GetSavageryRequired(client->GetPlayer()); // dunno why we need to do this
				packet->setSubstructDataByName("spell_info", "next_savagery_req", savagery_req);
				packet->setSubstructDataByName("spell_info", "next_savagery_upkeep", spell->savagery_upkeep);
			}
			if (client->GetVersion() >= 57048)
			{
				int16 dissonance_req = GetDissonanceRequired(client->GetPlayer()); // dunno why we need to do this
				packet->setSubstructDataByName("spell_info", "next_dissonance_req", dissonance_req);
				packet->setSubstructDataByName("spell_info", "next_dissonance_upkeep", spell->dissonance_upkeep);
			}
		}
		packet->setSubstructDataByName("spell_info", "next_target", spell->target_type);
		packet->setSubstructDataByName("spell_info", "next_recovery", spell->recovery);
		packet->setSubstructDataByName("spell_info", "next_health_upkeep", spell->hp_upkeep);
		packet->setSubstructDataByName("spell_info", "next_health_req", hp_req);
		packet->setSubstructDataByName("spell_info", "next_tier", spell->tier);
		packet->setSubstructDataByName("spell_info", "next_power_req", power_req);
		packet->setSubstructDataByName("spell_info", "next_power_upkeep", spell->power_upkeep);

		packet->setSubstructDataByName("spell_info", "next_cast_time", spell->cast_time);
		packet->setSubstructDataByName("spell_info", "next_recast", spell->recast);
		packet->setSubstructDataByName("spell_info", "next_radius", spell->radius);
		packet->setSubstructDataByName("spell_info", "next_req_concentration", spell->req_concentration);
		//packet->setSubstructDataByName("spell_info","req_concentration2", 2);
		packet->setSubstructDataByName("spell_info", "next_max_aoe_targets", spell->max_aoe_targets);
		packet->setSubstructDataByName("spell_info", "next_friendly_spell", spell->friendly_spell);
		packet->setSubstructArrayLengthByName("spell_info", "next_num_effects", next_spell->effects.size());
		for (int32 i = 0; i < next_spell->effects.size(); i++) {
			packet->setArrayDataByName("next_subbulletflag", next_spell->effects[i]->subbullet, i);
			string effect_message;
			if (next_spell->effects[i]->description.length() > 0) {
				effect_message = next_spell->effects[i]->description;
				if (effect_message.find("%LM") < 0xFFFFFFFF) {
					int string_index = effect_message.find("%LM");
					int data_index = stoi(effect_message.substr(string_index + 3, 2));
					float value;
					if (next_spell->lua_data[data_index]->type == 1)
						value = next_spell->lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
					else
						value = next_spell->lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
					string strValue = to_string(value);
					strValue.erase(strValue.find_last_not_of('0') + 1, std::string::npos);
					effect_message.replace(effect_message.find("%LM"), 5, strValue);
				}
				// Magic damage min
				if (effect_message.find("%DML") < 0xFFFFFFFF) {
					int string_index = effect_message.find("%DML");
					int data_index = stoi(effect_message.substr(string_index + 4, 2));
					float value;
					if (next_spell->lua_data[data_index]->type == 1)
						value = next_spell->lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
					else
						value = next_spell->lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
						
					value = client->GetPlayer()->CalculateDamageAmount(nullptr, value, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
					string damage = to_string((int)round(value));
					damage.erase(damage.find_last_not_of('0') + 1, std::string::npos);
					effect_message.replace(effect_message.find("%DML"), 6, damage);
				}
				// Magic damage max
				if (effect_message.find("%DMH") < 0xFFFFFFFF) {
					int string_index = effect_message.find("%DMH");
					int data_index = stoi(effect_message.substr(string_index + 4, 2));
					float value;
					if (next_spell->lua_data[data_index]->type == 1)
						value = next_spell->lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
					else
						value = next_spell->lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
						
					value = client->GetPlayer()->CalculateDamageAmount(nullptr, value, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
					string damage = to_string((int)round(value));
					damage.erase(damage.find_last_not_of('0') + 1, std::string::npos);
					effect_message.replace(effect_message.find("%DMH"), 6, damage);
				}
				// level based Magic damage min
				if (effect_message.find("%LDML") < 0xFFFFFFFF) {
					int string_index = effect_message.find("%LDML");
					int data_index = stoi(effect_message.substr(string_index + 5, 2));
					float value;
					if (next_spell->lua_data[data_index]->type == 1)
						value = next_spell->lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
					else
						value = next_spell->lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
						
					value = client->GetPlayer()->CalculateDamageAmount(nullptr, value, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
					string damage = to_string((int)round(value));
					effect_message.replace(effect_message.find("%LDML"), 7, damage);
				}
				// level based Magic damage max
				if (effect_message.find("%LDMH") < 0xFFFFFFFF) {
					int string_index = effect_message.find("%LDMH");
					int data_index = stoi(effect_message.substr(string_index + 5, 2));
					float value;
					if (next_spell->lua_data[data_index]->type == 1)
						value = next_spell->lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
					else
						value = next_spell->lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
						
					value = client->GetPlayer()->CalculateDamageAmount(nullptr, value, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
					string damage = to_string((int)round(value));
					effect_message.replace(effect_message.find("%LDMH"), 7, damage);
				}
				//GetZone()->SimpleMessage(CHANNEL_COLOR_SPELL_EFFECT, effect_message.c_str(), victim, 50);
				packet->setArrayDataByName("next_effect", effect_message.c_str(), i);
			}

			packet->setArrayDataByName("next_percentage", next_spell->effects[i]->percentage, i);

		}
		if (display_tier == true)
			packet->setSubstructDataByName("spell_info", "next_display_spell_tier", 1);// spell->display_spell_tier);
		else
			packet->setSubstructDataByName("spell_info", "next_display_spell_tier", 1);//0
		packet->setSubstructDataByName("spell_info", "next_unknown_1", 1);//0
		packet->setSubstructDataByName("spell_info", "next_range", spell2->range);
		packet->setSubstructDataByName("spell_info", "next_duration_1", spell2->duration1);
		packet->setSubstructDataByName("spell_info", "next_duration_2", spell2->duration2);

		packet->setSubstructDataByName("spell_info", "next_can_effect_raid", spell2->can_effect_raid);
		packet->setSubstructDataByName("spell_info", "next_affect_only_group_members", spell2->affect_only_group_members);
		packet->setSubstructDataByName("spell_info", "next_group_spell", spell2->group_spell);
		packet->setSubstructDataByName("spell_info", "next_resistibility", spell2->resistibility);
		packet->setSubstructDataByName("spell_info", "next_name", &(spell2->name));
		packet->setSubstructDataByName("spell_info", "next_description", &(spell2->description));

	}
}

void Spell::AppendLevelInformation(PacketStruct* packet) {
	if(!packet)
		return;
	
	vector <LevelArray*> newLevels;
	vector <LevelArray*>* tmpArray = &levels;
	if(packet->GetVersion() <= 561) {
		for (int32 i = 0; i < tmpArray->size(); i++) {
			LevelArray* levelData = tmpArray->at(i);
			if((levelData->adventure_class != 255 && levelData->adventure_class > CLASSIC_MAX_ADVENTURE_CLASS) || (levelData->tradeskill_class != 255 && levelData->tradeskill_class > CLASSIC_MAX_TRADESKILL_CLASS))
				continue;
			
			newLevels.push_back(levelData);
		}
		tmpArray = &newLevels;
	}
	
	packet->setSubstructArrayLengthByName("spell_info", "num_levels", tmpArray->size());
	for (int32 i = 0; i < tmpArray->size(); i++) {
		LevelArray* levelData = tmpArray->at(i);
		packet->setArrayDataByName("adventure_class", levelData->adventure_class, i);
		packet->setArrayDataByName("tradeskill_class", levelData->tradeskill_class, i);
		if(rule_manager.GetGlobalRule(R_Spells, UseClassicSpellLevel)->GetInt8() && levelData->classic_spell_level) {
			packet->setArrayDataByName("spell_level", (int16)(levelData->classic_spell_level * 10.0f), i);
		}
		else {
			packet->setArrayDataByName("spell_level", levelData->spell_level, i);
		}
	}
}

sint16 Spell::TranslateClientSpellIcon(int16 version) {
	sint16 spell_icon = GetSpellIcon();
	if(version <= 561) {
		switch(spell_icon) {
			case 772: // tracking
				spell_icon = 231; // ??
				break;
			case 773: // mining
				spell_icon = 33; // OK
				break;
			case 774: // gathering
				spell_icon = 56; // OK
				break;
			case 775: // fishing
				spell_icon = 55; // OK
				break;
			case 776: // trapping
				spell_icon = 46; // OK
				break;
			case 777: // foresting
				spell_icon = 125; // OK
				break;
			case 778: // collecting
				spell_icon = 167; // OK
				break;
		}
	}
	return spell_icon;
}

void Spell::SetPacketInformation(PacketStruct* packet, Client* client, bool display_tier) {
	packet->setSubstructDataByName("spell_info", "id", spell->id);
	packet->setSubstructDataByName("spell_info", "icon", TranslateClientSpellIcon(client->GetVersion()));
	packet->setSubstructDataByName("spell_info", "icon2", spell->icon_heroic_op);	// fix struct element name eventually
	packet->setSubstructDataByName("spell_info", "icontype", spell->icon_backdrop);	// fix struct element name eventually

	if (packet->GetVersion() >= 63119) {
		packet->setSubstructDataByName("spell_info", "version", 0x04);
		packet->setSubstructDataByName("spell_info", "sub_version", 0x24);
	}
	else if (packet->GetVersion() >= 60114) {
		packet->setSubstructDataByName("spell_info", "version", 0x03);
		packet->setSubstructDataByName("spell_info", "sub_version", 4890);
	}
	else if (packet->GetVersion() <= 561) {
		packet->setSubstructDataByName("spell_info", "version", 0x10);
		packet->setSubstructDataByName("spell_info", "sub_version", 0x0f);
	}
	else {
		packet->setSubstructDataByName("spell_info", "version", 0x11);
		packet->setSubstructDataByName("spell_info", "sub_version", 0x14);
	}

	packet->setSubstructDataByName("spell_info", "type", spell->type);
	packet->setSubstructDataByName("spell_info", "unknown_MJ1d", 1); //63119 test
	packet->setSubstructDataByName("spell_info", "class_skill", spell->class_skill);
	packet->setSubstructDataByName("spell_info", "min_class_skill_req", spell->min_class_skill_req);
	packet->setSubstructDataByName("spell_info", "mastery_skill", spell->mastery_skill);
	packet->setSubstructDataByName("spell_info", "duration_flag", spell->duration_until_cancel);
	if (client && spell->type != 2) {
		sint8 spell_text_color = client->GetPlayer()->GetArrowColor(GetLevelRequired(client->GetPlayer()));
		if (spell_text_color != ARROW_COLOR_WHITE && spell_text_color != ARROW_COLOR_RED && spell_text_color != ARROW_COLOR_GRAY)
			spell_text_color = ARROW_COLOR_WHITE;
		spell_text_color -= 6;
		if (spell_text_color < 0)
			spell_text_color *= -1;
		packet->setSubstructDataByName("spell_info", "spell_text_color", spell_text_color);
	}
	else
		packet->setSubstructDataByName("spell_info", "spell_text_color", 3);
	if (spell->type != 2) {
		AppendLevelInformation(packet);
	}
	packet->setSubstructDataByName("spell_info", "unknown9", 20);
	int16 hp_req = 0;
	int16 power_req = 0;
	if (client) {
		hp_req = GetHPRequired(client->GetPlayer());
		power_req = GetPowerRequired(client->GetPlayer());

		// might need version checks around these?
		if (client->GetVersion() >= 1193)
		{
			int16 savagery_req = GetSavageryRequired(client->GetPlayer()); // dunno why we need to do this
			packet->setSubstructDataByName("spell_info", "savagery_req", savagery_req);
			packet->setSubstructDataByName("spell_info", "savagery_upkeep", spell->savagery_upkeep);
		}
		if (client->GetVersion() >= 57048)
		{
			int16 dissonance_req = GetDissonanceRequired(client->GetPlayer()); // dunno why we need to do this
			packet->setSubstructDataByName("spell_info", "dissonance_req", dissonance_req);
			packet->setSubstructDataByName("spell_info", "dissonance_upkeep", spell->dissonance_upkeep);
		}
	}
	packet->setSubstructDataByName("spell_info", "target", spell->target_type);
	packet->setSubstructDataByName("spell_info", "recovery", spell->recovery);
	packet->setSubstructDataByName("spell_info", "health_upkeep", spell->hp_upkeep);
	packet->setSubstructDataByName("spell_info", "health_req", hp_req);
	packet->setSubstructDataByName("spell_info", "tier", spell->tier);
	packet->setSubstructDataByName("spell_info", "power_req", power_req);
	packet->setSubstructDataByName("spell_info", "power_upkeep", spell->power_upkeep);
	if (packet->GetVersion() <= 561) {//cast times are displayed differently on new clients
		packet->setSubstructDataByName("spell_info", "cast_time", spell->cast_time/10);
	}
	else {
		packet->setSubstructDataByName("spell_info", "cast_time", spell->cast_time);
	}
	packet->setSubstructDataByName("spell_info", "recast", CalculateRecastTimer(client->GetPlayer())/1000);
	packet->setSubstructDataByName("spell_info", "radius", spell->radius);
	packet->setSubstructDataByName("spell_info", "req_concentration", spell->req_concentration);
	//packet->setSubstructDataByName("spell_info","req_concentration2", 2);
	packet->setSubstructDataByName("spell_info", "max_aoe_targets", spell->max_aoe_targets);
	packet->setSubstructDataByName("spell_info", "friendly_spell", spell->friendly_spell);
	packet->setSubstructArrayLengthByName("spell_info", "num_effects", effects.size());
	for (int32 i = 0; i < effects.size(); i++) {

		packet->setArrayDataByName("subbulletflag", effects[i]->subbullet, i);
		string effect_message;
		if (effects[i]->description.length() > 0) {
			effect_message = effects[i]->description;
			if (effect_message.find("%LM") < 0xFFFFFFFF) {
				int string_index = effect_message.find("%LM");
				int data_index = stoi(effect_message.substr(string_index + 3, 2));
				float value;
				if (lua_data[data_index]->type == 1)
					value = lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
				else
					value = lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
				string strValue = to_string(value);
				strValue.erase(strValue.find_last_not_of('0') + 1, std::string::npos);
				effect_message.replace(effect_message.find("%LM"), 5, strValue);
			}
			// Magic damage min
			if (effect_message.find("%DML") < 0xFFFFFFFF) {
				int string_index = effect_message.find("%DML");
				int data_index = stoi(effect_message.substr(string_index + 4, 2));
				float value;
				if (lua_data[data_index]->type == 1)
					value = lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
				else
					value = lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
				
				value = client->GetPlayer()->CalculateDamageAmount(nullptr, value, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
				string damage = to_string((int)round(value));
				effect_message.replace(effect_message.find("%DML"), 6, damage);
			}
			// Magic damage max
			if (effect_message.find("%DMH") < 0xFFFFFFFF) {
				int string_index = effect_message.find("%DMH");
				int data_index = stoi(effect_message.substr(string_index + 4, 2));
				float value;
				if (lua_data[data_index]->type == 1)
					value = lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
				else
					value = lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
					
				value = client->GetPlayer()->CalculateDamageAmount(nullptr, value, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
				string damage = to_string((int)round(value));
				effect_message.replace(effect_message.find("%DMH"), 6, damage);
			}
			// level based Magic damage min
			if (effect_message.find("%LDML") < 0xFFFFFFFF) {
				int string_index = effect_message.find("%LDML");
				int data_index = stoi(effect_message.substr(string_index + 5, 2));
				float value;
				if (lua_data[data_index]->type == 1)
					value = lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
				else
					value = lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();

				value = client->GetPlayer()->CalculateDamageAmount(nullptr, value, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
				string damage = to_string((int)round(value));
				effect_message.replace(effect_message.find("%LDML"), 7, damage);
			}
			// level based Magic damage max
			if (effect_message.find("%LDMH") < 0xFFFFFFFF) {
				int string_index = effect_message.find("%LDMH");
				int data_index = stoi(effect_message.substr(string_index + 5, 2));
				float value;
				if (lua_data[data_index]->type == 1)
					value = lua_data[data_index]->float_value * client->GetPlayer()->GetLevel();
				else
					value = lua_data[data_index]->int_value * client->GetPlayer()->GetLevel();
				
				value = client->GetPlayer()->CalculateDamageAmount(nullptr, value, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
				string damage = to_string((int)round(value));
				effect_message.replace(effect_message.find("%LDMH"), 7, damage);
			}

			boost::regex re("([0-9]{1,10})\\s+\\-\\s+([0-9]{1,10})");
			boost::match_results<std::string::const_iterator> base_match;
			if (boost::regex_search(effect_message, base_match, re, boost::match_partial)) {
				boost::ssub_match match = base_match[0];
				std::size_t midPos = match.str().find(" - ");
				if(midPos != std::string::npos)
				{
					int32 minValue = atoul(match.str().substr(0, midPos).c_str());
					int32 maxValue = atoul(match.str().substr(midPos+3).c_str());

					int32 newMin = minValue;
					
					bool crit = false;
					Skill* skill = master_skill_list.GetSkill(spell->mastery_skill);
					std::string skillName = "";
					
					if(skill)
						skillName = skill->name.data;

					if(skillName == "Aggression")
						newMin = (int32)client->GetPlayer()->CalculateHateAmount(nullptr, (sint32)minValue);
					else if(spell->friendly_spell && skillName == "Ministration")
						newMin = (int32)client->GetPlayer()->CalculateHealAmount(nullptr, (sint32)minValue, 0, &crit, true);
					else
						newMin = (int32)client->GetPlayer()->CalculateDamageAmount(nullptr, (sint32)minValue, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
					
					int32 newMax = maxValue;
					if(skillName == "Aggression")
						newMax = (int32)client->GetPlayer()->CalculateHateAmount(nullptr, (sint32)maxValue);
					else if(spell->friendly_spell && skillName == "Ministration")
						newMax = (int32)client->GetPlayer()->CalculateHealAmount(nullptr, (sint32)maxValue, 0, &crit, true);
					else
						newMax = (int32)client->GetPlayer()->CalculateDamageAmount(nullptr, (sint32)maxValue, DAMAGE_PACKET_TYPE_SPELL_DAMAGE, spell->type, spell->target_type);
					
					std::string newStr = std::to_string(newMin) + " - " + std::to_string(newMax);

					effect_message.replace(effect_message.find(match.str()), match.str().size(), newStr);
				}
			}
			//GetZone()->SimpleMessage(CHANNEL_COLOR_SPELL_EFFECT, effect_message.c_str(), victim, 50);
		}
		packet->setArrayDataByName("effect", effect_message.c_str(), i);
		packet->setArrayDataByName("percentage", effects[i]->percentage, i);
	}
	if (display_tier == true)
		packet->setSubstructDataByName("spell_info", "display_spell_tier", spell->display_spell_tier);
	else
		packet->setSubstructDataByName("spell_info", "display_spell_tier", 0);
	packet->setSubstructDataByName("spell_info", "range", spell->range);
	packet->setSubstructDataByName("spell_info", "duration1", spell->duration1);
	packet->setSubstructDataByName("spell_info", "duration2", spell->duration2);

	packet->setSubstructDataByName("spell_info", "can_effect_raid", spell->can_effect_raid);
	packet->setSubstructDataByName("spell_info", "affect_only_group_members", spell->affect_only_group_members);
	packet->setSubstructDataByName("spell_info", "group_spell", spell->group_spell);
	packet->setSubstructDataByName("spell_info", "resistibility", spell->resistibility);
	packet->setSubstructDataByName("spell_info", "name", &(spell->name));
	packet->setSubstructDataByName("spell_info", "description", &(spell->description));
	//packet->PrintPacket();
}
EQ2Packet* Spell::SerializeSpecialSpell(Client* client, bool display, int8 packet_type, int8 sub_packet_type) {
	if (client->GetVersion() <= 373)
		return SerializeSpell(client, display, false, packet_type, 0, "WS_ExaminePartialSpellInfo");
	return SerializeSpell(client, display, false, packet_type, sub_packet_type, "WS_ExamineSpecialSpellInfo");
}


EQ2Packet* Spell::SerializeAASpell(Client* client, int8 tier, AltAdvanceData* data, bool display, bool trait_display, int8 packet_type, int8 sub_packet_type, const char* struct_name) {
	if (!client)
		return 0;
	int16 version = 1;
	if (client)
		version = client->GetVersion();
	if (!struct_name)
		struct_name = "WS_ExamineAASpellInfo";
	PacketStruct* packet = configReader.getStruct(struct_name, version);
	if (display)
		packet->setSubstructDataByName("info_header", "show_name", 1);//1
	else
		if (!trait_display)
			packet->setSubstructDataByName("info_header", "show_popup", 1);//1
		else
			packet->setSubstructDataByName("info_header", "show_popup", 0);

	if (packet_type > 0)
		packet->setSubstructDataByName("info_header", "packettype", packet_type * 256 + 0xFE);
	else
		packet->setSubstructDataByName("info_header", "packettype", 0x4FFE);// 0x45FE    GetItemPacketType(version));
	//packet->setDataByName("unknown2",5);
	//packet->setDataByName("unknown7", 1);
	//packet->setDataByName("unknown9", 20);
	//packet->setDataByName("unknown10", 1, 2);
	if (sub_packet_type == 0)
		sub_packet_type = 0x83;
	packet->setSubstructDataByName("info_header", "packetsubtype", 4);// sub_packet_type);
	packet->setSubstructDataByName("spell_info", "aa_id", data->spellID);
	packet->setSubstructDataByName("spell_info", "aa_tab_id", data->group);
	packet->setSubstructDataByName("spell_info", "aa_icon", data->icon);
	packet->setSubstructDataByName("spell_info", "aa_icon2", data->icon2);
	packet->setSubstructDataByName("spell_info", "aa_current_rank", tier); // how to get this info to here?
	packet->setSubstructDataByName("spell_info", "aa_max_rank", data->maxRank);
	packet->setSubstructDataByName("spell_info", "aa_rank_cost", data->rankCost);
	packet->setSubstructDataByName("spell_info", "aa_unknown_2", 20);
	packet->setSubstructDataByName("spell_info", "aa_name", &(spell->name));
	packet->setSubstructDataByName("spell_info", "aa_description", &(spell->description));




	//packet->setDataByName("unknown3",2);
	//packet->setDataByName("unknown7", 50);
	if (sub_packet_type == 0x81)
		SetAAPacketInformation(packet, data, client);
	else
		SetAAPacketInformation(packet, data, client, true);
	packet->setSubstructDataByName("spell_info", "uses_remaining", 0xFFFF);
	packet->setSubstructDataByName("spell_info", "damage_remaining", 0xFFFF);
	//packet->PrintPacket();
	// This adds the second portion to the spell packet. Could be used for bonuses etc.?
	string* data1 = packet->serializeString();
	uchar* data2 = (uchar*)data1->c_str();
	uchar* ptr2 = data2;
	int32 size = data1->length();// *2;
	////uchar* data3 = new uchar[size];
	////memcpy(data3, data2, data1->length());
	////uchar* ptr = data3;
	////size -= 17;
	////memcpy(ptr, &size, sizeof(int32));
	////size += 3;
	////ptr += data1->length();
	////ptr2 += 14;
	////memcpy(ptr, ptr2, data1->length() - 14);

	EQ2Packet* outapp = new EQ2Packet(OP_ClientCmdMsg, data2, size);
	//DumpPacket(outapp);
	//safe_delete_array(data3);
	safe_delete(packet);
	return outapp;
}

EQ2Packet* Spell::SerializeSpell(Client* client, bool display, bool trait_display, int8 packet_type, int8 sub_packet_type, const char* struct_name, bool send_partial_packet) {
	int16 version = 1;
	if (client)
		version = client->GetVersion();
	if (!struct_name)
		struct_name = "WS_ExamineSpellInfo";
	if (version <= 561) {
		if (packet_type == 1)
			struct_name = "WS_ExamineEffectInfo";
		else if (!display && (version<=373 || send_partial_packet))
			struct_name = "WS_ExaminePartialSpellInfo";
		else
			struct_name = "WS_ExamineSpellInfo";
	}
	PacketStruct* packet = configReader.getStruct(struct_name, version);
	if (display)
		packet->setSubstructDataByName("info_header", "show_name", 1);
	else {
		if (!trait_display)
			packet->setSubstructDataByName("info_header", "show_popup", 1);
		else
			packet->setSubstructDataByName("info_header", "show_popup", 0);
	}
	if (version > 561) {
		if (packet_type > 0)
			packet->setSubstructDataByName("info_header", "packettype", packet_type * 256 + 0xFE);
		else
			packet->setSubstructDataByName("info_header", "packettype", GetItemPacketType(version));
	}
	else {
		if (packet_type == 3 || packet_type == 0)
			packet->setSubstructDataByName("info_header", "packettype", 3); // 0: item, 1: effect, 2: recipe, 3: spell/ability
		else
			packet->setSubstructDataByName("info_header", "packettype", 1);
	}
	//packet->setDataByName("unknown2",5);
	//packet->setDataByName("unknown7", 1);
	//packet->setDataByName("unknown9", 20);
	//packet->setDataByName("unknown10", 1, 2);
	if (sub_packet_type == 0)
		sub_packet_type = 0x83;
	packet->setSubstructDataByName("info_header", "packetsubtype", sub_packet_type);
	//packet->setDataByName("unknown3",2);
	//packet->setDataByName("unknown7", 50);
	if (sub_packet_type == 0x81)
		SetPacketInformation(packet, client);
	else
		SetPacketInformation(packet, client, true);
	packet->setSubstructDataByName("spell_info", "uses_remaining", 0xFFFF);
	packet->setSubstructDataByName("spell_info", "damage_remaining", 0xFFFF);
	//packet->PrintPacket();
	// This adds the second portion to the spell packet. Could be used for bonuses etc.?
	int8 offset = 0;
	if (packet->GetVersion() == 60114) {
		offset = 28;
	}
	else {
		offset = 14;
	}
	EQ2Packet* outapp = 0;
	if (version > 561) {
		string* data1 = packet->serializeString();
		uchar* data2 = (uchar*)data1->c_str();
		uchar* ptr2 = data2;
		int32 size = data1->length() * 2;
		uchar* data3 = new uchar[size];
		memcpy(data3, data2, data1->length());
		uchar* ptr = data3;
		size -= offset + 3;
		memcpy(ptr, &size, sizeof(int32));
		size += 3;
		ptr += data1->length();
		ptr2 += offset;
		memcpy(ptr, ptr2, data1->length() - offset);

		outapp = new EQ2Packet(OP_ClientCmdMsg, data3, size);
		safe_delete_array(data3);
		safe_delete(packet);
	}
	else
		outapp = packet->serialize();
	//DumpPacket(outapp);
	return outapp;
}

void Spell::AddSpellEffect(int8 percentage, int8 subbullet, string description){
	std::unique_lock lock(MSpellInfo);
	SpellDisplayEffect* effect = new SpellDisplayEffect;
	effect->description = description;
	effect->subbullet = subbullet;
	effect->percentage = percentage;
	effect->needs_db_save = false;
	
	effects.push_back(effect);
}

int16 Spell::GetHPRequired(Spawn* spawn){
	int16 hp_req = spell->hp_req;
	if(spawn && spell->hp_req_percent > 0){
		double result = ((double)spell->hp_req_percent/100)*spawn->GetTotalHP();
		if(result >= (((int16)result) + .5))
			result++;
		hp_req = (int16)result;
	}
	return hp_req;
}

int16 Spell::GetPowerRequired(Spawn* spawn){
	int16 power_req;
	if (spell->power_by_level == true) {
		power_req =round( (spell->power_req) * spawn->GetLevel());
	}
	else {
		power_req = round(spell->power_req);
	}
	if(spawn && spell->power_req_percent > 0){
		double result = ((double)spell->power_req_percent/100)*spawn->GetTotalPower();
		if(result >= (((int16)result) + .5))
			result++;
		power_req = (int16)result;
	}
	if(spawn && spawn->IsPlayer()) {
		int32 ministry_skill_id = rule_manager.GetGlobalRule(R_Spells, MinistrationSkillID)->GetInt32();
		if(spell->mastery_skill == ministry_skill_id) { // ministration offers a power reduction
			Skill* skill = ((Player*)spawn)->GetSkillByID(spell->mastery_skill, false);
			if(skill) {
				float ministry_reduction_percent = rule_manager.GetGlobalRule(R_Spells, MinistrationPowerReductionMax)->GetFloat();
				if(ministry_reduction_percent <= 0.0f)
					ministry_reduction_percent = 15.0f;
				
				int32 ministration_skill_reduce = rule_manager.GetGlobalRule(R_Spells, MinistrationPowerReductionSkill)->GetInt32();
				if(ministration_skill_reduce < 1)
					ministration_skill_reduce = 25;
				
				float item_stat_bonus = 0.0f;
				int32 item_stat = master_item_list.GetItemStatIDByName(::ToLower(skill->name.data));
				if(item_stat != 0xFFFFFFFF) {
					item_stat_bonus = ((Entity*)spawn)->GetStat(item_stat);
				}
				
				float reduction = (skill->current_val + item_stat_bonus) / ministration_skill_reduce;
				
				if(reduction > ministry_reduction_percent)
					reduction = ministry_reduction_percent;
				int16 power_to_reduce = (int16)(float)(power_req * (ministry_reduction_percent/100.0f)) * (reduction / ministry_reduction_percent);
				if(power_to_reduce > power_req) {
					power_req = 0;
				}
				else {
					power_req = power_req - power_to_reduce;
				}
			}
		}
		
		float power_reduction = ((Entity*)spawn)->GetStat(ITEM_STAT_POWER_COST_REDUCTION);
		if(power_reduction > 0.0f) {
				int16 power_to_reduce = (int16)(float)(power_req * (power_reduction/100.0f));
				if(power_to_reduce > power_req) {
					power_req = 0;
				}
				else {
					power_req = power_req - power_to_reduce;
				}
		}
	}
	return power_req;
}

int16 Spell::GetSavageryRequired(Spawn* spawn){
	int16 savagery_req = spell->savagery_req;
	if(spawn && spell->savagery_req_percent > 0){
		double result = ((double)spell->savagery_req_percent/100)*spawn->GetTotalSavagery();
		if(result >= (((int16)result) + .5))
			result++;
		savagery_req = (int16)result;
	}
	return savagery_req;
}

int16 Spell::GetDissonanceRequired(Spawn* spawn){
	int16 dissonance_req = spell->dissonance_req;
	if(spawn && spell->dissonance_req_percent > 0){
		double result = ((double)spell->dissonance_req_percent/100)*spawn->GetTotalDissonance();
		if(result >= (((int16)result) + .5))
			result++;
		dissonance_req = (int16)result;
	}
	return dissonance_req;
}

int32 Spell::GetSpellDuration(){
	if(spell->duration1 == spell->duration2)
		return spell->duration1;

	int32 difference = 0;
	int32 lower = 0;
	if(spell->duration2 > spell->duration1){
		difference = spell->duration2 - spell->duration1;
		lower = spell->duration1;
	}
	else{
		difference = spell->duration1 - spell->duration2;
		lower = spell->duration2;
	}
	int32 duration = (rand()%difference) + lower;
	return duration;
}

const char* Spell::GetName(){
	return spell->name.data.c_str();
}

const char* Spell::GetDescription(){
	return spell->description.data.c_str();
}

void Spell::AddSpellLevel(int8 adventure_class, int8 tradeskill_class, int16 level, float classic_spell_level){
	std::unique_lock lock(MSpellInfo);
	LevelArray* lvl = new LevelArray;
	lvl->adventure_class = adventure_class;
	lvl->tradeskill_class = tradeskill_class;
	lvl->spell_level = level;
	lvl->classic_spell_level = classic_spell_level;
	
	levels.push_back(lvl);
}

int32 Spell::GetSpellID(){
	if (spell)
		return spell->id;
	return 0;
}

int8 Spell::GetSpellTier(){
	if (spell)
		return spell->tier;
	return 0;
}

vector<LUAData*>* Spell::GetLUAData(){
	return &lua_data;
}
SpellData* Spell::GetSpellData(){
	return spell;
}

bool Spell::GetSpellData(lua_State* state, std::string field)
{
	if (!lua_interface)
		return false;

	bool valSet = false;

	if (field == "spell_book_type")
	{
		lua_interface->SetInt32Value(state, GetSpellData()->spell_book_type);
		valSet = true;
	}
	else if (field == "icon")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->icon);
		valSet = true;
	}
	else if (field == "icon_heroic_op")
	{
		lua_interface->SetInt32Value(state, GetSpellData()->icon_heroic_op);
		valSet = true;
	}
	else if (field == "icon_backdrop")
	{
		lua_interface->SetInt32Value(state, GetSpellData()->icon_backdrop);
		valSet = true;
	}
	else if (field == "type")
	{
		lua_interface->SetInt32Value(state, GetSpellData()->type);
		valSet = true;
	}
	else if (field == "class_skill")
	{
		lua_interface->SetInt32Value(state, GetSpellData()->class_skill);
		valSet = true;
	}
	else if (field == "min_class_skill_req")
	{
		lua_interface->SetInt32Value(state, GetSpellData()->min_class_skill_req);
		valSet = true;
	}
	else if (field == "mastery_skill")
	{
		lua_interface->SetInt32Value(state, GetSpellData()->mastery_skill);
		valSet = true;
	}
	else if (field == "ts_loc_index")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->ts_loc_index);
		valSet = true;
	}
	else if (field == "num_levels")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->num_levels);
		valSet = true;
	}
	else if (field == "tier")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->tier);
		valSet = true;
	}
	else if (field == "hp_req")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->hp_req);
		valSet = true;
	}
	else if (field == "hp_upkeep")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->hp_upkeep);
		valSet = true;
	}
	else if (field == "power_req")
	{
		lua_interface->SetFloatValue(state, GetSpellData()->power_req);
		valSet = true;
	}
	else if (field == "power_by_level")
	{
		lua_interface->SetBooleanValue(state, GetSpellData()->power_by_level);
		valSet = true;
	}
	else if (field == "power_upkeep")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->power_upkeep);
		valSet = true;
	}
	else if (field == "savagery_req")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->savagery_req);
		valSet = true;
	}
	else if (field == "savagery_upkeep")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->savagery_upkeep);
		valSet = true;
	}
	else if (field == "dissonance_req")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->dissonance_req);
		valSet = true;
	}
	else if (field == "dissonance_upkeep")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->dissonance_upkeep);
		valSet = true;
	}
	else if (field == "target_type")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->target_type);
		valSet = true;
	}
	else if (field == "cast_time")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->cast_time);
		valSet = true;
	}
	else if (field == "recovery")
	{
		lua_interface->SetFloatValue(state, GetSpellData()->recovery);
		valSet = true;
	}
	else if (field == "recast")
	{
		lua_interface->SetFloatValue(state, GetSpellData()->recast);
		valSet = true;
	}
	else if (field == "linked_timer")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->linked_timer);
		valSet = true;
	}
	else if (field == "radius")
	{
		lua_interface->SetFloatValue(state, GetSpellData()->radius);
		valSet = true;
	}
	else if (field == "max_aoe_targets")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->max_aoe_targets);
		valSet = true;
	}
	else if (field == "friendly_spell")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->friendly_spell);
		valSet = true;
	}
	else if (field == "req_concentration")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->req_concentration);
		valSet = true;
	}
	else if (field == "range")
	{
		lua_interface->SetFloatValue(state, GetSpellData()->range);
		valSet = true;
	}
	else if (field == "duration1")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->duration1);
		valSet = true;
	}
	else if (field == "duration2")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->duration2);
		valSet = true;
	}
	else if (field == "resistibility")
	{
		lua_interface->SetFloatValue(state, GetSpellData()->resistibility);
		valSet = true;
	}
	else if (field == "duration_until_cancel")
	{
		lua_interface->SetBooleanValue(state, GetSpellData()->duration_until_cancel);
		valSet = true;
	}
	else if (field == "power_req_percent")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->power_req_percent);
		valSet = true;
	}
	else if (field == "hp_req_percent")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->hp_req_percent);
		valSet = true;
	}
	else if (field == "savagery_req_percent")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->savagery_req_percent);
		valSet = true;
	}
	else if (field == "dissonance_req_percent")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->dissonance_req_percent);
		valSet = true;
	}
	else if (field == "name")
	{
		lua_interface->SetStringValue(state, GetSpellData()->name.data.c_str());
		valSet = true;
	}
	else if (field == "description")
	{
		lua_interface->SetStringValue(state, GetSpellData()->description.data.c_str());
		valSet = true;
	}
	else if (field == "success_message")
	{
		lua_interface->SetStringValue(state, GetSpellData()->success_message.c_str());
		valSet = true;
	}
	else if (field == "fade_message")
	{
		lua_interface->SetStringValue(state, GetSpellData()->fade_message.c_str());
		valSet = true;
	}
	else if (field == "fade_message_others")
	{
	lua_interface->SetStringValue(state, GetSpellData()->fade_message_others.c_str());
	valSet = true;
	}
	else if (field == "cast_type")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->cast_type);
		valSet = true;
	}
	else if (field == "lua_script")
	{
		lua_interface->SetStringValue(state, GetSpellData()->lua_script.c_str());
		valSet = true;
	}
	else if (field == "interruptable")
	{
		lua_interface->SetBooleanValue(state, GetSpellData()->interruptable);
		valSet = true;
	}
	else if (field == "spell_visual")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->spell_visual);
		valSet = true;
	}
	else if (field == "effect_message")
	{
		lua_interface->SetStringValue(state, GetSpellData()->effect_message.c_str());
		valSet = true;
	}
	else if (field == "min_range")
	{
		lua_interface->SetFloatValue(state, GetSpellData()->min_range);
		valSet = true;
	}
	else if (field == "can_effect_raid")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->can_effect_raid);
		valSet = true;
	}
	else if (field == "affect_only_group_members")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->affect_only_group_members);
		valSet = true;
	}
	else if (field == "group_spell")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->group_spell);
		valSet = true;
	}
	else if (field == "hit_bonus")
	{
		lua_interface->SetFloatValue(state, GetSpellData()->hit_bonus);
		valSet = true;
	}
	else if (field == "display_spell_tier")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->display_spell_tier);
		valSet = true;
	}
	else if (field == "is_active")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->is_active);
		valSet = true;
	}
	else if (field == "det_type")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->det_type);
		valSet = true;
	}
	else if (field == "incurable")
	{
		lua_interface->SetBooleanValue(state, GetSpellData()->incurable);
		valSet = true;
	}
	else if (field == "control_effect_type")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->control_effect_type);
		valSet = true;
	}
	else if (field == "casting_flags")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->casting_flags);
		valSet = true;
	}
	else if (field == "cast_while_moving")
	{
		lua_interface->SetBooleanValue(state, GetSpellData()->cast_while_moving);
		valSet = true;
	}
	else if (field == "persist_through_death")
	{
		lua_interface->SetBooleanValue(state, GetSpellData()->persist_through_death);
		valSet = true;
	}
	else if (field == "not_maintained")
	{
		lua_interface->SetBooleanValue(state, GetSpellData()->not_maintained);
		valSet = true;
	}
	else if (field == "is_aa")
	{
		lua_interface->SetBooleanValue(state, GetSpellData()->is_aa);
		valSet = true;
	}
	else if (field == "savage_bar")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->savage_bar);
		valSet = true;
	}
	else if (field == "savage_bar_slot")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->savage_bar_slot);
		valSet = true;
	}
	else if (field == "soe_spell_crc")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->soe_spell_crc);
		valSet = true;
	}
	else if (field == "spell_type")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->spell_type);
		valSet = true;
	}
	else if (field == "spell_name_crc")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->spell_name_crc);
		valSet = true;
	}
	else if (field == "type_group_spell_id")
	{
		lua_interface->SetSInt32Value(state, GetSpellData()->type_group_spell_id);
		valSet = true;
	}
	else if (field == "can_fizzle")
	{
		lua_interface->SetBooleanValue(state, GetSpellData()->can_fizzle);
		valSet = true;
	}

	return valSet;
}

bool Spell::SetSpellData(lua_State* state, std::string field, int8 fieldArg)
{
	if (!lua_interface)
		return false;

	bool valSet = false;

	if (field == "spell_book_type")
	{
		int32 spell_book_type = lua_interface->GetInt32Value(state, fieldArg);
		GetSpellData()->spell_book_type = spell_book_type;
		valSet = true;
	}
	else if (field == "icon")
	{
		sint16 icon = lua_interface->GetSInt32Value(state, fieldArg);
		GetSpellData()->icon = icon;
		valSet = true;
	}
	else if (field == "icon_heroic_op")
	{
		int16 icon_heroic_op = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->icon_heroic_op = icon_heroic_op;
		valSet = true;
	}
	else if (field == "icon_backdrop")
	{
		int16 icon_backdrop = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->icon_backdrop = icon_backdrop;
		valSet = true;
	}
	else if (field == "type")
	{
		int16 type = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->type = type;
		valSet = true;
	}
	else if (field == "class_skill")
	{
		int32 class_skill = lua_interface->GetInt32Value(state, fieldArg);
		GetSpellData()->class_skill = class_skill;
		valSet = true;
	}
	else if (field == "min_class_skill_req")
	{
		int16 min_class_skill_req = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->min_class_skill_req = min_class_skill_req;
		valSet = true;
	}
	else if (field == "mastery_skill")
	{
		int32 mastery_skill = lua_interface->GetInt32Value(state, fieldArg);
		GetSpellData()->mastery_skill = mastery_skill;
		valSet = true;
	}
	else if (field == "ts_loc_index")
	{
		int8 ts_loc_index = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->ts_loc_index = ts_loc_index;
		valSet = true;
	}
	else if (field == "num_levels")
	{
		int8 num_levels = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->num_levels = num_levels;
		valSet = true;
	}
	else if (field == "tier")
	{
		int8 tier = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->tier = tier;
		valSet = true;
	}
	else if (field == "hp_req")
	{
		int16 hp_req = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->hp_req = hp_req;
		valSet = true;
	}
	else if (field == "hp_upkeep")
	{
		int16 hp_upkeep = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->hp_upkeep = hp_upkeep;
		valSet = true;
	}
	else if (field == "power_req")
	{
		float power_req = lua_interface->GetFloatValue(state, fieldArg);
		GetSpellData()->power_req = power_req;
		valSet = true;
	}
	else if (field == "power_by_level")
	{
		bool power_by_level = lua_interface->GetBooleanValue(state, fieldArg);
		GetSpellData()->power_by_level = power_by_level;
		valSet = true;
	}
	else if (field == "power_upkeep")
	{
		int16 power_upkeep = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->power_upkeep = power_upkeep;
		valSet = true;
	}
	else if (field == "savagery_req")
	{
		int16 savagery_req = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->savagery_req = savagery_req;
		valSet = true;
	}
	else if (field == "savagery_upkeep")
	{
		int16 savagery_upkeep = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->savagery_upkeep = savagery_upkeep;
		valSet = true;
	}
	else if (field == "dissonance_req")
	{
		int16 dissonance_req = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->dissonance_req = dissonance_req;
		valSet = true;
	}
	else if (field == "dissonance_upkeep")
	{
		int16 dissonance_upkeep = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->dissonance_upkeep = dissonance_upkeep;
		valSet = true;
	}
	else if (field == "target_type")
	{
		int16 target_type = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->target_type = target_type;
		valSet = true;
	}
	else if (field == "cast_time")
	{
		int16 cast_time = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->orig_cast_time = cast_time;
		GetSpellData()->cast_time = cast_time;
		valSet = true;
	}
	else if (field == "recovery")
	{
		float recovery = lua_interface->GetFloatValue(state, fieldArg);
		GetSpellData()->recovery = recovery;
		valSet = true;
	}
	else if (field == "recast")
	{
		float recast = lua_interface->GetFloatValue(state, fieldArg);
		GetSpellData()->recast = recast;
		valSet = true;
	}
	else if (field == "linked_timer")
	{
		int32 linked_timer = lua_interface->GetInt32Value(state, fieldArg);
		GetSpellData()->linked_timer = linked_timer;
		valSet = true;
	}
	else if (field == "radius")
	{
		float radius = lua_interface->GetFloatValue(state, fieldArg);
		GetSpellData()->radius = radius;
		valSet = true;
	}
	else if (field == "max_aoe_targets")
	{
		int16 max_aoe_targets = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->max_aoe_targets = max_aoe_targets;
		valSet = true;
	}
	else if (field == "friendly_spell")
	{
		int8 friendly_spell = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->friendly_spell = friendly_spell;
		valSet = true;
	}
	else if (field == "req_concentration")
	{
		int16 req_concentration = lua_interface->GetInt16Value(state, fieldArg);
		GetSpellData()->req_concentration = req_concentration;
		valSet = true;
	}
	else if (field == "range")
	{
		float range = lua_interface->GetFloatValue(state, fieldArg);
		GetSpellData()->range = range;
		valSet = true;
	}
	else if (field == "duration1")
	{
		sint32 duration = lua_interface->GetSInt32Value(state, fieldArg);
		GetSpellData()->duration1 = duration;
		valSet = true;
	}
	else if (field == "duration2")
	{
		sint32 duration = lua_interface->GetSInt32Value(state, fieldArg);
		GetSpellData()->duration2 = duration;
		valSet = true;
	}
	else if (field == "resistibility")
	{
		float resistibility = lua_interface->GetFloatValue(state, fieldArg);
		GetSpellData()->resistibility = resistibility;
		valSet = true;
	}
	else if (field == "duration_until_cancel")
	{
		bool duration_until_cancel = lua_interface->GetBooleanValue(state, fieldArg);
		GetSpellData()->duration_until_cancel = duration_until_cancel;
		valSet = true;
	}
	else if (field == "power_req_percent")
	{
		int8 power_req_percent = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->power_req_percent = power_req_percent;
		valSet = true;
	}
	else if (field == "hp_req_percent")
	{
		int8 hp_req_percent = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->hp_req_percent = hp_req_percent;
		valSet = true;
	}
	else if (field == "savagery_req_percent")
	{
		int8 savagery_req_percent = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->savagery_req_percent = savagery_req_percent;
		valSet = true;
	}
	else if (field == "dissonance_req_percent")
	{
		int8 dissonance_req_percent = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->dissonance_req_percent = dissonance_req_percent;
		valSet = true;
	}
	else if (field == "name")
	{
		string name = lua_interface->GetStringValue(state, fieldArg);
		GetSpellData()->name.data = name;
		valSet = true;
	}
	else if (field == "description")
	{
		string description = lua_interface->GetStringValue(state, fieldArg);
		GetSpellData()->description.data = description;
		valSet = true;
	}
	else if (field == "success_message")
	{
		string success_message = lua_interface->GetStringValue(state, fieldArg);
		GetSpellData()->success_message = success_message;
		valSet = true;
	}
	else if (field == "fade_message")
	{
		string fade_message = lua_interface->GetStringValue(state, fieldArg);
		GetSpellData()->fade_message = fade_message;
		valSet = true;
	}
	else if (field == "fade_message_others")
	{
		string fade_message_others = lua_interface->GetStringValue(state, fieldArg);
		GetSpellData()->fade_message_others = fade_message_others;
		valSet = true;
	}
	else if (field == "cast_type")
	{
		int8 cast_type = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->cast_type = cast_type;
		valSet = true;
	}
	else if (field == "cast_type")
	{
		int32 call_frequency = lua_interface->GetInt32Value(state, fieldArg);
		GetSpellData()->call_frequency = call_frequency;
		valSet = true;
	}
	else if (field == "interruptable")
	{
		bool interruptable = lua_interface->GetBooleanValue(state, fieldArg);
		GetSpellData()->interruptable = interruptable;
		valSet = true;
	}
	else if (field == "spell_visual")
	{
		int32 spell_visual = lua_interface->GetInt32Value(state, fieldArg);
		GetSpellData()->spell_visual = spell_visual;
		valSet = true;
	}
	else if (field == "effect_message")
	{
		string effect_message = lua_interface->GetStringValue(state, fieldArg);
		GetSpellData()->effect_message = effect_message;
		valSet = true;
	}
	else if (field == "min_range")
	{
		float min_range = lua_interface->GetFloatValue(state, fieldArg);
		GetSpellData()->min_range = min_range;
		valSet = true;
	}
	else if (field == "can_effect_raid")
	{
		int8 can_effect_raid = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->can_effect_raid = can_effect_raid;
		valSet = true;
	}
	else if (field == "affect_only_group_members")
	{
		int8 affect_only_group_members = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->affect_only_group_members = affect_only_group_members;
		valSet = true;
	}
	else if (field == "group_spell")
	{
		int8 group_spell = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->group_spell = group_spell;
		valSet = true;
	}
	else if (field == "hit_bonus")
	{
		float hit_bonus = lua_interface->GetFloatValue(state, fieldArg);
		GetSpellData()->hit_bonus = hit_bonus;
		valSet = true;
	}
	else if (field == "display_spell_tier")
	{
		int8 display_spell_tier = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->display_spell_tier = display_spell_tier;
		valSet = true;
	}
	else if (field == "is_active")
	{
		int8 is_active = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->is_active = is_active;
		valSet = true;
	}
	else if (field == "det_type")
	{
		int8 det_type = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->det_type = det_type;
		valSet = true;
	}
	else if (field == "incurable")
	{
		bool incurable = lua_interface->GetBooleanValue(state, fieldArg);
		GetSpellData()->incurable = incurable;
		valSet = true;
	}
	else if (field == "control_effect_type")
	{
		int8 control_effect_type = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->control_effect_type = control_effect_type;
		valSet = true;
	}
	else if (field == "casting_flags")
	{
		int32 casting_flags = lua_interface->GetInt32Value(state, fieldArg);
		GetSpellData()->casting_flags = casting_flags;
		valSet = true;
	}
	else if (field == "cast_while_moving")
	{
		bool cast_while_moving = lua_interface->GetBooleanValue(state, fieldArg);
		GetSpellData()->cast_while_moving = cast_while_moving;
		valSet = true;
	}
	else if (field == "persist_through_death")
	{
		bool persist_through_death = lua_interface->GetBooleanValue(state, fieldArg);
		GetSpellData()->persist_through_death = persist_through_death;
		valSet = true;
	}
	else if (field == "not_maintained")
	{
		bool not_maintained = lua_interface->GetBooleanValue(state, fieldArg);
		GetSpellData()->not_maintained = not_maintained;
		valSet = true;
	}
	else if (field == "is_aa")
	{
		bool is_aa = lua_interface->GetBooleanValue(state, fieldArg);
		GetSpellData()->is_aa = is_aa;
		valSet = true;
	}
	else if (field == "savage_bar")
	{
		int8 savage_bar = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->savage_bar = savage_bar;
		valSet = true;
	}
	else if (field == "spell_type")
	{
		int8 spell_type = lua_interface->GetInt8Value(state, fieldArg);
		GetSpellData()->spell_type = spell_type;
		valSet = true;
	}
	else if (field == "type_group_spell_id")
	{
		sint32 type_group_spell_id = lua_interface->GetSInt32Value(state, fieldArg);
		GetSpellData()->type_group_spell_id = type_group_spell_id;
		valSet = true;
	}
	else if (field == "can_fizzle")
	{
		bool can_fizzle = lua_interface->GetBooleanValue(state, fieldArg);
		GetSpellData()->can_fizzle = can_fizzle;
		valSet = true;
	}

	return valSet;
}
int16 Spell::GetSpellIcon(){
	if (spell)
		return spell->icon;
	return 0;
}

int16 Spell::GetSpellIconBackdrop(){
	if (spell)
		return spell->icon_backdrop;
	return 0;
}

int16 Spell::GetSpellIconHeroicOp(){
	if (spell)
		return spell->icon_heroic_op;
	return 0;
}

bool Spell::IsHealSpell(){
	return heal_spell;
}

bool Spell::IsBuffSpell(){
	return buff_spell;
}

bool Spell::IsDamageSpell(){
	return damage_spell;
}
bool Spell::IsControlSpell(){
	return control_spell;
}

bool Spell::IsOffenseSpell() {
	return offense_spell;
}

bool Spell::IsCopiedSpell() {
	return copied_spell;
}

void Spell::ModifyCastTime(Entity* caster){
	int16 cast_time = spell->orig_cast_time;
	spell->cast_time = cast_time;
	float cast_speed = caster->GetInfoStruct()->get_casting_speed();
	if (cast_speed > 0.0f){
		bool modifiedSpeed = false;
		if (cast_speed > 0.0f && (modifiedSpeed = true)) // casting speed can only reduce up to half a cast time
			spell->cast_time *= max(0.5f, (float) (1 / (1 + (cast_speed * .01))));
		else if (cast_speed < 0.0f && (modifiedSpeed = true)) // not sure if casting speed debuff is capped on live or not, capping at 1.5 * the normal rate for now
			spell->cast_time *= min(1.5f, (float) (1 + (1 - (1 / (1 + (cast_speed * -.01))))));
		
		if(modifiedSpeed) {
			LogWrite(SPELL__DEBUG, 9, "Spells", "%s: spell %s cast time %u to %u based on cast_speed %f", GetName(), caster->GetName(), cast_time, spell->cast_time, cast_speed);
		}
	}
}

int32 Spell::CalculateRecastTimer(Entity* caster, float override_timer) {
	int32 original_recast = static_cast<int32>(GetSpellData()->recast * 1000.0f);
	
	if(override_timer > 0.0f) {
		original_recast = static_cast<int32>(override_timer);
	}
	
	int32 recast_time = original_recast;
	float cast_speed = caster->GetInfoStruct()->get_spell_reuse_speed();
	if (cast_speed > 0.0f){
		bool modifiedSpeed = false;
		if (cast_speed > 0.0f && (modifiedSpeed = true)) // casting speed can only reduce up to half a cast time
			recast_time *= max(0.5f, (float) (1 / (1 + (cast_speed * .01))));
		else if (cast_speed < 0.0f && (modifiedSpeed = true)) // not sure if casting speed debuff is capped on live or not, capping at 1.5 * the normal rate for now
			recast_time *= min(1.5f, (float) (1 + (1 - (1 / (1 + (cast_speed * -.01))))));
		
		if(modifiedSpeed) {
			LogWrite(SPELL__DEBUG, 9, "Spells", "%s: spell %s recast time %u to %u based on spell_reuse_time %f", GetName(), caster->GetName(), original_recast, recast_time, cast_speed);
		}
	}
	return recast_time;
}

vector <SpellDisplayEffect*>* Spell::GetSpellEffects(){
	std::shared_lock lock(MSpellInfo);
	vector <SpellDisplayEffect*>* ret = &effects;
	return ret;
}

vector <LevelArray*>* Spell::GetSpellLevels(){
	std::shared_lock lock(MSpellInfo);
	vector <LevelArray*>* ret = &levels;
	return ret;
}

bool Spell::ScribeAllowed(Player* player){
	std::shared_lock lock(MSpellInfo);
	bool ret = false;
	double current_xp_percent = ((double)player->GetXP()/(double)player->GetNeededXP());
	if(player){
		for(int32 i=0;!ret && i<levels.size();i++){
			bool classiclevelmatch = ((double)player->GetLevel()+current_xp_percent) >= levels[i]->classic_spell_level;
			bool classic_match_only = false;
			if(levels[i]->classic_spell_level > 0.0f && rule_manager.GetGlobalRule(R_Spells, UseClassicSpellLevel)->GetInt8()) {
				classic_match_only = true;
			}
			if((player->GetAdventureClass() == levels[i]->adventure_class || player->GetTradeskillClass() == levels[i]->tradeskill_class) && ((!classic_match_only && player->GetLevel() >= levels[i]->spell_level/10) || (classic_match_only && classiclevelmatch)))
				ret = true;
		}
	}
	return ret;
}

MasterSpellList::MasterSpellList(){
	max_spell_id = 0;
	MMasterSpellList.SetName("MasterSpellList::MMasterSpellList");
}

MasterSpellList::~MasterSpellList(){
	DestroySpells();
}
void MasterSpellList::DestroySpells(){

	spell_errors.clear();

	MMasterSpellList.lock();
	map<int32, map<int32, Spell*> >::iterator iter;
	map<int32, Spell*>::iterator iter2;
	for(iter = spell_list.begin();iter != spell_list.end(); iter++){
		for(iter2 = iter->second.begin();iter2 != iter->second.end(); iter2++){
			safe_delete(iter2->second);
		}
	}
	spell_list.clear();
	for(int i=0;i<MAX_CLASSES;i++) {
		class_spell_list[i].clear();
	}
	MMasterSpellList.unlock();
}
void MasterSpellList::AddSpell(int32 id, int8 tier, Spell* spell){
	MMasterSpellList.lock();
	spell_list[id][tier] = spell;
	
	vector<LevelArray*>* levels = spell->GetSpellLevels();
	LevelArray* level = 0;
	vector<LevelArray*>::iterator level_itr;
	for(level_itr = levels->begin(); level_itr != levels->end(); level_itr++){
		level = *level_itr;
		if(level->adventure_class && level->adventure_class < MAX_CLASSES){
			class_spell_list[level->adventure_class][id][tier] = spell;
		}
		if(level->tradeskill_class && level->tradeskill_class < MAX_CLASSES) {
			class_spell_list[level->tradeskill_class][id][tier] = spell;
		}
	}
	
	spell_name_map[spell->GetName()] = spell;
	spell_soecrc_map[spell->GetSpellData()->soe_spell_crc] = spell;

	if (id > max_spell_id)
		max_spell_id = id;

	MMasterSpellList.unlock();
}

Spell* MasterSpellList::GetSpell(int32 id, int8 tier){
	if (spell_list.count(id) > 0 && spell_list[id].count(tier) > 0)
		return spell_list[id][tier];
	else if (spell_list.count(id) > 0 && tier == 0 && spell_list[id].count(1) > 0)
		return spell_list[id][1];
	return 0;
}

Spell* MasterSpellList::GetSpellByName(const char* name){
	if(spell_name_map.count(name) > 0)
		return spell_name_map[name];
	return 0;
}

Spell* MasterSpellList::GetSpellByCRC(int32 spell_crc){
	if(spell_soecrc_map.count(spell_crc) > 0)
		return spell_soecrc_map[spell_crc];
	return 0;
}

EQ2Packet* MasterSpellList::GetSpellPacket(int32 id, int8 tier, Client* client, bool display, int8 packet_type){
	Spell* spell = GetSpell(id, tier);

	// if we can't find it on the master spell list, see if it is a custom spell
	if (!spell)
	{
		lua_interface->FindCustomSpellLock();
		LuaSpell* tmpSpell = lua_interface->FindCustomSpell(id);
		EQ2Packet* pack = 0;
		if (tmpSpell)
		{
			spell = tmpSpell->spell;
			pack = spell->SerializeSpell(client, display, packet_type);
		}

		lua_interface->FindCustomSpellUnlock();
		return pack;
	}

	if(spell)
		return spell->SerializeSpell(client, display, packet_type);
	return 0;
}
EQ2Packet* MasterSpellList::GetAASpellPacket(int32 id, int8 tier, Client* client, bool display, int8 packet_type) {
	Spell* spell = GetSpell(id, (tier == 0 ? 1 : tier));

	// if we can't find it on the master spell list, see if it is a custom spell
	if (!spell)
	{
		lua_interface->FindCustomSpellLock();
		LuaSpell* tmpSpell = lua_interface->FindCustomSpell(id);
		EQ2Packet* pack = 0;
		if (tmpSpell)
		{
			spell = tmpSpell->spell;
			// TODO: this isn't a tested thing yet... need to add custom spells to alt advancement?
			AltAdvanceData* data = master_aa_list.GetAltAdvancement(id);

			if(data)
				pack = spell->SerializeAASpell(client, tier, data, display, false, packet_type);
		}

		lua_interface->FindCustomSpellUnlock();
		return pack;
	}

	//Spell* spell2= GetSpell(id, (tier +1));
	AltAdvanceData* data = master_aa_list.GetAltAdvancement(id);
	if (spell)
		return spell->SerializeAASpell(client,tier, data, display,false, packet_type);
	return 0;
}

EQ2Packet* MasterSpellList::GetSpecialSpellPacket(int32 id, int8 tier, Client* client, bool display, int8 packet_type){
	Spell* spell = GetSpell(id, tier);

	// if we can't find it on the master spell list, see if it is a custom spell
	if (!spell)
	{
		lua_interface->FindCustomSpellLock();
		LuaSpell* tmpSpell = lua_interface->FindCustomSpell(id);
		EQ2Packet* pack = 0;
		if (tmpSpell)
		{
			spell = tmpSpell->spell;
			pack = spell->SerializeSpecialSpell(client, display, packet_type, 0x81);
		}

		lua_interface->FindCustomSpellUnlock();
		return pack;
	}

	if(spell)
		return spell->SerializeSpecialSpell(client, display, packet_type, 0x81);
	return 0;
}

vector<Spell*>* MasterSpellList::GetSpellListByAdventureClass(int8 class_id, double max_level_classic, int8 max_tier){
	vector<Spell*>* ret = new vector<Spell*>;
	if(class_id >= MAX_CLASSES) {
		return ret;
	}
	
	int8 use_classic_levels = rule_manager.GetGlobalRule(R_Spells, UseClassicSpellLevel)->GetInt8();
	Spell* spell = 0;
	vector<LevelArray*>* levels = 0;
	LevelArray* level = 0;
	vector<LevelArray*>::iterator level_itr;
	MMasterSpellList.lock();
	map<int32, map<int32, Spell*> >::iterator iter;
	map<int32, Spell*>::iterator iter2;
	int16 max_level = std::floor(max_level_classic) * 10; //convert to client level format, which is 10 times higher
	for(iter = class_spell_list[class_id].begin();iter != class_spell_list[class_id].end(); iter++){
		for(iter2 = iter->second.begin();iter2 != iter->second.end(); iter2++){
			spell = iter2->second;
			if(iter2->first <= max_tier && spell && spell->GetSpellData()->given_by_type != GivenByType::GivenBy_SpellScroll && 
				spell->GetSpellData()->given_by_type != GivenByType::GivenBy_TradeskillClass){
				levels = spell->GetSpellLevels();
				for(level_itr = levels->begin(); level_itr != levels->end(); level_itr++){
					level = *level_itr;
					if(level->adventure_class == class_id){
						if((!use_classic_levels || level->classic_spell_level == 0 || level->classic_spell_level == 0.0f) && level->spell_level <= max_level) {
							ret->push_back(spell);
							break;
						}
						else if(use_classic_levels && level->classic_spell_level <= max_level_classic && level->classic_spell_level > 0.0f) {
							ret->push_back(spell);
							break;
						}
					}
				}
			}
		}
	}
	MMasterSpellList.unlock();
	return ret;
}

vector<Spell*>* MasterSpellList::GetSpellListByTradeskillClass(int8 class_id, int16 max_level, int8 max_tier){
	vector<Spell*>* ret = new vector<Spell*>;
	if(class_id >= MAX_CLASSES) {
		return ret;
	}
	
	Spell* spell = 0;
	vector<LevelArray*>* levels = 0;
	LevelArray* level = 0;
	vector<LevelArray*>::iterator level_itr;
	MMasterSpellList.lock();
	map<int32, map<int32, Spell*> >::iterator iter;
	map<int32, Spell*>::iterator iter2;
	for(iter = class_spell_list[class_id].begin();iter != class_spell_list[class_id].end(); iter++){
		for(iter2 = iter->second.begin();iter2 != iter->second.end(); iter2++){
			spell = iter2->second;
			if(iter2->first <= max_tier && spell && spell->GetSpellData()->given_by_type == GivenByType::GivenBy_TradeskillClass){
				levels = spell->GetSpellLevels();
				for(level_itr = levels->begin(); level_itr != levels->end(); level_itr++){
					level = *level_itr;
					if(level->spell_level <= max_level && level->tradeskill_class == class_id){
						ret->push_back(spell);
						break;
					}
				}
			}
		}
	}
	MMasterSpellList.unlock();
	return ret;
}

void MasterSpellList::Reload(){
	master_trait_list.DestroyTraits();
	DestroySpells();
	database.LoadSpells();
	database.LoadSpellErrors();
	database.LoadTraits();
}

int16 MasterSpellList::GetSpellErrorValue(int16 version, int8 error_index) {
	version = GetClosestVersion(version);

	if (spell_errors[version].count(error_index) == 0) {
		LogWrite(SPELL__ERROR, 0, "Spells", "No spell error entry. (version = %i, error_index = %i)", version, error_index);
		// 1 will give the client a pop up message of "Cannot cast" and a chat message of "[BUG] Cannot cast. Unknown failure casting spell."
		return 1;
	}
	return spell_errors[version][error_index];
}

void MasterSpellList::AddSpellError(int16 version, int8 error_index, int16 error_value) {
	if (spell_errors[version].count(error_index) == 0)
		spell_errors[version][error_index] = error_value;
}

int16 MasterSpellList::GetClosestVersion(int16 version) {
	int16 ret = 0;
	map<int16, map<int8, int16> >::iterator itr;
	// Get the closest version in the list that is less then or equal to the given version
	for (itr = spell_errors.begin(); itr != spell_errors.end(); itr++) {
		if (itr->first <= version) {
			if (itr->first > ret)
				ret = itr->first;
		}
	}

	return ret;
}

bool Spell::CastWhileStunned(){
	return (spell->casting_flags & CASTING_FLAG_STUNNED) == CASTING_FLAG_STUNNED;
}

bool Spell::CastWhileMezzed(){
	return (spell->casting_flags & CASTING_FLAG_MEZZED) == CASTING_FLAG_MEZZED;
}

bool Spell::CastWhileStifled(){
	return (spell->casting_flags & CASTING_FLAG_STIFLED) == CASTING_FLAG_STIFLED;
}

bool Spell::CastWhileFeared(){
	return (spell->casting_flags & CASTING_FLAG_FEARED) == CASTING_FLAG_FEARED;
}
