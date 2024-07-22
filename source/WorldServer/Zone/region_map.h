#ifndef EQ2EMU_REGION_MAP_H
#define EQ2EMU_REGION_MAP_H

#include "../../common/types.h"
#include "../../common/MiscFunctions.h"

#include "position.h"
#include <string>

class Client;
class Spawn;
class ZoneServer;
class Region_Node;
class ZBSP_Node;

enum WaterRegionType : int {
	RegionTypeUnsupported = -2,
	RegionTypeUntagged = -1,
	RegionTypeNormal = 0,
	RegionTypeWater = 1,
	RegionTypeLava = 2,
	RegionTypeZoneLine = 3,
	RegionTypePVP = 4,
	RegionTypeSlime = 5,
	RegionTypeIce = 6,
	RegionTypeVWater = 7
};

enum WaterRegionClass : int32 {
	ClassWaterVolume = 0, // matching .region file type by name "watervol"
	ClassWaterRegion = 1, // matching .region file type by name "waterregion"
	ClassWaterRegion2 = 2, // represents .region file name "water_region" potentially defunct and just a WaterVolume (0)
	ClassWaterOcean = 3, // represents .region file with "ocean" and a select node as a parent
	ClassWaterCavern = 4, // represents .region file with matches on name "ocean" and "water"
	ClassWaterOcean2 = 5 // represents .region file with matches on name "ocean" without previous matches (no select node parent and no water string match)
};

class RegionMap
{
public:
	RegionMap() { }
	virtual ~RegionMap() { }

	static RegionMap* LoadRegionMapfile(std::string filename, std::string zone_name);
	virtual WaterRegionType ReturnRegionType(const glm::vec3& location, int32 gridid=0) const = 0;
	virtual bool InWater(const glm::vec3& location, int32 gridid=0) const = 0;
	virtual bool InLava(const glm::vec3& location, int32 gridid=0) const = 0;
	virtual bool InLiquid(const glm::vec3& location) const = 0;
	virtual bool InPvP(const glm::vec3& location) const = 0;
	virtual bool InZoneLine(const glm::vec3& location) const = 0;
	
	virtual void IdentifyRegionsInGrid(Client* client, const glm::vec3& location) const = 0;
	virtual void MapRegionsNearSpawn(Spawn* spawn, Client* client=0) const = 0;
	virtual void UpdateRegionsNearSpawn(Spawn* spawn, Client* client=0) const = 0;
	virtual void TicRegionsNearSpawn(Spawn* spawn, Client* client=0) const = 0;
	
	virtual void InsertRegionNode(ZoneServer* zone, int32 version, std::string regionName, std::string envName, uint32 gridID, uint32 triggerWidgetID, float dist = 0.0f) = 0;
	virtual void RemoveRegionNode(std::string regionName) = 0;
protected:
	virtual bool Load(FILE *fp) { return false; }
};


class RegionMapRange {
public:
	RegionMapRange()
	{
		
	}

	~RegionMapRange()
	{
		map<VersionRange*, RegionMap*>::iterator itr;
		for (itr = version_map.begin(); itr != version_map.end(); itr++)
		{
			VersionRange* range = itr->first;
			RegionMap* map = itr->second;
			delete range;
			delete map;
		}

		version_map.clear();
	}

	void AddVersionRange(std::string zoneName);

	map<VersionRange*, RegionMap*>::iterator FindVersionRange(int32 min_version, int32 max_version)
	{
		map<VersionRange*, RegionMap*>::iterator itr;
		for (itr = version_map.begin(); itr != version_map.end(); itr++)
		{
			VersionRange* range = itr->first;
			// if min and max version are both in range
			if (range->GetMinVersion() <= min_version && max_version <= range->GetMaxVersion())
				return itr;
			// if the min version is in range, but max range is 0
			else if (range->GetMinVersion() <= min_version && range->GetMaxVersion() == 0)
				return itr;
			// if min version is 0 and max_version has a cap
			else if (range->GetMinVersion() == 0 && max_version <= range->GetMaxVersion())
				return itr;
		}

		return version_map.end();
	}

	map<VersionRange*, RegionMap*>::iterator FindRegionByVersion(int32 version)
	{
		map<VersionRange*, RegionMap*>::iterator enditr = version_map.end();
		map<VersionRange*, RegionMap*>::iterator itr;
		for (itr = version_map.begin(); itr != version_map.end(); itr++)
		{
			VersionRange* range = itr->first;
			// if min and max version are both in range
			if(range->GetMinVersion() == 0 && range->GetMaxVersion() == 0)
				enditr = itr;
			else if (version >= range->GetMinVersion() && version <= range->GetMaxVersion())
				return itr;
		}

		return enditr;
	}

	map<VersionRange*, RegionMap*>::iterator GetRangeEnd() { return version_map.end(); }
private:
	std::map<VersionRange*, RegionMap*> version_map;
	string name;
};

#endif
