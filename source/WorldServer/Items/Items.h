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
#ifndef __EQ2_ITEMS__
#define __EQ2_ITEMS__
#include <map>
#include <vector>
#include <ctime>
#include "../../common/types.h"
#include "../../common/DataBuffer.h"
#include "../Commands/Commands.h"
#include "../../common/ConfigReader.h"

using namespace std;
class MasterItemList;
class Player;
class Entity;
extern MasterItemList master_item_list;

#define BASE_EQUIPMENT 		 0
#define APPEARANCE_EQUIPMENT 1
#define MAX_EQUIPMENT 2 // max iterations for equipment (base is 0, appearance is 1, so this is 2)

#define EQ2_PRIMARY_SLOT 0
#define EQ2_SECONDARY_SLOT 1
#define EQ2_HEAD_SLOT 2
#define EQ2_CHEST_SLOT 3
#define EQ2_SHOULDERS_SLOT 4
#define EQ2_FOREARMS_SLOT 5
#define EQ2_HANDS_SLOT 6
#define EQ2_LEGS_SLOT 7
#define EQ2_FEET_SLOT 8
#define EQ2_LRING_SLOT 9
#define EQ2_RRING_SLOT 10
#define EQ2_EARS_SLOT_1 11
#define EQ2_EARS_SLOT_2 12
#define EQ2_NECK_SLOT 13
#define EQ2_LWRIST_SLOT 14
#define EQ2_RWRIST_SLOT 15
#define EQ2_RANGE_SLOT 16
#define EQ2_AMMO_SLOT 17
#define EQ2_WAIST_SLOT 18
#define EQ2_CLOAK_SLOT 19
#define EQ2_CHARM_SLOT_1 20
#define EQ2_CHARM_SLOT_2 21
#define EQ2_FOOD_SLOT 22
#define EQ2_DRINK_SLOT 23
#define EQ2_TEXTURES_SLOT 24
#define EQ2_HAIR_SLOT 25
#define EQ2_BEARD_SLOT 26
#define EQ2_WINGS_SLOT 27
#define EQ2_NAKED_CHEST_SLOT 28
#define EQ2_NAKED_LEGS_SLOT 29
#define EQ2_BACK_SLOT 30
#define EQ2_ORIG_FOOD_SLOT 18
#define EQ2_ORIG_DRINK_SLOT 19
#define EQ2_DOF_FOOD_SLOT 20
#define EQ2_DOF_DRINK_SLOT 21

#define PRIMARY_SLOT 1
#define SECONDARY_SLOT 2
#define HEAD_SLOT 4
#define CHEST_SLOT 8
#define SHOULDERS_SLOT 16
#define FOREARMS_SLOT 32
#define HANDS_SLOT 64
#define LEGS_SLOT 128
#define FEET_SLOT 256
#define LRING_SLOT 512
#define RRING_SLOT 1024
#define EARS_SLOT_1 2048
#define EARS_SLOT_2 4096
#define NECK_SLOT 8192
#define LWRIST_SLOT 16384
#define RWRIST_SLOT 32768
#define RANGE_SLOT 65536
#define AMMO_SLOT 131072
#define WAIST_SLOT 262144
#define CLOAK_SLOT 524288
#define CHARM_SLOT_1 1048576
#define CHARM_SLOT_2 2097152
#define FOOD_SLOT 4194304
#define DRINK_SLOT 8388608
#define TEXTURES_SLOT 16777216
#define HAIR_SLOT 33554432
#define BEARD_SLOT 67108864
#define WINGS_SLOT 134217728
#define NAKED_CHEST_SLOT 268435456
#define NAKED_LEGS_SLOT 536870912
#define BACK_SLOT 1073741824
#define ORIG_FOOD_SLOT 524288
#define ORIG_DRINK_SLOT 1048576
#define DOF_FOOD_SLOT 1048576
#define DOF_DRINK_SLOT 2097152

#define CLASSIC_EQ_MAX_BAG_SLOTS 20
#define DOF_EQ_MAX_BAG_SLOTS 36
#define NUM_BANK_SLOTS 12
#define NUM_SHARED_BANK_SLOTS 8
#define CLASSIC_NUM_SLOTS 22
#define NUM_SLOTS 25
#define NUM_INV_SLOTS 6
#define INV_SLOT1 0
#define INV_SLOT2 50
#define INV_SLOT3 100
#define INV_SLOT4 150
#define INV_SLOT5 200
#define INV_SLOT6 250
#define BANK_SLOT1 1000
#define BANK_SLOT2 1100
#define BANK_SLOT3 1200
#define BANK_SLOT4 1300
#define BANK_SLOT5 1400
#define BANK_SLOT6 1500
#define BANK_SLOT7 1600
#define BANK_SLOT8 1700

// FLAGS
#define ATTUNED			1
#define ATTUNEABLE		2
#define ARTIFACT		4
#define LORE			8
#define TEMPORARY		16
#define NO_TRADE		32
#define NO_VALUE		64
#define NO_ZONE			128
#define NO_DESTROY		256
#define CRAFTED			512
#define GOOD_ONLY		1024
#define EVIL_ONLY		2048
#define STACK_LORE		4096  
#define LORE_EQUIP		8192  
#define NO_TRANSMUTE	16384
#define CURSED			32768

// FLAGS2
#define ORNATE			1
#define HEIRLOOM		2
#define APPEARANCE_ONLY	4
#define UNLOCKED		8
#define REFORGED		16
#define NO_REPAIR		32
#define ETHERAL			64
#define REFINED			128
#define NO_SALVAGE		256
#define INDESTRUCTABLE	512
#define NO_EXPERIMENT	1024
#define HOUSE_LORE		2048
#define FLAGS2_4096		4096//AoM: not used at this time
#define BUILDING_BLOCK	8192
#define FREE_REFORGE	16384
#define FLAGS2_32768	32768//AoM: not used at this time


#define ITEM_WIELD_TYPE_DUAL		1
#define ITEM_WIELD_TYPE_SINGLE		2
#define ITEM_WIELD_TYPE_TWO_HAND	4

#define ITEM_TYPE_NORMAL		  0
#define ITEM_TYPE_WEAPON		  1
#define ITEM_TYPE_RANGED		  2
#define ITEM_TYPE_ARMOR			  3
#define ITEM_TYPE_SHIELD		  4
#define ITEM_TYPE_BAG			  5
#define ITEM_TYPE_SKILL		      6
#define ITEM_TYPE_RECIPE		  7
#define ITEM_TYPE_FOOD			  8
#define ITEM_TYPE_BAUBLE		  9
#define ITEM_TYPE_HOUSE			  10
#define ITEM_TYPE_THROWN		  11
#define ITEM_TYPE_HOUSE_CONTAINER 12
#define ITEM_TYPE_ADORNMENT       13
#define ITEM_TYPE_GENERIC_ADORNMENT 14
#define ITEM_TYPE_PROFILE         16
#define ITEM_TYPE_PATTERN         17
#define ITEM_TYPE_ARMORSET        18
#define ITEM_TYPE_ITEMCRATE		  18
#define ITEM_TYPE_BOOK            19
#define ITEM_TYPE_DECORATION      20
#define ITEM_TYPE_DUNGEON_MAKER   21
#define ITEM_TYPE_MARKETPLACE     22


