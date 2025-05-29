### Function: BreatheUnderwater(Spawn, Allow)

**Description:** Toggles the underwater breathing ability for the given spawn (typically a player). When allowed, the spawn can breathe indefinitely underwater (no drowning). This is usually triggered by buffs or items like water-breathing spells.  When ran in a Spell Script applies to all Targets of the spell.  Otherwise it will apply only to the Spawn specified, Spawn is required.

**Parameters:**

`Spawn`: Spawn – The character whose underwater breathing is being set.
`Allow`: Boolean – true to grant underwater breathing; false to require normal breath (drowning applies).

**Returns:** None.

**Example:**

```lua
-- Example usage:
BreatheUnderwater(Player, true)
```