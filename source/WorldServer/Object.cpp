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
#include "World.h"
#include "Object.h"
#include "Spells.h"

extern World world;
extern ConfigReader configReader;
extern MasterSpellList master_spell_list;

Object::Object(){ 
	clickable = false; 
	zone_name = 0;
	packet_num = 0;
	appearance.activity_status = 64;
	appearance.pos.state = 1;
	appearance.difficulty = 0;
	spawn_type = 2;
	m_deviceID = 0;
}
Object::~Object(){

}

EQ2Packet* Object::serialize(Player* player, int16 version){
	return spawn_serialize(player, version);
}

void Object::HandleUse(Client* client, string command){
	vector<TransportDestination*> destinations;
	if(GetTransporterID() > 0)
		GetZone()->GetTransporters(&destinations, client, GetTransporterID());
	if (destinations.size())
	{
		client->SetTemporaryTransportID(0);
		client->ProcessTeleport(this, &destinations, GetTransporterID());
	}
	else if (client && command.length() > 0 && appearance.show_command_icon == 1 && MeetsSpawnAccessRequirements(client->GetPlayer())){
		EntityCommand* entity_command = FindEntityCommand(command);
		if (entity_command)
			client->GetCurrentZone()->ProcessEntityCommand(entity_command, client->GetPlayer(), client->GetPlayer()->GetTarget());
	}
}

Object*	Object::Copy(){
	Object* new_spawn = new Object();
	new_spawn->SetCollector(IsCollector());
	new_spawn->SetMerchantID(merchant_id);
	new_spawn->SetMerchantType(merchant_type);
	new_spawn->SetMerchantLevelRange(GetMerchantMinLevel(), GetMerchantMaxLevel());
	if(GetSizeOffset() > 0){
		int8 offset = GetSizeOffset()+1;
		sint32 tmp_size = size + (rand()%offset - rand()%offset);
		if(tmp_size < 0)
			tmp_size = 1;
		else if(tmp_size >= 0xFFFF)
			tmp_size = 0xFFFF;
		new_spawn->size = (int16)tmp_size;
	}
	else
		new_spawn->size = size;
	new_spawn->SetPrimaryCommands(&primary_command_list);
	new_spawn->SetSecondaryCommands(&secondary_command_list);
	new_spawn->database_id = database_id;
	new_spawn->primary_command_list_id = primary_command_list_id;
	new_spawn->secondary_command_list_id = secondary_command_list_id;
	memcpy(&new_spawn->appearance, &appearance, sizeof(AppearanceData));
	new_spawn->faction_id = faction_id;
	new_spawn->target = 0;
	new_spawn->SetTotalHP(GetTotalHP());
	new_spawn->SetTotalPower(GetTotalPower());
	new_spawn->SetHP(GetHP());
	new_spawn->SetPower(GetPower());
	SetQuestsRequired(new_spawn);
	new_spawn->SetTransporterID(GetTransporterID());
	new_spawn->SetDeviceID(GetDeviceID());
	new_spawn->SetSoundsDisabled(IsSoundsDisabled());
	new_spawn->SetOmittedByDBFlag(IsOmittedByDBFlag());
	new_spawn->SetLootTier(GetLootTier());
	new_spawn->SetLootDropType(GetLootDropType());
	return new_spawn;
}
