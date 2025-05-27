### Function: KillSpawnByDistance(spawn, max_distance, include_players, send_packet)

**Description:**

Kill's spawns in the distance radius around the Spawn (Entity based, NPC, Player, Bot).  The include_players is false by default, as-is send_packet.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `max_distance` (float) - Float value `max_distance`.
- `include_players` (uint8) - Integer value `include_players`.
- `send_packet` (uint8) - Integer value `send_packet`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/FrostfangSea/qst_scourgeson_x2_rygorr_tent.lua
function KillArea(NPC)
	KillSpawnByDistance(NPC, 20, 0, 0)
end
```
