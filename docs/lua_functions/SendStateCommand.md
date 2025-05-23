### Function: SendStateCommand(spawn, new_state, player)

**Description:**
Updates the state of the Spawn and it's behavior, can optionally be sent to a single Player.  The new_state is based on the Appearance ID's https://wiki.eq2emu.com/ReferenceLists by client version.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `new_state` (int32) - Integer value `new_state`.
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** None.

**Example:**

```lua
-- From Quests/TheCryptofBetrayal/forgotten_potion.lua
function Accepted(Quest, QuestGiver, Player)
	FaceTarget(QuestGiver, Player)
	local conversation = CreateConversation()
	if GetClientVersion(Player) == 546 then
	SendStateCommand(QuestGiver, 13061)
	else
    PlayAnimation(QuestGiver, 13061)
    end
```
