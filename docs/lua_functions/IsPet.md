### Function: IsPet(Spawn)

**Description:** Checks whether the given spawn is a pet (either a player’s pet or some kind of charmed/temporary pet).

**Parameters:**

`Spawn`: Spawn – The entity to check.

**Returns:** Boolean – true if the spawn is a pet under someone’s control; false otherwise.

**Example:**

```lua
-- Example usage (skip certain actions if the spawn is just a pet)
if not IsPet(TargetSpawn) then
    Attack(NPC, TargetSpawn)
end
```