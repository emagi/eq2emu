#include "map.h"
#include "raycast_mesh.h"
#include "../../common/Log.h"

#ifdef WIN32
#define _snprintf snprintf
#include <WinSock2.h>
#include <windows.h>
#endif

#include <algorithm>
#include <map>
#include <memory>
#include <tuple>
#include <vector>
#include <fstream>
#include <iostream>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp> 
#include <boost/asio.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

struct Map::impl
{
	RaycastMesh *rm;
};


inline bool file_exists(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}

ThreadReturnType LoadMapAsync(void* mapToLoad)
{
	Map* map = (Map*)mapToLoad;
	map->SetMapLoaded(false);

	
	std::string filename = "Maps/";
	filename += map->GetFileName();

	std::string deflatedFileName = filename + ".EQ2MapDeflated";

	filename += ".EQ2Map";

	if(file_exists(deflatedFileName))
		filename = deflatedFileName;

	map->SetFileName(filename);

	if (map->Load(filename))
		map->SetMapLoaded(true);

	map->SetMapLoading(false);
	THREAD_RETURN(NULL);
}

Map::Map(string zonename, string file) {
	CheckMapMutex.SetName(file + "MapMutex");
	SetMapLoaded(false);
	m_ZoneName = zonename;
	m_ZoneFile = file;
	imp = nullptr;
	m_MinY = 9999999.0f;
	m_MaxY = -9999999.0f;
}

Map::~Map() {
	SetMapLoaded(false);
	if(imp) {
		imp->rm->release();
		safe_delete(imp);
	}
	
	std::map<int32,GridMapBorder*>::iterator itr;
	for(itr = grid_map_border.begin(); itr != grid_map_border.end(); itr++) {
		safe_delete(itr->second);
	}
	grid_map_border.clear();
}

float Map::FindBestZ(glm::vec3 &start, glm::vec3 *result, std::map<int32, bool>* ignored_widgets, uint32* GridID, uint32* WidgetID)
{
	if (!IsMapLoaded())
		return BEST_Z_INVALID;
	if (!imp)
		return BEST_Z_INVALID;

	glm::vec3 tmp;
	if(!result)
		result = &tmp;

	start.z += 1.0f;//RuleI(Map, FindBestZHeightAdjust);
	glm::vec3 from(start.x, start.y, start.z);
	glm::vec3 to(start.x, start.y, BEST_Z_INVALID);
	float hit_distance;
	bool hit = false;

	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance, (RmUint32*)GridID, (RmUint32*)WidgetID, (RmMap*)ignored_widgets);
	if(hit) {
		return result->z;
	}
	
	// Find nearest Z above us
	
	to.z = -BEST_Z_INVALID;
	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance, (RmUint32*)GridID, (RmUint32*)WidgetID, (RmMap*)ignored_widgets);
	if (hit)
	{
		return result->z;
	}
	
	return BEST_Z_INVALID;
}

float Map::FindClosestZ(glm::vec3 &start, glm::vec3 *result, std::map<int32, bool>* ignored_widgets, uint32 *GridID, uint32* WidgetID) {
	if (!IsMapLoaded())
		return false;
	// Unlike FindBestZ, this method finds the closest Z value above or below the specified point.
	//
	if (!imp)
		return false;
	
	float ClosestZ = BEST_Z_INVALID;
	
	glm::vec3 tmp;
	if (!result)
		result = &tmp;
	
	glm::vec3 from(start.x, start.y, start.z);
	glm::vec3 to(start.x, start.y, BEST_Z_INVALID);
	float hit_distance;
	bool hit = false;
	// first check is below us
	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance, (RmUint32*)GridID, (RmUint32*)WidgetID, (RmMap*)ignored_widgets);
	if (hit) {
		ClosestZ = result->z;
		
	}
	
	// Find nearest Z above us
	to.z = -BEST_Z_INVALID;
	hit = imp->rm->raycast((const RmReal*)&from, (const RmReal*)&to, (RmReal*)result, nullptr, &hit_distance, (RmUint32*)GridID, (RmUint32*)WidgetID, (RmMap*)ignored_widgets);
	if (hit) {
		if (std::abs(from.z - result->z) < std::abs(ClosestZ - from.z))
			return result->z;
	}

	return ClosestZ;
}