//DOV defines everything till 13 is the same
//#define ITEM_TYPE_BOOK			  13
//#define ITEM_TYPE_ADORNMENT		  14
//#define ITEM_TYPE_PATTERN		  15
//#define ITEM_TYPE_ARMORSET		  16



#define ITEM_MENU_TYPE_GENERIC			1 //0 (NON_EQUIPABLE)
#define ITEM_MENU_TYPE_EQUIP			2 //1 (This is SLOT_FULL for classic)
#define ITEM_MENU_TYPE_BAG				4//2
#define ITEM_MENU_TYPE_HOUSE			8 //3 Place
#define ITEM_MENU_TYPE_EMPTY_BAG		16	//4
#define ITEM_MENU_TYPE_SCRIBE			32//5
#define ITEM_MENU_TYPE_BANK_BAG			64//6
#define ITEM_MENU_TYPE_INSUFFICIENT_KNOWLEDGE			128//7
#define ITEM_MENU_TYPE_ACTIVATE			256//8
#define ITEM_MENU_TYPE_BROKEN			512//9
#define ITEM_MENU_TYPE_TWO_HANDED		1024//10
#define ITEM_MENU_TYPE_ATTUNED			2048//11
#define ITEM_MENU_TYPE_ATTUNEABLE		4096//12 
#define ITEM_MENU_TYPE_BOOK				8192//13
#define ITEM_MENU_TYPE_DISPLAY_CHARGES  16384//14
#define ITEM_MENU_TYPE_TEST1			32768//15 Possibly toogle decorator mode
#define ITEM_MENU_TYPE_NAMEPET		    65536 //16 Right CLick Menu
#define ITEM_MENU_TYPE_MENTORED			131072 //sets a purple background on item
#define ITEM_MENU_TYPE_CONSUME			262144//18
#define ITEM_MENU_TYPE_USE			    524288//19
#define ITEM_MENU_TYPE_CONSUME_OFF		1048576//20
#define ITEM_MENU_TYPE_TEST3			1310720// bad number combo of 2 bits
#define ITEM_MENU_TYPE_TEST4			2097152//21
#define ITEM_MENU_TYPE_TEST5			4194304//22 infusable
#define ITEM_MENU_TYPE_TEST6			8388608 //drink option on menu
#define ITEM_MENU_TYPE_TEST7			16777216//24
#define ITEM_MENU_TYPE_TEST8			33554432 // bit 25 use option in bags
#define ITEM_MENU_TYPE_TEST9			67108864//26
#define ITEM_MENU_TYPE_DAMAGED			134217728 //27
#define ITEM_MENU_TYPE_BROKEN2			268435456 //28
#define ITEM_MENU_TYPE_REDEEM	        536870912 //29 //READ??
#define ITEM_MENU_TYPE_TEST10			1073741824 //30
#define ITEM_MENU_TYPE_UNPACK			2147483648//31 * on items i found this unpack is used at same time as UNPACK below
#define ORIG_ITEM_MENU_TYPE_FOOD		2048
#define ORIG_ITEM_MENU_TYPE_DRINK		4096
#define ORIG_ITEM_MENU_TYPE_ATTUNED		8192
#define ORIG_ITEM_MENU_TYPE_ATTUNEABLE	16384
#define ORIG_ITEM_MENU_TYPE_BOOK		32768
#define ORIG_ITEM_MENU_TYPE_STACKABLE	65536
#define ORIG_ITEM_MENU_TYPE_NAMEPET	    262144

#define ITEM_MENU_TYPE2_TEST1			1 //0 auto consume on
#define ITEM_MENU_TYPE2_TEST2			2 //1
#define ITEM_MENU_TYPE2_UNPACK			4//2
#define ITEM_MENU_TYPE2_TEST4			8 //3 
#define ITEM_MENU_TYPE2_TEST5			16	//4
#define ITEM_MENU_TYPE2_TEST6			32//5
#define ITEM_MENU_TYPE2_TEST7			64//6
#define ITEM_MENU_TYPE2_TEST8			128//7
#define ITEM_MENU_TYPE2_TEST9			256//8
#define ITEM_MENU_TYPE2_TEST10			512//9
#define ITEM_MENU_TYPE2_TEST11			1024//10
#define ITEM_MENU_TYPE2_TEST12			2048//11
#define ITEM_MENU_TYPE2_TEST13			4096//12 
#define ITEM_MENU_TYPE2_TEST14				8192//13
#define ITEM_MENU_TYPE2_TEST15		 16384//14
#define ITEM_MENU_TYPE2_TEST16			32768//15

#define ITEM_TAG_COMMON					2
#define ITEM_TAG_UNCOMMON				3 //tier tags
#define ITEM_TAG_TREASURED				4
#define ITEM_TAG_LEGENDARY				7
#define ITEM_TAG_FABLED					9
#define ITEM_TAG_MYTHICAL				12

#define ITEM_BROKER_TYPE_ANY			0xFFFFFFFF
#define ITEM_BROKER_TYPE_ANY64BIT		0xFFFFFFFFFFFFFFFF
#define ITEM_BROKER_TYPE_ADORNMENT		134217728
#define ITEM_BROKER_TYPE_AMMO			1024
#define ITEM_BROKER_TYPE_ATTUNEABLE		16384
#define ITEM_BROKER_TYPE_BAG			2048
#define ITEM_BROKER_TYPE_BAUBLE			16777216
#define ITEM_BROKER_TYPE_BOOK			128
#define ITEM_BROKER_TYPE_CHAINARMOR		2097152
#define ITEM_BROKER_TYPE_CLOAK			1073741824
#define ITEM_BROKER_TYPE_CLOTHARMOR		524288
#define ITEM_BROKER_TYPE_COLLECTABLE	67108864
#define ITEM_BROKER_TYPE_CRUSHWEAPON	4
#define ITEM_BROKER_TYPE_DRINK			131072
#define ITEM_BROKER_TYPE_FOOD			4096
#define ITEM_BROKER_TYPE_HOUSEITEM		512
#define ITEM_BROKER_TYPE_JEWELRY		262144
#define ITEM_BROKER_TYPE_LEATHERARMOR	1048576
#define ITEM_BROKER_TYPE_LORE			8192
#define ITEM_BROKER_TYPE_MISC			1
#define ITEM_BROKER_TYPE_PIERCEWEAPON	8
#define ITEM_BROKER_TYPE_PLATEARMOR		4194304
#define ITEM_BROKER_TYPE_POISON			65536
#define ITEM_BROKER_TYPE_POTION			32768
#define ITEM_BROKER_TYPE_RECIPEBOOK		8388608
#define ITEM_BROKER_TYPE_SALESDISPLAY	33554432
#define ITEM_BROKER_TYPE_SHIELD			32
#define ITEM_BROKER_TYPE_SLASHWEAPON	2
#define ITEM_BROKER_TYPE_SPELLSCROLL	64
#define ITEM_BROKER_TYPE_TINKERED		268435456
#define ITEM_BROKER_TYPE_TRADESKILL		256

#define ITEM_BROKER_TYPE_2H_CRUSH		17179869184
#define ITEM_BROKER_TYPE_2H_PIERCE		34359738368
#define ITEM_BROKER_TYPE_2H_SLASH		8589934592

