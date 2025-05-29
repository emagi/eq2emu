### Function: RemoveSpawnSpellBonus(Spawn)

**Description:** Used in a Spell Script Only.  Removes a previously applied spell bonus from the spawn

**Parameters:**

`Spawn`: Spawn – The entity to remove the bonus from.

**Returns:** None.

**Example:**

```lua
-- Example usage (remove the defense bonus when buff ends)
function remove(Caster, Target)
	RemoveSpawnSpellBonus(Spawn)
end
```