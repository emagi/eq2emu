### Function: SetTradeskillClass(spawn, value)

**Description:**
Set's the Spawn's tradeskill class to the value provided, class list is available at https://wiki.eq2emu.com/ReferenceLists/ClassList
**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `value` (int32) - Integer value `value`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Baubbleshire/RalaEurocarry.lua
function dlg_39_1(NPC, Spawn)
	FaceTarget(NPC, Spawn)
	conversation = CreateConversation()
if GetTradeskillLevel(Spawn) <2  then
        Quest = GetQuest(Spawn,5749)
        SummonItem(Spawn,1030001,1)
        SetTradeskillLevel(Spawn,2)
        SetTradeskillClass(Spawn,1)
	    SendMessage(Spawn, "You are now an Artisan!")
        SendPopUpMessage(Spawn, "You are now an Artisan!", 200, 200, 200)            
    end
```
