### Function: EnableGameEvent(player, event_name, enabled)

**Description:**
Triggers a game event on the Player's interface/UI, all behavior depends on the event_name.  The `event_name` options are defined in https://github.com/emagi/eq2emu/blob/main/docs/data_types/game_events.md

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `event_name` (string) - String `event_name`.
- `enabled` (uint8) - Integer value `enabled`.

**Returns:** None.

**Example:**

```lua
EnableGameEvent(Player, "UI_SCREENSHOT", 1) -- client takes a screenshot
```
