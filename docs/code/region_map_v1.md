# File: `region_map_v1.h`

## Classes

- `Client`
- `Spawn`
- `Region_Status`
- `RegionMapV1`

## Functions

- `WaterRegionType BSPReturnRegionType(int32 node_number, const glm::vec3& location, int32 gridid=0) const;`
- `WaterRegionType BSPReturnRegionTypeNode(const Region_Node* node, const ZBSP_Node* BSP_Root, int32 node_number, const glm::vec3& location, float distToNode=0.0f) const;`
- `WaterRegionType BSPReturnRegionWaterRegion(const Region_Node* node, const ZBSP_Node* BSP_Root, int32 node_number, const glm::vec3& location, float distToNode=0.0f) const;`
- `WaterRegionType EstablishDistanceAtAngle(const Region_Node* region_node, const ZBSP_Node* current_node, float distance, float absDistance, float absSplitDist, bool checkEdgedAngle=false) const;`
- `std::string TestFile(std::string testFile);`

## Notable Comments

_None detected_
