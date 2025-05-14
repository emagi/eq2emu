Function: GetQuestFlags(Quest)

Description: Retrieves the bitwise flags set for a quest. These flags might denote various quest properties (such as hidden, completed, failed, etc.). This is more of an internal function for quest data management.

Parameters:

    Quest: Quest – The quest object in question.

Returns: Int32 – The flags value for the quest.

Example:

-- Example usage (debugging quest state flags)
local flags = GetQuestFlags(Quest)
print("Quest flags: " .. flags)