bool Map::LineIntersectsZone(glm::vec3 start, glm::vec3 end, float step, std::map<int32, bool>* ignored_widgets, glm::vec3 *result) {
	if (!IsMapLoaded())
		return false;
	if(!imp)
		return false;
	return imp->rm->raycast((const RmReal*)&start, (const RmReal*)&end, (RmReal*)result, nullptr, nullptr, nullptr, nullptr, (RmMap*)ignored_widgets);
}

bool Map::LineIntersectsZoneNoZLeaps(glm::vec3 start, glm::vec3 end, float step_mag, std::map<int32, bool>* ignored_widgets, glm::vec3 *result) {
	if (!IsMapLoaded())
		return false;
	if (!imp)
		return false;
	
	float z = BEST_Z_INVALID;
	glm::vec3 step;
	glm::vec3 cur;
	cur.x = start.x;
	cur.y = start.y;
	cur.z = start.z;

	step.x = end.x - start.x;
	step.y = end.y - start.y;
	step.z = end.z - start.z;
	float factor = step_mag / sqrt(step.x*step.x + step.y*step.y + step.z*step.z);

	step.x *= factor;
	step.y *= factor;
	step.z *= factor;

	int steps = 0;

	if (step.x > 0 && step.x < 0.001f)
		step.x = 0.001f;
	if (step.y > 0 && step.y < 0.001f)
		step.y = 0.001f;
	if (step.z > 0 && step.z < 0.001f)
		step.z = 0.001f;
	if (step.x < 0 && step.x > -0.001f)
		step.x = -0.001f;
	if (step.y < 0 && step.y > -0.001f)
		step.y = -0.001f;
	if (step.z < 0 && step.z > -0.001f)
		step.z = -0.001f;

	//while we are not past end
	//always do this once, even if start == end.
	while(cur.x != end.x || cur.y != end.y || cur.z != end.z)
	{
		steps++;
		glm::vec3 me;
		me.x = cur.x;
		me.y = cur.y;
		me.z = cur.z;
		glm::vec3 hit;

		float best_z = FindBestZ(me, &hit, ignored_widgets);
		float diff = best_z - z;
		diff = diff < 0 ? -diff : diff;

		if (z <= BEST_Z_INVALID || best_z <= BEST_Z_INVALID || diff < 12.0)
			z = best_z;
		else
			return true;

		//look at current location
		if(LineIntersectsZone(start, end, step_mag, ignored_widgets, result))
		{
			return true;
		}

		//move 1 step
		if (cur.x != end.x)
			cur.x += step.x;
		if (cur.y != end.y)
			cur.y += step.y;
		if (cur.z != end.z)
			cur.z += step.z;

		//watch for end conditions
		if ( (cur.x > end.x && end.x >= start.x) || (cur.x < end.x && end.x <= start.x) || (step.x == 0) ) {
			cur.x = end.x;
		}
		if ( (cur.y > end.y && end.y >= start.y) || (cur.y < end.y && end.y <= start.y) || (step.y == 0) ) {
			cur.y = end.y;
		}
		if ( (cur.z > end.z && end.z >= start.z) || (cur.z < end.z && end.z < start.z) || (step.z == 0) ) {
			cur.z = end.z;
		}
	}

	//walked entire line and didnt run into anything...
	return false;
}

bool Map::CheckLoS(glm::vec3 myloc, glm::vec3 oloc, std::map<int32, bool>* ignored_widgets)
{
	if (!IsMapLoaded())
		return false;
	if(!imp)
		return false;

	return !imp->rm->raycast((const RmReal*)&myloc, (const RmReal*)&oloc, nullptr, nullptr, nullptr, nullptr, nullptr, (RmMap*)ignored_widgets);
}

