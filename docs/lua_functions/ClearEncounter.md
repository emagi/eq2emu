### Function: ClearEncounter(Spawn)

**Description:** Clears the encounter association for the given spawn. In EQ2, NPCs engaged in combat are part of an “encounter” group. This function removes the spawn from any encounter, effectively resetting its fight grouping.

**Parameters:**

`Spawn`: Spawn – The NPC or player whose encounter to clear.

**Returns:** None.

**Example:**

```lua
-- Example usage (after battle ends, remove any leftover encounter tags)
ClearEncounter(NPC)
```