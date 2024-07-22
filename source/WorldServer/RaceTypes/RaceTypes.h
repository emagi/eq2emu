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

#ifndef __RACETYPES_H__
#define __RACETYPES_H__

#include "../../common/types.h"
#include <map>

#define DRAGONKIND			101
#define DRAGON				102
#define DRAKE				103
#define DRAKOTA				104
#define DROAG				105
#define FAEDRAKE			106
//FLYINGSNAKE  Defined in natural as well, think is a better fit there then here
#define SOKOKAR				107
#define WURM				108
#define WYRM				109
#define WYVERN				110

#define FAY					111
#define ARASAI_NPC			112
#define BIXIE				113
#define BROWNIE				114
#define DRYAD				115
#define FAE_NPC				116
#define FAIRY				117
#define SIREN				118
#define SPIRIT				119
#define SPRITE				120
#define TREANT				121 //L&L 8
#define WISP				122

#define MAGICAL				123
#define AMORPH				124
#define CONSTRUCT			125
#define ANIMATION			126
#define BONEGOLEM			127
#define BOVOCH				128
#define CARRIONGOLEM		129
#define CLAYGOLEM			130
#define CUBE				131
#define DERVISH				132
#define DEVOURER			133
#define GARGOYLE			134
#define GOLEM				135
#define GOO					136
#define HARPY				137
#define IMP					138
#define LIVINGSTATUE		139
#define MANNEQUIN			140
#define MIMIC				141
#define MOPPET				142
#define NAGA				143
#define NAYAD				144
#define OOZE				145
#define RUMBLER				146
#define RUSTMONSTER			147
#define SATYR				148
#define SCARECROW			149
#define SPHEROID			150
#define TENTACLETERROR		151
#define TOME				152
#define UNICORN				153
#define WOODELEMENTAL		154

#define MECHANIMAGICAL		155
#define CLOCKWORK			156
#define IRONGUARDIAN		157

#define NATURAL				158
#define ANIMAL				159
#define AQUATIC				160
#define AVIAN				161
#define CANINE				162
#define EQUINE				163
#define FELINE				164
#define INSECT				165
#define PRIMATE				166
#define REPTILE				167
#define ANEMONE				168
#define APOPHELI			169
#define ARMADILLO			170
#define BADGER				171
#define BARRACUDA			172
#define BASILISK			173
#define BAT					174
#define BEAR				175
#define BEAVER				176
#define BEETLE				177
#define BOVINE				178
#define BRONTOTHERIUM		179
#define BRUTE				180
#define CAMEL				181
#define CAT					182
#define CENTIPEDE			183
#define CERBERUS			184
#define CHIMERA				185
#define CHOKIDAI			186
#define COBRA				187
#define COCKATRICE			188
#define CRAB				189
#define CROCODILE			190
#define DEER				191
#define DRAGONFLY			192
#define DUCK				193
#define EEL					194
#define ELEPHANT			195
#define FLYINGSNAKE			196
#define FROG				197
#define GOAT				198
#define GORILLA				199
#define GRIFFIN				200
#define HAWK				201
#define HIVEQUEEN			202
#define HORSE				203
#define HYENA				204
#define KHOALRAT			205
#define KYBUR				206
#define LEECH				207
#define LEOPARD				208
#define LION				209
#define LIZARD				210
#define MAMMOTH				211
#define MANTARAY			212
#define MOLERAT				213
#define MONKEY				214
#define MYTHICAL			215
#define OCTOPUS				216
#define OWLBEAR				217
#define PIG					218
#define PIRANHA				219
#define RAPTOR				220
#define RAT					221
#define RHINOCEROS			222
#define ROCKCRAWLER			223
#define SABERTOOTH			224
#define SCORPION			225
#define SEATURTLE			226
#define SHARK				227
#define SHEEP				228
#define SLUG				229
#define SNAKE				230
#define SPIDER				231
#define STIRGE				232
#define SWORDFISH			233
#define TIGER				234
#define TURTLE				235
#define VERMIN				236
#define VULRICH				237
#define WOLF				238
#define YETI				239

