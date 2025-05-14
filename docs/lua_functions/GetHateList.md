Function: GetHateList(NPC)

Description: Returns a list of all spawns currently on the specified NPC’s hate (aggro) list. This includes anyone who has attacked or otherwise generated hate on the NPC.

Parameters:

    NPC: Spawn – The NPC whose hate list to retrieve.

Returns: Table – A list/array of spawns that the NPC currently hates, typically sorted by hate amount (highest first).

Example:

-- Example usage (apply an effect to all players an epic boss has aggro on)
for _, enemy in ipairs(GetHateList(EpicBoss)) do
    CastSpell(EpicBoss, AOE_DEBUFF_ID, 1, enemy)
end