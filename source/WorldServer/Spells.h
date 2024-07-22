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
#ifndef __EQ2_SPELLS__
#define __EQ2_SPELLS__
#include <map>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include "../common/types.h"
#include "../common/EQPacket.h"
#include "../common/MiscFunctions.h"
#include "client.h"
#include "classes.h"
#include "../common/Mutex.h"
#include "AltAdvancement/AltAdvancement.h"

#include <LUA/lua.hpp>

#define SPELL_TARGET_SELF			0
#define SPELL_TARGET_ENEMY			1
#define SPELL_TARGET_GROUP_AE		2
#define SPELL_TARGET_CASTER_PET		3
#define SPELL_TARGET_ENEMY_PET		4
#define SPELL_TARGET_ENEMY_CORPSE	5
#define SPELL_TARGET_GROUP_CORPSE	6
#define SPELL_TARGET_NONE			7
#define SPELL_TARGET_RAID_AE		8
#define SPELL_TARGET_OTHER_GROUP_AE	9


#define SPELL_BOOK_TYPE_SPELL		0
#define SPELL_BOOK_TYPE_COMBAT_ART	1
#define SPELL_BOOK_TYPE_ABILITY		2
#define SPELL_BOOK_TYPE_TRADESKILL	3
#define SPELL_BOOK_TYPE_NOT_SHOWN	4

#define SPELL_CAST_TYPE_NORMAL			0
#define SPELL_CAST_TYPE_TOGGLE			1


