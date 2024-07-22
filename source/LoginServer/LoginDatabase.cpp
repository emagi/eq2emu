/*  
	EQ2Emulator:  Everquest II Server Emulator
	Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

	This file is part of EQ2Emulator.
*/
#include "../common/debug.h"

#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else
#include "../common/unix.h"
#include <netinet/in.h>
#endif

#include "../common/Log.h"
#include "../common/DatabaseNew.h"
#include "LoginDatabase.h"
#include "LoginAccount.h"
#include "../common/MiscFunctions.h"
#include "../common/packet_functions.h"
#include "../common/packet_dump.h"
#include "LWorld.h"

extern LoginDatabase database;
extern LWorldList world_list;


bool LoginDatabase::ConnectNewDatabase() {
	return dbLogin.Connect();
}

void LoginDatabase::RemoveDeletedCharacterData()
{
	dbLogin.Query("DELETE FROM login_char_colors WHERE login_characters_id IN (SELECT id FROM login_characters WHERE deleted = 1)");
	dbLogin.Query("DELETE FROM login_equipment WHERE login_characters_id IN (SELECT id FROM login_characters WHERE deleted = 1)");
}

void LoginDatabase::SetZoneInformation(int32 server_id, int32 zone_id, int32 version, PacketStruct* packet){	
	if(packet){
		Query query;
		MYSQL_RES* result = 0;
		
		if ( version >= 1212 )
			result = query.RunQuery2(Q_SELECT, "SELECT name, description from ls_world_zones where server_id=%i and zone_id=%i", server_id, zone_id);

		MYSQL_ROW row;
		if(result && (row = mysql_fetch_row(result))) {
				if (row[0])
					packet->setMediumStringByName("zone", row[0]);
				else
					packet->setMediumStringByName("zone", " ");
			if(row[1])
				packet->setMediumStringByName("zonedesc", row[1]);
			else
				packet->setMediumStringByName("zonedesc", " ");			
		}
		else{
			Query query2;
			MYSQL_RES* result2 = 0;

			if (version < 1212)
				result2 = query2.RunQuery2(Q_SELECT, "SELECT file, description from zones where id=%i", zone_id);
			else
				result2 = query2.RunQuery2(Q_SELECT, "SELECT name, description from zones where id=%i", zone_id);

			MYSQL_ROW row2;
			if(result2 && (row2 = mysql_fetch_row(result2))) {

				if (version < 546 && version > 561 && version < 1212)
				{
					if (row2[0])
					{
						int len = strlen(row2[0]);
						char* zoneName = new char[len + 2];
						strncpy(zoneName, row2[0], len);
						zoneName[len] = 0x2E;
						zoneName[len + 1] = 0x30;

						packet->setMediumStringByName("zone", zoneName);
						safe_delete_array(zoneName);
					}
					else
						packet->setMediumStringByName("zone", ".0");
				}
				else
				{
					if (row2[0])
						packet->setMediumStringByName("zone", row2[0]);
					else
						packet->setMediumStringByName("zone", " ");
				}
				if(row2[1])
					packet->setMediumStringByName("zonedesc", row2[1]);
				else
					packet->setMediumStringByName("zonedesc", " ");
			}
		}
		packet->setMediumStringByName("zonename2"," ");
	}
}

string LoginDatabase::GetZoneDescription(char* name){
	string ret;
	Query query;
	query.escaped_name = getEscapeString(name);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT description from zones where file=substring_index('%s', '.', 1)", query.escaped_name);
	MYSQL_ROW row;
	if((row = mysql_fetch_row(result))) {
		ret = string(row[0]);
	}
	return ret;
}


int32 LoginDatabase::GetLoginCharacterIDFromWorldCharID(int32 server_id, int32 char_id)
{
	int32 ret;
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id FROM login_characters WHERE server_id = %u AND char_id = %u AND deleted = 0 LIMIT 0,1", server_id, char_id);
	MYSQL_ROW row;
	if((row = mysql_fetch_row(result))) {
		ret = atoi(row[0]);
	}

	return ret;
}

void LoginDatabase::SetServerEquipmentAppearances(int32 server_id, map<int32, LoginEquipmentUpdate> equip_updates)
{
 
	if(equip_updates.size() > 0)
	{

		LogWrite(LOGIN__DEBUG, 0, "Login", "Saving appearance info from world %u...", server_id);

		map<int32, LoginEquipmentUpdate>::iterator equip_itr;
		stringstream ss;
		ss << "replace into login_equipment (login_characters_id, equip_type, red, green, blue, highlight_red, highlight_green, highlight_blue, slot) values";
		int count=0;
		int32 char_id = 0;

		for(equip_itr = equip_updates.begin(); equip_itr != equip_updates.end(); equip_itr++)
		{
			char_id = GetLoginCharacterIDFromWorldCharID(server_id, (int32)equip_itr->second.world_char_id);

			if( char_id == 0 ) // invalid character/world match
				continue;

			LogWrite(LOGIN__DEBUG, 5, "Login", "--Processing character %u, slot %i", char_id, (int32)equip_itr->second.slot);

			if(count > 0)
				ss << ", ";

			ss << "(" << char_id << ", ";
			ss << (int32)equip_itr->second.equip_type << ", ";
			ss << (int32)equip_itr->second.red << ", ";
			ss << (int32)equip_itr->second.green << ", ";
			ss << (int32)equip_itr->second.blue << ", ";
			ss << (int32)equip_itr->second.highlight_red << ", ";
			ss << (int32)equip_itr->second.highlight_green << ", ";
			ss << (int32)equip_itr->second.highlight_blue << ", ";
			ss << (int32)equip_itr->second.slot << ")";

			count++;
		}
      
   		Query query;
		query.RunQuery2(ss.str(), Q_REPLACE);
      
		if (query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF)
			LogWrite(LOGIN__ERROR, 0, "Login", "Error saving login_equipment data Error: ", query.GetError());
	}
}


void LoginDatabase::SetServerZoneDescriptions(int32 server_id, map<int32, LoginZoneUpdate> zone_descriptions){
	if(zone_descriptions.size() > 0){
		map<int32, LoginZoneUpdate>::iterator zone_itr;
		string query_string = "replace into ls_world_zones (server_id, zone_id, name, description) values";
		int count=0;
		char server_id_str[12] = {0};		
		sprintf(server_id_str, "%i", server_id);
		for(zone_itr = zone_descriptions.begin(); zone_itr != zone_descriptions.end(); zone_itr++, count++){
			char zone_id_str[12] = {0};
			sprintf(zone_id_str, "%i", zone_itr->first);	
			if(count > 0)
				query_string.append(", ");
			query_string.append("(").append(server_id_str).append(",");
			query_string.append(zone_id_str).append(",");
			query_string.append("'").append(getSafeEscapeString(zone_itr->second.name.c_str()).c_str()).append("', '");
			query_string.append(getSafeEscapeString(zone_itr->second.description.c_str()).c_str()).append("')");			
		}
		Query query;
		query.RunQuery2(query_string, Q_REPLACE); 
	}
}

