# File: `pathfinder_waypoint.h`

## Classes

- `PathFileHeader`
- `Node`
- `FindPerson_Point`
- `PathfinderWaypoint`
- `Implementation`

## Functions

- `void Load(const std::string &filename);`
- `void LoadV2(FILE *f, const PathFileHeader &header);`
- `void LoadV3(FILE *f, const PathFileHeader &header);`
- `void ShowNodes();`
- `void ShowPath(Client *c, const glm::vec3 &start, const glm::vec3 &end);`
- `void NodeInfo(Client *c);`
- `void BuildGraph();`
- `void ShowNode(const Node &n);`

## Notable Comments

_None detected_
