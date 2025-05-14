Function: DismissPet(Spawn)

Description: Dismisses (despawns) the specified player’s active pet. This works for combat pets, cosmetic pets, deity pets, etc., causing them to vanish as if the player dismissed them manually.

Parameters:

    Spawn: Spawn – The player whose pet should be dismissed.

Returns: None.

Example:

-- Example usage (dismissing a pet at the end of an event)
DismissPet(Player)