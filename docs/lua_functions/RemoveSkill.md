### Function: RemoveSkill(player_spawn, skill_id, more_to_remove)

**Description:**
Remove's the skill from the player's skill list.

**Parameters:**
- `player_spawn` (Spawn) - Spawn object representing `player_spawn`.
- `skill_id` (uint32) - Integer value `skill_id`.
- `more_to_remove` (bool) - Boolean flag `more_to_remove`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/BardCertificationPapers.lua
if HasSkill(Player, 1408356869) then -- Martial/Fighter
    RemoveSkill(Player, 1408356869)
end
```