// returns true if a collision happens
bool Map::DoCollisionCheck(glm::vec3 myloc, glm::vec3 oloc, std::map<int32, bool>* ignored_widgets, glm::vec3 &outnorm, float &distance) {
	if (!IsMapLoaded())
		return false;
	if(!imp)
		return false;

	return imp->rm->raycast((const RmReal*)&myloc, (const RmReal*)&oloc, nullptr, (RmReal *)&outnorm, (RmReal *)&distance, nullptr, nullptr, (RmMap*)ignored_widgets);
}

Map *Map::LoadMapFile(std::string zonename, std::string file) {

	std::string filename = "Maps/";
	filename += file;
	
	std::string deflatedFileName = filename + ".EQ2MapDeflated";

	filename += ".EQ2Map";

	if(file_exists(deflatedFileName))
		filename = deflatedFileName;

	LogWrite(MAP__INFO, 7, "Map", "Attempting to load Map File [{%s}]", filename.c_str());

	auto m = new Map(zonename, file);
	m->SetMapLoading(true);
	m->SetFileName(filename);
#ifdef WIN32
	_beginthread(LoadMapAsync, 0, (void*)m);
#else
	pthread_t t1;
	pthread_create(&t1, NULL, LoadMapAsync, (void*)m);
	pthread_detach(t1);
#endif

	return m;
}

/**
 * @param filename
 * @return
 */
bool Map::Load(const std::string &filename)
{
	FILE *map_file = fopen(filename.c_str(), "rb");
	if (map_file) {
		LogWrite(MAP__INFO, 7, "Map", "Loading Map File [{%s}]", filename.c_str());
		bool loaded_map_file = LoadV2(map_file);
		fclose(map_file);

			if (loaded_map_file) {
				LogWrite(MAP__INFO, 7, "Map", "Loaded Map File [{%s}]", filename.c_str());
			}
			else {
				LogWrite(MAP__ERROR, 7, "Map", "FAILED Loading Map File [{%s}]", filename.c_str());
			}
			return loaded_map_file;
		}
	else {
		return false;
	}

	return false;
}

struct ModelEntry
{
	struct Poly
	{
		uint32 v1, v2, v3;
		uint8 vis;
	};
	std::vector<glm::vec3> verts;
	std::vector<Poly> polys;
};


