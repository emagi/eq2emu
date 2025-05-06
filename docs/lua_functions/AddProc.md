### Function: AddProc(Spawn, Type, Chance, Item, UseAllSpellTargets)

**Description:**
Add's a proc for a Spell and calls function proc when proc succeeds.

**Parameters:**
- `Spawn`: Spawn - The spawn or entity involved.
- `Type`: int8 - See PROC_TYPE defines.
- `Chance`: float - Chance of proc 1-100.
- `Item`: Item - An item reference.
- `UseAllSpellTargets`: int8 - Default is 0, when set to 1 all spell targets will apply the proc not the Spawn

**Returns:** None.

**Example:**

```lua
-- Example usage: Example Spell Script, when casted Target has 10% chance to trigger proc on type 15 (PROC_TYPE_DAMAGED) when damaged.

function cast(Caster, Target)
    -- 10% chance to dispel when target takes damage
    AddProc(Target, 15, 10.0)
end

function proc(Caster, Target, Type)
    if Type == 15 then
        CancelSpell()
    end
end

```
