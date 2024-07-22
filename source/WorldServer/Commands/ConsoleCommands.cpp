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

#include <iostream>
using namespace std;
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../common/debug.h"
#include "../../common/Log.h"
#include "../../common/seperator.h"
#include "ConsoleCommands.h"
#include "../World.h"
#include "../Rules/Rules.h"
#include "../WorldDatabase.h"

extern volatile bool RunLoops;
bool ContinueLoops = false;
extern Variables variables;
extern ZoneList zone_list;
extern RuleManager rule_manager;
extern WorldDatabase database;

void ProcessConsoleInput(const char * cmdInput)
{
	static ConsoleCommand Commands[] = {

		// account controls
		{ &ConsoleBanCommand,			"ban",		"[player] {duration} {reason}",	"Ban player with {optional} duration and reason." },
		{ &ConsoleUnbanCommand,			"unban",	"[player]",						"Unban a player." },
		{ &ConsoleKickCommand,			"kick",		"[player] {reason}",			"Kick player with {optional} reason." },

		// chat controls
		{ &ConsoleAnnounceCommand,		"announce",	"[message]",			"Sends Announcement message to all channels/clients." },
		{ &ConsoleBroadcastCommand,		"broadcast","[message]",			"Sends Broadcast message to all channels/clients." },
		{ &ConsoleChannelCommand,		"channel",	"[channel] [message]",	"Sends Channel message to channel." },
		{ &ConsoleTellCommand,			"tell",		"[player] [message]",	"Sends Private message to player." },

		// world system controls
		{ &ConsoleGuildCommand,			"guild",	"[params]",	"" },
		{ &ConsolePlayerCommand,		"player",	"[params]", "" },
		{ &ConsoleSetAdminPlayer,		"makeadmin",	"[charname] [status=0]", "" },
		{ &ConsoleZoneCommand,			"zone",		"[command][value]",	"command = help to get help" },
		{ &ConsoleWorldCommand,			"world",	"[params]", "" },
		{ &ConsoleGetMOTDCommand,		"getmotd",	"",	"Display current MOTD" },
		{ &ConsoleSetMOTDCommand,		"setmotd",	"[new motd]",	"Sets a new MOTD" },

		/// misc controls
		{ &ConsoleWhoCommand,			"who",		"{zone id | player}",	"Shows who is online globally, or in a given zone." },
		{ &ConsoleReloadCommand,		"reload",	"[all | [type]]",		"Reload main systems." },
		{ &ConsoleRulesCommand,			"rules",	"{zone} {id}",			"Show Global Ruleset (or Zone ruleset {optional})" },
		{ &ConsoleShutdownCommand,		"shutdown",	"[delay]",				"Gracefully shutdown world in [delay] sesconds." },
		{ &ConsoleCancelShutdownCommand,"cancel",	"",						"Cancel shutdown command." },
		{ &ConsoleExitCommand,			"exit",		"",						"Brutally kills the world without mercy." },
		{ &ConsoleExitCommand,			"quit",		"",						"Brutally kills the world without mercy." },
		{ &ConsoleTestCommand,			"test",		"",						"Dev testing command." },
		{ NULL, NULL, NULL, NULL },
	};

	Seperator *sep = new Seperator(cmdInput, ' ', 20, 100, true);
	bool found = false;
	uint32 i;

	if (!sep)
		return;

	if (!strcasecmp(sep->arg[0], "help") || sep->arg[0][0] == 'h' || sep->arg[0][0] == 'H' || sep->arg[0][0] == '?') {
		found = true;
		printf("======================================================================================================\n");
		printf("| %10s | %30s | %52s |\n", "Name", "Params", "Description");
		printf("======================================================================================================\n");
		for (i = 0; Commands[i].Name != NULL; i++) {
			printf("| %10s | %30s | %52s |\n", Commands[i].Name, Commands[i].ParameterFormat, Commands[i].Description);
		}
		printf("======================================================================================================\n");
		printf("-[ Help formatted for 120 chars wide screen ]-\n");
	}
	else {
		for (i = 0; Commands[i].Name != NULL; ++i) {
			if (!strcasecmp(Commands[i].Name, sep->arg[0])) {
				found = true;
				if (!Commands[i].CommandPointer(sep))
					printf("\nError, incorrect syntax for '%s'.\n Correct syntax is: '%s'.\n\n", Commands[i].Name, Commands[i].ParameterFormat );
			}
		}
	}

	if (!found)
		printf("Invalid Command '%s'! Type '?' or 'help' to get a command list.\n\n", sep->arg[0]);

	fflush(stdout);

	delete sep;
}

/************************************************* COMMANDS *************************************************/

