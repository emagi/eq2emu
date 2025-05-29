### Function: AddSpellTimer(DelayMS, FunctionName, Caster, Target)

**Description:** Used only in a Spell Script.  Schedules a call to a function (by name) in the spawn’s script after a specified delay in milliseconds. This is typically used within NPC scripts to create timed events (like delayed attacks or actions) without blocking the main thread.

**Parameters:**

`DelayMS`: Int32 – The delay in milliseconds before the function is called.
`FunctionName`: String – The name of the function in the NPC’s Lua script to call when the timer expires.
`Caster`: Spawn – The source spawn who was the caster/originator.
`Target`: Spawn – The target spawn who will be included as a secondary argument

**Returns:** None.

**Example:**

```lua
-- taken from Spells/Commoner/Knockdown.lua
-- Timer argument taken from spell data in the database, after Timer elapses we call RemoveStunBlur
function cast(Caster, Target, Timer)
if not IsEpic(Target) then
        PlayAnimation(Target, 72)
		AddControlEffect(Target, 4)
        BlurVision(Target, 1.0)
        AddSpellTimer(Timer, "RemoveStunBlur")
    end
end

function RemoveStunBlur(Caster, Target)
    RemoveControlEffect(Target, 4)
    BlurVision(Target, 0)
end
```