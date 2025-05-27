### Function: AddTimer(spawn, time, function, max_count, player)

**Description:**

Adds a spawn script timer to call the function when triggering at the elapsed time.  The player field can optionally be passed as a field to the function.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `time` (uint32) - Integer value `time`.
- `function` (string) - String `function`.
- `max_count` (uint32) - Integer value `max_count`.
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/ForgeryFreeportCitizenshipPapers.lua
-- removed function AddTimer was in for simplifying example
    AddTimer(Player,1000,"TaskDone",1)

function TaskDone(Item,Player)
CloseItemConversation(Item,Player)
if HasItem(Player,1001112) then
    RemoveItem(Player,1001112,1)
    end
end

```
