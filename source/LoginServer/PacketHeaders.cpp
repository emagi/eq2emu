/*  
	EQ2Emulator:  Everquest II Server Emulator
	Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

	This file is part of EQ2Emulator.
*/
#include "PacketHeaders.h"
#include "../common/MiscFunctions.h"
#include "LoginDatabase.h"
#include "LWorld.h"

extern LWorldList	world_list;
extern LoginDatabase database;

void LS_DeleteCharacterRequest::loadData(EQApplicationPacket* packet){
	InitializeLoadData(packet->pBuffer, packet->size);
	LoadData(character_number);
	LoadData(server_id);
	LoadData(spacer);
	LoadDataString(name);
}

EQ2Packet* LS_CharSelectList::serialize(int16 version){
	Clear();
	AddData(num_characters);
	AddData(char_data);
	if (version <= 561) {
		LS_CharListAccountInfoEarlyClient account_info;
		account_info.account_id = account_id;
		account_info.unknown1 = 0xFFFFFFFF;
		account_info.unknown2 = 0;
		account_info.maxchars = 7; //live has a max of 7 on gold accounts base.	
		account_info.unknown4 = 0;
		AddData(account_info);
	}
	else {
		LS_CharListAccountInfo account_info;
		account_info.account_id = account_id;
		account_info.unknown1 = 0xFFFFFFFF;
		account_info.unknown2 = 0;
		account_info.maxchars = database.GetMaxCharsSetting(); 
		account_info.vet_adv_bonus = database.GetAccountBonus(account_id);
		account_info.vet_trade_bonus = 0;
		account_info.unknown4 = 0;
		for (int i = 0; i < 3; i++)
			account_info.unknown5[i] = 0xFFFFFFFF;
		account_info.unknown5[3] = 0;

		AddData(account_info);
	}	
	return new EQ2Packet(OP_AllCharactersDescReplyMsg, getData(), getDataSize());
}

void LS_CharSelectList::addChar(uchar* data, int16 size){
	char_data.append((char*)data, size);
}

void LS_CharSelectList::loadData(int32 account, vector<CharSelectProfile*> charlist, int16 version){
	vector<CharSelectProfile*>::iterator itr;
	account_id = account;
	num_characters = 0;
	char_data = "";
	CharSelectProfile* character = 0;
	for(itr = charlist.begin();itr != charlist.end();itr++){
		character = *itr;
		int32 serverID = character->packet->getType_int32_ByName("server_id");
		if(character->deleted) { // workaround for old clients <= 561 that crash if you delete a char (Doesn't refresh the char panel correctly)
			character->packet->setDataByName("name", "(deleted)");
			character->packet->setDataByName("charid", 0xFFFFFFFF);
			character->packet->setDataByName("name", 0xFFFFFFFF);
			character->packet->setDataByName("server_id", 0xFFFFFFFF);
			character->packet->setDataByName("created_date", 0xFFFFFFFF);
			character->packet->setDataByName("unknown1", 0xFFFFFFFF);
			character->packet->setDataByName("unknown2", 0xFFFFFFFF);
			character->packet->setDataByName("flags", 0xFF);
		}
		else if(serverID == 0 || !world_list.FindByID(serverID))
			continue;
		num_characters++;		
		character->SaveData(version);
		addChar(character->getData(), character->getDataSize());
	}
}

void CharSelectProfile::SaveData(int16 in_version){
	Clear();
	AddData(*packet->serializeString());
}
