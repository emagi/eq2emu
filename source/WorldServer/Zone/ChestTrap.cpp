#include "ChestTrap.h"
#include <vector>
#include <string.h>

//required for c++17 compat (random_shuffle removed, replaced with shuffle)
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <random>
#else
#include <algorithm>    // std::random_shuffle
#endif

int32 ChestTrapList::Size() {

	MChestTrapList.readlock(__FUNCTION__, __LINE__);
	int32 size = chesttrap_list.size();
	MChestTrapList.releasereadlock(__FUNCTION__, __LINE__);
	return size;
}

void ChestTrapList::AddChestTrap(ChestTrap* trap) {
	if (trap->GetDBID() < 1)
		return;

	MChestTrapList.writelock(__FUNCTION__, __LINE__);
	if (chesttrap_list.count(trap->GetDBID()) > 0)
	{
		ChestTrap* tmpTrap = chesttrap_list[trap->GetDBID()];
		chesttrap_list.erase(trap->GetDBID());
		safe_delete(tmpTrap);
	}

	chesttrap_list[trap->GetDBID()] = trap;
	MChestTrapList.releasewritelock(__FUNCTION__, __LINE__);
}

bool ChestTrapList::GetChestTrap(int32 id, ChestTrap::ChestTrapInfo* cti) {
	ChestTrap* res = 0;
	MChestTrapList.readlock(__FUNCTION__, __LINE__);
	if (chesttrap_list.count(id) > 0)
		res = chesttrap_list[id];

	memset(cti, 0, sizeof(ChestTrap::ChestTrapInfo));
	if (res)
		memcpy(cti, res->GetChestTrapInfo(), sizeof(ChestTrap::ChestTrapInfo));
	MChestTrapList.releasereadlock(__FUNCTION__, __LINE__);

	return cti;
}

bool ChestTrapList::GetNextTrap(int32 zoneid, int32 chest_difficulty, ChestTrap::ChestTrapInfo* cti)
{
	MChestListsInUse.writelock(__FUNCTION__, __LINE__);
	ChestTrapList* zoneTrapList = GetChestListByZone(zoneid);
	ChestTrapList* zoneDifficultyTrapList = zoneTrapList->GetChestListByDifficulty(chest_difficulty);

	bool ret = zoneTrapList->GetNextChestTrap(cti);
	MChestListsInUse.releasewritelock(__FUNCTION__, __LINE__);

	return ret;
}

void ChestTrapList::Clear() {
	MChestListsInUse.writelock(__FUNCTION__, __LINE__);
	ClearTraps();
	ClearTrapList();
	MChestListsInUse.releasewritelock(__FUNCTION__, __LINE__);
}

bool ChestTrapList::GetNextChestTrap(ChestTrap::ChestTrapInfo* cti) {
	MChestTrapList.readlock(__FUNCTION__, __LINE__);
	if (cycleItr == chesttrap_list.end())
	{
		MChestTrapList.releasereadlock(__FUNCTION__, __LINE__);
		//re-shuffle the map, we reached the end
		shuffleMap(this);
	}
	else
		MChestTrapList.releasereadlock(__FUNCTION__, __LINE__);

	if (cycleItr == chesttrap_list.end())
		return false;

	MChestTrapList.writelock(__FUNCTION__, __LINE__);
	ChestTrap* trap = cycleItr->second;

	memset(cti, 0, sizeof(ChestTrap::ChestTrapInfo));
	if (trap)
		memcpy(cti, trap->GetChestTrapInfo(), sizeof(ChestTrap::ChestTrapInfo));

	cycleItr++;
	MChestTrapList.releasewritelock(__FUNCTION__, __LINE__);

	return true;
}

ChestTrapList* ChestTrapList::GetChestListByDifficulty(int32 difficulty) {
		ChestTrapList* usedList = 0;

		int32 id = 0;
		if (ChestTrapParent)
		{
			usedList = GetChestTrapList(ChestTrapBaseList::DIFFICULTY);
			id = ChestTrapBaseList::DIFFICULTY;
		}
		else
		{
			usedList = GetChestTrapListByID(difficulty);
			id = difficulty;
		}

	if (usedList && usedList->IsListLoaded())
		return usedList;
	else if (!usedList)
	{
		usedList = new ChestTrapList();
		AddChestTrapList(usedList, id);
	}

	MChestTrapList.writelock(__FUNCTION__, __LINE__);
	map<int32, ChestTrap*>::iterator itr;
	for (itr = chesttrap_list.begin(); itr != chesttrap_list.end(); itr++) {
		ChestTrap* curTrap = itr->second;
		if ((curTrap->GetMinChestDifficulty() <= difficulty && difficulty <= curTrap->GetMaxChestDifficulty()) ||
			(curTrap->GetMinChestDifficulty() == 0 && curTrap->GetMaxChestDifficulty() == 0))
			usedList->AddChestTrap(curTrap);
	}

	shuffleMap(usedList);
	usedList->SetListLoaded(true);

	MChestTrapList.releasewritelock(__FUNCTION__, __LINE__);

	return usedList;
}