#define SPELL_ERROR_NOT_ENOUGH_KNOWLEDGE					1
#define SPELL_ERROR_INTERRUPTED								2
#define SPELL_ERROR_TAKE_EFFECT_MOREPOWERFUL				3
#define SPELL_ERROR_TAKE_EFFECT_SAMESPELL					4
#define SPELL_ERROR_CANNOT_CAST_DEAD						5
#define SPELL_ERROR_NOT_ALIVE								6
#define SPELL_ERROR_NOT_DEAD								7
#define SPELL_ERROR_CANNOT_CAST_SITTING						8
#define SPELL_ERROR_CANNOT_CAST_UNCON						9
#define SPELL_ERROR_ALREADY_CASTING							10
#define SPELL_ERROR_RECOVERING								11
#define SPELL_ERROR_NON_COMBAT_ONLY							12
#define SPELL_ERROR_CANNOT_CAST_STUNNED						13
#define SPELL_ERROR_CANNOT_CAST_STIFFLED					14
#define SPELL_ERROR_CANNOT_CAST_CHARMED						15
#define SPELL_ERROR_NOT_WHILE_MOUNTED						16
#define SPELL_ERROR_NOT_WHILE_FLYING						17
#define SPELL_ERROR_NOT_WHILE_CLIMBING						18
#define SPELL_ERROR_NOT_READY								19
#define SPELL_ERROR_CANT_SEE_TARGET							20
#define SPELL_ERROR_INCORRECT_STANCE						21
#define SPELL_ERROR_CANNOT_CAST_FEIGNDEATH					22
#define SPELL_ERROR_INVENTORY_FULL							23
#define SPELL_ERROR_NOT_ENOUGH_COIN							24
#define SPELL_ERROR_NOT_ALLOWED_HERE						25
#define SPELL_ERROR_NOT_WHILE_CRAFTING						26
#define SPELL_ERROR_ONLY_WHEN_CRAFTING						27
#define SPELL_ERROR_ITEM_NOT_ATTUNED						28
#define SPELL_ERROR_ITEM_WORN_OUT							29
#define SPELL_ERROR_MUST_EQUIP_WEAPON						30
#define SPELL_ERROR_WEAPON_BROKEN							31
#define SPELL_ERROR_CANNOT_CAST_FEARED						32
#define SPELL_ERROR_TARGET_IMMUNE_HOSTILE					33
#define SPELL_ERROR_TARGET_IMMUNE_BENEFICIAL				34
#define SPELL_ERROR_NO_TAUNT_SPELLS							35
#define SPELL_ERROR_CANNOT_USE_IN_BATTLEGROUNDS				36
#define SPELL_ERROR_CANNOT_PREPARE							37
#define SPELL_ERROR_NO_ELIGIBLE_TARGET						38
#define SPELL_ERROR_NO_TARGETS_IN_RANGE						39
#define SPELL_ERROR_TOO_CLOSE								40
#define SPELL_ERROR_TOO_FAR_AWAY							41
#define SPELL_ERROR_TARGET_TOO_WEAK							42
#define SPELL_ERROR_TARGET_TOO_POWERFUL						43
#define SPELL_ERROR_WONT_WORK_ON_TARGET						44
#define SPELL_ERROR_TARGET_INVULNERABLE						45
#define SPELL_ERROR_TARGET_IMMUNE							46
#define SPELL_ERROR_TARGET_ENGAGED							47
#define SPELL_ERROR_TARGET_NOT_GROUPED						48
#define SPELL_ERROR_TARGET_IN_USE							49
#define SPELL_ERROR_TARGET_GROUP_HAS_SPELL					50
#define SPELL_ERROR_TARGET_ALREADY_ENGAGED					51
#define SPELL_ERROR_CANNOT_ENGAGE							52
#define SPELL_ERROR_NOT_A_FRIEND							53
#define SPELL_ERROR_NOT_AN_ENEMY							54
#define SPELL_ERROR_TARGET_INVENTORY_FULL					55
#define SPELL_ERROR_FINISH_DUELING_FIRST					56
#define SPELL_ERROR_ILLEGAL_TARGET_ATTACK					57
#define SPELL_ERROR_NOT_WHILE_MENTORING_PVP					58
#define SPELL_ERROR_NOT_WHILE_MENTORING_BENEFICIAL			59
#define SPELL_ERROR_ILLEGAL_TARGET_HEAL_OUTSIDE_LEVEL_RANGE	60
#define SPELL_ERROR_NOTHING_TO_CURE							61
#define SPELL_ERROR_NOT_ENOUGH_POWER						62
#define SPELL_ERROR_NOT_ENOUGH_HEALTH						63
#define SPELL_ERROR_NOT_ENOUGH_CONC							64
#define SPELL_ERROR_MISSING_COMPONENT						65
#define SPELL_ERROR_OUT_OF_CHARGES							66
#define SPELL_ERROR_LACK_AMMO								67
#define SPELL_ERROR_NO_RANGED_EQUIPPED						68
#define SPELL_ERROR_RANGED_NEEDS_REPAIR						69
#define SPELL_ERROR_LACK_WEAPON_TYPE						70
#define SPELL_ERROR_NOT_ENOUGH_SAVAGERY						71
#define SPELL_ERROR_ALREADY_PREPARED						72
#define SPELL_ERROR_ALREADY_HAVE_SPELL						73
#define SPELL_ERROR_NOT_SMART_ENOUGH						74 // "You lack the intellectual capacity to prepare another spell."
#define SPELL_ERROR_NO_HOSTILE_SPELLS						75
#define SPELL_ERROR_NO_BENEFICIAL_SPELLS					76
#define SPELL_ERROR_CANNOT_MOUNT_NOW_SITTING				77
#define SPELL_ERROR_CANNOT_MOUNT_NOW_DEAD					78
#define SPELL_ERROR_CANNOT_MOUNT_NOW_CLIMBING				79
#define SPELL_ERROR_CANNOT_MOUNT_NOW_FORM					80
#define SPELL_ERROR_CANNOT_MOUNT_NOW_WATER_TO_DEEP			81
#define SPELL_ERROR_ALREADY_CAST							82
#define SPELL_ERROR_LOTTERY_IN_PROGRESS						83
#define SPELL_ERROR_NOT_IN_PVP								84
#define SPELL_ERROR_NOT_ENOUGH_DISSONANCE                   85
#define SPELL_ERROR_NOT_CANNOT_CAST_BUG_UNKNOWN_FAILURE		86
#define SPELL_ERROR_NOT_CANNOT_CAST_BUG_SPELL_TEMPLATE		87
#define SPELL_ERROR_NOT_PREPARED_BUG						88
#define SPELL_ERROR_NOT_CANNOT_CAST_BUG_NO_GAME_WORLD		89
#define SPELL_ERROR_NOT_CANNOT_CAST_BUG_NO_OWNER			90
#define SPELL_ERROR_NOT_CANNOT_CAST_BUG_OWNER_TYPE			91
#define SPELL_ERROR_NOT_CANNOT_CAST_BUG_NO_CASTER			92
#define SPELL_ERROR_NO_RESPONSE_10							93
#define SPELL_ERROR_BUG_PARTIAL_INTERUPT					94
#define SPELL_ERROR_NO_RESPONSE_15							95
#define SPELL_ERROR_BUG_TARGET_RESISTED						96
#define SPELL_ERROR_BUG_TARGET_REFLECTED					97
#define SPELL_ERROR_NO_RESPONSE_18							98
#define SPELL_ERROR_NO_RESPONSE_35							99
#define SPELL_ERROR_BUG_UNKNOWN_43							100
#define SPELL_ERROR_BUG_UNKNOWN_44							101
#define SPELL_ERROR_BUG_UNKNOWN_47							102
#define SPELL_ERROR_TARGET_IMMUNE_HEALED_WITH_REPAIRS		103
#define SPELL_ERROR_NOT_WHILE_MENTORING						104
#define SPELL_ERROR_BUG_NO_EFFECTS_LANDED					105
#define SPELL_ERROR_TOO_MUCH_DISSONANCE						106
#define SPELL_ERROR_BUG_INVALID_SPELL_INDEX					107
#define SPELL_ERROR_CANNOT_CAST_NOT_FOUND_95				108
#define SPELL_ERROR_BUG_CONTAINMENT_TYPE					109
#define SPELL_ERROR_BUG_SLOT_FULL							110
#define SPELL_ERROR_CANNOT_CAST_NO_SPELL_101				111
#define SPELL_ERROR_RECOVERING_ITEM_ABILITY					112
#define SPELL_ERROR_NO_RESPONSE_110							113
#define SPELL_ERROR_ALREADY_CAST_ON_TARGET					114