#define ITEM_BROKER_SLOT_ANY			0xFFFFFFFF
#define ITEM_BROKER_SLOT_AMMO			65536
#define ITEM_BROKER_SLOT_CHARM			524288
#define ITEM_BROKER_SLOT_CHEST			32
#define ITEM_BROKER_SLOT_CLOAK			262144
#define ITEM_BROKER_SLOT_DRINK			2097152
#define ITEM_BROKER_SLOT_EARS			4096
#define ITEM_BROKER_SLOT_FEET			1024
#define ITEM_BROKER_SLOT_FOOD			1048576
#define ITEM_BROKER_SLOT_FOREARMS		128
#define ITEM_BROKER_SLOT_HANDS			256
#define ITEM_BROKER_SLOT_HEAD			16
#define ITEM_BROKER_SLOT_LEGS			512
#define ITEM_BROKER_SLOT_NECK			8192
#define ITEM_BROKER_SLOT_PRIMARY		1
#define ITEM_BROKER_SLOT_PRIMARY_2H		2
#define ITEM_BROKER_SLOT_RANGE_WEAPON	32768
#define ITEM_BROKER_SLOT_RING			2048
#define ITEM_BROKER_SLOT_SECONDARY		8
#define ITEM_BROKER_SLOT_SHOULDERS		64
#define ITEM_BROKER_SLOT_WAIST			131072
#define ITEM_BROKER_SLOT_WRIST			16384

#define ITEM_BROKER_STAT_TYPE_NONE			0
#define ITEM_BROKER_STAT_TYPE_DEF			2
#define ITEM_BROKER_STAT_TYPE_STR			4
#define ITEM_BROKER_STAT_TYPE_STA			8
#define ITEM_BROKER_STAT_TYPE_AGI			16
#define ITEM_BROKER_STAT_TYPE_WIS			32
#define ITEM_BROKER_STAT_TYPE_INT			64
#define ITEM_BROKER_STAT_TYPE_HEALTH		128
#define ITEM_BROKER_STAT_TYPE_POWER			256
#define ITEM_BROKER_STAT_TYPE_HEAT			512
#define ITEM_BROKER_STAT_TYPE_COLD			1024
#define ITEM_BROKER_STAT_TYPE_MAGIC			2048
#define ITEM_BROKER_STAT_TYPE_MENTAL		4096
#define ITEM_BROKER_STAT_TYPE_DIVINE		8192
#define ITEM_BROKER_STAT_TYPE_POISON		16384
#define ITEM_BROKER_STAT_TYPE_DISEASE		32768
#define ITEM_BROKER_STAT_TYPE_CRUSH			65536
#define ITEM_BROKER_STAT_TYPE_SLASH			131072
#define ITEM_BROKER_STAT_TYPE_PIERCE		262144
#define ITEM_BROKER_STAT_TYPE_CRITICAL		524288
#define ITEM_BROKER_STAT_TYPE_DBL_ATTACK	1048576
#define ITEM_BROKER_STAT_TYPE_ABILITY_MOD	2097152
#define ITEM_BROKER_STAT_TYPE_POTENCY		4194304
#define ITEM_BROKER_STAT_TYPE_AEAUTOATTACK	8388608
#define ITEM_BROKER_STAT_TYPE_ATTACKSPEED	16777216
#define ITEM_BROKER_STAT_TYPE_BLOCKCHANCE	33554432
#define ITEM_BROKER_STAT_TYPE_CASTINGSPEED	67108864
#define ITEM_BROKER_STAT_TYPE_CRITBONUS		134217728
#define ITEM_BROKER_STAT_TYPE_CRITCHANCE	268435456
#define ITEM_BROKER_STAT_TYPE_DPS			536870912
#define ITEM_BROKER_STAT_TYPE_FLURRYCHANCE	1073741824
#define ITEM_BROKER_STAT_TYPE_HATEGAIN		2147483648
#define ITEM_BROKER_STAT_TYPE_MITIGATION	4294967296
#define ITEM_BROKER_STAT_TYPE_MULTI_ATTACK	8589934592
#define ITEM_BROKER_STAT_TYPE_RECOVERY		17179869184
#define ITEM_BROKER_STAT_TYPE_REUSE_SPEED	34359738368
#define ITEM_BROKER_STAT_TYPE_SPELL_WPNDMG	68719476736
#define ITEM_BROKER_STAT_TYPE_STRIKETHROUGH	137438953472
#define ITEM_BROKER_STAT_TYPE_TOUGHNESS		274877906944
#define ITEM_BROKER_STAT_TYPE_WEAPONDMG		549755813888


#define OVERFLOW_SLOT 0xFFFFFFFE
#define SLOT_INVALID 0xFFFF

#define ITEM_STAT_STR					0
#define ITEM_STAT_STA					1
#define ITEM_STAT_AGI					2
#define ITEM_STAT_WIS					3
#define ITEM_STAT_INT					4

#define ITEM_STAT_ADORNING				100
#define ITEM_STAT_AGGRESSION			101
#define ITEM_STAT_ARTIFICING			102
#define ITEM_STAT_ARTISTRY				103
#define ITEM_STAT_CHEMISTRY				104
#define ITEM_STAT_CRUSHING				105
#define ITEM_STAT_DEFENSE				106
#define ITEM_STAT_DEFLECTION			107
#define ITEM_STAT_DISRUPTION			108
#define ITEM_STAT_FISHING				109
#define ITEM_STAT_FLETCHING				110
#define ITEM_STAT_FOCUS					111
#define ITEM_STAT_FORESTING				112
#define ITEM_STAT_GATHERING				113
#define ITEM_STAT_METAL_SHAPING			114
#define ITEM_STAT_METALWORKING			115
#define ITEM_STAT_MINING				116
#define ITEM_STAT_MINISTRATION			117
#define ITEM_STAT_ORDINATION			118
#define ITEM_STAT_PARRY     			119
#define ITEM_STAT_PIERCING     			120
#define ITEM_STAT_RANGED     			121
#define ITEM_STAT_SAFE_FALL    			122
#define ITEM_STAT_SCRIBING     			123
#define ITEM_STAT_SCULPTING    			124
#define ITEM_STAT_SLASHING     			125
#define ITEM_STAT_SUBJUGATION  			126
#define ITEM_STAT_SWIMMING  			127
#define ITEM_STAT_TAILORING  			128
#define ITEM_STAT_TINKERING  			129
#define ITEM_STAT_TRANSMUTING  			130
#define ITEM_STAT_TRAPPING  			131
#define ITEM_STAT_WEAPON_SKILLS			132
#define ITEM_STAT_POWER_COST_REDUCTION	133
#define ITEM_STAT_SPELL_AVOIDANCE		134

#define ITEM_STAT_VS_PHYSICAL			200
#define ITEM_STAT_VS_HEAT				201 //elemental
#define ITEM_STAT_VS_POISON				202 //noxious
#define ITEM_STAT_VS_MAGIC				203 //arcane
#define ITEM_STAT_VS_DROWNING			210
#define ITEM_STAT_VS_FALLING			211
#define ITEM_STAT_VS_PAIN				212
#define ITEM_STAT_VS_MELEE				213

