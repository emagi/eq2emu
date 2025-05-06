### Function: AddPrimaryEntityCommand(Player, Spawn, Name, Distance, Command, ErrorText, CastTime, SpellVisual, DenyListDefault)

**Description:**
Puts a context menu (right click) entity command on the Spawn for the Player (or all to access if Player is nil).

**Parameters:**
- `Player`: Spawn - The Player that can access the entity command.  Optionally set to nil to allow all access.
- `Spawn`: Spawn - The spawn to add the entity command.
- `Name`: string - Text display of the entity command.
- `Distance`: float - Distance the player can access the command otherwise it is gray.
- `Command`: string - Command function to call when used by Player.
- `ErrorText`: string - Error text displayed when cannot use.
- `CastTime`: UInt16 - Time in /10 of a second to complete the command after clicking it.
- `SpellVisual`: UInt32 - Optional spell visual when using the entity command.
- `DenyListDefault`: UInt8 - Default is 0, add to allow, set to 1 to put on deny list.

**Returns:** None.

**Example:**

```lua
-- Example usage: On Hail add Destroy command to Spawn.

function hail(NPC, Spawn)
	AddPrimaryEntityCommand(Spawn, NPC, "Destroy", 5)
end

function casted_on(NPC, Spawn, SpellName)
	if SpellName == 'Destroy' then
		-- perform action
	end	
end
```
