# File: `CommandsDB.cpp`

## Classes

_None detected_

## Functions

- `int32 WorldDatabase::SaveSpawnTemplate(int32 placement_id, const char* template_name)`
- `bool WorldDatabase::RemoveSpawnTemplate(int32 template_id)`
- `int32 WorldDatabase::CreateSpawnFromTemplateByID(Client* client, int32 template_id)`
- `int32 WorldDatabase::CreateSpawnFromTemplateByName(Client* client, const char* template_name)`
- `bool WorldDatabase::SaveZoneSafeCoords(int32 zone_id, float x, float y, float z, float heading)`
- `bool WorldDatabase::SaveSignZoneToCoords(int32 spawn_id, float x, float y, float z, float heading)`

## Notable Comments

- /*
- */
- // find the spawn_location_id in the template we plan to duplicate
- // insert a new spawn_location_name record
- // get all spawn_location_entries that match the templates spawn_location_id value and insert as new
- // get all spawn_location_placements that match the templates spawn_location_id value and insert as new
- // Note: /spawn templates within current zone_id only, because of spawn_id issues (cannot template an Antonic spawn in Commonlands)
- // find the spawn_location_id in the template we plan to duplicate
- // insert a new spawn_location_name record
- // get all spawn_location_entries that match the templates spawn_location_id value and insert as new
