#include "region_map_v1.h"
#include "../../common/Log.h"
#include "../client.h"
#include "../Spawn.h"
#include "../LuaInterface.h"
#include "../World.h"

#undef snprintf
#include <boost/filesystem.hpp>

extern LuaInterface* lua_interface;
extern World world;

RegionMapV1::RegionMapV1() {
	mVersion = 1;
}

RegionMapV1::~RegionMapV1() {
    std::unique_lock lock(MRegions);
	map<Region_Node*, ZBSP_Node*>::const_iterator itr;
	int region_num = 0;
	for (itr = Regions.begin(); itr != Regions.end();)
	{
		Region_Node* node = itr->first;
		ZBSP_Node* bsp_node = itr->second;
		map<Region_Node*, ZBSP_Node*>::const_iterator deleteItr = itr;
		itr++;
		Regions.erase(deleteItr);
		safe_delete(node);
		safe_delete_array(bsp_node);
	}

	Regions.clear();
}

WaterRegionType RegionMapV1::ReturnRegionType(const glm::vec3& location, int32 gridid) const {
	return BSPReturnRegionType(1, glm::vec3(location.x, location.y, location.z), gridid);
}

bool RegionMapV1::InWater(const glm::vec3& location, int32 gridid) const {
	return ReturnRegionType(location, gridid) == RegionTypeWater;
}

bool RegionMapV1::InLava(const glm::vec3& location, int32 gridid) const {
	return ReturnRegionType(location, gridid) == RegionTypeLava;
}

bool RegionMapV1::InLiquid(const glm::vec3& location) const {
	return InWater(location) || InLava(location);
}

bool RegionMapV1::InPvP(const glm::vec3& location) const {
	return ReturnRegionType(location) == RegionTypePVP;
}

bool RegionMapV1::InZoneLine(const glm::vec3& location) const {
	return ReturnRegionType(location) == RegionTypeZoneLine;
}

std::string RegionMapV1::TestFile(std::string testFile)
{
	std::string tmpStr(testFile);
	std::size_t pos = tmpStr.find("."); 
	if ( pos != testFile.npos )
		tmpStr = testFile.substr (0, pos);
			
	string tmpScript("RegionScripts/");
	tmpScript.append(mZoneNameLower);
	tmpScript.append("/" + tmpStr + ".lua");
	std::ifstream f(tmpScript.c_str());
	return f.good() ? tmpScript : string("");
}