bool Map::LoadV2(FILE* f) {

	std::size_t foundDeflated = m_FileName.find(".EQ2MapDeflated");
	if(foundDeflated != std::string::npos)
		return LoadV2Deflated(f);

	// Read the string for the zone file name this was created for
	int8 strSize;
	char name[256];
	fread(&strSize, sizeof(int8), 1, f);
	LogWrite(MAP__DEBUG, 0, "Map", "strSize = %u", strSize);

	size_t len = fread(&name, sizeof(char), strSize, f);
	name[len] = '\0';
	LogWrite(MAP__DEBUG, 0, "Map", "name = %s", name);

	string fileName(name);
	std::size_t found = fileName.find(m_ZoneName);
	// Make sure file contents are for the correct zone
	if (found == std::string::npos) {
		fclose(f);
		LogWrite(MAP__ERROR, 0, "Map", "Map::LoadV2() map contents (%s) do not match its name (%s).", &name, m_ZoneName.c_str());
		return false;
	}
	// Read the min bounds
	fread(&m_MinX, sizeof(float), 1, f);
	fread(&m_MinZ, sizeof(float), 1, f);

	// Read the max bounds
	fread(&m_MaxX, sizeof(float), 1, f);
	fread(&m_MaxZ, sizeof(float), 1, f);

	// Read the number of grids
	int32 NumGrids;
	fread(&NumGrids, sizeof(int32), 1, f);

	std::vector<glm::vec3> verts;
	std::vector<uint32> indices;
	std::vector<uint32> grids;
	std::vector<uint32> widgets;

	uint32 face_count = 0;
	// Loop through the grids loading the face list
	for (int32 i = 0; i < NumGrids; i++) {
		// Read the grid id
		int32 GridID;
		fread(&GridID, sizeof(int32), 1, f);

		// Read the number of vertices
		int32 NumFaces;
		fread(&NumFaces, sizeof(int32), 1, f);

		face_count += NumFaces;
		// Loop through the vertices list reading
		// 3 at a time to creat a triangle (face)
		GridMapBorder* border = GetMapGridBorder(GridID);
		
		for (int32 y = 0; y < NumFaces; ) {
			// Each vertex need an x,y,z coordinate and 
// we will be reading 3 to create the face
			float x1, x2, x3;
			float y1, y2, y3;
			float z1, z2, z3;

			// Read the first vertex
			fread(&x1, sizeof(float), 1, f);
			fread(&y1, sizeof(float), 1, f);
			fread(&z1, sizeof(float), 1, f);
			y++;

			// Read the second vertex
			fread(&x2, sizeof(float), 1, f);
			fread(&y2, sizeof(float), 1, f);
			fread(&z2, sizeof(float), 1, f);
			y++;

			// Read the third (final) vertex
			fread(&x3, sizeof(float), 1, f);
			fread(&y3, sizeof(float), 1, f);
			fread(&z3, sizeof(float), 1, f);
			y++;

			glm::vec3 a(x1, z1, y1);
			glm::vec3 b(x2, z2, y2);
			glm::vec3 c(x3, z3, y3);
			
			MapMinMaxY(y1);
			MapMinMaxY(y2);
			MapMinMaxY(y3);
			
			size_t sz = verts.size();
			verts.push_back(a);
			indices.push_back((uint32)sz);

			verts.push_back(b);
			indices.push_back((uint32)sz + 1);

			verts.push_back(c);
			indices.push_back((uint32)sz + 2);

			grids.push_back((uint32)GridID);
			widgets.push_back((uint32)0);
			MapGridMinMaxBorderArray(border, a, b, c);
		}
	}
	face_count = face_count / 3;

	if (imp) {
		imp->rm->release();
		imp->rm = nullptr;
	}
	else {
		imp = new impl;
	}

	imp->rm = createRaycastMesh((RmUint32)verts.size(), (const RmReal*)&verts[0], face_count, &indices[0], &grids[0], &widgets[0]);

	if (!imp->rm) {
		delete imp;
		imp = nullptr;
		return false;
	}

	return true;
}