//this is really just for the version that doesn't send the server id in its play request
int32 LoginDatabase::GetServer(int32 accountID, int32 charID, string name) {
	int32 id = 0;
	Query query;
	MYSQL_ROW row;
	query.escaped_name = getEscapeString(name.c_str());
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT server_id from login_characters where account_id=%i and char_id=%i and name='%s'", accountID, charID, query.escaped_name);
	if (result && mysql_num_rows(result) == 1) {
		row = mysql_fetch_row(result);
		id = atoi(row[0]);
	}
	return id;
}

void LoginDatabase::LoadCharacters(LoginAccount* acct, int16 version){
	if(acct != NULL)
		acct->flushCharacters ( );

	Query query;
	Query query2;
	int32 id = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT lc.char_id, lc.server_id, lc.name, lc.race, lc.class, lc.gender, lc.deity, lc.body_size, lc.body_age, lc.current_zone_id, lc.level, lc.soga_wing_type, lc.soga_chest_type, lc.soga_legs_type, lc.soga_hair_type, lc.legs_type, lc.chest_type, lc.wing_type, lc.hair_type, unix_timestamp(lc.created_date), unix_timestamp(lc.last_played), lc.id, lw.name, lc.facial_hair_type, lc.soga_facial_hair_type, lc.soga_model_type, lc.model_type from login_characters lc, login_worldservers lw where lw.id = lc.server_id and lc.account_id=%i and lc.deleted=0",acct->getLoginAccountID());
	if(result) {
		MYSQL_ROW row;
		MYSQL_ROW row2;
		MYSQL_ROW row3;
		while ((row = mysql_fetch_row(result))) {
			CharSelectProfile* player = new CharSelectProfile(version);
			id = atoul(row[0]);		
			//for (int i = 0; i < 10; i++)
			//	player->packet->setDataByName("hair_type", 0, i);
			//player->packet->setDataByName("test23", 413);
			//player->packet->setDataByName("test24", 414);
			player->packet->setDataByName("charid", id);
			player->packet->setDataByName("server_id", atoul(row[1]));
			player->packet->setMediumStringByName("name", row[2]);
			player->packet->setDataByName("race", atoi(row[3]));
			player->packet->setDataByName("class", atoi(row[4]));
			player->packet->setDataByName("gender", atoi(row[5]));
			player->packet->setDataByName("deity", atoi(row[6]));
			player->packet->setDataByName("body_size", atof(row[7]));
			player->packet->setDataByName("body_age", atof(row[8]));
			SetZoneInformation(atoi(row[1]), atoi(row[9]), version, player->packet);
			player->packet->setDataByName("level", atoi(row[10]));
			if(atoi(row[11]) > 0)
				player->packet->setDataByName("soga_wing_type", atoi(row[11]));
			else
				player->packet->setDataByName("soga_wing_type", atoi(row[17]));
			if(atoi(row[12]) > 0)
				player->packet->setDataByName("soga_chest_type", atoi(row[12]));
			else
				player->packet->setDataByName("soga_chest_type", atoi(row[16]));
			if(atoi(row[13]) > 0)
				player->packet->setDataByName("soga_legs_type", atoi(row[13]));
			else
				player->packet->setDataByName("soga_legs_type", atoi(row[15]));
			if(atoi(row[14]) > 0)
				player->packet->setDataByName("soga_hair_type", atoi(row[14]));
			else
				player->packet->setDataByName("soga_hair_type", atoi(row[18]));
			player->packet->setDataByName("legs_type", atoi(row[15]));
			player->packet->setDataByName("chest_type", atoi(row[16]));
			player->packet->setDataByName("wing_type", atoi(row[17]));
			player->packet->setDataByName("hair_type", atoi(row[18]));
			player->packet->setDataByName("created_date", atol(row[19]));
			if (row[20])
				player->packet->setDataByName("last_played", atol(row[20]));
			if(version == 546 || version == 561)
				player->packet->setDataByName("version", 11);
			else if(version >= 887)
				player->packet->setDataByName("version", 6);
			else
				player->packet->setDataByName("version", 5);
			player->packet->setDataByName("account_id", acct->getLoginAccountID());
			player->packet->setDataByName("account_id2", acct->getLoginAccountID());
			
			LoadAppearanceData(atoul(row[21]), player->packet);

			if(row[22])
				player->packet->setMediumStringByName("server_name", row[22]);
			player->packet->setDataByName("hair_face_type", atoi(row[23]));
			if(atoi(row[24]) > 0)				
				player->packet->setDataByName("soga_hair_face_type", atoi(row[24]));
			else
				player->packet->setDataByName("soga_hair_face_type", atoi(row[23]));
			if(atoi(row[25]) > 0)
				player->packet->setDataByName("soga_race_type", atoi(row[25]));
			else
				player->packet->setDataByName("soga_race_type", atoi(row[26]));
			player->packet->setDataByName("race_type", atoi(row[26]));

			player->packet->setDataByName("unknown3", 57);
			player->packet->setDataByName("unknown4", 56);
			player->packet->setDataByName("unknown6", 1, 1); //if not here will not display character
			player->packet->setDataByName("unknown8", 15);
			player->packet->setDataByName("unknown13", 212);
			player->packet->setColorByName("unknown14", 0xFF, 0xFF, 0xFF);

			uchar tmp[] = {0xFF, 0xFF, 0xFF, 0x61, 0x00, 0x2C, 0x04, 0xA5, 0x09, 0x02, 0x0F, 0x00, 0x00};
			for(size_t y=0;y<sizeof(tmp);y++)
				player->packet->setDataByName("unknown11", tmp[y], y);
			MYSQL_RES* result3 = query2.RunQuery2(Q_SELECT, "SELECT slot, equip_type, red, green, blue, highlight_red, highlight_green, highlight_blue from login_equipment where login_characters_id=%i order by slot",atoi(row[21]));
			if(result3){
				for(int i=0;(row3 = mysql_fetch_row(result3)) && i<24; i++){
					player->packet->setEquipmentByName("equip", atoi(row3[1]), atoi(row3[2]), atoi(row3[3]), atoi(row3[4]), atoi(row3[5]), atoi(row3[6]), atoi(row3[7]), atoi(row3[0]));
				}
			}			
			player->packet->setDataByName("mount", 1377);
			player->packet->setDataByName("mount_color1", 57);
			/*
			enum NetAppearance::NetAppearanceFlags
			{
				NAF_INVISIBLE=1,
				NAF_SHOW_HOOD=2
			};
			*/
			acct->addCharacter(player);
		}
	}
	else
		LogWrite(LOGIN__ERROR, 0, "Login", "Error in LoadCharacters query '%s': %s", query.GetQuery(), query.GetError());

}

