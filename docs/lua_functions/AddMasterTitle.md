### Function: AddMasterTitle(TitleName, IsPrefix)

**Description:**
Creates a new Title to be used for Characters/Players with AddCharacterTitle

**Parameters:**
- `TitleName`: string - String value of the new master title to be used for characters/players.
- `IsPrefix`: int8 - If this title is a prefix or not, default is 0.

**Returns:** The Database ID of the new Title that is created.

**Example:**

```lua
-- Example usage: Create a new title called "My New Title!" the dbID retrieved can be used to assign to a character with AddCharacterTitle
local dbID = AddMasterTitle("My New Title!")
```
