### Function: DisplayText(player, type, text)

**Description:**
Displays Channel Text on the Player's screen (without a Spawn attributed with a name, just a plain message).  The `type` are based on channel types in https://github.com/emagi/eq2emu/blob/main/docs/data_types/channel_types.md and support will vary on client version.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `type` (uint8) - Integer value `type`.
- `text` (string) - String `text`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Antonica/wanted_gnoll_bandit.lua
function Accepted(Quest, QuestGiver, Player)
if HasItem(Player,3213)then
    DisplayText(Spawn, 34, "You roll up the wanted poster and stuff it in your quest satchle.")
    RemoveItem(Player,3213,1)
end
```
