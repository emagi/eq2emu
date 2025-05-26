### Function: SetSpellDisplayEffect(spell, idx, field, percentage)

**Description:**
Set's the spell display effect value.

**Parameters:**
- `spell` (Spell) - Spell object representing `spell`.
- `idx` (uint32) - Integer value `idx`.
- `field` (string) - String `field`.
- `percentage` (uint8) - Integer value `percentage`.

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
