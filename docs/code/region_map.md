# File: `region_map.h`

## Classes

- `Client`
- `Spawn`
- `ZoneServer`
- `Region_Node`
- `ZBSP_Node`
- `RegionMap`
- `RegionMapRange`

## Functions

- `void AddVersionRange(std::string zoneName);`
- `else if (range->GetMinVersion() <= min_version && range->GetMaxVersion() == 0)`
- `else if (range->GetMinVersion() == 0 && max_version <= range->GetMaxVersion())`
- `else if (version >= range->GetMinVersion() && version <= range->GetMaxVersion())`

## Notable Comments

- // if min and max version are both in range
- // if the min version is in range, but max range is 0
- // if min version is 0 and max_version has a cap
- // if min and max version are both in range
