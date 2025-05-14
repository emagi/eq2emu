### Function: IsPlayer(Player)

**Description:**
Checks if Player is an actual Player or Spawn.

**Parameters:**
- `Player`: Spawn - The spawn or entity to check.

Returns: Boolean – true if this spawn is a Player; false if not.

**Example:**

-- Example usage (NPC will attack stealthed players only if it can see invis)
if IsPlayer(Target) or CanSeeInvis(NPC, Target) then
    Attack(NPC, Target)
end