Function: EndAutoMount(Spawn)

Description: Dismounts a player who was auto-mounted via StartAutoMount. Typically called at the end of an automated travel route or upon leaving the area where auto-mount is enforced.

Parameters:

    Spawn: Spawn – The player to dismount.

Returns: None.

Example:

-- Example usage (dismount the player after griffon flight ends)
EndAutoMount(Player)