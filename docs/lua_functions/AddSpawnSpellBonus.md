### Function: AddSpawnSpellBonus(Spawn, BonusType, Value)

**Description:** Used only in a Spell Script.  Applies a spell bonus or modifier to the specified spawn, versus AddSpellBonus applying to all targets of the Spell. This could be things like increased stats, mitigation, damage, etc., as defined by BonusType.

**Parameters:**

`Spawn`: Spawn – The entity to receive the bonus.
`BonusType`: UInt16 – The type of bonus to apply (as defined in game constants, e.g., a particular stat or resist).  These are based on the item stat types.
`Value`: SInt32 – The value of the bonus to add (could be absolute or percentage depending on type).

**Returns:** None.

**Example:**

```lua
-- See item stat types for BonusType ID's: https://raw.githubusercontent.com/emagi/eq2emu/refs/heads/main/docs/data_types/item_stat_types.md
-- Spell Script Example usage (increase NPC's defense by 50 temporarily during spell's lifetime)
function cast(Caster, Target)
    AddSpawnSpellBonus(Target, 106, 50.0)
end

function remove(Caster, Target)
    RemoveSpawnSpellBonus(Target)
end
```