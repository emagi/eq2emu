### Function: SetInfoStructUInt(spawn, field, value)

**Description:**
Sets the unsigned integer field to the value provided.  See https://github.com/emagi/eq2emu/blob/main/docs/data_types/info_struct.md for field types.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `field` (string) - String `field`.
- `value` (uint64) - Integer value `value`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/discordimbuedroughspunhexdoll.lua
function examined(Item, Player)
SetInfoStructUInt(Player, "status_points", 20000)
end
```
