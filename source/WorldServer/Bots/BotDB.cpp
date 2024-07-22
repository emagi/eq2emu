#include "../WorldDatabase.h"
#include "../../common/Log.h"
#include "Bot.h"
#include "../classes.h"
#include "../races.h"

extern Classes classes;
extern Races races;

int32 WorldDatabase::CreateNewBot(int32 char_id, string name, int8 race, int8 advClass, int8 gender, int16 model_id, int32& index) {
	DatabaseResult result;
	index = 0;

	if (!database_new.Select(&result, "SELECT MAX(`bot_id`) FROM `bots` WHERE `char_id` = %u", char_id)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return 0;
	}

	if (result.Next()) {
		if (result.IsNull(0))
			index = 1;
		else
			index = result.GetInt32(0) + 1;
	}

	if (!database_new.Query("INSERT INTO `bots` (`char_id`, `bot_id`, `name`, `race`, `class`, `gender`, `model_type`) VALUES (%u, %u, \"%s\", %u, %u, %u, %u)", char_id, index, name.c_str(), race, advClass, gender, model_id)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return 0;
	}
	int32 ret = database_new.LastInsertID();

	LogWrite(PLAYER__DEBUG, 0, "Player", "New bot (%s) created for player (%u)", name.c_str(), char_id);
	return ret;
}

void WorldDatabase::SaveBotAppearance(Bot* bot) {
	SaveBotColors(bot->BotID, "skin_color", bot->features.skin_color);
	SaveBotColors(bot->BotID, "model_color", bot->features.model_color);
	SaveBotColors(bot->BotID, "eye_color", bot->features.eye_color);
	SaveBotColors(bot->BotID, "hair_color1", bot->features.hair_color1);
	SaveBotColors(bot->BotID, "hair_color2", bot->features.hair_color2);
	SaveBotColors(bot->BotID, "hair_highlight", bot->features.hair_highlight_color);
	SaveBotColors(bot->BotID, "hair_type_color", bot->features.hair_type_color);
	SaveBotColors(bot->BotID, "hair_type_highlight_color", bot->features.hair_type_highlight_color);
	SaveBotColors(bot->BotID, "hair_face_color", bot->features.hair_face_color);
	SaveBotColors(bot->BotID, "hair_face_highlight_color", bot->features.hair_face_highlight_color);
	SaveBotColors(bot->BotID, "wing_color1", bot->features.wing_color1);
	SaveBotColors(bot->BotID, "wing_color2", bot->features.wing_color2);
	SaveBotColors(bot->BotID, "shirt_color", bot->features.shirt_color);
	//SaveBotColors(bot->BotID, "unknown_chest_color", );
	SaveBotColors(bot->BotID, "pants_color", bot->features.pants_color);
	//SaveBotColors(bot->BotID, "unknown_legs_color", );
	//SaveBotColors(bot->BotID, "unknown9", );
	SaveBotFloats(bot->BotID, "eye_type", bot->features.eye_type[0], bot->features.eye_type[1], bot->features.eye_type[2]);
	SaveBotFloats(bot->BotID, "ear_type", bot->features.ear_type[0], bot->features.ear_type[1], bot->features.ear_type[2]);
	SaveBotFloats(bot->BotID, "eye_brow_type", bot->features.eye_brow_type[0], bot->features.eye_brow_type[1], bot->features.eye_brow_type[2]);
	SaveBotFloats(bot->BotID, "cheek_type", bot->features.cheek_type[0], bot->features.cheek_type[1], bot->features.cheek_type[2]);
	SaveBotFloats(bot->BotID, "lip_type", bot->features.lip_type[0], bot->features.lip_type[1], bot->features.lip_type[2]);
	SaveBotFloats(bot->BotID, "chin_type", bot->features.chin_type[0], bot->features.chin_type[1], bot->features.chin_type[2]);
	SaveBotFloats(bot->BotID, "nose_type", bot->features.nose_type[0], bot->features.nose_type[1], bot->features.nose_type[2]);

	SaveBotFloats(bot->BotID, "body_size", bot->features.body_size, 0, 0);
	SaveBotFloats(bot->BotID, "body_age", bot->features.body_age, 0, 0);

	SaveBotColors(bot->BotID, "soga_skin_color", bot->features.soga_skin_color);
	SaveBotColors(bot->BotID, "soga_model_color", bot->features.soga_model_color);
	SaveBotColors(bot->BotID, "soga_eye_color", bot->features.soga_eye_color);
	SaveBotColors(bot->BotID, "soga_hair_color1", bot->features.soga_hair_color1);
	SaveBotColors(bot->BotID, "soga_hair_color2", bot->features.soga_hair_color2);
	SaveBotColors(bot->BotID, "soga_hair_highlight", bot->features.soga_hair_highlight_color);
	SaveBotColors(bot->BotID, "soga_hair_type_color", bot->features.soga_hair_type_color);
	SaveBotColors(bot->BotID, "soga_hair_type_highlight_color", bot->features.soga_hair_type_highlight_color);
	SaveBotColors(bot->BotID, "soga_hair_face_color", bot->features.soga_hair_face_color);
	SaveBotColors(bot->BotID, "soga_hair_face_highlight_color", bot->features.soga_hair_face_highlight_color);
	SaveBotColors(bot->BotID, "soga_wing_color1", bot->features.wing_color1);
	SaveBotColors(bot->BotID, "soga_wing_color2", bot->features.wing_color2);
	SaveBotColors(bot->BotID, "soga_shirt_color", bot->features.shirt_color);
	//SaveBotColors(bot->BotID, "soga_unknown_chest_color", );
	SaveBotColors(bot->BotID, "soga_pants_color", bot->features.pants_color);
	//SaveBotColors(bot->BotID, "soga_unknown_legs_color", );
	//SaveBotColors(bot->BotID, "soga_unknown13", );
	SaveBotFloats(bot->BotID, "soga_eye_type", bot->features.soga_eye_type[0], bot->features.soga_eye_type[1], bot->features.soga_eye_type[2]);
	SaveBotFloats(bot->BotID, "soga_ear_type", bot->features.soga_ear_type[0], bot->features.soga_ear_type[1], bot->features.soga_ear_type[2]);
	SaveBotFloats(bot->BotID, "soga_eye_brow_type", bot->features.soga_eye_brow_type[0], bot->features.soga_eye_brow_type[1], bot->features.soga_eye_brow_type[2]);
	SaveBotFloats(bot->BotID, "soga_cheek_type", bot->features.soga_cheek_type[0], bot->features.soga_cheek_type[1], bot->features.soga_cheek_type[2]);
	SaveBotFloats(bot->BotID, "soga_lip_type", bot->features.soga_lip_type[0], bot->features.soga_lip_type[1], bot->features.soga_lip_type[2]);
	SaveBotFloats(bot->BotID, "soga_chin_type", bot->features.soga_chin_type[0], bot->features.soga_chin_type[1], bot->features.soga_chin_type[2]);
	SaveBotFloats(bot->BotID, "soga_nose_type", bot->features.soga_nose_type[0], bot->features.soga_nose_type[1], bot->features.soga_nose_type[2]);

	if (!database_new.Query("UPDATE `bots` SET `model_type` = %u, `hair_type` = %u, `face_type` = %u, `wing_type` = %u, `chest_type` = %u, `legs_type` = %u, `soga_model_type` = %u, `soga_hair_type` = %u, `soga_face_type` = %u WHERE `id` = %u",
												bot->GetModelType(), bot->GetHairType(), bot->GetFacialHairType(), bot->GetWingType(), bot->GetChestType(), bot->GetLegsType(), bot->GetSogaModelType(), bot->GetSogaHairType(), bot->GetSogaFacialHairType(), bot->BotID)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}
}