bool RegionMapV1::Load(FILE* fp, std::string inZoneNameLwr, int32 version) {
	mZoneNameLower = string(inZoneNameLwr.c_str());
	
	uint32 region_size;
	if (fread(&region_size, sizeof(uint32), 1, fp) != 1) {
		return false;
	}

	LogWrite(REGION__DEBUG, 0, "RegionMap", "region count = %u", region_size);

	for (int i = 0; i < region_size; i++)
	{
		uint32 region_num;
		if (fread(&region_num, sizeof(uint32), 1, fp) != 1) {
			return false;
		}

		uint32 region_type;
		if (fread(&region_type, sizeof(uint32), 1, fp) != 1) {
			return false;
		}

		float x, y, z, dist;
		if (fread(&x, sizeof(float), 1, fp) != 1) {
			return false;
		}
		if (fread(&y, sizeof(float), 1, fp) != 1) {
			return false;
		}
		if (fread(&z, sizeof(float), 1, fp) != 1) {
			return false;
		}
		if (fread(&dist, sizeof(float), 1, fp) != 1) {
			return false;
		}

		int8 strSize;
		char envName[256] = {""};
		char regionName[256] = {""};
		uint32 grid_id = 0;

		if ( version > 1 )
		{
			fread(&strSize, sizeof(int8), 1, fp);
			LogWrite(REGION__DEBUG, 7, "Region", "Region environment strSize = %u", strSize);

			if(strSize)
			{
				size_t len = fread(&envName, sizeof(char), strSize, fp);
				envName[len] = '\0';
			}

			LogWrite(REGION__DEBUG, 7, "Region", "Region environment file name = %s", envName);

			fread(&strSize, sizeof(int8), 1, fp);
			LogWrite(REGION__DEBUG, 7, "Region", "Region name strSize = %u", strSize);

			if(strSize)
			{
				size_t len = fread(&regionName, sizeof(char), strSize, fp);
				regionName[len] = '\0';
			}
			
			LogWrite(REGION__DEBUG, 7, "Region", "Region name file name = %s", regionName);

			if (fread(&grid_id, sizeof(uint32), 1, fp) != 1) {
				return false;
			}

		}

		int32 bsp_tree_size;
		if (fread(&bsp_tree_size, sizeof(int32), 1, fp) != 1) {
			return false;
		}

		LogWrite(REGION__DEBUG, 7, "Region", "region x,y,z,dist = %f, %f, %f, %f, region bsp tree size: %i\n", x, y, z, dist, bsp_tree_size);

		ZBSP_Node* BSP_Root = new ZBSP_Node[bsp_tree_size];
		if (fread(BSP_Root, sizeof(ZBSP_Node), bsp_tree_size, fp) != bsp_tree_size) {
			LogWrite(REGION__ERROR, 0, "RegionMap", "Failed to load region.");
			return false;
		}

		Region_Node* tmpNode = new Region_Node;
		tmpNode->x = x;
		tmpNode->y = y;
		tmpNode->z = z;
		tmpNode->dist = dist;
		tmpNode->region_type = region_type;
		tmpNode->regionName = string(regionName);
		tmpNode->regionEnvFileName = string(envName);
		tmpNode->grid_id = grid_id;
		tmpNode->regionScriptName = string("");
		
		tmpNode->regionScriptName = TestFile(regionName);
		
		if ( tmpNode->regionScriptName.size() < 1 )
		{
			tmpNode->regionScriptName = TestFile(envName);
		}
		if ( tmpNode->regionScriptName.size() < 1 )
		{
			tmpNode->regionScriptName = TestFile("default");
		}
		
		tmpNode->vert_count = bsp_tree_size;
		
		MRegions.lock();
		Regions.insert(make_pair(tmpNode, BSP_Root));
		MRegions.unlock();
	}

	fclose(fp);

	LogWrite(REGION__DEBUG, 0, "RegionMap", "completed load!");

	return true;
}

void RegionMapV1::IdentifyRegionsInGrid(Client *client, const glm::vec3 &location) const
{
    std::shared_lock lock(MRegions);
	map<Region_Node*, ZBSP_Node*>::const_iterator itr;
	int region_num = 0;

	int32 grid = 0;
	int32 widget_id = 0;
	float x =0.0f,y = 0.0f,z = 0.0f;
	if (client->GetPlayer()->GetMap() != nullptr && client->GetPlayer()->GetMap()->IsMapLoaded())
	{
		auto loc = glm::vec3(location.x, location.z, location.y);
		
		float new_z = client->GetPlayer()->FindBestZ(loc, nullptr, &grid, &widget_id);
		
		std::map<int32, glm::vec3>::iterator itr = client->GetPlayer()->GetMap()->widget_map.find(widget_id);
		if(itr != client->GetPlayer()->GetMap()->widget_map.end()) {
			x = itr->second.x;
			y = itr->second.y;
			z = itr->second.z;
		}
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_RED, "No map to establish grid id, using grid id 0 (attempt match all).");

	client->Message(2, "Region check against location %f / %f / %f. Grid to try: %u, player grid is %u, widget id is %u.  Widget location is %f %f %f.", location.x, location.y, location.z, grid, client->GetPlayer()->GetLocation(), widget_id, x, y, z);
	for (itr = Regions.begin(); itr != Regions.end(); itr++)
	{
		Region_Node *node = itr->first;
		ZBSP_Node *BSP_Root = itr->second;

		if (grid == 0 || node->grid_id == grid)
		{
			float x1 = node->x - location.x;
			float y1 = node->y - location.y;
			float z1 = node->z - location.z;
			float dist = sqrt(x1 *x1 + y1 *y1 + z1 *z1);
			glm::vec3 testLoc(location.x, location.y, location.z);
			if(!BSP_Root) {
				if(client)
				client->Message(CHANNEL_COLOR_YELLOW, "[%s] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, Script: %s.  X: %f, Y: %f, Z: %f, Distance: %f, Widget ID Marker: %u", (widget_id == node->trigger_widget_id) ? "IN REGION" : "WIDGET MARKER", region_num, 
				node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(), node->regionScriptName.c_str(), node->x, node->y, node->z, node->dist, node->trigger_widget_id);
			}
			else if (dist <= node->dist)
			{
				WaterRegionType regionType = RegionTypeUntagged;

				if (node->region_type == ClassWaterRegion)
					regionType = BSPReturnRegionWaterRegion(node, BSP_Root, 1, testLoc, dist);
				else
					regionType = BSPReturnRegionTypeNode(node, BSP_Root, 1, testLoc, dist);

				if (regionType != RegionTypeNormal)
				{
					client->Message(CHANNEL_COLOR_YELLOW, "[DETECTED IN REGION %i] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", regionType, region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
						node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());
				}
				else
				{
					client->Message(CHANNEL_COLOR_RED, "[IN DIST RANGE] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
						node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());
				}
			}
			else
				client->Message(CHANNEL_COLOR_RED, "[OUT OF RANGE] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
					node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());
		}
		else
			client->Message(CHANNEL_COLOR_RED, "[OUT OF GRID] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
				node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());

		region_num++;
	}
}

