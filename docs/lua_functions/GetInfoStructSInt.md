Function: GetInfoStructSInt(Spawn, FieldName)

Description: Gets a signed integer field from the spawn’s info struct. Similar to GetInfoStructUInt but for fields that can be negative.

Parameters:

    Spawn: Spawn – The entity to query.

    FieldName: String – The field name to retrieve.

Returns: Int32 – The value of that field.

Example:

-- Example usage (get an NPC's faction alignment which could be negative or positive)
local faction = GetInfoStructSInt(NPC, "faction_id")