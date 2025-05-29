### Function: BlurVision(Spawn, Intensity)

**Description:** For use in Spell Script or against a Spawn directly.  Sets the intensity of drunkness on the player.  When used in Spell Script applies to all Spell Targets.

**Parameters:**

`Spawn`: Spawn – The player whose vision to affect.
`Intensity`: Float – Intensity of the player being drunk, setting to 0.0 will mean player is not drunk.  The higher the more extreme the screen distortion.

**Returns:** None.

**Example:**

```lua
-- Example usage (blur player's vision while under a drunken effect)
BlurVision(Player, 0.5)
```