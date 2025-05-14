Function: AddWard(WardAmount, KeepWard, WardType, DamageTypes, DamageAbsorptionPct, DamageAbsorptionMaxHealthPct, RedirectDamagePct, MaxHitCount)

Description: Used in a Spell Script.  Applies a protective ward to all spell targets of the spell. A ward absorbs a certain amount of incoming damage before it expires. This function creates a new ward on the target with the given value.

Parameters:

    WardAmount: UInt32 – The amount of damage the ward can absorb.
    KeepWard: Boolean – The amount of damage the ward can absorb.
    WardType: UInt8 – The amount of damage the ward can absorb. Options: ALL = 0, Physical Only = 1, Magical Only = 2.
    DamageTypes: UInt8 – The amount of damage the ward can absorb.  If Ward is Magical Only then 0 allows any Magical or select a Damage Type: https://github.com/emagi/eq2emu/blob/main/docs/data_types/damage_types.md
    DamageAbsorptionPct: UInt32 – The amount of damage the ward can absorb.  Max of 100.
    DamageAbsorptionMaxHealthPct: UInt32 – The amount of damage the ward can absorb.  Max of 100.
    RedirectDamagePct: UInt32 – The amount of damage the ward can absorb.
    MaxHitCount: Int32 – The player or NPC to protect with the ward.

Returns: None.

Example:

-- Example usage (shield the player with a 500 hp point ward)
AddWard(500)