### Function: GiveExp(player, amount)

**Description:**
Awards a certain amount of experience points to the specified player. This can be used to grant quest rewards or bonus experience outside the normal combat exp flow.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `amount` (uint32) - Quantity `amount`.

**Returns:** None.

**Example:**

```lua
-- From Quests/FarJourneyFreeport/TasksaboardtheFarJourney.lua
function Step1Complete(Quest, QuestGiver, Player)
	GiveExp(Player, 110)
	Step2Init(Quest, QuestGiver, Player)
	CurrentStep(Quest, QuestGiver, Player)
end
```