#define CASTING_FLAG_MEZZED    1
#define CASTING_FLAG_STIFLED   2
#define CASTING_FLAG_STUNNED   4
#define CASTING_FLAG_FEARED    8

// Spell type is for AI so code knows what a spell is
#define SPELL_TYPE_UNSET		1
#define SPELL_TYPE_DD			2
#define SPELL_TYPE_DOT			3
#define SPELL_TYPE_HEAL			4
#define SPELL_TYPE_HOT_WARD		5
#define SPELL_TYPE_DEBUFF		6
#define SPELL_TYPE_BUFF			7
#define SPELL_TYPE_COMBATBUFF	8
#define SPELL_TYPE_TAUNT		9
#define SPELL_TYPE_DETAUNT		10
#define SPELL_TYPE_REZ			11
#define SPELL_TYPE_CURE			12
#define SPELL_TYPE_FOOD			13
#define SPELL_TYPE_DRINK		14
#define SPELL_TYPE_ROOT			15
#define SPELL_TYPE_SNARE		16
#define SPELL_TYPE_ALLGROUPTARGETS      17


struct LUAData{
	int8	type;
	sint32	int_value;
	bool	bool_value;
	float	float_value;
	string	string_value;
	string	string_value2;
	sint32	int_value2;
	float	float_value2;
	string	string_helper;
};
struct SpellScriptTimer {
	LuaSpell*		spell;
	string			customFunction;
	int32			time;
	int32			caster;
	int32			target;
	bool			deleteWhenDone;
};
struct LevelArray{
	int8	adventure_class;
	int8	tradeskill_class;
	int16	spell_level;
};
struct SpellDisplayEffect{
	int8	percentage;
	int8	subbullet;
	string	description;
};

enum GivenByType {
	GivenBy_Unset = 0,
	GivenBy_TradeskillClass = 1,
	GivenBy_SpellScroll = 2,
	GivenBy_AltAdvancement = 3,
	GivenBy_Race = 4,
	GivenBy_RacialInnate = 5,
	GivenBy_RacialTradition = 6,
	GivenBy_Class = 7,
	GivenBy_CharacterTrait = 8,
	GivenBy_FocusAbility = 9,
	GivenBy_ClassTraining = 10,
	GivenBy_WarderSpell = 11
};

