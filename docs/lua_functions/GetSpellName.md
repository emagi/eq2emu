### Function: GetSpellName(spell)

**Description:**
Obtain the Spell Name of the Spell.  Must be used in a Spell Script.

**Parameters:**
- None

**Returns:** None.

**Example:**

```lua
-- From Spells/Priest/Cleric/Inquisitor/HereticsDemise.lua
function remove(Caster, Target, Reason, DoTType, MinVal, MaxVal)
    MinVal = CalculateRateValue(Caster, Target, GetSpellRequiredLevel(Caster), GetLevel(Caster), 1.25, MinVal)
    MaxVal = CalculateRateValue(Caster, Target, GetSpellRequiredLevel(Caster), GetLevel(Caster), 1.25, MaxVal)

	if Reason == "target_dead" then
		local Zone = GetZone(Target)
	    local encounterSpawn = GetSpawnByGroupID(Zone, GetSpawnGroupID(Target))
	    if encounterSpawn ~= nil then
            local targets = GetGroup(encounterSpawn)
        	for k,v in ipairs(targets) do
                SpawnSet(v,"visual_state",0)
                if IsAlive(v) then
                    DamageSpawn(Caster, v, 193, 3, MinVal, MaxVal, GetSpellName())
                end
```
