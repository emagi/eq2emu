### Function: GetDifficulty(spawn)

**Description:**
Gets the difficulty value of the current spawn.  10+ usually represents epics, 6+ heroic, so on.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 of the Spawn's difficulty setting.

**Example:**

```lua
-- From SpawnScripts/Generic/CombatModule.lua
function  combatModule(NPC, Spawn) 
    level = GetLevel(NPC)           -- NPC Level
    difficulty = GetDifficulty(NPC) -- NPC Difficulty || Function in testing phase, default to 6 if necessary.
    levelSwitch(NPC)
    regen(NPC)
    attributes(NPC)
    
end
```
