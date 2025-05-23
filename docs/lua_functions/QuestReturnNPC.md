### Function: QuestReturnNPC(quest, spawn_id)

**Description:**
Sets the return NPC point for a Quest when the quest has been completed.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `spawn_id` (uint32) - Integer value `spawn_id`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Hallmark/archetype_selection.lua
function Init(Quest)
    AddQuestStepChat(Quest, 1, "I need to talk to Garven Tralk", 1, "I need to talk to Garven Tralk", 11, 3250020)
	AddQuestStepCompleteAction(Quest, 1, "QuestComplete")
    QuestReturnNPC(Quest, 3250020)
end
```
