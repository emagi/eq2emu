# File: `ChestTrap.h`

## Classes

- `ChestTrap`
- `ChestTrapInfo`
- `ChestTrapList`

## Functions

- `int32	GetDBID() { return s_ChestTrapInfo.id; }`
- `sint32	GetApplicableZoneID() { return s_ChestTrapInfo.applicable_zone_id; }`
- `int32	GetMinChestDifficulty() { return s_ChestTrapInfo.min_chest_difficulty; }`
- `int32	GetMaxChestDifficulty() { return s_ChestTrapInfo.max_chest_difficulty; }`
- `int32	GetSpellID() { return s_ChestTrapInfo.spell_id; }`
- `int32	GetSpellTier() { return s_ChestTrapInfo.spell_tier; }`
- `int32 Size();`
- `void AddChestTrap(ChestTrap* trap);`
- `bool GetChestTrap(int32 id, ChestTrap::ChestTrapInfo* cti);`
- `bool GetNextTrap(int32 zoneid, int32 chest_difficulty, ChestTrap::ChestTrapInfo* cti);`
- `void Clear();`
- `bool GetNextChestTrap(ChestTrap::ChestTrapInfo* cti);`
- `bool	IsListLoaded();`
- `void	SetListLoaded(bool val);`
- `void	AddChestTrapList(ChestTrapList* trap, int32 id);`
- `void	SetCycleIterator(map<int32, ChestTrap*>::iterator itr);`
- `void ClearTraps();`
- `void ClearTrapList();`
- `void SetupMutexes();`
- `void InstantiateLists(bool parent);`
- `void shuffleMap(ChestTrapList* list);`

## Notable Comments

- //Constructors **must** always set all ChestTrapInfo as we don't memset so a data value will be wack if not set!
- // instantiate the parent lists for zone/difficulty/etc, later on we will do the inverse of each map, zone->difficulty and difficulty->zone
- // not to be called externally from ChestTrapList/ChestTrap
- // randomized maps so we just iterate the map for our next 'random' result
