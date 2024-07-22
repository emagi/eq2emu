

#include "region_map.h"
#include "region_map_v1.h"
#include "../../common/Log.h"


#ifdef WIN32
#define _snprintf snprintf
#include <WinSock2.h>
#include <windows.h>
#endif

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp> 
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <cctype>
#include <stdio.h>
#include <fstream>
#include <string.h>

/**
 * @param name
 * @return
 */
inline bool file_exists(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}

/**
 * @param zone_name
 * @return
 */
RegionMap* RegionMap::LoadRegionMapfile(std::string filename, std::string zone_name) {
	std::string loadedFile = "Regions/";
	loadedFile += filename;
	loadedFile += ".EQ2Region";
	FILE* f = fopen(loadedFile.c_str(), "rb");

	LogWrite(REGION__DEBUG, 7, "Region", "Attempting load of %s", filename.c_str());

	if (!f)
	{
		LogWrite(REGION__ERROR, 7, "Region", "Failed to load of %s", filename.c_str());
		return nullptr;
	}

	// Read the string for the zone file name this was created for
	int8 strSize;
	char name[256];
	fread(&strSize, sizeof(int8), 1, f);
	LogWrite(REGION__DEBUG, 7, "Region", "strSize = %u", strSize);

	size_t len = fread(&name, sizeof(char), strSize, f);
	name[len] = '\0';
	LogWrite(REGION__DEBUG, 7, "Region", "name = %s", name);

	string inFileName(name);
	boost::algorithm::to_lower(inFileName);
	string zoneNameLwr(zone_name);
	boost::algorithm::to_lower(zoneNameLwr);

	std::size_t found = inFileName.find(zoneNameLwr);
	// Make sure file contents are for the correct zone
	if (found == std::string::npos) {
		fclose(f);
		LogWrite(REGION__ERROR, 0, "Region", "RegionMap::LoadRegionMapfile() map contents (%s) do not match its name (%s).", inFileName, zoneNameLwr.c_str());
		return nullptr;
	}
	
	int32 regionMapVersion;
	fread(&regionMapVersion, sizeof(int32), 1, f);
	LogWrite(REGION__INFO, 0, "Region", "Loading %s RegionMapVersion = %u", name, regionMapVersion);

	RegionMapV1* regionmap = new RegionMapV1();
	regionmap->Load(f, zoneNameLwr, regionMapVersion);

	return regionmap;
}


void RegionMapRange::AddVersionRange(std::string zoneName) {
  boost::filesystem::path targetDir("Regions/");

  // crash fix since the dir isn't present
  if(!boost::filesystem::is_directory(targetDir))
  {
	LogWrite(REGION__ERROR, 7, "Region", "Unable to find directory %s", targetDir.c_str());
  	return;
  }

  boost::filesystem::recursive_directory_iterator iter(targetDir), eod;
  boost::smatch base_match;
  std::string formula = "(.*\\/|.*\\\\)((" + zoneName + ")(\\-([0-9]+)\\-([0-9]+))?)\\.EQ2Region$";
  boost::regex re(formula.c_str());
  LogWrite(REGION__INFO, 0, "Region", "Region Formula to match: %s\n", formula.c_str());

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
        LogWrite(REGION__INFO, 0, "Region", "Region to Load: %s, size: %i, string: %s, min: %s, max: %s\n", fileName.c_str(), base_match.size(), baseMatch.c_str(), baseMatch2.c_str(), baseMatch3.c_str());
        RegionMap * regionmap = RegionMap::LoadRegionMapfile(base_sub_match.str().c_str(), zoneName);

        int32 min_version = 0, max_version = 0;
        if (strlen(base_sub_match2.str().c_str()) > 0)
          min_version = atoul(base_sub_match2.str().c_str());

        if (strlen(base_sub_match2.str().c_str()) > 0)
          max_version = atoul(base_sub_match3.str().c_str());
        version_map.insert(std::make_pair(new VersionRange(min_version, max_version), regionmap));
      }
    }
  }
}