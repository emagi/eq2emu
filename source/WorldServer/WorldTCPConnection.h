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
#ifndef WorldTCPCONNECTION_H
#define WorldTCPCONNECTION_H
class WorldTCPConnection
{
public:
	WorldTCPConnection() { }
	virtual ~WorldTCPConnection() { }
	virtual void SendEmoteMessage(const char* to, int32 to_guilddbid, sint16 to_minstatus, int32 type, const char* message, ...) { }
	virtual void SendEmoteMessageRaw(const char* to, int32 to_guilddbid, sint16 to_minstatus, int32 type, const char* message) { }

	virtual inline bool IsConsole() { return false; }
};

#endif