void RegionMapV1::MapRegionsNearSpawn(Spawn *spawn, Client *client) const
{
    std::shared_lock lock(MRegions);
	map<Region_Node*, ZBSP_Node*>::const_iterator itr;
	int region_num = 0;

	spawn->RegionMutex.writelock();

	glm::vec3 testLoc(spawn->GetX(), spawn->GetY(), spawn->GetZ());
	for (itr = Regions.begin(); itr != Regions.end(); itr++)
	{
		Region_Node *node = itr->first;
		ZBSP_Node *BSP_Root = itr->second;

		if (node->regionScriptName.size() < 1)	// only track ones that are used with LUA scripting
			continue;

		if(!BSP_Root) {
			int32 currentGridID = spawn->GetLocation();
			bool inRegion = false;
			if(!(inRegion = spawn->InRegion(node, nullptr)) && currentGridID == node->grid_id && 
			( node->trigger_widget_id == spawn->trigger_widget_id || (node->dist > 0.0f && spawn->GetDistance(node->x, node->y, node->z) < node->dist)) ) {
				int32 returnValue = spawn->InsertRegionToSpawn(node, nullptr, RegionTypeUntagged);
				if (client)
					client->Message(CHANNEL_COLOR_YELLOW, "[ENTER REGION %i %u] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", RegionTypeUntagged, returnValue, region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
						node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());
			}	
			continue;
		}
		
		float x1 = node->x - testLoc.x;
		float y1 = node->y - testLoc.y;
		float z1 = node->z - testLoc.z;
		float dist = sqrt(x1 *x1 + y1 *y1 + z1 *z1);
		if (dist <= node->dist)
		{
			WaterRegionType regionType = RegionTypeUntagged;

			if (node->region_type == ClassWaterRegion)
				regionType = BSPReturnRegionWaterRegion(node, BSP_Root, 1, testLoc, dist);
			else
				regionType = BSPReturnRegionTypeNode(node, BSP_Root, 1, testLoc, dist);

			if (regionType != RegionTypeNormal)
			{
				if (!spawn->InRegion(node, BSP_Root))
				{
					spawn->DeleteRegion(node, BSP_Root);
					int32 returnValue = spawn->InsertRegionToSpawn(node, BSP_Root, regionType);
					if (client)
						client->Message(CHANNEL_COLOR_YELLOW, "[ENTER REGION %i %u] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", regionType, returnValue, region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
							node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());
				}
			}
			else
			{
				if(spawn->HasRegionTracked(node, BSP_Root, false)) {
					continue;
				} // UpdateRegionsNearSpawn will capture it for nodes that have BSP_Root's
				if (spawn->InRegion(node, BSP_Root))
				{
					if (client)
						client->Message(CHANNEL_COLOR_RED, "[LEAVE REGION] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
							node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());
					WaterRegionType whatWasRegionType = (WaterRegionType) spawn->GetRegionType(node, BSP_Root);
					lua_interface->RunRegionScript(node->regionScriptName, "LeaveRegion", spawn->GetZone(), spawn, whatWasRegionType);
				}
				spawn->DeleteRegion(node, BSP_Root);

				spawn->InsertRegionToSpawn(node, BSP_Root, RegionTypeNormal, false);
				if (client)
					client->Message(CHANNEL_COLOR_RED, "[NEAR REGION] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
						node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());
			}
		}

		region_num++;
	}

	spawn->RegionMutex.releasewritelock();
}

