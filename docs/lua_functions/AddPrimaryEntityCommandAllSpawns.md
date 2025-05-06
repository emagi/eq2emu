### Function: AddPrimaryEntityCommandAllSpawns(Player, SpawnID, Name, Distance, Command, ErrorText, CastTime, SpellVisual, DenyListDefault)

**Description:**
Puts a context menu (right click) entity command on all the Spawn's by the Spawn ID in the zone for the Player.

**Parameters:**
- `Player`: Spawn - The Player that can access the entity command.  Optionally set to nil to allow all access.
- `SpawnID`: UInt32 - The spawn id of all related Spawns to add the entity command.
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
-- Example usage: On Hail add context command to Spawn's with Spawn ID 123.

function hail(NPC, Spawn)
	AddPrimaryEntityCommandAllSpawns(Spawn, 123, "Destroy", 5)
end

function casted_on(NPC, Spawn, SpellName)
	if SpellName == 'Destroy' then
		-- perform action
	end	
end
```