#define ITEM_STAT_VS_SLASH				204
#define ITEM_STAT_VS_CRUSH				205
#define ITEM_STAT_VS_PIERCE				206
//#define ITEM_STAT_VS_HEAT				203 //just so no build error
#define ITEM_STAT_VS_COLD				207
//#define ITEM_STAT_VS_MAGIC			205 //just so no build error
#define ITEM_STAT_VS_MENTAL				208
#define ITEM_STAT_VS_DIVINE				209
#define ITEM_STAT_VS_DISEASE			214
//#define ITEM_STAT_VS_POISON			209 //just so no build error
//#define ITEM_STAT_VS_DROWNING			210 //just so no build error
//#define ITEM_STAT_VS_FALLING			211 //just so no build error
//#define ITEM_STAT_VS_PAIN				212 //just so no build error
//#define ITEM_STAT_VS_MELEE			213 //just so no build error

#define ITEM_STAT_DMG_SLASH				300
#define ITEM_STAT_DMG_CRUSH				301
#define ITEM_STAT_DMG_PIERCE			302
#define ITEM_STAT_DMG_HEAT				303
#define ITEM_STAT_DMG_COLD 				304
#define ITEM_STAT_DMG_MAGIC				305
#define ITEM_STAT_DMG_MENTAL			306
#define ITEM_STAT_DMG_DIVINE			307
#define ITEM_STAT_DMG_DISEASE			308
#define ITEM_STAT_DMG_POISON			309
#define ITEM_STAT_DMG_DROWNING			310
#define ITEM_STAT_DMG_FALLING			311
#define ITEM_STAT_DMG_PAIN				312
#define ITEM_STAT_DMG_MELEE				313

#define ITEM_STAT_DEFLECTIONCHANCE 		400 //just so no build error

#define ITEM_STAT_HEALTH				500
#define ITEM_STAT_POWER					501
#define ITEM_STAT_CONCENTRATION			502
#define ITEM_STAT_SAVAGERY				503

//this is the master stat list you should be using and names match what is in census. it is based off of DoV. the comment is what is displayed on items when examining
//the itemstats table will maintain the custom lists per expansion
// emu # is digits after the 6