void RegionMapV1::UpdateRegionsNearSpawn(Spawn *spawn, Client *client) const
{
    std::shared_lock lock(MRegions);
	map<map<Region_Node*, ZBSP_Node*>, Region_Status>::iterator testitr;
	int region_num = 0;

	spawn->RegionMutex.writelock();

	glm::vec3 testLoc(spawn->GetX(), spawn->GetY(), spawn->GetZ());

	map<Region_Node*, ZBSP_Node*> deleteNodes;
	for (testitr = spawn->Regions.begin(); testitr != spawn->Regions.end(); testitr++)
	{
		map<Region_Node*, ZBSP_Node*>::const_iterator actualItr = testitr->first.begin();
		Region_Node *node = actualItr->first;
		ZBSP_Node *BSP_Root = actualItr->second;

		std::map<Region_Node*, bool>::const_iterator dead_itr = dead_nodes.find(node);
		if(dead_itr != dead_nodes.end()) {
			deleteNodes.insert(make_pair(node, BSP_Root));
			continue;
		}
		if(!BSP_Root) {
			continue;
		}
		
		float x1 = node->x - testLoc.x;
		float y1 = node->y - testLoc.y;
		float z1 = node->z - testLoc.z;
		float dist = sqrt(x1 *x1 + y1 *y1 + z1 *z1);
		if (dist <= node->dist)
		{
			WaterRegionType regionType = RegionTypeUntagged;

			if (node->region_type == ClassWaterRegion)
				regionType = BSPReturnRegionWaterRegion(node, BSP_Root, 1, testLoc, dist);
			else
				regionType = BSPReturnRegionTypeNode(node, BSP_Root, 1, testLoc, dist);

			if (regionType != RegionTypeNormal)
			{
				if (!testitr->second.inRegion)
				{
					testitr->second.inRegion = true;
					int32 returnValue = 0;
					lua_interface->RunRegionScript(node->regionScriptName, "EnterRegion", spawn->GetZone(), spawn, regionType, &returnValue);
					if (client)
						client->Message(CHANNEL_COLOR_YELLOW, "[ENTER RANGE %i %u] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", regionType, returnValue, region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
							node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());
					testitr->second.timerTic = returnValue;
					testitr->second.lastTimerTic = returnValue ? Timer::GetCurrentTime2() : 0;
				}
			}
			else
			{
				if (testitr->second.inRegion)
				{
					testitr->second.inRegion = false;
					testitr->second.timerTic = 0;
					testitr->second.lastTimerTic = 0;
					if (client)
						client->Message(CHANNEL_COLOR_RED, "[LEAVE RANGE] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
							node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());
					WaterRegionType whatWasRegionType = (WaterRegionType) spawn->GetRegionType(node, BSP_Root);
					lua_interface->RunRegionScript(node->regionScriptName, "LeaveRegion", spawn->GetZone(), spawn, whatWasRegionType);
				}
			}
		}
		else
		{
			if (client)
				client->Message(CHANNEL_COLOR_RED, "[LEAVE RANGE - OOR] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
					node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());
			deleteNodes.insert(make_pair(node, BSP_Root));
		}

		region_num++;
	}

	map<Region_Node*, ZBSP_Node*>::const_iterator deleteItr;
	for (deleteItr = deleteNodes.begin(); deleteItr != deleteNodes.end(); deleteItr++)
	{
		Region_Node *tmpNode = deleteItr->first;
		ZBSP_Node *bspNode = deleteItr->second;
		spawn->DeleteRegion(tmpNode, bspNode);
	}

	spawn->RegionMutex.releasewritelock();
}

