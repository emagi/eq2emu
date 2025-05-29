### Function: CheckRaceType(Spawn, RaceTypeID)

**Description:** Checks if the given spawn’s race matches a specific race type category (such as “Undead,” “Giant,” “Animal,” etc.). This is used in quests or abilities that target categories of creatures.

**Parameters:**

`Spawn`: Spawn – The entity whose race to check.
`RaceTypeID`: Int32 – The ID of the race type category to compare against.

**Returns:** Boolean – true if the spawn belongs to that race type; false otherwise.

**Example:**

```lua
-- Example usage (quest update if target is of the Gnoll race type)
if CheckRaceType(TargetNPC, RACE_TYPE_GNOLL) then
    UpdateQuestStep(Player, QuestID, Step)
end
```