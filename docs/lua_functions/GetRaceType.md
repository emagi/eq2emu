### Function: GetRaceType(Spawn)

**Description:**
Retrieves the race type category ID of the specified spawn. Instead of the specific race (human, elf, etc.), this returns a broader category (e.g., humanoid, animal, etc.).

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Example:**

```lua
-- From Spells/Priest/Cleric/Templar/DivineStrike.lua
function cast(Caster, Target, DmgType, MinDmg, MaxDmg)
    SpellDamage(Target, DmgType, MinDmg, MaxDmg)

    --[[ We don't have racetypes on npcs yet
    if GetRaceType(Target) == "Undead" then
        SpellDamage(Target, DmgType, MinDmg, MaxDmg)
    end--]]
end
```
