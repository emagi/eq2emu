### Function: CastSpell(target, spell_id, spell_tier, caster, custom_cast_time)

**Description:**

Casts a spell on the specified target with the spell_id and spell_tier specified.

**Parameters:**
- `target` (Spawn) - Spawn object representing `target`.
- `spell_id` (uint32) - Integer value `spell_id`.
- `spell_tier` (uint8) - Integer value `spell_tier`.
- `caster` (Spawn) - Spawn object representing `caster`.
- `custom_cast_time` (uint16) - Integer value `custom_cast_time`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/abasicfirework.lua
function used(Item, Player)
CastSpell(Player, 5003, 1) 

end
```
