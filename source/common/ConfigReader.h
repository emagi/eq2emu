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
#ifndef __CONFIG_READER__
#define __CONFIG_READER__
#include <stdio.h>
#include "PacketStruct.h"
#include <map>
#include <string>
#include <vector>
#include "xmlParser.h"
#include "Mutex.h"

using namespace std;

class ConfigReader{
public:
	~ConfigReader();

	void addStruct(const char* name, int16 version, PacketStruct* new_struct);
	PacketStruct* getStruct(const char* name, int16 version);
	PacketStruct* getStructByVersion(const char* name, int16 version);
	void loadDataStruct(PacketStruct* packet, XMLNode parentNode, bool array_packet = false);
	bool processXML_Elements(const char* fileName);
	int16 GetStructVersion(const char* name, int16 version);
	void DestroyStructs();
	void ReloadStructs();
	bool LoadFile(const char* name);
private:
	Mutex MStructs;
	vector<string> load_files;
	map<string, vector<PacketStruct*>*> structs;  
	//vector<PacketStruct*> structs;
};
#endif

