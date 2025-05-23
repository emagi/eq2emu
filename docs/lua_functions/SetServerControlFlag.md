### Function: SetServerControlFlag(spawn, param, param_value, value)

**Description:**
Tie to the control effect types found in https://github.com/emagi/eq2emu/blob/main/docs/data_types/control_effect_types.md

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `param` (int32) - Integer value `param`.
- `param_value` (int32) - Integer value `param_value`.
- `value` (int32) - Integer value `value`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/everfrost_frostfell_new01/BrenloBixiebopperVI1586827.lua
function InRange(NPC, Spawn)
        Say(NPC, "Hey! Get outta my way! I am practicing for the all Halfling Olympics!")
        --SetServerControlFlag(Spawn, 4, 64, 1) 
        PlayAnimation(Spawn, 11767)
        --AddTimer(NPC, 4000, "Control", 1, Spawn)
end
```
