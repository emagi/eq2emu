#include "../../common/Log.h"
#include "../client.h"

#include "pathfinder_null.h"
#include "pathfinder_nav_mesh.h"
#include "pathfinder_waypoint.h"
#include <sys/stat.h>

IPathfinder *IPathfinder::Load(const std::string &zone) {
	struct stat statbuffer;
	//std::string navmesh_path = fmt::format("maps/nav/{0}.nav", zone);
	std::string navmesh_path = "Maps/nav/" + zone + ".nav";

	if (stat(navmesh_path.c_str(), &statbuffer) == 0) {
		return new PathfinderNavmesh(navmesh_path);
	}
	else
		LogWrite(MAP__INFO, 7, "Map", "Could not find Navmesh File [{%s}]", navmesh_path.c_str());
	
	return new PathfinderNull();
}
