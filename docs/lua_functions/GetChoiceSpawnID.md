### Function: GetChoiceSpawnID(spawn, commandMatch, declineValue)

**Description:**
Spawn represents the Player with the choice window.  Return's the spawn_id if the choice was determined from a previously provided CreateChoiceWindow.  This remains until ClearChoice is called.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `commandMatch` (string) - String `commandMatch`.
- `declineValue` (uint8) - Integer value `declineValue`.

**Returns:** UInt32 of the spawn_id the Player is using the choice dialog with.

**Example:**

Example Required
