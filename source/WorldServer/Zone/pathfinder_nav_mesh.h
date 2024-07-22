#pragma once

#include "pathfinder_interface.h"
#include <string>
#include <DetourNavMesh.h>

static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;

struct NavMeshSetHeader
{
	int magic;
	int version;
	int numTiles;
	dtNavMeshParams params;
};

struct NavMeshTileHeader
{
	dtTileRef tileRef;
	int dataSize;
};

class PathfinderNavmesh : public IPathfinder
{
public:
	PathfinderNavmesh(const std::string &path);
	virtual ~PathfinderNavmesh();

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, int flags = PathingNotDisabled);
	virtual IPath FindPath(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, const PathfinderOptions& opts);
	virtual glm::vec3 GetRandomLocation(const glm::vec3 &start);

private:
	void Clear();
	void Load(const std::string &path);
	void ShowPath(Client *c, const glm::vec3 &start, const glm::vec3 &end);
	dtStatus GetPolyHeightNoConnections(dtPolyRef ref, const float *pos, float *height) const;
	dtStatus GetPolyHeightOnPath(const dtPolyRef *path, const int path_len, const glm::vec3 &pos, float *h) const;

	struct Implementation;
	std::unique_ptr<Implementation> m_impl;
};