#define PLANAR				240
#define ABOMINATION			241
#define AIRELEMENTAL		242
#define AMYGDALAN			243
#define AVATAR				244
#define CYCLOPS				245
#define DEMON				246
#define DJINN				247
#define EARTHELEMENTAL		248
#define EFREETI				249
#define ELEMENTAL			250
#define ETHEREAL			251
#define ETHERPINE			252
#define EVILEYE				253
#define FIREELEMENTAL		254
#define GAZER				255
#define GEHEIN				256
#define GEONID				257
#define GIANT				258 //L&L 5
#define SALAMANDER			259
#define SHADOWEDMAN			260
#define SPHINX				261
#define SPORE				262
#define SUCCUBUS			263
#define VALKYRIE			264
#define VOIDBEAST			265
#define WATERELEMENTAL		266
#define WRAITH				267

#define PLANT				268
#define CARNIVOROUSPLANT	269
#define CATOPLEBAS			270
#define MANTRAP				271
#define ROOTABOMINATION		272
#define ROOTHORROR			273
#define SUCCULENT			274

#define SENTIENT			275
#define ASHLOK				276
#define AVIAK				277
#define BARBARIAN_NPC		278
#define BIRDMAN				279
#define BOARFIEND			280
#define BUGBEAR				281
#define BURYNAI				282
#define CENTAUR				283 ////L&L 4
#define COLDAIN				284
#define DAL					285
#define DARKELF_NPC			286
#define DIZOK				287
#define DRACHNID			288
#define DRAFLING			289
#define DROLVARG			290
#define DWARF_NPC			291
#define ERUDITE_NPC			292
#define ETTIN				293
#define FREEBLOOD_NPC		294
#define FROGLOK_NPC			295
#define FROSTFELLELF		296
#define FUNGUSMAN			297
#define GNOLL				298 //L&L 1
#define GNOME_NPC			299
#define GOBLIN				300 //L&L 3
#define GRUENGACH			301
#define HALFELF_NPC			302		// Not on the list from wikia but all other races were here so added them
#define HALFLING_NPC		303
#define HIGHELF_NPC			304		// Not on the list from wikia but all other races were here so added them
#define HOLGRESH			305
#define HOOLUK				306
#define HUAMEIN				307
#define HUMAN_NPC			308
#define HUMANOID			309
#define IKSAR_NPC			310
#define KERIGDAL			311
#define KERRAN_NPC			312
#define KOBOLD				313
#define LIZARDMAN			314
#define MINOTAUR			315
#define OGRE_NPC			316
#define ORC					317 //L&L 2
#define OTHMIR				318
#define RATONGA_NPC			319
#define RAVASECT			320
#define RENDADAL			321
#define ROEKILLIK			322
#define SARNAK_NPC			323
#define SKORPIKIS			324
#define SPIROC				325
#define TROGLODYTE			326
#define TROLL_NPC			327
#define ULTHORK				328
#define VULTAK				329
#define WOODELF_NPC			330
#define WRAITHGUARD			331
#define YHALEI				332

#define UNDEAD				333
#define GHOST				334
#define GHOUL				335
#define GUNTHAK				336
#define HORROR				337
#define MUMMY				338
#define SHINREEORCS			339
#define SKELETON			340 //L&L 6
#define SPECTRE				341
#define VAMPIRE_NPC			342
#define ZOMBIE				343 //L&L 7

#define WERE				344
#define AHROUNWEREWOLVES	345
#define LYKULAKWEREWOLVES	346
#define WEREWOLF			347
 
struct RaceTypeStructure {
		int16 race_type_id;
		char category[64];
		char subcategory[64];
		char modelname[250];
};

class MasterRaceTypeList {
public:
	MasterRaceTypeList();
	~MasterRaceTypeList();

	/// <summary>Add a race type define to the list</summary>
	/// <param name='model_id'>The id of the model</param>
	/// <param name=race_type_id'>The id of the race type</param>
	/// <param name=category'>The category of the race type</param>
	/// <param name=subcategory'>The subcategory of the race type</param>
	/// <param name=modelname'>The model name of the model id</param>
	bool AddRaceType(int16 model_id, int16 race_type_id, const char* category, const char* subcategory, const char* modelname, bool allow_override = false);

	/// <summary>Gets the race type for the given model</summary>
	/// <param name='model_id'>The model id to get the race type for</param>
	int16 GetRaceType(int16 model_id);
	char* GetRaceTypeCategory(int16 model_id);
	char* GetRaceTypeSubCategory(int16 model_id);
	char* GetRaceTypeModelName(int16 model_id);
	
	/// <summary>Gets the base race type for the given model</summary>
	/// <param name='model_id'>The model id to get the base race type for</param>
	int16 GetRaceBaseType(int16 model_id);

private:
	// model id, race type id
	map<int16, RaceTypeStructure> m_raceList;
};

#endif