struct SpellData{
	int32	spell_book_type;
	int32	id;
	int32	inherited_spell_id;
	sint16	icon;
	int16	icon_heroic_op;
	int16	icon_backdrop;
	int16	type;
	int32	class_skill;
	int16	min_class_skill_req;
	int32	mastery_skill;
	int8    ts_loc_index;
	int8	num_levels;
	int8	tier;
	int16	hp_req;
	int16	hp_upkeep;
	float	power_req;
	bool	power_by_level;
	int16	power_upkeep;
	int16	savagery_req;
	int16	savagery_upkeep;
	int16	dissonance_req;
	int16	dissonance_upkeep;
	int8	target_type;
	int16	cast_time;
	int16	orig_cast_time;
	float	recovery;
	float	recast;
	int32	linked_timer;
	float	radius;
	int16	max_aoe_targets;
	int8    friendly_spell;
	int16	req_concentration;
	float	range;
	int32	duration1;
	int32	duration2;
	float	resistibility;
	bool	duration_until_cancel;
	int8	power_req_percent;
	int8	hp_req_percent;
	int8	savagery_req_percent;
	int8	dissonance_req_percent;
	EQ2_8BitString name;
	EQ2_16BitString description;
	string	success_message;
	string	fade_message;
	string	fade_message_others;
	int8	cast_type;
	string	lua_script;
	int32	call_frequency;
	bool	interruptable;
	int32	spell_visual;
	string	effect_message;
	float	min_range;
	int8	can_effect_raid;
	int8	affect_only_group_members;
	int8    group_spell;
	float	hit_bonus;
	int8    display_spell_tier;
	int8	is_active;
	int8    det_type;
	bool    incurable;
	int8    control_effect_type;
	int32   casting_flags;
	bool    cast_while_moving;
	bool    persist_through_death;
	bool    not_maintained;
	bool    is_aa;
	int8	savage_bar;
	int8	savage_bar_slot;
	int32	soe_spell_crc;
	int8	spell_type;
	int32	spell_name_crc;
	sint32	type_group_spell_id;
	bool	can_fizzle;
	EQ2_8BitString given_by;
	GivenByType given_by_type;
};

class Spell{
public:
	~Spell();
	Spell();
	Spell(SpellData* in_spell);
	Spell(Spell* host_spell, bool unique_spell = true);
	EQ2Packet* SerializeSpell(Client* client, bool display, bool trait_display = false, int8 packet_type = 0, int8 sub_packet_type = 0, const char* struct_name = 0, bool send_partial_packet = false);
	EQ2Packet* SerializeSpecialSpell(Client* client, bool display, int8 packet_type = 0, int8 sub_packet_type = 0);
	EQ2Packet* SerializeAASpell(Client* client,int8 tier, AltAdvanceData* data, bool display, bool trait_display = false, int8 packet_type = 0, int8 sub_packet_type = 0, const char* struct_name = 0);
	void AddSpellLevel(int8 adventure_class, int8 tradeskill_class, int16 level);
	void AddSpellEffect(int8 percentage, int8 subbullet, string description);
	void AddSpellLuaData(int8 type, int int_value, int int_value2, float float_value, float float_value2, bool bool_value, string string_value,string string_value2, string helper);
	void AddSpellLuaDataInt(int value, int value2, string helper);
	void AddSpellLuaDataFloat(float value, float value2, string helper);
	void AddSpellLuaDataBool(bool value, string helper);
	void AddSpellLuaDataString(string value, string value2, string helper);
	int32 GetSpellID();
	sint16 TranslateClientSpellIcon(int16 version);
	void SetPacketInformation(PacketStruct* packet, Client* client = 0, bool display_tier = false);
	void SetAAPacketInformation(PacketStruct* packet, AltAdvanceData* data, Client* client = 0, bool display_tier = false);
	void AppendLevelInformation(PacketStruct* packet);
	int8 GetSpellTier();
	int32 GetSpellDuration();
	int16 GetSpellIcon();
	int16 GetSpellIconBackdrop();
	int16 GetSpellIconHeroicOp();
	int16 GetLevelRequired(Player* player);
	int16 GetHPRequired(Spawn* spawn);
	int16 GetPowerRequired(Spawn* spawn);
	int16 GetSavageryRequired(Spawn* spawn);
	int16 GetDissonanceRequired(Spawn* spawn);
	SpellData* GetSpellData();
	bool GetSpellData(lua_State* state, std::string field);
	bool SetSpellData(lua_State* state, std::string field, int8 fieldArg);
	bool ScribeAllowed(Player* player);
	vector<LUAData*>* GetLUAData();
	vector <LevelArray*>* GetSpellLevels();
	vector <SpellDisplayEffect*>* GetSpellEffects();
	const char* GetName();
	const char* GetDescription();
	bool IsHealSpell();
	bool IsBuffSpell();
	bool IsDamageSpell();
	bool IsControlSpell();
	bool IsOffenseSpell();
	bool IsCopiedSpell();
	void ModifyCastTime(Entity* caster);
	int32 CalculateRecastTimer(Entity* caster, float override_timer = 0.0f);
	bool CastWhileStunned();
	bool CastWhileMezzed();
	bool CastWhileStifled();
	bool CastWhileFeared();
	bool GetStayLocked() { return stay_locked; }
	void StayLocked(bool val) { stay_locked = val; }

