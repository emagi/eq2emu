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
#include "../../common/types.h"
#include "../../common/DataBuffer.h"
#include "../../common/MiscFunctions.h"
#include "../Commands/Commands.h"
#include "../../common/ConfigReader.h"

using namespace std;
class MasterItemList;
class Player;
class Entity;
extern MasterItemList master_item_list;
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

#define NUM_BANK_SLOTS 12
#define NUM_SHARED_BANK_SLOTS 8
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
#define ITEM_TYPE_BOOK			  13
#define ITEM_TYPE_ADORNMENT		  14
#define ITEM_TYPE_PATTERN		  15
#define ITEM_TYPE_ARMORSET		  16

#define ITEM_MENU_TYPE_GENERIC			1
#define ITEM_MENU_TYPE_EQUIP			2
#define ITEM_MENU_TYPE_BAG				4
#define ITEM_MENU_TYPE_HOUSE			8
#define ITEM_MENU_TYPE_SCRIBE			32
#define ITEM_MENU_TYPE_INVALID			128
#define ITEM_MENU_TYPE_BROKEN			512
#define ITEM_MENU_TYPE_ATTUNED			2048
#define ITEM_MENU_TYPE_ATTUNEABLE		4096
#define ITEM_MENU_TYPE_BOOK				8192
#define ITEM_MENU_TYPE_DISPLAY_CHARGES  16384
#define ITEM_MENU_TYPE_NAMEPET		    65536
#define ITEM_MENU_TYPE_CONSUME			262144
#define ITEM_MENU_TYPE_USE			    524288
#define ITEM_MENU_TYPE_DRINK	        8388608
#define ITEM_MENU_TYPE_REDEEM	        536870912

#define ITEM_TAG_UNCOMMON				3 //tier tags
#define ITEM_TAG_TREASURED				4
#define ITEM_TAG_LEGENDARY				7
#define ITEM_TAG_FABLED					9
#define ITEM_TAG_MYTHICAL				12

#define ITEM_BROKER_TYPE_ANY			0xFFFFFFFF
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

#define ITEM_STAT_HEALTH				500
#define ITEM_STAT_POWER					501
#define ITEM_STAT_CONCENTRATION			502

