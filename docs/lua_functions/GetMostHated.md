Function: GetMostHated(NPC)

Description: Retrieves the spawn that currently has the highest hate (aggro) on the specified NPC’s hate list. This is usually the NPC’s current primary target in combat.

Parameters:

    NPC: Spawn – The NPC whose hate list to examine.

Returns: Spawn – The entity with top hate on the NPC (often the tank or highest damage dealer), or nil if the NPC has no hate list.

Example:

-- Example usage (make the boss shout at whoever has top aggro)
local topAggro = GetMostHated(BossNPC)
if topAggro ~= nil then
    Say(BossNPC, "I will destroy you, " .. GetName(topAggro) .. "!")
end