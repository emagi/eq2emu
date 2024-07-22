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
#include "Sign.h"

#include "../common/ConfigReader.h"
#include "WorldDatabase.h"
#include "World.h"
#include "../common/Log.h"

extern World world;
extern ConfigReader configReader;
extern WorldDatabase database;
extern ZoneList zone_list;
extern MasterSpellList master_spell_list;

Sign::Sign(){
	widget_id = 0;
	widget_x = 0;
	widget_y = 0;
	widget_z = 0;
	appearance.pos.state = 1;
	appearance.difficulty = 0;
	spawn_type = 2;
	appearance.activity_status = 64;
	sign_type = 0;
	zone_x = 0;
	zone_y = 0;
	zone_z = 0;
	zone_heading = 0;
	sign_distance = 0;
	include_location = false;
	include_heading = false;
	zone_id = 0;
	language = 0;
}

Sign::~Sign(){

}

int32 Sign::GetWidgetID(){
	return widget_id;
}

EQ2Packet* Sign::serialize(Player* player, int16 version){
	return spawn_serialize(player, version);
}

void Sign::SetWidgetID(int32 val){
	widget_id = val;
}

void  Sign::SetWidgetX(float val){
	widget_x = val;
}

float Sign::GetWidgetX(){
	return widget_x;
}

void  Sign::SetWidgetY(float val){
	widget_y = val;
}

float Sign::GetWidgetY(){
	return widget_y;
}

void  Sign::SetWidgetZ(float val){
	widget_z = val;
}

float Sign::GetWidgetZ(){
	return widget_z;
}

void Sign::SetSignIcon(int8 val){
	appearance.icon = val;
}

void Sign::SetIncludeLocation(bool val){
	include_location = val;
}
bool Sign::GetIncludeLocation(){
	return include_location;
}
void Sign::SetIncludeHeading(bool val){
	include_heading = val;
}
bool Sign::GetIncludeHeading(){
	return include_heading;
}

Sign* Sign::Copy(){
	Sign* new_spawn = new Sign();
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
	new_spawn->SetCollector(IsCollector());
	new_spawn->SetMerchantID(merchant_id);
	new_spawn->SetMerchantType(merchant_type);
	new_spawn->SetMerchantLevelRange(GetMerchantMinLevel(), GetMerchantMaxLevel());
	new_spawn->SetPrimaryCommands(&primary_command_list);
	new_spawn->primary_command_list_id = primary_command_list_id;
	new_spawn->SetSecondaryCommands(&secondary_command_list);
	new_spawn->secondary_command_list_id = secondary_command_list_id;
	new_spawn->database_id = database_id;
	memcpy(&new_spawn->appearance, &appearance, sizeof(AppearanceData));
	new_spawn->SetWidgetID(widget_id);
	new_spawn->SetWidgetX(widget_x);
	new_spawn->SetWidgetY(widget_y);
	new_spawn->SetWidgetZ(widget_z);
	new_spawn->SetSignType(sign_type);
	new_spawn->SetSignZoneX(zone_x);
	new_spawn->SetSignZoneY(zone_y);
	new_spawn->SetSignZoneZ(zone_z);
	new_spawn->SetSignZoneHeading(zone_heading);
	new_spawn->SetSignZoneID(GetSignZoneID());
	new_spawn->SetSignTitle(GetSignTitle());
	new_spawn->SetSignDescription(GetSignDescription());
	new_spawn->SetSignDistance(sign_distance);
	new_spawn->SetIncludeHeading(include_heading);
	new_spawn->SetIncludeLocation(include_location);
	new_spawn->SetTransporterID(GetTransporterID());
	new_spawn->SetSoundsDisabled(IsSoundsDisabled());
	new_spawn->SetOmittedByDBFlag(IsOmittedByDBFlag());
	new_spawn->SetLootTier(GetLootTier());
	new_spawn->SetLootDropType(GetLootDropType());
	new_spawn->SetLanguage(GetLanguage());
	return new_spawn;
}

int32 Sign::GetSignZoneID(){
	return zone_id;
}

void Sign::SetSignZoneID(int32 val){
	zone_id = val;
}

const char* Sign::GetSignTitle(){
	if(title.length() > 0)
		return title.c_str();
	else
		return 0;
}

void Sign::SetSignTitle(const char* val){
	if(val)
		title = string(val);
}

const char* Sign::GetSignDescription(){
	if(description.length() > 0)
		return description.c_str();
	else
		return 0;
}

void Sign::SetSignDescription(const char* val){
	if(val)
		description = string(val);
}

int8 Sign::GetSignType(){
	return sign_type;
}

void Sign::SetSignType(int8 val){
	sign_type = val;
}