void WorldDatabase::SaveBotColors(int32 bot_id, const char* type, EQ2_Color color) {
	if (!database_new.Query("INSERT INTO `bot_appearance` (`bot_id`, `type`, `red`, `green`, `blue`) VALUES (%i, '%s', %i, %i, %i) ON DUPLICATE KEY UPDATE `red` = %i, `blue` = %i, `green` = %i", bot_id, type, color.red, color.green, color.blue, color.red, color.blue, color.green)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}
}

void WorldDatabase::SaveBotFloats(int32 bot_id, const char* type, float float1, float float2, float float3) {
	if (!database_new.Query("INSERT INTO `bot_appearance` (`bot_id`, `type`, `red`, `green`, `blue`, `signed_value`) VALUES (%i, '%s', %i, %i, %i, 1) ON DUPLICATE KEY UPDATE `red` = %i, `blue` = %i, `green` = %i", bot_id, type, float1, float2, float3, float1, float2, float3)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}
}

bool WorldDatabase::LoadBot(int32 char_id, int32 bot_index, Bot* bot) {
	DatabaseResult result;

	if (!database_new.Select(&result, "SELECT * FROM bots WHERE `char_id` = %u AND `bot_id` = %u", char_id, bot_index)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return false;
	}

	if (result.Next()) {
		bot->BotID = result.GetInt32(0);
		bot->BotIndex = result.GetInt32(2);
		bot->SetName(result.GetString(3));
		bot->SetRace(result.GetInt8(4));
		bot->SetAdventureClass(result.GetInt8(5));
		bot->SetGender(result.GetInt8(6));
		bot->SetModelType(result.GetInt16(7));
		bot->SetHairType(result.GetInt16(8));
		bot->SetFacialHairType(result.GetInt16(9));
		bot->SetWingType(result.GetInt16(10));
		bot->SetChestType(result.GetInt16(11));
		bot->SetLegsType(result.GetInt16(12));
		bot->SetSogaModelType(result.GetInt16(13));
		bot->SetSogaHairType(result.GetInt16(14));
		bot->SetSogaFacialHairType(result.GetInt16(15));
	}
	else
		return false;
	
	LoadBotAppearance(bot);
	LoadBotEquipment(bot);
	return true;
}