void LoginDatabase::CheckCharacterTimeStamps(LoginAccount* acct){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT char_id, unix_timestamp from login_characters where account_id=%i",acct->getLoginAccountID());
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;

		ServerPacket* outpack = new ServerPacket(ServerOP_CharTimeStamp, sizeof(CharacterTimeStamp_Struct));
		CharacterTimeStamp_Struct* cts = (CharacterTimeStamp_Struct*) outpack->pBuffer;
		cts->account_id = acct->getLoginAccountID();
		int32 server_id = 0;
		LWorld* world_server = 0;
		while ((row = mysql_fetch_row(result))) {
			server_id = atoi(row[1]);
			if(server_id != 0)
			world_server = world_list.FindByAccount(server_id, World);
			if(world_server) // If the pointer is 0, the world server must be down, we can't do any updates...
			{
			cts->char_id = atoi(row[0]);
			cts->unix_timestamp = atoi(row[1]);
			world_server->SendPacket(outpack);
			//Reset for next character
			world_server = 0;
			server_id = 0;
			}
		}
		safe_delete(outpack);
	}
}

void LoginDatabase::SaveCharacterFloats(int32 char_id, char* type, float float1, float float2, float float3,float multiplier){
	Query query;
	string create_char = string("insert into login_char_colors (login_characters_id, type, red, green, blue, signed_value) values(%i,'%s',%i,%i,%i, 1)");
	query.RunQuery2(Q_INSERT, create_char.c_str(), char_id, type, (sint8)(float1*multiplier), (sint8)(float2*multiplier), (sint8)(float3*multiplier));
}

void LoginDatabase::SaveCharacterColors(int32 char_id, char* type, EQ2_Color color){
	Query query;
	string create_char = string("insert into login_char_colors (login_characters_id, type, red, green, blue) values(%i,'%s',%i,%i,%i)");
	query.RunQuery2(Q_INSERT, create_char.c_str(), char_id, type, color.red, color.green, color.blue);
}

void LoginDatabase::LoadAppearanceData(int32 char_id, PacketStruct* char_select_packet){
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT type, signed_value, red, green, blue from login_char_colors where login_characters_id = %i",char_id);
	while((row = mysql_fetch_row(result))){
		if(atoi(row[1]) == 0)
			char_select_packet->setColorByName(row[0], atoul(row[2]), atoul(row[3]), atoul(row[4]));
		else{
				char_select_packet->setDataByName(row[0], atoi(row[2]), 0);
				char_select_packet->setDataByName(row[0], atoi(row[3]), 1);
				char_select_packet->setDataByName(row[0], atoi(row[4]), 2);
		}
	}
}
int16 LoginDatabase::GetAppearanceID(string name){
	int32 id = 0;
	Query query;
	MYSQL_ROW row;
	query.escaped_name = getEscapeString(name.c_str());
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT appearance_id from appearances where name='%s'", query.escaped_name);
	if(result && mysql_num_rows(result) == 1){
		row = mysql_fetch_row(result);
		id = atoi(row[0]);
	}
	return id;
}

void LoginDatabase::DeactivateCharID(int32 server_id, int32 char_id, int32 exception_id){
	Query query;
	query.RunQuery2(Q_UPDATE, "update login_characters set deleted=1 where char_id=%u and server_id=%u and id!=%u",char_id,server_id,exception_id);
}

