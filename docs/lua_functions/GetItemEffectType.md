### Function: GetItemEffectType(item)

**Description:**
Get the effect type of the item.

**Parameters:**
- `item` (Item) - Item object representing `item`.

**Returns:** UInt32 value of the item effect type.

**Example:**

```lua
-- From ItemScripts/cure_test.lua
    function used(Item, Player, Target)
    	local effect_type = GetItemEffectType(Item)
    	
    	if effect_type < 7 then -- 7 is cure all
    		CureByType(1, effect_type, "", (GetLevel(Player) * 1.08) + 1, Target, Player)
    	elseif effect_type == 7 then
    		CureByType(1, 1, "", (GetLevel(Player) * 1.08) + 1, Target, Player)
    		CureByType(1, 2, "", (GetLevel(Player) * 1.08) + 1, Target, Player)
    		CureByType(1, 3, "", (GetLevel(Player) * 1.08) + 1, Target, Player)
    		CureByType(1, 4, "", (GetLevel(Player) * 1.08) + 1, Target, Player)
    		CureByType(1, 5, "", (GetLevel(Player) * 1.08) + 1, Target, Player)
    		CureByType(1, 6, "", (GetLevel(Player) * 1.08) + 1, Target, Player)
    	end
```
