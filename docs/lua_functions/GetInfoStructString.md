Function: GetInfoStructString(Spawn, FieldName)

Description: Retrieves a string field from the spawn’s info data structure. The info struct contains various attributes of a spawn (like name, last name, guild, etc.). FieldName is the identifier of the string field.

Parameters:

    Spawn: Spawn – The entity to query.

    FieldName: String – The field to retrieve (e.g., "last_name", "guild_name").

Returns: String – The value of that field, or an empty string if not set.

Example:

-- Example usage (get a player's surname)
local surname = GetInfoStructString(Player, "last_name")