	vector<SpellDisplayEffect*> effects;
	vector<LUAData*> lua_data;
	
	mutable std::shared_mutex MSpellInfo;
private:
	bool stay_locked = false;
	bool heal_spell;
	bool buff_spell;
	bool damage_spell;
	bool control_spell;
	bool offense_spell;
	bool copied_spell;

	SpellData* spell;
	
	//vector<SpellDisplayEffect*> effects;
	vector <LevelArray*> levels;
};
class MasterSpellList{
public:
	MasterSpellList();
	~MasterSpellList();
	void DestroySpells();
	map<string, Spell*> spell_name_map;
	map<int32, map<int32, Spell* > > spell_list;
	map<int32, map<int32, Spell* > > class_spell_list[MAX_CLASSES];
	map<int32, Spell*> spell_soecrc_map;
	Spell* GetSpell(int32 id, int8 tier);
	vector<Spell*>* GetSpellListByAdventureClass(int8 class_id, int16 max_level, int8 max_tier);
	vector<Spell*>* GetSpellListByTradeskillClass(int8 class_id, int16 max_level, int8 max_tier);
	Spell* GetSpellByName(const char* name);
	Spell* GetSpellByCRC(int32 spell_crc);
	void Reload();
	EQ2Packet* GetSpellPacket(int32 id, int8 tier, Client* client = 0, bool display = false, int8 packet_type = 0);
	EQ2Packet* GetAASpellPacket(int32 id, int8 group, Client* client, bool display, int8 packet_type);
	EQ2Packet* GetSpecialSpellPacket(int32 id, int8 tier, Client* client = 0, bool display = false, int8 packet_type = 0);
	void AddSpell(int32 id, int8 tier, Spell* spell);
	Mutex MMasterSpellList;
	
	/// <summary>Gets the correct spell error value for the given version</summary>
	/// <param name='version'>Client version</param>
	/// <param name='error_index'>ID of the error</param>
	/// <returns>The int16 value for the given error and version</returns>
	int16 GetSpellErrorValue(int16 version, int8 error_index);

	/// <summary>Adds a spell error to the list</summary>
	/// <param name='version'>Client version for the error</param>
	/// <param name='error_index'>ID for the error</param>
	/// <param name='error_value'>Value for the error</param>
	void AddSpellError(int16 version, int8 error_index, int16 error_value);

	int32 GetNewMaxSpellID() { 
		int32 id = 0;
		MMasterSpellList.lock();
		max_spell_id++;
		id = max_spell_id;
		MMasterSpellList.unlock();
		return id; 
	}
private:
	/// <summary>Helper function that gets the closest version in the spell_errors map that is less then or equal to the given version</summary>
	/// <param name='version'>Client version</param>
	/// <returns>int16 version that is closest to the given version</returns>
	int16 GetClosestVersion(int16 version);
	// map <version, map<error_index, error_value> >
	map<int16, map<int8, int16> > spell_errors;
	int32 max_spell_id;
};
#endif

