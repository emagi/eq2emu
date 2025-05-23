### Function: Shout(spawn, message, player, dist, language)

**Description:**
Send's a shout from the Spawn into the general vicinity based on the distance provided, or otherwise the player if specified.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `message` (string) - String `message`.
- `player` (Spawn) - Spawn object representing `player`.
- `dist` (int32) - Integer value `dist`.
- `language` (int32) - Integer value `language`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Castleview/BupipaGuryup.lua
function Tryout3(NPC,Spawn)
PlayFlavor(NPC, "","Oh... this is wonderful!","happy",0,0,Spawn)
end
```
