Function: GetInfoStructFloat(Spawn, FieldName)

Description: Retrieves a floating-point field from a spawn’s info data. Could be used for precise position, speed multipliers, etc. if stored there.

Parameters:

    Spawn: Spawn – The entity whose info to check.

    FieldName: String – The name of the float field.

Returns: Float – The value of the field.

Example:

-- Example usage (get an entity's size scale factor)
local scale = GetInfoStructFloat(NPC, "size")