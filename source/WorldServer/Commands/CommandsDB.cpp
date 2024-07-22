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
#ifdef WIN32
	#include <WinSock2.h>
	#include <windows.h>
#endif
#include <mysql.h>
#include <assert.h>
#include "../../common/Log.h"
#include "../WorldDatabase.h"
#include "Commands.h"
#include "ConsoleCommands.h"



map<int32, string>* WorldDatabase::GetSpawnTemplateListByName(const char* name)
{
	LogWrite(COMMAND__DEBUG, 0, "Command", "Player listing spawn templates by template name ('%s')...", name);

	map<int32, string>* ret = 0;
	string template_name = "";

	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, name FROM spawn_templates WHERE name RLIKE '%s' LIMIT 0,10", getSafeEscapeString(name).c_str());
	if(result && mysql_num_rows(result) > 0)
	{
		ret = new map<int32, string>;
		MYSQL_ROW row;
		while(result && (row = mysql_fetch_row(result)))
		{
			template_name = string(row[1]);
			(*ret)[atoul(row[0])] = template_name;
			LogWrite(COMMAND__DEBUG, 5, "Command", "\t%u. '%s'", atoul(row[0]), template_name.c_str());
		}
	}
	return ret;
}

map<int32, string>* WorldDatabase::GetSpawnTemplateListByID(int32 location_id)
{
	LogWrite(COMMAND__DEBUG, 0, "Command", "Player listing spawn templates by LocaionID: %u...", location_id);

	map<int32, string>* ret = 0;
	string template_name = "";

	Query query;
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, name FROM spawn_templates WHERE spawn_location_id = %u", location_id);
	if(result && mysql_num_rows(result) > 0)
	{
		ret = new map<int32, string>;
		MYSQL_ROW row;
		while(result && (row = mysql_fetch_row(result)))
		{
			template_name = string(row[1]);
			(*ret)[atoul(row[0])] = template_name;
			LogWrite(COMMAND__DEBUG, 5, "Command", "\t%u. '%s'", atoul(row[0]), template_name.c_str());
		}
	}
	return ret;
}


int32 WorldDatabase::SaveSpawnTemplate(int32 placement_id, const char* template_name)
{
	Query query;

	string str_name = getSafeEscapeString(template_name).c_str();
	LogWrite(COMMAND__DEBUG, 0, "Command", "Player saving spawn template '%s' for placement_id %u...", str_name.c_str(), placement_id);

	query.RunQuery2(Q_INSERT, "INSERT INTO spawn_templates (name, spawn_location_id) VALUES ('%s', %u)", str_name.c_str(), placement_id);
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(COMMAND__ERROR, 0, "Command", "Error in SaveSpawnTemplate query '%s': %s", query.GetQuery(), query.GetError());
		return 0;
	}
	int32 ret = query.GetLastInsertedID();
	LogWrite(COMMAND__DEBUG, 0, "Command", "Success! Returning TemplateID: %u...", ret);
	return ret;
}