ChestTrapList* ChestTrapList::GetChestListByZone(int32 zoneid) {
	ChestTrapList* usedList = 0;

	int32 id = 0;
	if (ChestTrapParent)
	{
		usedList = GetChestTrapList(ChestTrapBaseList::ZONE);
		id = ChestTrapBaseList::ZONE;
	}
	else
	{
		usedList = GetChestTrapListByID(zoneid);
		id = zoneid;
	}

	if (usedList && usedList->IsListLoaded())
		return usedList;
	else if (!usedList)
	{
		usedList = new ChestTrapList();
		AddChestTrapList(usedList, id);
	}

	MChestTrapList.writelock(__FUNCTION__, __LINE__);
	map<int32, ChestTrap*>::iterator itr;
	for (itr = chesttrap_list.begin(); itr != chesttrap_list.end(); itr++) {
		ChestTrap* curTrap = itr->second;
		if (curTrap->GetApplicableZoneID() == zoneid || curTrap->GetApplicableZoneID() == -1)
			usedList->AddChestTrap(curTrap);
	}

	shuffleMap(usedList);
	usedList->SetListLoaded(true);

	MChestTrapList.releasewritelock(__FUNCTION__, __LINE__);

	return usedList;
}

map<int32, ChestTrap*>* ChestTrapList::GetAllChestTraps() { return &chesttrap_list; }
bool	ChestTrapList::IsListLoaded() { return ListLoaded; }
void	ChestTrapList::SetListLoaded(bool val) { ListLoaded = val; }

void ChestTrapList::AddChestTrapList(ChestTrapList* traplist, int32 id) {
	if (chesttrap_innerlist.count(id) > 0)
	{
		ChestTrapList* tmpTrapList = chesttrap_innerlist[id];
		chesttrap_innerlist.erase(id);
		safe_delete(tmpTrapList);
	}

	chesttrap_innerlist[id] = traplist;
}


ChestTrapList* ChestTrapList::GetChestTrapList(ChestTrapBaseList listName) {
	ChestTrapList* ctl = 0;
	MChestTrapInnerList.readlock(__FUNCTION__, __LINE__);
	if (chesttrap_innerlist.count(listName) > 0)
		ctl = chesttrap_innerlist[listName];
	MChestTrapInnerList.releasereadlock(__FUNCTION__, __LINE__);

	return ctl;
}

ChestTrapList* ChestTrapList::GetChestTrapListByID(int32 id) {
	ChestTrapList* ctl = 0;
	MChestTrapInnerList.readlock(__FUNCTION__, __LINE__);
	if (chesttrap_innerlist.count(id) > 0)
		ctl = chesttrap_innerlist[id];
	MChestTrapInnerList.releasereadlock(__FUNCTION__, __LINE__);

	return ctl;
}

void ChestTrapList::ClearTraps() {
	MChestTrapList.writelock(__FUNCTION__, __LINE__);
	map<int32, ChestTrap*>::iterator itr;
	for (itr = chesttrap_list.begin(); itr != chesttrap_list.end(); itr++)
		safe_delete(itr->second);
	chesttrap_list.clear();
	MChestTrapList.releasewritelock(__FUNCTION__, __LINE__);
}

void ChestTrapList::ClearTrapList() {
	MChestTrapInnerList.writelock(__FUNCTION__, __LINE__);
	map<int32, ChestTrapList*>::iterator itr2;
	for (itr2 = chesttrap_innerlist.begin(); itr2 != chesttrap_innerlist.end(); itr2++)
		safe_delete(itr2->second);
	chesttrap_innerlist.clear();
	MChestTrapInnerList.releasewritelock(__FUNCTION__, __LINE__);

	// reinstantiate the base lists (zone/difficulty/etc)
	InstantiateLists(ChestTrapParent);
}

void ChestTrapList::SetupMutexes()
{
	MChestTrapList.SetName("ChestTrapList");
	MChestTrapInnerList.SetName("MChestTrapInnerList");
	MChestListsInUse.SetName("MChestListsInUse");
}

void ChestTrapList::InstantiateLists(bool parent)
{
	if (parent)
	{
		difficultyList = new ChestTrapList(false);
		zoneList = new ChestTrapList(false);
		MChestTrapInnerList.writelock(__FUNCTION__, __LINE__);
		chesttrap_innerlist[ChestTrapBaseList::DIFFICULTY] = difficultyList;
		chesttrap_innerlist[ChestTrapBaseList::ZONE] = zoneList;
		MChestTrapInnerList.releasewritelock(__FUNCTION__, __LINE__);
	}
}

void ChestTrapList::shuffleMap(ChestTrapList* list) {
	std::vector<ChestTrap*> tmp_chests;

	map<int32, ChestTrap*>::iterator itr;
	for (itr = chesttrap_list.begin(); itr != chesttrap_list.end(); itr++) {
		ChestTrap* curTrap = itr->second;
		tmp_chests.push_back(curTrap);
	}

#ifdef WIN32
	//c++17/windows removed random_shuffle replaced with this ugly bullshit 9/22/22
	//taken right from their example.
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(tmp_chests.begin(), tmp_chests.end(),g);
#else
	//let linux continue on, with the function as is since it still works.
	std::random_shuffle(tmp_chests.begin(), tmp_chests.end());
#endif

	chesttrap_list.clear();



	int count = 0;

	for (std::vector<ChestTrap*>::iterator it = tmp_chests.begin(); it != tmp_chests.end(); ++it)
	{
		chesttrap_list[count] = *it;
		count++;
	}

	cycleItr = chesttrap_list.begin();
}