float Sign::GetSignZoneX(){
	return zone_x;
}
void Sign::SetSignZoneX(float val){
	zone_x = val;
}
float Sign::GetSignZoneY(){
	return zone_y;
}
void Sign::SetSignZoneY(float val){
	zone_y = val;
}
float Sign::GetSignZoneZ(){
	return zone_z;
}
void Sign::SetSignZoneZ(float val){
	zone_z = val;
}
float Sign::GetSignZoneHeading(){
	return zone_heading;
}
void Sign::SetSignZoneHeading(float val){
	zone_heading = val;
}
float Sign::GetSignDistance(){
	return sign_distance;
}
void Sign::SetSignDistance(float val){
	sign_distance = val;
}
void Sign::HandleUse(Client* client, string command)
{
	vector<TransportDestination*> destinations;

	//The following check disables the use of doors and other widgets if the player does not meet the quest requirements
	//If this is from a script ignore this check (client will be null)
	if (client) {
		bool meets_quest_reqs = MeetsSpawnAccessRequirements(client->GetPlayer());
		if (!meets_quest_reqs && (GetQuestsRequiredOverride() & 2) == 0)
			return;
		else if (meets_quest_reqs && appearance.show_command_icon != 1)
			return;
	}

	if( GetTransporterID() > 0 )
		GetZone()->GetTransporters(&destinations, client, GetTransporterID());

	if( destinations.size() )
	{
		client->SetTemporaryTransportID(0);
		client->ProcessTeleport(this, &destinations, GetTransporterID());
	}
	else if( sign_type == SIGN_TYPE_ZONE && GetSignZoneID() > 0 )
	{
		if( GetSignDistance() == 0 || client->GetPlayer()->GetDistance(this) <= GetSignDistance() )
		{
			string name = database.GetZoneName(GetSignZoneID());
			if( name.length() >0 )
			{
				if( !client->CheckZoneAccess(name.c_str()) )
					return;

				// determine if the coordinates should be set (returns false if they should)
				// clearer, if the sign has x,y,z,heading coordinates, use them otherwise I assume we use the zones safe coords(?)
				bool zone_coords_invalid = ( zone_x == 0 && zone_y == 0 && zone_z == 0 && zone_heading == 0 );

				// I really hate double-negatives. Seriously?
				if ( !zone_coords_invalid )
				{
					LogWrite(SIGN__DEBUG, 0, "Sign", "Sign has valid zone-to coordinates (%.2f, %.2f, %.2f, %.2f)", zone_x, zone_y, zone_z, zone_heading);
					client->GetPlayer()->SetX(zone_x);
					client->GetPlayer()->SetY(zone_y);
					client->GetPlayer()->SetZ(zone_z);
					client->GetPlayer()->SetHeading(zone_heading);					
				}
				else // alert client we couldnt set the coordinates
				{
					LogWrite(SIGN__WARNING, 0, "Sign", "Sign has no zone-to coordinates set, using zones safe coords.");
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Invalid zone in coords, taking you to a safe point.");
				}
				
				// Test if where we're going is an Instanced zone
				if ( !client->TryZoneInstance(GetSignZoneID(), zone_coords_invalid) )
				{
					LogWrite(SIGN__DEBUG, 0, "Sign", "Sending client to instance of zone_id: %u", GetSignZoneID());
					client->Zone(name.c_str(), zone_coords_invalid);
				}
			}
			else
			{
				LogWrite(SIGN__WARNING, 0, "Sign", "Unable to find zone with ID: %u", GetSignZoneID());
				client->Message(CHANNEL_COLOR_YELLOW, "Unable to find zone with ID: %u", GetSignZoneID());
			}
		}
		else
		{
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are too far away!");
		}
	}
	
	else if (client && command.length() > 0)
	{
		EntityCommand* entity_command = FindEntityCommand(command);


		//devn00b: Add support for marking objects
		if (entity_command && strcmp(entity_command->command.c_str(), "mark") == 0) {
			LogWrite(SIGN__DEBUG, 0, "Sign", "ActivateMarkReqested Sign - Command: '%s' (Should read mark)", entity_command->command.c_str());
			int32 char_id = client->GetCharacterID();
			database.SaveSignMark(char_id, GetWidgetID(), database.GetCharacterName(char_id), client);
			return;
		}

		if (entity_command)
		{
			LogWrite(SIGN__DEBUG, 0, "Sign", "ActivateQuestRequired Sign - Command: '%s'", entity_command->command.c_str());
			client->GetCurrentZone()->ProcessEntityCommand(entity_command, client->GetPlayer(), client->GetPlayer()->GetTarget());
		}

	}

}
