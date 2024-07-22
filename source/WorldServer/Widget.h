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
#ifndef __EQ2_WIDGET__
#define __EQ2_WIDGET__
#include "Spawn.h"
#include "client.h"
#include <string.h>
#include <mutex>

using namespace std;
#define WIDGET_TYPE_GENERIC	0
#define WIDGET_TYPE_DOOR	1
#define WIDGET_TYPE_LIFT	2
class Widget : public Spawn{
public:
	Widget();
	virtual ~Widget();
	bool	IsWidget(){ return true; }
	int32	GetWidgetID();
	void	SetWidgetID(int32 val);
	void	SetWidgetX(float val);
	float	GetWidgetX();
	void	SetWidgetY(float val);
	float	GetWidgetY();
	void	SetWidgetZ(float val);
	float	GetWidgetZ();
	void	SetIncludeLocation(bool val);
	bool	GetIncludeLocation();
	void	SetIncludeHeading(bool val);
	bool	GetIncludeHeading();
	void	SetWidgetIcon(int8 val);
	Widget*	Copy();
	EQ2Packet* serialize(Player* player, int16 version);
	void	HandleTimerUpdate();
	void	OpenDoor();
	void	CloseDoor();
	void	HandleUse(Client* client, string command, int8 overrideWidgetType=0xFF);
	float	GetOpenHeading();
	void	SetOpenHeading(float val);
	float	GetClosedHeading();
	void	SetClosedHeading(float val);
	float	GetOpenY();
	void	SetOpenY(float val);
	float	GetCloseY();
	void	SetCloseY(float val);
	float GetOpenX(){return open_x;}
	float GetOpenZ(){return open_z;}
	float GetCloseX(){return close_x;}
	float GetCloseZ(){return close_z;}
	void SetOpenX(float x){open_x = x;}
	void SetOpenZ(float z){open_z = z;}
	void SetCloseX(float x){close_x = x;}
	void SetCloseZ(float z){close_z = z;}
	int8	GetWidgetType();
	void	SetWidgetType(int8 val);
	bool	IsOpen();
	int32	GetActionSpawnID();
	void	SetActionSpawnID(int32 id);
	int32	GetLinkedSpawnID();
	void	SetLinkedSpawnID(int32 id);
	const char*	GetOpenSound();
	void	SetOpenSound(const char* name);
	const char*	GetCloseSound();
	void	SetCloseSound(const char* name);
	void	SetOpenDuration(int16 val);
	int16	GetOpenDuration();
	void	ProcessUse(Spawn* caller=nullptr);
	void	SetHouseID(int32 val) { m_houseID = val; }
	int32	GetHouseID() { return m_houseID; }

	void	SetMultiFloorLift(bool val) { multi_floor_lift = val; }
	bool	GetMultiFloorLift() { return multi_floor_lift; }

	static	string GetWidgetTypeNameByTypeID(int8 type)
	{
		switch (type)
		{
		case WIDGET_TYPE_DOOR:
			return string("Door");
			break;
		case WIDGET_TYPE_LIFT:
			return string("Lift");
			break;
		}

		return string("Generic");
	}
private:
	int8	widget_type;
	bool	include_location;
	bool	include_heading;
	float	widget_x;
	float	widget_y;
	float	widget_z;
	int32	widget_id;
	float	open_heading;
	float	closed_heading;
	float	open_y;
	float	close_y;
	Widget*	action_spawn;
	int32	action_spawn_id;
	Widget*	linked_spawn;
	int32	linked_spawn_id;
	bool	is_open;
	string	open_sound;
	string	close_sound;
	int16	open_duration;
	int32	m_houseID;
	float   open_x;
	float   open_z;
	float	close_x;
	float	close_z;
	bool	multi_floor_lift;
	std::mutex MWidgetMutex;
};
#endif