void RegionMapV1::TicRegionsNearSpawn(Spawn *spawn, Client *client) const
{
    std::shared_lock lock(MRegions);
	map<map<Region_Node*, ZBSP_Node*>, Region_Status>::iterator testitr;
	int region_num = 0;

	spawn->RegionMutex.writelock();

	for (testitr = spawn->Regions.begin(); testitr != spawn->Regions.end();)
	{
		map<Region_Node*, ZBSP_Node*>::const_iterator actualItr = testitr->first.begin();
		
		if(actualItr == testitr->first.end()) {
			testitr++;
			continue;
		}
		
		Region_Node *node = actualItr->first;
		ZBSP_Node *BSP_Root = actualItr->second;

		std::map<Region_Node*, bool>::const_iterator dead_itr = dead_nodes.find(node);
		if(dead_itr != dead_nodes.end()) {
			testitr++;
			continue;
		}
		
		if(!BSP_Root) {
			bool passDistCheck = false;
			int32 currentGridID = spawn->GetLocation();
			if(testitr->second.timerTic && currentGridID == node->grid_id && (node->trigger_widget_id == spawn->trigger_widget_id || (node->dist > 0.0f && spawn->GetDistance(node->x, node->y, node->z) <= node->dist && (passDistCheck = true)))
				&& Timer::GetCurrentTime2() >= (testitr->second.lastTimerTic + testitr->second.timerTic)) {
					testitr->second.lastTimerTic = Timer::GetCurrentTime2();
					if (client)
						client->Message(CHANNEL_COLOR_RED, "[TICK] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
							node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());

					int32 returnValue = 0;
					lua_interface->RunRegionScript(node->regionScriptName, "Tick", spawn->GetZone(), spawn, RegionTypeUntagged, &returnValue);

					if (returnValue == 1)
					{
						testitr->second.lastTimerTic = 0;
						testitr->second.timerTic = 0;
					}
			}
			else if(currentGridID != node->grid_id || (node->trigger_widget_id != spawn->trigger_widget_id && !passDistCheck)) {
					if (client)
						client->Message(CHANNEL_COLOR_RED, "[LEAVE REGION] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
							node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());
					lua_interface->RunRegionScript(node->regionScriptName, "LeaveRegion", spawn->GetZone(), spawn, RegionTypeUntagged);
				map<map<Region_Node*, ZBSP_Node*>, Region_Status>::iterator endItr = testitr;
				endItr++;
				spawn->DeleteRegion(node, nullptr);
				if(endItr == spawn->Regions.end()) {
					break;
				}
				else {
					testitr++;
					continue;
				}
				
			}
		}
		else if (testitr->second.timerTic && testitr->second.inRegion && Timer::GetCurrentTime2() >= (testitr->second.lastTimerTic + testitr->second.timerTic))
		{
			testitr->second.lastTimerTic = Timer::GetCurrentTime2();
			if (client)
				client->Message(CHANNEL_COLOR_RED, "[TICK] Region %i, GridID: %u, RegionName: %s, RegionEnvFileName: %s, distance: %f, X/Y/Z: %f / %f / %f.  Script: %s", region_num, node->grid_id, node->regionName.c_str(), node->regionEnvFileName.c_str(),
					node->dist, node->x, node->y, node->z, node->regionScriptName.c_str());
			WaterRegionType whatWasRegionType = RegionTypeNormal;	// default will be 0

			if (BSP_Root->special == SPECIAL_REGION_LAVA_OR_DEATH)
				whatWasRegionType = RegionTypeLava;	// 2
			else if (BSP_Root->special == SPECIAL_REGION_WATER)
				whatWasRegionType = RegionTypeWater;	// 1

			int32 returnValue = 0;
			lua_interface->RunRegionScript(node->regionScriptName, "Tick", spawn->GetZone(), spawn, whatWasRegionType, &returnValue);

			if (returnValue == 1)
			{
				testitr->second.lastTimerTic = 0;
				testitr->second.timerTic = 0;
			}
		}

		region_num++;
		testitr++;
	}

	spawn->RegionMutex.releasewritelock();
}

