### Function: GetCanBind(spawn)

**Description:**
Gets if the Spawn(Player) can bind in the current zone.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Returns UINT32 1 if you can bind, otherwise 0.

**Example:**

```lua
-- From Spells/Commoner/SetRecallPoint.lua
function cast(Caster, Target)

	canbind = GetCanBind(Caster)
	incombat = IsInCombat(Caster)
	
	if ( incombat == true)
	then
		SendMessage(Caster, "You cannot use Set Recall Point while in combat.", "red")
	    goto exit;
	end
```
