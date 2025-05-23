### Function: SetAccessToEntityCommandByCharID(Spawn, CharID, CommandString, Allowed)

**Description:**
Similar to SetAccessToEntityCommand, but targets a specific player (by character ID) and spawn. It toggles a command’s availability for that one player on the given spawn.

**Parameters:**
- `Spawn`: Spawn – The entity offering the command.
- `CharID`: Int32 – The character ID of the player whose access to adjust.
- `CommandString`: String – The command name.
- `Allowed`: UInt8 – `1` to allow that player to use the command; `0` to deny them.

**Returns:** If successful at setting permission to the entity command, function return's true.

**Example:**

```lua
-- Example usage (allow only a specific player to use a secret door’s “Open” command)
SetAccessToEntityCommandByCharID(SecretDoor, PlayerCharID, "Open", 1)
```