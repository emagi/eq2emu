### Function: GetInfoStructString(spawn, field)

**Description:**
Retrieves a string field from the spawn’s info data structure. The info struct contains various attributes of a spawn (like name, last name, guild, etc.). FieldName is the identifier of the string field.  See https://github.com/emagi/eq2emu/blob/main/docs/data_types/info_struct.md for a full list of options.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `field` (string) - String `field`.

**Returns:** String – The value of that field, or an empty string if not set.

**Example:**

```lua
-- Example usage (get a player's surname)
local surname = GetInfoStructString(Player, "last_name")
```
