# File: `HousingDB.cpp`

## Classes

_None detected_

## Functions

- `void WorldDatabase::LoadHouseZones() {`
- `int64 WorldDatabase::AddPlayerHouse(int32 char_id, int32 house_id, int32 instance_id, int32 upkeep_due) {`
- `void WorldDatabase::SetHouseUpkeepDue(int32 char_id, int32 house_id, int32 instance_id, int32 upkeep_due) {`
- `void WorldDatabase::UpdateHouseEscrow(int32 house_id, int32 instance_id, int64 amount_coins, int32 amount_status) {`
- `void WorldDatabase::RemovePlayerHouse(int32 char_id, int32 house_id) {`
- `void WorldDatabase::LoadPlayerHouses() {`
- `void WorldDatabase::LoadDeposits(PlayerHouse* ph)`
- `void WorldDatabase::LoadHistory(PlayerHouse* ph)`
- `void WorldDatabase::AddHistory(PlayerHouse* house, char* name, char* reason, int32 timestamp, int64 amount, int32 status, int8 pos_flag)`
- `HouseHistory h(Timer::GetUnixTimeStamp(), amount, string(name), string(reason), status, pos_flag);`

## Notable Comments

_None detected_
