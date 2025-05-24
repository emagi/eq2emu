### Function: GetFollowTarget(Spawn)

**Description:**
Retrieves the current follow target of a given NPC or pet. If the spawn is following someone, this returns the entity being followed.

**Parameters:**
- `spawn` (Spawn) – Spawn object representing NPC (or player’s pet) that might be following a target.

**Returns:** Spawn – The entity that Spawn is currently following, or nil if it’s not following anyone.

**Example:**

```lua
-- Example usage (check who an escort NPC is following)
local leader = GetFollowTarget(EscortNPC)
if leader == Player then
    Say(EscortNPC, "I am right behind you!")
end
```
