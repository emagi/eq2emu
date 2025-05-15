Function: SetInvulnerable(Spawn, Enable)

Description: Toggles an entity’s invulnerability. When set to true, the spawn will not take damage from any source.

Parameters:

    Spawn: Spawn – The entity to modify.

    Enable: Boolean – true to make invulnerable; false to remove invulnerability.

Returns: None.

Example:

-- Example usage (make an NPC invulnerable during a dialogue scene)
SetInvulnerable(QuestNPC, true)