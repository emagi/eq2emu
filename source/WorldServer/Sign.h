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

#ifndef __EQ2_SIGN__
#define __EQ2_SIGN__
#include "Spawn.h"

#define SIGN_TYPE_GENERIC	0
#define SIGN_TYPE_ZONE		1

using namespace std;

class Sign : public Spawn{
public:
	Sign();
	virtual ~Sign();
	bool	IsSign(){ return true; }
	int32	GetWidgetID();
	void	SetWidgetID(int32 val);
	void	SetWidgetX(float val);
	float	GetWidgetX();
	void	SetWidgetY(float val);
	float	GetWidgetY();
	void	SetWidgetZ(float val);
	float	GetWidgetZ();
	void	SetSignIcon(int8 val);
	Sign*	Copy();
	EQ2Packet* serialize(Player *player, int16 version);
	void	HandleUse(Client* client, string command);
	int8	GetSignType();
	void	SetSignType(int8 val);
	float	GetSignZoneX();
	void	SetSignZoneX(float val);
	float	GetSignZoneY();
	void	SetSignZoneY(float val);
	float	GetSignZoneZ();
	void	SetSignZoneZ(float val);
	float	GetSignZoneHeading();
	void	SetSignZoneHeading(float val);
	float	GetSignDistance();
	void	SetSignDistance(float val);
	int32	GetSignZoneID();
	void	SetSignZoneID(int32 val);
	const char*	GetSignTitle();
	void	SetSignTitle(const char* val);
	const char*	GetSignDescription();
	void	SetSignDescription(const char* val);
	void	SetIncludeLocation(bool val);
	bool	GetIncludeLocation();
	void	SetIncludeHeading(bool val);
	bool	GetIncludeHeading();
	void	SetLanguage(int8 in_language) { language = in_language; }
	int8	GetLanguage() { return language; }
	
private:
	string	description;
	string	title;
	int8	sign_type;
	float	widget_x;
	float	widget_y;
	float	widget_z;
	int32	widget_id;
	float	zone_x;
	float	zone_y;
	float	zone_z;
	float	zone_heading;
	int32	zone_id;
	float	sign_distance;
	bool	include_location;
	bool	include_heading;
	int8	language;
};

#endif