#define ITEM_STAT_HPREGEN 				600
#define ITEM_STAT_MANAREGEN 			601
#define ITEM_STAT_HPREGENPPT 			602
#define ITEM_STAT_MPREGENPPT 			603
#define ITEM_STAT_COMBATHPREGENPPT 		604
#define ITEM_STAT_COMBATMPREGENPPT 		605
#define ITEM_STAT_MAXHP 				606
#define ITEM_STAT_MAXHPPERC 			607
#define ITEM_STAT_SPEED 				608
#define ITEM_STAT_SLOW 					609
#define ITEM_STAT_MOUNTSPEED 			610
#define ITEM_STAT_MOUNTAIRSPEED 		611
#define ITEM_STAT_OFFENSIVESPEED 		612
#define ITEM_STAT_ATTACKSPEED 			613
#define ITEM_STAT_MAXMANA 				614
#define ITEM_STAT_MAXMANAPERC 			615
#define ITEM_STAT_MAXATTPERC 			616
#define ITEM_STAT_BLURVISION 			617
#define ITEM_STAT_MAGICLEVELIMMUNITY 	618
#define ITEM_STAT_HATEGAINMOD 			619
#define ITEM_STAT_COMBATEXPMOD 			620
#define ITEM_STAT_TRADESKILLEXPMOD 		621
#define ITEM_STAT_ACHIEVEMENTEXPMOD 	622
#define ITEM_STAT_SIZEMOD 				623
#define ITEM_STAT_DPS 					624
#define ITEM_STAT_STEALTH 				625
#define ITEM_STAT_INVIS 				626
#define ITEM_STAT_SEESTEALTH 			627
#define ITEM_STAT_SEEINVIS 				628
#define ITEM_STAT_EFFECTIVELEVELMOD 	629
#define ITEM_STAT_RIPOSTECHANCE 		630
#define ITEM_STAT_PARRYCHANCE 			631
#define ITEM_STAT_DODGECHANCE 			632
#define ITEM_STAT_AEAUTOATTACKCHANCE 	633
#define ITEM_STAT_MULTIATTACKCHANCE 		634
#define ITEM_STAT_SPELLMULTIATTACKCHANCE 	635
#define ITEM_STAT_FLURRY 					636
#define ITEM_STAT_MELEEDAMAGEMULTIPLIER 	637
#define ITEM_STAT_EXTRAHARVESTCHANCE 		638
#define ITEM_STAT_EXTRASHIELDBLOCKCHANCE 	639
#define ITEM_STAT_ITEMHPREGENPPT 			640
#define ITEM_STAT_ITEMPPREGENPPT 			641
#define ITEM_STAT_MELEECRITCHANCE 			642
#define ITEM_STAT_CRITAVOIDANCE 			643
#define ITEM_STAT_BENEFICIALCRITCHANCE 		644
#define ITEM_STAT_CRITBONUS 				645
#define ITEM_STAT_POTENCY 					646
#define ITEM_STAT_UNCONSCIOUSHPMOD 			647
#define ITEM_STAT_ABILITYREUSESPEED 		648
#define ITEM_STAT_ABILITYRECOVERYSPEED 		649
#define ITEM_STAT_ABILITYCASTINGSPEED 		650
#define ITEM_STAT_SPELLREUSESPEED		 	651
#define ITEM_STAT_MELEEWEAPONRANGE 			652
#define ITEM_STAT_RANGEDWEAPONRANGE 		653
#define ITEM_STAT_FALLINGDAMAGEREDUCTION 	654
#define ITEM_STAT_RIPOSTEDAMAGE 			655
#define ITEM_STAT_MINIMUMDEFLECTIONCHANCE 	656
#define ITEM_STAT_MOVEMENTWEAVE 			657
#define ITEM_STAT_COMBATHPREGEN 			658
#define ITEM_STAT_COMBATMANAREGEN 			659
#define ITEM_STAT_CONTESTSPEEDBOOST 		660
#define ITEM_STAT_TRACKINGAVOIDANCE 		661
#define ITEM_STAT_STEALTHINVISSPEEDMOD 		662
#define ITEM_STAT_LOOT_COIN 				663
#define ITEM_STAT_ARMORMITIGATIONINCREASE 	664
#define ITEM_STAT_AMMOCONSERVATION 			665
#define ITEM_STAT_STRIKETHROUGH 			666
#define ITEM_STAT_STATUSBONUS 				667
#define ITEM_STAT_ACCURACY 					668
#define ITEM_STAT_COUNTERSTRIKE 			669
#define ITEM_STAT_SHIELDBASH 				670
#define ITEM_STAT_WEAPONDAMAGEBONUS 		671
#define ITEM_STAT_ADDITIONALRIPOSTECHANCE 	672
#define ITEM_STAT_CRITICALMITIGATION 		673
#define ITEM_STAT_PVPTOUGHNESS 				674
#define ITEM_STAT_STAMINABONUS 				675
#define ITEM_STAT_WISDOMITBONUS 			676
#define ITEM_STAT_HEALRECEIVE 				677
#define ITEM_STAT_HEALRECEIVEPERC 			678
#define ITEM_STAT_PVPCRITICALMITIGATION		679
#define ITEM_STAT_BASEAVOIDANCEBONUS 		680