#define ITEM_STAT_HPREGEN				600 //Health Regeneration
#define ITEM_STAT_MANAREGEN				601 //Power Regeneration
#define ITEM_STAT_HPREGENPPT			602 //Out-of-Combat Health Regeneration  %%?
#define ITEM_STAT_MPREGENPPT			603 //Out-of-Combat Power Regeneration   %%?
#define ITEM_STAT_COMBATHPREGENPPT		604 //In-Combat Health Regeneration       %%?
#define ITEM_STAT_COMBATMPREGENPPT		605 //In-Combat Power Regeneration        %%?
#define ITEM_STAT_MAXHP					606 //Max Health
#define ITEM_STAT_MAXHPPERC				607 
#define ITEM_STAT_MAXHPPERCFINAL		608 //% Max Mealth
#define ITEM_STAT_SPEED					609 //Out of Combat Run Speed
#define ITEM_STAT_SLOW					610 //Slow
#define ITEM_STAT_MOUNTSPEED			611 //Ground Mount Speed
#define ITEM_STAT_MOUNTAIRSPEED			612 //Mount Air Speed
#define ITEM_STAT_LEAPSPEED				613 
#define ITEM_STAT_LEAPTIME				614
#define ITEM_STAT_GLIDEEFFICIENCY		615
#define ITEM_STAT_OFFENSIVESPEED		616	//In Combat Run Speed
#define ITEM_STAT_ATTACKSPEED			617 //% Attack Speed
#define ITEM_STAT_SPELLWEAPONATTACKSPEED 618
#define ITEM_STAT_MAXMANA				619 //Max Power
#define ITEM_STAT_MAXMANAPERC			620 //% Max Power
#define ITEM_STAT_MAXATTPERC			621 //All Attributes    //is this a percent or is it a stat change
#define ITEM_STAT_BLURVISION			622 //Blurs Vision
#define ITEM_STAT_MAGICLEVELIMMUNITY	623 //Magic Level Immunity
#define ITEM_STAT_HATEGAINMOD			624 //% Hate Gain
#define ITEM_STAT_COMBATEXPMOD			625 //Combat XP Gain
#define ITEM_STAT_TRADESKILLEXPMOD		626 //Tradeskill XP Gain
#define ITEM_STAT_ACHIEVEMENTEXPMOD		627 //AA XP Gain
#define ITEM_STAT_SIZEMOD				628 //Size
#define ITEM_STAT_DPS					629 //%Damage Per Second
#define ITEM_STAT_SPELLWEAPONDPS		630 //%Damage Per Second
#define ITEM_STAT_STEALTH				631 //Stealth
#define ITEM_STAT_INVIS					632 //Invisibility
#define ITEM_STAT_SEESTEALTH			633 //See Stealth
#define ITEM_STAT_SEEINVIS				634 //See Invisible 
#define ITEM_STAT_EFFECTIVELEVELMOD		635 //Effective Level
#define ITEM_STAT_RIPOSTECHANCE			636 //%Extra Riposte Chance
#define ITEM_STAT_PARRYCHANCE			637 //%Extra Parry Chance
#define ITEM_STAT_DODGECHANCE			638 //%Extra Dodge Chance
#define ITEM_STAT_AEAUTOATTACKCHANCE	639 //% AE Autoattck Chance
#define ITEM_STAT_SPELLWEAPONAEAUTOATTACKCHANCE		640 //
#define ITEM_STAT_MULTIATTACKCHANCE					641 //% Multi Attack Chance  // inconsistant with db
#define ITEM_STAT_PVPDOUBLEATTACKCHANCE				642
#define ITEM_STAT_SPELLWEAPONDOUBLEATTACKCHANCE		643 // missing in db
#define ITEM_STAT_PVPSPELLWEAPONDOUBLEATTACKCHANCE  644
#define ITEM_STAT_SPELLMULTIATTACKCHANCE			645 //% Spell Multi Atttack Chance
#define ITEM_STAT_PVPSPELLDOUBLEATTACKCHANCE		646
#define ITEM_STAT_FLURRY					647 //%Flurry
#define ITEM_STAT_SPELLWEAPONFLURRY			648
#define ITEM_STAT_MELEEDAMAGEMULTIPLIER		649 //Melee Damage Multiplier
#define ITEM_STAT_EXTRAHARVESTCHANCE		650 //Extra Harvest Chance
#define ITEM_STAT_EXTRASHIELDBLOCKCHANCE	651 //Block Chance
#define ITEM_STAT_ITEMHPREGENPPT	  		652 //In-Combat Health Regeneration
#define ITEM_STAT_ITEMPPREGENPPT			653 //In-Combat Power Regeneration
#define ITEM_STAT_MELEECRITCHANCE			654 //% Crit Chance
#define ITEM_STAT_CRITAVOIDANCE				655 //% Crit Avoidance
#define ITEM_STAT_BENEFICIALCRITCHANCE		656 //% Beneficial Crit Chance
#define ITEM_STAT_CRITBONUS					657 //% Crit Bonus
#define ITEM_STAT_PVPCRITBONUS				658 
#define ITEM_STAT_POTENCY				    659 //% Potency
#define ITEM_STAT_PVPPOTENCY				660 
#define ITEM_STAT_UNCONSCIOUSHPMOD			661 //Unconcious Health
#define ITEM_STAT_ABILITYREUSESPEED			662 //% Ability Reuse Speed
#define ITEM_STAT_ABILITYRECOVERYSPEED		663 //% Ability Recovery Speed
#define ITEM_STAT_ABILITYCASTINGSPEED		664 //% Ability Casting Speed
#define ITEM_STAT_SPELLREUSESPEED			665 //% Spell Reuse Speed
#define ITEM_STAT_MELEEWEAPONRANGE			666 //% Melee Weapon Range Increase
#define ITEM_STAT_RANGEDWEAPONRANGE			667 //% Ranged Weapon Range Increase
#define ITEM_STAT_FALLINGDAMAGEREDUCTION	668 //Fallling Damage Reduction
#define ITEM_STAT_RIPOSTEDAMAGE				669 //% Riposte Damage
#define ITEM_STAT_MINIMUMDEFLECTIONCHANCE	670 //% Minimum Block Chance
#define ITEM_STAT_MOVEMENTWEAVE				671 //Movement Weave
#define ITEM_STAT_COMBATHPREGEN				672 //Combat HP Regen
#define ITEM_STAT_COMBATMANAREGEN			673 //Combat Mana Regen
#define ITEM_STAT_CONTESTSPEEDBOOST			674 //Contest Only Speed
#define ITEM_STAT_TRACKINGAVOIDANCE			675 //Tracking avoidance
#define ITEM_STAT_STEALTHINVISSPEEDMOD		676 //Movement Bonus whie Stealthed or Invisible
#define ITEM_STAT_LOOT_COIN					677 //Loot Coin
#define ITEM_STAT_ARMORMITIGATIONINCREASE   678 //% Mitigation Increase
#define ITEM_STAT_AMMOCONSERVATION			679 // Ammo Conservation
#define ITEM_STAT_STRIKETHROUGH				680 //Strikethrough
#define ITEM_STAT_STATUSBONUS				681 //Status Bonus
#define ITEM_STAT_ACCURACY					682 //% Accuracy
#define ITEM_STAT_COUNTERSTRIKE				683  //CounterStrike
#define ITEM_STAT_SHIELDBASH				684 //Shield Bash
#define ITEM_STAT_WEAPONDAMAGEBONUS			685 //Weapon Damage Bonus
#define ITEM_STAT_WEAPONDAMAGEBONUSMELEEONLY  686 //additional chance to Riposte
#define ITEM_STAT_ADDITIONALRIPOSTECHANCE     687 //additional chance to Riposte
#define ITEM_STAT_CRITICALMITIGATION		688 //Critical Mitigation
#define ITEM_STAT_PVPTOUGHNESS				689 //Toughness
#define ITEM_STAT_PVPLETHALITY				690 //
#define ITEM_STAT_STAMINABONUS				691 //Stamina Bonus
#define ITEM_STAT_WISDOMMITBONUS			692 //Wisdom Mitigation Bonus
#define ITEM_STAT_HEALRECEIVE				693 //Applied Heals
#define ITEM_STAT_HEALRECEIVEPERC			694 //% Applied Heals
#define ITEM_STAT_PVPCRITICALMITIGATION		695 //PvP Critical Mitigation
#define ITEM_STAT_BASEAVOIDANCEBONUS		696
#define ITEM_STAT_INCOMBATSAVAGERYREGEN		697
#define ITEM_STAT_OUTOFCOMBATSAVAGERYREGEN  698
#define ITEM_STAT_SAVAGERYREGEN				699
#define ITEM_STAT_SAVAGERYGAINMOD			6100
#define ITEM_STAT_MAXSAVAGERYLEVEL			6101
#define ITEM_STAT_SPELLWEAPONDAMAGEBONUS	6102
#define ITEM_STAT_INCOMBATDISSONANCEREGEN	6103
#define ITEM_STAT_OUTOFCOMBATDISSONANCEREGEN 6104
#define ITEM_STAT_DISSONANCEREGEN			6105
#define ITEM_STAT_DISSONANCEGAINMOD			6106	
#define ITEM_STAT_AEAUTOATTACKAVOID			6107
#define ITEM_STAT_AGNOSTICDAMAGEBONUS		6108
#define ITEM_STAT_AGNOSTICHEALBONUS			6109	
#define ITEM_STAT_TITHEGAIN					6110
#define ITEM_STAT_FERVER					6111
#define ITEM_STAT_RESOLVE					6112
#define ITEM_STAT_COMBATMITIGATION			6113
#define ITEM_STAT_ABILITYMITIGATION			6114
#define ITEM_STAT_MULTIATTACKAVOIDANCE		6115
#define ITEM_STAT_DOUBLECASTAVOIDANCE		6116
#define ITEM_STAT_ABILITYDOUBLECASTAVOIDANCE 6117
#define ITEM_STAT_DAMAGEPERSECONDMITIGATION	6118
#define ITEM_STAT_FERVERMITIGATION			6119
#define ITEM_STAT_FLURRYAVOIDANCE			6120
#define ITEM_STAT_WEAPONDAMAGEBONUSMITIGATION	6121
#define ITEM_STAT_ABILITYDOUBLECASTCHANCE	6122	
#define ITEM_STAT_ABILITYMODIFIERMITIGATATION	6123
#define ITEM_STAT_STATUSEARNED				6124




#define ITEM_STAT_SPELL_DAMAGE			700
#define ITEM_STAT_HEAL_AMOUNT			701
#define ITEM_STAT_SPELL_AND_HEAL		702
#define ITEM_STAT_COMBAT_ART_DAMAGE		703
#define ITEM_STAT_SPELL_AND_COMBAT_ART_DAMAGE			704
#define ITEM_STAT_TAUNT_AMOUNT			705
#define ITEM_STAT_TAUNT_AND_COMBAT_ART_DAMAGE			706
#define ITEM_STAT_ABILITY_MODIFIER			707

// Other stats not listed above (not sent from the server), never send these to the client
// using type 8 as it is not used by the client as far as we know
#define ITEM_STAT_DURABILITY_MOD		800
#define ITEM_STAT_DURABILITY_ADD		801
#define ITEM_STAT_PROGRESS_ADD			802
#define ITEM_STAT_PROGRESS_MOD			803
#define ITEM_STAT_SUCCESS_MOD			804
#define ITEM_STAT_CRIT_SUCCESS_MOD		805
#define ITEM_STAT_EX_DURABILITY_MOD		806
#define ITEM_STAT_EX_DURABILITY_ADD		807
#define ITEM_STAT_EX_PROGRESS_MOD		808
#define ITEM_STAT_EX_PROGRESS_ADD		809
#define ITEM_STAT_EX_SUCCESS_MOD		810
#define ITEM_STAT_EX_CRIT_SUCCESS_MOD	811
#define ITEM_STAT_EX_CRIT_FAILURE_MOD	812
#define ITEM_STAT_RARE_HARVEST_CHANCE	813
#define ITEM_STAT_MAX_CRAFTING			814
#define ITEM_STAT_COMPONENT_REFUND		815
#define ITEM_STAT_BOUNTIFUL_HARVEST		816

