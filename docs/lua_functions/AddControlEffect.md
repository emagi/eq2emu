### Function: AddControlEffect(Spawn, Type, OnlyAddSpawn)

**Description:**
Apply a Control Effect to a Spawn, this includes Mesmerize, Stun, Root, so on.

**Parameters:**
- `Spawn`: Spawn - The spawn or entity involved.
- `Type`: UInt8 - Control Effect Type
- `OnlyAddSpawn`: Boolean (Optional) - False by default, set to True will only apply Control Effect to Spawn.  Otherwise if Spell all Spawn Targets will apply control effect.

**Returns:** None.

**Notes:**
- See CONTROL_EFFECT_TYPE defines for more details.

**Example:**

```lua
-- Example usage: Will apply Control Effect to Spawn, or if in a Spell Script all Spell Targets, Type 1 is Mesmerize.
AddControlEffect(Spawn, 1)
```
