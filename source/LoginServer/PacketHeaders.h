/*  
	EQ2Emulator:  Everquest II Server Emulator
	Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

	This file is part of EQ2Emulator.
*/
#ifndef __PACKET_HEADERS__
#define __PACKET_HEADERS__

#include "../common/types.h"
#include "../common/EQPacket.h"
#include "../common/EQ2_Common_Structs.h"
#include "login_structs.h"
#include "../common/DataBuffer.h"
#include "../common/GlobalHeaders.h"
#include "../common/ConfigReader.h"
#include <vector>

extern ConfigReader configReader;

class CharSelectProfile : public DataBuffer{
public:
	CharSelectProfile(int16 version){
		deleted = false;
		packet = configReader.getStruct("CharSelectProfile",version);
		for(int8 i=0;i<24;i++){
			packet->setEquipmentByName("equip",0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,i);
		}
	}

	~CharSelectProfile(){
		safe_delete(packet);
	}
	PacketStruct*		packet;

	void				SaveData(int16 in_version);
	void				Data();
	int16				size;
	bool				deleted;
};

class LS_CharSelectList : public DataBuffer {
public:
	int8					num_characters;
	int32					account_id;
	
	EQ2Packet*			serialize(int16 version);
	void					addChar(uchar* data, int16 size);
	string					char_data;
	void					loadData(int32 account, vector<CharSelectProfile*> charlist, int16 version);
};

class LS_DeleteCharacterRequest : public DataBuffer{
public:
	int32			character_number;
	int32			server_id;
	int32			spacer;
	EQ2_16BitString name;
	void			loadData(EQApplicationPacket* packet);
};
#endif