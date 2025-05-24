### Function: CureByType(cure_count, cure_type, cure_name, cure_level, target, caster)

**Description:**
Cures the control effects.  cure_count controls the amount of cures activated, cure_type will be the control effect type.  If in a spell script and no target it will apply to all spell targets.

**Parameters:**
- `cure_count` (uint8) - Integer value `cure_count`.
- `cure_type` (uint8) - Integer value `cure_type`.
- `cure_name` (string) - String `cure_name`.
- `cure_level` (uint8) - Integer value `cure_level`.
- `target` (Spawn) - Spawn object representing `target`.
- `caster` (Spawn) - Spawn object representing `caster`.

**Returns:** None.

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
