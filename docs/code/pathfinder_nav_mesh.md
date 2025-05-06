# File: `pathfinder_nav_mesh.h`

## Classes

- `NavMeshSetHeader`
- `NavMeshTileHeader`
- `PathfinderNavmesh`
- `Implementation`

## Functions

- `void Clear();`
- `void Load(const std::string &path);`
- `void ShowPath(Client *c, const glm::vec3 &start, const glm::vec3 &end);`
- `dtStatus GetPolyHeightNoConnections(dtPolyRef ref, const float *pos, float *height) const;`
- `dtStatus GetPolyHeightOnPath(const dtPolyRef *path, const int path_len, const glm::vec3 &pos, float *h) const;`

## Notable Comments

_None detected_
