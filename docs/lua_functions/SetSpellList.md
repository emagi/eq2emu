### Function: SetSpellList(spawn, primary_list, secondary_list)

**Description:**
Set the spell list of the Spawn for primary and secondary list.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `primary_list` (uint32) - Integer value `primary_list`.
- `secondary_list` (uint32) - Integer value `secondary_list`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Antonica/abloodsaberseditionist.lua
function ChooseClass(NPC)
    SetClass = MakeRandomInt(1,2)
    if SetClass == 1 then
        SpawnSet(NPC,"class", 2)
        SetSpellList(NPC, 451)
        IdleAggressive(NPC)
        DervishChain(NPC)
    elseif SetClass == 2 then
        SpawnSet(NPC, "class", 32)
        SetSpellList(NPC, 469)
        IdleAlert(NPC)
        DervishLeather(NPC)
    end
```