bool Map::LoadV3Deflated(std::ifstream* file, std::streambuf * const srcbuf) {
	
	std::vector<glm::vec3> verts;
	std::vector<uint32> indices;
	std::vector<uint32> grids;
	std::vector<uint32> widgets;
	
	int8 strSize = 0;
	char* buf = new char[1024];
	
	int32 mapVersion = 0;
	srcbuf->sgetn(buf,sizeof(int32));
	memcpy(&mapVersion,&buf[0],sizeof(int32));
	LogWrite(MAP__DEBUG, 0, "Map", "MapVersion = %u", mapVersion);
	
	srcbuf->sgetn(buf,sizeof(int8));
	memcpy(&strSize,&buf[0],sizeof(int8));
	LogWrite(MAP__DEBUG, 0, "Map", "strSize = %u", strSize);

	char name[256];
	srcbuf->sgetn(&name[0],strSize);
	name[strSize] = '\0';
	LogWrite(MAP__DEBUG, 0, "Map", "name = %s", name);
	string fileName(name);
	std::size_t found = fileName.find(m_ZoneName);
	// Make sure file contents are for the correct zone
	if (found == std::string::npos) {
		file->close();
		safe_delete_array(buf);
		LogWrite(MAP__ERROR, 0, "Map", "Map::LoadV3Deflated() map contents (%s) do not match its name (%s).", &name, m_ZoneFile.c_str());
		return false;
	}
	// Read the min bounds
	srcbuf->sgetn(buf,sizeof(float));
	memcpy(&m_MinX,&buf[0],sizeof(float));
	srcbuf->sgetn(buf,sizeof(float));
	memcpy(&m_MinY,&buf[0],sizeof(float));
	srcbuf->sgetn(buf,sizeof(float));
	memcpy(&m_MinZ,&buf[0],sizeof(float));

	srcbuf->sgetn(buf,sizeof(float));
	memcpy(&m_MaxX,&buf[0],sizeof(float));
	srcbuf->sgetn(buf,sizeof(float));
	memcpy(&m_MaxY,&buf[0],sizeof(float));
	srcbuf->sgetn(buf,sizeof(float));
	memcpy(&m_MaxZ,&buf[0],sizeof(float));

	// Read the number of grids
	int32 NumGrids;
	srcbuf->sgetn(buf,sizeof(int32));
	memcpy(&NumGrids,&buf[0],sizeof(int32));

	uint32 face_count = 0;
	// Loop through the grids loading the face list
	for (int32 i = 0; i < NumGrids; i++) {
		// Read the grid id
		int32 GridID;
		srcbuf->sgetn(buf,sizeof(int32));
		memcpy(&GridID,&buf[0],sizeof(int32));

		// Read the number of vertices
		int32 vertex_map_count;
		srcbuf->sgetn(buf,sizeof(int32));
		memcpy(&vertex_map_count,&buf[0],sizeof(int32));
		
		GridMapBorder* border = GetMapGridBorder(GridID);
		for(int32 m = 0; m < vertex_map_count; m++) {
			int32 WidgetID;
			srcbuf->sgetn(buf,sizeof(int32));
			memcpy(&WidgetID,&buf[0],sizeof(int32));
			
			float w_x1, w_y1, w_z1;

			// read widget coords
			srcbuf->sgetn(buf,sizeof(float)*3);
			memcpy(&w_x1,&buf[0],sizeof(float));
			memcpy(&w_y1,&buf[4],sizeof(float));
			memcpy(&w_z1,&buf[8],sizeof(float));
			
			glm::vec3 a(w_x1, w_y1, w_z1);
			widget_map.insert(make_pair(WidgetID, a));
			
			int32 NumFaces;
			srcbuf->sgetn(buf,sizeof(int32));
			memcpy(&NumFaces,&buf[0],sizeof(int32));
			
			face_count += NumFaces;	
			
			for (int32 y = 0; y < NumFaces; ) {
						// Each vertex need an x,y,z coordinate and 
			// we will be reading 3 to create the face
						float x1, x2, x3;
						float y1, y2, y3;
						float z1, z2, z3;

						// Read the first vertex
						srcbuf->sgetn(buf,sizeof(float)*3);
						memcpy(&x1,&buf[0],sizeof(float));
						memcpy(&y1,&buf[4],sizeof(float));
						memcpy(&z1,&buf[8],sizeof(float));
						y++;
						
						// Read the second vertex
						srcbuf->sgetn(buf,sizeof(float)*3);
						memcpy(&x2,&buf[0],sizeof(float));
						memcpy(&y2,&buf[4],sizeof(float));
						memcpy(&z2,&buf[8],sizeof(float));
						y++;

						// Read the third (final) vertex
						srcbuf->sgetn(buf,sizeof(float)*3);
						memcpy(&x3,&buf[0],sizeof(float));
						memcpy(&y3,&buf[4],sizeof(float));
						memcpy(&z3,&buf[8],sizeof(float));
						y++;
						
						glm::vec3 a(x1, z1, y1);
						glm::vec3 b(x2, z2, y2);
						glm::vec3 c(x3, z3, y3);
						
						size_t sz = verts.size();
						verts.push_back(a);
						indices.push_back((uint32)sz);

						verts.push_back(b);
						indices.push_back((uint32)sz + 1);

						verts.push_back(c);
						indices.push_back((uint32)sz + 2);
									
						grids.push_back(GridID);
						widgets.push_back(WidgetID);
						MapGridMinMaxBorderArray(border, a, b, c);
					}

		}
		// Loop through the vertices list reading
		// 3 at a time to creat a triangle (face)
	}
	face_count = face_count / 3;

	if (imp) {
		imp->rm->release();
		imp->rm = nullptr;
	}
	else {
		imp = new impl;
	}

	imp->rm = createRaycastMesh((RmUint32)verts.size(), (const RmReal*)&verts[0], face_count, &indices[0], &grids[0], &widgets[0]);

	file->close();
	safe_delete_array(buf);
	if (!imp->rm) {
		delete imp;
		imp = nullptr;
		return false;
	}

	return true;
}