#define ITEM_STAT_UNCONTESTED_PARRY     850
#define ITEM_STAT_UNCONTESTED_BLOCK     851
#define ITEM_STAT_UNCONTESTED_DODGE     852
#define ITEM_STAT_UNCONTESTED_RIPOSTE     853

#define	DISPLAY_FLAG_RED_TEXT			1 // old clients
#define	DISPLAY_FLAG_NO_GUILD_STATUS	8
#define	DISPLAY_FLAG_NO_BUYBACK			16
#define	DISPLAY_FLAG_NOT_FOR_SALE		64
#define	DISPLAY_FLAG_NO_BUY				128 // disables buying on merchant 'buy' list

enum ItemEffectType {
	NO_EFFECT_TYPE=0,
	EFFECT_CURE_TYPE_TRAUMA=1,
	EFFECT_CURE_TYPE_ARCANE=2,
	EFFECT_CURE_TYPE_NOXIOUS=3,
	EFFECT_CURE_TYPE_ELEMENTAL=4,
	EFFECT_CURE_TYPE_CURSE=5,
	EFFECT_CURE_TYPE_MAGIC=6,
	EFFECT_CURE_TYPE_ALL=7
};
#pragma pack(1)
struct ItemStatsValues{
	sint16			str;
	sint16			sta;
	sint16			agi;
	sint16			wis;
	sint16			int_;
	sint16			vs_slash;
	sint16			vs_crush;
	sint16			vs_pierce;
	sint16			vs_physical;
	sint16			vs_heat;
	sint16			vs_cold;
	sint16			vs_magic;
	sint16			vs_mental;
	sint16			vs_divine;
	sint16			vs_disease;
	sint16			vs_poison;
	sint16			health;
	sint16			power;
	sint8			concentration;
	sint16			ability_modifier;
	sint16			criticalmitigation;
	sint16			extrashieldblockchance;
	sint16			beneficialcritchance;
	sint16			critbonus;
	sint16			potency;
	sint16			hategainmod;
	sint16			abilityreusespeed;
	sint16			abilitycastingspeed;
	sint16			abilityrecoveryspeed;
	sint16			spellreusespeed;
	sint16			spellmultiattackchance;
	sint16			dps;
	sint16			attackspeed;
	sint16			multiattackchance;
	sint16			flurry;
	sint16			aeautoattackchance;
	sint16			strikethrough;
	sint16			accuracy;
	sint16			offensivespeed;
	float			uncontested_parry;
	float			uncontested_block;
	float			uncontested_dodge;
	float			uncontested_riposte;


};
struct ItemCore{
	int32	item_id;
	sint32	soe_id;
	int32	bag_id;
	sint32	inv_slot_id;
	sint16	slot_id;
	sint16	equip_slot_id; // used for when a bag is equipped
	sint16	appearance_type; // 0 for combat armor, 1 for appearance armor
	int8	index;
	int16	icon;
	int16	classic_icon;
	int16	count;
	int8	tier;
	int8	num_slots;
	int32	unique_id;
	int8	num_free_slots;
	int16	recommended_level;
	bool	item_locked;
	bool	new_item;
	int16	new_index;
};
#pragma pack()
struct ItemStat{
	string					stat_name;
	int8					stat_type;
	sint16					stat_subtype;
	int16					stat_type_combined;
	float					value;
	int8					level;
};
struct ItemSet{
	int32					item_id;
	int32					item_crc;
	int16					item_icon;
	int16					item_stack_size;
	int32					item_list_color;
	std::string				name;
	int8					language;
};
struct Classifications{
	int32					classification_id;  //classifications MJ
	string					classification_name;
};
struct ItemLevelOverride{
	int8					adventure_class;
	int8					tradeskill_class;
	int16					level;
};
struct ItemClass{
	int8					adventure_class;
	int8					tradeskill_class;
	int16					level;
};
struct ItemAppearance{
	int16					type;
	int8					red;
	int8					green;
	int8					blue;
	int8					highlight_red;
	int8					highlight_green;
	int8					highlight_blue;
};

enum AddItemType {
	NOT_SET = 0,
	BUY_FROM_BROKER = 1,
	GM_COMMAND = 2
};

struct QuestRewardData {
	int32 quest_id;
	bool is_temporary;
	std::string description;
	bool is_collection;
	bool has_displayed;
	int64 tmp_coin;
	int32 tmp_status;
	bool db_saved;
	int32 db_index;
};

