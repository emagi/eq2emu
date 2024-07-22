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
#include "ClientPacketFunctions.h"
#include "WorldDatabase.h"
#include "../common/ConfigReader.h"
#include "Variables.h"
#include "World.h"
#include "classes.h"
#include "../common/Log.h"
#include "Traits/Traits.h"

extern Classes classes;
extern Commands commands;
extern WorldDatabase database;
extern ConfigReader configReader;
extern MasterSpellList master_spell_list;
extern MasterTraitList master_trait_list;
extern Variables variables;
extern World world;

void ClientPacketFunctions::SendFinishedEntitiesList ( Client* client ){

	EQ2Packet* finishedEntitiesApp = new EQ2Packet(OP_DoneSendingInitialEntitiesMsg, 0, 0);
	client->QueuePacket(finishedEntitiesApp);

}

void ClientPacketFunctions::SendSkillSlotMappings(Client* client){
	EQ2Packet* app = client->GetPlayer()->GetSpellSlotMappingPacket(client->GetVersion());
	if(app)
		client->QueuePacket(app);
}

void ClientPacketFunctions::SendLoginDenied ( Client* client ){
	PacketStruct* packet = configReader.getStruct("LS_LoginResponse", 1);
	if(packet){
		packet->setDataByName("reply_code", 1);
		packet->setDataByName("unknown03", 0xFFFFFFFF);
		packet->setDataByName("unknown04", 0xFFFFFFFF);
		EQ2Packet* app = packet->serialize();
		client->QueuePacket(app);
		safe_delete(packet);
	}
}

void ClientPacketFunctions::SendLoginAccepted ( Client* client ){
	LogWrite(PACKET__DEBUG, 0, "Packet", "Sending Login Accepted packet (LS_LoginResponse, %i)", client->GetVersion());
	PacketStruct* response_packet = configReader.getStruct("LS_LoginResponse", client->GetVersion());
	if(response_packet){
		response_packet->setDataByName("unknown02", 1);
		response_packet->setDataByName("unknown05", -959971393);
		response_packet->setDataByName("unknown08", 2);
		response_packet->setDataByName("unknown09", 585);
		response_packet->setDataByName("unknown10", 1597830);
		response_packet->setDataByName("accountid", 3); //client->GetAccountID());
		EQ2Packet* outapp = response_packet->serialize();
		client->QueuePacket(outapp);
		safe_delete(response_packet);
	}
}

void ClientPacketFunctions::SendCommandList ( Client* client ){
	EQ2Packet* app = commands.GetRemoteCommands()->serialize(client->GetVersion());
	client->QueuePacket(app);
}

