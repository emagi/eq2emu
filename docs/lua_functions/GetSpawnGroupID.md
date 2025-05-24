### Function: GetSpawnGroupID(spawn)

**Description:**
Gets a spawn by its group id.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Spawn object reference.

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
