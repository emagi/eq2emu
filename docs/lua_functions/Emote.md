### Function: Emote(spawn, message, spawn2, player)

**Description:**
The spawn sends an emote to the general area, spawn2 is whom the emote is targetted at.  If player is defined then the emote will only be sent to that player.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `message` (string) - String `message`.
- `spawn2` (Spawn) - Spawn object representing `spawn2`.
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/guiderobes.lua
function equipped(Item, Spawn)
    while HasItem(Spawn, 157245)
    do 
	    PlayAnimation(Spawn, 16583)
	end
	Emote(Spawn, "feels empowered.")
	ModifyHP(Spawn, 1000000000)
end
```