bool ConsoleBanCommand(Seperator *sep)
{
	if( strlen(sep->arg[1]) == 0 ) 
		return false;

	return true;
}

bool ConsoleUnbanCommand(Seperator *sep)
{
	if( strlen(sep->arg[1]) == 0 ) 
		return false;

	return true;
}

bool ConsoleKickCommand(Seperator *sep)
{
	if( strlen(sep->arg[1]) == 0 ) 
		return false;

	return true;
}


bool ConsoleAnnounceCommand(Seperator *sep)
{
	if( strlen(sep->arg[1]) == 0 ) 
		return false;

	return true;
}

bool ConsoleBroadcastCommand(Seperator *sep)
{
	if( strlen(sep->arg[1]) == 0 ) 
		return false;

	char message[4096]; 
	snprintf(message, sizeof(message), "%s %s", "BROADCAST:", sep->argplus[1]);
	zone_list.HandleGlobalBroadcast(message);
	return true;
}

bool ConsoleChannelCommand(Seperator *sep)
{
	if( strlen(sep->arg[1]) == 0 ) 
		return false;

	return true;
}

bool ConsoleTellCommand(Seperator *sep)
{
	if( strlen(sep->arg[1]) == 0 ) 
		return false;

	return true;
}


bool ConsoleWhoCommand(Seperator *sep) 
{

	// zone_list.ProcessWhoQuery(who, client);

	if (!strcasecmp(sep->arg[1], "zone")) {
		printf("Who's Online in Zone");
		if (sep->IsNumber(2)) {
			printf("ID %s:\n", sep->arg[2]);
			printf("===============================================================================\n");
			printf("| %10s | %62s |\n", "CharID", "Name");
			printf("===============================================================================\n");
		}
		else {
			printf(" '%s':\n", sep->arg[2]);
			printf("===============================================================================\n");
			printf("| %10s | %62s |\n", "CharID", "Name");
			printf("===============================================================================\n");
		}
	}
	else {
		printf("Who's Online (Global):\n");
		printf("===============================================================================\n");
		printf("| %10s | %20s | %39s |\n", "CharID", "Name", "Zone");
		printf("===============================================================================\n");
	}
	printf("Not Implemented... yet :)\n");
	printf("===============================================================================\n");
	return true;
}

bool ConsoleGuildCommand(Seperator *sep)
{
	if( strlen(sep->arg[1]) == 0 ) 
		return false;

	return true;
}

bool ConsolePlayerCommand(Seperator *sep)
{
	if( strlen(sep->arg[1]) == 0 ) 
		return false;

	return true;
}

bool ConsoleSetAdminPlayer(Seperator *sep)
{
	if(!sep->arg[1] ||  strlen(sep->arg[1]) == 0)
		return false;

	sint16 status = 0;
	if(sep->IsNumber(2))
		status = atoi(sep->arg[2]);
	
	Client* client = zone_list.GetClientByCharName(sep->arg[1]);
	
	if(!client) {
		printf("Client not found by char name, must be logged in\n");
		return true;
	}

	if(!client->GetPlayer()) {

		printf("Player is not available for client class, try again\n");
		return true;
	}

	client->SetAdminStatus(status);
	if(status)
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Admin status updated.");
	database.UpdateAdminStatus(client->GetPlayer()->GetName(), status);
	printf("Admin status for %s is updated to %i\n", client->GetPlayer()->GetName(), status);
	
	return true;
}

bool ConsoleWorldCommand(Seperator *sep)
{
	if( strlen(sep->arg[1]) == 0 ) 
		return false;

	return true;
}

