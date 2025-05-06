# File: `GuildDB.cpp`

## Classes

_None detected_

## Functions

- `void WorldDatabase::LoadGuilds() {`
- `void WorldDatabase::LoadGuild(int32 guild_id) {`
- `int32 WorldDatabase::LoadGuildMembers(Guild* guild) {`
- `else if (high >= 'a' && high <= 'f')`
- `else if (low >= 'a' && low <= 'f')`
- `void WorldDatabase::LoadGuildEvents(Guild* guild) {`
- `void WorldDatabase::LoadGuildRanks(Guild* guild) {`
- `void WorldDatabase::LoadGuildEventFilters(Guild* guild) {`
- `void WorldDatabase::LoadGuildPointsHistory(Guild* guild, GuildMember* guild_member) {`
- `void WorldDatabase::LoadGuildRecruiting(Guild* guild) {`
- `void WorldDatabase::SaveGuild(Guild* guild, bool new_guild) {`
- `void WorldDatabase::SaveGuildMembers(Guild* guild) {`
- `void WorldDatabase::SaveGuildEvents(Guild* guild) {`
- `void WorldDatabase::SaveGuildRanks(Guild* guild) {`
- `void WorldDatabase::SaveGuildEventFilters(Guild* guild) {`
- `void WorldDatabase::SaveGuildPointsHistory(Guild* guild) {`
- `void WorldDatabase::SaveGuildRecruiting(Guild* guild) {`
- `void WorldDatabase::DeleteGuild(Guild* guild) {`
- `void WorldDatabase::DeleteGuildMember(Guild* guild, int32 character_id) {`
- `void WorldDatabase::DeleteGuildEvent(Guild* guild, int64 event_id) {`
- `void WorldDatabase::DeleteGuildPointHistory(Guild* guild, int32 character_id, PointHistory* point_history) {`
- `void WorldDatabase::ArchiveGuildEvent(Guild* guild, GuildEvent* guild_event) {`
- `void WorldDatabase::SaveHiddenGuildEvent(Guild* guild, GuildEvent* guild_event) {`
- `int32 WorldDatabase::GetGuildIDByCharacterID(int32 char_id) {`
- `return atoul(row[0]);`
- `void WorldDatabase::LoadGuildDefaultRanks(Guild* guild) {`
- `void WorldDatabase::LoadGuildDefaultEventFilters(Guild* guild) {`
- `bool WorldDatabase::AddNewPlayerToServerGuild(int32 account_id, int32 char_id)`

## Notable Comments

- /*
- */
- *cpy++ = low | (high << 4);
- /*for (int16 i = 0; i < gm->recruiter_picture_data_size; i++)
- // Check if this servers rule allow auto-joining Server guild
- // if so, what is the guild ID of the default server guild?
- // guild was not valid, abort!
- // guild was found, so what default Rank to make the players? if not set, use 7 (recruit)
- // assuming all is good, insert the new guild member here...
- //gm->adventure_class = player->GetAdventureClass();
