### Function: GetHateList(spawn)

**Description:**
Returns a list of all spawns currently on the specified NPC’s hate (aggro) list. This includes anyone who has attacked or otherwise generated hate on the NPC.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** Table – A list/array of spawns that the NPC currently hates.

**Example:**

```lua
-- Example usage (apply an effect to all players an epic boss has aggro on)
for _, enemy in ipairs(GetHateList(EpicBoss)) do
    CastSpell(enemy, AOE_DEBUFF_ID, 1, EpicBoss) -- this assumes the AOE_DEBUFF_ID has no cast time so the mob can instantly cast
end
```
