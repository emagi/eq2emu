### Function: ModifyHP(spawn, value)

**Description:**
Sets the Spawn's HP to the value if the value plus the current health is greater than the Spawn's Total HP.  If the current health plus the is less than the total hp, then it will restore the current HP up to the value. 

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `value` (sint32) - Integer value `value`.

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
