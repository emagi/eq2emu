### Function: PlaySound(spawn, sound_string, x, y, z, player)

**Description:**
Plays a sound using the sound_string to the area or specified player.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `sound_string` (string) - String `sound_string`.
- `x` (int32) - Integer value `x`.
- `y` (int32) - Integer value `y`.
- `z` (int32) - Integer value `z`.
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/BardCertificationPapers.lua
PlaySound(Player, "sounds/test/endquest.wav", GetX(Player), GetY(Player), GetZ(Player), Player)
```
