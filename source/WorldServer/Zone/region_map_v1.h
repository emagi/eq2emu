#ifndef EQ2EMU_REGION_MAP_V1_H
#define EQ2EMU_REGION_MAP_V1_H

#include <mutex>
#include <shared_mutex>
#include <map>

#include "region_map.h"

class Client;
class Spawn;

#define SPECIAL_REGION_LAVA_OR_DEATH 4294967293
#define SPECIAL_REGION_WATER		 1

#pragma pack(1)
typedef struct ZBSP_Node {
	int32 node_number;
	float normal[3], splitdistance;
	int32 region;
	int32 special;
	int32 left, right;
} ZBSP_Node;

typedef struct Region_Node {
	int32 region_type;
	float x;
	float y;
	float z;
	float dist;
	string regionEnvFileName;
	string regionName;
	int32 grid_id;
	string regionScriptName;
	int32 vert_count;
	int32 trigger_widget_id;
} Region_Node;
#pragma pack()

struct Region_Status {
	bool inRegion;
	int32 timerTic;
	int32 lastTimerTic;
	int32 regionType;
};

class RegionMapV1 : public RegionMap
{
public:
	RegionMapV1();
	~RegionMapV1();
	
	virtual WaterRegionType ReturnRegionType(const glm::vec3& location, int32 grid_id=0) const;
	virtual bool InWater(const glm::vec3& location, int32 grid_id=0) const;
	virtual bool InLava(const glm::vec3& location, int32 grid_id=0) const;
	virtual bool InLiquid(const glm::vec3& location) const;
	virtual bool InPvP(const glm::vec3& location) const;
	virtual bool InZoneLine(const glm::vec3& location) const;

	virtual void IdentifyRegionsInGrid(Client* client, const glm::vec3& location) const;
	virtual void MapRegionsNearSpawn(Spawn* spawn, Client* client=0) const;
	virtual void UpdateRegionsNearSpawn(Spawn* spawn, Client* client=0) const;
	virtual void TicRegionsNearSpawn(Spawn* spawn, Client* client=0) const;
	
	virtual void InsertRegionNode(ZoneServer* zone, int32 version, std::string regionName, std::string envName, uint32 gridID, uint32 triggerWidgetID, float dist = 0.0f);
	virtual void RemoveRegionNode(std::string regionName);
protected:
	virtual bool Load(FILE *fp, std::string inZoneLowerName, int32 regionVersion);

private:
	WaterRegionType BSPReturnRegionType(int32 node_number, const glm::vec3& location, int32 gridid=0) const;
	WaterRegionType BSPReturnRegionTypeNode(const Region_Node* node, const ZBSP_Node* BSP_Root, int32 node_number, const glm::vec3& location, float distToNode=0.0f) const;

	WaterRegionType BSPReturnRegionWaterRegion(const Region_Node* node, const ZBSP_Node* BSP_Root, int32 node_number, const glm::vec3& location, float distToNode=0.0f) const;
	map<Region_Node*, ZBSP_Node*> Regions;

	WaterRegionType EstablishDistanceAtAngle(const Region_Node* region_node, const ZBSP_Node* current_node, float distance, float absDistance, float absSplitDist, bool checkEdgedAngle=false) const;
	
	std::string TestFile(std::string testFile);

	friend class RegionMap;

	int32 mVersion;
	std::string mZoneNameLower;
	
	mutable std::shared_mutex MRegions;
	std::map<Region_Node*, bool> dead_nodes;
};

#endif