bool ConsoleZoneCommand(Seperator *sep)
{
	if( strlen(sep->arg[1]) == 0 ) // has to be at least 1 arg (command)
		return false;

	ZoneServer* zone = 0;

	if( strlen(sep->arg[2]) == 0 )
	{
		// process commands without values
		if (!strcasecmp(sep->arg[1], "active") )
		{
			// not correct, but somehow need to access the Private zlist from World.h ???
			list<ZoneServer*> zlist;

			list<ZoneServer*>::iterator zone_iter;
			ZoneServer* tmp = 0;
			int zonesListed = 0;

			printf("> List Active Zones...\n");
			printf("======================================================================================================\n");
			printf("| %7s | %30s | %10s | %42s |\n", "ID", "Name", "Instance", "Description");
			printf("======================================================================================================\n");

			for(zone_iter=zlist.begin(); zone_iter!=zlist.end(); zone_iter++){
				tmp = *zone_iter;
				zonesListed++;
				printf("| %7d | %30s | %10d | %42s |\n", tmp->GetZoneID(), tmp->GetZoneName(), tmp->GetInstanceID(),tmp->GetZoneDescription());
			}
			return true;
		}
		else if (!strcasecmp(sep->arg[1], "help") || sep->arg[1][0] == '?') 
		{
			printf("======================================================================================================\n");
			printf("| %10s | %30s | %52s |\n", "Command", "Value", "Description");
			printf("======================================================================================================\n");
			printf("| %10s | %30s | %52s |\n", "active", "n/a", "List currently active zones");
			printf("| %10s | %30s | %52s |\n", "list", "[name]", "Lookup zone by name");
			printf("| %10s | %30s | %52s |\n", "status", "[zone_id | name | ALL]", "List zone stats");
			printf("| %10s | %30s | %52s |\n", "lock", "[zone_id | name]", "Locks a zone");
			printf("| %10s | %30s | %52s |\n", "unlock", "[zone_id | name]", "Unlocks a zone");
			printf("| %10s | %30s | %52s |\n", "shutdown", "[zone_id | name | ALL]", "Gracefully shuts down a zone");
			printf("| %10s | %30s | %52s |\n", "kill", "[zone_id | name | ALL]", "Terminates a zone");
			printf("======================================================================================================\n");
			return true;
		}
		else
			return false;
	}
	else
	{
		if( !strcasecmp(sep->arg[1], "list") )
		{
			const char* name = 0;
			name = sep->argplus[2];
			map<int32, string>* zone_names = database.GetZoneList(name);

			if(!zone_names)
			{
				printf("> No zones found.\n");
			}
			else
			{
				printf("> List zones matching '%s'...\n", sep->arg[2]);
				printf("====================================================\n");
				printf("| %3s | %42s |\n", "ID", "Name");
				printf("====================================================\n");
				map<int32, string>::iterator itr;
					
				for(itr = zone_names->begin(); itr != zone_names->end(); itr++)
					printf("| %3u | %42s |\n", itr->first, itr->second.c_str());
				safe_delete(zone_names);
				printf("====================================================\n");
			}
			return true;
		}

		if( !strcasecmp(sep->arg[1], "lock") )
		{
			if( sep->IsNumber(2) )
				printf("> Locking zone ID %i...\n", atoul(sep->arg[2]));
			else
				printf("> Locking zone '%s'...\n", sep->arg[2]);
			return true;
		}

		if( !strcasecmp(sep->arg[1], "unlock") )
		{
			if( strlen(sep->arg[2]) > 0 && sep->IsNumber(2) )
				printf("> Unlocking zone ID %i...\n", atoi(sep->arg[2]));
			else
				printf("> Unlocking zone '%s'...\n", sep->arg[2]);
			return true;
		}

		if( !strcasecmp(sep->arg[1], "status") )
		{
			if( sep->IsNumber(2) )
			{
				zone = zone_list.Get(atoi(sep->arg[2]), false, false, false);
				if( zone )
				{
					printf("> Zone status for zone ID %i...\n", atoi(sep->arg[2]));
					printf("============================================================================================\n");
					printf("| %30s | %10s | %42s |\n", "Zone", "Param", "Value");
					printf("============================================================================================\n");
					printf("| %30s | %10s | %42s |\n", zone->GetZoneName(), "locked", zone->GetZoneLockState() ? "true" : "false");
				}
				else
				{
					printf("> Zone ID %i not running, so not locked.\n", atoi(sep->arg[2]));
				}
			}
			else if( !strcasecmp(sep->arg[2], "ALL") )
			{
				printf("> Zone status for ALL active zones...\n");
			}
			else
			{
				printf("> Zone status for zone '%s'...\n", sep->arg[2]);
			}
			return true;
		}

		if( !strcasecmp(sep->arg[1], "shutdown") )
		{
			if( sep->IsNumber(2) )
				printf("> Shutdown zone ID %i...\n", atoi(sep->arg[2]));
			else if( !strcasecmp(sep->arg[2], "ALL") )
				printf("> Shutdown ALL active zones...\n");
			else
				printf("> Shutdown zone '%s'...\n", sep->arg[2]);
			return true;
		}

		if( !strcasecmp(sep->arg[1], "kill") )
		{
			if( sep->IsNumber(2) )
				printf("> Kill zone ID %i...\n", atoi(sep->arg[2]));
			else if( !strcasecmp(sep->arg[2], "ALL") )
				printf("> Kill ALL active zones...\n");
			else
				printf("> Kill zone '%s'...\n", sep->arg[2]);
			return true;
		}
	}
	return false;
}

bool ConsoleGetMOTDCommand(Seperator *sep)
{
	const char* motd = 0;
	Variable* var = variables.FindVariable("motd");
	if( var == NULL || strlen (var->GetValue()) == 0){
		printf("No MOTD.");
	}
	else{
		motd = var->GetValue();
		printf("%s\n", motd);
	}
	return true;
}

