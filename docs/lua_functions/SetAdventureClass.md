### Function: SetAdventureClass(spawn, value)

**Description:**
Changes the Spawn(Player) class to the new value.  Class list is available at https://wiki.eq2emu.com/ReferenceLists/ClassList

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `value` (int32) - Integer value `value`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/BardCertificationPapers.lua
    StartDialogConversation(conversation, 2, Item, Player, "You are now known as \n\n"..GetName(Player).." the Bard.")
 	if GetClass(Player)== 1 or GetClass(Player)== 0 then
    SetAdventureClass(Player,35)   
```
