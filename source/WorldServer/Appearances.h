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
#include <map>

using namespace std;

// Appearances must use a hash table because of the large amount that exists and the large spacing
// between their ID's.  String and character arrays could not be used for the first iterator because
// it would require the same pointer to access it from the hash table, which is obviously not possible
// since the text is from the client.

// maximum amount of iterations it will attempt to find a entree
#define HASH_SEARCH_MAX 20

class Appearance
{
public:
	// JA: someday add the min_client_version to the map to determine which appearance_id to set per client version
	Appearance(int32 inID, const char *inName, int16 inVer)
	{
		if( !inName )
			return;
		name = string(inName);
		id = inID;
		min_client = inVer;
	}

	int32 GetID() { return id; }
	const char* GetName() { return name.c_str(); }
	int16 GetMinClientVersion() { return min_client; }
	string GetNameString() { return name; }

private:
	int32 id;
	string name;
	int16 min_client;
};

class Appearances
{
public:
	~Appearances(){
		Reset();
	}

	void Reset(){
		ClearAppearances();
	}

	void ClearAppearances(){
		map<int32, Appearance*>::iterator map_list;
		for(map_list = appearanceMap.begin(); map_list != appearanceMap.end(); map_list++ )
			safe_delete(map_list->second);
		appearanceMap.clear();
	}

	void InsertAppearance(Appearance* a){
		appearanceMap[a->GetID()] = a;
	}

	Appearance* FindAppearanceByID(int32 id){
		if(appearanceMap.count(id) > 0)
			return appearanceMap[id];
		return 0;
	}

private:
	map<int32, Appearance*> appearanceMap;
};