WaterRegionType RegionMapV1::BSPReturnRegionType(int32 node_number, const glm::vec3& location, int32 gridid) const {
    std::shared_lock lock(MRegions);
	map<Region_Node*, ZBSP_Node*>::const_iterator itr;
	int region_num = 0;
	for (itr = Regions.begin(); itr != Regions.end(); itr++)
	{

		Region_Node* node = itr->first;

		// did not match grid id of current region, skip
		//if ( gridid > 0 && gridid != node->grid_id)
		//	continue;

		ZBSP_Node* BSP_Root = itr->second;

		float x1 = node->x - location.x;
		float y1 = node->y - location.y;
		float z1 = node->z - location.z;
		float dist = sqrt(x1 * x1 + y1 * y1 + z1 * z1);

#ifdef REGIONDEBUG
		printf("Region %i (%i) dist %f / node dist %f.  NodeXYZ: %f %f %f, XYZ: %f %f %f.\n", region_num, node->region_type, dist, node->dist, node->x, node->y, node->z, location.x, location.y, location.z);
#endif

		if (dist <= node->dist)
		{
			ZBSP_Node* BSP_Root = itr->second;

			WaterRegionType regionType = RegionTypeUntagged;

			if (node->region_type == ClassWaterRegion)
				regionType = BSPReturnRegionWaterRegion(node, BSP_Root, node_number, location, dist);
			else
				regionType = BSPReturnRegionTypeNode(node, BSP_Root, node_number, location, dist);

			if (regionType != RegionTypeNormal)
				return regionType;
		}
		region_num++;
	}

	return(RegionTypeNormal);
}

WaterRegionType RegionMapV1::BSPReturnRegionTypeNode(const Region_Node* region_node, const ZBSP_Node* BSP_Root, int32 node_number, const glm::vec3& location, float distToNode) const {
	if(node_number > region_node->vert_count)
	{
		LogWrite(REGION__DEBUG, 0, "Region", "Region %s grid %u (%s) - Node %u is out of range for region max vert count of %i.  Hit at location %f %f %f.", 
		region_node->regionName.c_str(), region_node->grid_id, region_node->regionScriptName.c_str(), node_number, region_node->vert_count,
		location.x, location.y, location.z);
		return (RegionTypeWater);
	}
		
	const ZBSP_Node* current_node = &BSP_Root[node_number - 1];
	float distance;

#ifdef REGIONDEBUG
	printf("left = %u, right %u (Size: %i)\n", current_node->left, current_node->right, region_node->vert_count);
#endif

	if (region_node->region_type == ClassWaterRegion2)
	{
		distance = (location.x * current_node->normal[0]) +
			(location.y * current_node->normal[1]) +
			(location.z * current_node->normal[2]) +
			current_node->splitdistance;
	}
	else {
		distance = (location.x * current_node->normal[0]) +
			(location.y * current_node->normal[1]) +
			(location.z * current_node->normal[2]) -
			current_node->splitdistance;
	}

	float absDistance = distance;
	if (absDistance < 0.0f)
		absDistance *= -1.0f;

	float absSplitDist = current_node->splitdistance;
	if (absSplitDist < 0.0f)
		absSplitDist *= -1.0f;

#ifdef REGIONDEBUG
	printf("distance = %f, normals: %f %f %f, location: %f %f %f, split distance: %f\n", distance, current_node->left, current_node->right, current_node->normal[0], current_node->normal[1], current_node->normal[2],
		location.x, location.y, location.z, current_node->splitdistance);
#endif

	if ((current_node->left == -2) &&
		(current_node->right == -1 || current_node->right == -2)) {
		if (region_node->region_type == ClassWaterOcean || region_node->region_type == ClassWaterOcean2)
		{
			if ( region_node->region_type == ClassWaterOcean && current_node->right == -1 &&
			current_node->normal[1] >= 0.9f && distance > 0 )
				return RegionTypeWater;
			else
				return EstablishDistanceAtAngle(region_node, current_node, distance, absDistance, absSplitDist, true);
		}
		else
		{
			if (distance > 0)
				return(RegionTypeWater);
			else
				return RegionTypeNormal;
		}
	}
	else if ((region_node->region_type == ClassWaterOcean || region_node->region_type == ClassWaterOcean2) && current_node->normal[1] != 1.0f && current_node->normal[1] != -1.0f)
	{
		float fraction = abs(current_node->normal[0] * current_node->normal[2]);
		float diff = distToNode / region_node->dist;
		if (distance > 0)
			diff = distance * diff;

#ifdef REGIONDEBUG
		printf("Diff: %f (%f + %f), fraction %f\n", diff, distToNode, distance, fraction);
#endif
		if ((abs(diff) / 2.0f) > (absSplitDist * (1.0f / fraction)) * 2.0f)
			return RegionTypeNormal;
	}

	if (distance == 0.0f) {
		return(RegionTypeNormal);
	}

	if (distance > 0.0f) {

#ifdef REGIONDEBUG
		printf("to left node %i\n", current_node->left);
#endif
		if (current_node->left == -2)
		{
			switch(region_node->region_type)
			{
				case ClassWaterVolume:
				case ClassWaterOcean:
					return RegionTypeWater;
				break;
				
				case ClassWaterOcean2:
					return EstablishDistanceAtAngle(region_node, current_node, distance, absDistance, absSplitDist, false);
				break;
				
				case ClassWaterCavern:
					return EstablishDistanceAtAngle(region_node, current_node, distance, absDistance, absSplitDist, true);
				break;
				
				default:
					return RegionTypeNormal;
				break;
			}
		}
		else if (current_node->left == -1) {
				return(RegionTypeNormal);
		}
		return BSPReturnRegionTypeNode(region_node, BSP_Root, current_node->left + 1, location, distToNode);
	}

#ifdef REGIONDEBUG
	printf("to right node %i, sign bit %i\n", current_node->right, signbit(current_node->normal[1]));
#endif
	if (current_node->right == -1) {
		if (region_node->region_type == ClassWaterOcean2 && signbit(current_node->normal[1]) == 0 && absDistance < absSplitDist)
			return RegionTypeWater;
		else if ((region_node->region_type == ClassWaterOcean || region_node->region_type == ClassWaterOcean2) &&
			(current_node->normal[1] > 0.0f && distance < 0.0f && absDistance < absSplitDist))
		{
			return(RegionTypeWater);
		}
		return(RegionTypeNormal);
	}

	return BSPReturnRegionTypeNode(region_node, BSP_Root, current_node->right + 1, location, distToNode);
}


