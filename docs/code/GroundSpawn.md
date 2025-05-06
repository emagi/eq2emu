# File: `GroundSpawn.h`

## Classes

- `GroundSpawn`

## Functions

- `bool IsGroundSpawn(){ return true; }`
- `int8 GetNumberHarvests();`
- `void SetNumberHarvests(int8 val);`
- `int8 GetAttemptsPerHarvest();`
- `void SetAttemptsPerHarvest(int8 val);`
- `int32 GetGroundSpawnEntryID();`
- `void SetGroundSpawnEntryID(int32 val);`
- `void ProcessHarvest(Client* client);`
- `void SetCollectionSkill(const char* val);`
- `string GetHarvestMessageName(bool present_tense = false, bool failure = false);`
- `string GetHarvestSpellType();`
- `string GetHarvestSpellName();`
- `void HandleUse(Client* client, string type);`
- `void SetRandomizeHeading(bool val) { randomize_heading = val; }`
- `bool GetRandomizeHeading() { return randomize_heading; }`

## Notable Comments

- /*
- */
