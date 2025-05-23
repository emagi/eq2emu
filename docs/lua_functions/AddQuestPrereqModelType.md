### Function: AddQuestPrereqModelType(quest, model_type)

**Description:**
CanReceiveQuest checks if the Player is the model_type defined.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `model_type` (int32) - Integer value `model_type`.

**Returns:** None.

**Example:**

```lua
-- Require the Player to have a model_type of 123 for the Quest
function Init(Quest)
    AddQuestPrereqModelType(Quest, 123)
end
```
