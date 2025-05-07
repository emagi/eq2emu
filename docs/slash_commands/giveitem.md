### Command: /giveitem character_name item_id

**Handler Macro:** COMMAND_GIVEITEM

**Handler Value:** 101

**Required Status:** 10

**Arguments:**
- `arg[0]`: `string character_name`
- `arg[1]`: `string item_id`

**Usage Examples:**
- `/giveitem [character_name] [item_id]`

**Notes:**
- Finds a client by `character_name` on the zone list (of the existing worldserver) and gives the Item based on `item_id`
- Result: Gave `character_name` item id `1234`.