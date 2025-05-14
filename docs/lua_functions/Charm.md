Function: Charm(Spawn, Target)

Description: Must be used inside a Spell Script.  Sets the Target to become a Pet of the Spawn (Owner).

Parameters:

    Spawn: Spawn – The entity casting the charm (e.g., a player or NPC that will become the controller).

    Target: Spawn – The NPC to be charmed.

Returns: None.

Example:

-- Example usage (NPC charms another NPC during an encounter)
Charm(NPC, EnemyNPC)