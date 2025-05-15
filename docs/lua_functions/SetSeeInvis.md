Function: SetSeeInvis(Spawn, Enable)

Description: Toggles an NPC’s ability to see invisible on or off.

Parameters:

    Spawn: Spawn – The NPC whose invis detection to set.

    Enable: Boolean – true to grant see-invis; false to revoke it.

Returns: None.

Example:

-- Example usage (temporarily allow a boss to see invis during a phase)
SetSeeInvis(BossNPC, true)