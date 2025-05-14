Function: GetQuestCompleteCount(Spawn, QuestID)

Description: Retrieves how many times the specified player (Spawn) has completed a particular quest (identified by QuestID). For non-repeatable quests this is usually 0 or 1; for repeatable quests it could be higher.

Parameters:

    Spawn: Spawn – The player to check.

    QuestID: Int32 – The quest ID to count completions of.

Returns: Int32 – The number of times the player has completed that quest.

Example:

-- Example usage (give a different dialogue if the player has done a quest multiple times)
local timesDone = GetQuestCompleteCount(Player, 9001)
if timesDone > 0 then
    Say(NPC, "Back again? You know the drill.")
end