bool Map::LoadV2Deflated(FILE* f) {
    std::ifstream file(m_FileName.c_str(), ios_base::in | ios_base::binary);
    boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
    inbuf.push(boost::iostreams::gzip_decompressor());
    inbuf.push(file);
	ostream out(&inbuf);
	std::streambuf * const srcbuf = out.rdbuf();
	std::streamsize size = srcbuf->in_avail();
	if(size == -1)
	{
		file.close();
		LogWrite(MAP__ERROR, 0, "Map", "Map::LoadV2Deflated() unable to deflate (%s).", m_ZoneFile.c_str());
		return false;
	}
	// Read the string for the zone file name this was created for
	int8 strSize;
	char* buf = new char[1024];
	srcbuf->sgetn(buf,sizeof(int8));
	memcpy(&strSize,&buf[0],sizeof(int8));
	LogWrite(MAP__DEBUG, 0, "Map", "strSize = %u", strSize);

	char name[256];
	srcbuf->sgetn(&name[0],strSize);
	name[strSize] = '\0';
	LogWrite(MAP__DEBUG, 0, "Map", "name = %s", name);
	string fileName(name);
	
	if(fileName.find("EQ2EmuMapTool") != std::string::npos) {
		safe_delete_array(buf);
		return(LoadV3Deflated(&file, srcbuf));
	}
	
	
	std::size_t found = fileName.find(m_ZoneName);
	// Make sure file contents are for the correct zone
	if (found == std::string::npos) {
		file.close();
		safe_delete_array(buf);
		LogWrite(MAP__ERROR, 0, "Map", "Map::LoadV2Deflated() map contents (%s) do not match its name (%s).", &name, m_ZoneFile.c_str());
		return false;
	}
	// Read the min bounds
	srcbuf->sgetn(buf,sizeof(float));
	memcpy(&m_MinX,&buf[0],sizeof(float));
	srcbuf->sgetn(buf,sizeof(float));
	memcpy(&m_MinZ,&buf[0],sizeof(float));

	srcbuf->sgetn(buf,sizeof(float));
	memcpy(&m_MaxX,&buf[0],sizeof(float));
	srcbuf->sgetn(buf,sizeof(float));
	memcpy(&m_MaxZ,&buf[0],sizeof(float));

	// Read the number of grids
	int32 NumGrids;
	srcbuf->sgetn(buf,sizeof(int32));
	memcpy(&NumGrids,&buf[0],sizeof(int32));

	std::vector<glm::vec3> verts;
	std::vector<uint32> indices;
	std::vector<uint32> grids;
	std::vector<uint32> widgets;

	uint32 face_count = 0;
	// Loop through the grids loading the face list
	for (int32 i = 0; i < NumGrids; i++) {
		// Read the grid id
		int32 GridID;
		srcbuf->sgetn(buf,sizeof(int32));
		memcpy(&GridID,&buf[0],sizeof(int32));

		// Read the number of vertices
		int32 NumFaces;
		srcbuf->sgetn(buf,sizeof(int32));
		memcpy(&NumFaces,&buf[0],sizeof(int32));

		face_count += NumFaces;
		// Loop through the vertices list reading
		// 3 at a time to creat a triangle (face)
		GridMapBorder* border = GetMapGridBorder(GridID);
		
		for (int32 y = 0; y < NumFaces; ) {
			// Each vertex need an x,y,z coordinate and 
// we will be reading 3 to create the face
			float x1, x2, x3;
			float y1, y2, y3;
			float z1, z2, z3;

			// Read the first vertex
			srcbuf->sgetn(buf,sizeof(float)*3);
			memcpy(&x1,&buf[0],sizeof(float));
			memcpy(&y1,&buf[4],sizeof(float));
			memcpy(&z1,&buf[8],sizeof(float));
			y++;

			// Read the second vertex
			srcbuf->sgetn(buf,sizeof(float)*3);
			memcpy(&x2,&buf[0],sizeof(float));
			memcpy(&y2,&buf[4],sizeof(float));
			memcpy(&z2,&buf[8],sizeof(float));
			y++;

			// Read the third (final) vertex
			srcbuf->sgetn(buf,sizeof(float)*3);
			memcpy(&x3,&buf[0],sizeof(float));
			memcpy(&y3,&buf[4],sizeof(float));
			memcpy(&z3,&buf[8],sizeof(float));
			y++;

			glm::vec3 a(x1, z1, y1);
			glm::vec3 b(x2, z2, y2);
			glm::vec3 c(x3, z3, y3);

			MapMinMaxY(y1);
			MapMinMaxY(y2);
			MapMinMaxY(y3);
			
			size_t sz = verts.size();
			verts.push_back(a);
			indices.push_back((uint32)sz);

			verts.push_back(b);
			indices.push_back((uint32)sz + 1);

			verts.push_back(c);
			indices.push_back((uint32)sz + 2);

			grids.push_back(GridID);
			widgets.push_back((uint32)0);
			MapGridMinMaxBorderArray(border, a, b, c);
		}
	}
	face_count = face_count / 3;

	if (imp) {
		imp->rm->release();
		imp->rm = nullptr;
	}
	else {
		imp = new impl;
	}

	imp->rm = createRaycastMesh((RmUint32)verts.size(), (const RmReal*)&verts[0], face_count, &indices[0], &grids[0], &widgets[0]);

	file.close();
	safe_delete_array(buf);
	if (!imp->rm) {
		delete imp;
		imp = nullptr;
		return false;
	}

	return true;
}


