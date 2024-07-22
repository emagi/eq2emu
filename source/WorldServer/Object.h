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
#ifndef __EQ2_OBJECT__
#define __EQ2_OBJECT__

#include "Spawn.h"

class Object : public Spawn{
public:
	Object();
	virtual ~Object();
	void SetClickable(bool click){
		clickable = click;
	}
	void SetZone(char* zone){
		zone_name = zone;
	}
	Object*	Copy();
	bool IsObject(){ return true; }
	void	HandleUse(Client* client, string command);
	bool clickable;
	char* zone_name;
	EQ2Packet* serialize(Player* player, int16 version);

	void SetDeviceID(int8 val) { m_deviceID = val; }
	int8 GetDeviceID() { return m_deviceID; }

private:
	int8 m_deviceID;
};
#endif