//#define ITEM_STAT_HPREGEN 				600
//#define ITEM_STAT_MANAREGEN 			601
//#define ITEM_STAT_HPREGENPPT 			602
//#define ITEM_STAT_MPREGENPPT 			603
//#define ITEM_STAT_COMBATHPREGENPPT 		604
//#define ITEM_STAT_COMBATMPREGENPPT 		605
//#define ITEM_STAT_MAXHP 				606
//#define ITEM_STAT_MAXHPPERC 			607
//#define ITEM_STAT_SPEED 				608
//#define ITEM_STAT_SLOW 					609
//#define ITEM_STAT_MOUNTSPEED 			610
//#define ITEM_STAT_OFFENSIVESPEED 		611
//#define ITEM_STAT_ATTACKSPEED 			612
//#define ITEM_STAT_MAXMANA 				613
//#define ITEM_STAT_MAXMANAPERC 			614
//#define ITEM_STAT_MAXATTPERC 			615
//#define ITEM_STAT_BLURVISION 			616
//#define ITEM_STAT_MAGICLEVELIMMUNITY 	617
//#define ITEM_STAT_HATEGAINMOD 			618
//#define ITEM_STAT_COMBATEXPMOD 			619
//#define ITEM_STAT_TRADESKILLEXPMOD 		620
//#define ITEM_STAT_ACHIEVEMENTEXPMOD 	621
//#define ITEM_STAT_SIZEMOD 				622
//#define ITEM_STAT_UNKNOWN 				623
//#define ITEM_STAT_STEALTH 				624
//#define ITEM_STAT_INVIS 				625
//#define ITEM_STAT_SEESTEALTH 			626
//#define ITEM_STAT_SEEINVIS 				627
//#define ITEM_STAT_EFFECTIVELEVELMOD 	628
//#define ITEM_STAT_RIPOSTECHANCE 		629
//#define ITEM_STAT_PARRYCHANCE 			630
//#define ITEM_STAT_DODGECHANCE 			631
//#define ITEM_STAT_AEAUTOATTACKCHANCE 	632
//#define ITEM_STAT_DOUBLEATTACKCHANCE 	633
//#define ITEM_STAT_RANGEDDOUBLEATTACKCHANCE 	634
//#define ITEM_STAT_SPELLDOUBLEATTACKCHANCE 	635
//#define ITEM_STAT_FLURRY 				636
//#define ITEM_STAT_EXTRAHARVESTCHANCE 	637
//#define ITEM_STAT_EXTRASHIELDBLOCKCHANCE 	638
#define ITEM_STAT_DEFLECTIONCHANCE 		400 //just so no build error
//#define ITEM_STAT_ITEMHPREGENPPT 		640
//#define ITEM_STAT_ITEMPPREGENPPT 		641
//#define ITEM_STAT_MELEECRITCHANCE 		642
//#define ITEM_STAT_RANGEDCRITCHANCE 		643
//#define ITEM_STAT_DMGSPELLCRITCHANCE 	644
//#define ITEM_STAT_HEALSPELLCRITCHANCE 	645
//#define ITEM_STAT_MELEECRITBONUS 		646
//#define ITEM_STAT_RANGEDCRITBONUS 		647
//#define ITEM_STAT_DMGSPELLCRITBONUS 	648
//#define ITEM_STAT_HEALSPELLCRITBONUS 	649
//#define ITEM_STAT_UNCONSCIOUSHPMOD 		650
//#define ITEM_STAT_SPELLTIMEREUSEPCT 	651
//#define ITEM_STAT_SPELLTIMERECOVERYPCT 	652
//#define ITEM_STAT_SPELLTIMECASTPCT 		653
//#define ITEM_STAT_MELEEWEAPONRANGE 		654
//#define ITEM_STAT_RANGEDWEAPONRANGE 	655
//#define ITEM_STAT_FALLINGDAMAGEREDUCTION 	656
//#define ITEM_STAT_SHIELDEFFECTIVENESS 	657
//#define ITEM_STAT_RIPOSTEDAMAGE 		658
//#define ITEM_STAT_MINIMUMDEFLECTIONCHANCE 	659
//#define ITEM_STAT_MOVEMENTWEAVE 		660
//#define ITEM_STAT_COMBATHPREGEN 		661
//#define ITEM_STAT_COMBATMANAREGEN 		662
//#define ITEM_STAT_CONTESTSPEEDBOOST 	663
//#define ITEM_STAT_TRACKINGAVOIDANCE 	664
//#define ITEM_STAT_STEALTHINVISSPEEDMOD 	665
//#define ITEM_STAT_LOOT_COIN 			666
//#define ITEM_STAT_ARMORMITIGATIONINCREASE 	667
//#define ITEM_STAT_AMMOCONSERVATION 		668
//#define ITEM_STAT_STRIKETHROUGH 		669
//#define ITEM_STAT_STATUSBONUS 			670
//#define ITEM_STAT_ACCURACY 				671
//#define ITEM_STAT_COUNTERSTRIKE 		672
//#define ITEM_STAT_SHIELDBASH 			673
//#define ITEM_STAT_WEAPONDAMAGEBONUS 	674
//#define ITEM_STAT_ADDITIONALRIPOSTECHANCE 	675
//#define ITEM_STAT_CRITICALMITIGATION 	676
//#define ITEM_STAT_COMBATARTDAMAGE 		677
//#define ITEM_STAT_SPELLDAMAGE 			678
//#define ITEM_STAT_HEALAMOUNT 			679
//#define ITEM_STAT_TAUNTAMOUNT 			680

