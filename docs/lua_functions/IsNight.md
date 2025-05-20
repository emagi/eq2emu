### Function: IsNight(Zone)

***Description:***
Checks if it is currently nighttime in the specified zone. This usually refers to the game’s day/night cycle.

**Parameters:**
- `Zone`: Zone - The zone to check if the time is night.

**Returns:** Return's true if the current time is dusk/night.  Otherwise return's false.

**Example:**

```lua
-- Example usage: On hail tell the user if it is night or day

function hail(NPC,Spawn)
	if IsNight(GetZone(NPC)) then
		Say(NPC, "It is night!")
	else
		Say(NPC, "It is day!")
	end
end
```