class PlayerItemList;
class Item{
public:
	#pragma pack(1)
	struct ItemStatString{
		EQ2_8BitString			stat_string;
	};
	struct Generic_Info{
		int8					show_name;
		int8					creator_flag;
		int16					item_flags;
		int16					item_flags2;
		int8					condition;
		int32					weight; // num/10
		int32					skill_req1;
		int32					skill_req2;
		int16					skill_min;
		int8					item_type; //0=normal, 1=weapon, 2=range, 3=armor, 4=shield, 5=bag, 6=scroll, 7=recipe, 8=food, 9=bauble, 10=house item, 11=thrown, 12=house container, 13=adormnet, 14=??, 16=profile, 17=patter set, 18=item set, 19=book, 20=decoration, 21=dungeon maker, 22=marketplace
		int16					appearance_id;
		int8					appearance_red;
		int8					appearance_green;
		int8					appearance_blue;
		int8					appearance_highlight_red;
		int8					appearance_highlight_green;
		int8					appearance_highlight_blue;
		int8					collectable;
		int32					offers_quest_id;
		int32					part_of_quest_id;
		int16					max_charges;
		int8					display_charges;
		int64					adventure_classes;
		int64					tradeskill_classes;
		int16					adventure_default_level;
		int16					tradeskill_default_level;
		int8					usable;
		int8					harvest;
		int8					body_drop;
		int8					pvp_description;
		int8					merc_only;
		int8					mount_only;
		int32					set_id;
		int8					collectable_unk;
		char					offers_quest_name[255];
		char					required_by_quest_name[255];
		int8					transmuted_material;
	};
	struct Armor_Info {
		int16					mitigation_low;
		int16					mitigation_high;
	};
	struct Adornment_Info {
		float					duration;
		int16					item_types;
		int16					slot_type;
	};
	struct Weapon_Info {
		int16					wield_type;
		int16					damage_low1;
		int16					damage_high1;
		int16					damage_low2;
		int16					damage_high2;
		int16					damage_low3;
		int16					damage_high3;
		int16					delay;
		float					rating;
	};
	struct Shield_Info {
		Armor_Info				armor_info;
	};
	struct Ranged_Info {
		Weapon_Info				weapon_info;
		int16					range_low;
		int16					range_high;
	};
	struct Bag_Info {
		int8					num_slots;
		int16					weight_reduction;
	};
	struct Food_Info{
		int8					type; //0=water, 1=food
		int8					level;
		float					duration;
		int8					satiation;
	};
	struct Bauble_Info{
		int16					cast;
		int16					recovery;
		int32					duration;
		float					recast;
		int8					display_slot_optional;
		int8					display_cast_time;
		int8					display_bauble_type;
		float					effect_radius;
		int32					max_aoe_targets;
		int8					display_until_cancelled;
	};
	struct Book_Info{
		int8					language;
		EQ2_16BitString			author;
		EQ2_16BitString			title;
	};
	struct Book_Info_Pages {
		int8					page;
		EQ2_16BitString			page_text;
		int8					page_text_valign;
		int8					page_text_halign;
	};
	struct Skill_Info{
		int32					spell_id;
		int32					spell_tier;
	};
	struct HouseItem_Info{
		int32					status_rent_reduction;
		float					coin_rent_reduction;
		int8					house_only;
		int8					house_location; // 0 = floor, 1 = ceiling, 2 = wall
	};
	struct HouseContainer_Info{
		int64                   allowed_types;
		int8                    num_slots;
		int8                    broker_commission;
		int8                    fence_commission;
	};
	struct RecipeBook_Info{
		vector<uint32>			recipes;
		int32					recipe_id;
		int8					uses;
	};
	struct ItemSet_Info{
		int32					item_id;
		int32					item_crc;
		int16					item_icon;
		int32					item_stack_size;
		int32					item_list_color;
		int32					soe_item_id_unsigned;
		int32					soe_item_crc_unsigned;
	};
	struct Thrown_Info{
		sint32					range;
		sint32					damage_modifier;
		float					hit_bonus;
		int32					damage_type;
	};
	struct ItemEffect{
		EQ2_16BitString			effect;
		int8					percentage;
		int8					subbulletflag;
	};
	struct BookPage {
		int8					page;
		EQ2_16BitString			page_text;
		int8					valign;
		int8					halign;
	};
	#pragma pack()
	Item();
	Item(Item* in_item);
	~Item();
	string					lowername;
	string					name;
	string					description;
	int16					stack_count;
	int32					sell_price;
	int32					sell_status;
	int32					max_sell_value;
	bool					save_needed;
	int8					weapon_type;
	string					adornment;
	string					creator;
	int32					adorn0;
	int32					adorn1;
	int32					adorn2;
	vector<Classifications*>classifications;  //classifications MJ
	vector<ItemStat*>		item_stats;
	vector<ItemSet*>		item_sets;
	vector<ItemStatString*>	item_string_stats;
	vector<ItemLevelOverride*> item_level_overrides;
	vector<ItemEffect*>		item_effects;
	vector<BookPage*>		book_pages;
	Generic_Info			generic_info;
	Weapon_Info*			weapon_info;
	Ranged_Info*			ranged_info;
	Armor_Info*				armor_info;
	Adornment_Info*			adornment_info;
	Bag_Info*				bag_info;
	Food_Info*				food_info;
	Bauble_Info*			bauble_info;
	Book_Info*				book_info;
	Book_Info_Pages*		book_info_pages;
	HouseItem_Info*			houseitem_info;
	HouseContainer_Info*    housecontainer_info;
	Skill_Info*				skill_info;
	RecipeBook_Info*		recipebook_info;
	ItemSet_Info*			itemset_info;
	Thrown_Info*			thrown_info;
	vector<int8>			slot_data;
	ItemCore				details;
	int32					spell_id;
	int8					spell_tier;
	string					item_script;
	bool					no_buy_back;
	bool					no_sale;
	bool 					needs_deletion;
	std::time_t				created;
	std::map<int32, bool>	grouped_char_ids;
	ItemEffectType			effect_type;
	bool 					crafted;
	bool					tinkered;
	int8					book_language;
	
	void AddEffect(string effect, int8 percentage, int8 subbulletflag);
	void AddBookPage(int8 page, string page_text,int8 valign, int8 halign);
	int32 GetMaxSellValue();
	void SetMaxSellValue(int32 val);
	void SetItem(Item* old_item);
	int16 GetOverrideLevel(int8 adventure_class, int8 tradeskill_class);
	void AddLevelOverride(int8 adventure_class, int8 tradeskill_class, int16 level);
	void AddLevelOverride(ItemLevelOverride* class_);
	bool CheckClassLevel(int8 adventure_class, int8 tradeskill_class, int16 level);
	bool CheckClass(int8 adventure_class, int8 tradeskill_class);
	bool CheckArchetypeAdvClass(int8 adventure_class, map<int8, int16>* adv_class_levels = 0);
	bool CheckArchetypeAdvSubclass(int8 adventure_class, map<int8, int16>* adv_class_levels = 0);
	bool CheckLevel(int8 adventure_class, int8 tradeskill_class, int16 level);
	void SetAppearance(int16 type, int8 red, int8 green, int8 blue, int8 highlight_red, int8 highlight_green, int8 highlight_blue);
	void SetAppearance(ItemAppearance* appearance);
	void AddStat(ItemStat* in_stat);
	bool HasStat(uint32 statID, std::string statName = std::string(""));
	void DeleteItemSets();
	void AddSet(ItemSet* in_set);
	void AddStatString(ItemStatString* in_stat);
	void AddStat(int8 type, int16 subtype, float value, int8 level, char* name = 0);
	void AddSet(int32 item_id, int32 item_crc, int16 item_icon, int32 item_stack_size, int32 item_list_color, std::string name, int8 language);
	void SetWeaponType(int8 type);
	int8 GetWeaponType();
	bool HasSlot(int8 slot, int8 slot2 = 255);
	bool HasAdorn0();
	bool HasAdorn1();
	bool HasAdorn2();
	bool IsNormal();
	bool IsWeapon();
	bool IsArmor();
	bool IsDualWieldAble(Client* client, Item* item, int8 slot = -1);
	bool IsRanged();
	bool IsBag();
	bool IsFood();
	bool IsBauble();
	bool IsSkill();
	bool IsHouseItem();
	bool IsHouseContainer();
	bool IsShield();
	bool IsAdornment();
	bool IsAmmo();
	bool IsBook();
	bool IsChainArmor();
	bool IsClothArmor();
	bool IsCollectable();
	bool IsCloak();
	bool IsCrushWeapon();
	bool IsFoodFood();
	bool IsFoodDrink();
	bool IsJewelry();
	bool IsLeatherArmor();
	bool IsMisc();
	bool IsPierceWeapon();
	bool IsPlateArmor();
	bool IsPoison();
	bool IsPotion();
	bool IsRecipeBook();
	bool IsSalesDisplay();
	bool IsSlashWeapon();
	bool IsSpellScroll();
	bool IsTinkered();
	bool IsTradeskill();
	bool IsThrown();
	bool IsHarvest();
	bool IsBodyDrop();
	void SetItemScript(string name);
	const char*	GetItemScript();
	int32 CalculateRepairCost();
	string CreateItemLink(int16 client_Version, bool bUseUniqueID=false);

	void SetItemType(int8 in_type);
	void serialize(PacketStruct* packet, bool show_name = false, Player* player = 0, int16 packet_type = 0, int8 subtype = 0, bool loot_item = false, bool inspect = false);
	EQ2Packet* serialize(int16 version, bool show_name = false, Player* player = 0, bool include_twice = true, int16 packet_type = 0, int8 subtype = 0, bool merchant_item = false, bool loot_item = false, bool inspect = false);
	PacketStruct* PrepareItem(int16 version, bool merchant_item = false, bool loot_item = false, bool inspection = false);
	bool CheckFlag(int32 flag);
	bool CheckFlag2(int32 flag);
	void AddSlot(int8 slot_id);
	void SetSlots(int32 slots);
	int16 GetIcon(int16 version);
};
class MasterItemList{
public:
	MasterItemList();
	~MasterItemList();
	map<int32,Item*> items;

