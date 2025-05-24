### Function: GetSpell(spell_id, spell_tier, custom_lua_script)

**Description:**
Gets a spell object reference by spell_id, spell_tier and can supply a custom lua script to run.

**Parameters:**
- `spell_id` (uint32) - Integer value `spell_id`.
- `spell_tier` (uint8) - Integer value `spell_tier`.
- `custom_lua_script` (string) - String `custom_lua_script`.

**Returns:** Spell object reference.

**Example:**

```lua
-- From ItemScripts/AbbatoirCoffee.lua
function cast(Item, Player)
	Spell = GetSpell(5463)
	Regenz = 18.0
	newDuration = 180000
	SetSpellData(Spell, "duration1", newDuration)
	SetSpellData(Spell, "duration2", newDuration)
	SetSpellDataIndex(Spell, 0, Regenz)
	SetSpellDisplayEffect(Spell, 0, "description", "Increases Out-of-Combat Power Regeneration of target by " .. Regenz)
	CastCustomSpell(Spell, Player, Player)
end
```
