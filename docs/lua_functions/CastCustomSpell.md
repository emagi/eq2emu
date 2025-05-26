### Function: CastCustomSpell(spell, caster, target)

**Description:**
Casts a custom spell that has been created through a Spell Script.

**Parameters:**
- `spell` (Spell) - Spell object representing `spell`.
- `caster` (Spawn) - Spawn object representing `caster`.
- `target` (Spawn) - Spawn object representing `target`.

**Returns:** None.

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
