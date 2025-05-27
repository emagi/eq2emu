### Function: HasCollectionsToHandIn(player)

**Description:**

Return's if the player has collections to turn in.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** True if the player has active collections to turn in.

**Example:**

```lua
-- From SpawnScripts/EastFreeport/RennyParvat.lua
function Dialog1(NPC, Spawn)
	FaceTarget(NPC, Spawn)
	Dialog.New(NPC, Spawn)
	Dialog.AddDialog("This is a decent find, I suppose. I can give you a small reward for it.")
	Dialog.AddOption("Thanks a lot.")
	Dialog.Start()
    if HasCollectionsToHandIn(Spawn) then
    HandInCollections(Spawn) 
    end
```
