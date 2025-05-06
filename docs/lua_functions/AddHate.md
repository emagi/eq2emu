### Function: AddHate(Spawn, NPC, Amount, SendPacket)

**Description:**
Add or Remove hate from a Spawn or Spell Targets if in a LUA Spell.

**Parameters:**
- `Spawn`: Spawn - The target involved for the hate to be added.
- `NPC`: Spawn - NPC related to the hate list we want to update.
- `Amount`: SInt32 - The amount of hate to add or subtract (negative to remove hate).
- `SendPacket`: UInt8 - Default is false, if set to 1 then we will send the threat packet to the client if in a lua spell.

**Returns:** None.

**Example:**

```lua
-- Example usage: Adds 100 hate to the Spawn on the NPC's hate list.
AddHate(Spawn, NPC, 100)
```
