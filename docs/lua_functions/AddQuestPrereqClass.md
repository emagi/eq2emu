Function: AddQuestPrereqClass(Quest: quest, Int8: ClassID)

Description: Add a pre requirement class to the Quest.

Parameters:
    quest: Quest - Quest to apply the class pre req.
    ClassID: Int8 - class id from the classes ID's.

Returns: None.

Example:

-- Example usage: Restricts Quest to Inquisitor(14) class.
AddQuestPrereqClass(Quest, 14)