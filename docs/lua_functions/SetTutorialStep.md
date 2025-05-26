### Function: SetTutorialStep(player, step)

**Description:**
Sets the current tutorial step for the Player.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `step` (uint8) - Integer value `step`.

**Returns:** None.

**Example:**

```lua
-- From Quests/FarJourneyFreeport/TasksaboardtheFarJourney.lua
function Step3Init(Quest, QuestGiver, Player)
	SetTutorialStep(Player, 17)
	UpdateQuestStepDescription(Quest, 2, "I found Waulon's hat.")
	UpdateQuestTaskGroupDescription(Quest, 2, "I found Waulon's hat in one of the boxes.")

	AddQuestStepChat(Quest, 3, "I should speak to Waulon.", 1, "Now that I found Waulon's hat, I should return it.", 258, Waulon)
	AddQuestStepCompleteAction(Quest, 3, "Step3Complete")
end
```
