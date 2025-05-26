### Function: SetCharacterTitlePrefix(spawn, titleName)

**Description:**
Set's the character title prefix of the Spawn to the title name.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `titleName` (string) - String `titleName`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/IsleRefuge1/TitleBot.lua
function hailed(NPC, Spawn)
	FaceTarget(NPC, Spawn)
	AddMasterTitle("Lord", 1) -- create new title for all players, is prefix
	AddCharacterTitle(Spawn, "Lord") -- add title to the current player
	SetCharacterTitlePrefix(Spawn, "Lord") -- set the characters current prefix to the newly created title
	
end
```