WaterRegionType RegionMapV1::BSPReturnRegionWaterRegion(const Region_Node* region_node, const ZBSP_Node* BSP_Root, int32 node_number, const glm::vec3& location, float distToNode) const {
	if(node_number > region_node->vert_count)
	{
		LogWrite(REGION__DEBUG, 0, "Region", "Region %s grid %u (%s) - Node %u is out of range for region max vert count of %i.  Hit at location %f %f %f.", 
		region_node->regionName.c_str(), region_node->grid_id, region_node->regionScriptName.c_str(), node_number, region_node->vert_count,
		location.x, location.y, location.z);
		return (RegionTypeNormal);
	}

	const ZBSP_Node* current_node = &BSP_Root[node_number - 1];
	float distance;

#ifdef REGIONDEBUG
	printf("left = %u, right %u\n", current_node->left, current_node->right);
#endif

	distance = (location.x * current_node->normal[0]) +
		(location.y * current_node->normal[1]) +
		(location.z * current_node->normal[2]) -
		current_node->splitdistance;

#ifdef REGIONDEBUG
	printf("distance = %f, normals: %f %f %f, location: %f %f %f, split distance: %f\n", distance, current_node->left, current_node->right, current_node->normal[0], current_node->normal[1], current_node->normal[2],
		location.x, location.y, location.z, current_node->splitdistance);
#endif

	if (distance > 0.0f) {
#ifdef REGIONDEBUG
		printf("to left node %i\n", current_node->left);
#endif
		if (current_node->left == -1) {
			return(RegionTypeNormal);
		}
		else if (current_node->left == -2) {
			switch(current_node->special)
			{
				case SPECIAL_REGION_LAVA_OR_DEATH:
					return(RegionTypeLava);
					break;
				case SPECIAL_REGION_WATER:
					return(RegionTypeWater);
					break;
				default:
					return(RegionTypeUntagged);
					break;
			}
		}
		return BSPReturnRegionWaterRegion(region_node, BSP_Root, current_node->left + 1, location, distToNode);
	}

#ifdef REGIONDEBUG
	printf("to right node %i, sign bit %i\n", current_node->right, signbit(current_node->normal[1]));
#endif

	if (current_node->right == -1) {
		return(RegionTypeNormal);
	}

	return BSPReturnRegionWaterRegion(region_node, BSP_Root, current_node->right + 1, location, distToNode);
}

