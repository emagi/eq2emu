### Function: AddCharacterTitle(Spawn, TitleName)

**Description:**
Provides the Spawn (Player Character) a new title they can apply from their Profile.

**Parameters:**
- `Player`: Spawn - The Spawn (Player only) to provide the new title.
- `TitleName`: String - Title to set to the Spawn.

**Returns:** SInt32 -1 if invalid, otherwise provides Title database ID.

**Example:**

```lua
-- Example usage
AddCharacterTitle(Player, "My New Title!")
```
