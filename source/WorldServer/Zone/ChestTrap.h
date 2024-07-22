#include <string>
#include <map>
#include <list>
#include "../../common/Mutex.h"
#include "../../common/types.h"
#pragma once

using namespace std;

enum ChestTrapBaseList {
	DIFFICULTY = 0,
	ZONE = 1
};

class ChestTrap {
public:
	struct ChestTrapInfo {
		int32	id;
		int32	applicable_zone_id;
		int32	min_chest_difficulty;
		int32	max_chest_difficulty;
		int32	spell_id;
		int32	spell_tier;
	};

	//Constructors **must** always set all ChestTrapInfo as we don't memset so a data value will be wack if not set!
	ChestTrap(int32 dbid, sint32 zoneid, int32 mindifficulty, int32 maxdifficulty, int32 spellid, int32 tier)
	{
		s_ChestTrapInfo.id = dbid;
		s_ChestTrapInfo.applicable_zone_id = zoneid;
		s_ChestTrapInfo.min_chest_difficulty = mindifficulty;
		s_ChestTrapInfo.max_chest_difficulty = maxdifficulty;
		s_ChestTrapInfo.spell_id = spellid;
		s_ChestTrapInfo.spell_tier = tier;
	}

	ChestTrap(ChestTrap* parent)
	{
		s_ChestTrapInfo.id = parent->GetDBID();
		s_ChestTrapInfo.applicable_zone_id = parent->GetApplicableZoneID();
		s_ChestTrapInfo.min_chest_difficulty = parent->GetMinChestDifficulty();
		s_ChestTrapInfo.max_chest_difficulty = parent->GetMaxChestDifficulty();
		s_ChestTrapInfo.spell_id = parent->GetSpellID();
		s_ChestTrapInfo.spell_tier = parent->GetSpellTier();
	}

	int32	GetDBID() { return s_ChestTrapInfo.id; }
	sint32	GetApplicableZoneID() { return s_ChestTrapInfo.applicable_zone_id; }
	int32	GetMinChestDifficulty() { return s_ChestTrapInfo.min_chest_difficulty; }
	int32	GetMaxChestDifficulty() { return s_ChestTrapInfo.max_chest_difficulty; }
	int32	GetSpellID() { return s_ChestTrapInfo.spell_id; }
	int32	GetSpellTier() { return s_ChestTrapInfo.spell_tier; }

	ChestTrapInfo* GetChestTrapInfo() { return &s_ChestTrapInfo; }
private:
	ChestTrapInfo s_ChestTrapInfo;
};

class ChestTrapList {
public:
	ChestTrapList() {
		SetupMutexes();

		ChestTrapParent = true;
		// instantiate the parent lists for zone/difficulty/etc, later on we will do the inverse of each map, zone->difficulty and difficulty->zone
		InstantiateLists(true);
		ListLoaded = true;
	}

	// not to be called externally from ChestTrapList/ChestTrap
	ChestTrapList(bool parentClass) {
		SetupMutexes();

		ChestTrapParent = parentClass;

		ListLoaded = false;
	}

	~ChestTrapList() {
			Clear();
	}

	int32 Size();

	void AddChestTrap(ChestTrap* trap);

	bool GetChestTrap(int32 id, ChestTrap::ChestTrapInfo* cti);

	bool GetNextTrap(int32 zoneid, int32 chest_difficulty, ChestTrap::ChestTrapInfo* cti);

	void Clear();
private:
	// randomized maps so we just iterate the map for our next 'random' result
	bool GetNextChestTrap(ChestTrap::ChestTrapInfo* cti);

	ChestTrapList* GetChestListByDifficulty(int32 difficulty);

	ChestTrapList* GetChestListByZone(int32 zoneid);

	map<int32, ChestTrap*>* GetAllChestTraps();
	bool	IsListLoaded();
	void	SetListLoaded(bool val);

	void	AddChestTrapList(ChestTrapList* trap, int32 id);

	void	SetCycleIterator(map<int32, ChestTrap*>::iterator itr);

	ChestTrapList* GetChestTrapList(ChestTrapBaseList listName);
	ChestTrapList* GetChestTrapListByID(int32 id);

	void ClearTraps();
	void ClearTrapList();

	void SetupMutexes();

	void InstantiateLists(bool parent);
	
	void shuffleMap(ChestTrapList* list);

	bool ChestTrapParent;
	bool ListLoaded;
	map<int32, ChestTrap*> chesttrap_list;
	map<int32, ChestTrapList*> chesttrap_innerlist;

	ChestTrapList* difficultyList;
	ChestTrapList* zoneList;

	map<int32, ChestTrap*>::iterator cycleItr;

	Mutex	MChestTrapList;
	Mutex	MChestTrapInnerList;

	Mutex	MChestListsInUse;
};