void Map::RotateVertex(glm::vec3 &v, float rx, float ry, float rz) {
	glm::vec3 nv = v;

	nv.y = (std::cos(rx) * v.y) - (std::sin(rx) * v.z);
	nv.z = (std::sin(rx) * v.y) + (std::cos(rx) * v.z);

	v = nv;

	nv.x = (std::cos(ry) * v.x) + (std::sin(ry) * v.z);
	nv.z = -(std::sin(ry) * v.x) + (std::cos(ry) * v.z);

	v = nv;

	nv.x = (std::cos(rz) * v.x) - (std::sin(rz) * v.y);
	nv.y = (std::sin(rz) * v.x) + (std::cos(rz) * v.y);

	v = nv;
}

void Map::ScaleVertex(glm::vec3 &v, float sx, float sy, float sz) {
	v.x = v.x * sx;
	v.y = v.y * sy;
	v.z = v.z * sz;
}

void Map::TranslateVertex(glm::vec3 &v, float tx, float ty, float tz) {
	v.x = v.x + tx;
	v.y = v.y + ty;
	v.z = v.z + tz;
}

void Map::MapMinMaxY(float y) {
	if(y < m_MinY)
		m_MinY = y;
	if(y > m_MaxY)
		m_MaxY = y;
}

void Map::MapGridMinMaxBorderArray(GridMapBorder* border, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
	if(!border)
		return;
	
	MapGridMinMaxBorder(border, a);
	MapGridMinMaxBorder(border, b);
	MapGridMinMaxBorder(border, c);
}

void Map::MapGridMinMaxBorder(GridMapBorder* border, glm::vec3 a) {
	if(!border)
		return;
	
	if(a.x < border->m_MinX)
		border->m_MinX = a.x;
	if(a.x > border->m_MaxX)
		border->m_MaxX = a.x;
	if(a.y < border->m_MinY)
		border->m_MinY = a.y;
	if(a.y > border->m_MaxY)
		border->m_MaxY = a.y;
	if(a.z < border->m_MinZ)
		border->m_MinZ = a.z;
	if(a.z > border->m_MaxZ)
		border->m_MaxZ = a.z;
}

bool Map::IsPointInGrid(GridMapBorder* border, glm::vec3 a, float radius) {
    return border != nullptr && (a.x >= (border->m_MinX - radius) && a.x <= (border->m_MaxX + radius) && a.y >= (border->m_MinY - radius) && a.y <= (border->m_MaxY + radius) && a.z >= (border->m_MinZ - radius) && a.z <= (border->m_MaxZ + radius));
}

