# File: `LootDB.cpp`

## Classes

_None detected_

## Functions

- `void WorldDatabase::LoadLoot(ZoneServer* zone)`
- `void WorldDatabase::LoadGlobalLoot(ZoneServer* zone) {`
- `else if (strcmp(type, "Racial") == 0) {`
- `else if (strcmp(type, "Zone") == 0) {`
- `bool WorldDatabase::LoadSpawnLoot(ZoneServer* zone, Spawn* spawn)`
- `void WorldDatabase::AddLootTableToSpawn(Spawn* spawn, int32 loottable_id) {`
- `bool WorldDatabase::RemoveSpawnLootTable(Spawn* spawn, int32 loottable_id) {`

## Notable Comments

- /*
- */
- // First, clear previous loot tables...
- // Load loottable from DB
- // Now, load Loot Drops for configured loot tables
- // Finally, load loot tables into spawns that are set to use these loot tables
- // Load global loot lists
- // Finally, load loot tables into spawns that are set to use these loot tables
- //No error just in case ppl try doing stupid stuff
