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

#ifndef _CONSOLECOMMANDS_H
#define _CONSOLECOMMANDS_H

#include "../../common/seperator.h"

struct ConsoleCommand
{
	bool(*CommandPointer)(Seperator *);
	const char * Name;					// 10 chars
	const char * ParameterFormat;		// 30 chars
	const char * Description;			// 40 chars
										// = 70 chars
};

	void ProcessConsoleInput(const char * command);

	bool ConsoleBanCommand(Seperator *sep);
	bool ConsoleUnbanCommand(Seperator *sep);
	bool ConsoleKickCommand(Seperator *sep);

	bool ConsoleAnnounceCommand(Seperator *sep);
	bool ConsoleBroadcastCommand(Seperator *sep);
	bool ConsoleChannelCommand(Seperator *sep);
	bool ConsoleTellCommand(Seperator *sep);

	bool ConsoleGuildCommand(Seperator *sep);
	bool ConsolePlayerCommand(Seperator *sep);
	bool ConsoleSetAdminPlayer(Seperator *sep);
	bool ConsoleWorldCommand(Seperator *sep);
	bool ConsoleZoneCommand(Seperator *sep);
	bool ConsoleGetMOTDCommand(Seperator *sep);
	bool ConsoleSetMOTDCommand(Seperator *sep);
	bool ConsoleWhoCommand(Seperator *sep);
	bool ConsoleReloadCommand(Seperator *sep);

	bool ConsoleShutdownCommand(Seperator *sep);
	bool ConsoleCancelShutdownCommand(Seperator *sep);
	bool ConsoleExitCommand(Seperator *sep);
	bool ConsoleRulesCommand(Seperator *sep);
	bool ConsoleTestCommand(Seperator *sep);

#endif