int32 LoginDatabase::SaveCharacter(PacketStruct* create, LoginAccount* acct, int32 world_charid, int32 client_version){
	int32 ret_id = 0;
	Query query;
	string create_char = 
		string("Insert into login_characters (account_id, server_id, char_id, name, race, class, gender, deity, body_size, body_age, soga_wing_type, soga_chest_type, soga_legs_type, soga_hair_type, soga_facial_hair_type, legs_type, chest_type, wing_type, hair_type, facial_hair_type, soga_model_type, model_type)" 
		" values(%i, %i, %i, '%s', %i, %i, %i, %i, %f, %f, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i)");
	query.RunQuery2(Q_INSERT, create_char.c_str(), 
		acct->getLoginAccountID(), 
		create->getType_int32_ByName("server_id"), world_charid, 
		create->getType_EQ2_16BitString_ByName("name").data.c_str(), 
		create->getType_int8_ByName("race"), 
		create->getType_int8_ByName("class"), 
		create->getType_int8_ByName("gender"), 
		create->getType_int8_ByName("deity"), 
		create->getType_float_ByName("body_size"), 
		create->getType_float_ByName("body_age"),
		GetAppearanceID(create->getType_EQ2_16BitString_ByName("soga_wing_file").data),
		GetAppearanceID(create->getType_EQ2_16BitString_ByName("soga_chest_file").data), 
		GetAppearanceID(create->getType_EQ2_16BitString_ByName("soga_legs_file").data), 
		GetAppearanceID(create->getType_EQ2_16BitString_ByName("soga_hair_file").data),
		GetAppearanceID(create->getType_EQ2_16BitString_ByName("soga_face_file").data), 
		GetAppearanceID(create->getType_EQ2_16BitString_ByName("legs_file").data), 
		GetAppearanceID(create->getType_EQ2_16BitString_ByName("chest_file").data),
		GetAppearanceID(create->getType_EQ2_16BitString_ByName("wing_file").data), 
		GetAppearanceID(create->getType_EQ2_16BitString_ByName("hair_file").data), 
		GetAppearanceID(create->getType_EQ2_16BitString_ByName("face_file").data),
		GetAppearanceID(create->getType_EQ2_16BitString_ByName("soga_race_file").data),
		GetAppearanceID(create->getType_EQ2_16BitString_ByName("race_file").data));
	if(query.GetError() && strlen(query.GetError()) > 0){
		LogWrite(LOGIN__ERROR, 0, "Login", "Error in SaveCharacter query '%s': %s", query.GetQuery(), query.GetError());
		return 0;
	}

	int32 last_insert_id = query.GetLastInsertedID();

	//mark any remaining characters with same id as deleted (creates problems if world deleted their db and started assigning new char ids)
	DeactivateCharID(create->getType_int32_ByName("server_id"), world_charid, last_insert_id);
	int32 char_id = last_insert_id;
	if (client_version <= 561) {
		float classic_multiplier = 250.0f;
		SaveCharacterFloats(char_id, "skin_color", create->getType_float_ByName("skin_color", 0), create->getType_float_ByName("skin_color", 1), create->getType_float_ByName("skin_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "eye_color", create->getType_float_ByName("eye_color", 0), create->getType_float_ByName("eye_color", 1), create->getType_float_ByName("eye_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_color1", create->getType_float_ByName("hair_color1", 0), create->getType_float_ByName("hair_color1", 1), create->getType_float_ByName("hair_color1", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_color2", create->getType_float_ByName("hair_color2", 0), create->getType_float_ByName("hair_color2", 1), create->getType_float_ByName("hair_color2", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_highlight", create->getType_float_ByName("hair_highlight", 0), create->getType_float_ByName("hair_highlight", 1), create->getType_float_ByName("hair_highlight", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_type_color", create->getType_float_ByName("hair_type_color", 0), create->getType_float_ByName("hair_type_color", 1), create->getType_float_ByName("hair_type_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_type_highlight_color", create->getType_float_ByName("hair_type_highlight_color", 0), create->getType_float_ByName("hair_type_highlight_color", 1), create->getType_float_ByName("hair_type_highlight_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_type_color", create->getType_float_ByName("hair_type_color", 0), create->getType_float_ByName("hair_type_color", 1), create->getType_float_ByName("hair_type_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_type_highlight_color", create->getType_float_ByName("hair_type_highlight_color", 0), create->getType_float_ByName("hair_type_highlight_color", 1), create->getType_float_ByName("hair_type_highlight_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_face_color", create->getType_float_ByName("hair_face_color", 0), create->getType_float_ByName("hair_face_color", 1), create->getType_float_ByName("hair_face_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "hair_face_highlight_color", create->getType_float_ByName("hair_face_highlight_color", 0), create->getType_float_ByName("hair_face_highlight_color", 1), create->getType_float_ByName("hair_face_highlight_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "shirt_color", create->getType_float_ByName("shirt_color", 0), create->getType_float_ByName("shirt_color", 1), create->getType_float_ByName("shirt_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "unknown_chest_color", create->getType_float_ByName("unknown_chest_color", 0), create->getType_float_ByName("unknown_chest_color", 1), create->getType_float_ByName("unknown_chest_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "pants_color", create->getType_float_ByName("pants_color", 0), create->getType_float_ByName("pants_color", 1), create->getType_float_ByName("pants_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "unknown_legs_color", create->getType_float_ByName("unknown_legs_color", 0), create->getType_float_ByName("unknown_legs_color", 1), create->getType_float_ByName("unknown_legs_color", 2), classic_multiplier);
		SaveCharacterFloats(char_id, "unknown9", create->getType_float_ByName("unknown9", 0), create->getType_float_ByName("unknown9", 1), create->getType_float_ByName("unknown9", 2), classic_multiplier);
	}
	else {
		SaveCharacterColors(char_id, "skin_color", create->getType_EQ2_Color_ByName("skin_color"));
		SaveCharacterColors(char_id, "model_color", create->getType_EQ2_Color_ByName("model_color"));
		SaveCharacterColors(char_id, "eye_color", create->getType_EQ2_Color_ByName("eye_color"));
		SaveCharacterColors(char_id, "hair_color1", create->getType_EQ2_Color_ByName("hair_color1"));
		SaveCharacterColors(char_id, "hair_color2", create->getType_EQ2_Color_ByName("hair_color2"));
		SaveCharacterColors(char_id, "hair_highlight", create->getType_EQ2_Color_ByName("hair_highlight"));
		SaveCharacterColors(char_id, "hair_type_color", create->getType_EQ2_Color_ByName("hair_type_color"));
		SaveCharacterColors(char_id, "hair_type_highlight_color", create->getType_EQ2_Color_ByName("hair_type_highlight_color"));
		SaveCharacterColors(char_id, "hair_face_color", create->getType_EQ2_Color_ByName("hair_face_color"));
		SaveCharacterColors(char_id, "hair_face_highlight_color", create->getType_EQ2_Color_ByName("hair_face_highlight_color"));
		SaveCharacterColors(char_id, "wing_color1", create->getType_EQ2_Color_ByName("wing_color1"));
		SaveCharacterColors(char_id, "wing_color2", create->getType_EQ2_Color_ByName("wing_color2"));
		SaveCharacterColors(char_id, "shirt_color", create->getType_EQ2_Color_ByName("shirt_color"));
		SaveCharacterColors(char_id, "unknown_chest_color", create->getType_EQ2_Color_ByName("unknown_chest_color"));
		SaveCharacterColors(char_id, "pants_color", create->getType_EQ2_Color_ByName("pants_color"));
		SaveCharacterColors(char_id, "unknown_legs_color", create->getType_EQ2_Color_ByName("unknown_legs_color"));
		SaveCharacterColors(char_id, "unknown9", create->getType_EQ2_Color_ByName("unknown9"));		

		SaveCharacterColors(char_id, "soga_skin_color", create->getType_EQ2_Color_ByName("soga_skin_color"));
		SaveCharacterColors(char_id, "soga_model_color", create->getType_EQ2_Color_ByName("soga_model_color"));
		SaveCharacterColors(char_id, "soga_eye_color", create->getType_EQ2_Color_ByName("soga_eye_color"));
		SaveCharacterColors(char_id, "soga_hair_color1", create->getType_EQ2_Color_ByName("soga_hair_color1"));
		SaveCharacterColors(char_id, "soga_hair_color2", create->getType_EQ2_Color_ByName("soga_hair_color2"));
		SaveCharacterColors(char_id, "soga_hair_highlight", create->getType_EQ2_Color_ByName("soga_hair_highlight"));
		SaveCharacterColors(char_id, "soga_hair_type_color", create->getType_EQ2_Color_ByName("soga_hair_type_color"));
		SaveCharacterColors(char_id, "soga_hair_type_highlight_color", create->getType_EQ2_Color_ByName("soga_hair_type_highlight_color"));
		SaveCharacterColors(char_id, "soga_hair_face_color", create->getType_EQ2_Color_ByName("soga_hair_face_color"));
		SaveCharacterColors(char_id, "soga_hair_face_highlight_color", create->getType_EQ2_Color_ByName("soga_hair_face_highlight_color"));
		SaveCharacterColors(char_id, "soga_wing_color1", create->getType_EQ2_Color_ByName("soga_wing_color1"));
		SaveCharacterColors(char_id, "soga_wing_color2", create->getType_EQ2_Color_ByName("soga_wing_color2"));
		SaveCharacterColors(char_id, "soga_shirt_color", create->getType_EQ2_Color_ByName("soga_shirt_color"));
		SaveCharacterColors(char_id, "soga_unknown_chest_color", create->getType_EQ2_Color_ByName("soga_unknown_chest_color"));
		SaveCharacterColors(char_id, "soga_pants_color", create->getType_EQ2_Color_ByName("soga_pants_color"));
		SaveCharacterColors(char_id, "soga_unknown_legs_color", create->getType_EQ2_Color_ByName("soga_unknown_legs_color"));
		SaveCharacterColors(char_id, "soga_unknown13", create->getType_EQ2_Color_ByName("soga_unknown13"));
		SaveCharacterFloats(char_id, "soga_eye_type", create->getType_float_ByName("soga_eyes2", 0), create->getType_float_ByName("soga_eyes2", 1), create->getType_float_ByName("soga_eyes2", 2));
		SaveCharacterFloats(char_id, "soga_ear_type", create->getType_float_ByName("soga_ears", 0), create->getType_float_ByName("soga_ears", 1), create->getType_float_ByName("soga_ears", 2));
		SaveCharacterFloats(char_id, "soga_eye_brow_type", create->getType_float_ByName("soga_eye_brows", 0), create->getType_float_ByName("soga_eye_brows", 1), create->getType_float_ByName("soga_eye_brows", 2));
		SaveCharacterFloats(char_id, "soga_cheek_type", create->getType_float_ByName("soga_cheeks", 0), create->getType_float_ByName("soga_cheeks", 1), create->getType_float_ByName("soga_cheeks", 2));
		SaveCharacterFloats(char_id, "soga_lip_type", create->getType_float_ByName("soga_lips", 0), create->getType_float_ByName("soga_lips", 1), create->getType_float_ByName("soga_lips", 2));
		SaveCharacterFloats(char_id, "soga_chin_type", create->getType_float_ByName("soga_chin", 0), create->getType_float_ByName("soga_chin", 1), create->getType_float_ByName("soga_chin", 2));
		SaveCharacterFloats(char_id, "soga_nose_type", create->getType_float_ByName("soga_nose", 0), create->getType_float_ByName("soga_nose", 1), create->getType_float_ByName("soga_nose", 2));
	}
	SaveCharacterFloats(char_id, "eye_type", create->getType_float_ByName("eyes2", 0), create->getType_float_ByName("eyes2", 1), create->getType_float_ByName("eyes2", 2));
	SaveCharacterFloats(char_id, "ear_type", create->getType_float_ByName("ears", 0), create->getType_float_ByName("ears", 1), create->getType_float_ByName("ears", 2));
	SaveCharacterFloats(char_id, "eye_brow_type", create->getType_float_ByName("eye_brows", 0), create->getType_float_ByName("eye_brows", 1), create->getType_float_ByName("eye_brows", 2));
	SaveCharacterFloats(char_id, "cheek_type", create->getType_float_ByName("cheeks", 0), create->getType_float_ByName("cheeks", 1), create->getType_float_ByName("cheeks", 2));
	SaveCharacterFloats(char_id, "lip_type", create->getType_float_ByName("lips", 0), create->getType_float_ByName("lips", 1), create->getType_float_ByName("lips", 2));
	SaveCharacterFloats(char_id, "chin_type", create->getType_float_ByName("chin", 0), create->getType_float_ByName("chin", 1), create->getType_float_ByName("chin", 2));
	SaveCharacterFloats(char_id, "nose_type", create->getType_float_ByName("nose", 0), create->getType_float_ByName("nose", 1), create->getType_float_ByName("nose", 2));
	SaveCharacterFloats(char_id, "body_size", create->getType_float_ByName("body_size", 0), 0, 0);
	return ret_id;
}

bool LoginDatabase::DeleteCharacter(int32 account_id, int32 character_id, int32 server_id){
	Query query;
	string delete_char = string("delete from login_characters where char_id=%i and account_id=%i and server_id=%i");
	query.RunQuery2(Q_DELETE, delete_char.c_str(),character_id,account_id,server_id);
	if(!query.GetAffectedRows())
	{
		//No error just in case ppl try doing stupid stuff
		return false;
	}

	return true;
}

string LoginDatabase::GetCharacterName(int32 char_id, int32 server_id, int32 account_id){
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT name from login_characters where char_id=%lu and server_id=%lu and account_id=%lu and deleted = 0 limit 1", char_id, server_id, account_id);

	if(result && mysql_num_rows(result) == 1){
		row = mysql_fetch_row(result);
		return string(row[0]);
	}
	return string("");
}

bool LoginDatabase::UpdateCharacterTimeStamp(int32 account_id, int32 character_id, int32 timestamp_update, int32 server_id){
	Query query;
	string update_charts = string("update login_characters set unix_timestamp=%lu where char_id=%lu and account_id=%lu and server_id=%lu");
	query.RunQuery2(Q_UPDATE, update_charts.c_str(),timestamp_update,character_id,account_id,server_id);
	if(!query.GetAffectedRows())
	{
		LogWrite(LOGIN__ERROR, 0, "Login", "Error in UpdateCharacterTimeStamp query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}

	return true;
}

bool LoginDatabase::UpdateCharacterLevel(int32 account_id, int32 character_id, int8 in_level, int32 server_id){
	Query query;
	string update_charts = string("update login_characters set level=%i where char_id=%lu and account_id=%lu and server_id=%lu");
	query.RunQuery2(Q_UPDATE, update_charts.c_str(),in_level,character_id,account_id,server_id);
	if(!query.GetAffectedRows())
	{
		LogWrite(LOGIN__ERROR, 0, "Login", "Error in UpdateCharacterLevel query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}

	return true;
}

bool LoginDatabase::UpdateCharacterRace(int32 account_id, int32 character_id, int16 in_racetype, int8 in_race, int32 server_id){
	Query query;
	string update_charts = string("update login_characters set race_type=%i, race=%i where char_id=%lu and account_id=%lu and server_id=%lu");
	query.RunQuery2(Q_UPDATE, update_charts.c_str(),in_racetype,in_race,character_id,account_id,server_id);
	if(!query.GetAffectedRows())
	{
		LogWrite(LOGIN__ERROR, 0, "Login", "Error in UpdateCharacterRace query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}

	return true;
}

bool LoginDatabase::UpdateCharacterZone(int32 account_id, int32 character_id, int32 zone_id, int32 server_id){
	Query query;
	string update_chars = string("update login_characters set current_zone_id=%i where char_id=%lu and account_id=%lu and server_id=%lu");
	query.RunQuery2(Q_UPDATE, update_chars.c_str(), zone_id, character_id, account_id, server_id);
	if(!query.GetAffectedRows())
	{
		LogWrite(LOGIN__ERROR, 0, "Login", "Error in UpdateCharacterZone query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}

	return true;
}

bool LoginDatabase::UpdateCharacterClass(int32 account_id, int32 character_id, int8 in_class, int32 server_id){
	Query query;
	string update_charts = string("update login_characters set class=%i where char_id=%lu and account_id=%lu and server_id=%lu");
	query.RunQuery2(Q_UPDATE, update_charts.c_str(),in_class,character_id,account_id,server_id);
	if(!query.GetAffectedRows())
	{
		LogWrite(LOGIN__ERROR, 0, "Login", "Error in UpdateCharacterClass query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}

	return true;
}

bool LoginDatabase::UpdateCharacterGender(int32 account_id, int32 character_id, int8 in_gender, int32 server_id){
	Query query;
	string update_charts = string("update login_characters set gender=%i where char_id=%lu and account_id=%lu and server_id=%lu");
	query.RunQuery2(Q_UPDATE, update_charts.c_str(),in_gender,character_id,account_id,server_id);
	if(!query.GetAffectedRows())
	{
		LogWrite(LOGIN__ERROR, 0, "Login", "Error in UpdateCharacterClass query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}

	return true;
}

LoginAccount* LoginDatabase::LoadAccount(const char* name, const char* password, bool attemptAccountCreation){
	LoginAccount* acct = NULL;
	Query query;
	query.escaped_name = getEscapeString(name);
	query.escaped_pass = getEscapeString(password);
	time_t now = time(0); //get the current epoc time
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id from account where name='%s' and passwd=sha2('%s',512)", query.escaped_name, query.escaped_pass);
	if(result){
		if (mysql_num_rows(result) == 1){
			row = mysql_fetch_row(result);
			
			int32 id = atol(row[0]);

			acct = new LoginAccount(id, name, password);
			acct->setAuthenticated(true);
		}
		else if(mysql_num_rows(result) > 0)
			LogWrite(LOGIN__ERROR, 0, "Login", "Error in LoginAccount: more than one account returned for '%s'", name);
		else if (attemptAccountCreation && !database.GetAccountIDByName(name))
		{
			Query newquery;
			newquery.RunQuery2(Q_INSERT, "insert into account set name='%s',passwd=sha2('%s',512), created_date=%i", query.escaped_name, query.escaped_pass, now);
			// re-run the query for select only not account creation
			return LoadAccount(name, password, false);
		}

	}
	return acct;
}

int32 LoginDatabase::GetAccountIDByName(const char* name) {
	int32 id = 0;
	Query query;
	MYSQL_ROW row;
	query.escaped_name = getEscapeString(name);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id from account where name='%s'", query.escaped_name);
	if (result && mysql_num_rows(result) == 1) {
		row = mysql_fetch_row(result);
		id = atoi(row[0]);
	}
	return id;
}

int32 LoginDatabase::CheckServerAccount(char* name, char* passwd){
	int32 id = 0;
	Query query;
	MYSQL_ROW row;
	query.escaped_name = getEscapeString(name);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT lower(password), id from login_worldservers where account='%s' and disabled = 0", query.escaped_name);

	LogWrite(LOGIN__INFO, 0, "Login", "WorldServer CheckServerAccount Account=%s\nSHA=%s", (char*)query.escaped_name, passwd);
	if(result && mysql_num_rows(result) == 1){
		row = mysql_fetch_row(result);

		LogWrite(LOGIN__INFO, 0, "Login", "WorldServer CheckServerAccountResult Account=%s\nPassword=%s", (char*)query.escaped_name, (row && row[0]) ? row[0] : "(NULL)");

		if (memcmp(row[0], passwd, strnlen(row[0], 256)) == 0)
		{
			LogWrite(LOGIN__INFO, 0, "Login", "WorldServer CheckServerAccountResultMatch Account=%s", (char*)query.escaped_name);
			id = atoi(row[1]);
		}
	}
	return id;
}

bool LoginDatabase::IsServerAccountDisabled(char* name){
	Query query;
	MYSQL_ROW row;
	query.escaped_name = getEscapeString(name);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id from login_worldservers where account='%s' and disabled = 1", query.escaped_name);

	LogWrite(LOGIN__DEBUG, 0, "Login", "WorldServer IsServerAccountDisabled Account=%s", (char*)query.escaped_name);
	if(result && mysql_num_rows(result) > 0){
		row = mysql_fetch_row(result);

		LogWrite(LOGIN__INFO, 0, "Login", "WorldServer IsServerAccountDisabled Match Account=%s", (char*)query.escaped_name);

		return true;
	}
	return false;
}

bool LoginDatabase::IsIPBanned(char* ipaddr){
	if(!ipaddr)
		return false;

	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT ip from login_bannedips where '%s' LIKE CONCAT(ip ,'%%')", ipaddr);

	LogWrite(LOGIN__DEBUG, 0, "Login", "WorldServer IsServerIPBanned IPPartial=%s", (char*)ipaddr);
	if(result && mysql_num_rows(result) > 0){
		row = mysql_fetch_row(result);

		LogWrite(LOGIN__INFO, 0, "Login", "WorldServer IsServerIPBanned Match IPBan=%s", row[0]);

		return true;
	}
	return false;
}

void LoginDatabase::GetServerAccounts(vector<LWorld*>* server_list){
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, account, name, admin_id from login_worldservers");
	while((row = mysql_fetch_row(result))){
		LWorld* world = new LWorld(atol(row[0]), row[1], row[2], atoi(row[3]));
		world->SetID(world->GetAccountID());
		server_list->push_back(world);
	}
}
void LoginDatabase::SaveClientLog(const char* type, const char* message, const char* player_name, int16 version){
	Query query;
	query.escaped_data1 = getEscapeString(message);
	query.escaped_name = getEscapeString(player_name);
	query.RunQuery2(Q_INSERT, "insert into log_messages (type, message, name, version) values('%s', '%s', '%s', %i)", type, query.escaped_data1, query.escaped_name, version); 
}
bool  LoginDatabase::VerifyDelete(int32 account_id, int32 character_id, const char* name){
	Query query;
	query.escaped_name = getEscapeString(name);
	query.RunQuery2(Q_UPDATE, "update login_characters set deleted = 1 where char_id=%i and account_id=%i and name='%s'", character_id, account_id, query.escaped_name);
	if(query.GetAffectedRows() == 1)
		return true;
	else
		return false;
}
char* LoginDatabase::GetServerAccountName(int32 id){
	Query query;
	MYSQL_ROW row;
	char* name = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT name from login_worldservers where id=%lu", id);
	if(result && mysql_num_rows(result) == 1){
		row = mysql_fetch_row(result);
		if(strlen(row[0]) > 0){
			name = new char[strlen(row[0])+1];
			strcpy(name, row[0]);
		}
	}
	return name;
}
int32 LoginDatabase::GetRaceID(char* name){
	int32 ret = 1487;
	Query query;
	MYSQL_ROW row;
	query.escaped_name = getEscapeString(name);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT race_type from login_races where name='%s'", query.escaped_name);
	if(result && mysql_num_rows(result) == 1){
		row = mysql_fetch_row(result);
		ret = atol(row[0]);
	}
	else if(!result || mysql_num_rows(result) == 0)
		UpdateRaceID(query.escaped_name);
	return ret;
}
void LoginDatabase::UpdateRaceID(char* name){
	Query query;
	query.RunQuery2(Q_UPDATE, "insert into login_races (name) values('%s')", name);
}
bool LoginDatabase::CheckVersion(char* in_version){
	Query query;
	query.escaped_data1 = getEscapeString(in_version);
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id from login_versions where version='%s' or version='*'", query.escaped_data1);
	if(result && mysql_num_rows(result) > 0)
		return true;
	else
		return false;
}
void LoginDatabase::GetLatestTableVersions(LatestTableVersions* table_versions){
	Query query;
	MYSQL_ROW row;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT name, max(version) from login_table_versions group by name order by id");
	if(result && mysql_num_rows(result) > 0){
		table_versions->SetTableSize(mysql_num_rows(result));
	}
	else // we need to return if theres no result, otherwise it will crash attempting to loop through rows

		return;
	while((row = mysql_fetch_row(result))){
		if(VerifyDataTable(row[0]))
			table_versions->AddTable(row[0], atoi(row[1]), GetDataVersion(row[0]));
		else
			table_versions->AddTable(row[0], atoi(row[1]), 0);
	}
}
bool LoginDatabase::VerifyDataTable(char* name){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT table_name from download_tables where table_name='%s'", name);
	if(result && mysql_num_rows(result) > 0)
		return true;
	return false;
}
string LoginDatabase::GetColumnNames(char* name){
	Query query;
	MYSQL_ROW row;
	string columns = "(";
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "show columns from %s", name);
	if(result && mysql_num_rows(result) > 0){
		int16 i = 0;
		while((row = mysql_fetch_row(result))){
			if(strcmp(row[0], "table_data_version") != 0){
				if(i>0)
					columns.append(",");
				columns.append(row[0]);
				i++;
			}
		}
	}
	columns.append(") ");
	return columns;
}
TableDataQuery* LoginDatabase::GetTableDataQuery(int32 server_ip, char* name, int16 version){
	Query query;
	MYSQL_ROW row;
	query.escaped_name = getEscapeString(name);
	TableDataQuery* table_query = 0;
	MYSQL_RES* result = 0;
	string columns;

	if(VerifyDataTable(query.escaped_name)){
		result = query.RunQuery2(Q_SELECT, "SELECT * from %s where table_data_version > %i", query.escaped_name, version);
		columns = GetColumnNames(query.escaped_name);
	}
	if(result && mysql_num_rows(result) > 0){
		table_query = new TableDataQuery(query.escaped_name);
		table_query->num_queries = mysql_num_rows(result);
		table_query->columns_size = columns.length() + 1;
		table_query->columns = new char[table_query->columns_size + 1];
		table_query->version = GetDataVersion(query.escaped_name);
		strcpy(table_query->columns, (char*)columns.c_str());
		string query_data;
		MYSQL_FIELD* field;
		int* int_list = new int[mysql_num_fields(result)];
		int16 ndx = 0;
		while((field = mysql_fetch_field(result))){
			int_list[ndx] = IS_NUM(field->type);
			if(strcmp(field->name,"table_data_version") == 0)
				int_list[ndx] = 2;
			ndx++;
		}
		ndx = 0;
		while((row = mysql_fetch_row(result))){
			query_data = "";
			for(int i=0;i<mysql_num_fields(result);i++){
				if(int_list[i]<2){
					if(i>0)
						query_data.append(",");
					if(!int_list[i]){
						query_data.append("'").append(getEscapeString(row[i])).append("'");
					}
					else
						query_data.append(row[i]);
				}
			}
			TableData* new_query = new TableData;
			new_query->size = query_data.length() + 1;
			new_query->query = new char[query_data.length() + 1];
			strcpy(new_query->query, query_data.c_str());
			table_query->queries.push_back(new_query);
			ndx++;
		}
		safe_delete_array(int_list);
	}
	else{
		string query2 = string("The user tried to download the following table: ").append(query.escaped_name);
		SaveClientLog("Possible Hacking Attempt",  (char*)query2.c_str(), "Hacking Data", server_ip);
	}
	return table_query;	
}
TableQuery* LoginDatabase::GetLatestTableQuery(int32 server_ip, char* name, int16 version){
	Query query;
	MYSQL_ROW row;
	query.escaped_name = getEscapeString(name);
	TableQuery* table_query = 0;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT query, version from login_table_versions where name = '%s' and version>=%i order by version", query.escaped_name, version + 1);
	if(result && mysql_num_rows(result) > 0){
		int16 i = 0;
		table_query = new TableQuery;
		while((row = mysql_fetch_row(result))){
			char* rowdata = row[0];
			if(strstr(rowdata, ";")){
				char* token = strtok(rowdata,";");
				while(token){
					char* new_query = new char[strlen(token) + 1];
					strcpy(new_query, token);
					table_query->AddQuery(new_query);
					token = strtok(NULL, ";");
				}
			}
			else
				table_query->AddQuery(rowdata);
			table_query->latest_version = atoi(row[1]);
		}
		strcpy(table_query->tablename, name);
		table_query->your_version = version;
	}
	else{
		string query2 = string("The following was the DB Query: ").append(query.GetQuery());
		SaveClientLog("Possible Hacking Attempt",  (char*)query2.c_str(), "Hacking Query", server_ip);
	}
	return table_query;
}
sint16 LoginDatabase::GetDataVersion(char* name){
	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT max(table_data_version) from %s", name);
	sint16 ret_version = 0;
	if(result && mysql_num_rows(result) > 0) {
		MYSQL_ROW row;
		row = mysql_fetch_row(result);
		if(row[0])
			ret_version = atoi(row[0]);
	}
	return ret_version;
}

void LoginDatabase::RemoveOldWorldServerStats(){
	Query query;
	query.RunQuery2(Q_DELETE, "delete from login_worldstats where (UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(last_update)) > 86400");
}


void LoginDatabase::UpdateWorldServerStats(LWorld* world, sint32 status)
{
	Query query;
	query.RunQuery2(Q_INSERT, "insert into login_worldstats (world_id, world_status, current_players, current_zones, last_update, world_max_level) values(%u, %i, %i, %i, NOW(), %i) ON DUPLICATE KEY UPDATE current_players=%i,current_zones=%i,world_max_level=%i,world_status=%i,last_update=NOW()",
		world->GetAccountID(), status, world->GetPlayerNum(), world->GetZoneNum(), world->GetMaxWorldLevel(), world->GetPlayerNum(), world->GetZoneNum(), world->GetMaxWorldLevel(), status);

	string update_stats = string("update login_worldservers set lastseen=%u where id=%i");
	query.RunQuery2(Q_UPDATE, update_stats.c_str(), Timer::GetUnixTimeStamp(), world->GetAccountID());
}

bool LoginDatabase::ResetWorldServerStatsConnectedTime(LWorld* world){
	if(!world || world->GetAccountID() == 0)
		return false;

	Query query;
	string update_stats = string("update login_worldstats set connected_time=now() where world_id=%i and (UNIX_TIMESTAMP(NOW())-UNIX_TIMESTAMP(last_update)) > 300");
	query.RunQuery2(Q_UPDATE, update_stats.c_str(),world->GetAccountID());

	return true;
}

void LoginDatabase::ResetWorldStats ( )
{
	Query query;
	string update_stats = string("update login_worldstats set world_status=-4, current_players=0, current_zones=0");
	query.RunQuery2(update_stats.c_str(), Q_UPDATE);
}

void LoginDatabase::SaveBugReport(int32 world_id, char* category, char* subcategory, char* causes_crash, char* reproducible, char* summary, char* description, char* version, char* player, int32 account_id, char* spawn_name, int32 spawn_id, int32 zone_id){
	Query query;
	string bug_report = string("insert into bugs (world_id, category, subcategory, causes_crash, reproducible, summary, description, version, player, account_id, spawn_name, spawn_id, zone_id) values(%lu, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %lu, '%s', %lu, %lu)");
	query.RunQuery2(Q_INSERT, bug_report.c_str(), world_id, database.getSafeEscapeString(category).c_str(), database.getSafeEscapeString(subcategory).c_str(),
		database.getSafeEscapeString(causes_crash).c_str(), database.getSafeEscapeString(reproducible).c_str(), database.getSafeEscapeString(summary).c_str(),
		database.getSafeEscapeString(description).c_str(), database.getSafeEscapeString(version).c_str(), database.getSafeEscapeString(player).c_str(), account_id,
		database.getSafeEscapeString(spawn_name).c_str(), spawn_id, zone_id);
	FixBugReport();
}

void LoginDatabase::FixBugReport(){
	Query query;
	string bug_report = string("update bugs set description = REPLACE(description,SUBSTRING(description,INSTR(description,'%'), 3),char(CONV(SUBSTRING(description,INSTR(description,'%')+1, 2), 16, 10))), summary = REPLACE(summary,SUBSTRING(summary,INSTR(summary,'%'), 3),char(CONV(SUBSTRING(summary,INSTR(summary,'%')+1, 2), 16, 10)))");
	query.RunQuery2(bug_report.c_str(), Q_UPDATE);
}

void LoginDatabase::UpdateWorldIPAddress(int32 world_id, int32 address){
	struct in_addr	in;
	in.s_addr = address;
	Query query;
	query.RunQuery2(Q_UPDATE, "update login_worldservers set ip_address='%s' where id=%lu", inet_ntoa(in), world_id);
}

void LoginDatabase::UpdateAccountIPAddress(int32 account_id, int32 address){
	struct in_addr	in;
	in.s_addr = address;
	Query query;
	query.RunQuery2(Q_UPDATE, "update account set ip_address='%s' where id=%lu", inet_ntoa(in), account_id);
}

//devn00b: There is no rulesystem for login, so im going to use login_config for future things like this.
//devn00b: Returns the number of characters a player may create per account. This should be set by server owners -> login,
//devn00b: However, better semi-working for now than not working at all.
//devn00b: TODO: EQ2World sends max char per acct.
int8 LoginDatabase::GetMaxCharsSetting() {
	//live defaults to 7 for GOLD members.
	int8 max_chars = 7;
	Query query;
	MYSQL_ROW row;
	
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "select config_value from login_config where config_name='max_characters_per_account'");
	if (result && mysql_num_rows(result) == 1) {
		row = mysql_fetch_row(result);
		if (row[0])
			max_chars = atoi(row[0]);
	}
	//if nothing else return the default.
	return max_chars;
}

int16 LoginDatabase::GetAccountBonus(int32 acct_id) {
	int32 bonus = 0;
	int16 world_id = 0;
	Query query;
	MYSQL_ROW row;
	Query query2;
	MYSQL_ROW row2;

	//get the world ID for the character. TODO: Support multi server characters.
	MYSQL_RES* result2 = query2.RunQuery2(Q_SELECT, "select server_id from login_characters where account_id=%i", acct_id);

	if (result2 && mysql_num_rows(result2) >= 1) {
		row2 = mysql_fetch_row(result2);
		if (row2[0])
			world_id = atoi(row2[0]);
	}
	
	//pull all characters greater than the max level from the server
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT COUNT(id) FROM login_characters WHERE LEVEL >= (select world_max_level from login_worldstats where world_id=%i) AND account_id=%i", world_id, acct_id);

	if (result && mysql_num_rows(result) == 1) {
		row = mysql_fetch_row(result);
		if(row[0])
			bonus = atoi(row[0]);
	}
	return bonus;
}

void LoginDatabase::UpdateWorldVersion(int32 world_id, char* version) {
	Query query;
	query.RunQuery2(Q_UPDATE, "update login_worldservers set login_version='%s' where id=%u", version, world_id);
}

void LoginDatabase::UpdateAccountClientDataVersion(int32 account_id, int16 version)
{
	Query query;
	query.RunQuery2(Q_UPDATE, "UPDATE account SET last_client_version='%i' WHERE id = %u", version, account_id);
}

//devn00b todo: finish this.
void LoginDatabase::SaveCharacterPicture(int32 account_id, int32 character_id, int32 server_id, int16 picture_size, uchar* picture) {
	stringstream ss_hex;
	stringstream ss_query;
	ss_hex.flags(ios::hex);
	for (int32 i = 0; i < picture_size; i++)
		ss_hex << setfill('0') << setw(2) << (int32)picture[i];

	ss_query << "INSERT INTO `ls_character_picture` (`server_id`, `account_id`, `character_id`, `picture`) VALUES (" << server_id << ", " << account_id << ", " << character_id << ", '" << ss_hex.str() << "') ON DUPLICATE KEY UPDATE `picture` = '" << ss_hex.str() << "'";

	if (!dbLogin.Query(ss_query.str().c_str()))
		LogWrite(DATABASE__ERROR, 0, "DBNew", "MySQL Error %u: %s", dbLogin.GetError(), dbLogin.GetErrorMsg());
}