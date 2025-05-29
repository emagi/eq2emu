### Function: CopySpawnAppearance(SourceSpawn, TargetSpawn)

**Description:** Copies the appearance (race, gender, outfit, etc.) from one spawn to another. This effectively makes the target look identical to the source. Often used for illusion or clone effects.

**Parameters:**

`SourceSpawn`: Spawn – The entity whose appearance to copy.
`TargetSpawn`: Spawn – The entity that will receive the appearance.

**Returns:** None.

**Example:**

```lua
-- Example usage (make a decoy NPC look like the player)
CopySpawnAppearance(Player, DecoyNPC)
```