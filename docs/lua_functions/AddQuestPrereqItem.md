### Function: AddQuestPrereqItem(quest, item_id, quantity)

**Description:**
Not implemented.  CanReceiveQuest should require an item of item_id and quantity to be allowed to receive the quest.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `item_id` (uint32) - Integer value `item_id`.
- `quantity` (int32) - Integer value `quantity`.

**Returns:** None.

**Example:**

```lua
-- Require the item with item_id 123 and a quantity of 2 be required to receive the quest.
function Init(Quest)
    AddQuestPrereqItem(Quest, 123, 2)
end
```
