### Function: AddImmunitySpell(ImmunityType, Spawn)

**Description:**
Add an Immunity to the Spawn by the Immunity Type.

**Parameters:**
- `ImmunityType`: UInt8 - Immunity Type to add.
- `Spawn`: Spawn - Spawn to apply immunity.

**Returns:** None.

**Notes:**
- See IMMUNITY_TYPE defines for more details.
- If Spawn is specified only the Spawn receives the Immunity, otherwise if it is in a Spell Script we will apply to all Spell Targets.

**Example:**

```lua
-- Example usage: Immunity Type 7 makes NPC immune to AOE spells.
AddImmunitySpell(7, NPC)
```