void ClientPacketFunctions::SendGameWorldTime ( Client* client ){
	PacketStruct* packet = world.GetWorldTime(client->GetVersion());
	if(packet){
		client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
	//opcode 501 was the selection display opcode
}

void ClientPacketFunctions::SendCharacterData ( Client* client ){
	client->GetPlayer()->SetCharacterID(client->GetCharacterID());
	if(!client->IsReloadingZone()) {
		EQ2Packet* outapp = client->GetPlayer()->serialize(client->GetPlayer(), client->GetVersion());
		//DumpPacket(outapp);
		client->QueuePacket(outapp);
	}
}

void ClientPacketFunctions::SendCharacterSheet ( Client* client ){
	EQ2Packet* app = client->GetPlayer()->GetPlayerInfo()->serialize(client->GetVersion());
	client->QueuePacket(app);

	if (client->GetVersion() >= 1188) {
		EQ2Packet* app2 = client->GetPlayer()->GetPlayerInfo()->serializePet(client->GetVersion());
		if (app2)
			client->QueuePacket(app2);
	}
}

void ClientPacketFunctions::SendSkillBook ( Client* client ){
	EQ2Packet* app = client->GetPlayer()->skill_list.GetSkillPacket(client->GetVersion());
	if(app)
		client->QueuePacket(app);
}

// Jabantiz: Attempt to get the char trait list working
void ClientPacketFunctions::SendTraitList(Client* client) {
	if (client->GetVersion() >= 562) {
		EQ2Packet* traitApp = master_trait_list.GetTraitListPacket(client);
		//DumpPacket(traitApp);
		if (traitApp) {
			client->QueuePacket(traitApp);
		}
	}
}

void ClientPacketFunctions::SendAbilities ( Client* client ){
	LogWrite(MISC__TODO, 1, "TODO", " Add SendAbilities functionality\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);
	// this is the featherfall ability data
	// later this would loop through and send all abilities
	/*uchar abilityData[] ={0x11,0x00,0x00,0x00,0xff,0x15,0x02,0x00,0x0b,0x00,0x46,0x65,0x61,0x74
		,0x68,0x65,0x72,0x66,0x61,0x6c,0x6c};
	EQ2Packet* abilityApp = new EQ2Packet(OP_ClientCmdMsg, abilityData, sizeof(abilityData));
	client->QueuePacket(abilityApp);*/
}

void ClientPacketFunctions::SendCommandNamePacket ( Client* client ){
	LogWrite(MISC__TODO, 1, "TODO", " fix, this is actually quest/collection information\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);
	/*
	PacketStruct* command_packet = configReader.getStruct("WS_CommandName", client->GetVersion());
	if(command_packet){
		command_packet->setDataByName("unknown03", 0x221bfb47);

		char* charName = { "BogusName" };
		command_packet->setMediumStringByName("character_name",charName);
		EQ2Packet* outapp = command_packet->serialize();
		client->QueuePacket(outapp);
		safe_delete(command_packet);
	}
	*/
}

void ClientPacketFunctions::SendQuickBarInit ( Client* client ){
	int32 count = database.LoadPlayerSkillbar(client);
	if(count == 0) {
		LogWrite(PACKET__DEBUG, 0, "Packet", "No character quickbar found!");
		database.UpdateStartingSkillbar(client->GetCharacterID(), client->GetPlayer()->GetAdventureClass(), client->GetPlayer()->GetRace());
		database.LoadPlayerSkillbar(client);
	}
	EQ2Packet* quickbarApp = client->GetPlayer()->GetQuickbarPacket(client->GetVersion());
	if(quickbarApp)
		client->QueuePacket(quickbarApp);
}

void ClientPacketFunctions::SendCharacterMacros(Client* client) {
	LogWrite(PACKET__DEBUG, 0, "Packet", "Sending Character Macro packet (WS_MacroInit, %i)", client->GetVersion());
	map<int8, vector<MacroData*> >* macros = database.LoadCharacterMacros(client->GetCharacterID());
	if (macros) {
		PacketStruct* macro_packet = configReader.getStruct("WS_MacroInit", client->GetVersion());
		if (macro_packet) {
			map<int8, vector<MacroData*> >::iterator itr;
			macro_packet->setArrayLengthByName("macro_count", macros->size());
			int8 x = 0;
			for (itr = macros->begin(); itr != macros->end(); itr++, x++) {
				macro_packet->setArrayDataByName("number", itr->first, x);
				if (itr->second.size() > 0) {
					LogWrite(PACKET__DEBUG, 5, "Packet", "Loading Macro %i, name: %s", itr->first, itr->second[0]->name.c_str());
					macro_packet->setArrayDataByName("name", itr->second[0]->name.c_str(), x);
				}
				if (client->GetVersion() > 373) {
					char tmp_details_count[25] = { 0 };
					sprintf(tmp_details_count, "macro_details_count_%i", x);
					macro_packet->setArrayLengthByName(tmp_details_count, itr->second.size());
					for (int8 i = 0; i < itr->second.size(); i++) {
						char tmp_command[15] = { 0 };
						sprintf(tmp_command, "command%i", x);
						LogWrite(PACKET__DEBUG, 5, "Packet", "\tLoading Command %i: %s", itr->first, x, itr->second[i]->text.c_str());
						macro_packet->setArrayDataByName(tmp_command, itr->second[i]->text.c_str(), i);
						if ( i > 0 ) // itr->second[0] used below, we will delete it later
							safe_delete(itr->second[i]); // delete MacroData*
					}
					macro_packet->setArrayDataByName("unknown2", 2, x);
					macro_packet->setArrayDataByName("unknown3", 0xFFFFFFFF, x);
				}
				else {
					if (itr->second.size() > 0)
						macro_packet->setArrayDataByName("command", itr->second[0]->text.c_str(), x);
				}
				macro_packet->setArrayDataByName("icon", itr->second[0]->icon, x);
				client->GetPlayer()->macro_icons[itr->first] = itr->second[0]->icon;
				
				// remove itr->second[0] now that we are done with it
				safe_delete(itr->second[0]); // delete MacroData*
			}
			EQ2Packet* packet = macro_packet->serialize();
			client->QueuePacket(packet);
			safe_delete(macro_packet);
		}
		safe_delete(macros);
	}
}

void ClientPacketFunctions::SendMOTD ( Client* client ){	

	const char* motd = 0;

	// fetch MOTD from `variables` table
	Variable* var = variables.FindVariable("motd");

	if( var == NULL || strlen (var->GetValue()) == 0) {
		LogWrite(WORLD__WARNING, 0, "World", "No MOTD set. Sending generic message...");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Message of the Day: Welcome to EQ2Emulator! Customize this message in the `variables`.`motd` data!");
	}
	else {
		motd = var->GetValue();
		LogWrite(WORLD__DEBUG, 0, "World", "Send MOTD...");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, motd);
	}
		
}

void ClientPacketFunctions::SendUpdateSpellBook ( Client* client ){
	if(client->IsReadyForSpawns()){
		EQ2Packet* app = client->GetPlayer()->GetSpellBookUpdatePacket(client->GetVersion());
		if(app)
			client->QueuePacket(app);
	}
	client->GetPlayer()->UnlockAllSpells(true);
}

void ClientPacketFunctions::SendServerControlFlagsClassic(Client* client, int32 param, int32 value) {
	PacketStruct* packet = configReader.getStruct("WS_ServerControlFlags", client->GetVersion());
	if(packet) {
		packet->setDataByName("parameter", param);
		packet->setDataByName("value", value);
		client->QueuePacket(packet->serialize());
	}
	safe_delete(packet);
}
void ClientPacketFunctions::SendServerControlFlags(Client* client, int8 param, int8 param_val, int8 value) {
	PacketStruct* packet = configReader.getStruct("WS_ServerControlFlags", client->GetVersion());
	if(packet) {
		if (param == 1)
			packet->setDataByName("parameter1", param_val);
		else if (param == 2)
			packet->setDataByName("parameter2", param_val);
		else if (param == 3)
			packet->setDataByName("parameter3", param_val);
		else if (param == 4)
			packet->setDataByName("parameter4", param_val);
		else if (param == 5)
			packet->setDataByName("parameter5", param_val);
		else {
			safe_delete(packet);
			return;
		}
		packet->setDataByName("value", value);
		client->QueuePacket(packet->serialize());
		/*
		Some other values for this packet
		first param:
		01 flymode
		02 collisons off
		04 unknown
		08 heading movement only
		16 forward/reverse movement only
		32 low gravity
		64 sit

		second
		2 crouch


		third:
		04 float when trying to jump, no movement
		08 jump high, no movement
		128 walk underwater

		fourth:
		01 moon jump underwater
		04 fear
		16 moon jumps
		32 safe fall (float to ground)
		64 cant move

		fifth:
		01 die
		08 hover (fae)
		32 flymode2?

		*/
	}
	safe_delete(packet);
}

void ClientPacketFunctions::SendInstanceList(Client* client) {
	if (client->GetPlayer()->GetCharacterInstances()->GetInstanceCount() > 0) {
		PacketStruct* packet = configReader.getStruct("WS_InstanceCreated", client->GetVersion());
		if (packet) {
			vector<InstanceData> persist = client->GetPlayer()->GetCharacterInstances()->GetPersistentInstances();
			vector<InstanceData> lockout = client->GetPlayer()->GetCharacterInstances()->GetLockoutInstances();

			packet->setArrayLengthByName("num_instances", lockout.size());
			for (int32 i = 0; i < lockout.size(); i++) {
				InstanceData data = lockout.at(i);

				packet->setArrayDataByName("unknown1", data.db_id, i);					// unique id per player
				packet->setArrayDataByName("instance_zone_name", data.zone_name.c_str(), i);
				packet->setArrayDataByName("unknown2", 0x0B, i);								// Always set to 0x0B on live packets
				packet->setArrayDataByName("success_last", data.last_success_timestamp, i);
				packet->setArrayDataByName("last_failure", data.last_failure_timestamp, i);
				packet->setArrayDataByName("failure", data.failure_lockout_time, i);
				packet->setArrayDataByName("success", data.success_lockout_time, i);
			}

			packet->setArrayLengthByName("num_persistent", persist.size());
			for (int32 i = 0; i < persist.size(); i++) {
				InstanceData data = persist.at(i);

				packet->setArrayDataByName("unknown1a", data.db_id, i);							// unique id per player
				packet->setArrayDataByName("persistent_zone_name", data.zone_name.c_str(), i);
				packet->setArrayDataByName("unknown2a", 0x0B, i);										// set to 0x0B in all live packets
				packet->setArrayDataByName("persist_success_timestamp", data.last_success_timestamp, i);
				packet->setArrayDataByName("persist_failure_timestamp", data.last_failure_timestamp, i);

				// Check min duration (last success + failure)
				//if (Timer::GetUnixTimeStamp() < data.last_success_timestamp + data.failure_lockout_time*/)
					//packet->setArrayDataByName("unknown3b", 1, i);
				packet->setArrayDataByName("unknown3b", 1, i);

				packet->setArrayDataByName("minimum_duration", data.failure_lockout_time, i);
				packet->setArrayDataByName("maximum_duration", data.success_lockout_time, i);

				packet->setArrayDataByName("unknown4a", 1800, i); // All live logs have 0x0708
			}

			client->QueuePacket(packet->serialize());
		}
		safe_delete(packet);
	}
}

void ClientPacketFunctions::SendMaintainedExamineUpdate(Client* client, int8 slot_pos, int32 update_value, int8 update_type){
	if (!client)
		return;

	PacketStruct* packet = configReader.getStruct("WS_UpdateMaintainedExamine", client->GetVersion());

	if (packet){
		packet->setSubstructDataByName("info_header", "show_name", 1);
		packet->setSubstructDataByName("info_header", "packettype", 19710);
		packet->setSubstructDataByName("info_header", "packetsubtype", 5);
		packet->setDataByName("time_stamp", Timer::GetCurrentTime2());
		packet->setDataByName("slot_pos", slot_pos);
		packet->setDataByName("update_value", update_value > 0 ? update_value : 0xFFFFFFFF);
		packet->setDataByName("update_type", update_type);
		client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
}

void ClientPacketFunctions::SendZoneChange(Client* client, char* zone_ip, int16 zone_port, int32 key) {
	if (!client)
		return;

	PacketStruct* packet = configReader.getStruct("WS_ZoneChangeMsg", client->GetVersion());
	if (packet) {
		packet->setDataByName("account_id", client->GetAccountID());
		packet->setDataByName("key", key);
		packet->setDataByName("ip_address", zone_ip);
		packet->setDataByName("port", zone_port);
		client->QueuePacket(packet->serialize());
	}
	safe_delete(packet);
}

void ClientPacketFunctions::SendStateCommand(Client* client, int32 spawn_id, int32 state) {
	if (!client || !spawn_id) {
		return;
	}

	PacketStruct* packet = configReader.getStruct("WS_StateCmd", client->GetVersion());
	if (packet) {
		packet->setDataByName("spawn_id", spawn_id);
		packet->setDataByName("state", state);
		client->QueuePacket(packet->serialize());
	}
	safe_delete(packet);
}

void ClientPacketFunctions::SendFlyMode(Client* client, int8 flymode, bool updateCharProperty)
{
	if (updateCharProperty)
		database.insertCharacterProperty(client, CHAR_PROPERTY_FLYMODE, (char*)std::to_string(flymode).c_str());
	if(client->GetVersion() <= 561) {
		if(flymode) {
			// old flymode
			SendServerControlFlagsClassic(client, flymode, 1);
			if(flymode == 1) {
				// disable noclip
				SendServerControlFlagsClassic(client, 2, 0);
			}
		}
		else {
			// disable flymode and noclip
			SendServerControlFlagsClassic(client, 2, 0);
			SendServerControlFlagsClassic(client, 1, 0);
		}
	}
	else {	
		if(flymode == 2) {
			// new flymode + noclip
			SendServerControlFlags(client, 5, 32, 1);
			SendServerControlFlags(client, 1, 2, 1);
		}
		else if(flymode == 1) {
			// new flymode
			SendServerControlFlags(client, 5, 32, 1);
			SendServerControlFlags(client, 1, 2, 0);
		}
		else {
			// disable flymode and noclip
			SendServerControlFlags(client, 5, 32, 0);
			SendServerControlFlags(client, 1, 2, 0);
		}
	}
		
		client->Message(CHANNEL_STATUS, "Flymode %s, No Clip %s", flymode > 0 ? "on" : "off", flymode > 1 ? "on" : "off");
		/*
		CLASSIC/DOF ONLY HAS THE FIRST SET OF FLAGS
		Some other values for this packet
		first param:
		01 flymode
		02 collisons off
		04 unknown
		08 forward movement
		16 heading movement
		32 low gravity
		64 sit

		EVERYTHING BELOW NOT SUPPORTED BY CLASSIC/DOF
		second
		2 crouch


		third:
		04 float when trying to jump, no movement
		08 jump high, no movement

		fourth:
		04 autorun (fear?)
		16 moon jumps
		32 safe fall (float to ground)
		64 cant move

		fifth:
		01 die
		08 hover (fae)
		32 flymode2?

		*/
}