#define ITEM_STAT_SPELL_DAMAGE			700
#define ITEM_STAT_HEAL_AMOUNT			701
#define ITEM_STAT_SPELL_AND_HEAL		702
#define ITEM_STAT_COMBAT_ART_DAMAGE		703
#define ITEM_STAT_SPELL_AND_COMBAT_ART_DAMAGE			704
#define ITEM_STAT_TAUNT_AMOUNT			705
#define ITEM_STAT_TAUNT_AND_COMBAT_ART_DAMAGE			706
#define ITEM_STAT_ABILITY_MODIFIER			707



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
	sint16			aeautoattackchance;
	sint16			strikethrough;
	sint16			accuracy;
	sint16			offensivespeed;
};
struct ItemCore{
	int32	item_id;
	sint32	soe_id;
	int32	bag_id;
	sint32	inv_slot_id;
	sint16	slot_id;
	int8	index;
	int16	icon;
	int16	count;
	int8	tier;
	int8	num_slots;
	int32	unique_id;
	int8	num_free_slots;
	int16	recommended_level;
	bool	item_locked;
};
#pragma pack()
struct ItemStat{
	string					stat_name;
	int8					stat_type;
	sint16					stat_subtype;
	int16					stat_type_combined;
	float					value;
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
	};
	struct Armor_Info {
		int16					mitigation_low;
		int16					mitigation_high;
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
	struct Skill_Info{
		int32					spell_id;
		int32					spell_tier;
	};
	struct House_Info{
		int32					status_rent_reduction;
	};
	struct HouseContainer_Info{
		int16					disallowed_types;
		int16					allowed_types;
		int8					num_slots;
	};
	struct RecipeBook_Info{
		vector<string>			recipes;
		int8					uses;
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
	#pragma pack()
	Item();
	Item(Item* in_item);
	~Item();
	string					lowername;
	string					name;
	string					description;
	int8					stack_count;
	int32					sell_price;
	int32					max_sell_value;
	bool					save_needed;
	int8					weapon_type;
	string					adornment;
	string					creator;
	vector<ItemStat*>		item_stats;
	vector<ItemStatString*>	item_string_stats;
	vector<ItemLevelOverride*> item_level_overrides;
	vector<ItemEffect*>		item_effects;
	Generic_Info			generic_info;
	Weapon_Info*			weapon_info;
	Ranged_Info*			ranged_info;
	Armor_Info*				armor_info;
	Bag_Info*				bag_info;
	Food_Info*				food_info;
	Bauble_Info*			bauble_info;
	Book_Info*				book_info;
	Skill_Info*				skill_info;
	RecipeBook_Info*		recipebook_info;
	Thrown_Info*			thrown_info;
	vector<int8>			slot_data;
	ItemCore				details;
	int32					spell_id;
	int8					spell_tier;
	string					item_script;

	void AddEffect(string effect, int8 percentage, int8 subbulletflag);
	int32 GetMaxSellValue();
	void SetMaxSellValue(int32 val);
	void SetItem(Item* old_item);
	int16 GetOverrideLevel(int8 adventure_class, int8 tradeskill_class);
	void AddLevelOverride(int8 adventure_class, int8 tradeskill_class, int16 level);
	void AddLevelOverride(ItemLevelOverride* class_);
	bool CheckClassLevel(int8 adventure_class, int8 tradeskill_class, int16 level);
	bool CheckClass(int8 adventure_class, int8 tradeskill_class);
	bool CheckLevel(int8 adventure_class, int8 tradeskill_class, int16 level);
	void SetAppearance(int16 type, int8 red, int8 green, int8 blue, int8 highlight_red, int8 highlight_green, int8 highlight_blue);
	void SetAppearance(ItemAppearance* appearance);
	void AddStat(ItemStat* in_stat);
	void AddStatString(ItemStatString* in_stat);
	void AddStat(int8 type, int16 subtype, float value, char* name = 0);
	void SetWeaponType(int8 type);
	int8 GetWeaponType();
	bool HasSlot(int8 slot, int8 slot2 = 255);
	bool IsNormal();
	bool IsWeapon();
	bool IsArmor();
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
	void SetItemScript(string name);
	const char*	GetItemScript();
	int32 CalculateRepairCost();

	void SetItemType(int8 in_type);
	void serialize(PacketStruct* packet, bool show_name = false, Player* player = 0, int16 packet_type = 0, int8 subtype = 0, bool loot_item = false);
	EQ2Packet* serialize(int16 version, bool show_name = false, Player* player = 0, bool include_twice = true, int16 packet_type = 0, int8 subtype = 0, bool merchant_item = false, bool loot_item = false);
	PacketStruct* PrepareItem(int16 version, bool merchant_item = false, bool loot_item = false);
	bool CheckFlag(int32 flag);
	void AddSlot(int8 slot_id);
	void SetSlots(int32 slots);
	bool needs_deletion;
};
class MasterItemList{
public:
	~MasterItemList();
	map<int32,Item*> items;

	Item* GetItem(int32 id);
	Item* GetItemByName(const char *name);
	ItemStatsValues* CalculateItemBonuses(int32 item_id, Entity* entity = 0);
	ItemStatsValues* CalculateItemBonuses(Item* desc, Entity* entity = 0, ItemStatsValues* values = 0);
	vector<Item*>* GetItems(string name, int32 itype, int32 ltype, int32 btype, int64 minprice, int64 maxprice, int8 minskill, int8 maxskill, string seller, string adornment, int8 mintier, int8 maxtier, int16 minlevel, int16 maxlevel, sint8 itemclass);
	vector<Item*>* GetItems(map<string, string> criteria);
	void AddItem(Item* item);
	bool IsBag(int32 item_id);
	void RemoveAll();
	static int32 NextUniqueID();
	static void ResetUniqueID(int32 new_id);
	static int32 next_unique_id;
};
class PlayerItemList {
public:
	PlayerItemList();
	~PlayerItemList();
//	int16 number;
	map<int32, Item*> indexed_items;
	map<sint32, map<int16, Item*> > items;
//	map< int8, Item* > inv_items;
//	map< int8, Item* > bank_items;
	bool  SharedBankAddAllowed(Item* item);
	vector<Item*>* GetItemsFromBagID(sint32 bag_id);
	vector<Item*>* GetItemsInBag(Item* bag);
	Item* GetBag(int8 inventory_slot, bool lock = true);
	bool  HasItem(int32 id, bool include_bank = false);
	Item* GetItemFromIndex(int32 index);
	void  MoveItem(Item* item, sint32 inv_slot, int16 slot, bool erase_old = true);
	bool  MoveItem(sint32 to_bag_id, int16 from_index, sint8 to, int8 charges);
	Item* GetItemFromUniqueID(int32 item_id, bool include_bank = false, bool lock = true);
	Item* GetItemFromID(int32 item_id, int8 count = 0, bool include_bank = false, bool lock = true);
	bool  AssignItemToFreeSlot(Item* item);
	int16 GetNumberOfFreeSlots();
	int16 GetNumberOfItems();
	bool  HasFreeSlot();
	bool  HasFreeBagSlot();
	void DestroyItem(int16 index);
	Item* CanStack(Item* item, bool include_bank = false);
	
	void RemoveItem(Item* item, bool delete_item = false);
	void AddItem(Item* item);

	Item* GetItem(sint32 bag_slot, int16 slot);
	
	EQ2Packet*	serialize(Player* player, int16 version);
	uchar* xor_packet;
	uchar* orig_packet;
	map<int32, Item*>* GetAllItems();
	bool HasFreeBankSlot();
	int8 FindFreeBankSlot();

	///<summary>Get the first free slot and stor them in the provided variables</summary>
	///<param name='bag_id'>Will contain the bag id of the first free spot</param>
	///<param name='slot'>Will contain the slot id of the first free slot</param>
	///<returns>True if a free slot was found</returns>
	bool GetFirstFreeSlot(sint32* bag_id, sint16* slot);
private:
	void Stack(Item* orig_item, Item* item);
	Mutex MPlayerItems;
	int16 packet_count;
};
class OverFlowItemList : public PlayerItemList {
public:
	bool OverFlowSlotFull();
	int8 GetNextOverFlowSlot();
	bool AddItem(Item* item);
	Item* GetOverFlowItem();
};
class EquipmentItemList{
public:
	EquipmentItemList();
	EquipmentItemList(const EquipmentItemList& list);
	~EquipmentItemList();
	Item* items[NUM_SLOTS];

	vector<Item*>* GetAllEquippedItems();

	bool	HasItem(int32 id);
	int8	GetNumberOfItems();
	Item*	GetItemFromUniqueID(int32 item_id);
	Item*	GetItemFromItemID(int32 item_id);
	void	SetItem(int8 slot_id, Item* item);
	void	RemoveItem(int8 slot, bool delete_item = false);
	Item*	GetItem(int8 slot_id);
	bool	AddItem(int8 slot, Item* item);
	bool	CheckEquipSlot(Item* tmp, int8 slot);
	bool	CanItemBeEquippedInSlot(Item* tmp, int8 slot);
	int8	GetFreeSlot(Item* tmp, int8 slot_id = 255);
	ItemStatsValues*	CalculateEquipmentBonuses(Entity* entity = 0);
	EQ2Packet* serialize(int16 version);
	uchar* xor_packet;
	uchar* orig_packet;
private:
	Mutex MEquipmentItems;
};

#endif

