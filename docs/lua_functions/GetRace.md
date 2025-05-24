### Function: GetRace(spawn)

**Description:**
Gets the race of the Spawn. See https://wiki.eq2emu.com/ReferenceLists/RaceList for a list.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 race of the Spawn.

**Example:**

```lua
-- From ItemScripts/ForgeryFreeportCitizenshipPapers.lua
function examined(Item, Player)
local Race = GetRace(Player)
conversation = CreateConversation()
if not HasQuest(Player,BB)
 and not HasQuest(Player,BB_F)
     
 and not HasQuest(Player,BC) 
 and not HasQuest(Player,BC_F) 
     
 and not HasQuest(Player,SB) 
 and not HasQuest(Player,SB_F) 
 
  and not HasQuest(Player,LA) 
 and not HasQuest(Player,LA_F) 
     
 and not HasQuest(Player,SY) 
 and not HasQuest(Player,SY_F) 
  
  and not HasQuest(Player,TS) 
 and not HasQuest(Player,TS_F) then
     
    if CanReceiveQuest(Player,BB) or     
    CanReceiveQuest(Player,BC) or 
    CanReceiveQuest(Player,SB) or 
    CanReceiveQuest(Player,LA) or 
    CanReceiveQuest(Player,SY) or 
    CanReceiveQuest(Player,TS) then
    AddConversationOption(conversation, "[Glance over the forms]","Intro")
    end
```