bool WorldDatabase::RemoveSpawnTemplate(int32 template_id)
{
	Query query;

	LogWrite(COMMAND__DEBUG, 0, "Command", "Player removing spawn template ID %u...", template_id);

	query.RunQuery2(Q_DELETE, "DELETE FROM spawn_templates WHERE id = %u", template_id);
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(COMMAND__ERROR, 0, "Command", "Error in RemoveSpawnTemplate query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}

	if (query.GetAffectedRows() > 0 )
	{
		LogWrite(COMMAND__DEBUG, 0, "Command", "Success! Removed spawn template ID %u...", template_id);
		return true;
	}

	return false;
}


int32 WorldDatabase::CreateSpawnFromTemplateByID(Client* client, int32 template_id)
{
	Query query, query2, query3, query4, query5, query6;
	MYSQL_ROW row;
	int32 spawn_location_id = 0;
	float new_x = client->GetPlayer()->GetX();
	float new_y = client->GetPlayer()->GetY();
	float new_z = client->GetPlayer()->GetZ();
	float new_heading = client->GetPlayer()->GetHeading();

	LogWrite(COMMAND__DEBUG, 0, "Command", "Creating spawn point from templateID %u...", template_id);
	LogWrite(COMMAND__DEBUG, 5, "Command", "\tCoords: %.2f %.2f %.2f...", new_x, new_y, new_z);

	// find the spawn_location_id in the template we plan to duplicate
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT spawn_location_id FROM spawn_templates WHERE id = %u", template_id);
	if (result && (row = mysql_fetch_row(result))) {
		if (row[0])
			spawn_location_id = atoi(row[0]);
	}

	if( spawn_location_id > 0 )
	{
		LogWrite(COMMAND__DEBUG, 5, "Command", "\tUsing LocationID: %u...", spawn_location_id);

		// insert a new spawn_location_name record
		string name = "TemplateGenerated";
		query2.RunQuery2(Q_INSERT, "INSERT INTO spawn_location_name (name) VALUES ('%s')", name.c_str());
		if(query2.GetErrorNumber() && query2.GetError() && query2.GetErrorNumber() < 0xFFFFFFFF){
			LogWrite(COMMAND__ERROR, 0, "Command", "Error in CreateSpawnFromTemplateByID query '%s': %s", query2.GetQuery(), query2.GetError());
			return 0;
		}
		int32 new_location_id = query2.GetLastInsertedID();
		LogWrite(COMMAND__DEBUG, 5, "Command", "Created new Spawn Location: '%s' (%u)", name.c_str(), new_location_id);

		// get all spawn_location_entries that match the templates spawn_location_id value and insert as new
		LogWrite(COMMAND__DEBUG, 5, "Command", "Finding existing spawn_location_entry(s) for location_id %u", spawn_location_id);
		MYSQL_RES* result2 = query3.RunQuery2(Q_SELECT, "SELECT spawn_id, spawnpercentage FROM spawn_location_entry WHERE spawn_location_id = %u", spawn_location_id);
		if(result2 && mysql_num_rows(result2) > 0){
			MYSQL_ROW row2;
			while(result2 && (row2 = mysql_fetch_row(result2)) && row2[0])
			{
				query4.RunQuery2(Q_INSERT, "INSERT INTO spawn_location_entry (spawn_id, spawn_location_id, spawnpercentage) VALUES (%u, %u, %i)", 
					atoul(row2[0]), new_location_id, atoi(row2[1]));
				if(query4.GetErrorNumber() && query4.GetError() && query4.GetErrorNumber() < 0xFFFFFFFF){
					LogWrite(COMMAND__ERROR, 0, "Command", "Error in CreateSpawnFromTemplateByID query '%s': %s", query4.GetQuery(), query4.GetError());
					return 0;
				}
				LogWrite(COMMAND__DEBUG, 5, "Command", "Insert Entry for spawn_id %u, location_id %u, percentage %i", atoul(row2[0]), new_location_id, atoi(row2[1]));
			}
		}

		// get all spawn_location_placements that match the templates spawn_location_id value and insert as new
		// Note: /spawn templates within current zone_id only, because of spawn_id issues (cannot template an Antonic spawn in Commonlands)
		LogWrite(COMMAND__DEBUG, 5, "Command", "Finding existing spawn_location_placement(s) for location_id %u", spawn_location_id);
		MYSQL_RES* result3 = query5.RunQuery2(Q_SELECT, "SELECT zone_id, x_offset, y_offset, z_offset, respawn, expire_timer, expire_offset, grid_id FROM spawn_location_placement WHERE spawn_location_id = %u", spawn_location_id);
		if(result3 && mysql_num_rows(result3) > 0){
			MYSQL_ROW row3;
			while(result3 && (row3 = mysql_fetch_row(result3)) && row3[0])
			{
				query6.RunQuery2(Q_INSERT, "INSERT INTO spawn_location_placement (zone_id, spawn_location_id, x, y, z, heading, x_offset, y_offset, z_offset, respawn, expire_timer, expire_offset, grid_id) VALUES (%i, %u, %2f, %2f, %2f, %2f, %2f, %2f, %2f, %i, %i, %i, %u)", 
					atoi(row3[0]), new_location_id, new_x, new_y, new_z, new_heading, atof(row3[1]), atof(row3[2]), atof(row3[3]), atoi(row3[4]), atoi(row3[5]), atoi(row3[6]), atoul(row3[7]));
				if(query6.GetErrorNumber() && query6.GetError() && query6.GetErrorNumber() < 0xFFFFFFFF){
					LogWrite(COMMAND__ERROR, 0, "Command", "Error in CreateSpawnFromTemplateByID query '%s': %s", query6.GetQuery(), query6.GetError());
					return 0;
				}
				LogWrite(COMMAND__DEBUG, 5, "Command", "Insert Placement at new coords for location_id %u", new_location_id);
			}
		}
		LogWrite(COMMAND__DEBUG, 0, "Command", "Success! New spawn(s) from TemplateID %u created from location %u", template_id, spawn_location_id);
		return new_location_id;
	}
	
	return 0;
}


int32 WorldDatabase::CreateSpawnFromTemplateByName(Client* client, const char* template_name)
{
	Query query, query1, query2, query3, query4, query5, query6;
	MYSQL_ROW row;
	int32 template_id = 0;
	int32 spawn_location_id = 0;
	float new_x = client->GetPlayer()->GetX();
	float new_y = client->GetPlayer()->GetY();
	float new_z = client->GetPlayer()->GetZ();
	float new_heading = client->GetPlayer()->GetHeading();

	LogWrite(COMMAND__DEBUG, 0, "Command", "Creating spawn point from template '%s'...", template_name);
	LogWrite(COMMAND__DEBUG, 5, "Command", "\tCoords: %.2f %.2f %.2f...", new_x, new_y, new_z);

	// find the spawn_location_id in the template we plan to duplicate
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT id, spawn_location_id FROM spawn_templates WHERE name = '%s'", template_name);
	if (result && (row = mysql_fetch_row(result))) {
		if (row[0])
		{
			template_id = atoul(row[0]);
			spawn_location_id = atoi(row[1]);
		}
	}

	if( spawn_location_id > 0 )
	{
		LogWrite(COMMAND__DEBUG, 5, "Command", "\tUsing LocationID: %u...", spawn_location_id);

		// insert a new spawn_location_name record
		string name = "TemplateGenerated";
		query2.RunQuery2(Q_INSERT, "INSERT INTO spawn_location_name (name) VALUES ('%s')", name.c_str());
		if(query2.GetErrorNumber() && query2.GetError() && query2.GetErrorNumber() < 0xFFFFFFFF){
			LogWrite(COMMAND__ERROR, 0, "Command", "Error in CreateSpawnFromTemplateByID query '%s': %s", query2.GetQuery(), query2.GetError());
			return 0;
		}
		int32 new_location_id = query2.GetLastInsertedID();
		LogWrite(COMMAND__DEBUG, 5, "Command", "Created new Spawn Location: '%s' (%u)", name.c_str(), new_location_id);

		// get all spawn_location_entries that match the templates spawn_location_id value and insert as new
		LogWrite(COMMAND__DEBUG, 5, "Command", "Finding existing spawn_location_entry(s) for location_id %u", spawn_location_id);
		MYSQL_RES* result2 = query3.RunQuery2(Q_SELECT, "SELECT spawn_id, spawnpercentage FROM spawn_location_entry WHERE spawn_location_id = %u", spawn_location_id);
		if(result2 && mysql_num_rows(result2) > 0){
			MYSQL_ROW row2;
			while(result2 && (row2 = mysql_fetch_row(result2)) && row2[0])
			{
				query4.RunQuery2(Q_INSERT, "INSERT INTO spawn_location_entry (spawn_id, spawn_location_id, spawnpercentage) VALUES (%u, %u, %i)", 
					atoul(row2[0]), new_location_id, atoi(row2[1]));
				if(query4.GetErrorNumber() && query4.GetError() && query4.GetErrorNumber() < 0xFFFFFFFF){
					LogWrite(COMMAND__ERROR, 0, "Command", "Error in CreateSpawnFromTemplateByID query '%s': %s", query4.GetQuery(), query4.GetError());
					return 0;
				}
				LogWrite(COMMAND__DEBUG, 5, "Command", "Insert Entry for spawn_id %u, location_id %u, percentage %i", atoul(row2[0]), new_location_id, atoi(row2[1]));
			}
		}

		// get all spawn_location_placements that match the templates spawn_location_id value and insert as new
		// Note: /spawn templates within current zone_id only, because of spawn_id issues (cannot template an Antonic spawn in Commonlands)
		LogWrite(COMMAND__DEBUG, 5, "Command", "Finding existing spawn_location_placement(s) for location_id %u", spawn_location_id);
		MYSQL_RES* result3 = query5.RunQuery2(Q_SELECT, "SELECT zone_id, x_offset, y_offset, z_offset, respawn, expire_timer, expire_offset, grid_id FROM spawn_location_placement WHERE spawn_location_id = %u", spawn_location_id);
		if(result3 && mysql_num_rows(result3) > 0){
			MYSQL_ROW row3;
			while(result3 && (row3 = mysql_fetch_row(result3)) && row3[0])
			{
				query6.RunQuery2(Q_INSERT, "INSERT INTO spawn_location_placement (zone_id, spawn_location_id, x, y, z, heading, x_offset, y_offset, z_offset, respawn, expire_timer, expire_offset, grid_id) VALUES (%i, %u, %2f, %2f, %2f, %2f, %2f, %2f, %2f, %i, %i, %i, %u)", 
					atoi(row3[0]), new_location_id, new_x, new_y, new_z, new_heading, atof(row3[1]), atof(row3[2]), atof(row3[3]), atoi(row3[4]), atoi(row3[5]), atoi(row3[6]), atoul(row3[7]));
				if(query6.GetErrorNumber() && query6.GetError() && query6.GetErrorNumber() < 0xFFFFFFFF){
					LogWrite(COMMAND__ERROR, 0, "Command", "Error in CreateSpawnFromTemplateByID query '%s': %s", query6.GetQuery(), query6.GetError());
					return 0;
				}
				LogWrite(COMMAND__DEBUG, 5, "Command", "Insert Placement at new coords for location_id %u", new_location_id);
			}
		}
		LogWrite(COMMAND__DEBUG, 0, "Command", "Success! New spawn(s) from TemplateID %u created from location %u", template_id, spawn_location_id);
		return new_location_id;
	}
	
	return 0;
}

bool WorldDatabase::SaveZoneSafeCoords(int32 zone_id, float x, float y, float z, float heading)
{
	Query query;

	LogWrite(COMMAND__DEBUG, 0, "Command", "Setting safe coords for zone %u (X: %.2f, Y: %.2f, Z: %.2f, H: %.2f)", zone_id, x, y, z, heading);

	query.RunQuery2(Q_UPDATE, "UPDATE zones SET safe_x = %f, safe_y = %f, safe_z = %f, safe_heading = %f WHERE id = %u", x, y, z, heading, zone_id);
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(DATABASE__ERROR, 0, "DBCore", "Error in SaveZoneSafeCoords query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}

	if (query.GetAffectedRows() > 0 )
	{
		LogWrite(COMMAND__DEBUG, 0, "Command", "Success! Set new safe coordinates in zone ID %u...", zone_id);
		return true;
	}

	LogWrite(COMMAND__ERROR, 0, "Command", "FAILED! Set new safe coordinates in zone ID %u...", zone_id);
	return false;
}

bool WorldDatabase::SaveSignZoneToCoords(int32 spawn_id, float x, float y, float z, float heading)
{
	Query query;

	LogWrite(COMMAND__DEBUG, 0, "Command", "Setting Zone-To coords for Spawn ID %u (X: %.2f, Y: %.2f, Z: %.2f, H: %.2f)", spawn_id, x, y, z, heading);

	query.RunQuery2(Q_UPDATE, "UPDATE spawn_signs SET zone_x = %f, zone_y = %f, zone_z = %f, zone_heading = %f WHERE spawn_id = %u", x, y, z, heading, spawn_id);
	if(query.GetErrorNumber() && query.GetError() && query.GetErrorNumber() < 0xFFFFFFFF){
		LogWrite(DATABASE__ERROR, 0, "DBCore", "Error in SaveSignZoneToCoords query '%s': %s", query.GetQuery(), query.GetError());
		return false;
	}

	if (query.GetAffectedRows() > 0 )
	{
		LogWrite(COMMAND__DEBUG, 0, "Command", "Success! Set new Zone-To coordinates in zone ID %u...", spawn_id);
		return true;
	}

	LogWrite(COMMAND__ERROR, 0, "Command", "FAILED! Set new Zone-To coordinates in zone ID %u...", spawn_id);
	return false;
}