std::vector<int32> Map::GetGridsByPoint(glm::vec3 a, float radius) {
	std::vector<int32> grids;
	std::map<int32,GridMapBorder*>::iterator itr;
	for(itr = grid_map_border.begin(); itr != grid_map_border.end(); itr++) {
		if(IsPointInGrid(itr->second, a, radius)) {
			grids.push_back(itr->first);
		}
	}
	return grids;
}

GridMapBorder* Map::GetMapGridBorder(int32 grid_id, bool instantiate_border) {
		std::map<int32,GridMapBorder*>::iterator itr = grid_map_border.find(grid_id);
		GridMapBorder* border = nullptr;
		if(itr != grid_map_border.end()) {
			border = itr->second;
		}
		else if(instantiate_border) {
			border = new GridMapBorder;
			border->m_MinX = 999999.0f;
			border->m_MaxX = -999999.0f;
			border->m_MinY = 999999.0f;
			border->m_MaxY = -999999.0f;
			border->m_MinZ = 999999.0f;
			border->m_MaxZ = -999999.0f;
			grid_map_border.insert(make_pair(grid_id, border));
	}
	return border;
}

void MapRange::AddVersionRange(std::string zoneName) {
  boost::filesystem::path targetDir("Maps/");

  // crash fix since the dir isn't present
  if(!boost::filesystem::is_directory(targetDir))
  {
	LogWrite(MAP__ERROR, 7, "Map", "Unable to find directory %s", targetDir.c_str());
  	return;
  }

  boost::filesystem::recursive_directory_iterator iter(targetDir), eod;
  boost::smatch base_match;
  std::string formula = "(.*\\/|.*\\\\)((" + zoneName + ")(\\-([0-9]+)\\-([0-9]+))?)(\\.EQ2Map|\\.EQ2MapDeflated)$";
  boost::regex re(formula.c_str());
  LogWrite(MAP__INFO, 0, "Map", "Map Formula to match: %s", formula.c_str());
  BOOST_FOREACH(boost::filesystem::path
    const & i, make_pair(iter, eod)) {
    if (is_regular_file(i)) {
		std::string fileName(i.string());

      if (boost::regex_match(fileName, base_match, re)) {
        boost::ssub_match base_sub_match = base_match[2];
        boost::ssub_match base_sub_match2 = base_match[5];
		boost::ssub_match base_sub_match3 = base_match[6];
		std::string baseMatch(base_sub_match.str().c_str());
		std::string baseMatch2(base_sub_match2.str().c_str());
		std::string baseMatch3(base_sub_match3.str().c_str());
        LogWrite(MAP__INFO, 0, "Map", "Map To Load: %s, size: %i, string: %s, min: %s, max: %s\n", i.string().c_str(), base_match.size(), baseMatch.c_str(), baseMatch2.c_str(), baseMatch3.c_str());

        Map * zonemap = Map::LoadMapFile(zoneName, base_sub_match.str().c_str());

        int32 min_version = 0, max_version = 0;
        if (strlen(base_sub_match2.str().c_str()) > 0)
          min_version = atoul(base_sub_match2.str().c_str());

        if (strlen(base_sub_match2.str().c_str()) > 0)
          max_version = atoul(base_sub_match3.str().c_str());
        version_map.insert(std::make_pair(new VersionRange(min_version, max_version), zonemap));
      }
    }
  }
}

MapRange::MapRange()
{
	
}

MapRange::~MapRange()
{
	Clear();
}

void MapRange::Clear()
{
	map<VersionRange*, Map*>::iterator itr;
	for (itr = version_map.begin(); itr != version_map.end(); itr++)
	{
		VersionRange* range = itr->first;
		Map* map = itr->second;
		delete range;
		delete map;
	}

	version_map.clear();
}

map<VersionRange*, Map*>::iterator MapRange::FindVersionRange(int32 min_version, int32 max_version)
{
	map<VersionRange*, Map*>::iterator itr;
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

map<VersionRange*, Map*>::iterator MapRange::FindMapByVersion(int32 version)
{
	map<VersionRange*, Map*>::iterator enditr = version_map.end();
	map<VersionRange*, Map*>::iterator itr;
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