	Item* GetItem(int32 id);
	Item* GetItemByName(const char *name);
	Item* GetAllItemsByClassification(const char* name);
	ItemStatsValues* CalculateItemBonuses(int32 item_id, Entity* entity = 0);
	ItemStatsValues* CalculateItemBonuses(Item* desc, Entity* entity = 0, ItemStatsValues* values = 0);
	vector<Item*>* GetItems(string name, int64 itype, int64 ltype, int64 btype, int64 minprice, int64 maxprice, int8 minskill, int8 maxskill, string seller, string adornment, int8 mintier, int8 maxtier, int16 minlevel, int16 maxlevel, sint8 itemclass);
	vector<Item*>* GetItems(map<string, string> criteria, Client* client_to_map);
	void AddItem(Item* item);
	bool IsBag(int32 item_id);
	void RemoveAll();
	static int32 NextUniqueID();
	static void ResetUniqueID(int32 new_id);
	static int32 next_unique_id;
	int32 GetItemStatIDByName(std::string name);
	std::string GetItemStatNameByID(int32 id);
	void AddMappedItemStat(int32 id, std::string lower_case_name);
	

	void AddBrokerItemMapRange(int32 min_version, int32 max_version, int64 client_bitmask, int64 server_bitmask);
	map<VersionRange*, map<int64,int64>>::iterator FindBrokerItemMapVersionRange(int32 min_version, int32 max_version);
	map<VersionRange*, map<int64,int64>>::iterator FindBrokerItemMapByVersion(int32 version);

	map<std::string, int32> mappedItemStatsStrings; 
	map<int32, std::string> mappedItemStatTypeIDs; 
	std::map<VersionRange*, std::map<int64,int64>> broker_item_map;
};
class PlayerItemList {
public:
	PlayerItemList();
	~PlayerItemList();
//	int16 number;
	int32 max_saved_index;
	map<int32, Item*> indexed_items;
	map<sint32, map<int8, map<int16, Item*>> > items;
//	map< int8, Item* > inv_items;
//	map< int8, Item* > bank_items;
	int32 SetMaxItemIndex();
	bool  SharedBankAddAllowed(Item* item);
	vector<Item*>* GetItemsFromBagID(sint32 bag_id);
	vector<Item*>* GetItemsInBag(Item* bag);
	Item* GetBag(int8 inventory_slot, bool lock = true);
	bool  HasItem(int32 id, bool include_bank = false);
	Item* GetItemFromIndex(int32 index);
	void  MoveItem(Item* item, sint32 inv_slot, int16 slot, int8 appearance_type, bool erase_old); // erase old was true
	bool  MoveItem(sint32 to_bag_id, int16 from_index, sint8 to, int8 appearance_type, int8 charges);
	void  EraseItem(Item* item);
	Item* GetItemFromUniqueID(int32 item_id, bool include_bank = false, bool lock = true);
	Item* GetItemFromID(int32 item_id, int8 count = 0, bool include_bank = false, bool lock = true);
	sint32 GetAllStackCountItemFromID(int32 item_id, int8 count = 0, bool include_bank = false, bool lock = true);
	bool  AssignItemToFreeSlot(Item* item);
	int16 GetNumberOfFreeSlots();
	int16 GetNumberOfItems();
	int32 GetWeight();
	bool  HasFreeSlot();
	bool  HasFreeBagSlot();
	void DestroyItem(int16 index);
	Item* CanStack(Item* item, bool include_bank = false);
	vector<Item*> GetAllItemsFromID(int32 item, bool include_bank = false, bool lock = false);
	void RemoveItem(Item* item, bool delete_item = false);
	bool AddItem(Item* item);

	Item* GetItem(sint32 bag_slot, int16 slot, int8 appearance_type = 0);
	
	EQ2Packet*	serialize(Player* player, int16 version);
	uchar* xor_packet;
	uchar* orig_packet;
	map<int32, Item*>* GetAllItems();
	bool HasFreeBankSlot();
	int8 FindFreeBankSlot();

	///<summary>Get the first free slot and store them in the provided variables</summary>
	///<param name='bag_id'>Will contain the bag id of the first free spot</param>
	///<param name='slot'>Will contain the slot id of the first free slot</param>
	///<returns>True if a free slot was found</returns>
	bool GetFirstFreeSlot(sint32* bag_id, sint16* slot);

	/// <summary>Get the first free slot in the bank and store it in the provided variables
	/// <param name='bag_id'>Will contain the bag id of the first free bank slot</param>
	/// <param name='slot'>Will contain the slot id of the first free bank slot</param>
	/// <returns>True if a free bank slot was found</returns>
	bool GetFirstFreeBankSlot(sint32* bag_id, sint16* slot);

	/// <summary></summary>
	Item* GetBankBag(int8 inventory_slot, bool lock = true);

	/// <summary></summary>
	bool AddOverflowItem(Item* item);

	Item* GetOverflowItem();

	void RemoveOverflowItem(Item* item);

	vector<Item*>* GetOverflowItemList();
	
	void	ResetPackets();

	int32	CheckSlotConflict(Item* tmp, bool check_lore_only = false, bool lock_mutex = true, int16* lore_stack_count = 0);
	
	int32   GetItemCountInBag(Item* bag);

	int16	GetFirstNewItem();
	int16	GetNewItemByIndex(int16 in_index);
	
	Mutex MPlayerItems;
private:
	void AddItemToPacket(PacketStruct* packet, Player* player, Item* item, int16 i, bool overflow = false, int16 new_index = 0);
	void Stack(Item* orig_item, Item* item);
	int16 packet_count;
	vector<Item*> overflowItems;
};

class EquipmentItemList{
public:
	EquipmentItemList();
	EquipmentItemList(const EquipmentItemList& list);
	~EquipmentItemList();
	Item* items[NUM_SLOTS];
	Mutex MEquipmentItems;

	vector<Item*>* GetAllEquippedItems();

	void	ResetPackets();

	bool	HasItem(int32 id);
	int8	GetNumberOfItems();
	int32	GetWeight();
	Item*	GetItemFromUniqueID(int32 item_id);
	Item*	GetItemFromItemID(int32 item_id);
	void	SetItem(int8 slot_id, Item* item, bool locked = false);
	void	RemoveItem(int8 slot, bool delete_item = false);
	Item*	GetItem(int8 slot_id);
	bool	AddItem(int8 slot, Item* item);
	bool	CheckEquipSlot(Item* tmp, int8 slot);
	bool	CanItemBeEquippedInSlot(Item* tmp, int8 slot);
	int8	GetFreeSlot(Item* tmp, int8 slot_id = 255, int16 version = 0);
	int32	CheckSlotConflict(Item* tmp, bool check_lore_only = false, int16* lore_stack_count = 0);
	
	int8	GetSlotByItem(Item* item);
	ItemStatsValues*	CalculateEquipmentBonuses(Entity* entity = 0);
	EQ2Packet* serialize(int16 version, Player* player);
	void SendEquippedItems(Player* player);
	uchar* xor_packet;
	uchar* orig_packet;

	void	SetAppearanceType(int8 type) { AppearanceType = type; }
	int8	GetAppearanceType() { return AppearanceType; }
private:
	int8 AppearanceType; // 0 for normal equip, 1 for appearance
};

#endif