void WorldDatabase::LoadBotAppearance(Bot* bot) {
	DatabaseResult result;
	string type;
	map<string, int8> appearance_types;
	EQ2_Color color;
	color.red = 0;
	color.green = 0;
	color.blue = 0;

	if (!database_new.Select(&result, "SELECT distinct `type` FROM bot_appearance WHERE length(`type`) > 0 AND `bot_id` = %u", bot->BotID)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}

	while (result.Next()) {
		type = result.GetString(0);
		appearance_types[type] = GetAppearanceType(type);
		if (appearance_types[type] == 255)
			LogWrite(WORLD__ERROR, 0, "Appearance", "Unknown appearance type '%s' in LoadBotAppearances.", type.c_str());
	}

	if (!database_new.Select(&result, "SELECT `type`, `signed_value`, `red`, `green`, `blue` FROM bot_appearance WHERE length(`type`) > 0 AND bot_id = %u", bot->BotID)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}
	
	while (result.Next()) {
		type = result.GetString(0);
		if (appearance_types[type] < APPEARANCE_SOGA_EBT) {
			color.red = result.GetInt8(2);
			color.green = result.GetInt8(3);
			color.blue = result.GetInt8(4);
		}
		switch (appearance_types[type]) {
		case APPEARANCE_SOGA_HFHC: {
			bot->features.soga_hair_face_highlight_color = color;
			break;
		}
		case APPEARANCE_SOGA_HTHC: {
			bot->features.soga_hair_type_highlight_color = color;
			break;
		}
		case APPEARANCE_SOGA_HFC: {
			bot->features.soga_hair_face_color = color;
			break;
		}
		case APPEARANCE_SOGA_HTC: {
			bot->features.soga_hair_type_color = color;
			break;
		}
		case APPEARANCE_SOGA_HH: {
			bot->features.soga_hair_highlight_color = color;
			break;
		}
		case APPEARANCE_SOGA_HC1: {
			bot->features.soga_hair_color1 = color;
			break;
		}
		case APPEARANCE_SOGA_HC2: {
			bot->features.soga_hair_color2 = color;
			break;
		}
		case APPEARANCE_SOGA_SC: {
			bot->features.soga_skin_color = color;
			break;
		}
		case APPEARANCE_SOGA_EC: {
			bot->features.soga_eye_color = color;
			break;
		}
		case APPEARANCE_HTHC: {
			bot->features.hair_type_highlight_color = color;
			break;
		}
		case APPEARANCE_HFHC: {
			bot->features.hair_face_highlight_color = color;
			break;
		}
		case APPEARANCE_HTC: {
			bot->features.hair_type_color = color;
			break;
		}
		case APPEARANCE_HFC: {
			bot->features.hair_face_color = color;
			break;
		}
		case APPEARANCE_HH: {
			bot->features.hair_highlight_color = color;
			break;
		}
		case APPEARANCE_HC1: {
			bot->features.hair_color1 = color;
			break;
		}
		case APPEARANCE_HC2: {
			bot->features.hair_color2 = color;
			break;
		}
		case APPEARANCE_WC1: {
			bot->features.wing_color1 = color;
			break;
		}
		case APPEARANCE_WC2: {
			bot->features.wing_color2 = color;
			break;
		}
		case APPEARANCE_SC: {
			bot->features.skin_color = color;
			break;
		}
		case APPEARANCE_EC: {
			bot->features.eye_color = color;
			break;
		}
		case APPEARANCE_SOGA_EBT: {
			for (int i = 0; i < 3; i++)
				bot->features.soga_eye_brow_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_SOGA_CHEEKT: {
			for (int i = 0; i < 3; i++)
				bot->features.soga_cheek_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_SOGA_NT: {
			for (int i = 0; i < 3; i++)
				bot->features.soga_nose_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_SOGA_CHINT: {
			for (int i = 0; i < 3; i++)
				bot->features.soga_chin_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_SOGA_LT: {
			for (int i = 0; i < 3; i++)
				bot->features.soga_lip_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_SOGA_EART: {
			for (int i = 0; i < 3; i++)
				bot->features.soga_ear_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_SOGA_EYET: {
			for (int i = 0; i < 3; i++)
				bot->features.soga_eye_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_EBT: {
			for (int i = 0; i < 3; i++)
				bot->features.eye_brow_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_CHEEKT: {
			for (int i = 0; i < 3; i++)
				bot->features.cheek_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_NT: {
			for (int i = 0; i < 3; i++)
				bot->features.nose_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_CHINT: {
			for (int i = 0; i < 3; i++)
				bot->features.chin_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_EART: {
			for (int i = 0; i < 3; i++)
				bot->features.ear_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_EYET: {
			for (int i = 0; i < 3; i++)
				bot->features.eye_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_LT: {
			for (int i = 0; i < 3; i++)
				bot->features.lip_type[i] = result.GetSInt8(2 + i);
			break;
		}
		case APPEARANCE_SHIRT: {
			bot->features.shirt_color = color;
			break;
		}
		case APPEARANCE_UCC: {
			break;
		}
		case APPEARANCE_PANTS: {
			bot->features.pants_color = color;
			break;
		}
		case APPEARANCE_ULC: {
			break;
		}
		case APPEARANCE_U9: {
			break;
		}
		case APPEARANCE_BODY_SIZE: {
			bot->features.body_size = color.red;
			break;
		}
		case APPEARANCE_SOGA_WC1: {
			break;
		}
		case APPEARANCE_SOGA_WC2: {
			break;
		}
		case APPEARANCE_SOGA_SHIRT: {
			break;
		}
		case APPEARANCE_SOGA_UCC: {
			break;
		}
		case APPEARANCE_SOGA_PANTS: {
			break;
		}
		case APPEARANCE_SOGA_ULC: {
			break;
		}
		case APPEARANCE_SOGA_U13: {
			break;
		}
		case APPEARANCE_BODY_AGE: {
			bot->features.body_age = color.red;
			break;
		}
		case APPEARANCE_MC:{
			bot->features.model_color = color;
			break;
		}
		case APPEARANCE_SMC:{
			bot->features.soga_model_color = color;
			break;
		}
		case APPEARANCE_SBS: {
			bot->features.soga_body_size = color.red;
			break;
		}
		case APPEARANCE_SBA: {
			bot->features.soga_body_age = color.red;
			break;
		}
		}
	}
}

void WorldDatabase::SaveBotItem(int32 bot_id, int32 item_id, int8 slot) {
	if (!database_new.Query("INSERT INTO `bot_equipment` (`bot_id`, `slot`, `item_id`) VALUES (%u, %u, %u) ON DUPLICATE KEY UPDATE `item_id` = %u", bot_id, slot, item_id, item_id)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}
}

void WorldDatabase::LoadBotEquipment(Bot* bot) {
	DatabaseResult result;

	if (!database_new.Select(&result, "SELECT `slot`, `item_id` FROM `bot_equipment` WHERE `bot_id` = %u", bot->BotID)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}

	Item* master_item = 0;
	Item* item = 0;

	while (result.Next()) {
		int8 slot = result.GetInt8(0);
		int32 item_id = result.GetInt32(1);

		master_item = master_item_list.GetItem(item_id);
		if (master_item) {
			item = new Item(master_item);
			if (item) {
				bot->GetEquipmentList()->AddItem(slot, item);
				bot->SetEquipment(item, slot);
			}
		}
	}
}

string WorldDatabase::GetBotList(int32 char_id) {
	DatabaseResult result;
	string ret;

	if (!database_new.Select(&result, "SELECT `bot_id`, `name`, `race`, `class` FROM `bots` WHERE `char_id` = %u", char_id)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return ret;
	}

	while (result.Next()) {
		ret += to_string(result.GetInt32(0)) + ": ";
		ret += result.GetString(1);
		ret += " the ";
		ret += races.GetRaceNameCase(result.GetInt8(2));
		ret += " ";
		ret += classes.GetClassNameCase(result.GetInt8(3)) + "\n";
	}

	return ret;
}

void WorldDatabase::DeleteBot(int32 char_id, int32 bot_index) {
	if (!database_new.Query("DELETE FROM `bots` WHERE `char_id` = %u AND `bot_id` = %u", char_id, bot_index)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
	}
}

void WorldDatabase::SetBotStartingItems(Bot* bot, int8 class_id, int8 race_id) {
	int32 bot_id = bot->BotID;
	LogWrite(PLAYER__DEBUG, 0, "Bot", "Adding default items for race: %u, class: %u for bot_id: %u", race_id, class_id, bot_id);

	DatabaseResult result;
	if (!database_new.Select(&result, "SELECT item_id FROM starting_items WHERE class_id IN (%i, %i, %i, 255) AND race_id IN (%i, 255) ORDER BY id", classes.GetBaseClass(class_id), classes.GetSecondaryBaseClass(class_id), class_id, race_id)) {
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", database_new.GetError(), database_new.GetErrorMsg());
		return;
	}

	while (result.Next()) {
		bot->GiveItem(result.GetInt32(0));
	}
}