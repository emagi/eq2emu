### Function: AddProcExt(Spawn, ProcType, DamageType, Chance, HPRatio, BelowHealth, TargetHealth, Item, UseAllSpellTargets)

**Description:**
Add's a proc for a Spell and calls function proc_ext when proc succeeds.

**Parameters:**
- `Spawn`: Spawn - The spawn or entity involved.
- `ProcType`: int8 - See ProcType options
- `DamageType`: int8 - See DamageType options
- `Chance`: float - Floating point value.
- `HPRatio`: int8 - Small integer or boolean flag.
- `BelowHealth`: bool - Use HPRatio against health lower than if false (default).  Otherwise if health is higher when true.
- `TargetHealth`: bool - Use Target's Health for formula, default is false (use self Spawn health).  Otherwise when true use Target's health.
- `Item`: Item - An item reference.
- `UseAllSpellTargets`: int8 - By default 0 (false) use just Spawn, otherwise use all Spell Targets.

**Returns:** None.

**Example:**

```lua
-- Example usage: Spell Script when on cast there is a AddProcExt called for when player is going to die Type 13 PROC_TYPE_DEATH
				  - then proc_ext is called, which in turn casts the RedemptionReactiveSpell and the RemoveProc(Target) is called to remove spell from Target.
local RedemptionReactiveSpell = 2550537

function cast(Caster, Target, HealAmt, MaxHealthAmt)
	AddProcExt(Target, 13, 255, 100.0)
end

function proc_ext(Caster, Target, Type, DamageType, InitialCaster, HealAmt, MaxHealthAmt)
    CastSpell(Caster, RedemptionReactiveSpell, GetSpellTier(), InitialCaster)
	SetSpellTriggerCount(1, 1)
	RemoveProc(Target)
end

function remove(Caster, Target)
	RemoveProc(Target)
end
```
