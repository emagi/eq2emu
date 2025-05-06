### Function: AddLanguage(Player, LanguageID)

**Description:**
Add a Language to a Player by the Language ID in the Database (within the languages table).

**Parameters:**
- `Player`: Spawn - The spawn or entity involved.
- `LanguageID`: int32 - Integer value.

**Returns:** None.

**Notes:**
- See the languages table for the LanguageID.

**Example:**

```lua
-- Example usage: Adds Halasian Language (1) to Player.
AddLanguage(Player, 1)
```
