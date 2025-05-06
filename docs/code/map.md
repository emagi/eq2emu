# File: `map.h`

## Classes

- `GridMapBorder`
- `Map`
- `impl`
- `MapRange`

## Functions

- `float FindBestZ(glm::vec3 &start, glm::vec3 *result, std::map<int32, bool>* ignored_widgets, uint32 *GridID = 0, uint32* WidgetID = 0);`
- `float FindClosestZ(glm::vec3 &start, glm::vec3 *result, std::map<int32, bool>* ignored_widgets, uint32 *GridID = 0, uint32* WidgetID = 0);`
- `bool LineIntersectsZone(glm::vec3 start, glm::vec3 end, float step, std::map<int32, bool>* ignored_widgets, glm::vec3 *result);`
- `bool LineIntersectsZoneNoZLeaps(glm::vec3 start, glm::vec3 end, float step_mag, std::map<int32, bool>* ignored_widgets, glm::vec3 *result);`
- `bool CheckLoS(glm::vec3 myloc, glm::vec3 oloc, std::map<int32, bool>* ignored_widgets);`
- `bool DoCollisionCheck(glm::vec3 myloc, glm::vec3 oloc, std::map<int32, bool>* ignored_widgets, glm::vec3 &outnorm, float &distance);`
- `bool Load(const std::string& filename);`
- `std::string GetFileName() { return m_ZoneFile; }`
- `void SetMapLoaded(bool val) {`
- `bool IsMapLoaded() {`
- `void SetMapLoading(bool val) {`
- `bool IsMapLoading() {`
- `float GetMinX() { return m_MinX; }`
- `float GetMaxX() { return m_MaxX; }`
- `float GetMinY() { return m_MinY; }`
- `float GetMaxY() { return m_MaxY; }`
- `float GetMinZ() { return m_MinZ; }`
- `float GetMaxZ() { return m_MaxZ; }`
- `bool isPointWithinMap(double x, double y, double z, double minX, double minY, double minZ, double maxX, double maxY, double maxZ) {`
- `void SetFileName(std::string newfile) { m_FileName = string(newfile); }`
- `void MapMinMaxY(float y);`
- `void MapGridMinMaxBorderArray(GridMapBorder* border, glm::vec3 a, glm::vec3 b, glm::vec3 c);`
- `void MapGridMinMaxBorder(GridMapBorder* border, glm::vec3 a);`
- `bool IsPointInGrid(GridMapBorder* border, glm::vec3 a, float radius);`
- `std::vector<int32> GetGridsByPoint(glm::vec3 a, float radius);`
- `void RotateVertex(glm::vec3 &v, float rx, float ry, float rz);`
- `void ScaleVertex(glm::vec3 &v, float sx, float sy, float sz);`
- `void TranslateVertex(glm::vec3 &v, float tx, float ty, float tz);`
- `bool LoadV2(FILE *f);`
- `bool LoadV2Deflated(FILE *f);`
- `bool LoadV3Deflated(std::ifstream* file, std::streambuf * const srcbuf);`
- `void Clear();`
- `void AddVersionRange(std::string zoneName);`

## Notable Comments

- /*
- */