bool ConsoleSetMOTDCommand(Seperator *sep)
{
	if( strlen(sep->arg[1]) == 0 ) 
		return false;

	return true;
}

bool ConsoleReloadCommand(Seperator *sep)
{
	#ifdef _WIN32
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(console, FOREGROUND_YELLOW_BOLD);
	#else
		printf("\033[1;33m");
	#endif
	printf("Usage: ");

	#ifdef _WIN32
		SetConsoleTextAttribute(console, FOREGROUND_WHITE_BOLD);
	#else
		printf("\033[1;37m");
	#endif
	printf("reload [type]\n");

	#ifdef _WIN32
		SetConsoleTextAttribute(console, 8);
	#else
		printf("\033[0m");
	#endif
	printf("Valid [type] paramters are:\n");
	printf("===============================================================================\n");
	printf("| %21s | %51s |\n", "all", "Reloads all systems (why not just restart?)");
	printf("| %21s | %51s |\n", "structs", "Reloads structs (XMLs)");
	printf("| %21s | %51s |\n", "items", "Reload Items data");
	printf("| %21s | %51s |\n", "luasystem", "Reload LUA System Scripts");
	printf("| %21s | %51s |\n", "spawnscripts", "Reload SpawnScripts");
	printf("| %21s | %51s |\n", "quests", "Reload Quest Data and Scripts");
	printf("| %21s | %51s |\n", "spawns", "Reload ALL Spawns from DB");
	printf("| %21s | %51s |\n", "groundspawn_items", "Reload Groundspawn Items lists");
	printf("| %21s | %51s |\n", "zonescripts", "Reload Zone Scripts");
	printf("| %21s | %51s |\n", "entity_commands", "Reload Entity Commands");
	printf("| %21s | %51s |\n", "factions", "Reload Factions");
	printf("| %21s | %51s |\n", "mail", "Reload in-game Mail data");
	printf("| %21s | %51s |\n", "guilds", "Reload Guilds");
	printf("| %21s | %51s |\n", "locations", "Reload Locations data");
	printf("===============================================================================\n");
	if( strlen(sep->arg[1]) > 0 ) {
		// handle reloads here
		if (!strcasecmp(sep->arg[1], "spawns"))
			zone_list.ReloadSpawns();
	}

	return true;
}

bool ConsoleShutdownCommand(Seperator *sep) 
{
	if ( IsNumber(sep->arg[1]) ) {
		int8 shutdown_delay = atoi(sep->arg[1]);
		printf("Shutdown World in %i second(s)...\n", shutdown_delay);
		// shutting down gracefully, warn players.
		char message[4096]; 
		snprintf(message, sizeof(message), "BROADCAST: Server is shutting down in %s second(s)", sep->arg[1]);
		zone_list.HandleGlobalBroadcast(message);
		Sleep(shutdown_delay * 1000);
	}
	else {
		printf("Shutdown World immediately... you probably won't even see this message, huh!\n");
	}
	if( !ContinueLoops )
		RunLoops = false;
	return true;
}

bool ConsoleCancelShutdownCommand(Seperator *sep) 
{
	printf("Cancel World Shutdown...\n");
	ContinueLoops = true;
	return true;
}

bool ConsoleExitCommand(Seperator *sep) 
{
	// I wanted this to be a little more Terminate-y... killkillkill
	printf("Terminate World immediately...\n");
	RunLoops = false;
	return true;
}

bool ConsoleRulesCommand(Seperator *sep) 
{
	/*if( strlen(sep->arg[1]) == 0 ) 
		return false;*/

	printf("Current Active Ruleset");
	if (!strcasecmp(sep->arg[1], "zone")) 
	{
		if (sep->IsNumber(2)) {
			printf(" in Zone ID: %s\n", sep->arg[2]);
		}
		else
			return false;
	}
	else
	{
		printf(" (global):\n");
	}
	printf("===============================================================================\n");
	printf("| %20s | %20s | %29s |\n", "Category", "Type", "Value");
	printf("===============================================================================\n");
	
	return true;
}

bool ConsoleTestCommand(Seperator *sep) 
{
	// devs put whatever test code in here
	printf("Testing Server Guild Rules values:\n");
	printf("AutoJoin: %i\n", rule_manager.GetGlobalRule(R_World, GuildAutoJoin)->GetInt8());
	printf("Guild ID: %i\n", rule_manager.GetGlobalRule(R_World, GuildAutoJoinID)->GetInt32());
	printf("Rank: %i\n", rule_manager.GetGlobalRule(R_World, GuildAutoJoinDefaultRankID)->GetInt8());
	return true;
}
