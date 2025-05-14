Function: GetInfoStructUInt(Spawn, FieldName)

Description: Retrieves an unsigned integer field from a spawn’s info struct. This can include things like level, model type, gender, etc.

Parameters:

    Spawn: Spawn – The entity in question.

    FieldName: String – The name of the UInt field to get (e.g., "age", "race_id").

Returns: Int32 – The value of the field.

Example:

-- Example usage (check an NPC's model type for conditional behavior)
if GetInfoStructUInt(NPC, "model_type") == 3 then
    -- model_type 3 (maybe indicating a mounted model)
end