WaterRegionType RegionMapV1::EstablishDistanceAtAngle(const Region_Node* region_node, const ZBSP_Node* current_node, float distance, float absDistance, float absSplitDist, bool checkEdgedAngle) const {
	float fraction = abs(current_node->normal[0] * current_node->normal[2]);
#ifdef REGIONDEBUG
	printf("Distcheck: %f < %f\n", absDistance, absSplitDist);
#endif
	if (absDistance < absSplitDist &&
		(current_node->normal[0] >= 1.0f || current_node->normal[0] <= -1.0f ||
			(current_node->normal[1] >= .9f && distance < 0.0f) ||
			(current_node->normal[1] <= -.9f && distance > 0.0f)))
	{
		return RegionTypeWater;
	}
	else if (fraction > 0.0f && (region_node->region_type == ClassWaterOcean2 || checkEdgedAngle))
	{
		if (current_node->normal[2] >= 1.0f || current_node->normal[2] <= -1.0f)
			return RegionTypeNormal;
		else if (current_node->normal[1] == 0.0f && (current_node->normal[0] < -0.5f || current_node->normal[0] > 0.5f) &&
			((abs(absDistance * current_node->normal[0]) / 2.0f) < ((abs(absSplitDist * (1.0f / fraction))))))
		{
			return RegionTypeWater;
		}
		else if (current_node->normal[1] == 0.0f && (current_node->normal[2] < -0.5f || current_node->normal[2] > 0.5f) &&
			((abs(absDistance * current_node->normal[2]) / 2.0f) < ((abs(absSplitDist * (1.0f / fraction))))))
		{
			return RegionTypeWater;
		}
	}

	return RegionTypeNormal;
}

void RegionMapV1::InsertRegionNode(ZoneServer* zone, int32 version, std::string regionName, std::string envName, uint32 gridID, uint32 triggerWidgetID, float dist)
{
		Region_Node* tmpNode = new Region_Node;
		
		tmpNode->x = 0.0f;
		tmpNode->y = 0.0f;
		tmpNode->z = 0.0f;
		
		if(!zone)
			return;
		
		Map* current_map = world.GetMap(std::string(zone->GetZoneFile()), version);
		if(current_map) {
			std::map<int32, glm::vec3>::iterator itr = current_map->widget_map.find(triggerWidgetID);
			if(itr != current_map->widget_map.end()) {
				tmpNode->x = itr->second.x;
				tmpNode->y = itr->second.y;
				tmpNode->z = itr->second.z;
			}
		}
		
		tmpNode->dist = dist;
		tmpNode->region_type = RegionTypeUntagged;
		tmpNode->regionName = string(regionName);
		tmpNode->regionEnvFileName = string(envName);
		tmpNode->grid_id = gridID;
		tmpNode->regionScriptName = string("");
		tmpNode->trigger_widget_id = triggerWidgetID;
		
		tmpNode->regionScriptName = TestFile(regionName);
		
		if ( tmpNode->regionScriptName.size() < 1 )
		{
			tmpNode->regionScriptName = TestFile(envName);
		}
		if ( tmpNode->regionScriptName.size() < 1 )
		{
			tmpNode->regionScriptName = TestFile("default");
		}
		
		tmpNode->vert_count = 0;

		ZBSP_Node* BSP_Root = nullptr;
		
		MRegions.lock();
		Regions.insert(make_pair(tmpNode, BSP_Root));
		MRegions.unlock();
}

void RegionMapV1::RemoveRegionNode(std::string name) {
	
    std::unique_lock lock(MRegions);
	map<Region_Node*, ZBSP_Node*>::const_iterator itr;
	for (itr = Regions.begin(); itr != Regions.end();)
	{
		Region_Node *node = itr->first;
		ZBSP_Node *BSP_Root = itr->second;
		if(node->regionName.find(name) != node->regionName.npos) {
			itr = Regions.erase(itr);
			dead_nodes.insert(make_pair(node, true));
			safe_delete(node);
			safe_delete_array(BSP_Root);
		}
		else {
			itr++;
		}
	}
}